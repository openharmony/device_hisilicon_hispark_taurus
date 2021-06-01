/*
 * Copyright (C) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "watchdog.h"
#ifdef __HuaweiLite__
#include "pthread.h"
#include <sys/prctl.h>
#endif
#include "hi_osal.h"
#include "hi_wtdg_hal.h"
#include "securec.h"

#ifndef NULL
#define NULL  ((void *)0)
#endif

#define hiwdt_reg(x)    (HIWDT_BASE + (x))

#define HIWDT_LOAD      0x000
#define HIWDT_VALUE     0x004
#define HIWDT_CTRL      0x008
#define HIWDT_INTCLR    0x00C
#define HIWDT_RIS       0x010
#define HIWDT_MIS       0x014
#define HIWDT_LOCK      0xC00

#define HIWDT_UNLOCK_VAL    0x1ACCE551

volatile void *g_wtdg_reg_base = NULL;

#define hiwdt_io_address(x) ((uintptr_t)g_wtdg_reg_base + (x) - HIWDT_BASE)

#define hiwdt_readl(x)       osal_readl(hiwdt_io_address(hiwdt_reg(x)))
#define hiwdt_writel(v, x)   osal_writel(v, hiwdt_io_address(hiwdt_reg(x)))

/* debug */
#define HIDOG_PFX "HiDog: "
#define hidog_dbg(params...) osal_printk(HIDOG_PFX params)

/* module param */
#define HIDOG_TIMER_MARGIN    60
int default_margin = HIDOG_TIMER_MARGIN;    /* in seconds */
#define HIDOG_TIMER_DEMULTIPLY  9

#define wtdg_check_securec_return(err) \
    do { \
        if ((err) != EOK) { \
            return (-1); \
        } \
    } while (0)

int nodeamon = 0;

/* watchdog info */
static struct watchdog_info g_ident = {
    .options =  WDIOF_SETTIMEOUT |
                WDIOF_KEEPALIVEPING |
                WDIOF_MAGICCLOSE,
    .firmware_version = 0,
    .identity = "Hisilicon Watchdog",
};

/* local var */
static struct osal_spinlock g_hidog_lock;
static int g_cur_margin = HIDOG_TIMER_MARGIN;
static int g_need_iounmap = 0;

#ifdef __HuaweiLite__
pthread_t g_task_hidog_deamon = 0;
#else
struct osal_task *g_task_hidog_deamon = NULL;
#endif

#define HIDOG_EXIT      0
#define HIDOG_SELFCLR   1
#define HIDOG_EXTCLR    2

static volatile unsigned int g_hidog_state = 0;
static osal_atomic_t g_driver_open;
static unsigned int g_options = WDIOS_ENABLECARD;

#ifndef MHZ
#define MHZ (1000 * 1000)
#endif

static const unsigned long g_rate = 3 * MHZ; /* 3MHZ */

static void hidog_set_timeout(unsigned int nr)
{
    unsigned int cnt_0 = ~0x0;
    unsigned int cnt = cnt_0 / g_rate;  /* max cnt */
    unsigned long flags;

    osal_spin_lock_irqsave(&g_hidog_lock, &flags);

    if ((nr == 0) || (nr > cnt)) {
        cnt = ~0x0;
    } else {
        cnt = nr * g_rate;
    }
    /* unlock watchdog registers */
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
    hiwdt_writel(cnt, HIWDT_LOAD);
    hiwdt_writel(cnt, HIWDT_VALUE);
    /* lock watchdog registers */
    hiwdt_writel(0, HIWDT_LOCK);
    osal_spin_unlock_irqrestore(&g_hidog_lock, &flags);
}

static void hidog_feed(void)
{
    unsigned long flags;

    /* read the RIS state of current wdg */
    unsigned int v = (unsigned int)hiwdt_readl(HIWDT_RIS);
    v &= 0x1; /* 0x1: get INT bit [1] */
    if (v == 0) { /* no INT on current wdg */
        return;
    }

    osal_spin_lock_irqsave(&g_hidog_lock, &flags);
    /* unlock watchdog registers */
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
    /* clear watchdog */
    hiwdt_writel(0x00, HIWDT_INTCLR);
    /* lock watchdog registers */
    hiwdt_writel(0, HIWDT_LOCK);
    osal_spin_unlock_irqrestore(&g_hidog_lock, &flags);
}

static int hidog_set_heartbeat(int t)
{
    int ret = 0;
    unsigned int cnt_0 = ~0x0;
    unsigned int cnt = cnt_0 / g_rate;

    if (t <= 0) {
        osal_printk("set heartbeat less or equal to 0, heartbeat will not be changed.\n");
        t = g_cur_margin;
        ret = -1;
    } else if ((unsigned int)t > cnt) {
        osal_printk("set heartbeat range error, t = %d\n", t);
        osal_printk("force heartbeat to %u\n", cnt);
        t = cnt;
        ret = -1;
    }

    g_cur_margin = t;

    hidog_set_timeout(t);
    hidog_feed();

    return ret;
}

static int hidog_keepalive(void)
{
    hidog_feed();
    return 0;
}

static void hidog_start(void)
{
    unsigned long flags;

    osal_spin_lock_irqsave(&g_hidog_lock, &flags);
    /* unlock watchdog registers */
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
    hiwdt_writel(0x00, HIWDT_CTRL);     /* 0x00: disable watch dog reset signal and interrupt */
    hiwdt_writel(0x00, HIWDT_INTCLR);   /* 0x00: clear interrupt */
    hiwdt_writel(0x03, HIWDT_CTRL);     /* 0x03: enable watch dog reset signal and interrupt */
    /* lock watchdog registers */
    hiwdt_writel(0, HIWDT_LOCK);
    osal_spin_unlock_irqrestore(&g_hidog_lock, &flags);

    g_options = WDIOS_ENABLECARD;
}

static void hidog_stop(void)
{
    unsigned long flags;

    osal_spin_lock_irqsave(&g_hidog_lock, &flags);
    /* unlock watchdog registers */
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
    /* stop watchdog timer */
    hiwdt_writel(0x00, HIWDT_CTRL);     /* 0x00: disable watch dog reset signal and interrupt */
    hiwdt_writel(0x00, HIWDT_INTCLR);   /* 0x00: clear interrupt */
    /* lock watchdog registers */
    hiwdt_writel(0, HIWDT_LOCK);
    osal_spin_unlock_irqrestore(&g_hidog_lock, &flags);

    hidog_set_timeout(0);
    g_options = WDIOS_DISABLECARD;
}

static int hidog_open(void *private_data)
{
    int ret = 0;
    hi_wtdg_unused(private_data);

    if (osal_atomic_dec_return(&g_driver_open) != 0) {
        ret = osal_atomic_inc_return(&g_driver_open);
        osal_printk("Error: device already open:%d.\n", ret);
        return -1;
    }

    hidog_keepalive();

    return ret;
}

static int hidog_release(void *private_data)
{
    hi_wtdg_unused(private_data);

    if (osal_atomic_inc_return(&g_driver_open) != 1) {
        osal_atomic_dec_return(&g_driver_open);
        return 0;
    }

    g_hidog_state = HIDOG_SELFCLR;
    hidog_set_heartbeat(g_cur_margin);

    if (g_options == WDIOS_DISABLECARD) {
        osal_printk("Watchdog is disabled!\n");
    }
    return 0;
}

static long hidog_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    void *argp = (void *)(uintptr_t)arg;
    int new_margin;
    unsigned int new_options;
    errno_t err;
    hi_wtdg_unused(private_data);

    switch (cmd) {
        case WDIOC_GETSUPPORT:
            err = memcpy_s(argp, sizeof(g_ident), &g_ident, sizeof(g_ident));
            wtdg_check_securec_return(err);
            return 0;

        case WDIOC_GETSTATUS:
        case WDIOC_GETBOOTSTATUS:
            err = memcpy_s((int *)argp, sizeof(int), &g_options, sizeof(int));
            wtdg_check_securec_return(err);
            return 0;

        case WDIOC_KEEPALIVE:
            hidog_keepalive();
            return 0;

        case WDIOC_SETTIMEOUT:
            err = memcpy_s(&new_margin, sizeof(int), (int *)argp, sizeof(int));
            wtdg_check_securec_return(err);

            if (hidog_set_heartbeat(new_margin)) {
                return -1;
            }
            hidog_keepalive();
            return 0;

        case WDIOC_GETTIMEOUT:
            err = memcpy_s((int *)argp, sizeof(int), &g_cur_margin, sizeof(int));
            wtdg_check_securec_return(err);
            return 0;

        case WDIOC_SETOPTIONS:
            err = memcpy_s(&new_options, sizeof(int), (int *)argp, sizeof(int));
            wtdg_check_securec_return(err);

            if (new_options & WDIOS_ENABLECARD) {
                hidog_start();
                hidog_set_heartbeat(g_cur_margin);
                return 0;
            } else if (new_options & WDIOS_DISABLECARD) {
                hidog_stop();
                return 0;
            } else {
                return -WDIOS_UNKNOWN;
            }

        default:
            return -1;
    }
}

/* Kernel Interfaces */
static struct osal_fileops g_hidog_fops = {
    .unlocked_ioctl = hidog_ioctl,
    .open           = hidog_open,
    .release        = hidog_release,
};

static struct osal_dev *g_hidog_miscdev = NULL;

static int hidog_deamon(void *data)
{
    hi_wtdg_unused(data);

#ifdef __HuaweiLite__
    prctl(PR_SET_NAME, "hidog_deamon", 0, 0, 0);
#endif
    while (g_hidog_state != HIDOG_EXIT) {
        switch (g_hidog_state) {
            case HIDOG_SELFCLR:
                if (g_options & WDIOS_ENABLECARD) {
                    hidog_feed();
                }
                break;
            case HIDOG_EXTCLR:
                break;
            default:
                break;
        }
        /* sleep; when self feed dog, only use the default margin */
        osal_msleep(default_margin * 1000 / 2 + 10);  /* sleep (60*1000/2 + 10)->30.01s */
    }

    return 0;
}

static int hidog_init(void)
{
    hidog_start();

    g_hidog_state = HIDOG_SELFCLR;
    if (!nodeamon) {
#ifdef __HuaweiLite__
        if (pthread_create(&g_task_hidog_deamon, NULL, (void *)hidog_deamon, 0) < 0) {
            osal_printk("create hidog_deamon failed!\n");
            return -1;
        }
#else
        struct osal_task *dog = NULL;

        dog = osal_kthread_create(hidog_deamon, NULL, "hidog");
        if (dog == NULL) {
            osal_printk("create hidog_deamon failed!\n");
            return -1;
        }
        g_task_hidog_deamon = dog;
#endif
    }
    return 0;
}

static int get_margin(void)
{
    int ret = default_margin;

    /* Check that the default_margin value is within it's range ; if not reset to the default */
    if (hidog_set_heartbeat(default_margin)) {
        default_margin = HIDOG_TIMER_MARGIN;
        hidog_set_heartbeat(HIDOG_TIMER_MARGIN);
        osal_printk("default_margin value must be 0<default_margin<%lu, using %d\n",
            (~0x0UL) / g_rate, HIDOG_TIMER_MARGIN);
    }

    return ret;
}

static int ptr_ioremap(void)
{
    if (g_wtdg_reg_base == NULL) {
        g_wtdg_reg_base = (volatile void *)osal_ioremap(HIWDT_BASE, 0x1000); /* 0x1000: watch dog reg length */
        if (g_wtdg_reg_base == NULL) {
            osal_printk("osal_ioremap err. \n");
            osal_spin_lock_destroy(&g_hidog_lock);
            osal_atomic_destroy(&g_driver_open);
            return -1;
        }
        g_need_iounmap = 1;
    }
    return 1;
}

int watchdog_init(void)
{
    if (osal_atomic_init(&g_driver_open) != 0) {
        osal_printk("Error: init atomic\n");
        return -1;
    }
    osal_atomic_set(&g_driver_open, 1);

    if (osal_spin_lock_init(&g_hidog_lock) != 0) {
        osal_printk("function %s line %d failed\n", __FUNCTION__, __LINE__);
        goto spin_lock_init_fail;
    }

    if (ptr_ioremap() != 1) {
        return -1;
    }

    g_cur_margin = get_margin();

    g_hidog_miscdev = osal_createdev("watchdog");
    if (g_hidog_miscdev == NULL) {
        osal_printk("fail to create dev\n");
        goto create_dev_fail;
    }

    g_hidog_miscdev->minor = WATCHDOG_MINOR;
    g_hidog_miscdev->fops = &g_hidog_fops;
    if (osal_registerdevice(g_hidog_miscdev) != 0) {
        osal_printk("fail to register dev\n");
        goto register_dev_fail;
    }

    if (hidog_init() != 0) {
        goto dog_init_fail;
    }

    osal_printk("Hisilicon Watchdog Timer: 0.01 initialized. default_margin=%d sec (nodeamon= %d)\n",
        default_margin, nodeamon);
    osal_printk("hiwtdg init ok. ver=%s, %s.\n", __DATE__, __TIME__);
    return 0;

dog_init_fail:
    osal_deregisterdevice(g_hidog_miscdev);
register_dev_fail:
    osal_destroydev(g_hidog_miscdev);
create_dev_fail:
    if (g_need_iounmap) {
        osal_iounmap((void *)g_wtdg_reg_base, 0x1000);
        g_need_iounmap = 0;
        g_wtdg_reg_base = NULL;
    }
    osal_spin_lock_destroy(&g_hidog_lock);
spin_lock_init_fail:
    osal_atomic_destroy(&g_driver_open);
    return -1;
}

static void hidog_exit(void)
{
    hidog_set_timeout(0);
    hidog_stop();
    g_hidog_state = HIDOG_EXIT;
    if (!nodeamon) {
#ifdef __HuaweiLite__
        pthread_join(g_task_hidog_deamon, NULL);
#else
        struct osal_task *p_dog = g_task_hidog_deamon;
        if (p_dog == NULL) {
            return;
        }

        osal_kthread_destroy(p_dog, 1);
#endif
        osal_yield();
    }

#ifdef __HuaweiLite__
    g_task_hidog_deamon = 0;
#else
    g_task_hidog_deamon = NULL;
#endif
}

void watchdog_exit(void)
{
    hidog_exit();

    osal_deregisterdevice(g_hidog_miscdev);
    osal_destroydev(g_hidog_miscdev);
    if (g_need_iounmap) {
        osal_iounmap((void *)g_wtdg_reg_base, 0x1000);
        g_need_iounmap = 0;
        g_wtdg_reg_base = NULL;
    }
    osal_spin_lock_destroy(&g_hidog_lock);
    osal_atomic_destroy(&g_driver_open);
    osal_printk("hiwtdg exit ok.\n");
}

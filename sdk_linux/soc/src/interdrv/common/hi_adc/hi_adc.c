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

#include "hi_adc.h"
#include "hi_osal.h"
#include "hi_adc_hal.h"

#ifndef NULL
#define NULL  ((void *)0)
#endif

#define OSDRV_MODULE_VERSION_STRING "HISI_LSADC"

#define lsadc_print(fmt, ...) osal_printk("Func:%s, Line:%d, "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define lsadc_addr_offset(x)    (LSADC_BASE_ADDR + (x))

#define HISI_LSADC_CONFIG       0x00
#define HISI_LSADC_GLITCH       0x04
#define HISI_LSADC_TIMESCAN     0x08
#define HISI_LSADC_INTEN        0x10
#define HISI_LSADC_INTSTATUS    0x14
#define HISI_LSADC_INTCLR       0x18
#define HISI_LSADC_START        0x1C
#define HISI_LSADC_STOP         0x20
#define HISI_LSADC_ACTBIT       0x24
#define HISI_LSADC_CHNDATA      0x2C

volatile void *g_lsadc_reg_base = NULL;
unsigned int g_lsadc_irq = LSADC_IRQ_ID;
static osal_spinlock_t g_lsadc_lock;
static int g_need_iounmap = 0;

#define lsadc_vir_addr(x)   ((uintptr_t)g_lsadc_reg_base + (x) - LSADC_BASE_ADDR)
#define lsadc_writel(v, x)  osal_writel(v, lsadc_vir_addr(lsadc_addr_offset(x)))
#define lsadc_readl(x)      osal_readl(lsadc_vir_addr(lsadc_addr_offset(x)))

static inline void lsadc_reg_write(unsigned long value, unsigned long mask, unsigned long addr)
{
    unsigned long t;

    t = lsadc_readl(addr);
    t &= ~mask;
    t |= value & mask;
    lsadc_writel(t, addr);
}

static void write_reg32(unsigned int value, unsigned int mask, const void *addr)
{
    unsigned int t;

    t = osal_readl((void *)addr);
    t &= ~mask;
    t |= value & mask;
    osal_writel(t, (void *)addr);
}

static int hilsadc_open(void *private_data)
{
    hi_adc_unused(private_data);

    lsadc_reg_write(0 << 15, 1 << 15, HISI_LSADC_CONFIG); /* 15: unreset analog circuit [15] */
    return 0;
}

static int hilsadc_release(void *private_data)
{
    hi_adc_unused(private_data);

    lsadc_reg_write(1 << 15, 1 << 15, HISI_LSADC_CONFIG); /* 15: reset analog circuit [15] */
    return 0;
}

/*
 * 0: single scanning mode
 * 1: continuous scanning mode
 * The filter glitch is already enable, only the continuous mode has a filter glitch, and the single mode is invalid.
 */
static int lsadc_model_select(int value)
{
    unsigned int val;
    unsigned long flag;

    val = (unsigned int)value;
    if ((val != 0) && (val != 1)) {
        lsadc_print("error value:%x\n", val);
        return -1;
    }

    osal_spin_lock_irqsave(&g_lsadc_lock, &flag);
    lsadc_reg_write(val << 13, 1 << 13, HISI_LSADC_CONFIG); /* 13: [13] bit */
    if (val == 1) {
        lsadc_reg_write(0xc << 20, 0xc << 20, HISI_LSADC_CONFIG); /* 20, 0xc: [23:20] bits */

        lsadc_writel(GLITCH, HISI_LSADC_GLITCH); /* glitch_sample, must > 0 */
        lsadc_writel(TIME_SCAN, HISI_LSADC_TIMESCAN); /* time_scan, must > 20 */

        lsadc_reg_write(0 << 17, 0 << 17, HISI_LSADC_CONFIG); /* 17: [17] bit set glitch not bypass */
    } else {
        lsadc_reg_write(1 << 17, 1 << 17, HISI_LSADC_CONFIG); /* 17: [17] bit set glitch bypass */
    }
    osal_spin_unlock_irqrestore(&g_lsadc_lock, &flag);

    return 0;
}

static int lsadc_chn_valid(int chn, int enable)
{
    unsigned long flag;
    unsigned long value;

    osal_spin_lock_irqsave(&g_lsadc_lock, &flag);
    value = enable ? 1 : 0;
    switch (chn) {
        case 0: /* chn 0 */
            lsadc_reg_write(value << 8, 1 << 8, HISI_LSADC_CONFIG); /* 8: [8] bit for chn 0 */
            break;
        case 1: /* chn 1 */
            lsadc_reg_write(value << 9, 1 << 9, HISI_LSADC_CONFIG); /* 9: [9] bit for chn 1 */
            break;
#if (LSADC_MAX_CHN_NUM == 4)
        case 2: /* chn 2 */
            lsadc_reg_write(value << 10, 1 << 10, HISI_LSADC_CONFIG); /* 10: [10] bit for chn 2 */
            break;
        case 3: /* chn 3 */
            lsadc_reg_write(value << 11, 1 << 11, HISI_LSADC_CONFIG); /* 11: [11] bit for chn 3 */
            break;
#endif
        default:
            osal_spin_unlock_irqrestore(&g_lsadc_lock, &flag);
            lsadc_print("error chn:%d\n", chn);
            return -1;
    }
    osal_spin_unlock_irqrestore(&g_lsadc_lock, &flag);

    return 0;
}

static int lsadc_enable_clock(void)
{
    void *lsadc_crg_addr = NULL;

    lsadc_crg_addr = osal_ioremap(LSADC_CRG_ADDR, (unsigned long)LSADC_CLOCK_REG_LENGTH);
    if (lsadc_crg_addr == NULL) {
        return -1;
    }

    write_reg32(0x1 << ADC_BIT, 0x1 << ADC_BIT, lsadc_crg_addr); /* 0x1: [1] bit for clock control */

    osal_iounmap((void *)lsadc_crg_addr, (unsigned long)LSADC_CLOCK_REG_LENGTH);

    return 0;
}

static void lsadc_disable_clock(void)
{
    void *lsadc_crg_addr = NULL;

    lsadc_crg_addr = osal_ioremap(LSADC_CRG_ADDR, (unsigned long)LSADC_CLOCK_REG_LENGTH);
    if (lsadc_crg_addr == NULL) {
        return;
    }

    write_reg32(0x0 << ADC_BIT, 0x1 << ADC_BIT, lsadc_crg_addr); /* 0x1: [1] bit for clock control */

    osal_iounmap((void *)lsadc_crg_addr, (unsigned long)LSADC_CLOCK_REG_LENGTH);
}

static int lsadc_start(void)
{
    unsigned long flag;
    osal_spin_lock_irqsave(&g_lsadc_lock, &flag);

#ifdef ENABLE_ADC_IRQ
    lsadc_reg_write(1, 1, HISI_LSADC_INTEN); /* int enable */
#endif

    lsadc_reg_write(1, 1, HISI_LSADC_START); /* start */

    osal_spin_unlock_irqrestore(&g_lsadc_lock, &flag);

    return 0;
}

static int lsadc_stop(void)
{
    unsigned long flag;
    osal_spin_lock_irqsave(&g_lsadc_lock, &flag);

    lsadc_reg_write(1, 1, HISI_LSADC_STOP); /* 1: [1] bit for adc control, stop adc */

#ifdef ENABLE_ADC_IRQ
    lsadc_reg_write(0, 1, HISI_LSADC_INTEN); /* 1: [1] bit for int enable control, disable int */
#endif

    osal_spin_unlock_irqrestore(&g_lsadc_lock, &flag);

    return 0;
}

static int lsadc_get_chn_value(unsigned int chn)
{
    if (chn >= LSADC_MAX_CHN_NUM) {
        lsadc_print("error chn:%u\n", chn);
        return -1;
    }

    return lsadc_readl(HISI_LSADC_CHNDATA + (chn << 2)); /* 2: each chn has 4 bytes int data, so shift left 2 bits */
}

#ifdef ENABLE_ADC_IRQ
static int lsadc_irq_proc(int irq, void *dev_id)
{
    unsigned int intstate;
    int chn_value;
    unsigned int chn;
    unsigned long flag;
    osal_spin_lock_irqsave(&g_lsadc_lock, &flag);

    intstate = lsadc_readl(HISI_LSADC_INTSTATUS);
    lsadc_reg_write(LSADC_CHN_MASK, LSADC_CHN_MASK, HISI_LSADC_INTCLR);  /* clr int flag all */

    osal_spin_unlock_irqrestore(&g_lsadc_lock, &flag);

    for (chn = 0; chn < LSADC_MAX_CHN_NUM; chn++) {
        if (intstate & (1 << chn)) {
            chn_value = lsadc_get_chn_value(chn);
            lsadc_print("chn[%u] value:%d\n", chn, chn_value);
        }
    }

    /* do what you want to do in irq */
    return OSAL_IRQ_HANDLED;
}
#endif

static long hilsadc_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    int ret = -1;
    if (arg == 0) {
        return ret;
    }
    int param;
    hi_adc_unused(private_data);

    switch (cmd) {
        case LSADC_IOC_MODEL_SEL:
            param = *(int *)(uintptr_t)arg;
            ret = lsadc_model_select(param);
            break;

        case LSADC_IOC_CHN_ENABLE:
            param = *(int *)(uintptr_t)arg;
            ret = lsadc_chn_valid(param, 1);
            break;

        case LSADC_IOC_CHN_DISABLE:
            param = *(int *)(uintptr_t)arg;
            ret = lsadc_chn_valid(param, 0);
            break;

        case LSADC_IOC_START:
            ret = lsadc_start();
            break;

        case LSADC_IOC_STOP:
            ret = lsadc_stop();
            break;

        case LSADC_IOC_GET_CHNVAL:
            param = *(int *)(uintptr_t)arg;
            ret = lsadc_get_chn_value(param);
            break;

        default:
            lsadc_print("error cmd:%08x\n", cmd);
            ret = -1;
            break;
    }

    return ret;
}

static struct osal_fileops g_hi_lsadc_fops = {
    .open           = hilsadc_open,
    .release        = hilsadc_release,
    .unlocked_ioctl = hilsadc_ioctl,
};

static osal_dev_t *g_lsadc_device = NULL;

int lsadc_init(void)
{
    if (g_lsadc_reg_base == NULL) {
        g_lsadc_reg_base = (volatile void *)osal_ioremap(LSADC_BASE_ADDR, 0x100); /* 0x100: ADC reg length */
        if (g_lsadc_reg_base == NULL) {
            lsadc_print("lsadc ioremap error.\n");
            return -1;
        }
        g_need_iounmap = 1;
    }

#ifdef ENABLE_ADC_IRQ
    if (g_lsadc_irq <= 0) {
        g_lsadc_irq = LSADC_IRQ_ID;
    }

    if (osal_request_irq(g_lsadc_irq, lsadc_irq_proc, 0, "hi_lsadc", &g_hi_lsadc_fops) != 0) {
        lsadc_print("lsadc request irq error.\n");
        goto requeset_irq_fail;
    }
#endif

    osal_spin_lock_init(&g_lsadc_lock);

    g_lsadc_device = osal_createdev("hi_lsadc");
    g_lsadc_device->minor = 255; /* dev_minor 255 */
    g_lsadc_device->fops = &g_hi_lsadc_fops;
    if (osal_registerdevice(g_lsadc_device) != 0) {
        lsadc_print("lsadc register device error.\n");
        goto register_device_fail;
    }

    if (lsadc_enable_clock() != 0) {
        lsadc_print("lsadc enable clock error.\n");
        goto enable_clock_fail;
    }
    osal_printk("hi_lsadc init ok. ver=%s, %s.\n", __DATE__, __TIME__);

    return 0;

enable_clock_fail:
    osal_deregisterdevice(g_lsadc_device);
register_device_fail:
    osal_destroydev(g_lsadc_device);
    osal_spin_lock_destroy(&g_lsadc_lock);

#ifdef ENABLE_ADC_IRQ
    osal_free_irq(g_lsadc_irq, &g_hi_lsadc_fops);
requeset_irq_fail:
#endif

    if (g_need_iounmap) {
        osal_iounmap((void *)g_lsadc_reg_base, (unsigned long)LSADC_CLOCK_REG_LENGTH);
        g_need_iounmap = 0;
        g_lsadc_reg_base = NULL;
    }

    return -1;
}

void lsadc_exit(void)
{
#ifdef ENABLE_ADC_IRQ
    osal_free_irq(g_lsadc_irq, &g_hi_lsadc_fops);
#endif

    osal_spin_lock_destroy(&g_lsadc_lock);

    osal_deregisterdevice(g_lsadc_device);
    osal_destroydev(g_lsadc_device);
    if (g_need_iounmap) {
        osal_iounmap((void *)g_lsadc_reg_base, (unsigned long)LSADC_CLOCK_REG_LENGTH);
        g_need_iounmap = 0;
        g_lsadc_reg_base = NULL;
    }

    lsadc_disable_clock();
    osal_printk("hi_lsadc exit ok.\n");
}

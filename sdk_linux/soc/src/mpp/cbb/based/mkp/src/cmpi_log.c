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

#ifdef __HuaweiLite__
#include <pthread.h>
#include <shell.h>
#include <sys/prctl.h>
#include "linux/kernel.h"
#endif

#include "hi_osal.h"
#include "securec.h"

#include "hi_type.h"
#include "hi_debug.h"
#include "proc_ext.h"
#include "dev_ext.h"
#include "mod_ext.h"
#include "mkp_log.h"

static hi_s32 g_log_level[HI_ID_BUTT + 1] = {
    [0 ... HI_ID_BUTT - 1] = HI_DBG_ERR
};
#define LOG_STATE_INIT  0
#define LOG_STATE_OPEN  1
#define LOG_STATE_READ  2
#define LOG_STATE_WRITE 3
#define LOG_STATE_CLOSE 4

static hi_u32 g_state = LOG_STATE_INIT;
static hi_bool g_wait_data = HI_TRUE;

#define LOG_BUFFER_LEN  64 /* unit is KB. the ring buffer's default length is 64KB */
#define LOG_MAX_ITEMLEN 512 /* unit is byte. suppose one log is short than 512 byte */

typedef struct {
    hi_u32 max_len; /* the max length of ring buffer    */
    hi_char *addr; /* the address of ring buffer       */
    hi_u32 read_pos; /* the read pointer of ring buffer  */
    hi_u32 write_pos; /* the write pointer of ring buffer */
    hi_u32 butt_pos; /* leave a blank for circle round   */
    osal_spinlock_t lock; /* the lock                         */
    osal_wait_t block_wait; /* wait for read                    */
} log_buffer;

/* All the mode use the same one buffer. so the time order can be record. */
static log_buffer g_log_buf = {0};

static int search_mod(const char *s)
{
    int i;
    for (i = 0; i < HI_ID_BUTT; i++) {
        hi_char *pa_name = cmpi_get_module_name(i);
        if (pa_name && !osal_strcmp(pa_name, s)) {
            return i;
        }
    }
    return -1;
}

/* ate all blanks in a line */
static char *strip_string(char *s, char *d, int len)
{
    int i = 0;
    char *p = d;
    do {
        if (*s == '\n') {
            *s = '\0';
        }
        if (*s != ' ') {
            *p++ = *s;
        }
        i++;
    } while ((*s++ != '\0') && (i < len));
    return d;
}

static int separate_string(char *s, char **left, char **right)
{
    char *p = s;
    /* find '=' */
    while (*p != '\0' && *p++ != '=') {
    };

    if (*--p != '=') {
        return -1;
    }

    /* separate left from right value by '=' */
    *p = '\0';
    *left = s;
    *right = p + 1;
    return 0;
}

hi_s32 hi_chk_log_level(hi_s32 level, hi_mod_id mod_id)
{
    if ((mod_id >= HI_ID_BUTT) ||
        (level > g_log_level[mod_id])) {
            return 0;
        }
    return 1;
}

static inline hi_u64 log_get_time(hi_void)
{
    osal_timeval_t time;
    osal_gettimeofday(&time);
    return time.tv_sec * 1000000LLU + time.tv_usec;
}

hi_s32 log_write(hi_s32 level, hi_mod_id mod_id, const char *fmt, osal_va_list args)
{
    hi_s32 new_len;
    hi_s32 tmp_len;
    hi_u32 old_wp, old_rp, new_wp, new_rp;
    hi_char *addr = NULL;
    unsigned long lock_flag;
    hi_char *mod_name = NULL;

    if (g_log_buf.addr == NULL) {
        return 0;
    }
    HI_ASSERT(g_log_buf.write_pos <= g_log_buf.max_len);
    HI_ASSERT(g_log_buf.read_pos <= g_log_buf.max_len);

    osal_spin_lock_irqsave(&(g_log_buf.lock), &lock_flag);

    old_wp = new_wp = g_log_buf.write_pos;
    old_rp = new_rp = g_log_buf.read_pos;
    mod_name = cmpi_get_module_name(mod_id);

    if (g_log_buf.max_len - old_wp > LOG_MAX_ITEMLEN) {
        addr = g_log_buf.addr + old_wp;
        new_len = snprintf_s(addr, LOG_MAX_ITEMLEN, LOG_MAX_ITEMLEN - 1, "<%d>[%6s] ", level, mod_name);
        new_len = (new_len < 0) ? 0 : new_len;
        tmp_len = vsnprintf_s(addr + new_len, LOG_MAX_ITEMLEN - new_len, LOG_MAX_ITEMLEN - new_len - 1, fmt, args);
        tmp_len = (tmp_len < 0) ? 0 : tmp_len;
        new_len = new_len + tmp_len;

        if (new_len >= LOG_MAX_ITEMLEN) {
            osal_printk("[%6s]: log message is too long!!\n", mod_name);
        } else {
            new_wp = old_wp + new_len;
            if ((new_wp >= old_rp) && (old_wp < old_rp)) {
                new_rp = new_wp + 1;
            }
        }
    } else {
        g_log_buf.butt_pos = old_wp;
        addr = g_log_buf.addr;
        new_len = snprintf_s(addr, LOG_MAX_ITEMLEN, LOG_MAX_ITEMLEN - 1, "<%d>[%6s] ", level, mod_name);
        new_len = (new_len < 0) ? 0 : new_len;
        tmp_len = vsnprintf_s(addr + new_len, LOG_MAX_ITEMLEN - new_len, LOG_MAX_ITEMLEN - new_len - 1, fmt, args);
        tmp_len = (tmp_len < 0) ? 0 : tmp_len;
        new_len = new_len + tmp_len;

        if (new_len >= LOG_MAX_ITEMLEN) {
            osal_printk("[%6s]: log message is too long!!\n", mod_name);
        } else {
            new_wp = new_len;
            if (!(((hi_u32)new_len < old_rp) && (old_wp >= old_rp))) {
                new_rp = new_len + 1;
            }
        }
    }

    g_log_buf.write_pos = new_wp;
    g_log_buf.read_pos = new_rp;

    osal_spin_unlock_irqrestore(&(g_log_buf.lock), &lock_flag);

    osal_wakeup(&(g_log_buf.block_wait));

    return new_len;
}

int HI_LOG(hi_s32 level, hi_mod_id mod_id, const char *fmt, ...)
{
    osal_va_list args;
    int r;
    if (!hi_chk_log_level(level, mod_id)) {
        return 0;
    }

    osal_va_start(args, fmt);
    r = log_write(level, mod_id, fmt, args);
    osal_va_end(args);
    return r;
}

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
/* use the "cat /proc/umap/log"  to get the debug level */
static hi_s32 log_proc_read(struct osal_proc_dir_entry *s)
{
    hi_u32 i;

    osal_seq_printf(s, "-----LOG BUFFER STATE----------------------------------------------------------\n");
    osal_seq_printf(s, "MaxLen  ReadPos WritePos ButtPos\n");
    osal_seq_printf(s, "%3d(KB) %7d %8d %7d\n",
                    g_log_buf.max_len >> 10, g_log_buf.read_pos, /* 10,width */
                    g_log_buf.write_pos, g_log_buf.butt_pos);
    osal_seq_printf(s, "\n");

    osal_seq_printf(s, "-----CURRENT LOG LEVEL---------------------------------------------------------\n");
    for (i = 0; i < HI_ID_BUTT; i++) {
        hi_char *pa_temp_name = cmpi_get_module_name(i);
        if (pa_temp_name == NULL) {
            continue;
        }
        osal_seq_printf(s, "%s\t:  %d\n",
                        pa_temp_name, g_log_level[i]);
    }

    return 0;
}

/* use the "echo 'mode = level' > /proc/umap/log"  to modify the debug level */
static hi_s32 log_proc_write(struct osal_proc_dir_entry *s, const char *buf,
    int count, long long *ppos)
{
#define TMP_BUF_LEN     32
    char m[TMP_BUF_LEN] = {0};
    char d[TMP_BUF_LEN] = { [0 ... TMP_BUF_LEN - 1] = '\0' };
    int len = TMP_BUF_LEN;
    char *left = NULL;
    char *right = NULL;
    int idx;
    int level;

    hi_unused(s);
    if (*ppos >= TMP_BUF_LEN) {
        return HI_FAILURE;
    }

    if (count <= 0) {
        return HI_FAILURE;
    }

    len = osal_min(len, count);
    if (osal_copy_from_user(m, buf, len) != 0) {
        return HI_FAILURE;
    }

    m[len - 1] = '\0';
    strip_string(m, d, len);
    if (separate_string(d, &left, &right)) {
        osal_printk("string is unknown!\n");
        goto out;
    }

    level = osal_strtol(right, NULL, 10);  /* 10,base */
    if (!osal_strcmp("wait", left)) {
        g_wait_data = level;
        goto out;
    }

    if (!level && *right != '0') {
        osal_printk("invalid value!\n");
        goto out;
    }

    if (level > HI_DBG_DEBUG) {
        osal_printk("level: %d illegal!\n", level);
    }

    if (!osal_strcmp("all", left)) {
        int i = 0;
        for (; i < HI_ID_BUTT; i++) {
            g_log_level[i] = level;
        }
        goto out;
    }
    idx = search_mod(left);
    if ((idx < 0) || (idx >= HI_ID_BUTT)) {
        osal_printk("%s not found in array!\n", left);
        return count;
    }
    g_log_level[idx] = level;

out:
    return len;
#undef TMP_BUF_LEN
}
#endif

static hi_s32 log_close(void *private_data);
/* IOCTL operation for get and config the debug level */
static hi_s32 do_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    hi_unused(private_data);
    switch (cmd) {
        case LOG_SETLOGLEVEL_CTRL: {
            hi_log_level_conf *conf = (hi_log_level_conf *)(hi_uintptr_t)arg;

            if (conf->level > HI_DBG_DEBUG) {
                HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "illegal level: %d\n", conf->level);
                return HI_FAILURE;
            }

            conf->mod_name[sizeof(conf->mod_name) - 1] = '\0';

            if (!osal_strcmp("all", conf->mod_name)) {
                int i = 0;
                for (; i < HI_ID_BUTT; i++) {
                    g_log_level[i] = conf->level;
                }
            } else {
                if (conf->mod_id >= HI_ID_BUTT) {
                    HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "%s, line: %d, illegal parma, modeid: %d\n",
                        __FUNCTION__, __LINE__, conf->mod_id);
                    return HI_FAILURE;
                }

                g_log_level[conf->mod_id] = conf->level;
            }
            break;
        }
        case LOG_GETLOGLEVEL_CTRL: {
            errno_t err_value;
            hi_log_level_conf *conf = (hi_log_level_conf *)(hi_uintptr_t)arg;

            if (conf->mod_id >= HI_ID_BUTT) {
                HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "%s, line: %d, illegal parma, modeid: %d\n",
                    __FUNCTION__, __LINE__, conf->mod_id);
                return HI_FAILURE;
            }

            conf->level = g_log_level[conf->mod_id];

            if (cmpi_get_module_name(conf->mod_id) == NULL) {
                return HI_FAILURE;
            }

            err_value = strncpy_s(conf->mod_name, sizeof(conf->mod_name),  cmpi_get_module_name(conf->mod_id),
                sizeof(conf->mod_name) - 1);
            if (err_value != EOK) {
                HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "copy err!\n");
                return HI_FAILURE;
            }

            break;
        }
        case LOG_CLOSE_CTRL: {
            log_close(HI_NULL);
            break;
        }
        case LOG_SETWAITFLAG_CTRL: {
            hi_bool wait;

            wait = *((hi_bool *)(hi_uintptr_t)arg);

            if ((wait != HI_FALSE) && (wait != HI_TRUE)) {
                HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "%s, line: %d, illegal parma, wait: %d\n",
                    __FUNCTION__, __LINE__, wait);
                return HI_FAILURE;
            }

            g_wait_data = wait;
            break;
        }
        default:
        {
            break;
        }
    }

    return HI_SUCCESS;
}

static long log_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    return do_ioctl(cmd, arg, private_data);
}

#ifdef __HuaweiLite__
hi_void *read_q(hi_void *data)
{
    int c;

    hi_unused(data);
    prctl(PR_SET_NAME, "hi_log_read_q", 0, 0, 0);
    while (1) {
        c = getchar();
        if ((c == 'q') || (c == 'Q')) {
            log_close(HI_NULL);
            return 0;
        }
    }
}
#endif

int wait_condition_call_back(const void *param)
{
    hi_unused(param);
    return ((g_log_buf.read_pos != g_log_buf.write_pos) ||
            (g_state == LOG_STATE_CLOSE) ||
            (g_wait_data == HI_FALSE));
}

/* read operation for log information */
static int log_read(char *buf, int count, long *offset, void *private_data)
{
    hi_s32 read_len = 0;
    hi_s32 error;
    hi_char c;
    unsigned long lock_flag;

    hi_unused(private_data);
    hi_unused(offset);
#ifdef __HuaweiLite__

    hi_unused(buf);
    count = LOG_BUFFER_LEN * 1024;   /* 1024, 1 k bit */
    pthread_t threadid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&threadid, &attr, read_q, NULL) != 0) {
        HI_PRINT("cmpi_log read pthread_create error!\n");
        return -1;
    }
#endif

    g_state = LOG_STATE_READ;

    error = osal_wait_event_interruptible(&g_log_buf.block_wait, wait_condition_call_back, NULL);
    if (error) {
        return error;
    }

    osal_spin_lock_irqsave(&(g_log_buf.lock), &lock_flag);
    while ((!error) && (read_len < count) &&
           (g_log_buf.write_pos != g_log_buf.read_pos)) {
            if (g_log_buf.read_pos > g_log_buf.butt_pos) {
                g_log_buf.read_pos = 0;
            }

            if (g_log_buf.write_pos == g_log_buf.read_pos) {
                break;
            }

            c = *(g_log_buf.addr + g_log_buf.read_pos);
            g_log_buf.read_pos += 1;

            osal_spin_unlock_irqrestore(&(g_log_buf.lock), &lock_flag);

#ifdef __HuaweiLite__
            printk("%c", c);
#else
            error = osal_copy_to_user(buf, &c, 1);
            buf++;
#endif

            read_len++;
            osal_yield();
            osal_spin_lock_irqsave(&(g_log_buf.lock), &lock_flag);
        }
    osal_spin_unlock_irqrestore(&(g_log_buf.lock), &lock_flag);

    return read_len;
}

#ifdef __HuaweiLite__
/*
    This function uses Camel_Style ,not Linux_Style.
    Because it is exported to the sample/app_init.c
*/
void CatLogShell(void)
{
    osCmdReg(CMD_TYPE_EX, "cat_logmpp", 0, (CMD_CBK_FUNC)log_read);
}
#endif

static hi_s32 log_open(void *data)
{
    hi_unused(data);
    g_state = LOG_STATE_OPEN;
    return HI_SUCCESS;
}

static hi_s32 log_close(void *data)
{
    hi_unused(data);
    g_state = LOG_STATE_CLOSE;
    g_wait_data = HI_TRUE;
    osal_wakeup(&(g_log_buf.block_wait));
    return HI_SUCCESS;
}

static struct osal_fileops g_log_file_op = {
    .open = log_open,
    .unlocked_ioctl = log_ioctl,
    .release = log_close,
    .read = log_read
};

static osal_dev_t *s_log_device;

hi_s32 cmpi_log_init(hi_u32 log_buf_len)
{
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_proc_entry_t *item = NULL;
#endif
    s_log_device = osal_createdev(UMAP_DEVNAME_LOG_BASE);
    if (s_log_device == NULL) {
        osal_printk("log createdev failed!\n");
        goto OUT0;
    }
    s_log_device->fops = &g_log_file_op;
    s_log_device->minor = UMAP_LOG_MINOR_BASE;

    if (osal_registerdevice(s_log_device)) {
        osal_printk("log register device failed!\n");
        goto OUT1;
    }

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    item = osal_create_proc_entry(PROC_ENTRY_LOG, NULL);
    if (item == NULL) {
        goto OUT2;
    }
    item->read = log_proc_read;
    item->write = log_proc_write;
#endif
    if (osal_spin_lock_init(&(g_log_buf.lock)) < 0) {
        osal_printk("spinlock init failed!\n");
        goto OUT3;
    }

    if (osal_wait_init(&(g_log_buf.block_wait)) < 0) {
        osal_printk("wait init failed!\n");
        goto OUT4;
    }

    if ((log_buf_len >= 2) && (log_buf_len <= 128)) { /* 2,min len. 128 max len */
        g_log_buf.max_len = log_buf_len * 1024; /* 1024,1kB is 1024 byte */
    } else {
        g_log_buf.max_len = LOG_BUFFER_LEN * 1024; /* 1024,1kB is 1024 byte */
    }
    g_log_buf.butt_pos = g_log_buf.max_len;
    g_log_buf.read_pos = g_log_buf.write_pos = 0;
    g_log_buf.addr = (hi_char *)osal_vmalloc(g_log_buf.max_len);
    if (g_log_buf.addr == NULL) {
        osal_printk("log buffer %d_b malloc failed.\n", g_log_buf.max_len);
        goto OUT5;
    }

    return HI_SUCCESS;

OUT5:
    osal_wait_destroy(&(g_log_buf.block_wait));
OUT4:
    osal_spin_lock_destroy(&(g_log_buf.lock));
OUT3:
    osal_remove_proc_entry(PROC_ENTRY_LOG, NULL);
OUT2:
    osal_deregisterdevice(s_log_device);
OUT1:
    osal_destroydev(s_log_device);
    s_log_device = HI_NULL;
OUT0:
    return HI_FAILURE;
}

hi_void cmpi_log_exit(hi_void)
{
    if (g_log_buf.addr != NULL) {
        osal_vfree(g_log_buf.addr);
        g_log_buf.addr = NULL;
    }
    osal_deregisterdevice(s_log_device);
    osal_destroydev(s_log_device);
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_LOG, NULL);
#endif

    osal_spin_lock_destroy(&(g_log_buf.lock));
    osal_wait_destroy(&(g_log_buf.block_wait));
}

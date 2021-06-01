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
#include "drv_hdmi_debug.h"
#ifndef HDMI_LITEOS_SUPPORT
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/major.h>
#include <asm/uaccess.h>
#endif
#include "drv_hdmi_common.h"
#include "drv_hdmi_intf.h"
#include "drv_hdmi_event.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_common.h"
#include "drv_hdmi_frl.h"
#include "hdmi_osal.h"
#include "drv_hdmi_compatibility.h"
#ifdef HDMI_FRL_SUPPORT
#include "hdmi_hal_timer.h"
#endif

#define DEBUG_MAX_CMD_CHAR_NUM  128
#define DEBUG_MAX_ARGV_NUM      10
#define DEBUG_CHAR_SPACE        0x20
#define DEBUG_CHAR_TAB          0x09
#define DEBUG_CHAR_END          0x0a
#define DEBUG_MAX_EDIDBUF_SIZE  1024
#define DEBUG_PROC_BUFFER_SIZE  1024
#define SERIAL_MAJOR            204
#define DEBUG_FRL_CTRL_TYPE_MAX 0x3
#define STR_SUCC                "success"
#define STR_FAIL                "fail"
#define DEBUG_CMD_ARGC_COUNT_1  2
#define DEBUG_CMD_ARGC_COUNT_2  3
#define DEBUG_CMD_ARGC_COUNT_3  4
#define DEBUG_MAX_DATA_SIZE     5000
#define DEBUG_TIME_TEST_MAX     10
#define SCDC_SET_WAIT           90
#define SCDC_GET_WAIT           3000
#define FRL_CRC_WAIT            1000
#define FRL_WAIT                50
#define DEBUG_MAX_FRL_TIME      5000
#define DEBUG_NUM_2             2
#define DEBUG_NUM_3             3
#define DEBUG_NUM_5             5
#define DEBUG_NUM_8             8
#define DEBUG_THOUSAND          1000
#define DEBUG_RC_MAX            4

#ifndef tolower
#define tolower(x) ((x) | 0x20)
#endif

#define hdmi_dbg_err(param...)                                                    \
    do {                                                                          \
        hdmi_dbg_print("[HDMI DBG ERROR]: %s [%u],", __func__, (hi_u32)__LINE__); \
        hdmi_dbg_print(param);                                                    \
    } while (0)

#define debug_null_check_return(p)                  \
    do {                                            \
        if ((p) == HI_NULL) {                       \
            hdmi_warn("%s null pointer!\n", #p);    \
            hdmi_dbg_err("%s null pointer!\n", #p); \
            return HI_FAILURE;                      \
        }                                           \
    } while (0)

#define debug_failure_check_return(ret)        \
    do {                                       \
        if ((ret) != HI_SUCCESS) {             \
            hdmi_warn("return failure!\n");    \
            hdmi_dbg_err("return failure!\n"); \
            return HI_FAILURE;                 \
        }                                      \
    } while (0)

typedef struct {
    hi_bool data_valid;
    hi_s32 len;
    hi_u8 data[HDMI_EDID_TOTAL_SIZE];
} hdmi_debug_edid;

static hdmi_debug_edid g_debug_edid = {0};

static const char *g_event_str_table[] = {
    "HPD",                  // HDMI_EVENT_HOTPLUG  0x10
    "HOTUNPLUG",            // HDMI_EVENT_HOTUNPLUG
    "EDID_FAIL",            // HDMI_EVENT_EDID_FAIL
    "HDCP_FAIL",            // HDMI_EVENT_HDCP_FAIL
    "HDCP_SUCCESS",         // HDMI_EVENT_HDCP_SUCCESS
    "RSEN_CONNECT",         // HDMI_EVENT_RSEN_CONNECT
    "RSEN_DISCONNECT",      // HDMI_EVENT_RSEN_DISCONNECT
    "HDCP_USERSETTING",     // HDMI_EVENT_HDCP_USERSETTING
    "HDCP_OFF",             // HDMI_EVENT_HDCP_OFF
    "SCRAMBLE_FAIL",        // HDMI_EVENT_SCRAMBLE_FAIL
    "SCRAMBLE_SUCCESS",     // HDMI_EVENT_SCRAMBLE_SUCCESS
    "ZERO_DRM_TIMEOUT",     // HDMI_EVENT_ZERO_DRMIF_TIMEOUT
    "HDR_SWITCH_TIMEOUT"    // HDMI_EVENT_SWITCH_TO_HDRMODE_TIMEOUT
};

typedef struct {
    hi_u32 ddc_reg_cfg;
    hi_u32 approximate_value;
    hi_char *read_value;
} hdmi_ddc_freq;

hdmi_ddc_freq g_ddc_freq[] = {
    { 0x45, 20,  " 19.89" },
    { 0x29, 30,  " 29.97" },
    { 0x21, 40,  " 40.06" },
    { 0x1a, 50,  " 49.90" },
    { 0x15, 60,  " 60.53" },
    { 0x12, 70,  " 69.42" },
    { 0x0f, 80,  " 81.38" },
    { 0x0d, 90,  " 91.87" },
    { 0x0c, 100, " 98.11" },
    { 0x0a, 110, "113.78" },
    { 0x07, 150, "149.85" },
    { 0x04, 220, "219.43" }
};

#ifdef HDMI_FRL_SUPPORT
static hi_u32 g_timer_test;
#endif

typedef enum {
    HDMI_DEBUG_BASE_OSD = 8,
    HDMI_DEBUG_BASE_DEC = 10,
    HDMI_DEBUG_BASE_HEX = 16
} hdmi_debug_base;

typedef struct {
    hdmi_device_id hdmi_id;
    hi_char *argv[DEBUG_MAX_ARGV_NUM];
    hi_u32 argc;
    hi_u32 remain_len;
} hdmi_debug_cmd_arg;

typedef hi_s32 (*cmd_func)(hdmi_debug_cmd_arg *cmd_arg);

typedef struct {
    hi_char *name;
    hi_char *short_name;
    cmd_func fn_cmd_func;
    hi_char *comment_help;
} hdmi_debug_cmd_info;

static hi_s32 debug_print_help(hdmi_debug_cmd_arg *cmd_arg);

static hi_void debug_echo_helper_vargs(hi_char *buf, hi_u32 size, const hi_char *fmt, osal_va_list args)
{
#define DEFAULT_ECHO_DEVICE_HANDLE 0

    hi_s32 ret;
    struct kstat stat = {0};

    if (size == 0) {
        return;
    }

    ret = vfs_fstat(DEFAULT_ECHO_DEVICE_HANDLE, &stat);
    if (ret) {
        hdmi_err("default echo device handle(%u) invalid!\n", DEFAULT_ECHO_DEVICE_HANDLE);
        return;
    }

    ret = vsnprintf_s(buf, size, size - 1, fmt, args);
    if (ret < 0) {
        hdmi_err("vsnprintf_s err\n");
        return;
    }

    /* echo device must be chrdev and major number must be SERIAL_MAJOR or TTYAUX_MAJOR or UNIX98_PTY_SLAVE_MAJOR */
    if (S_ISCHR(stat.mode) &&
        (MAJOR(stat.rdev) == SERIAL_MAJOR || MAJOR(stat.rdev) == TTYAUX_MAJOR ||
        MAJOR(stat.rdev) == UNIX98_PTY_SLAVE_MAJOR)) {
        struct file *f = fget(DEFAULT_ECHO_DEVICE_HANDLE);
        if (f != HI_NULL) {
            /* file pos is invalid for chrdev */
            hdmi_osal_file_write(f, buf, osal_strlen(buf));
            fput(f);
        }
    } else {
        hdmi_err("default echo device is invalid!\n");
    }
}

static hi_void hdmi_dbg_print(const hi_char *fmt, ...)
{
    osal_va_list args;
    hi_char *buf = HI_NULL;

    if (fmt == HI_NULL) {
        hdmi_err("invalid argument fmt!\n");
        return;
    }
    buf = osal_vmalloc(DEBUG_PROC_BUFFER_SIZE);
    if (buf == HI_NULL) {
        hdmi_err("memory allocate failed for proc\n");
        return;
    }
    (hi_void)memset_s(buf, DEBUG_PROC_BUFFER_SIZE, 0, DEBUG_PROC_BUFFER_SIZE);

    osal_va_start(args, fmt);
    debug_echo_helper_vargs(buf, DEBUG_PROC_BUFFER_SIZE, fmt, args);
    osal_va_end(args);

    osal_vfree(buf);
    buf = HI_NULL;
}

static hi_bool is_hex_space(hi_char a)
{
    hi_bool is_space = HI_FALSE;
    is_space = (a == DEBUG_CHAR_SPACE || a == DEBUG_CHAR_TAB ||
                a == ',' || a == ';' || a == '\0' || a == '\n' || a == '\r');
    return is_space;
}

static hi_bool is_hex_char(hi_char a)
{
    hi_bool is_char = HI_FALSE;
    is_char = ((a >= '0' && a <= '9') || (a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F'));
    return is_char;
}

static hi_u8 hex_value_get(hi_char a)
{
    if ((a >= '0' && a <= '9')) {
        return a - '0';
    } else if (a >= 'a' && a <= 'f') {
        return a - 'a' + 0xa;
    } else if (a >= 'A' && a <= 'F') {
        return a - 'A' + 0xa;
    } else {
        return 0xff;
    }
}

static hi_bool is_hex_start(hi_char *p, hi_char **end)
{
    debug_null_check_return(p);

    if (*p == '0' && (p[1] == 'x' || p[1] == 'X')) {
        if (end != HI_NULL) {
            *end = p + 2; /* P + 2, mean 16 hex value addr */
        }

        return HI_TRUE;
    } else if (is_hex_char(p[0]) && (is_hex_space(p[1]) || (is_hex_char(p[1]) && is_hex_space(p[2])))) {
        if (end != HI_NULL) {
            *end = p;
        }
        return HI_TRUE;
    } else {
        if (end != HI_NULL) {
            *end = p;
        }
        return HI_FALSE;
    }
}

static hi_s32 debug_str_to_hex(hi_char *dest, hi_s32 dlen, hi_char *src, hi_u32 slen)
{
    hi_u8 result;
    hi_char *p = src;
    hi_s32 old_len = dlen;

    while (p < (src + slen) && dlen) {
        if (is_hex_start(p, &p) == HI_FALSE) {
            p++;
            continue;
        }
        if (is_hex_char(p[0]) == HI_FALSE) {
            return HI_FAILURE;
        }
        result = hex_value_get(p[0]);
        p++;
        if (is_hex_char(p[0])) {
            result <<= 4; /* 4, means get the high 8 bit data of 16 bit data */
            result |= hex_value_get(p[0]);
            p++;
        }

        if (dest != HI_NULL) {
            *dest++ = result;
            dlen--;
        }
    }

    return old_len - dlen;
}

static hdmi_debug_base debug_base_get(const hi_char *str)
{
    if (str[0] == '0') {
        if (tolower((hi_u32)str[1]) == 'x') {
            return HDMI_DEBUG_BASE_HEX;
        }
        return HDMI_DEBUG_BASE_OSD;
    }
    return HDMI_DEBUG_BASE_DEC;
}

static hi_u32 debug_str_to_u32(const hi_char *str, hi_char **end, hdmi_debug_base base)
{
    hi_u32 result = 0;

    debug_null_check_return(str);

    if (base != HDMI_DEBUG_BASE_OSD && base != HDMI_DEBUG_BASE_DEC && base != HDMI_DEBUG_BASE_HEX) {
        base = debug_base_get(str);
    }

    if (base == HDMI_DEBUG_BASE_HEX && str[0] == '0' && tolower((hi_u8)str[1]) == 'x') {
        str += 2; /* str + 2, mean 16 hex value addr */
    }

    while (*str) {
        hi_u32 value;

        value = ((*str >= '0') && (*str <= '9')) ?
            *str - '0' : tolower((hi_u8)(*str)) - 'a' + 10; /* value size of 10 - 15 */
        if (value >= base) {
            break;
        }
        result = result * base + value;
        str++;
    }
    if (end != HI_NULL) {
        *end = (hi_char *)str;
    }

    return result;
}

static struct file *debug_file_open(const char *filename, int flags, int mode)
{
    return osal_klib_fopen(filename, flags, mode);
}

static void debug_file_close(struct file *filp)
{
    if (filp != HI_NULL) {
        osal_klib_fclose(filp);
    }
}

static int debug_file_read(char *buf, unsigned int len, struct file *filp)
{
    int readlen;
    /* use osal public interface. */
    readlen = osal_klib_fread(buf, len, filp);
    return readlen;
}

static hi_s32 debug_thread(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32 ret = HI_FAILURE;
    hdmi_device *hdmi_dev = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        ret = get_hdmi_thread_state_set(hdmi_dev, HDMI_THREAD_STATE_STOP);
        hdmi_dbg_print("\n thread set OFF %s! \n\n", ret != HI_SUCCESS ? STR_FAIL : STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        ret = get_hdmi_thread_state_set(hdmi_dev, HDMI_THREAD_STATE_RUN);
        hdmi_dbg_print("\n thread set ON %s! \n\n", ret != HI_SUCCESS ? STR_FAIL : STR_SUCC);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    return ret;

usage_error:
    hdmi_dbg_print("[usage  ]: echo thread argv1 > /proc/umap/hdmi0\n");
    hdmi_dbg_print("[argv1  ]: 0\t--thread off.\n"
                   "           1\t--thread on.\n");
    hdmi_dbg_print("[example]: echo thread 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_oe(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_black_frame_info black_info = {0};

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    black_info.in_bit_depth = hdmi_dev->attr.vo_attr.in_bit_depth;
    black_info.in_color_space = hdmi_dev->attr.vo_attr.in_color_space;
    black_info.in_quantization = (black_info.in_color_space == HDMI_COLORSPACE_RGB) ?
        hdmi_dev->attr.vo_attr.rgb_quantization : hdmi_dev->attr.vo_attr.ycc_quantization;

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_FALSE);
        hdmi_dbg_print("\n oe set OFF %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_TRUE);
        hdmi_dbg_print("\n oe set ON %s! \n\n", STR_SUCC);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo oe argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--oe off.\n"
                   "           1\t--oe on.\n");
    hdmi_dbg_print("[example]: echo oe 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_event(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32      ret = HI_FAILURE;
    hdmi_event  event;
    hdmi_device *hdmi_dev = HI_NULL;
    hi_char     *str      = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    str = cmd_arg->argv[1];

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || str == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    event = (hdmi_event)debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if ((event >= HDMI_EVENT_HOTPLUG) &&
        (event < HDMI_EVENT_BUTT)) {
        ret = drv_hdmi_event_pool_write(cmd_arg->hdmi_id, event);
        if ((event - HDMI_EVENT_HOTPLUG) < hdmi_array_size(g_event_str_table)) {
            hdmi_dbg_print("hardware event %s simulate %s.\n",
                g_event_str_table[event - HDMI_EVENT_HOTPLUG], ret != HI_SUCCESS ? STR_FAIL : STR_SUCC);
        }
    } else {
        goto usage_error;
    }

    return ret;

usage_error:
    hdmi_dbg_print("[usage  ]: echo event argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: event type(DECMICAL) as follow\n");
    for (event = HDMI_EVENT_HOTPLUG;
         event < HDMI_EVENT_BUTT && ((event - HDMI_EVENT_HOTPLUG) < hdmi_array_size(g_event_str_table));
         event++) {
        hdmi_dbg_print("%-10s %u\t-- (0x%02x)%s\n", "", event, event,
            g_event_str_table[event - HDMI_EVENT_HOTPLUG]);
    }
    hdmi_dbg_print("[example]: echo event 16 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_edid_from_file(hdmi_device *hdmi_dev, hdmi_debug_cmd_arg *cmd_arg)
{
    struct file *f;
    hi_u8 *buf  = HI_NULL;
    hi_s32 len  = 0;
    hdmi_debug_edid *dbg_edid = &g_debug_edid;

    f = debug_file_open(cmd_arg->argv[2], OSAL_O_RDONLY, 0);
    if (f == HI_NULL) {
        hdmi_dbg_err("open file %s fail!\n", cmd_arg->argv[2]);
        return HI_FAILURE;
    }

    buf = (hi_u8 *)osal_vmalloc(DEBUG_MAX_EDIDBUF_SIZE);
    if (buf != HI_NULL) {
        (hi_void)memset_s(buf, DEBUG_MAX_EDIDBUF_SIZE, 0, DEBUG_MAX_EDIDBUF_SIZE);
        (hi_void)memset_s(dbg_edid, sizeof(hdmi_debug_edid), 0, sizeof(hdmi_debug_edid));
        len = debug_file_read(buf, DEBUG_MAX_EDIDBUF_SIZE, f);
        if (len <= 0) {
            hdmi_dbg_print("read file size = %d!\n", len);
        } else {
            len = debug_str_to_hex(dbg_edid->data, HDMI_EDID_TOTAL_SIZE, buf, (hi_u32)len);
            if (len < 0) {
                hdmi_dbg_err("string convert to hex error!\n");
            } else {
                dbg_edid->data_valid = HI_TRUE;
                dbg_edid->len += len;
                drv_hdmi_edid_update(&hdmi_dev->edid_info, HDMI_EDID_UPDATE_DEBUG);
            }
        }
        osal_vfree(buf);
        buf = HI_NULL;
    } else {
        hdmi_dbg_err("vmalloc error!\n");
    }
    debug_file_close(f);

    if (len < 0) {
        hdmi_dbg_err("error hex raw data format!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void print_raw_edid(hdmi_edid_info *edid_info)
{
    hi_s32 i;
    hi_s32 ret;
    hi_u8 *buf = HI_NULL;

    buf = (hi_u8 *)osal_vmalloc(HDMI_EDID_TOTAL_SIZE);
    if (buf != HI_NULL) {
        (hi_void)memset_s(buf, HDMI_EDID_TOTAL_SIZE, 0, HDMI_EDID_TOTAL_SIZE);
        ret = drv_hdmi_edid_raw_get(edid_info, buf, HDMI_EDID_TOTAL_SIZE);
        for (i = 0; (i < ret) && (i < HDMI_EDID_TOTAL_SIZE); i++) {
            if (!(i % 16)) { /* each Line length of 16 characters  */
                hdmi_dbg_print("\n");
            }
            hdmi_dbg_print(" %02x", buf[i]);
        }

        hdmi_dbg_print("\n\n");
        osal_vfree(buf);
    } else {
        hdmi_dbg_err("vmalloc fail!\n");
    }

    return;
}

static hi_s32 debug_edid(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32           ret      = HI_FAILURE;
    hdmi_device     *hdmi_dev = HI_NULL;
    hdmi_debug_edid *dbg_edid = &g_debug_edid;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_1) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    debug_null_check_return(cmd_arg->argv[1]);

    if (is_str_equal(cmd_arg->argv[1], "s") ||
        is_str_equal(cmd_arg->argv[1], "0")) {
        (hi_void)memset_s(dbg_edid, sizeof(hdmi_debug_edid), 0, sizeof(hdmi_debug_edid));
        ret = drv_hdmi_edid_update(&hdmi_dev->edid_info, HDMI_EDID_UPDATE_SINK);
        if (ret != HI_SUCCESS) {
            hdmi_dbg_err("update edid from sink fail!make sure cable connect!\n");
        }
    } else if (is_str_equal(cmd_arg->argv[1], "f") ||
               is_str_equal(cmd_arg->argv[1], "1")) {
        if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2) {
            hdmi_dbg_err("error argcs!\n");
            goto usage_error;
        }
        debug_null_check_return(cmd_arg->argv[2]);
        ret = debug_edid_from_file(hdmi_dev, cmd_arg);
        debug_failure_check_return(ret);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    hdmi_dbg_print(" edid load %s! \n", ret < 0 ? STR_FAIL : STR_SUCC);
    hdmi_dbg_print("[edid data]:");
    print_raw_edid(&hdmi_dev->edid_info);

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo edid argv1 argv2 > /proc/umap/hdmi0\n");
    hdmi_dbg_print("[argv1  ]: edid operation mode as below\n");
    hdmi_dbg_print("%10s 0(s)  --finish using debug edid and read from sink.\n", "");
    hdmi_dbg_print("%10s 1(f)  --read debug edid from argv2(a hex-string-file-path).\n", "");
    hdmi_dbg_print("[argv2  ]: you have to use at least 1 character of 'SPACE|TAB|,|0x' between 2 hex-bytes.\n");
    hdmi_dbg_print("[example]: echo edid s > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[example]: echo edid f /mnt/edid_skyworth_300m.txt > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_avmute(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device *hdmi_dev = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        drv_hdmi_avmute_set(hdmi_dev, HI_FALSE);
        hdmi_dbg_print("\n avmute set OFF %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        drv_hdmi_avmute_set(hdmi_dev, HI_TRUE);
        hdmi_dbg_print("\n avmute set ON %s! \n\n", STR_SUCC);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo avmute argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--avmute off.\n"
                   "           1\t--avmute on.\n");
    hdmi_dbg_print("[example]: echo avmute 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_cbar(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device *hdmi_dev = HI_NULL;
    hi_bool enable = HI_FALSE;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        enable = HI_FALSE;
        hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_COLOR_BAR, &enable);
        hdmi_dbg_print("\n color_bar is OFF %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        enable = HI_TRUE;
        hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_COLOR_BAR, &enable);
        hdmi_dbg_print("\n color_bar is ON %s! \n\n", STR_SUCC);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo cbar argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--colorbar off.\n"
                   "           1\t--colorbar on.\n");
    hdmi_dbg_print("[example]: echo cbar 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

#ifdef HDMI_SCDC_SUPPORT
static hi_s32 debug_scdc_off(const hdmi_device *hdmi_dev, hdmi_scdc_status *status)
{
    status->source_scramble_on = HI_FALSE;
    status->sink_scramble_on = HI_FALSE;
    status->tmds_bit_clk_ratio = SCDC_TMDS_BIT_CLK_RATIO_10X;
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_FALSE);

    hal_call_void(hal_hdmi_scdc_status_set, hdmi_dev->hal, status);
    osal_msleep(SCDC_SET_WAIT);
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_TRUE);

    (hi_void)memset_s(status, sizeof(hdmi_scdc_status), 0, sizeof(hdmi_scdc_status));
    osal_msleep(SCDC_GET_WAIT);
    hal_call_void(hal_hdmi_scdc_status_get, hdmi_dev->hal, status);
    hdmi_dbg_print("\n scdc set OFF %s! \n\n",
        (status->source_scramble_on == HI_FALSE) && (status->sink_scramble_on == HI_FALSE) &&
        (status->tmds_bit_clk_ratio == SCDC_TMDS_BIT_CLK_RATIO_10X) ? STR_SUCC : STR_FAIL);

    return HI_SUCCESS;
}

static hi_s32 debug_scdc_on(const hdmi_device *hdmi_dev, hdmi_scdc_status *status)
{
    status->source_scramble_on = HI_TRUE;
    status->sink_scramble_on = HI_TRUE;
    status->tmds_bit_clk_ratio = SCDC_TMDS_BIT_CLK_RATIO_40X;
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_FALSE);
    hal_call_void(hal_hdmi_scdc_status_set, hdmi_dev->hal, status);
    osal_msleep(SCDC_SET_WAIT);
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_TRUE);

    (hi_void)memset_s(status, sizeof(hdmi_scdc_status), 0, sizeof(hdmi_scdc_status));
    osal_msleep(SCDC_GET_WAIT);
    hal_call_void(hal_hdmi_scdc_status_get, hdmi_dev->hal, status);

    hdmi_dbg_print("\n scdc set ON %s! \n\n", (status->source_scramble_on == HI_TRUE) &&
        (status->sink_scramble_on == HI_TRUE) &&
        (status->tmds_bit_clk_ratio == SCDC_TMDS_BIT_CLK_RATIO_40X) ? STR_SUCC : STR_FAIL);

    return HI_SUCCESS;
}

static hi_s32 debug_scdc_status_get(const hdmi_device *hdmi_dev, hdmi_scdc_status *status)
{
    hal_call_void(hal_hdmi_scdc_status_get, hdmi_dev->hal, status);
    hdmi_dbg_print("scdc get source_scramble_on[%u] \n"
                   "           sink_scramble_on[%u] \n"
                   "         tmds_bit_clk_ratio[%u] \n\n",
                   status->source_scramble_on,
                   status->sink_scramble_on,
                   status->tmds_bit_clk_ratio);

    return HI_SUCCESS;
}


#ifdef HDMI_FRL_SUPPORT
static hi_s32 debug_scdc_read(const hdmi_device *hdmi_dev, const hdmi_debug_cmd_arg *cmd_arg, hi_u8 *value)
{
    hi_u8    offset;
    hi_s32   ret;
    hi_char *debug_char = HI_NULL;

    debug_char = cmd_arg->argv[2];
    debug_null_check_return(debug_char);
    offset = debug_str_to_u32(debug_char, &debug_char, HDMI_DEBUG_BASE_HEX);
    ret = scdc_read_trim(hdmi_dev->hdmi_dev_id, offset, value, 1);
    hdmi_dbg_print("scdc read offset(0x%x), val:0x%x\n\n", offset, *value);

    return ret;
}

static hi_s32 debug_scdc_write(const hdmi_device *hdmi_dev, const hdmi_debug_cmd_arg *cmd_arg, hi_u8 *value)
{
    hi_u8    offset;
    hi_s32   ret;
    hi_char *debug_char = HI_NULL;

    debug_char = cmd_arg->argv[2];
    offset = debug_str_to_u32(debug_char, &debug_char, HDMI_DEBUG_BASE_HEX);
    debug_char = cmd_arg->argv[3];
    *value = debug_str_to_u32(debug_char, &debug_char, HDMI_DEBUG_BASE_HEX);
    ret = scdc_write_trim(hdmi_dev->hdmi_dev_id, offset, value, 1);
    hdmi_dbg_print("SCDC write offset(0x%x), val:0x%x\n\n", offset, *value);

    return ret;
}

static hi_void debug_scdc_time_test(const hdmi_device *hdmi_dev)
{
    hi_u64 time1;
    hi_u64 time2;
    hi_u32 i;
    hi_u32 j = 0;
    hi_u8  data[10] = {0};

    for (i = 0; j < 20; j++) { /* 20, osal get llu time count */
        time1 = hdmi_osal_get_time_in_us();

        for (i = 0; i < 1000; i++) { /* 1000, scdc read count */
            scdc_read_trim(hdmi_dev->hdmi_dev_id, 0x01, &data[0], 1); /* 0x01 & 1, scdc offset value and read len */
            if (data[0] != 0x01) {
                hdmi_dbg_print("scdc read err(0x%x)\n", data[0]);
            }
            data[0] = 0;
        }
        time2 = hdmi_osal_get_time_in_us();
        hdmi_dbg_print("time1: %llu, time2: %llu, %uus\n", time1, time2, (time2 - time1));
    }
    return;
}
#endif

static hi_void debug_scdc_print(hi_void)
{
    hdmi_dbg_print("[usage  ]: echo scdc argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--scdc off.\n"
                   "           1\t--scdc on.\n"
                   "           2\t--scdc status get.\n");
#ifdef HDMI_FRL_SUPPORT
    hdmi_dbg_print("           3\t--scdc read. offset.\n"
                   "           4\t--scdc write. offset, data.\n"
                   "           5\t--simplified SCDC time test.\n");
#endif
    hdmi_dbg_print("[example]: echo scdc 1 > /proc/umap/hdmi0 \n");

    return;
}

static hi_s32 debug_scdc(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device     *hdmi_dev    = HI_NULL;
    hdmi_scdc_status status = {0};
    hi_s32           ret;
#ifdef HDMI_FRL_SUPPORT
    hi_u8            value;
#endif

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    if (cmd_arg->argc > DEBUG_CMD_ARGC_COUNT_3 || cmd_arg->argv[1] == HI_NULL) {
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        ret = debug_scdc_off(hdmi_dev, &status);
        debug_failure_check_return(ret);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        ret = debug_scdc_on(hdmi_dev, &status);
        debug_failure_check_return(ret);
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        ret = debug_scdc_status_get(hdmi_dev, &status);
        debug_failure_check_return(ret);
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
#ifdef HDMI_FRL_SUPPORT
        if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2) {
            goto usage_error;
        }
        ret = debug_scdc_read(hdmi_dev, cmd_arg, &value);
        debug_failure_check_return(ret);
    } else if (is_str_equal(cmd_arg->argv[1], "4")) {
        if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_3) {
            goto usage_error;
        }
        ret = debug_scdc_write(hdmi_dev, cmd_arg, &value);
        debug_failure_check_return(ret);
    } else if (is_str_equal(cmd_arg->argv[1], "5")) {
        debug_scdc_time_test(hdmi_dev);
#endif
    } else {
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_err("error argvs!\n");
    debug_scdc_print();

    return HI_FAILURE;
}
#endif

static hi_s32 debug_ddc(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 ddc_freq;
    hdmi_device *hdmi_dev = HI_NULL;
    hi_char *str = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    str = cmd_arg->argv[1];

    ddc_freq = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (ddc_freq > 300 || ddc_freq == 0) { /* ddc frequency extent is 1-300 kHz */
        goto usage_error;
    }

    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_DDC_FREQ, &ddc_freq);
    hdmi_dbg_print("\n ddc frequency set %u_k_hz! \n\n", ddc_freq);

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo ddc argv1 > /proc/msp/hdmi0\n");
    hdmi_dbg_print("[argv1  ]: ddc frequency [1,300] k_hz\n");

    return HI_FAILURE;
}

static hi_s32 debug_hdmi_mode(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device *hdmi_dev = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, HDMI_TMDS_MODE_DVI);
        hdmi_dbg_print("\n hdmi tmds mode set DVI %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, HDMI_TMDS_MODE_HDMI_1_4);
        hdmi_dbg_print("\n hdmi tmds mode set HDMI1.4 %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, HDMI_TMDS_MODE_HDMI_2_0);
        hdmi_dbg_print("\n hdmi tmds mode set HDMI2.0 %s! \n\n", STR_SUCC);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo hdmimode argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--DVI.\n"
                   "           1\t--HDMI1.4.\n"
                   "           2\t--HDMI2.0.\n");
    hdmi_dbg_print("[example]: echo hdmimode 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_out_color_space(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32       ret;
    hi_u32       old_mode;
    hdmi_attr    attr = {0};
    hdmi_device *hdmi_dev = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    ret = drv_hdmi_attr_get(hdmi_dev, &attr);
    debug_failure_check_return(ret);
    old_mode = (hi_u32)attr.app_attr.out_color_space;

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        attr.app_attr.out_color_space = HDMI_COLORSPACE_RGB;
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        attr.app_attr.out_color_space = HDMI_COLORSPACE_YCBCR422;
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        attr.app_attr.out_color_space = HDMI_COLORSPACE_YCBCR444;
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
        attr.app_attr.out_color_space = HDMI_COLORSPACE_YCBCR420;
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    drv_hdmi_stop(hdmi_dev);
    ret = drv_hdmi_attr_set(hdmi_dev, &attr);
    drv_hdmi_start(hdmi_dev);
    hdmi_dbg_print("\n output colorspace mode %u->%u %s! \n\n", old_mode,
                   attr.app_attr.out_color_space, ret == HI_SUCCESS ? STR_SUCC : STR_FAIL);

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo outclrspace argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--RGB.\n"
                   "           1\t--YUV422.\n"
                   "           2\t--YUV444.\n"
                   "           3\t--YUV420.\n");
    hdmi_dbg_print("[example]: echo outclrspace 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_s32 debug_deep_color(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32       ret;
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_attr    attr = {0};
    hi_u32       old_mode;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    ret = drv_hdmi_attr_get(hdmi_dev, &attr);
    debug_failure_check_return(ret);
    old_mode = (hi_u32)attr.app_attr.deep_color_mode;

    if (attr.app_attr.out_color_space == HDMI_COLORSPACE_YCBCR422) {
        hdmi_dbg_err("colorspace YUV422 not support deepcolor set! force set\n");
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        attr.app_attr.deep_color_mode = HDMI_DEEP_COLOR_24BIT;
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        attr.app_attr.deep_color_mode = HDMI_DEEP_COLOR_30BIT;
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        attr.app_attr.deep_color_mode = HDMI_DEEP_COLOR_36BIT;
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    drv_hdmi_stop(hdmi_dev);
    ret = drv_hdmi_attr_set(hdmi_dev, &attr);
    drv_hdmi_start(hdmi_dev);
    hdmi_dbg_print("\n deppcolor colorspace mode %u->%u %s! \n\n", old_mode,
                   attr.app_attr.deep_color_mode, ret == HI_SUCCESS ? STR_SUCC : STR_FAIL);

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo deepclr argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--08 bit.\n"
                   "           1\t--10 bit.\n"
                   "           2\t--12 bit.\n");
    hdmi_dbg_print("[example]: echo deepclr 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

#ifndef HDMI_PRODUCT_HI3559A
static hi_s32 debug_dither(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_video_dither dither_mode = HDMI_VIDEO_DITHER_DISABLE;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        dither_mode = HDMI_VIDEO_DITHER_DISABLE;

        hdmi_dbg_print("\n dither is disable %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        dither_mode = HDMI_VIDEO_DITHER_10_8;
        hdmi_dbg_print("\n dither is enable 10_08 %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        dither_mode = HDMI_VIDEO_DITHER_12_8;
        hdmi_dbg_print("\n dither is enable 12_08 %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
        dither_mode = HDMI_VIDEO_DITHER_12_10;
        hdmi_dbg_print("\n dither is enable 12_10 %s! \n\n", STR_SUCC);
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_DITHER, &dither_mode);

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo dither argv1 > /proc/umap/hdmi0\n");
    hdmi_dbg_print("[argv1  ]: 0\t--dither disable.\n"
                   "           1\t--10_08.\n"
                   "           2\t--12_08.\n"
                   "           3\t--12_10.\n");
    hdmi_dbg_print("[example]: echo dither 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}
#endif

static hi_s32 debug_authenticate(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device *hdmi_dev = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc != DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        hdmi_dev->attr.app_attr.auth_mode = HI_FALSE;
        hdmi_dbg_print("\n aut disable %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        hdmi_dev->attr.app_attr.auth_mode = HI_TRUE;
        hdmi_dbg_print("\n aut enable %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        hdmi_dbg_print("\n aut status %u! \n\n", hdmi_dev->attr.app_attr.auth_mode);
    } else {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    hdmi_dbg_print("[usage  ]: echo aut argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--aut off.\n"
                   "           1\t--aut on.\n"
                   "           2\t--aut status get.\n");
    hdmi_dbg_print("[example]: echo aut 1 > /proc/umap/hdmi0 \n");

    return HI_FAILURE;
}

static hi_void debug_cmd_help(hi_void)
{
    hdmi_dbg_err("error args!\n");
    hdmi_dbg_print("[usage  ]: echo cmd argv1 argv2(optional) > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: 0\t--hdmi stop.\n"
                   "           1\t--hdmi start.\n"
                   "           2\t--black frame enable.\n"
                   "           3\t--black frame disable.\n"
                   "           4\t--PCB length set (must set argv2).\n");
    hdmi_dbg_print("[example]: echo cmd 1 > /proc/umap/hdmi0 \n");
    return;
}

static hi_s32 debug_cmd(hdmi_debug_cmd_arg *cmd_arg)
{
    hdmi_device          *hdmi_dev = HI_NULL;
    hdmi_black_frame_info black_info = {0};
    hi_u32                data;
    hi_char              *str = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    (hi_void)memset_s(&black_info, sizeof(hdmi_black_frame_info), 0, sizeof(hdmi_black_frame_info));
    black_info.in_bit_depth = hdmi_dev->attr.vo_attr.in_bit_depth;
    black_info.in_color_space = hdmi_dev->attr.vo_attr.in_color_space;
    black_info.in_quantization = (black_info.in_color_space == HDMI_COLORSPACE_RGB) ?
        hdmi_dev->attr.vo_attr.rgb_quantization : hdmi_dev->attr.vo_attr.ycc_quantization;
    /* input require argc is 2, argv 1 is none null */
    if ((cmd_arg->argc < 2) || (cmd_arg->argc > 3) || cmd_arg->argv[1] == HI_NULL) {
        goto usage_error;
    }

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        drv_hdmi_stop(hdmi_dev);
        hdmi_dbg_print("\n HDMI stop %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        drv_hdmi_start(hdmi_dev);
        hdmi_dbg_print("\n HDMI start %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        black_info.black_enable = HI_TRUE;
        hal_call_void(hal_hdmi_black_data_set, hdmi_dev->hal, &black_info);
        hdmi_dbg_print("\n black frame enable %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
        black_info.black_enable = HI_FALSE;
        hal_call_void(hal_hdmi_black_data_set, hdmi_dev->hal, &black_info);
        hdmi_dbg_print("\n black frame disable %s! \n\n", STR_SUCC);
    } else if (is_str_equal(cmd_arg->argv[1], "4")) {
        if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2) {
            goto usage_error;
        }

        str = cmd_arg->argv[2];
        debug_null_check_return(str);
        data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
        data = (data > HDMI_TRACE_LEN_1) ? HDMI_TRACE_LEN_1 : data;
        hdmi_dbg_print("\n PCB trace len old(%u)->new(%u)! \n\n", hdmi_dev->mode_param.trace_len, data);
        hdmi_dev->mode_param.trace_len = (hdmi_trace_len)data;
    } else {
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    debug_cmd_help();

    return HI_FAILURE;
}

#ifdef HDMI_HDR_SUPPORT
static hi_s32 debug_hdr_mode_set(hdmi_device *hdmi_dev, hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32 ret;
    hi_u32 old_mode;
    hdmi_attr          attr     = {0};
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;

    hdr_attr = &(hdmi_dev->attr.hdr_attr);
    old_mode = hdr_attr->user_hdr_mode;

    debug_null_check_return(cmd_arg->argv[2]);
    if (is_str_equal(cmd_arg->argv[2], "1")) {
        hdr_attr->user_hdr_mode = HDMI_HDR_USERMODE_HDR10;
    } else if (is_str_equal(cmd_arg->argv[2], "2")) {
        hdr_attr->user_hdr_mode = HDMI_HDR_USERMODE_DOLBY;
    } else {
        hdr_attr->user_hdr_mode = HDMI_HDR_USERMODE_SDR;
    }

    ret = drv_hdmi_attr_get(hdmi_dev, &attr);
    debug_failure_check_return(ret);

    drv_hdmi_stop(hdmi_dev);
    ret = drv_hdmi_attr_set(hdmi_dev, &attr);
    drv_hdmi_start(hdmi_dev);

    hdmi_dbg_print("\n user hdr mode change %u->%u %s! \n\n", old_mode,
                   attr.hdr_attr.user_hdr_mode, ret == HI_SUCCESS ? STR_SUCC : STR_FAIL);

    return HI_SUCCESS;
}

static hi_void debug_hdr_hdr10(hdmi_debug_cmd_arg *cmd_arg, drv_hdmi_hdr_info *hdr_info)
{
    if (is_str_equal(cmd_arg->argv[2], "1")) {
        hdr_info->hdr_debug_mode = HDMI_HDR_DEBUG_MODE_OE;
    } else if (is_str_equal(cmd_arg->argv[2], "2")) {
        hdr_info->hdr_debug_mode = HDMI_HDR_DEBUG_MODE_AVMUTE;
    } else {
        hdr_info->hdr_debug_mode = HDMI_HDR_DEBUG_MODE_NONE;
    }
    hdmi_dbg_print("\n debug mode set %u! \n\n", hdr_info->hdr_debug_mode);

    return;
}

static hi_s32 debug_hdr_delay_set(hdmi_debug_cmd_arg *cmd_arg, drv_hdmi_hdr_info *hdr_info)
{
    hi_u32 time;
    hi_char *str = cmd_arg->argv[2];

    debug_null_check_return(str);
    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2) {
        return HI_FAILURE;
    }
    time = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (time < 1 || time > 10000) {  /* OE delay time extent 1~10000 */
        return HI_FAILURE;
    }
    hdmi_dbg_print("\n debug OE delay time change %u->%u! \n\n", hdr_info->hdr_mode_chn_timer.time, time);
    hdr_info->hdr_mode_chn_timer.time = time;

    return HI_SUCCESS;
}

static hi_void debug_hdr_print(hi_void)
{
    hdmi_dbg_print("[usage  ]: echo hdr argv1 argv2 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: HDR debug command.\n");
    hdmi_dbg_print("           0\t--debug HDR mode set (argv2 = 1-HDR10, 2-dolby, else-SDR).\n"
                   "           1\t--debug mode set when the HDR mode change to HDR10"
                   "(argv2 = 1-OE, 2-AVMUTE, else-disable.)\n"
                   "           2\t--debug mode delay time(1~10000ms, default 100ms).\n"
                   "           3\t--zero DRM infoframe send time(1-10000ms, default 2000ms).\n");

    hdmi_dbg_print("[argv2  ]: command param.\n");
    hdmi_dbg_print("[example]: echo hdr 0 1 > /proc/umap/hdmi0\n");
    hdmi_dbg_print("[example]: echo hdr 1 2 > /proc/umap/hdmi0\n");
    hdmi_dbg_print("[example]: echo hdr 2 500 > /proc/umap/hdmi0\n");
    hdmi_dbg_print("[example]: echo hdr 3 3000 > /proc/umap/hdmi0\n");

    return;
}

static hi_s32 debug_hdr(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_s32 ret;
    hi_u32 time;
    hi_char           *str      = HI_NULL;
    hdmi_device       *hdmi_dev = HI_NULL;
    drv_hdmi_hdr_info *hdr_info = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    debug_null_check_return(hdmi_dev->hal);

    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_1 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    hdr_info = &(hdmi_dev->hdr_info);

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        /* HDR mode set. */
        ret = debug_hdr_mode_set(hdmi_dev, cmd_arg);
        debug_failure_check_return(ret);
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2 || cmd_arg->argv[2] == HI_NULL) {
            goto usage_error;
        }
        debug_hdr_hdr10(cmd_arg, hdr_info);
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        if (debug_hdr_delay_set(cmd_arg, hdr_info) != HI_SUCCESS) {
            goto usage_error;
        }
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
        if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2) {
            goto usage_error;
        }

        str = cmd_arg->argv[2];
        debug_null_check_return(str);

        time = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
        if (time < 1 || time > 10000) { /* DRM send time extent 1~10000 */
            goto usage_error;
        }
        hdmi_dbg_print("\n debug zero DRM infoframe send time change %u->%u! \n\n",
                       hdr_info->zero_drm_if_timer.time, time);
        hdr_info->zero_drm_if_timer.time = time;
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    debug_hdr_print();

    return HI_FAILURE;
}
#endif

#ifdef HDMI_FRL_SUPPORT
hi_void debug_timer_isr(hi_void *param)
{
    hi_u8 data = 0;

    hi_unused(param);
    g_timer_test++;

    scdc_read_trim(0, 0x01, &data, 1);
    if (g_timer_test <= DEBUG_TIME_TEST_MAX) {
        hal_timer_start();
    } else {
        hal_timer_stop();
    }
    return;
}

frl_rx_crc_check g_rx_table[] = {
    { 0x11143820, 0x1   }, { 0x11143824, 0x940000  }, { 0x11143828, 0x0   }, { 0x11143824, 0x6b0000  },
    { 0x11143828, 0x0   }, { 0x11143824, 0x8c0000  }, { 0x11143828, 0x1   }, { 0x11143824, 0x730000  },
    { 0x11143828, 0x1   }, { 0x11143824, 0x4c0000  }, { 0x11143828, 0x2   }, { 0x11143824, 0xb30000  },
    { 0x11143828, 0x2   }, { 0x11143824, 0xc80000  }, { 0x11143828, 0x3   }, { 0x11143824, 0x370000  },
    { 0x11143828, 0x3   }, { 0x11143824, 0x2c0000  }, { 0x11143828, 0x4   }, { 0x11143824, 0xd30000  },
    { 0x11143828, 0x4   }, { 0x11143824, 0xa80000  }, { 0x11143828, 0x5   }, { 0x11143824, 0x570000  },
    { 0x11143828, 0x5   }, { 0x11143824, 0x680000  }, { 0x11143828, 0x6   }, { 0x11143824, 0x970000  },
    { 0x11143828, 0x6   }, { 0x11143824, 0xe10000  }, { 0x11143828, 0x7   }, { 0x11143824, 0xe10000  },
    { 0x11143828, 0x7   }, { 0x11143824, 0x920000  }, { 0x11143828, 0x8   }, { 0x11143824, 0x6d0000  },
    { 0x11143828, 0x8   }, { 0x11143824, 0x980000  }, { 0x11143828, 0x9   }, { 0x11143824, 0x670000  },
    { 0x11143828, 0x9   }, { 0x11143824, 0x580000  }, { 0x11143828, 0xa   }, { 0x11143824, 0xa70000  },
    { 0x11143828, 0xa   }, { 0x11143824, 0xd10000  }, { 0x11143828, 0xb   }, { 0x11143824, 0xd10000  },
    { 0x11143828, 0xb   }, { 0x11143824, 0x380000  }, { 0x11143828, 0xc   }, { 0x11143824, 0xc70000  },
    { 0x11143828, 0xc   }, { 0x11143824, 0xb10000  }, { 0x11143828, 0xd   }, { 0x11143824, 0xb10000  },
    { 0x11143828, 0xd   }, { 0x11143824, 0x710000  }, { 0x11143828, 0xe   }, { 0x11143824, 0x710000  },
    { 0x11143828, 0xe   }, { 0x11143824, 0xf0000   }, { 0x11143828, 0xf   }, { 0x11143824, 0xf00000  },
    { 0x11143828, 0xf   }, { 0x11143824, 0x8a0000  }, { 0x11143828, 0x10  }, { 0x11143824, 0x750000  },
    { 0x11143828, 0x10  }, { 0x11143824, 0x880000  }, { 0x11143828, 0x11  }, { 0x11143824, 0x770000  },
    { 0x11143828, 0x11  }, { 0x11143824, 0x480000  }, { 0x11143828, 0x12  }, { 0x11143824, 0xb70000  },
    { 0x11143828, 0x12  }, { 0x11143824, 0xc90000  }, { 0x11143828, 0x13  }, { 0x11143824, 0xc90000  },
    { 0x11143828, 0x13  }, { 0x11143824, 0x280000  }, { 0x11143828, 0x14  }, { 0x11143824, 0xd70000  },
    { 0x11143828, 0x14  }, { 0x11143824, 0xa90000  }, { 0x11143828, 0x15  }, { 0x11143824, 0xa90000  },
    { 0x11143828, 0x15  }, { 0x11143824, 0x690000  }, { 0x11143828, 0x16  }, { 0x11143824, 0x690000  },
    { 0x11143828, 0x16  }, { 0x11143824, 0xe80000  }, { 0x11143828, 0x17  }, { 0x11143824, 0xe80000  },
    { 0x11143828, 0x17  }, { 0x11143824, 0x180000  }, { 0x11143828, 0x18  }, { 0x11143824, 0xe70000  },
    { 0x11143828, 0x18  }, { 0x11143824, 0x990000  }, { 0x11143828, 0x19  }, { 0x11143824, 0x990000  },
    { 0x11143828, 0x19  }, { 0x11143824, 0x590000  }, { 0x11143828, 0x1a  }, { 0x11143824, 0x590000  },
    { 0x11143828, 0x1a  }, { 0x11143824, 0xd80000  }, { 0x11143828, 0x1b  }, { 0x11143824, 0xd80000  },
    { 0x11143828, 0x1b  }, { 0x11143824, 0x390000  }, { 0x11143828, 0x1c  }, { 0x11143824, 0x390000  },
    { 0x11143828, 0x1c  }, { 0x11143824, 0xb80000  }, { 0x11143828, 0x1d  }, { 0x11143824, 0xb80000  },
    { 0x11143828, 0x1d  }, { 0x11143824, 0x780000  }, { 0x11143828, 0x1e  }, { 0x11143824, 0x780000  },
    { 0x11143828, 0x1e  }, { 0x11143824, 0x160000  }, { 0x11143828, 0x1f  }, { 0x11143824, 0xe90000  },
    { 0x11143828, 0x1f  }, { 0x11143824, 0x540000  }, { 0x11143828, 0x20  }, { 0x11143824, 0xab0000  },
    { 0x11143828, 0x20  }, { 0x11143824, 0x840000  }, { 0x11143828, 0x21  }, { 0x11143824, 0x7b0000  },
    { 0x11143828, 0x21  }, { 0x11143824, 0x440000  }, { 0x11143828, 0x22  }, { 0x11143824, 0xbb0000  },
    { 0x11143828, 0x22  }, { 0x11143824, 0xc50000  }, { 0x11143828, 0x23  }, { 0x11143824, 0xc50000  },
    { 0x11143828, 0x23  }, { 0x11143824, 0x240000  }, { 0x11143828, 0x24  }, { 0x11143824, 0xdb0000  },
    { 0x11143828, 0x24  }, { 0x11143824, 0xa50000  }, { 0x11143828, 0x25  }, { 0x11143824, 0xa50000  },
    { 0x11143828, 0x25  }, { 0x11143824, 0x650000  }, { 0x11143828, 0x26  }, { 0x11143824, 0x650000  },
    { 0x11143828, 0x26  }, { 0x11143824, 0xe40000  }, { 0x11143828, 0x27  }, { 0x11143824, 0xe40000  },
    { 0x11143828, 0x27  }, { 0x11143824, 0x140000  }, { 0x11143828, 0x28  }, { 0x11143824, 0xeb0000  },
    { 0x11143828, 0x28  }, { 0x11143824, 0x950000  }, { 0x11143828, 0x29  }, { 0x11143824, 0x950000  },
    { 0x11143828, 0x29  }, { 0x11143824, 0x550000  }, { 0x11143828, 0x2a  }, { 0x11143824, 0x550000  },
    { 0x11143828, 0x2a  }, { 0x11143824, 0xd40000  }, { 0x11143828, 0x2b  }, { 0x11143824, 0xd40000  },
    { 0x11143828, 0x2b  }, { 0x11143824, 0x350000  }, { 0x11143828, 0x2c  }, { 0x11143824, 0x350000  },
    { 0x11143828, 0x2c  }, { 0x11143824, 0xb40000  }, { 0x11143828, 0x2d  }, { 0x11143824, 0xb40000  },
    { 0x11143828, 0x2d  }, { 0x11143824, 0x740000  }, { 0x11143828, 0x2e  }, { 0x11143824, 0x740000  },
    { 0x11143828, 0x2e  }, { 0x11143824, 0x1a0000  }, { 0x11143828, 0x2f  }, { 0x11143824, 0xe50000  },
    { 0x11143828, 0x2f  }, { 0x11143824, 0xc40000  }, { 0x11143828, 0x30  }, { 0x11143824, 0x3b0000  },
    { 0x11143828, 0x30  }, { 0x11143824, 0x8d0000  }, { 0x11143828, 0x31  }, { 0x11143824, 0x8d0000  },
    { 0x11143828, 0x31  }, { 0x11143824, 0x4d0000  }, { 0x11143828, 0x32  }, { 0x11143824, 0x4d0000  },
    { 0x11143828, 0x32  }, { 0x11143824, 0xcc0000  }, { 0x11143828, 0x33  }, { 0x11143824, 0xcc0000  },
    { 0x11143828, 0x33  }, { 0x11143824, 0x2d0000  }, { 0x11143828, 0x34  }, { 0x11143824, 0x2d0000  },
    { 0x11143828, 0x34  }, { 0x11143824, 0xac0000  }, { 0x11143828, 0x35  }, { 0x11143824, 0xac0000  },
    { 0x11143828, 0x35  }, { 0x11143824, 0x6c0000  }, { 0x11143828, 0x36  }, { 0x11143824, 0x6c0000  },
    { 0x11143828, 0x36  }, { 0x11143824, 0x130000  }, { 0x11143828, 0x37  }, { 0x11143824, 0xec0000  },
    { 0x11143828, 0x37  }, { 0x11143824, 0x1d0000  }, { 0x11143828, 0x38  }, { 0x11143824, 0x1d0000  },
    { 0x11143828, 0x38  }, { 0x11143824, 0x9c0000  }, { 0x11143828, 0x39  }, { 0x11143824, 0x9c0000  },
    { 0x11143828, 0x39  }, { 0x11143824, 0x5c0000  }, { 0x11143828, 0x3a  }, { 0x11143824, 0x5c0000  },
    { 0x11143828, 0x3a  }, { 0x11143824, 0x230000  }, { 0x11143828, 0x3b  }, { 0x11143824, 0xdc0000  },
    { 0x11143828, 0x3b  }, { 0x11143824, 0x3c0000  }, { 0x11143828, 0x3c  }, { 0x11143824, 0x3c0000  },
    { 0x11143828, 0x3c  }, { 0x11143824, 0x430000  }, { 0x11143828, 0x3d  }, { 0x11143824, 0xbc0000  },
    { 0x11143828, 0x3d  }, { 0x11143824, 0x830000  }, { 0x11143828, 0x3e  }, { 0x11143824, 0x7c0000  },
    { 0x11143828, 0x3e  }, { 0x11143824, 0x340000  }, { 0x11143828, 0x3f  }, { 0x11143824, 0xcb0000  },
    { 0x11143828, 0x3f  }, { 0x11143824, 0x520000  }, { 0x11143828, 0x40  }, { 0x11143824, 0xad0000  },
    { 0x11143828, 0x40  }, { 0x11143824, 0x820000  }, { 0x11143828, 0x41  }, { 0x11143824, 0x7d0000  },
    { 0x11143828, 0x41  }, { 0x11143824, 0x420000  }, { 0x11143828, 0x42  }, { 0x11143824, 0xbd0000  },
    { 0x11143828, 0x42  }, { 0x11143824, 0xc30000  }, { 0x11143828, 0x43  }, { 0x11143824, 0xc30000  },
    { 0x11143828, 0x43  }, { 0x11143824, 0x220000  }, { 0x11143828, 0x44  }, { 0x11143824, 0xdd0000  },
    { 0x11143828, 0x44  }, { 0x11143824, 0xa30000  }, { 0x11143828, 0x45  }, { 0x11143824, 0xa30000  },
    { 0x11143828, 0x45  }, { 0x11143824, 0x630000  }, { 0x11143828, 0x46  }, { 0x11143824, 0x630000  },
    { 0x11143828, 0x46  }, { 0x11143824, 0xe20000  }, { 0x11143828, 0x47  }, { 0x11143824, 0xe20000  },
    { 0x11143828, 0x47  }, { 0x11143824, 0x120000  }, { 0x11143828, 0x48  }, { 0x11143824, 0xed0000  },
    { 0x11143828, 0x48  }, { 0x11143824, 0x930000  }, { 0x11143828, 0x49  }, { 0x11143824, 0x930000  },
    { 0x11143828, 0x49  }, { 0x11143824, 0x530000  }, { 0x11143828, 0x4a  }, { 0x11143824, 0x530000  },
    { 0x11143828, 0x4a  }, { 0x11143824, 0xd20000  }, { 0x11143828, 0x4b  }, { 0x11143824, 0xd20000  },
    { 0x11143828, 0x4b  }, { 0x11143824, 0x330000  }, { 0x11143828, 0x4c  }, { 0x11143824, 0x330000  },
    { 0x11143828, 0x4c  }, { 0x11143824, 0xb20000  }, { 0x11143828, 0x4d  }, { 0x11143824, 0xb20000  },
    { 0x11143828, 0x4d  }, { 0x11143824, 0x720000  }, { 0x11143828, 0x4e  }, { 0x11143824, 0x720000  },
    { 0x11143828, 0x4e  }, { 0x11143824, 0x1c0000  }, { 0x11143828, 0x4f  }, { 0x11143824, 0xe30000  },
    { 0x11143828, 0x4f  }, { 0x11143824, 0xc20000  }, { 0x11143828, 0x50  }, { 0x11143824, 0x3d0000  },
    { 0x11143828, 0x50  }, { 0x11143824, 0x8b0000  }, { 0x11143828, 0x51  }, { 0x11143824, 0x8b0000  },
    { 0x11143828, 0x51  }, { 0x11143824, 0x4b0000  }, { 0x11143828, 0x52  }, { 0x11143824, 0x4b0000  },
    { 0x11143828, 0x52  }, { 0x11143824, 0xca0000  }, { 0x11143828, 0x53  }, { 0x11143824, 0xca0000  },
    { 0x11143828, 0x53  }, { 0x11143824, 0x2b0000  }, { 0x11143828, 0x54  }, { 0x11143824, 0x2b0000  },
    { 0x11143828, 0x54  }, { 0x11143824, 0xaa0000  }, { 0x11143828, 0x55  }, { 0x11143824, 0xaa0000  },
    { 0x11143828, 0x55  }, { 0x11143824, 0x6a0000  }, { 0x11143828, 0x56  }, { 0x11143824, 0x6a0000  },
    { 0x11143828, 0x56  }, { 0x11143824, 0x150000  }, { 0x11143828, 0x57  }, { 0x11143824, 0xea0000  },
    { 0x11143828, 0x57  }, { 0x11143824, 0x1b0000  }, { 0x11143828, 0x58  }, { 0x11143824, 0x1b0000  },
    { 0x11143828, 0x58  }, { 0x11143824, 0x9a0000  }, { 0x11143828, 0x59  }, { 0x11143824, 0x9a0000  },
    { 0x11143828, 0x59  }, { 0x11143824, 0x5a0000  }, { 0x11143828, 0x5a  }, { 0x11143824, 0x5a0000  },
    { 0x11143828, 0x5a  }, { 0x11143824, 0x250000  }, { 0x11143828, 0x5b  }, { 0x11143824, 0xda0000  },
    { 0x11143828, 0x5b  }, { 0x11143824, 0x3a0000  }, { 0x11143828, 0x5c  }, { 0x11143824, 0x3a0000  },
    { 0x11143828, 0x5c  }, { 0x11143824, 0x450000  }, { 0x11143828, 0x5d  }, { 0x11143824, 0xba0000  },
    { 0x11143828, 0x5d  }, { 0x11143824, 0x850000  }, { 0x11143828, 0x5e  }, { 0x11143824, 0x7a0000  },
    { 0x11143828, 0x5e  }, { 0x11143824, 0x620000  }, { 0x11143828, 0x5f  }, { 0x11143824, 0x9d0000  },
    { 0x11143828, 0x5f  }, { 0x11143824, 0x260000  }, { 0x11143828, 0x60  }, { 0x11143824, 0xd90000  },
    { 0x11143828, 0x60  }, { 0x11143824, 0x870000  }, { 0x11143828, 0x61  }, { 0x11143824, 0x870000  },
    { 0x11143828, 0x61  }, { 0x11143824, 0x470000  }, { 0x11143828, 0x62  }, { 0x11143824, 0x470000  },
    { 0x11143828, 0x62  }, { 0x11143824, 0xc60000  }, { 0x11143828, 0x63  }, { 0x11143824, 0xc60000  },
    { 0x11143828, 0x63  }, { 0x11143824, 0x270000  }, { 0x11143828, 0x64  }, { 0x11143824, 0x270000  },
    { 0x11143828, 0x64  }, { 0x11143824, 0xa60000  }, { 0x11143828, 0x65  }, { 0x11143824, 0xa60000  },
    { 0x11143828, 0x65  }, { 0x11143824, 0x660000  }, { 0x11143828, 0x66  }, { 0x11143824, 0x660000  },
    { 0x11143828, 0x66  }, { 0x11143824, 0x190000  }, { 0x11143828, 0x67  }, { 0x11143824, 0xe60000  },
    { 0x11143828, 0x67  }, { 0x11143824, 0x170000  }, { 0x11143828, 0x68  }, { 0x11143824, 0x170000  },
    { 0x11143828, 0x68  }, { 0x11143824, 0x960000  }, { 0x11143828, 0x69  }, { 0x11143824, 0x960000  },
    { 0x11143828, 0x69  }, { 0x11143824, 0x560000  }, { 0x11143828, 0x6a  }, { 0x11143824, 0x560000  },
    { 0x11143828, 0x6a  }, { 0x11143824, 0x290000  }, { 0x11143828, 0x6b  }, { 0x11143824, 0xd60000  },
    { 0x11143828, 0x6b  }, { 0x11143824, 0x360000  }, { 0x11143828, 0x6c  }, { 0x11143824, 0x360000  },
    { 0x11143828, 0x6c  }, { 0x11143824, 0x490000  }, { 0x11143828, 0x6d  }, { 0x11143824, 0xb60000  },
    { 0x11143828, 0x6d  }, { 0x11143824, 0x890000  }, { 0x11143828, 0x6e  }, { 0x11143824, 0x760000  },
    { 0x11143828, 0x6e  }, { 0x11143824, 0x320000  }, { 0x11143828, 0x6f  }, { 0x11143824, 0xcd0000  },
    { 0x11143828, 0x6f  }, { 0x11143824, 0x4a0000  }, { 0x11143828, 0x70  }, { 0x11143824, 0xb50000  },
    { 0x11143828, 0x70  }, { 0x11143824, 0x8e0000  }, { 0x11143828, 0x71  }, { 0x11143824, 0x8e0000  },
    { 0x11143828, 0x71  }, { 0x11143824, 0x4e0000  }, { 0x11143828, 0x72  }, { 0x11143824, 0x4e0000  },
    { 0x11143828, 0x72  }, { 0x11143824, 0x310000  }, { 0x11143828, 0x73  }, { 0x11143824, 0xce0000  },
    { 0x11143828, 0x73  }, { 0x11143824, 0x2e0000  }, { 0x11143828, 0x74  }, { 0x11143824, 0x2e0000  },
    { 0x11143828, 0x74  }, { 0x11143824, 0x510000  }, { 0x11143828, 0x75  }, { 0x11143824, 0xae0000  },
    { 0x11143828, 0x75  }, { 0x11143824, 0x910000  }, { 0x11143828, 0x76  }, { 0x11143824, 0x6e0000  },
    { 0x11143828, 0x76  }, { 0x11143824, 0x2a0000  }, { 0x11143828, 0x77  }, { 0x11143824, 0xd50000  },
    { 0x11143828, 0x77  }, { 0x11143824, 0x1e0000  }, { 0x11143828, 0x78  }, { 0x11143824, 0x1e0000  },
    { 0x11143828, 0x78  }, { 0x11143824, 0x610000  }, { 0x11143828, 0x79  }, { 0x11143824, 0x9e0000  },
    { 0x11143828, 0x79  }, { 0x11143824, 0xa10000  }, { 0x11143828, 0x7a  }, { 0x11143824, 0x5e0000  },
    { 0x11143828, 0x7a  }, { 0x11143824, 0x860000  }, { 0x11143828, 0x7b  }, { 0x11143824, 0x790000  },
    { 0x11143828, 0x7b  }, { 0x11143824, 0xc10000  }, { 0x11143828, 0x7c  }, { 0x11143824, 0x3e0000  },
    { 0x11143828, 0x7c  }, { 0x11143824, 0xa40000  }, { 0x11143828, 0x7d  }, { 0x11143824, 0x5b0000  },
    { 0x11143828, 0x7d  }, { 0x11143824, 0x640000  }, { 0x11143828, 0x7e  }, { 0x11143824, 0x9b0000  },
    { 0x11143828, 0x7e  }, { 0x11143824, 0xa20000  }, { 0x11143828, 0x7f  }, { 0x11143824, 0x5d0000  },
    { 0x11143828, 0x7f  }, { 0x11143824, 0xd20010  }, { 0x11143828, 0x0   }, { 0x11143824, 0x32d0010 },
    { 0x11143828, 0x0   }, { 0x11143824, 0x22c0010 }, { 0x11143828, 0x1   }, { 0x11143824, 0x1d30010 },
    { 0x11143828, 0x1   }, { 0x11143824, 0x14a0010 }, { 0x11143828, 0x2   }, { 0x11143824, 0x2b50010 },
    { 0x11143828, 0x2   }, { 0x11143824, 0x3060010 }, { 0x11143828, 0x3   }, { 0x11143824, 0xf90010  },
    { 0x11143828, 0x3   }, { 0x11143824, 0x18a0010 }, { 0x11143828, 0x4   }, { 0x11143824, 0x2750010 },
    { 0x11143828, 0x4   }, { 0x11143824, 0x2860010 }, { 0x11143828, 0x5   }, { 0x11143824, 0x1790010 },
    { 0x11143828, 0x5   }, { 0x11143824, 0x1860010 }, { 0x11143828, 0x6   }, { 0x11143824, 0x2790010 },
    { 0x11143828, 0x6   }, { 0x11143824, 0x3820010 }, { 0x11143828, 0x7   }, { 0x11143824, 0x7d0010  },
    { 0x11143828, 0x7   }, { 0x11143824, 0xca0010  }, { 0x11143828, 0x8   }, { 0x11143824, 0x3350010 },
    { 0x11143828, 0x8   }, { 0x11143824, 0x2460010 }, { 0x11143828, 0x9   }, { 0x11143824, 0x1b90010 },
    { 0x11143828, 0x9   }, { 0x11143824, 0x1460010 }, { 0x11143828, 0xa   }, { 0x11143824, 0x2b90010 },
    { 0x11143828, 0xa   }, { 0x11143824, 0x3420010 }, { 0x11143828, 0xb   }, { 0x11143824, 0xbd0010  },
    { 0x11143828, 0xb   }, { 0x11143824, 0xc60010  }, { 0x11143828, 0xc   }, { 0x11143824, 0x3390010 },
    { 0x11143828, 0xc   }, { 0x11143824, 0x2c20010 }, { 0x11143828, 0xd   }, { 0x11143824, 0x13d0010 },
    { 0x11143828, 0xd   }, { 0x11143824, 0x1c80010 }, { 0x11143828, 0xe   }, { 0x11143824, 0x2370010 },
    { 0x11143828, 0xe   }, { 0x11143824, 0x3440010 }, { 0x11143828, 0xf   }, { 0x11143824, 0xbb0010  },
    { 0x11143828, 0xf   }, { 0x11143824, 0x2340010 }, { 0x11143828, 0x10  }, { 0x11143824, 0x1cb0010 },
    { 0x11143828, 0x10  }, { 0x11143824, 0x2260010 }, { 0x11143828, 0x11  }, { 0x11143824, 0x1d90010 },
    { 0x11143828, 0x11  }, { 0x11143824, 0x1260010 }, { 0x11143828, 0x12  }, { 0x11143824, 0x2d90010 },
    { 0x11143828, 0x12  }, { 0x11143824, 0x3220010 }, { 0x11143828, 0x13  }, { 0x11143824, 0xdd0010  },
    { 0x11143828, 0x13  }, { 0x11143824, 0xa60010  }, { 0x11143828, 0x14  }, { 0x11143824, 0x3590010 },
    { 0x11143828, 0x14  }, { 0x11143824, 0x2a20010 }, { 0x11143828, 0x15  }, { 0x11143824, 0x15d0010 },
    { 0x11143828, 0x15  }, { 0x11143824, 0x1920010 }, { 0x11143828, 0x16  }, { 0x11143824, 0x26d0010 },
    { 0x11143828, 0x16  }, { 0x11143824, 0x3a10010 }, { 0x11143828, 0x17  }, { 0x11143824, 0x3a10010 },
    { 0x11143828, 0x17  }, { 0x11143824, 0x660010  }, { 0x11143828, 0x18  }, { 0x11143824, 0x3990010 },
    { 0x11143828, 0x18  }, { 0x11143824, 0x2620010 }, { 0x11143828, 0x19  }, { 0x11143824, 0x19d0010 },
    { 0x11143828, 0x19  }, { 0x11143824, 0x1620010 }, { 0x11143828, 0x1a  }, { 0x11143824, 0x29d0010 },
    { 0x11143828, 0x1a  }, { 0x11143824, 0x3610010 }, { 0x11143828, 0x1b  }, { 0x11143824, 0x3610010 },
    { 0x11143828, 0x1b  }, { 0x11143824, 0xe20010  }, { 0x11143828, 0x1c  }, { 0x11143824, 0x31d0010 },
    { 0x11143828, 0x1c  }, { 0x11143824, 0x2e10010 }, { 0x11143828, 0x1d  }, { 0x11143824, 0x2e10010 },
    { 0x11143828, 0x1d  }, { 0x11143824, 0x1e10010 }, { 0x11143828, 0x1e  }, { 0x11143824, 0x1e10010 },
    { 0x11143828, 0x1e  }, { 0x11143824, 0x3480010 }, { 0x11143828, 0x1f  }, { 0x11143824, 0xb70010  },
    { 0x11143828, 0x1f  }, { 0x11143824, 0x2320010 }, { 0x11143828, 0x20  }, { 0x11143824, 0x1cd0010 },
    { 0x11143828, 0x20  }, { 0x11143824, 0x2160010 }, { 0x11143828, 0x21  }, { 0x11143824, 0x1e90010 },
    { 0x11143828, 0x21  }, { 0x11143824, 0x1160010 }, { 0x11143828, 0x22  }, { 0x11143824, 0x2e90010 },
    { 0x11143828, 0x22  }, { 0x11143824, 0x3100010 }, { 0x11143828, 0x23  }, { 0x11143824, 0xef0010  },
    { 0x11143828, 0x23  }, { 0x11143824, 0x960010  }, { 0x11143828, 0x24  }, { 0x11143824, 0x3690010 },
    { 0x11143828, 0x24  }, { 0x11143824, 0x2900010 }, { 0x11143828, 0x25  }, { 0x11143824, 0x16f0010 },
    { 0x11143828, 0x25  }, { 0x11143824, 0x1900010 }, { 0x11143828, 0x26  }, { 0x11143824, 0x26f0010 },
    { 0x11143828, 0x26  }, { 0x11143824, 0x3910010 }, { 0x11143828, 0x27  }, { 0x11143824, 0x3910010 },
    { 0x11143828, 0x27  }, { 0x11143824, 0x560010  }, { 0x11143828, 0x28  }, { 0x11143824, 0x3a90010 },
    { 0x11143828, 0x28  }, { 0x11143824, 0x2500010 }, { 0x11143828, 0x29  }, { 0x11143824, 0x1af0010 },
    { 0x11143828, 0x29  }, { 0x11143824, 0x1500010 }, { 0x11143828, 0x2a  }, { 0x11143824, 0x2af0010 },
    { 0x11143828, 0x2a  }, { 0x11143824, 0x3510010 }, { 0x11143828, 0x2b  }, { 0x11143824, 0x3510010 },
    { 0x11143828, 0x2b  }, { 0x11143824, 0xd00010  }, { 0x11143828, 0x2c  }, { 0x11143824, 0x32f0010 },
    { 0x11143828, 0x2c  }, { 0x11143824, 0x2d10010 }, { 0x11143828, 0x2d  }, { 0x11143824, 0x2d10010 },
    { 0x11143828, 0x2d  }, { 0x11143824, 0x1d10010 }, { 0x11143828, 0x2e  }, { 0x11143824, 0x1d10010 },
    { 0x11143828, 0x2e  }, { 0x11143824, 0x2f0010  }, { 0x11143828, 0x2f  }, { 0x11143824, 0x3d00010 },
    { 0x11143828, 0x2f  }, { 0x11143824, 0xb80010  }, { 0x11143828, 0x30  }, { 0x11143824, 0x3470010 },
    { 0x11143828, 0x30  }, { 0x11143824, 0x2300010 }, { 0x11143828, 0x31  }, { 0x11143824, 0x1cf0010 },
    { 0x11143828, 0x31  }, { 0x11143824, 0x1300010 }, { 0x11143828, 0x32  }, { 0x11143824, 0x2cf0010 },
    { 0x11143828, 0x32  }, { 0x11143824, 0x3310010 }, { 0x11143828, 0x33  }, { 0x11143824, 0x3310010 },
    { 0x11143828, 0x33  }, { 0x11143824, 0xb00010  }, { 0x11143828, 0x34  }, { 0x11143824, 0x34f0010 },
    { 0x11143828, 0x34  }, { 0x11143824, 0x2b10010 }, { 0x11143828, 0x35  }, { 0x11143824, 0x2b10010 },
    { 0x11143828, 0x35  }, { 0x11143824, 0x1b10010 }, { 0x11143828, 0x36  }, { 0x11143824, 0x1b10010 },
    { 0x11143828, 0x36  }, { 0x11143824, 0x4f0010  }, { 0x11143828, 0x37  }, { 0x11143824, 0x3b00010 },
    { 0x11143828, 0x37  }, { 0x11143824, 0x700010  }, { 0x11143828, 0x38  }, { 0x11143824, 0x38f0010 },
    { 0x11143828, 0x38  }, { 0x11143824, 0x2710010 }, { 0x11143828, 0x39  }, { 0x11143824, 0x2710010 },
    { 0x11143828, 0x39  }, { 0x11143824, 0x1710010 }, { 0x11143828, 0x3a  }, { 0x11143824, 0x1710010 },
    { 0x11143828, 0x3a  }, { 0x11143824, 0x8f0010  }, { 0x11143828, 0x3b  }, { 0x11143824, 0x3700010 },
    { 0x11143828, 0x3b  }, { 0x11143824, 0xf10010  }, { 0x11143828, 0x3c  }, { 0x11143824, 0xf10010  },
    { 0x11143828, 0x3c  }, { 0x11143824, 0x10f0010 }, { 0x11143828, 0x3d  }, { 0x11143824, 0x2f00010 },
    { 0x11143828, 0x3d  }, { 0x11143824, 0x20f0010 }, { 0x11143828, 0x3e  }, { 0x11143824, 0x1f00010 },
    { 0x11143828, 0x3e  }, { 0x11143824, 0x1c20010 }, { 0x11143828, 0x3f  }, { 0x11143824, 0x23d0010 },
    { 0x11143828, 0x3f  }, { 0x11143824, 0x1380010 }, { 0x11143828, 0x40  }, { 0x11143824, 0x2c70010 },
    { 0x11143828, 0x40  }, { 0x11143824, 0x2580010 }, { 0x11143828, 0x41  }, { 0x11143824, 0x1a70010 },
    { 0x11143828, 0x41  }, { 0x11143824, 0x1580010 }, { 0x11143828, 0x42  }, { 0x11143824, 0x2a70010 },
    { 0x11143828, 0x42  }, { 0x11143824, 0x3080010 }, { 0x11143828, 0x43  }, { 0x11143824, 0xf70010  },
    { 0x11143828, 0x43  }, { 0x11143824, 0xd80010  }, { 0x11143828, 0x44  }, { 0x11143824, 0x3270010 },
    { 0x11143828, 0x44  }, { 0x11143824, 0x2880010 }, { 0x11143828, 0x45  }, { 0x11143824, 0x1770010 },
    { 0x11143828, 0x45  }, { 0x11143824, 0x1880010 }, { 0x11143828, 0x46  }, { 0x11143824, 0x2770010 },
    { 0x11143828, 0x46  }, { 0x11143824, 0x3890010 }, { 0x11143828, 0x47  }, { 0x11143824, 0x3890010 },
    { 0x11143828, 0x47  }, { 0x11143824, 0x14c0010 }, { 0x11143828, 0x48  }, { 0x11143824, 0x2b30010 },
    { 0x11143828, 0x48  }, { 0x11143824, 0x2480010 }, { 0x11143828, 0x49  }, { 0x11143824, 0x1b70010 },
    { 0x11143828, 0x49  }, { 0x11143824, 0x1480010 }, { 0x11143828, 0x4a  }, { 0x11143824, 0x2b70010 },
    { 0x11143828, 0x4a  }, { 0x11143824, 0x3490010 }, { 0x11143828, 0x4b  }, { 0x11143824, 0x3490010 },
    { 0x11143828, 0x4b  }, { 0x11143824, 0xc80010  }, { 0x11143828, 0x4c  }, { 0x11143824, 0x3370010 },
    { 0x11143828, 0x4c  }, { 0x11143824, 0x2c90010 }, { 0x11143828, 0x4d  }, { 0x11143824, 0x2c90010 },
    { 0x11143828, 0x4d  }, { 0x11143824, 0x1c90010 }, { 0x11143828, 0x4e  }, { 0x11143824, 0x1c90010 },
    { 0x11143828, 0x4e  }, { 0x11143824, 0x370010  }, { 0x11143828, 0x4f  }, { 0x11143824, 0x3c80010 },
    { 0x11143828, 0x4f  }, { 0x11143824, 0x3280010 }, { 0x11143828, 0x50  }, { 0x11143824, 0xd70010  },
    { 0x11143828, 0x50  }, { 0x11143824, 0x2280010 }, { 0x11143828, 0x51  }, { 0x11143824, 0x1d70010 },
    { 0x11143828, 0x51  }, { 0x11143824, 0x1280010 }, { 0x11143828, 0x52  }, { 0x11143824, 0x2d70010 },
    { 0x11143828, 0x52  }, { 0x11143824, 0x3290010 }, { 0x11143828, 0x53  }, { 0x11143824, 0x3290010 },
    { 0x11143828, 0x53  }, { 0x11143824, 0xa80010  }, { 0x11143828, 0x54  }, { 0x11143824, 0x3570010 },
    { 0x11143828, 0x54  }, { 0x11143824, 0x2a90010 }, { 0x11143828, 0x55  }, { 0x11143824, 0x2a90010 },
    { 0x11143828, 0x55  }, { 0x11143824, 0x1a90010 }, { 0x11143828, 0x56  }, { 0x11143824, 0x1a90010 },
    { 0x11143828, 0x56  }, { 0x11143824, 0x3a80010 }, { 0x11143828, 0x57  }, { 0x11143824, 0x3a80010 },
    { 0x11143828, 0x57  }, { 0x11143824, 0x680010  }, { 0x11143828, 0x58  }, { 0x11143824, 0x3970010 },
    { 0x11143828, 0x58  }, { 0x11143824, 0x2690010 }, { 0x11143828, 0x59  }, { 0x11143824, 0x2690010 },
    { 0x11143828, 0x59  }, { 0x11143824, 0x1690010 }, { 0x11143828, 0x5a  }, { 0x11143824, 0x1690010 },
    { 0x11143828, 0x5a  }, { 0x11143824, 0x3680010 }, { 0x11143828, 0x5b  }, { 0x11143824, 0x3680010 },
    { 0x11143828, 0x5b  }, { 0x11143824, 0xe90010  }, { 0x11143828, 0x5c  }, { 0x11143824, 0xe90010  },
    { 0x11143828, 0x5c  }, { 0x11143824, 0x2e80010 }, { 0x11143828, 0x5d  }, { 0x11143824, 0x2e80010 },
    { 0x11143828, 0x5d  }, { 0x11143824, 0x1e80010 }, { 0x11143828, 0x5e  }, { 0x11143824, 0x1e80010 },
    { 0x11143828, 0x5e  }, { 0x11143824, 0x1a80010 }, { 0x11143828, 0x5f  }, { 0x11143824, 0x2570010 },
    { 0x11143828, 0x5f  }, { 0x11143824, 0x3180010 }, { 0x11143828, 0x60  }, { 0x11143824, 0xe70010  },
    { 0x11143828, 0x60  }, { 0x11143824, 0x2180010 }, { 0x11143828, 0x61  }, { 0x11143824, 0x1e70010 },
    { 0x11143828, 0x61  }, { 0x11143824, 0x1180010 }, { 0x11143828, 0x62  }, { 0x11143824, 0x2e70010 },
    { 0x11143828, 0x62  }, { 0x11143824, 0x3190010 }, { 0x11143828, 0x63  }, { 0x11143824, 0x3190010 },
    { 0x11143828, 0x63  }, { 0x11143824, 0x980010  }, { 0x11143828, 0x64  }, { 0x11143824, 0x3670010 },
    { 0x11143828, 0x64  }, { 0x11143824, 0x2990010 }, { 0x11143828, 0x65  }, { 0x11143824, 0x2990010 },
    { 0x11143828, 0x65  }, { 0x11143824, 0x1990010 }, { 0x11143828, 0x66  }, { 0x11143824, 0x1990010 },
    { 0x11143828, 0x66  }, { 0x11143824, 0x3980010 }, { 0x11143828, 0x67  }, { 0x11143824, 0x3980010 },
    { 0x11143828, 0x67  }, { 0x11143824, 0x580010  }, { 0x11143828, 0x68  }, { 0x11143824, 0x3a70010 },
    { 0x11143828, 0x68  }, { 0x11143824, 0x2590010 }, { 0x11143828, 0x69  }, { 0x11143824, 0x2590010 },
    { 0x11143828, 0x69  }, { 0x11143824, 0x1590010 }, { 0x11143828, 0x6a  }, { 0x11143824, 0x1590010 },
    { 0x11143828, 0x6a  }, { 0x11143824, 0x3580010 }, { 0x11143828, 0x6b  }, { 0x11143824, 0x3580010 },
    { 0x11143828, 0x6b  }, { 0x11143824, 0xd90010  }, { 0x11143828, 0x6c  }, { 0x11143824, 0xd90010  },
    { 0x11143828, 0x6c  }, { 0x11143824, 0x2d80010 }, { 0x11143828, 0x6d  }, { 0x11143824, 0x2d80010 },
    { 0x11143828, 0x6d  }, { 0x11143824, 0x1d80010 }, { 0x11143828, 0x6e  }, { 0x11143824, 0x1d80010 },
    { 0x11143828, 0x6e  }, { 0x11143824, 0x2980010 }, { 0x11143828, 0x6f  }, { 0x11143824, 0x1670010 },
    { 0x11143828, 0x6f  }, { 0x11143824, 0x380010  }, { 0x11143828, 0x70  }, { 0x11143824, 0x3c70010 },
    { 0x11143828, 0x70  }, { 0x11143824, 0x2390010 }, { 0x11143828, 0x71  }, { 0x11143824, 0x2390010 },
    { 0x11143828, 0x71  }, { 0x11143824, 0x1390010 }, { 0x11143828, 0x72  }, { 0x11143824, 0x1390010 },
    { 0x11143828, 0x72  }, { 0x11143824, 0x3380010 }, { 0x11143828, 0x73  }, { 0x11143824, 0x3380010 },
    { 0x11143828, 0x73  }, { 0x11143824, 0xb90010  }, { 0x11143828, 0x74  }, { 0x11143824, 0xb90010  },
    { 0x11143828, 0x74  }, { 0x11143824, 0x2b80010 }, { 0x11143828, 0x75  }, { 0x11143824, 0x2b80010 },
    { 0x11143828, 0x75  }, { 0x11143824, 0x1b80010 }, { 0x11143828, 0x76  }, { 0x11143824, 0x1b80010 },
    { 0x11143828, 0x76  }, { 0x11143824, 0x470010  }, { 0x11143828, 0x77  }, { 0x11143824, 0x3b80010 },
    { 0x11143828, 0x77  }, { 0x11143824, 0x790010  }, { 0x11143828, 0x78  }, { 0x11143824, 0x790010  },
    { 0x11143828, 0x78  }, { 0x11143824, 0x2780010 }, { 0x11143828, 0x79  }, { 0x11143824, 0x2780010 },
    { 0x11143828, 0x79  }, { 0x11143824, 0x1780010 }, { 0x11143828, 0x7a  }, { 0x11143824, 0x1780010 },
    { 0x11143828, 0x7a  }, { 0x11143824, 0x870010  }, { 0x11143828, 0x7b  }, { 0x11143824, 0x3780010 },
    { 0x11143828, 0x7b  }, { 0x11143824, 0xf80010  }, { 0x11143828, 0x7c  }, { 0x11143824, 0xf80010  },
    { 0x11143828, 0x7c  }, { 0x11143824, 0x1070010 }, { 0x11143828, 0x7d  }, { 0x11143824, 0x2f80010 },
    { 0x11143828, 0x7d  }, { 0x11143824, 0x2070010 }, { 0x11143828, 0x7e  }, { 0x11143824, 0x1f80010 },
    { 0x11143828, 0x7e  }, { 0x11143824, 0x2680010 }, { 0x11143828, 0x7f  }, { 0x11143824, 0x1970010 },
    { 0x11143828, 0x7f  }, { 0x11143824, 0x1340010 }, { 0x11143828, 0x80  }, { 0x11143824, 0x2cb0010 },
    { 0x11143828, 0x80  }, { 0x11143824, 0x21c0010 }, { 0x11143828, 0x81  }, { 0x11143824, 0x1e30010 },
    { 0x11143828, 0x81  }, { 0x11143824, 0x11c0010 }, { 0x11143828, 0x82  }, { 0x11143824, 0x2e30010 },
    { 0x11143828, 0x82  }, { 0x11143824, 0x3040010 }, { 0x11143828, 0x83  }, { 0x11143824, 0xfb0010  },
    { 0x11143828, 0x83  }, { 0x11143824, 0x9c0010  }, { 0x11143828, 0x84  }, { 0x11143824, 0x3630010 },
    { 0x11143828, 0x84  }, { 0x11143824, 0x2840010 }, { 0x11143828, 0x85  }, { 0x11143824, 0x17b0010 },
    { 0x11143828, 0x85  }, { 0x11143824, 0x1840010 }, { 0x11143828, 0x86  }, { 0x11143824, 0x27b0010 },
    { 0x11143828, 0x86  }, { 0x11143824, 0x3850010 }, { 0x11143828, 0x87  }, { 0x11143824, 0x3850010 },
    { 0x11143828, 0x87  }, { 0x11143824, 0x5c0010  }, { 0x11143828, 0x88  }, { 0x11143824, 0x3a30010 },
    { 0x11143828, 0x88  }, { 0x11143824, 0x2440010 }, { 0x11143828, 0x89  }, { 0x11143824, 0x1bb0010 },
    { 0x11143828, 0x89  }, { 0x11143824, 0x1440010 }, { 0x11143828, 0x8a  }, { 0x11143824, 0x2bb0010 },
    { 0x11143828, 0x8a  }, { 0x11143824, 0x3450010 }, { 0x11143828, 0x8b  }, { 0x11143824, 0x3450010 },
    { 0x11143828, 0x8b  }, { 0x11143824, 0xc40010  }, { 0x11143828, 0x8c  }, { 0x11143824, 0x33b0010 },
    { 0x11143828, 0x8c  }, { 0x11143824, 0x2c50010 }, { 0x11143828, 0x8d  }, { 0x11143824, 0x2c50010 },
    { 0x11143828, 0x8d  }, { 0x11143824, 0x1c50010 }, { 0x11143828, 0x8e  }, { 0x11143824, 0x1c50010 },
    { 0x11143828, 0x8e  }, { 0x11143824, 0x3b0010  }, { 0x11143828, 0x8f  }, { 0x11143824, 0x3c40010 },
    { 0x11143828, 0x8f  }, { 0x11143824, 0x3240010 }, { 0x11143828, 0x90  }, { 0x11143824, 0xdb0010  },
    { 0x11143828, 0x90  }, { 0x11143824, 0x2240010 }, { 0x11143828, 0x91  }, { 0x11143824, 0x1db0010 },
    { 0x11143828, 0x91  }, { 0x11143824, 0x1240010 }, { 0x11143828, 0x92  }, { 0x11143824, 0x2db0010 },
    { 0x11143828, 0x92  }, { 0x11143824, 0x3250010 }, { 0x11143828, 0x93  }, { 0x11143824, 0x3250010 },
    { 0x11143828, 0x93  }, { 0x11143824, 0xa40010  }, { 0x11143828, 0x94  }, { 0x11143824, 0x35b0010 },
    { 0x11143828, 0x94  }, { 0x11143824, 0x2a50010 }, { 0x11143828, 0x95  }, { 0x11143824, 0x2a50010 },
    { 0x11143828, 0x95  }, { 0x11143824, 0x1a50010 }, { 0x11143828, 0x96  }, { 0x11143824, 0x1a50010 },
    { 0x11143828, 0x96  }, { 0x11143824, 0x3a40010 }, { 0x11143828, 0x97  }, { 0x11143824, 0x3a40010 },
    { 0x11143828, 0x97  }, { 0x11143824, 0x640010  }, { 0x11143828, 0x98  }, { 0x11143824, 0x39b0010 },
    { 0x11143828, 0x98  }, { 0x11143824, 0x2650010 }, { 0x11143828, 0x99  }, { 0x11143824, 0x2650010 },
    { 0x11143828, 0x99  }, { 0x11143824, 0x1650010 }, { 0x11143828, 0x9a  }, { 0x11143824, 0x1650010 },
    { 0x11143828, 0x9a  }, { 0x11143824, 0x3640010 }, { 0x11143828, 0x9b  }, { 0x11143824, 0x3640010 },
    { 0x11143828, 0x9b  }, { 0x11143824, 0xe50010  }, { 0x11143828, 0x9c  }, { 0x11143824, 0xe50010  },
    { 0x11143828, 0x9c  }, { 0x11143824, 0x2e40010 }, { 0x11143828, 0x9d  }, { 0x11143824, 0x2e40010 },
    { 0x11143828, 0x9d  }, { 0x11143824, 0x1e40010 }, { 0x11143828, 0x9e  }, { 0x11143824, 0x1e40010 },
    { 0x11143828, 0x9e  }, { 0x11143824, 0x1a40010 }, { 0x11143828, 0x9f  }, { 0x11143824, 0x25b0010 },
    { 0x11143828, 0x9f  }, { 0x11143824, 0x3140010 }, { 0x11143828, 0xa0  }, { 0x11143824, 0xeb0010  },
    { 0x11143828, 0xa0  }, { 0x11143824, 0x2140010 }, { 0x11143828, 0xa1  }, { 0x11143824, 0x1eb0010 },
    { 0x11143828, 0xa1  }, { 0x11143824, 0x1140010 }, { 0x11143828, 0xa2  }, { 0x11143824, 0x2eb0010 },
    { 0x11143828, 0xa2  }, { 0x11143824, 0x3150010 }, { 0x11143828, 0xa3  }, { 0x11143824, 0x3150010 },
    { 0x11143828, 0xa3  }, { 0x11143824, 0x940010  }, { 0x11143828, 0xa4  }, { 0x11143824, 0x36b0010 },
    { 0x11143828, 0xa4  }, { 0x11143824, 0x2950010 }, { 0x11143828, 0xa5  }, { 0x11143824, 0x2950010 },
    { 0x11143828, 0xa5  }, { 0x11143824, 0x1950010 }, { 0x11143828, 0xa6  }, { 0x11143824, 0x1950010 },
    { 0x11143828, 0xa6  }, { 0x11143824, 0x3940010 }, { 0x11143828, 0xa7  }, { 0x11143824, 0x3940010 },
    { 0x11143828, 0xa7  }, { 0x11143824, 0x540010  }, { 0x11143828, 0xa8  }, { 0x11143824, 0x3ab0010 },
    { 0x11143828, 0xa8  }, { 0x11143824, 0x2550010 }, { 0x11143828, 0xa9  }, { 0x11143824, 0x2550010 },
    { 0x11143828, 0xa9  }, { 0x11143824, 0x1550010 }, { 0x11143828, 0xaa  }, { 0x11143824, 0x1550010 },
    { 0x11143828, 0xaa  }, { 0x11143824, 0x3540010 }, { 0x11143828, 0xab  }, { 0x11143824, 0x3540010 },
    { 0x11143828, 0xab  }, { 0x11143824, 0xd50010  }, { 0x11143828, 0xac  }, { 0x11143824, 0xd50010  },
    { 0x11143828, 0xac  }, { 0x11143824, 0x2d40010 }, { 0x11143828, 0xad  }, { 0x11143824, 0x2d40010 },
    { 0x11143828, 0xad  }, { 0x11143824, 0x1d40010 }, { 0x11143828, 0xae  }, { 0x11143824, 0x1d40010 },
    { 0x11143828, 0xae  }, { 0x11143824, 0x2940010 }, { 0x11143828, 0xaf  }, { 0x11143824, 0x16b0010 },
    { 0x11143828, 0xaf  }, { 0x11143824, 0x340010  }, { 0x11143828, 0xb0  }, { 0x11143824, 0x3cb0010 },
    { 0x11143828, 0xb0  }, { 0x11143824, 0x2350010 }, { 0x11143828, 0xb1  }, { 0x11143824, 0x2350010 },
    { 0x11143828, 0xb1  }, { 0x11143824, 0x1350010 }, { 0x11143828, 0xb2  }, { 0x11143824, 0x1350010 },
    { 0x11143828, 0xb2  }, { 0x11143824, 0x3340010 }, { 0x11143828, 0xb3  }, { 0x11143824, 0x3340010 },
    { 0x11143828, 0xb3  }, { 0x11143824, 0xb50010  }, { 0x11143828, 0xb4  }, { 0x11143824, 0xb50010  },
    { 0x11143828, 0xb4  }, { 0x11143824, 0x2b40010 }, { 0x11143828, 0xb5  }, { 0x11143824, 0x2b40010 },
    { 0x11143828, 0xb5  }, { 0x11143824, 0x1b40010 }, { 0x11143828, 0xb6  }, { 0x11143824, 0x1b40010 },
    { 0x11143828, 0xb6  }, { 0x11143824, 0x4b0010  }, { 0x11143828, 0xb7  }, { 0x11143824, 0x3b40010 },
    { 0x11143828, 0xb7  }, { 0x11143824, 0x750010  }, { 0x11143828, 0xb8  }, { 0x11143824, 0x750010  },
    { 0x11143828, 0xb8  }, { 0x11143824, 0x2740010 }, { 0x11143828, 0xb9  }, { 0x11143824, 0x2740010 },
    { 0x11143828, 0xb9  }, { 0x11143824, 0x1740010 }, { 0x11143828, 0xba  }, { 0x11143824, 0x1740010 },
    { 0x11143828, 0xba  }, { 0x11143824, 0x8b0010  }, { 0x11143828, 0xbb  }, { 0x11143824, 0x3740010 },
    { 0x11143828, 0xbb  }, { 0x11143824, 0xf40010  }, { 0x11143828, 0xbc  }, { 0x11143824, 0xf40010  },
    { 0x11143828, 0xbc  }, { 0x11143824, 0x10b0010 }, { 0x11143828, 0xbd  }, { 0x11143824, 0x2f40010 },
    { 0x11143828, 0xbd  }, { 0x11143824, 0x20b0010 }, { 0x11143828, 0xbe  }, { 0x11143824, 0x1f40010 },
    { 0x11143828, 0xbe  }, { 0x11143824, 0x2640010 }, { 0x11143828, 0xbf  }, { 0x11143824, 0x19b0010 },
    { 0x11143828, 0xbf  }, { 0x11143824, 0x30c0010 }, { 0x11143828, 0xc0  }, { 0x11143824, 0xf30010  },
    { 0x11143828, 0xc0  }, { 0x11143824, 0x20c0010 }, { 0x11143828, 0xc1  }, { 0x11143824, 0x1f30010 },
    { 0x11143828, 0xc1  }, { 0x11143824, 0x10c0010 }, { 0x11143828, 0xc2  }, { 0x11143824, 0x2f30010 },
    { 0x11143828, 0xc2  }, { 0x11143824, 0x30d0010 }, { 0x11143828, 0xc3  }, { 0x11143824, 0x30d0010 },
    { 0x11143828, 0xc3  }, { 0x11143824, 0x8c0010  }, { 0x11143828, 0xc4  }, { 0x11143824, 0x3730010 },
    { 0x11143828, 0xc4  }, { 0x11143824, 0x28d0010 }, { 0x11143828, 0xc5  }, { 0x11143824, 0x28d0010 },
    { 0x11143828, 0xc5  }, { 0x11143824, 0x18d0010 }, { 0x11143828, 0xc6  }, { 0x11143824, 0x18d0010 },
    { 0x11143828, 0xc6  }, { 0x11143824, 0x38c0010 }, { 0x11143828, 0xc7  }, { 0x11143824, 0x38c0010 },
    { 0x11143828, 0xc7  }, { 0x11143824, 0x4c0010  }, { 0x11143828, 0xc8  }, { 0x11143824, 0x3b30010 },
    { 0x11143828, 0xc8  }, { 0x11143824, 0x24d0010 }, { 0x11143828, 0xc9  }, { 0x11143824, 0x24d0010 },
    { 0x11143828, 0xc9  }, { 0x11143824, 0x14d0010 }, { 0x11143828, 0xca  }, { 0x11143824, 0x14d0010 },
    { 0x11143828, 0xca  }, { 0x11143824, 0x34c0010 }, { 0x11143828, 0xcb  }, { 0x11143824, 0x34c0010 },
    { 0x11143828, 0xcb  }, { 0x11143824, 0xcd0010  }, { 0x11143828, 0xcc  }, { 0x11143824, 0xcd0010  },
    { 0x11143828, 0xcc  }, { 0x11143824, 0x2cc0010 }, { 0x11143828, 0xcd  }, { 0x11143824, 0x2cc0010 },
    { 0x11143828, 0xcd  }, { 0x11143824, 0x1cc0010 }, { 0x11143828, 0xce  }, { 0x11143824, 0x1cc0010 },
    { 0x11143828, 0xce  }, { 0x11143824, 0x28c0010 }, { 0x11143828, 0xcf  }, { 0x11143824, 0x1730010 },
    { 0x11143828, 0xcf  }, { 0x11143824, 0x2c0010  }, { 0x11143828, 0xd0  }, { 0x11143824, 0x3d30010 },
    { 0x11143828, 0xd0  }, { 0x11143824, 0x22d0010 }, { 0x11143828, 0xd1  }, { 0x11143824, 0x22d0010 },
    { 0x11143828, 0xd1  }, { 0x11143824, 0x12d0010 }, { 0x11143828, 0xd2  }, { 0x11143824, 0x12d0010 },
    { 0x11143828, 0xd2  }, { 0x11143824, 0x32c0010 }, { 0x11143828, 0xd3  }, { 0x11143824, 0x32c0010 },
    { 0x11143828, 0xd3  }, { 0x11143824, 0xad0010  }, { 0x11143828, 0xd4  }, { 0x11143824, 0xad0010  },
    { 0x11143828, 0xd4  }, { 0x11143824, 0x2ac0010 }, { 0x11143828, 0xd5  }, { 0x11143824, 0x2ac0010 },
    { 0x11143828, 0xd5  }, { 0x11143824, 0x1ac0010 }, { 0x11143828, 0xd6  }, { 0x11143824, 0x1ac0010 },
    { 0x11143828, 0xd6  }, { 0x11143824, 0x530010  }, { 0x11143828, 0xd7  }, { 0x11143824, 0x3ac0010 },
    { 0x11143828, 0xd7  }, { 0x11143824, 0x6d0010  }, { 0x11143828, 0xd8  }, { 0x11143824, 0x6d0010  },
    { 0x11143828, 0xd8  }, { 0x11143824, 0x26c0010 }, { 0x11143828, 0xd9  }, { 0x11143824, 0x26c0010 },
    { 0x11143828, 0xd9  }, { 0x11143824, 0x16c0010 }, { 0x11143828, 0xda  }, { 0x11143824, 0x16c0010 },
    { 0x11143828, 0xda  }, { 0x11143824, 0x930010  }, { 0x11143828, 0xdb  }, { 0x11143824, 0x36c0010 },
    { 0x11143828, 0xdb  }, { 0x11143824, 0xec0010  }, { 0x11143828, 0xdc  }, { 0x11143824, 0xec0010  },
    { 0x11143828, 0xdc  }, { 0x11143824, 0x1130010 }, { 0x11143828, 0xdd  }, { 0x11143824, 0x2ec0010 },
    { 0x11143828, 0xdd  }, { 0x11143824, 0x2130010 }, { 0x11143828, 0xde  }, { 0x11143824, 0x1ec0010 },
    { 0x11143828, 0xde  }, { 0x11143824, 0x24c0010 }, { 0x11143828, 0xdf  }, { 0x11143824, 0x1b30010 },
    { 0x11143828, 0xdf  }, { 0x11143824, 0xac0010  }, { 0x11143828, 0xe0  }, { 0x11143824, 0x3530010 },
    { 0x11143828, 0xe0  }, { 0x11143824, 0x21d0010 }, { 0x11143828, 0xe1  }, { 0x11143824, 0x21d0010 },
    { 0x11143828, 0xe1  }, { 0x11143824, 0x11d0010 }, { 0x11143828, 0xe2  }, { 0x11143824, 0x11d0010 },
    { 0x11143828, 0xe2  }, { 0x11143824, 0x31c0010 }, { 0x11143828, 0xe3  }, { 0x11143824, 0x31c0010 },
    { 0x11143828, 0xe3  }, { 0x11143824, 0x9d0010  }, { 0x11143828, 0xe4  }, { 0x11143824, 0x9d0010  },
    { 0x11143828, 0xe4  }, { 0x11143824, 0x29c0010 }, { 0x11143828, 0xe5  }, { 0x11143824, 0x29c0010 },
    { 0x11143828, 0xe5  }, { 0x11143824, 0x19c0010 }, { 0x11143828, 0xe6  }, { 0x11143824, 0x19c0010 },
    { 0x11143828, 0xe6  }, { 0x11143824, 0x630010  }, { 0x11143828, 0xe7  }, { 0x11143824, 0x39c0010 },
    { 0x11143828, 0xe7  }, { 0x11143824, 0x5d0010  }, { 0x11143828, 0xe8  }, { 0x11143824, 0x5d0010  },
    { 0x11143828, 0xe8  }, { 0x11143824, 0x25c0010 }, { 0x11143828, 0xe9  }, { 0x11143824, 0x25c0010 },
    { 0x11143828, 0xe9  }, { 0x11143824, 0x15c0010 }, { 0x11143828, 0xea  }, { 0x11143824, 0x15c0010 },
    { 0x11143828, 0xea  }, { 0x11143824, 0xa30010  }, { 0x11143828, 0xeb  }, { 0x11143824, 0x35c0010 },
    { 0x11143828, 0xeb  }, { 0x11143824, 0xdc0010  }, { 0x11143828, 0xec  }, { 0x11143824, 0xdc0010  },
    { 0x11143828, 0xec  }, { 0x11143824, 0x1230010 }, { 0x11143828, 0xed  }, { 0x11143824, 0x2dc0010 },
    { 0x11143828, 0xed  }, { 0x11143824, 0x2230010 }, { 0x11143828, 0xee  }, { 0x11143824, 0x1dc0010 },
    { 0x11143828, 0xee  }, { 0x11143824, 0xcc0010  }, { 0x11143828, 0xef  }, { 0x11143824, 0x3330010 },
    { 0x11143828, 0xef  }, { 0x11143824, 0x720010  }, { 0x11143828, 0xf0  }, { 0x11143824, 0x38d0010 },
    { 0x11143828, 0xf0  }, { 0x11143824, 0x23c0010 }, { 0x11143828, 0xf1  }, { 0x11143824, 0x23c0010 },
    { 0x11143828, 0xf1  }, { 0x11143824, 0x13c0010 }, { 0x11143828, 0xf2  }, { 0x11143824, 0x13c0010 },
    { 0x11143828, 0xf2  }, { 0x11143824, 0xc30010  }, { 0x11143828, 0xf3  }, { 0x11143824, 0x33c0010 },
    { 0x11143828, 0xf3  }, { 0x11143824, 0xbc0010  }, { 0x11143828, 0xf4  }, { 0x11143824, 0xbc0010  },
    { 0x11143828, 0xf4  }, { 0x11143824, 0x1430010 }, { 0x11143828, 0xf5  }, { 0x11143824, 0x2bc0010 },
    { 0x11143828, 0xf5  }, { 0x11143824, 0x2430010 }, { 0x11143828, 0xf6  }, { 0x11143824, 0x1bc0010 },
    { 0x11143828, 0xf6  }, { 0x11143824, 0x430010  }, { 0x11143828, 0xf7  }, { 0x11143824, 0x3bc0010 },
    { 0x11143828, 0xf7  }, { 0x11143824, 0x7c0010  }, { 0x11143828, 0xf8  }, { 0x11143824, 0x7c0010  },
    { 0x11143828, 0xf8  }, { 0x11143824, 0x1830010 }, { 0x11143828, 0xf9  }, { 0x11143824, 0x27c0010 },
    { 0x11143828, 0xf9  }, { 0x11143824, 0x2830010 }, { 0x11143828, 0xfa  }, { 0x11143824, 0x17c0010 },
    { 0x11143828, 0xfa  }, { 0x11143824, 0x830010  }, { 0x11143828, 0xfb  }, { 0x11143824, 0x37c0010 },
    { 0x11143828, 0xfb  }, { 0x11143824, 0x3030010 }, { 0x11143828, 0xfc  }, { 0x11143824, 0xfc0010  },
    { 0x11143828, 0xfc  }, { 0x11143824, 0x1030010 }, { 0x11143828, 0xfd  }, { 0x11143824, 0x2fc0010 },
    { 0x11143828, 0xfd  }, { 0x11143824, 0x2030010 }, { 0x11143828, 0xfe  }, { 0x11143824, 0x1fc0010 },
    { 0x11143828, 0xfe  }, { 0x11143824, 0x2a40010 }, { 0x11143828, 0xff  }, { 0x11143824, 0x15b0010 },
    { 0x11143828, 0xff  }, { 0x11143824, 0xaa0010  }, { 0x11143828, 0x100 }, { 0x11143824, 0x3550010 },
    { 0x11143828, 0x100 }, { 0x11143824, 0x21a0010 }, { 0x11143828, 0x101 }, { 0x11143824, 0x1e50010 },
    { 0x11143828, 0x101 }, { 0x11143824, 0x11a0010 }, { 0x11143828, 0x102 }, { 0x11143824, 0x2e50010 },
    { 0x11143828, 0x102 }, { 0x11143824, 0x3020010 }, { 0x11143828, 0x103 }, { 0x11143824, 0xfd0010  },
    { 0x11143828, 0x103 }, { 0x11143824, 0x9a0010  }, { 0x11143828, 0x104 }, { 0x11143824, 0x3650010 },
    { 0x11143828, 0x104 }, { 0x11143824, 0x2820010 }, { 0x11143828, 0x105 }, { 0x11143824, 0x17d0010 },
    { 0x11143828, 0x105 }, { 0x11143824, 0x1820010 }, { 0x11143828, 0x106 }, { 0x11143824, 0x27d0010 },
    { 0x11143828, 0x106 }, { 0x11143824, 0x3830010 }, { 0x11143828, 0x107 }, { 0x11143824, 0x3830010 },
    { 0x11143828, 0x107 }, { 0x11143824, 0x5a0010  }, { 0x11143828, 0x108 }, { 0x11143824, 0x3a50010 },
    { 0x11143828, 0x108 }, { 0x11143824, 0x2420010 }, { 0x11143828, 0x109 }, { 0x11143824, 0x1bd0010 },
    { 0x11143828, 0x109 }, { 0x11143824, 0x1420010 }, { 0x11143828, 0x10a }, { 0x11143824, 0x2bd0010 },
    { 0x11143828, 0x10a }, { 0x11143824, 0x3430010 }, { 0x11143828, 0x10b }, { 0x11143824, 0x3430010 },
    { 0x11143828, 0x10b }, { 0x11143824, 0xc20010  }, { 0x11143828, 0x10c }, { 0x11143824, 0x33d0010 },
    { 0x11143828, 0x10c }, { 0x11143824, 0x2c30010 }, { 0x11143828, 0x10d }, { 0x11143824, 0x2c30010 },
    { 0x11143828, 0x10d }, { 0x11143824, 0x1c30010 }, { 0x11143828, 0x10e }, { 0x11143824, 0x1c30010 },
    { 0x11143828, 0x10e }, { 0x11143824, 0x3d0010  }, { 0x11143828, 0x10f }, { 0x11143824, 0x3c20010 },
    { 0x11143828, 0x10f }, { 0x11143824, 0x22a0010 }, { 0x11143828, 0x110 }, { 0x11143824, 0x1d50010 },
    { 0x11143828, 0x110 }, { 0x11143824, 0x2220010 }, { 0x11143828, 0x111 }, { 0x11143824, 0x1dd0010 },
    { 0x11143828, 0x111 }, { 0x11143824, 0x1220010 }, { 0x11143828, 0x112 }, { 0x11143824, 0x2dd0010 },
    { 0x11143828, 0x112 }, { 0x11143824, 0x3230010 }, { 0x11143828, 0x113 }, { 0x11143824, 0x3230010 },
    { 0x11143828, 0x113 }, { 0x11143824, 0xa20010  }, { 0x11143828, 0x114 }, { 0x11143824, 0x35d0010 },
    { 0x11143828, 0x114 }, { 0x11143824, 0x2a30010 }, { 0x11143828, 0x115 }, { 0x11143824, 0x2a30010 },
    { 0x11143828, 0x115 }, { 0x11143824, 0x1a30010 }, { 0x11143828, 0x116 }, { 0x11143824, 0x1a30010 },
    { 0x11143828, 0x116 }, { 0x11143824, 0x3a20010 }, { 0x11143828, 0x117 }, { 0x11143824, 0x3a20010 },
    { 0x11143828, 0x117 }, { 0x11143824, 0x620010  }, { 0x11143828, 0x118 }, { 0x11143824, 0x39d0010 },
    { 0x11143828, 0x118 }, { 0x11143824, 0x2630010 }, { 0x11143828, 0x119 }, { 0x11143824, 0x2630010 },
    { 0x11143828, 0x119 }, { 0x11143824, 0x1630010 }, { 0x11143828, 0x11a }, { 0x11143824, 0x1630010 },
    { 0x11143828, 0x11a }, { 0x11143824, 0x3620010 }, { 0x11143828, 0x11b }, { 0x11143824, 0x3620010 },
    { 0x11143828, 0x11b }, { 0x11143824, 0xe30010  }, { 0x11143828, 0x11c }, { 0x11143824, 0xe30010  },
    { 0x11143828, 0x11c }, { 0x11143824, 0x2e20010 }, { 0x11143828, 0x11d }, { 0x11143824, 0x2e20010 },
    { 0x11143828, 0x11d }, { 0x11143824, 0x1e20010 }, { 0x11143828, 0x11e }, { 0x11143824, 0x1e20010 },
    { 0x11143828, 0x11e }, { 0x11143824, 0x1a20010 }, { 0x11143828, 0x11f }, { 0x11143824, 0x25d0010 },
    { 0x11143828, 0x11f }, { 0x11143824, 0x3120010 }, { 0x11143828, 0x120 }, { 0x11143824, 0xed0010  },
    { 0x11143828, 0x120 }, { 0x11143824, 0x2120010 }, { 0x11143828, 0x121 }, { 0x11143824, 0x1ed0010 },
    { 0x11143828, 0x121 }, { 0x11143824, 0x1120010 }, { 0x11143828, 0x122 }, { 0x11143824, 0x2ed0010 },
    { 0x11143828, 0x122 }, { 0x11143824, 0x3130010 }, { 0x11143828, 0x123 }, { 0x11143824, 0x3130010 },
    { 0x11143828, 0x123 }, { 0x11143824, 0x920010  }, { 0x11143828, 0x124 }, { 0x11143824, 0x36d0010 },
    { 0x11143828, 0x124 }, { 0x11143824, 0x2930010 }, { 0x11143828, 0x125 }, { 0x11143824, 0x2930010 },
    { 0x11143828, 0x125 }, { 0x11143824, 0x1930010 }, { 0x11143828, 0x126 }, { 0x11143824, 0x1930010 },
    { 0x11143828, 0x126 }, { 0x11143824, 0x3920010 }, { 0x11143828, 0x127 }, { 0x11143824, 0x3920010 },
    { 0x11143828, 0x127 }, { 0x11143824, 0x520010  }, { 0x11143828, 0x128 }, { 0x11143824, 0x3ad0010 },
    { 0x11143828, 0x128 }, { 0x11143824, 0x2530010 }, { 0x11143828, 0x129 }, { 0x11143824, 0x2530010 },
    { 0x11143828, 0x129 }, { 0x11143824, 0x1530010 }, { 0x11143828, 0x12a }, { 0x11143824, 0x1530010 },
    { 0x11143828, 0x12a }, { 0x11143824, 0x3520010 }, { 0x11143828, 0x12b }, { 0x11143824, 0x3520010 },
    { 0x11143828, 0x12b }, { 0x11143824, 0xd30010  }, { 0x11143828, 0x12c }, { 0x11143824, 0xd30010  },
    { 0x11143828, 0x12c }, { 0x11143824, 0x2d20010 }, { 0x11143828, 0x12d }, { 0x11143824, 0x2d20010 },
    { 0x11143828, 0x12d }, { 0x11143824, 0x1d20010 }, { 0x11143828, 0x12e }, { 0x11143824, 0x1d20010 },
    { 0x11143828, 0x12e }, { 0x11143824, 0x2920010 }, { 0x11143828, 0x12f }, { 0x11143824, 0x16d0010 },
    { 0x11143828, 0x12f }, { 0x11143824, 0x320010  }, { 0x11143828, 0x130 }, { 0x11143824, 0x3cd0010 },
    { 0x11143828, 0x130 }, { 0x11143824, 0x2330010 }, { 0x11143828, 0x131 }, { 0x11143824, 0x2330010 },
    { 0x11143828, 0x131 }, { 0x11143824, 0x1330010 }, { 0x11143828, 0x132 }, { 0x11143824, 0x1330010 },
    { 0x11143828, 0x132 }, { 0x11143824, 0x3320010 }, { 0x11143828, 0x133 }, { 0x11143824, 0x3320010 },
    { 0x11143828, 0x133 }, { 0x11143824, 0xb30010  }, { 0x11143828, 0x134 }, { 0x11143824, 0xb30010  },
    { 0x11143828, 0x134 }, { 0x11143824, 0x2b20010 }, { 0x11143828, 0x135 }, { 0x11143824, 0x2b20010 },
    { 0x11143828, 0x135 }, { 0x11143824, 0x1b20010 }, { 0x11143828, 0x136 }, { 0x11143824, 0x1b20010 },
    { 0x11143828, 0x136 }, { 0x11143824, 0x4d0010  }, { 0x11143828, 0x137 }, { 0x11143824, 0x3b20010 },
    { 0x11143828, 0x137 }, { 0x11143824, 0x730010  }, { 0x11143828, 0x138 }, { 0x11143824, 0x730010  },
    { 0x11143828, 0x138 }, { 0x11143824, 0x2720010 }, { 0x11143828, 0x139 }, { 0x11143824, 0x2720010 },
    { 0x11143828, 0x139 }, { 0x11143824, 0x1720010 }, { 0x11143828, 0x13a }, { 0x11143824, 0x1720010 },
    { 0x11143828, 0x13a }, { 0x11143824, 0x8d0010  }, { 0x11143828, 0x13b }, { 0x11143824, 0x3720010 },
    { 0x11143828, 0x13b }, { 0x11143824, 0xf20010  }, { 0x11143828, 0x13c }, { 0x11143824, 0xf20010  },
    { 0x11143828, 0x13c }, { 0x11143824, 0x10d0010 }, { 0x11143828, 0x13d }, { 0x11143824, 0x2f20010 },
    { 0x11143828, 0x13d }, { 0x11143824, 0x20d0010 }, { 0x11143828, 0x13e }, { 0x11143824, 0x1f20010 },
    { 0x11143828, 0x13e }, { 0x11143824, 0x2520010 }, { 0x11143828, 0x13f }, { 0x11143824, 0x1ad0010 },
    { 0x11143828, 0x13f }, { 0x11143824, 0x30a0010 }, { 0x11143828, 0x140 }, { 0x11143824, 0xf50010  },
    { 0x11143828, 0x140 }, { 0x11143824, 0x20a0010 }, { 0x11143828, 0x141 }, { 0x11143824, 0x1f50010 },
    { 0x11143828, 0x141 }, { 0x11143824, 0x10a0010 }, { 0x11143828, 0x142 }, { 0x11143824, 0x2f50010 },
    { 0x11143828, 0x142 }, { 0x11143824, 0x30b0010 }, { 0x11143828, 0x143 }, { 0x11143824, 0x30b0010 },
    { 0x11143828, 0x143 }, { 0x11143824, 0x8a0010  }, { 0x11143828, 0x144 }, { 0x11143824, 0x3750010 },
    { 0x11143828, 0x144 }, { 0x11143824, 0x28b0010 }, { 0x11143828, 0x145 }, { 0x11143824, 0x28b0010 },
    { 0x11143828, 0x145 }, { 0x11143824, 0x18b0010 }, { 0x11143828, 0x146 }, { 0x11143824, 0x18b0010 },
    { 0x11143828, 0x146 }, { 0x11143824, 0x38a0010 }, { 0x11143828, 0x147 }, { 0x11143824, 0x38a0010 },
    { 0x11143828, 0x147 }, { 0x11143824, 0x4a0010  }, { 0x11143828, 0x148 }, { 0x11143824, 0x3b50010 },
    { 0x11143828, 0x148 }, { 0x11143824, 0x24b0010 }, { 0x11143828, 0x149 }, { 0x11143824, 0x24b0010 },
    { 0x11143828, 0x149 }, { 0x11143824, 0x14b0010 }, { 0x11143828, 0x14a }, { 0x11143824, 0x14b0010 },
    { 0x11143828, 0x14a }, { 0x11143824, 0x34a0010 }, { 0x11143828, 0x14b }, { 0x11143824, 0x34a0010 },
    { 0x11143828, 0x14b }, { 0x11143824, 0xcb0010  }, { 0x11143828, 0x14c }, { 0x11143824, 0xcb0010  },
    { 0x11143828, 0x14c }, { 0x11143824, 0x2ca0010 }, { 0x11143828, 0x14d }, { 0x11143824, 0x2ca0010 },
    { 0x11143828, 0x14d }, { 0x11143824, 0x1ca0010 }, { 0x11143828, 0x14e }, { 0x11143824, 0x1ca0010 },
    { 0x11143828, 0x14e }, { 0x11143824, 0x3880010 }, { 0x11143828, 0x14f }, { 0x11143824, 0x770010  },
    { 0x11143828, 0x14f }, { 0x11143824, 0x2a0010  }, { 0x11143828, 0x150 }, { 0x11143824, 0x3d50010 },
    { 0x11143828, 0x150 }, { 0x11143824, 0x22b0010 }, { 0x11143828, 0x151 }, { 0x11143824, 0x22b0010 },
    { 0x11143828, 0x151 }, { 0x11143824, 0x12b0010 }, { 0x11143828, 0x152 }, { 0x11143824, 0x12b0010 },
    { 0x11143828, 0x152 }, { 0x11143824, 0x32a0010 }, { 0x11143828, 0x153 }, { 0x11143824, 0x32a0010 },
    { 0x11143828, 0x153 }, { 0x11143824, 0xab0010  }, { 0x11143828, 0x154 }, { 0x11143824, 0xab0010  },
    { 0x11143828, 0x154 }, { 0x11143824, 0x2aa0010 }, { 0x11143828, 0x155 }, { 0x11143824, 0x2aa0010 },
    { 0x11143828, 0x155 }, { 0x11143824, 0x1aa0010 }, { 0x11143828, 0x156 }, { 0x11143824, 0x1aa0010 },
    { 0x11143828, 0x156 }, { 0x11143824, 0x550010  }, { 0x11143828, 0x157 }, { 0x11143824, 0x3aa0010 },
    { 0x11143828, 0x157 }, { 0x11143824, 0x6b0010  }, { 0x11143828, 0x158 }, { 0x11143824, 0x6b0010  },
    { 0x11143828, 0x158 }, { 0x11143824, 0x26a0010 }, { 0x11143828, 0x159 }, { 0x11143824, 0x26a0010 },
    { 0x11143828, 0x159 }, { 0x11143824, 0x16a0010 }, { 0x11143828, 0x15a }, { 0x11143824, 0x16a0010 },
    { 0x11143828, 0x15a }, { 0x11143824, 0x950010  }, { 0x11143828, 0x15b }, { 0x11143824, 0x36a0010 },
    { 0x11143828, 0x15b }, { 0x11143824, 0xea0010  }, { 0x11143828, 0x15c }, { 0x11143824, 0xea0010  },
    { 0x11143828, 0x15c }, { 0x11143824, 0x1150010 }, { 0x11143828, 0x15d }, { 0x11143824, 0x2ea0010 },
    { 0x11143828, 0x15d }, { 0x11143824, 0x2150010 }, { 0x11143828, 0x15e }, { 0x11143824, 0x1ea0010 },
    { 0x11143828, 0x15e }, { 0x11143824, 0x24a0010 }, { 0x11143828, 0x15f }, { 0x11143824, 0x1b50010 },
    { 0x11143828, 0x15f }, { 0x11143824, 0x28a0010 }, { 0x11143828, 0x160 }, { 0x11143824, 0x1750010 },
    { 0x11143828, 0x160 }, { 0x11143824, 0x21b0010 }, { 0x11143828, 0x161 }, { 0x11143824, 0x21b0010 },
    { 0x11143828, 0x161 }, { 0x11143824, 0x11b0010 }, { 0x11143828, 0x162 }, { 0x11143824, 0x11b0010 },
    { 0x11143828, 0x162 }, { 0x11143824, 0x31a0010 }, { 0x11143828, 0x163 }, { 0x11143824, 0x31a0010 },
    { 0x11143828, 0x163 }, { 0x11143824, 0x9b0010  }, { 0x11143828, 0x164 }, { 0x11143824, 0x9b0010  },
    { 0x11143828, 0x164 }, { 0x11143824, 0x29a0010 }, { 0x11143828, 0x165 }, { 0x11143824, 0x29a0010 },
    { 0x11143828, 0x165 }, { 0x11143824, 0x19a0010 }, { 0x11143828, 0x166 }, { 0x11143824, 0x19a0010 },
    { 0x11143828, 0x166 }, { 0x11143824, 0x650010  }, { 0x11143828, 0x167 }, { 0x11143824, 0x39a0010 },
    { 0x11143828, 0x167 }, { 0x11143824, 0x5b0010  }, { 0x11143828, 0x168 }, { 0x11143824, 0x5b0010  },
    { 0x11143828, 0x168 }, { 0x11143824, 0x25a0010 }, { 0x11143828, 0x169 }, { 0x11143824, 0x25a0010 },
    { 0x11143828, 0x169 }, { 0x11143824, 0x15a0010 }, { 0x11143828, 0x16a }, { 0x11143824, 0x15a0010 },
    { 0x11143828, 0x16a }, { 0x11143824, 0xa50010  }, { 0x11143828, 0x16b }, { 0x11143824, 0x35a0010 },
    { 0x11143828, 0x16b }, { 0x11143824, 0xda0010  }, { 0x11143828, 0x16c }, { 0x11143824, 0xda0010  },
    { 0x11143828, 0x16c }, { 0x11143824, 0x1250010 }, { 0x11143828, 0x16d }, { 0x11143824, 0x2da0010 },
    { 0x11143828, 0x16d }, { 0x11143824, 0x2250010 }, { 0x11143828, 0x16e }, { 0x11143824, 0x1da0010 },
    { 0x11143828, 0x16e }, { 0x11143824, 0x250010  }, { 0x11143828, 0x16f }, { 0x11143824, 0x3da0010 },
    { 0x11143828, 0x16f }, { 0x11143824, 0x780010  }, { 0x11143828, 0x170 }, { 0x11143824, 0x3870010 },
    { 0x11143828, 0x170 }, { 0x11143824, 0x23a0010 }, { 0x11143828, 0x171 }, { 0x11143824, 0x23a0010 },
    { 0x11143828, 0x171 }, { 0x11143824, 0x13a0010 }, { 0x11143828, 0x172 }, { 0x11143824, 0x13a0010 },
    { 0x11143828, 0x172 }, { 0x11143824, 0xc50010  }, { 0x11143828, 0x173 }, { 0x11143824, 0x33a0010 },
    { 0x11143828, 0x173 }, { 0x11143824, 0xba0010  }, { 0x11143828, 0x174 }, { 0x11143824, 0xba0010  },
    { 0x11143828, 0x174 }, { 0x11143824, 0x1450010 }, { 0x11143828, 0x175 }, { 0x11143824, 0x2ba0010 },
    { 0x11143828, 0x175 }, { 0x11143824, 0x2450010 }, { 0x11143828, 0x176 }, { 0x11143824, 0x1ba0010 },
    { 0x11143828, 0x176 }, { 0x11143824, 0x450010  }, { 0x11143828, 0x177 }, { 0x11143824, 0x3ba0010 },
    { 0x11143828, 0x177 }, { 0x11143824, 0x7a0010  }, { 0x11143828, 0x178 }, { 0x11143824, 0x7a0010  },
    { 0x11143828, 0x178 }, { 0x11143824, 0x1850010 }, { 0x11143828, 0x179 }, { 0x11143824, 0x27a0010 },
    { 0x11143828, 0x179 }, { 0x11143824, 0x2850010 }, { 0x11143828, 0x17a }, { 0x11143824, 0x17a0010 },
    { 0x11143828, 0x17a }, { 0x11143824, 0x850010  }, { 0x11143828, 0x17b }, { 0x11143824, 0x37a0010 },
    { 0x11143828, 0x17b }, { 0x11143824, 0x3050010 }, { 0x11143828, 0x17c }, { 0x11143824, 0xfa0010  },
    { 0x11143828, 0x17c }, { 0x11143824, 0x1050010 }, { 0x11143828, 0x17d }, { 0x11143824, 0x2fa0010 },
    { 0x11143828, 0x17d }, { 0x11143824, 0x2050010 }, { 0x11143828, 0x17e }, { 0x11143824, 0x1fa0010 },
    { 0x11143828, 0x17e }, { 0x11143824, 0x2a80010 }, { 0x11143828, 0x17f }, { 0x11143824, 0x1570010 },
    { 0x11143828, 0x17f }, { 0x11143824, 0x1520010 }, { 0x11143828, 0x180 }, { 0x11143824, 0x2ad0010 },
    { 0x11143828, 0x180 }, { 0x11143824, 0x2060010 }, { 0x11143828, 0x181 }, { 0x11143824, 0x1f90010 },
    { 0x11143828, 0x181 }, { 0x11143824, 0x1060010 }, { 0x11143828, 0x182 }, { 0x11143824, 0x2f90010 },
    { 0x11143828, 0x182 }, { 0x11143824, 0x3070010 }, { 0x11143828, 0x183 }, { 0x11143824, 0x3070010 },
    { 0x11143828, 0x183 }, { 0x11143824, 0x860010  }, { 0x11143828, 0x184 }, { 0x11143824, 0x3790010 },
    { 0x11143828, 0x184 }, { 0x11143824, 0x2870010 }, { 0x11143828, 0x185 }, { 0x11143824, 0x2870010 },
    { 0x11143828, 0x185 }, { 0x11143824, 0x1870010 }, { 0x11143828, 0x186 }, { 0x11143824, 0x1870010 },
    { 0x11143828, 0x186 }, { 0x11143824, 0x3860010 }, { 0x11143828, 0x187 }, { 0x11143824, 0x3860010 },
    { 0x11143828, 0x187 }, { 0x11143824, 0x460010  }, { 0x11143828, 0x188 }, { 0x11143824, 0x3b90010 },
    { 0x11143828, 0x188 }, { 0x11143824, 0x2470010 }, { 0x11143828, 0x189 }, { 0x11143824, 0x2470010 },
    { 0x11143828, 0x189 }, { 0x11143824, 0x1470010 }, { 0x11143828, 0x18a }, { 0x11143824, 0x1470010 },
    { 0x11143828, 0x18a }, { 0x11143824, 0x3460010 }, { 0x11143828, 0x18b }, { 0x11143824, 0x3460010 },
    { 0x11143828, 0x18b }, { 0x11143824, 0xc70010  }, { 0x11143828, 0x18c }, { 0x11143824, 0xc70010  },
    { 0x11143828, 0x18c }, { 0x11143824, 0x2c60010 }, { 0x11143828, 0x18d }, { 0x11143824, 0x2c60010 },
    { 0x11143828, 0x18d }, { 0x11143824, 0x1c60010 }, { 0x11143828, 0x18e }, { 0x11143824, 0x1c60010 },
    { 0x11143828, 0x18e }, { 0x11143824, 0x3840010 }, { 0x11143828, 0x18f }, { 0x11143824, 0x7b0010  },
    { 0x11143828, 0x18f }, { 0x11143824, 0x260010  }, { 0x11143828, 0x190 }, { 0x11143824, 0x3d90010 },
    { 0x11143828, 0x190 }, { 0x11143824, 0x2270010 }, { 0x11143828, 0x191 }, { 0x11143824, 0x2270010 },
    { 0x11143828, 0x191 }, { 0x11143824, 0x1270010 }, { 0x11143828, 0x192 }, { 0x11143824, 0x1270010 },
    { 0x11143828, 0x192 }, { 0x11143824, 0x3260010 }, { 0x11143828, 0x193 }, { 0x11143824, 0x3260010 },
    { 0x11143828, 0x193 }, { 0x11143824, 0xa70010  }, { 0x11143828, 0x194 }, { 0x11143824, 0xa70010  },
    { 0x11143828, 0x194 }, { 0x11143824, 0x2a60010 }, { 0x11143828, 0x195 }, { 0x11143824, 0x2a60010 },
    { 0x11143828, 0x195 }, { 0x11143824, 0x1a60010 }, { 0x11143828, 0x196 }, { 0x11143824, 0x1a60010 },
    { 0x11143828, 0x196 }, { 0x11143824, 0x590010  }, { 0x11143828, 0x197 }, { 0x11143824, 0x3a60010 },
    { 0x11143828, 0x197 }, { 0x11143824, 0x670010  }, { 0x11143828, 0x198 }, { 0x11143824, 0x670010  },
    { 0x11143828, 0x198 }, { 0x11143824, 0x2660010 }, { 0x11143828, 0x199 }, { 0x11143824, 0x2660010 },
    { 0x11143828, 0x199 }, { 0x11143824, 0x1660010 }, { 0x11143828, 0x19a }, { 0x11143824, 0x1660010 },
    { 0x11143828, 0x19a }, { 0x11143824, 0x990010  }, { 0x11143828, 0x19b }, { 0x11143824, 0x3660010 },
    { 0x11143828, 0x19b }, { 0x11143824, 0xe60010  }, { 0x11143828, 0x19c }, { 0x11143824, 0xe60010  },
    { 0x11143828, 0x19c }, { 0x11143824, 0x1190010 }, { 0x11143828, 0x19d }, { 0x11143824, 0x2e60010 },
    { 0x11143828, 0x19d }, { 0x11143824, 0x2190010 }, { 0x11143828, 0x19e }, { 0x11143824, 0x1e60010 },
    { 0x11143828, 0x19e }, { 0x11143824, 0xe40010  }, { 0x11143828, 0x19f }, { 0x11143824, 0x31b0010 },
    { 0x11143828, 0x19f }, { 0x11143824, 0x2540010 }, { 0x11143828, 0x1a0 }, { 0x11143824, 0x1ab0010 },
    { 0x11143828, 0x1a0 }, { 0x11143824, 0x2170010 }, { 0x11143828, 0x1a1 }, { 0x11143824, 0x2170010 },
    { 0x11143828, 0x1a1 }, { 0x11143824, 0x1170010 }, { 0x11143828, 0x1a2 }, { 0x11143824, 0x1170010 },
    { 0x11143828, 0x1a2 }, { 0x11143824, 0x3160010 }, { 0x11143828, 0x1a3 }, { 0x11143824, 0x3160010 },
    { 0x11143828, 0x1a3 }, { 0x11143824, 0x970010  }, { 0x11143828, 0x1a4 }, { 0x11143824, 0x970010  },
    { 0x11143828, 0x1a4 }, { 0x11143824, 0x2960010 }, { 0x11143828, 0x1a5 }, { 0x11143824, 0x2960010 },
    { 0x11143828, 0x1a5 }, { 0x11143824, 0x1960010 }, { 0x11143828, 0x1a6 }, { 0x11143824, 0x1960010 },
    { 0x11143828, 0x1a6 }, { 0x11143824, 0x690010  }, { 0x11143828, 0x1a7 }, { 0x11143824, 0x3960010 },
    { 0x11143828, 0x1a7 }, { 0x11143824, 0x570010  }, { 0x11143828, 0x1a8 }, { 0x11143824, 0x570010  },
    { 0x11143828, 0x1a8 }, { 0x11143824, 0x2560010 }, { 0x11143828, 0x1a9 }, { 0x11143824, 0x2560010 },
    { 0x11143828, 0x1a9 }, { 0x11143824, 0x1560010 }, { 0x11143828, 0x1aa }, { 0x11143824, 0x1560010 },
    { 0x11143828, 0x1aa }, { 0x11143824, 0xa90010  }, { 0x11143828, 0x1ab }, { 0x11143824, 0x3560010 },
    { 0x11143828, 0x1ab }, { 0x11143824, 0xd60010  }, { 0x11143828, 0x1ac }, { 0x11143824, 0xd60010  },
    { 0x11143828, 0x1ac }, { 0x11143824, 0x1290010 }, { 0x11143828, 0x1ad }, { 0x11143824, 0x2d60010 },
    { 0x11143828, 0x1ad }, { 0x11143824, 0x2290010 }, { 0x11143828, 0x1ae }, { 0x11143824, 0x1d60010 },
    { 0x11143828, 0x1ae }, { 0x11143824, 0x290010  }, { 0x11143828, 0x1af }, { 0x11143824, 0x3d60010 },
    { 0x11143828, 0x1af }, { 0x11143824, 0x740010  }, { 0x11143828, 0x1b0 }, { 0x11143824, 0x38b0010 },
    { 0x11143828, 0x1b0 }, { 0x11143824, 0x2360010 }, { 0x11143828, 0x1b1 }, { 0x11143824, 0x2360010 },
    { 0x11143828, 0x1b1 }, { 0x11143824, 0x1360010 }, { 0x11143828, 0x1b2 }, { 0x11143824, 0x1360010 },
    { 0x11143828, 0x1b2 }, { 0x11143824, 0xc90010  }, { 0x11143828, 0x1b3 }, { 0x11143824, 0x3360010 },
    { 0x11143828, 0x1b3 }, { 0x11143824, 0xb60010  }, { 0x11143828, 0x1b4 }, { 0x11143824, 0xb60010  },
    { 0x11143828, 0x1b4 }, { 0x11143824, 0x1490010 }, { 0x11143828, 0x1b5 }, { 0x11143824, 0x2b60010 },
    { 0x11143828, 0x1b5 }, { 0x11143824, 0x2490010 }, { 0x11143828, 0x1b6 }, { 0x11143824, 0x1b60010 },
    { 0x11143828, 0x1b6 }, { 0x11143824, 0x490010  }, { 0x11143828, 0x1b7 }, { 0x11143824, 0x3b60010 },
    { 0x11143828, 0x1b7 }, { 0x11143824, 0x760010  }, { 0x11143828, 0x1b8 }, { 0x11143824, 0x760010  },
    { 0x11143828, 0x1b8 }, { 0x11143824, 0x1890010 }, { 0x11143828, 0x1b9 }, { 0x11143824, 0x2760010 },
    { 0x11143828, 0x1b9 }, { 0x11143824, 0x2890010 }, { 0x11143828, 0x1ba }, { 0x11143824, 0x1760010 },
    { 0x11143828, 0x1ba }, { 0x11143824, 0x890010  }, { 0x11143828, 0x1bb }, { 0x11143824, 0x3760010 },
    { 0x11143828, 0x1bb }, { 0x11143824, 0x3090010 }, { 0x11143828, 0x1bc }, { 0x11143824, 0xf60010  },
    { 0x11143828, 0x1bc }, { 0x11143824, 0x1090010 }, { 0x11143828, 0x1bd }, { 0x11143824, 0x2f60010 },
    { 0x11143828, 0x1bd }, { 0x11143824, 0x2090010 }, { 0x11143828, 0x1be }, { 0x11143824, 0x1f60010 },
    { 0x11143828, 0x1be }, { 0x11143824, 0x1c40010 }, { 0x11143828, 0x1bf }, { 0x11143824, 0x23b0010 },
    { 0x11143828, 0x1bf }, { 0x11143824, 0x6a0010  }, { 0x11143828, 0x1c0 }, { 0x11143824, 0x3950010 },
    { 0x11143828, 0x1c0 }, { 0x11143824, 0x20e0010 }, { 0x11143828, 0x1c1 }, { 0x11143824, 0x1f10010 },
    { 0x11143828, 0x1c1 }, { 0x11143824, 0x10e0010 }, { 0x11143828, 0x1c2 }, { 0x11143824, 0x2f10010 },
    { 0x11143828, 0x1c2 }, { 0x11143824, 0x30e0010 }, { 0x11143828, 0x1c3 }, { 0x11143824, 0x30e0010 },
    { 0x11143828, 0x1c3 }, { 0x11143824, 0x8e0010  }, { 0x11143828, 0x1c4 }, { 0x11143824, 0x3710010 },
    { 0x11143828, 0x1c4 }, { 0x11143824, 0x28e0010 }, { 0x11143828, 0x1c5 }, { 0x11143824, 0x28e0010 },
    { 0x11143828, 0x1c5 }, { 0x11143824, 0x18e0010 }, { 0x11143828, 0x1c6 }, { 0x11143824, 0x18e0010 },
    { 0x11143828, 0x1c6 }, { 0x11143824, 0x710010  }, { 0x11143828, 0x1c7 }, { 0x11143824, 0x38e0010 },
    { 0x11143828, 0x1c7 }, { 0x11143824, 0x4e0010  }, { 0x11143828, 0x1c8 }, { 0x11143824, 0x3b10010 },
    { 0x11143828, 0x1c8 }, { 0x11143824, 0x24e0010 }, { 0x11143828, 0x1c9 }, { 0x11143824, 0x24e0010 },
    { 0x11143828, 0x1c9 }, { 0x11143824, 0x14e0010 }, { 0x11143828, 0x1ca }, { 0x11143824, 0x14e0010 },
    { 0x11143828, 0x1ca }, { 0x11143824, 0xb10010  }, { 0x11143828, 0x1cb }, { 0x11143824, 0x34e0010 },
    { 0x11143828, 0x1cb }, { 0x11143824, 0xce0010  }, { 0x11143828, 0x1cc }, { 0x11143824, 0xce0010  },
    { 0x11143828, 0x1cc }, { 0x11143824, 0x1310010 }, { 0x11143828, 0x1cd }, { 0x11143824, 0x2ce0010 },
    { 0x11143828, 0x1cd }, { 0x11143824, 0x2310010 }, { 0x11143828, 0x1ce }, { 0x11143824, 0x1ce0010 },
    { 0x11143828, 0x1ce }, { 0x11143824, 0x310010  }, { 0x11143828, 0x1cf }, { 0x11143824, 0x3ce0010 },
    { 0x11143828, 0x1cf }, { 0x11143824, 0x6c0010  }, { 0x11143828, 0x1d0 }, { 0x11143824, 0x3930010 },
    { 0x11143828, 0x1d0 }, { 0x11143824, 0x22e0010 }, { 0x11143828, 0x1d1 }, { 0x11143824, 0x22e0010 },
    { 0x11143828, 0x1d1 }, { 0x11143824, 0x12e0010 }, { 0x11143828, 0x1d2 }, { 0x11143824, 0x12e0010 },
    { 0x11143828, 0x1d2 }, { 0x11143824, 0xd10010  }, { 0x11143828, 0x1d3 }, { 0x11143824, 0x32e0010 },
    { 0x11143828, 0x1d3 }, { 0x11143824, 0xae0010  }, { 0x11143828, 0x1d4 }, { 0x11143824, 0xae0010  },
    { 0x11143828, 0x1d4 }, { 0x11143824, 0x1510010 }, { 0x11143828, 0x1d5 }, { 0x11143824, 0x2ae0010 },
    { 0x11143828, 0x1d5 }, { 0x11143824, 0x2510010 }, { 0x11143828, 0x1d6 }, { 0x11143824, 0x1ae0010 },
    { 0x11143828, 0x1d6 }, { 0x11143824, 0x510010  }, { 0x11143828, 0x1d7 }, { 0x11143824, 0x3ae0010 },
    { 0x11143828, 0x1d7 }, { 0x11143824, 0x6e0010  }, { 0x11143828, 0x1d8 }, { 0x11143824, 0x6e0010  },
    { 0x11143828, 0x1d8 }, { 0x11143824, 0x1910010 }, { 0x11143828, 0x1d9 }, { 0x11143824, 0x26e0010 },
    { 0x11143828, 0x1d9 }, { 0x11143824, 0x2910010 }, { 0x11143828, 0x1da }, { 0x11143824, 0x16e0010 },
    { 0x11143828, 0x1da }, { 0x11143824, 0x910010  }, { 0x11143828, 0x1db }, { 0x11143824, 0x36e0010 },
    { 0x11143828, 0x1db }, { 0x11143824, 0x3110010 }, { 0x11143828, 0x1dc }, { 0x11143824, 0xee0010  },
    { 0x11143828, 0x1dc }, { 0x11143824, 0x1110010 }, { 0x11143828, 0x1dd }, { 0x11143824, 0x2ee0010 },
    { 0x11143828, 0x1dd }, { 0x11143824, 0x2110010 }, { 0x11143828, 0x1de }, { 0x11143824, 0x1ee0010 },
    { 0x11143828, 0x1de }, { 0x11143824, 0x2c80010 }, { 0x11143828, 0x1df }, { 0x11143824, 0x1370010 },
    { 0x11143828, 0x1df }, { 0x11143824, 0x2380010 }, { 0x11143828, 0x1e0 }, { 0x11143824, 0x1c70010 },
    { 0x11143828, 0x1e0 }, { 0x11143824, 0x21e0010 }, { 0x11143828, 0x1e1 }, { 0x11143824, 0x21e0010 },
    { 0x11143828, 0x1e1 }, { 0x11143824, 0x11e0010 }, { 0x11143828, 0x1e2 }, { 0x11143824, 0x11e0010 },
    { 0x11143828, 0x1e2 }, { 0x11143824, 0xe10010  }, { 0x11143828, 0x1e3 }, { 0x11143824, 0x31e0010 },
    { 0x11143828, 0x1e3 }, { 0x11143824, 0x9e0010  }, { 0x11143828, 0x1e4 }, { 0x11143824, 0x9e0010  },
    { 0x11143828, 0x1e4 }, { 0x11143824, 0x1610010 }, { 0x11143828, 0x1e5 }, { 0x11143824, 0x29e0010 },
    { 0x11143828, 0x1e5 }, { 0x11143824, 0x2610010 }, { 0x11143828, 0x1e6 }, { 0x11143824, 0x19e0010 },
    { 0x11143828, 0x1e6 }, { 0x11143824, 0x610010  }, { 0x11143828, 0x1e7 }, { 0x11143824, 0x39e0010 },
    { 0x11143828, 0x1e7 }, { 0x11143824, 0x5e0010  }, { 0x11143828, 0x1e8 }, { 0x11143824, 0x5e0010  },
    { 0x11143828, 0x1e8 }, { 0x11143824, 0x1a10010 }, { 0x11143828, 0x1e9 }, { 0x11143824, 0x25e0010 },
    { 0x11143828, 0x1e9 }, { 0x11143824, 0x2a10010 }, { 0x11143828, 0x1ea }, { 0x11143824, 0x15e0010 },
    { 0x11143828, 0x1ea }, { 0x11143824, 0xa10010  }, { 0x11143828, 0x1eb }, { 0x11143824, 0x35e0010 },
    { 0x11143828, 0x1eb }, { 0x11143824, 0x3210010 }, { 0x11143828, 0x1ec }, { 0x11143824, 0xde0010  },
    { 0x11143828, 0x1ec }, { 0x11143824, 0x1210010 }, { 0x11143828, 0x1ed }, { 0x11143824, 0x2de0010 },
    { 0x11143828, 0x1ed }, { 0x11143824, 0x2210010 }, { 0x11143828, 0x1ee }, { 0x11143824, 0x1de0010 },
    { 0x11143828, 0x1ee }, { 0x11143824, 0xd40010  }, { 0x11143828, 0x1ef }, { 0x11143824, 0x32b0010 },
    { 0x11143828, 0x1ef }, { 0x11143824, 0x3e0010  }, { 0x11143828, 0x1f0 }, { 0x11143824, 0x3c10010 },
    { 0x11143828, 0x1f0 }, { 0x11143824, 0x1c10010 }, { 0x11143828, 0x1f1 }, { 0x11143824, 0x23e0010 },
    { 0x11143828, 0x1f1 }, { 0x11143824, 0x2c10010 }, { 0x11143828, 0x1f2 }, { 0x11143824, 0x13e0010 },
    { 0x11143828, 0x1f2 }, { 0x11143824, 0xc10010  }, { 0x11143828, 0x1f3 }, { 0x11143824, 0x33e0010 },
    { 0x11143828, 0x1f3 }, { 0x11143824, 0x3410010 }, { 0x11143828, 0x1f4 }, { 0x11143824, 0xbe0010  },
    { 0x11143828, 0x1f4 }, { 0x11143824, 0x1410010 }, { 0x11143828, 0x1f5 }, { 0x11143824, 0x2be0010 },
    { 0x11143828, 0x1f5 }, { 0x11143824, 0x2410010 }, { 0x11143828, 0x1f6 }, { 0x11143824, 0x1be0010 },
    { 0x11143828, 0x1f6 }, { 0x11143824, 0x12a0010 }, { 0x11143828, 0x1f7 }, { 0x11143824, 0x2d50010 },
    { 0x11143828, 0x1f7 }, { 0x11143824, 0x3810010 }, { 0x11143828, 0x1f8 }, { 0x11143824, 0x7e0010  },
    { 0x11143828, 0x1f8 }, { 0x11143824, 0x1810010 }, { 0x11143828, 0x1f9 }, { 0x11143824, 0x27e0010 },
    { 0x11143828, 0x1f9 }, { 0x11143824, 0x2810010 }, { 0x11143828, 0x1fa }, { 0x11143824, 0x17e0010 },
    { 0x11143828, 0x1fa }, { 0x11143824, 0x1320010 }, { 0x11143828, 0x1fb }, { 0x11143824, 0x2cd0010 },
    { 0x11143828, 0x1fb }, { 0x11143824, 0xe80010  }, { 0x11143828, 0x1fc }, { 0x11143824, 0x3170010 },
    { 0x11143828, 0x1fc }, { 0x11143824, 0xb20010  }, { 0x11143828, 0x1fd }, { 0x11143824, 0x34d0010 },
    { 0x11143828, 0x1fd }, { 0x11143824, 0xb40010  }, { 0x11143828, 0x1fe }, { 0x11143824, 0x34b0010 },
    { 0x11143828, 0x1fe }, { 0x11143824, 0x1680010 }, { 0x11143828, 0x1ff }, { 0x11143824, 0x2970010 },
    { 0x11143828, 0x1ff }, { 0x11143820, 0x3       }, { 0x11143820, 0x3   }
};

hi_void frl_rx_lm_table_set(hi_void)
{
    hi_u32 i;
    frl_rx_crc_check *rc_table = HI_NULL;

    rc_table = &g_rx_table[0];
    /* 4L-0x01, 3L-0x00 */
    for (i = 0; i < hdmi_array_size(g_rx_table); i++, rc_table++) {
        hdmi_reg_write_u32(rc_table->addr, rc_table->value);
    }

    return;
}

static hi_void debug_frl_attr_help(hi_void)
{
    hdmi_dbg_print("[usage  ]: echo frlinfo argv1 argv2 argv3 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: FRL debug command.\n");
    hdmi_dbg_print("           0\t--FRL rate strategy   [argv2: 0--little, 1--big]\n"
                   "           1\t--HDMI2.1 strategy    [argv2: 0--(>600M), 1--(>340M), 2--(p-format)])\n");
    hdmi_dbg_print("           2\t--max train fail times[argv2: 1 < times <= 5000]\n");
    hdmi_dbg_print("           3\t--train exception bp  [argv2: b0(readychk) b1(resultchk) b2(retrain)]\n");
    hdmi_dbg_print("           4\t--FRL fail strategy   [argv2: [0-none, 1-TMDS, 2-notify]\n");
    hdmi_dbg_print("           5\t--FRL machine mode    [argv2: 0-step, 1-timeout]\n");
    hdmi_dbg_print("           6\t--FRL flt outtime     [argv2: 0-time(ms)]\n");

    return;
}

static hi_s32 frl_attr_parse(hdmi_frl_info *frl_info, const char *cmd,  hi_u32 data)
{
    if (is_str_equal(cmd, "0")) {
        if (data >= HDMI_FRL_RATE_SELECT_BUTT) {
            return HI_FAILURE;
        }
        frl_info->rate_select = (hdmi_frl_rate_select)data;
        hdmi_dbg_print("set FRL rete strategy %s.\n", (data == HDMI_FRL_RATE_SELECT_BIG) ? "BIG" : "LITTLE");
    } else if (is_str_equal(cmd, "1")) {
        if (data >= HDMI_FRL_STRATEGY_MODE_BUTT) {
            return HI_FAILURE;
        }
        hdmi_dbg_print("set 2.1 strategy old(%u)->new(%u).\n", frl_info->strategy_mode, data);
        frl_info->strategy_mode = (hdmi_frl_strategy_mode)data;
    } else if (is_str_equal(cmd, "2")) {
        if (data == 0 || data > DEBUG_MAX_DATA_SIZE) {
            return HI_FAILURE;
        }
        hdmi_dbg_print("set max training fail times: old(%u)->new(%u).\n", frl_info->max_fail_times, data);
        frl_info->max_fail_times = data;
    } else if (is_str_equal(cmd, "3")) {
        frl_info->bypass = (data & 0x7);
        hdmi_dbg_print("bypass: 0x%x.\n", frl_info->bypass);
    } else if (is_str_equal(cmd, "4")) {
        data = (data >= HDMI_FRL_FAIL_BUTT) ? HDMI_FRL_FAIL_BUTT : data;
        hdmi_dbg_print("FRL fail strategy old(%u)->new(%u).\n", frl_info->fail_strategy, data);
        frl_info->fail_strategy = data;
    } else if (is_str_equal(cmd, "5")) {
        data = (data >= HDMI_FRL_MACH_MODE_TIMEOUT) ? HDMI_FRL_MACH_MODE_TIMEOUT : data;
        hdmi_dbg_print("FRL mach change old(%u)->new(%u).\n", frl_info->state_mach_info.mach_mode, data);
        frl_info->state_mach_info.mach_mode = data;
    } else if (is_str_equal(cmd, "6")) {
        data = (data >= 1000) ? 1000 : data; /* cmd argv[1] is 14, restrict data size to 1000 */
        hdmi_dbg_print("FRL link training timeout old(%llu)->new(%llu).\n",
            frl_info->state_mach_info.train_timeout, data);
        frl_info->state_mach_info.train_timeout = data;
    } else {
        hdmi_dbg_err("error argvs!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_s32 debug_frl_attr(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 data;
    hi_s32 ret;
    hdmi_device   *hdmi_dev = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);
    frl_info = &hdmi_dev->frl_info;

    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto info_error;
    }
    debug_null_check_return(cmd_arg->argv[2]);
    data = debug_str_to_u32(cmd_arg->argv[2], &cmd_arg->argv[2], HDMI_DEBUG_BASE_DEC);
    ret = frl_attr_parse(frl_info, cmd_arg->argv[1], data);
    if (ret != HI_SUCCESS) {
        goto info_error;
    }

    return HI_SUCCESS;

info_error:
    debug_frl_attr_help();

    return HI_FAILURE;
}

static hi_void debug_frl_cfg_help(hi_void)
{
    hdmi_dbg_print("[usage  ]: echo frlcfg argv1 argv2 argv3 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: FRL debug command.\n");
    hdmi_dbg_print("          0\t--LM table get        [argv2: 0--unused]\n");
    hdmi_dbg_print("          1\t--FRL training break  [argv2: 0-hold, 1-break]\n");
    hdmi_dbg_print("          2\t--FRL sw train mode   [argv2: 0--delay, 1-timer]\n");
    hdmi_dbg_print("          3\t--train delay interval[argv2: time(ms)]\n");
    hdmi_dbg_print("          4\t--training pattern set[argv2: lane_id(0~3) argv3:lt_px(1~8)]\n");
    hdmi_dbg_print("          5\t--set LTS3 timeout    [argv2: timeout interval.(1ms default).\n");
    hdmi_dbg_print("          6\t--set control compress [argv2: 0-all, 3-none, 1/2-hw.\n");

    return;
}

static hi_s32 frl_cfg_parse(hdmi_device *hdmi_dev, hdmi_debug_cmd_arg *cmd_arg, hi_u32 data)
{
    hi_char *str = HI_NULL;
    frl_debug frl_cfg = {0};

    if (is_str_equal(cmd_arg->argv[1], "0")) {
        frl_cfg.debug_cmd = FRL_DEBUG_LM_TABLE_GET;
        hdmi_dbg_print("get LM table success.\n");
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        hdmi_dbg_print("FRL training break(%u).\n", data);
        frl_cfg.training_break = (hi_u8)data;
        frl_cfg.debug_cmd = FRL_DEBUG_TRAINING_BREAK;
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        frl_cfg.sw_train_mode = (data == 0) ? FRL_SW_TRAIN_DELAY : FRL_SW_TRAIN_TIMER;
        hdmi_dbg_print("set SW training mode to %s.\n", (data == 0) ? "delay" : "timer");
        frl_cfg.debug_cmd = FRL_DEBUG_SW_TRAIN_SEL;
        hdmi_dev->frl_info.state_mach_info.sw_train_mode = frl_cfg.sw_train_mode;
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
        data = (data >= 1000) ? 1000 : data; /* cmd argv[1] is 17, restrict interval time to 1000ms */
        hdmi_dbg_print("set LTS3 interval %ums.\n", data);
        frl_cfg.lts3_interval = data;
        frl_cfg.debug_cmd = FRL_DEBUG_LTS3_INTERVAL;
    }  else if (is_str_equal(cmd_arg->argv[1], "4")) {
        frl_cfg.lane_idx = (data > 3) ? 3 : data; /* cmd argv[1] is 19, restrict lane_idx data size to 3 */
        hdmi_dbg_print("lane_idx: %u\n", frl_cfg.lane_idx);
        str = cmd_arg->argv[3];
        debug_null_check_return(str);
        data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
        frl_cfg.ltp = (data > 8) ? 8 : data; /* cmd argv[1] is 19, restrict lane_idx data size to 8 */
        frl_cfg.debug_cmd = FRL_DEBUG_LTP_PATTERN;
        hdmi_dbg_print("LTP: %u\n", frl_cfg.ltp);
    } else if (is_str_equal(cmd_arg->argv[1], "5")) {
        data = (data >= 1000) ? 1000 : data; /* cmd argv[1] is 22, restrict LTS3 timeout data size to 1000 */
        hdmi_dbg_print("set LTS3 timeout %u.\n", data);
        frl_cfg.lts3_timeout = data;
        frl_cfg.debug_cmd = FRL_DEBUG_LTS3_TIMEOUT;
    } else if (is_str_equal(cmd_arg->argv[1], "6")) {
        data = (data > DEBUG_FRL_CTRL_TYPE_MAX) ? DEBUG_FRL_CTRL_TYPE_MAX : data;
        frl_cfg.crtl_type_config = data;
        frl_cfg.debug_cmd = FRL_DEBUG_CTRL_TYPE_CONFIG;
        hdmi_dbg_print("set RC to %u.\n", data);
    } else {
        hdmi_dbg_err("error argvs!\n");
        return HI_FAILURE;
    }

    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &frl_cfg);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_cfg(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 data;
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    hi_char *str = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto cfg_error;
    }
    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    ret = frl_cfg_parse(hdmi_dev, cmd_arg, data);
    if (ret != HI_SUCCESS) {
        goto cfg_error;
    }

    return HI_SUCCESS;

cfg_error:
    debug_frl_cfg_help();

    return HI_FAILURE;
}

static hi_void debug_frl_mach_help(hi_void)
{
    hdmi_dbg_print("[usage  ]: echo frlmach argv1 argv2 argv3 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: FRL debug command.\n");
    hdmi_dbg_print("           0\t--train machine start [argv2: 0--stop, other--start].\n");
    hdmi_dbg_print("           1\t--wait handle time    [argv2: 1 < time <= 5000]\n"
                   "           2\t--wait ready time     [argv2: 1 < time <= 5000]\n"
                   "           3\t--wait retrain time   [argv2: 1 < time <= 5000]\n");

    return;
}

static hi_s32 debug_frl_mach(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 data;
    hdmi_device              *hdmi_dev  = HI_NULL;
    hdmi_frl_info            *frl_info  = HI_NULL;
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    frl_info = &hdmi_dev->frl_info;
    mach_info = &frl_info->state_mach_info;

    debug_null_check_return(frl_info);
    debug_null_check_return(mach_info);

    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto mach_error;
    }
    debug_null_check_return(cmd_arg->argv[2]);
    data = debug_str_to_u32(cmd_arg->argv[2], &cmd_arg->argv[2], HDMI_DEBUG_BASE_DEC);
    if (is_str_equal(cmd_arg->argv[1], "0")) {
        hdmi_dbg_print("set FRL machine old(%s)->new(%s).\n",
            (mach_info->start_mach == HI_TRUE) ? "start" : "stop", (data == 0) ? "stop" : "start");
        mach_info->start_mach = (data == 0) ? HI_FALSE : HI_TRUE;
    } else if (is_str_equal(cmd_arg->argv[1], "1")) {
        if (data == 0 || data > DEBUG_MAX_DATA_SIZE) {
            goto mach_error;
        }
        hdmi_dbg_print("set wait handle time: old(%u)->new(%u).\n", mach_info->wait_handle_ms, data);
        mach_info->wait_handle_ms = data;
    } else if (is_str_equal(cmd_arg->argv[1], "2")) {
        if (data == 0 || data > DEBUG_MAX_DATA_SIZE) {
            goto mach_error;
        }
        hdmi_dbg_print("set wait ready time: old(%u)->new(%u).\n", mach_info->wait_ready_ms, data);
        mach_info->wait_ready_ms = data;
    } else if (is_str_equal(cmd_arg->argv[1], "3")) {
        if (data == 0 || data > DEBUG_MAX_DATA_SIZE) {
            goto mach_error;
        }
        hdmi_dbg_print("set wait retrain time: old(%u)->new(%u).\n", mach_info->wait_retrain_ms, data);
        mach_info->wait_retrain_ms = data;
    } else {
        hdmi_dbg_err("error argvs!\n");
        goto mach_error;
    }

mach_error:
    debug_frl_mach_help();

    return HI_FAILURE;
}

static hi_s32 debug_compat(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 data;
    hi_char            *str         = HI_NULL;
    hdmi_device        *hdmi_dev    = HI_NULL;
    compatibility_info *compat_info = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    compat_info = compat_info_get(hdmi_dev->hdmi_dev_id);
    debug_null_check_return(compat_info);

    if (cmd_arg->argc < DEBUG_CMD_ARGC_COUNT_2 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto avi_error;
    }
    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    hdmi_dbg_print("set AVI by gen5 old(%u)->new(%u).\n", compat_info->avi_use_gen5, data);
    compat_info->avi_use_gen5 = (data == 0) ? HI_FALSE : HI_TRUE;

avi_error:
    hdmi_dbg_print("[usage  ]: echo frlavi argv1 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: FRL debug command.\n");
    hdmi_dbg_print("           \t--set AVI send by gen5 [argv1: 0-avi, others-gen5.\n");

    return HI_FAILURE;
}

static hi_void debug_frl_help(hi_void)
{
    hdmi_dbg_print("[Usage  ]: echo frl argv1 argv2 argv3 > /proc/umap/hdmi0 \n");
    hdmi_dbg_print("[argv1  ]: FRL debug command.\n");
    hdmi_dbg_print("           0\t--Train machine start      [argv2: 0--stop, other--start].\n"
                   "           1\t--FRL rate strategy        [argv2: 0--little, 1--big]\n"
                   "           2\t--HDMI2.1 strategy         [argv2: 0--(>600M), 1--(>340M), 2--(p-format)])\n");
    hdmi_dbg_print("           3\t--Max train fail times     [argv2: 1 < times <= 5000]\n"
                   "           4\t--Wait handle time         [argv2: 1 < time <= 5000]\n"
                   "           5\t--Wait ready time          [argv2: 1 < time <= 5000]\n"
                   "           6\t--Wait retrain time        [argv2: 1 < time <= 5000]\n");
    hdmi_dbg_print("           7\t--Forcible FRL/TMDS        [argv2:(TMDS/FRL) argv3:(Rate)\n");
    hdmi_dbg_print("           8\t--Train exception bp       [argv2: b0(readychk) b1(resultchk) b2(retrain)]\n");
    hdmi_dbg_print("           9\t--Timer test               [argv2: 0--unused]\n");
    hdmi_dbg_print("          10\t--LM table get             [argv2: 0--unused]\n");
    hdmi_dbg_print("          11\t--FRL fail strategy        [argv2: [0-none, 1-TMDS, 2-notify]\n");
    hdmi_dbg_print("          12\t--Inner RX CRC check       [argv2: 0--unused]\n");
    hdmi_dbg_print("          13\t--FRL machine mode         [argv2: 0-step, 1-timeout]\n");
    hdmi_dbg_print("          14\t--FRL Flt outtime          [argv2: 0-time(ms)]\n");
    hdmi_dbg_print("          15\t--FRL Training break       [argv2: 0-hold, 1-break]\n");
    hdmi_dbg_print("          16\t--FRL sw train mode        [argv2: 0--delay, 1-timer]\n");
    hdmi_dbg_print("          17\t--Train delay interval     [argv2: time(ms)]\n");
    hdmi_dbg_print("          18\t--FRL rate set             [argv2: rate(maximum 6)]\n");
    hdmi_dbg_print("          19\t--Training Pattern set     [argv2: LaneId(0~3) argv3:LTPx(1~8)]\n");
    hdmi_dbg_print("          20\t--Select FRL debug channel [argv2: 0:FRLTX--PHY 1:FRLTX-FRLRX--TMDS\n");
    hdmi_dbg_print("          21\t--FFE level set            [argv2-4: lane0-3 FFE level(0~3)\n");
    hdmi_dbg_print("          22\t--Set LTS3 timeout         [argv2: Timeout interval.(1ms default).\n");
    hdmi_dbg_print("          23\t--Set AVI send by gen5     [argv2: 0-avi, 1-gen5, others-disable debug.\n");
    hdmi_dbg_print("          24\t--Set control compress     [argv2: 0-all, 3-none, 1/2-hw.\n");
    hdmi_dbg_print("          25\t--Set max frl rate         [argv2: 0 < rate <= 6.\n");

    return;
}

typedef hi_s32 (*debug_func)(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev);
typedef struct {
    hi_char cmd_index;
    debug_func cmd_func;
} frl_debug_func;

static hi_s32 debug_frl_machine(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;
    mach_info = &frl_info->state_mach_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    hdmi_dbg_print("set frl machine old(%s)->new(%s).\n",
        (mach_info->start_mach == HI_TRUE) ? "start" : "stop", (data == 0) ? "stop" : "start");

    mach_info->start_mach = (data == 0) ? HI_FALSE : HI_TRUE;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_rete_strategy(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data >= HDMI_FRL_RATE_SELECT_BUTT) {
        return HI_FAILURE;
    }
    frl_info->rate_select = (hdmi_frl_rate_select)data;
    hdmi_dbg_print("set frl rete strategy %s.\n", (data == HDMI_FRL_RATE_SELECT_BIG) ? "BIG" : "LITTLE");

    return HI_SUCCESS;
}

static hi_s32 debug_frl_strategy_mode(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data >= HDMI_FRL_STRATEGY_MODE_BUTT) {
        return HI_FAILURE;
    }

    hdmi_dbg_print("set 2.1 strategy old(%d)->new(%d).\n", frl_info->strategy_mode, data);
    frl_info->strategy_mode = (hdmi_frl_strategy_mode)data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_max_training_times(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data == 0 || data > DEBUG_MAX_FRL_TIME) {
        return HI_FAILURE;
    }

    hdmi_dbg_print("set max training fail times: old(%d)->new(%d).\n", frl_info->max_fail_times, data);
    frl_info->max_fail_times = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_wait_handle_time(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;
    mach_info = &frl_info->state_mach_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data >= DEBUG_MAX_FRL_TIME) {
        return HI_FAILURE;
    }

    hdmi_dbg_print("set wait handle time: old(%d)->new(%d).\n", mach_info->wait_handle_ms, data);
    mach_info->wait_handle_ms = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_wait_ready_time(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;
    mach_info = &frl_info->state_mach_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data == 0 || data > DEBUG_MAX_FRL_TIME) {
        return HI_FAILURE;
    }

    hdmi_dbg_print("set wait ready time: old(%d)->new(%d).\n", mach_info->wait_ready_ms, data);
    mach_info->wait_ready_ms = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_wait_retrain_time(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;
    mach_info = &frl_info->state_mach_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data == 0 || data > DEBUG_MAX_FRL_TIME) {
        return HI_FAILURE;
    }

    hdmi_dbg_print("set wait retrain time: old(%d)->new(%d).\n", mach_info->wait_retrain_ms, data);
    mach_info->wait_retrain_ms = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_mode(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_s32 ret;
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_debug debug = {0};

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    debug.hdmi_mode = (data == 1) ? HDMI_FRL_MODE_FRL : HDMI_FRL_MODE_TMDS;
    if (debug.hdmi_mode == HDMI_FRL_MODE_FRL) {
        if (cmd_arg->argc < DEBUG_NUM_3) {
            return HI_FAILURE;
        }

        str = cmd_arg->argv[3];
        debug_null_check_return(str);
        data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
        debug.rate = (data >= FRL_WORK_MODE_4L12G) ? FRL_WORK_MODE_4L12G : data;
    }

    ret = drv_hdmi_frl_mode_change(hdmi_dev->hdmi_dev_id, &debug);
    hdmi_dbg_print("frl debug: %s.\n", (ret == HI_SUCCESS) ? STR_SUCC : STR_FAIL);

    return ret;
}

static hi_s32 debug_frl_bypass(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    frl_info->bypass = (data & 0x7);
    hdmi_dbg_print("bypass: 0x%x.\n", frl_info->bypass);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_timer_test(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    timer_config config = {0};

    hi_unused(cmd_arg);
    hi_unused(hdmi_dev);
    g_timer_test = 0;
    config.start_time = hdmi_osal_get_time_in_ms();
    config.isr        = debug_timer_isr;
    config.delay_ms   = DEBUG_NUM_2;

    hal_timer_deinit();
    hal_timer_init(&config);
    hal_timer_start();
    osal_msleep(FRL_WAIT);
    hdmi_dbg_print("timer test: %s.\n", (g_timer_test > DEBUG_TIME_TEST_MAX) ? STR_SUCC : STR_FAIL);
    osal_msleep(FRL_WAIT);
    hal_timer_deinit();

    return HI_SUCCESS;
}

static hi_s32 debug_frl_get_lm_table(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    frl_debug debug = {0};

    hi_unused(cmd_arg);
    debug.debug_cmd = FRL_DEBUG_LM_TABLE_GET;
    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);
    hdmi_dbg_print("get lm table success.\n");

    return HI_SUCCESS;
}

static hi_s32 debug_frl_fail_strategy(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    data = (data >= HDMI_FRL_FAIL_BUTT) ? HDMI_FRL_FAIL_BUTT : data;
    hdmi_dbg_print("frl fail strategy old(%d)->new(%d).\n", frl_info->fail_strategy, data);
    frl_info->fail_strategy = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_crc_check(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 crc[2] = {0};

    hi_unused(cmd_arg);
    /* 4L-0x01, 3L-0x00 */
    hdmi_reg_write_u32(0x11143830, (hdmi_dev->frl_info.rate_info.cur_rate > FRL_WORK_MODE_3L6G) ? 0x01 : 0x00);
    frl_rx_lm_table_set();
    /* MSB */
    hdmi_reg_write_u32(0x11143204, 0x01010f0f); /* RX use MSB */
    osal_msleep(FRL_CRC_WAIT);
    crc[0] = hdmi_reg_read_u32(0x11143838);
    crc[1] = hdmi_reg_read_u32(0x11143180);
    /* LSB */
    hdmi_reg_write_u32(0x11143204, 0x81010f0f); /* TX use LSB */
    hdmi_dbg_print("CRC TX:0x%x, RX:0x%x\n", crc[1], crc[0]);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_mach_change(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    data = (data >= HDMI_FRL_MACH_MODE_TIMEOUT) ? HDMI_FRL_MACH_MODE_TIMEOUT : data;
    hdmi_dbg_print("frl mach change old(%d)->new(%d).\n",
        frl_info->state_mach_info.mach_mode, data);
    frl_info->state_mach_info.mach_mode = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_link_training_timeout(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    data = (data > DEBUG_THOUSAND) ? DEBUG_THOUSAND : data;
    hdmi_dbg_print("frl link training timeout old(%llu)->new(%llu).\n",
        frl_info->state_mach_info.train_timeout, data);
    frl_info->state_mach_info.train_timeout = data;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_training_break(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    frl_debug debug = {0};

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    hdmi_dbg_print("frl training break(%u).\n", data);
    debug.training_break = (hi_u8)data;
    debug.debug_cmd = FRL_DEBUG_TRAINING_BREAK;
    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_sw_training_mode(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    frl_debug debug = {0};
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;

    frl_info = &hdmi_dev->frl_info;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    debug.sw_train_mode = (data == 0) ? FRL_SW_TRAIN_DELAY : FRL_SW_TRAIN_TIMER;
    hdmi_dbg_print("set sw training mode to %s.\n", (data == 0) ? "delay" : "timer");
    debug.debug_cmd = FRL_DEBUG_SW_TRAIN_SEL;
    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);
    frl_info->state_mach_info.sw_train_mode = debug.sw_train_mode;

    return HI_SUCCESS;
}

static hi_s32 debug_frl_lts3_interval(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    frl_debug debug = {0};

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    hdmi_dbg_print("set lts3 interval %ums.\n", data);
    debug.lts3_interval = data;
    debug.debug_cmd = FRL_DEBUG_LTS3_INTERVAL;
    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_training_rate(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data > FRL_WORK_MODE_4L12G) {
        hdmi_dbg_err("error argcs!\n");
        return HI_FAILURE;
    }
    hdmi_dbg_print("training rate old(%u)->new(%u).\n",
        hdmi_dev->frl_info.rate_info.cur_rate, data);
    hdmi_dev->frl_info.rate_info.debug_rate = data;
    drv_hdmi_stop(hdmi_dev);
    drv_hdmi_vo_attr_set(hdmi_dev, &hdmi_dev->attr.vo_attr);
    drv_hdmi_start(hdmi_dev);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_ltp_pattern(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    frl_debug debug = {0};

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    debug.lane_idx = (data > DEBUG_NUM_3) ? DEBUG_NUM_3 : data;
    hdmi_dbg_print("lane_idx: %d\n", debug.lane_idx);
    if (cmd_arg->argc < DEBUG_NUM_3) {
        return HI_FAILURE;
    }

    str = cmd_arg->argv[3];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    debug.ltp = (data > DEBUG_NUM_8) ? DEBUG_NUM_8 : data;
    debug.debug_cmd = FRL_DEBUG_LTP_PATTERN;
    hdmi_dbg_print("LTP: %d\n", debug.ltp);

    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_channel_sel(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    frl_debug debug = {0};
    hdmi_frl_debug frl_cfg = {0};
    hi_char *str = HI_NULL;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    hdmi_dbg_print("select FRL CHANNEL %s.\n",
        (data == 0) ? "FRLTX--PHY" : "FRLTX-FRLRX--TMDS");
    debug.channel_sel = (data == 0) ? FRL_CHL_SEL_NORMAL : FRL_CHL_SEL_RX_TMDS;
    if (debug.channel_sel == FRL_CHL_SEL_RX_TMDS) {
        hal_call_void(hal_hdmi_frl_enable, hdmi_dev->hal, HI_FALSE);
        debug.rate = hdmi_dev->frl_info.rate_info.cur_rate;
        debug.debug_cmd = FRL_DEBUG_SELECT_CHANNEL;
        hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);
        frl_rx_lm_table_set();
        frl_cfg.hdmi_mode = HDMI_FRL_MODE_TMDS;
        drv_hdmi_frl_mode_change(cmd_arg->hdmi_id, &frl_cfg);

        debug.debug_cmd = FRL_DEBUG_RATE;
        hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);
        hal_call_void(hal_hdmi_frl_enable, hdmi_dev->hal, HI_TRUE);
    } else {
        hdmi_dev->frl_info.mode = HDMI_FRL_MODE_FRL;
        debug.debug_cmd = FRL_DEBUG_SELECT_CHANNEL;
        hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);
        drv_hdmi_stop(hdmi_dev);
        drv_hdmi_start(hdmi_dev);
    }

    return HI_SUCCESS;
}

static hi_s32 debug_frl_ffe(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    hdmi_phy_cfg phy_cfg = {0};

    if (cmd_arg->argc < DEBUG_NUM_5) {
        return HI_FAILURE;
    }

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);

    phy_cfg.aen_tx_ffe[0] = data;
    str = cmd_arg->argv[3];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    phy_cfg.aen_tx_ffe[1] = data;
    str = cmd_arg->argv[4];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    phy_cfg.aen_tx_ffe[2] = data;
    str = cmd_arg->argv[5];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    phy_cfg.aen_tx_ffe[3] = data;

    if (phy_cfg.aen_tx_ffe[0] > FRL_TXFFE_MODE_3 ||
        phy_cfg.aen_tx_ffe[1] > FRL_TXFFE_MODE_3 ||
        phy_cfg.aen_tx_ffe[2] > FRL_TXFFE_MODE_3 ||
        phy_cfg.aen_tx_ffe[3] > FRL_TXFFE_MODE_3) {
        return HI_FAILURE;
    }

    hdmi_dbg_print("FFE lane0:%d lane1:%d lane2:%d lane3:%d\n",
        phy_cfg.aen_tx_ffe[0], phy_cfg.aen_tx_ffe[1], phy_cfg.aen_tx_ffe[2], phy_cfg.aen_tx_ffe[3]);
    phy_cfg.rate = hdmi_dev->frl_info.rate_info.cur_rate;
    phy_cfg.mode_cfg = HDMI_PHY_MODE_CFG_TXFFE;
    hal_call_void(hal_hdmi_phy_set, hdmi_dev->hal, &phy_cfg);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_lts3_timeout(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    frl_debug debug = {0};
    hi_char *str = HI_NULL;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    hdmi_dbg_print("set LTS3 timeout %u.\n", data);
    debug.lts3_timeout = data;
    debug.debug_cmd = FRL_DEBUG_LTS3_TIMEOUT;
    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_avi_debug(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_char *str = HI_NULL;
    compatibility_info *compat_info = HI_NULL;

    compat_info = compat_info_get(hdmi_dev->hdmi_dev_id);
    debug_null_check_return(compat_info);

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data == 0) {
        compat_info->avi_use_gen5_debug = HI_TRUE;
        compat_info->avi_use_gen5 = HI_FALSE;
    } else if (data == 1) {
        compat_info->avi_use_gen5_debug = HI_TRUE;
        compat_info->avi_use_gen5 = HI_TRUE;
    } else {
        compat_info->avi_use_gen5_debug = HI_FALSE;
    }

    hdmi_dbg_print("set AVI debug(%d), use(%d).\n",
        compat_info->avi_use_gen5_debug, compat_info->avi_use_gen5);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_rc(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    frl_debug debug = {0};
    hi_char *str = HI_NULL;
    compatibility_info *compat_info = HI_NULL;

    compat_info = compat_info_get(hdmi_dev->hdmi_dev_id);
    debug_null_check_return(compat_info);

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data > DEBUG_RC_MAX) {
        data = DEBUG_RC_MAX;
    }
    hdmi_dbg_print("set rc to %d.\n", data);

    if (data == DEBUG_RC_MAX) { /* set default cfg. */
        compat_info->ctl_type_config = FRL_CTRL_TYPE_COMPRESS_HW;
    } else {
        compat_info->ctl_type_config = data | HDMI_FRL_COMPRESS_DEBUG_MASK;
    }

    debug.crtl_type_config = compat_info->ctl_type_config;
    debug.debug_cmd = FRL_DEBUG_CTRL_TYPE_CONFIG;
    hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug);

    return HI_SUCCESS;
}

static hi_s32 debug_frl_max_rate(hdmi_debug_cmd_arg *cmd_arg, hdmi_device *hdmi_dev)
{
    hi_u32 data;
    hi_u8 old_max_rate;
    hi_char *str = HI_NULL;
    hdmi_frl_info *frl_info = HI_NULL;
    hdmi_tx_capability_data tx_cap = {0};

    hal_call_void(hal_hdmi_tx_capability_get, hdmi_dev->hal, &tx_cap);

    frl_info = &hdmi_dev->frl_info;
    old_max_rate = frl_info->rate_info.max_rate;

    str = cmd_arg->argv[2];
    debug_null_check_return(str);
    data = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    if (data > FRL_WORK_MODE_4L12G) {
        hdmi_dbg_print("err! exceed the tx_max_frl_rate. force to tx max rate.\n");
        data = FRL_WORK_MODE_4L12G;
    }
    hdmi_dbg_print("set max rate: %d->%d.\n", old_max_rate, data);

    frl_info->rate_info.max_rate = data;

    return HI_SUCCESS;
}

static frl_debug_func g_frl_debug[] = {
    {0, debug_frl_machine},
    {1, debug_frl_rete_strategy},
    {2, debug_frl_strategy_mode},
    {3, debug_frl_max_training_times},
    {4, debug_frl_wait_handle_time},
    {5, debug_frl_wait_ready_time},
    {6, debug_frl_wait_retrain_time},
    {7, debug_frl_mode},
    {8, debug_frl_bypass},
    {9, debug_frl_timer_test},
    {10, debug_frl_get_lm_table},
    {11, debug_frl_fail_strategy},
    {12, debug_frl_crc_check},
    {13, debug_frl_mach_change},
    {14, debug_frl_link_training_timeout},
    {15, debug_frl_training_break},
    {16, debug_frl_sw_training_mode},
    {17, debug_frl_lts3_interval},
    {18, debug_frl_training_rate},
    {19, debug_frl_ltp_pattern},
    {20, debug_frl_channel_sel},
    {21, debug_frl_ffe},
    {22, debug_frl_lts3_timeout},
    {23, debug_frl_avi_debug},
    {24, debug_frl_rc},
    {25, debug_frl_max_rate}
};

static hi_s32 debug_frl(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 i;
    hi_s32 cmd_index;
    hi_s32 ret = HI_FAILURE;
    hi_char *str = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;

    debug_null_check_return(cmd_arg);
    hdmi_dev = get_hdmi_device(cmd_arg->hdmi_id);
    debug_null_check_return(hdmi_dev);

    if (cmd_arg->argc < DEBUG_NUM_3 || cmd_arg->argv[1] == HI_NULL) {
        hdmi_dbg_err("error argcs!\n");
        goto usage_error;
    }

    str = cmd_arg->argv[1];
    cmd_index = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
    for (i = 0; i < hdmi_array_size(g_frl_debug); i++) {
        if (cmd_index == g_frl_debug[i].cmd_index) {
            ret = g_frl_debug[i].cmd_func(cmd_arg, hdmi_dev);
            break;
        }
    }

    if (ret != HI_SUCCESS || i >= hdmi_array_size(g_frl_debug)) {
        goto usage_error;
    }

    return HI_SUCCESS;

usage_error:
    debug_frl_help();

    return HI_FAILURE;
}
#endif

static const hdmi_debug_cmd_info g_cmd_table[] = {
    /* name     short name    func                       help info */
    { "help",         "h", debug_print_help,      "print these help info." },
    { "thread",       "t", debug_thread,          "thread status set on/off." },
    { "oe",           "o", debug_oe,              "phyoutput enable(oe) set on/off." },
    { "event",        "e", debug_event,           "hardware event [16,25] simulate." },
    { "edid",        "ed", debug_edid,            "edid raw data load." },
    { "avmute",       "a", debug_avmute,          "audio & video mute set on/off." },
    { "cbar",         "c", debug_cbar,            "colorbar set on/off." },
#ifdef HDMI_SCDC_SUPPORT
    { "scdc",        "sc", debug_scdc,            "scdc set on/off." },
#endif
    { "ddc",         "dd", debug_ddc,             "ddc frequency set in range [0,11]." },
    { "hdmimode",     "m", debug_hdmi_mode,       "hdmi tmds mode set dvi/1.4/2.0." },
    { "outclrspace", "oc", debug_out_color_space, "output colorspace mode set rgb/yuv422/yuv444/yuv420." },
    { "deepclr",      "d", debug_deep_color,      "output color depth  set 08/10/12bit." },
#ifndef HDMI_PRODUCT_HI3559A
    /* 59A is not support dither */
    { "dither",      "di", debug_dither,          "dither set on/off." },
#endif
    { "aut",         "au", debug_authenticate,    "authentication mode set on/off." },
    { "cmd",        "cmd", debug_cmd,             "cmd set ." },
#ifdef HDMI_HDR_SUPPORT
    { "hdr",         "hm", debug_hdr,             "hdr debug info set." },
#endif
#ifdef HDMI_FRL_SUPPORT
    { "frl",         "fr", debug_frl,             "FRL training test." },
    { "frlcfg",     "cfg", debug_frl_cfg,         "FRL cfg training test." },
    { "frlattr",   "attr", debug_frl_attr,        "FRL attr training test." },
    { "frlmach",     "ma", debug_frl_mach,        "FRL mach training test." },
    { "compat",     "cmp", debug_compat,          "compat training test." }
#endif
};

static hi_s32 debug_print_help(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 i;

    hi_unused(cmd_arg);
    for (i = 0; i < hdmi_array_size(g_cmd_table); i++) {
        if (g_cmd_table[i].comment_help && g_cmd_table[i].name) {
            hdmi_dbg_print("    %02u. %-20s", i, g_cmd_table[i].name);
            hdmi_dbg_print("--[%-2s]%s\n", g_cmd_table[i].short_name, g_cmd_table[i].comment_help);
        } else {
            hdmi_dbg_err("g_cmd_table[%u].comment_help or name is null!\n", i);
        }
    }

    hdmi_dbg_print("\n");
    hdmi_dbg_print("    you can use 'echo cmd argv... > /proc/umap/hdmi0' execute a debug command.\n"
                   "    you can use 'echo cmd > /proc/umap/hdmi0' for more details about each command.\n"
                   "    here, 'cmd' refers to NO./complete-name/short-name above.\n");

    return HI_SUCCESS;
}

static hi_s32 debug_cmd_handle(hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 i = 0;
    hi_u32 cmd_num;
    hi_char *str = HI_NULL;

    str = cmd_arg->argv[0];
    if (str == HI_NULL) {
        goto error_help;
    }

    if (cmd_arg->argc > 0) {
        cmd_num = debug_str_to_u32(str, &str, HDMI_DEBUG_BASE_DEC);
        for (i = 0; i < hdmi_array_size(g_cmd_table); i++) {
            if (g_cmd_table[i].name == HI_NULL) {
                hdmi_dbg_err("g_cmd_table[%u].name null\n", i);
                goto error_help;
            }
            if (((cmd_num != i) || (*str != '\0')) &&
                (is_str_equal(g_cmd_table[i].name, cmd_arg->argv[0]) == HI_FALSE) &&
                (is_str_equal(g_cmd_table[i].short_name, cmd_arg->argv[0]) == HI_FALSE)) {
                continue;
            }
            if (g_cmd_table[i].fn_cmd_func != HI_NULL) {
                g_cmd_table[i].fn_cmd_func(cmd_arg);
            } else {
                hdmi_dbg_err("g_cmd_table[%u].fn_cmd_func null\n", i);
                goto error_help;
            }
            break;
        }
    }

    if (i == hdmi_array_size(g_cmd_table)) {
        goto error_help;
    }

    return HI_SUCCESS;

error_help:

    return debug_print_help(cmd_arg);
}

static hi_s32 debug_cmd_line_parse(hi_char *cmd_str, hi_u32 len, hdmi_debug_cmd_arg *cmd_arg)
{
    hi_u32 i;
    hi_u32 tmp_argc = 0;
    hi_char *tmp_str = NULL;

    if (len == 0 || len >= DEBUG_MAX_CMD_CHAR_NUM) {
        hdmi_dbg_err("invalid param!\n");
        return HI_FAILURE;
    }

    tmp_str = cmd_str;
    cmd_arg->argc = 0;

    for (i = 0; i < DEBUG_MAX_ARGV_NUM; i++) {
        cmd_arg->argv[i] = HI_NULL;
    }

    if ((*tmp_str != DEBUG_CHAR_SPACE) && (*tmp_str != DEBUG_CHAR_TAB)) {
        cmd_arg->argv[tmp_argc] = tmp_str;
        tmp_argc++;
        tmp_str++;
    }

    for (i = 0; i < len; tmp_str++, i++) {
        if ((*tmp_str == DEBUG_CHAR_SPACE) || (*tmp_str == DEBUG_CHAR_TAB)) {
            *tmp_str = '\0';

            if ((*(tmp_str + 1) == DEBUG_CHAR_SPACE) || (*(tmp_str + 1) == DEBUG_CHAR_TAB)) {
                continue;
            }
            cmd_arg->argv[tmp_argc] = tmp_str + 1;
            tmp_argc++;

            if (tmp_argc == 3) { /* 3, is tmp argument cont */
                cmd_arg->remain_len = len - i;
            }
            if (tmp_argc >= DEBUG_MAX_ARGV_NUM) {
                hdmi_dbg_print(" too much argcs.max argc = %u\n", DEBUG_MAX_ARGV_NUM);
                tmp_argc = DEBUG_MAX_ARGV_NUM;
                break;
            }
        }
    }

    cmd_arg->argc = tmp_argc;

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_debug_edid_raw_read(hdmi_edid_updata_mode mode, hi_u8 *data, hi_u32 len)
{
    errno_t ret;
    hdmi_debug_edid *dbg_edid = &g_debug_edid;

    if ((mode != HDMI_EDID_UPDATE_DEBUG) || (dbg_edid->data_valid == HI_FALSE) ||
        (data == HI_NULL) || (len > HDMI_EDID_TOTAL_SIZE)) {
        hdmi_err("debug edid read fail,data_valid=%u !\n", dbg_edid->data_valid);
        return HI_FAILURE;
    }
    ret = memcpy_s(data, len, dbg_edid->data, dbg_edid->len);
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return dbg_edid->len;
}

hi_s32 drv_hdmi_debug_source_write(hdmi_proc_entry *file, const char *buf, hi_u32 count, hi_s64 *data)
{
    hi_s32             ret;
    hi_char           *tmp_buf  = HI_NULL;
    hdmi_debug_cmd_arg cmd_arg;
    hdmi_device       *hdmi_dev = HI_NULL;

    debug_null_check_return(buf);
    debug_null_check_return(data);
    debug_null_check_return(file);

    (hi_void)memset_s(&cmd_arg, sizeof(hdmi_debug_cmd_arg), 0, sizeof(hdmi_debug_cmd_arg));
    cmd_arg.hdmi_id = HDMI_DEVICE_ID0;

    hdmi_dev = get_hdmi_device(cmd_arg.hdmi_id);
    debug_null_check_return(hdmi_dev);

    tmp_buf = (hi_char *)osal_vmalloc(DEBUG_MAX_CMD_CHAR_NUM);
    if (tmp_buf == HI_NULL) {
        hdmi_dbg_err("vmalloc fail!\n");
        return EFAULT;
    }
    (hi_void)memset_s(tmp_buf, DEBUG_MAX_CMD_CHAR_NUM, '\0', DEBUG_MAX_CMD_CHAR_NUM);

    if (count >= DEBUG_MAX_CMD_CHAR_NUM || count == 0) {
        hdmi_dbg_err("\n echo chars err. count=%lu, max cnt=%u\n", count, DEBUG_MAX_CMD_CHAR_NUM);
        goto error;
    }
    if (osal_copy_from_user(tmp_buf, buf, count)) {
        hdmi_dbg_err("\n copy from user failed. \n");
        goto error;
    }
    tmp_buf[count - 1] = '\0';
    ret = debug_cmd_line_parse(tmp_buf, count - 1, &cmd_arg);
    if (ret != HI_SUCCESS) {
        goto error;
    }

    hdmi_mutex_lock(hdmi_dev->mutex_proc);
    if (hdmi_dev->user_cnt == 0 && hdmi_dev->kernel_cnt == 0) {
        hdmi_mutex_unlock(hdmi_dev->mutex_proc);
        hdmi_dbg_err("\n HDMI not open! \n");
        goto error;
    }

    ret = debug_cmd_handle(&cmd_arg);
    hdmi_mutex_unlock(hdmi_dev->mutex_proc);

    if (ret != HI_SUCCESS) {
        hdmi_dbg_err("\n cmd handle error \n");
        goto error;
    }

    osal_vfree(tmp_buf);
    return count;

error:
    osal_vfree(tmp_buf);

    return -EFAULT;
}


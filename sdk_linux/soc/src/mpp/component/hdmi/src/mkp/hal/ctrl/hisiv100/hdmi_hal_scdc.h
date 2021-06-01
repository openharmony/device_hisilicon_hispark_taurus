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
#ifndef __HDMI_HAL_SCDC_H__
#define __HDMI_HAL_SCDC_H__

#include "hi_type.h"
#include "hdmi_hal_machine.h"
#include "hdmi_hal_ddc.h"
#include "drv_hdmi_common.h"
#include "hdmi_product_define.h"

#define SCDC_DEFAULT_SCRAMBLE_TIMEOUT  200
#define SCDC_MAX_SCRAMBLE_TIMEOUT      1000
#define SCDC_DEFAULT_SCRAMBLE_INTERVAL 20
#define SCDC_DEFAULT_POLL_INTERVAL     150
#define SCDC_MAX_UPDATE_RECORD_NUM     3
#define SCDC_WHILE_SCRAMBLE_TIMEOUT    3500
#define SCDC_ERROR_MAX_NUM_SCRAMBLE    3
#define SCDC_ERROR_MAX_NUM_OE_DISABLE  3
#define SCDC_ERROR_MAX_NUM_OE_ENABLE   3
#define SCDC_ERROR_MAX_NUM_RR_TEST     3
#define SCDC_ERROR_MAX_NUM_SINK_UNREAD 1
#define SCDC_GET_SCRAMBLE_STATUS_TIME  1
#define SCDC_OE_DISABLE_WAIT_TIME      5
#define SCDC_MAX_ERROR_RECORD_NUM      (SCDC_ERROR_MAX_NUM_SCRAMBLE + \
                                       SCDC_ERROR_MAX_NUM_OE_DISABLE + SCDC_ERROR_MAX_NUM_OE_ENABLE)

/* HDMI2.0 section 10.4.1.3, OFFSET 0x10 */
typedef union {
    struct {
        hi_u8 status_update : 1;
        hi_u8 ced_update    : 1;
        hi_u8 rr_test       : 1;
        hi_u8 rsvd          : 5;
    } u8;
    hi_u8 byte;
} scdc_updata_flag;

/* HDMI2.0 section 10.4.1.4, OFFSET 0x20 */
typedef union {
    struct {
        hi_u8 screambling_enable   : 1;
        hi_u8 tmds_bit_clock_ratio : 1;
        hi_u8 rsvd                 : 6;
    } u8;
    hi_u8 byte;
} scdc_tmds_config;

/* HDMI2.0 section 10.4.1.5, OFFSET 0x21 */
typedef union {
    struct {
        hi_u8 screambling_status : 1;
        hi_u8 rsvd               : 7;
    } u8;
    hi_u8 byte;
} scdc_scrambler_status;

/* HDMI2.0 section 10.4.1.6, OFFSET 0x30 */
typedef union {
    struct {
        hi_u8 rr_enable : 1;
        hi_u8 rsvd      : 7;
    } u8;
    hi_u8 byte;
} scdc_config0;

/* HDMI2.0 section 10.4.1.7, OFFSET 0x40 */
typedef union {
    struct {
        hi_u8 colck_detected : 1;
        hi_u8 ch0_locked     : 1;
        hi_u8 ch1_locked     : 1;
        hi_u8 ch2_locked     : 1;
        hi_u8 rsvd           : 4;
    } u8;
    hi_u8 byte;
} scdc_status_flag0;

/* HDMI2.0 section 10.4.1.8, OFFSET 0x50~0x55 */
typedef union {
    struct {
        hi_u8 ch_error_cnt_l : 8;
        hi_u8 ch_error_cnt_h : 7;
        hi_u8 ch_valid       : 1;
    } u16;
    hi_u16 word;
} scdc_char_err_detect;

/* HDMI2.0 section 10.4.1.9, OFFSET 0xC0 */
typedef union {
    struct {
        hi_u8 test_read_request_delay : 7;
        hi_u8 test_read_request       : 1;
    } u8;
    hi_u8 byte;
} scdc_rr_test_cfg;

typedef enum {
    SCDC_OFFSET_SINK_VERSION     = 0x01,
    SCDC_OFFSET_SRC_VERSION      = 0x02,
    SCDC_OFFSET_UPDATE_0         = 0x10,
    SCDC_OFFSET_UPDATE_1         = 0x11,
    SCDC_OFFSET_TMDS_CONFIG      = 0x20,
    SCDC_OFFSET_SCRAMBLER_STATUS = 0x21,
    SCDC_OFFSET_CONFIG_0         = 0x30,
    SCDC_OFFSET_STATUS_FLAG_0    = 0x40,
    SCDC_OFFSET_STATUS_FLAG_1    = 0x41,
    SCDC_OFFSET_DRR_DET_0_L      = 0x50,
    SCDC_OFFSET_DRR_DET_0_H      = 0x51,
    SCDC_OFFSET_DRR_DET_1_L      = 0x52,
    SCDC_OFFSET_DRR_DET_1_H      = 0x53,
    SCDC_OFFSET_DRR_DET_2_L      = 0x54,
    SCDC_OFFSET_DRR_DET_2_H      = 0x55,
    SCDC_OFFSET_DRR_DET_CHKSUM   = 0x56,
    SCDC_OFFSET_TEST_CONFIG_0    = 0xC0,
    SCDC_OFFSET_IEEE_OUI_3TH     = 0xD0,
    SCDC_OFFSET_IEEE_OUI_2ND     = 0xD1,
    SCDC_OFFSET_IEEE_OUI_1ST     = 0xD2,
    SCDC_OFFSET_DEVICE_ID_HEAD   = 0xD3,
    SCDC_OFFSET_DEVICE_ID_TAIL   = 0xDD
} scdc_offset;

typedef struct {
    hi_u8            sink_version; // SCDC_OFFSET_SINK_VERSION
    hi_u8            src_version;  // SCDC_OFFSET_SRC_VERSION
    scdc_tmds_config tmds_config0; // SCDC_OFFSET_TMDS_CONFIG
    scdc_config0     config0;      // SCDC_OFFSET_CONFIG_0
    scdc_rr_test_cfg rr_test_cfg;  // SCDC_OFFSET_TEST_CONFIG_0
    hi_u8            ieee_out[SCDC_OFFSET_IEEE_OUI_1ST - SCDC_OFFSET_IEEE_OUI_3TH + 1];
    hi_u8            device_id[SCDC_OFFSET_DEVICE_ID_TAIL - SCDC_OFFSET_DEVICE_ID_HEAD + 1];
} scdc_reg_static_status;

typedef struct {
    scdc_updata_flag      update0;         // SCDC_OFFSET_UPDATE_0
    scdc_scrambler_status scramble_status; // SCDC_OFFSET_SCRAMBLER_STATUS
    scdc_status_flag0     status_flag0;    // SCDC_OFFSET_STATUS_FLAG_0
    scdc_char_err_detect  ch0_err;         // SCDC_OFFSET_DRR_stb_det_0_L,SCDC_OFFSET_DRR_stb_det_0_H
    scdc_char_err_detect  ch1_err;         // SCDC_OFFSET_DRR_stb_det_1_L,SCDC_OFFSET_DRR_stb_det_1_H
    scdc_char_err_detect  ch2_err;         // SCDC_OFFSET_DRR_stb_det_2_L,SCDC_OFFSET_DRR_stb_det_2_H
    hi_u8                 err_chk_sum;     // SCDC_OFFSET_DRR_stb_det_CHKSUM
} scdc_updata_status;

typedef struct {
    hi_bool sink_read_quest;
    /* HI_TRUE indicate enable; HI_FALSE indicate disable. */
    hi_bool src_scramble;
    /* HI_TRUE indicate enable; HI_FALSE indicate disable. */
    hi_bool sink_scramble;
    /* HI_TRUE indicate 40x; HI_FALSE indicate 10x. */
    hi_bool tmds_clk_ratio40x;
    /* in unit of ms. for [0,200],force to default 200; or, set the value cfg(>200). */
    hi_u32  scramble_timeout;
    /* in unit of ms, range[20,200).for [0,20] or >=200,force to default 20; or, set the value cfg[20,200). */
    hi_u32  scramble_interval;
} scdc_attr;

typedef enum {
    SCDC_ERROR_SINK_UNREAD,
    SCDC_ERROR_SCRAMBLE,
    SCDC_ERROR_OE_DISABLE,
    SCDC_ERROR_OE_ENABLE,
    SCDC_ERROR_BUTT
} scdc_error_type;

typedef struct {
    hi_u32 sramble_fail_cnt;
    hi_u32 oe_disable_fail_cnt;
    hi_u32 oe_enable_fail_cnt;
    hi_u32 sink_un_read_cnt;
} scdc_error_cnt;

typedef struct {
    scdc_error_type err_type;
    hi_u64          time_stamp;
    scdc_attr       err_attr;
} scdc_error_record;

typedef struct {
    hi_bool                init;
    hdmi_device_id         hdmi_id;
    hi_u32                 mach_id;
    hi_bool                mach_enable;
    hi_u64                 mach_interval;
    scdc_error_cnt         err_cnt;
    hi_u32                 err_record_ptr;
    scdc_error_record      err_record[SCDC_MAX_ERROR_RECORD_NUM];
    hi_u32                 reg_update_ptr;
    scdc_updata_status     reg_update[SCDC_MAX_UPDATE_RECORD_NUM];
    scdc_reg_static_status reg_static;
} scdc_status;

typedef struct {
    scdc_status status;
    scdc_attr attr;
} scdc_infomation;

hi_s32 hal_hdmi_scdc_init(hdmi_device_id hdmi_id);

hi_s32 hal_hdmi_scdc_deinit(hdmi_device_id hdmi_id);

hi_s32 hal_hdmi_scdc_reset(hdmi_device_id hdmi_id);

hi_s32 hal_hdmi_scdc_attr_set(hdmi_device_id hdmi_id, scdc_attr *attr);

hi_s32 hal_hdmi_scdc_attr_get(hdmi_device_id hdmi_id, scdc_attr *attr);

hi_s32 hal_hdmi_scdc_all_status_get(hdmi_device_id hdmi_id, scdc_status *status);

hi_s32 hal_hdmi_scdc_support_get(hdmi_device_id hdmi_id, hi_bool *support);

hi_s32 hal_hdmi_scdc_scramble_force(hdmi_device_id hdmi_id, hi_bool enable);

hi_s32 hal_hdmi_scdc_mach_enable(hdmi_device_id hdmi_id, hi_bool enable);

#endif /* __HDMI_HAL_SCDC_H__ */


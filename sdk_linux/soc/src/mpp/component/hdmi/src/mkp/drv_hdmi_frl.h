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
#ifndef __DRV_HDMI_FRL_H__
#define __DRV_HDMI_FRL_H__

#include "hi_type.h"
#include "drv_hdmi_common.h"
#include "drv_hdmi_dfm.h"

typedef enum {
    HDMI_FRL_TRAIN_STEP_READY_CHECK,    // LTS:2
    HDMI_FRL_TRAIN_STEP_TRAIN_START,    // LTS:3
    HDMI_FRL_TRAIN_STEP_RESULT_CHECK,   // LTS:3
    HDMI_FRL_TRAIN_STEP_RATE_CHANGE,    // LTS:4
    HDMI_FRL_TRAIN_STEP_RESULT_HANDLE,  // LTS:P
    HDMI_FRL_TRAIN_STEP_RETRAIN_CHECK,  // LTS:P
    HDMI_FRL_TRAIN_STEP_STOP,
    HDMI_FRL_TRAIN_STEP_BUTT
} hdmi_frl_train_step;

typedef enum {
    HDMI_FRL_FAIL_NONE,
    HDMI_FRL_FAIL_TMDS,
    HDMI_FRL_FAIL_NOTIFY,
    HDMI_FRL_FAIL_BUTT
} hdmi_frl_fail_strategy;

typedef enum {
    HDMI_FRL_STRATEGY_MODE_1,
    HDMI_FRL_STRATEGY_MODE_2,
    HDMI_FRL_STRATEGY_MODE_3,
    HDMI_FRL_STRATEGY_MODE_BUTT
} hdmi_frl_strategy_mode;

typedef enum {
    HDMI_FRL_RATE_SELECT_LITTLE,
    HDMI_FRL_RATE_SELECT_BIG,
    HDMI_FRL_RATE_SELECT_BUTT
} hdmi_frl_rate_select;

typedef enum {
    DFM_BIT_RATE_3 = 3,
    DFM_BIT_RATE_6 = 6,
    DFM_BIT_RATE_8 = 8,
    DFM_BIT_RATE_10 = 10,
    DFM_BIT_RATE_12 = 12
} dfm_bit_rate_val;

typedef enum {
    DFM_COLOR_DEPTH_8 = 8,
    DFM_COLOR_DEPTH_10 = 10,
    DFM_COLOR_DEPTH_12 = 12,
    DFM_COLOR_DEPTH_16 = 16
} dfm_color_depth;

typedef enum {
    DFM_PIXEL_FORMAT_MODE_0,
    DFM_PIXEL_FORMAT_MODE_1,
    DFM_PIXEL_FORMAT_MODE_2,
    DFM_PIXEL_FORMAT_MODE_3
} dfm_pixel_format_mode;

typedef enum {
    AUDIO_SAMPLE_RATE_UNKNOWN,
    AUDIO_SAMPLE_RATE_8K   = 8000,
    AUDIO_SAMPLE_RATE_11K  = 11000,
    AUDIO_SAMPLE_RATE_12K  = 12000,
    AUDIO_SAMPLE_RATE_16K  = 16000,
    AUDIO_SAMPLE_RATE_22K  = 22000,
    AUDIO_SAMPLE_RATE_24K  = 24000,
    AUDIO_SAMPLE_RATE_32K  = 32000,
    AUDIO_SAMPLE_RATE_44K  = 44100,
    AUDIO_SAMPLE_RATE_48K  = 48000,
    AUDIO_SAMPLE_RATE_88K  = 88000,
    AUDIO_SAMPLE_RATE_96K  = 96000,
    AUDIO_SAMPLE_RATE_176K = 176000,
    AUDIO_SAMPLE_RATE_192K = 192000,
    AUDIO_SAMPLE_RATE_768K = 768000,
    AUDIO_SAMPLE_RATE_BUTT
} hdmi_frl_info_rate;

typedef struct {
    hi_bool             start_mach;
    hi_u64              start_time;
    hi_u32              wait_ready_ms;
    hi_u32              wait_handle_ms;
    hi_u32              wait_retrain_ms;
    hi_u32              train_timeout;
    hdmi_frl_train_step train_status;
    hdmi_frl_mach_mode  mach_mode;
    frl_sw_train_mode   sw_train_mode;
} hdmi_frl_state_mach_info;

typedef struct {
    hi_u8 max_rate;
    hi_u8 min_rate;
    hi_u8 cur_rate;
    hi_u8 debug_rate;
} hdmi_frl_rate_info;

typedef struct {
    hi_bool                  start;
    hi_bool                  dsc_enable;
    hi_bool                  scdc_present;
    hi_bool                  cts_mode;
    hi_bool                  frl_reset;
    hi_bool                  work_en;
    hi_u32                   tmds_clk;
    hi_u32                   fail_count;
    hi_u32                   max_fail_times;
    hi_u32                   bypass;
    hdmi_frl_mode            mode;
    hdmi_frl_strategy_mode   strategy_mode;
    hdmi_frl_rate_select     rate_select;
    hdmi_frl_state_mach_info state_mach_info;
    hdmi_frl_rate_info       rate_info;
    hdmi_frl_train           train_status;
    hdmi_frl_fail_strategy   fail_strategy;
    hdmi_txfff_mode          aen_cur_tx_ffe[4];
} hdmi_frl_info;

typedef struct {
    hdmi_frl_mode hdmi_mode;
    hdmi_work_mode rate;
} hdmi_frl_debug;

typedef enum {
    FRL_BYPASS_READY_CHECK   = 0x1,
    FRL_BYPASS_RESULT_CHECK  = 0x2,
    FRL_BYPASS_RETRAIN_CHECK = 0x4
} hdmi_frl_bypass;

typedef struct {
    hi_u32 addr;
    hi_u32 value;
} frl_rx_crc_check;

hi_void drv_hdmi_frl_mach_init(hdmi_device_id hdmi_id);

hi_void drv_hdmi_frl_enable(hdmi_device_id hdmi_id, hi_bool enable);

hi_s32 drv_hdmi_frl_mode_strategy(hdmi_device_id hdmi_id);

hi_void drv_hdmi_frl_train_mach(hdmi_device_id hdmi_id);

hi_s32 drv_hdmi_frl_mode_change(hdmi_device_id hdmi_id, const hdmi_frl_debug *debug);

#endif  /* __DRV_HDMI_FRL_H__ */


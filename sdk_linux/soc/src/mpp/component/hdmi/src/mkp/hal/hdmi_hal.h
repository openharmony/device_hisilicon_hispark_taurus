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
#ifndef __HDMI_HAL_H__
#define __HDMI_HAL_H__

#include "drv_hdmi_common.h"

typedef hi_void *hdmi_handle;

typedef enum {
    HDMI_DEBUG_CMD_COLOR_BAR,
    HDMI_DEBUG_CMD_SW_RESET,
    HDMI_DEBUG_CMD_RGB2YUV,
    HDMI_DEBUG_CMD_YUV2RGB,
    HDMI_DEBUG_CMD_DITHER,
    HDMI_DEBUG_CMD_BYPASS,
    HDMI_DEBUG_CMD_DDC_FREQ,
    HDMI_DEBUG_CMD_PHY_DEFAULT_GET,
    HDMI_DEBUG_CMD_PHY_PARA_SET,
    HDMI_DEBUG_CMD_DUMP,
#if defined(HDMI_SUPPORT_LOGIC_HISIV100)
    HDMI_DEBUG_CMD_PROC_MACH,
    HDMI_DEBUG_CMD_PROC_SCDC,
    HDMI_DEBUG_CMD_PROC_HDCP14,
    HDMI_DEBUG_CMD_PROC_HDCP22,
    HDMI_DEBUG_CMD_PROC_DDC,
    HDMI_DEBUG_CMD_PROC_CECTX,
    HDMI_DEBUG_CMD_PROC_CECRX,
    HDMI_DEBUG_CMD_DBG_VIDEO_GET,
    HDMI_DEBUG_CMD_DBG_VIDEO_SET,
    HDMI_DEBUG_CMD_SSC,
    HDMI_DEBUG_CMD_FRL
#endif
} hdmi_debug_cmd;

typedef struct {
    hi_u32 reg_addr;
    hi_u32 reg_vaule;
} hdmi_hal_reg;

typedef struct {
    hdmi_handle             hdmi_hw;
    hdmi_handle             hdmi_dev;
    hi_u32                  hdmi_id;
    hdmi_tx_capability_data tx_capability;
    hdmi_callback           callback;
    hdmi_video_config       video_cfg;
    hdmi_audio_config       audio_cfg;
    hi_char                *base_addr;
    hi_char                *phy_addr;
} hdmi_hal_context;

typedef struct {
    hi_u32 disp_fmt;
    hi_u32 pix_clk;
    hdmi_colorspace color_space;
    hdmi_deep_color deep_color;
} hdmi_hal_base_param;

typedef struct hdmi_hal_ {
    hdmi_hal_context hal_ctx;
    hi_void (*hal_hdmi_hardware_init)(const struct hdmi_hal_ *hal);
    hi_void (*hal_hdmi_tmds_mode_set)(const struct hdmi_hal_ *hal, hdmi_tmds_mode tmds_mode);
    hi_void (*hal_hdmi_avmute_set)(const struct hdmi_hal_ *hal, hi_bool av_mute);
    hi_void (*hal_hdmi_infoframe_set)(const struct hdmi_hal_ *, hdmi_infoframe_id, hi_u8 in_buffer[], hi_u32 buf_len);
    hi_void (*hal_hdmi_infoframe_enable_set)(const struct hdmi_hal_ *, hdmi_infoframe_id, hi_bool);
    hi_s32  (*hal_hdmi_video_path_set)(const struct hdmi_hal_ *hal, hdmi_video_config *video_cfg);
    hi_void (*hal_hdmi_phy_output_enable_set)(const struct hdmi_hal_ *hal, hi_bool enable);
    hi_void (*hal_hdmi_phy_power_enable_set)(const struct hdmi_hal_ *hal, hi_bool enable);
    hi_void (*hal_hdmi_tx_capability_get)(const struct hdmi_hal_ *hal, hdmi_tx_capability_data *tx_capability);
    hi_void (*hal_hdmi_emi_status_get)(const struct hdmi_hal_ *hal, hdmi_emi_status *emi_status);
    hi_void (*hal_hdmi_csc_param_set)(struct hdmi_hal_ *hal, hdmi_video_config *video_cfg);
    hi_void (*hal_hdmi_phy_set)(const struct hdmi_hal_ *hal, hdmi_phy_cfg *phy_cfg);
    hi_void (*hal_hdmi_ctrl_reset)(const struct hdmi_hal_ *hal);
    hi_s32  (*hal_hdmi_phy_hw_spec_set)(const struct hdmi_hal_ *hdmi_hal, hi_u32 tmds_clk, const hdmi_hw_spec *hw_spec);
    hi_s32  (*hal_hdmi_phy_hw_spec_get)(const struct hdmi_hal_ *hdmi_hal, hdmi_hw_spec *hw_spec);
    hi_void (*hal_hdmi_sequencer_handler_process)(const struct hdmi_hal_ *hal);
    hi_void (*hal_hdmi_hardware_status_get)(const struct hdmi_hal_ *hal, hdmi_hardware_status *hw_status);
    hi_void (*hal_hdmi_hot_plug_status_get)(const struct hdmi_hal_ *hal, hi_bool *hot_plug);
    hi_void (*hal_hdmi_hdp_intr_status_get)(const struct hdmi_hal_ *hal, hi_bool *intr_status);
    hi_void (*hal_hdmi_audio_mute_set)(const struct hdmi_hal_ *hal, hi_bool audio_mute);
    hi_s32  (*hal_hdmi_audio_path_set)(const struct hdmi_hal_ *hal, hdmi_audio_config *audio_cfg);
    hi_s32  (*hal_hdmi_audio_n_cts_set)(const struct hdmi_hal_ *hal, hdmi_audio_ncts *audio_cfg);
    hi_void (*hal_hdmi_audio_path_enable_set)(const struct hdmi_hal_ *hal, hi_bool enable);
    hi_s32  (*hal_hdmi_edid_raw_data_read)(const struct hdmi_hal_ *hal, hi_u32 size, hi_u8 out_buffer[]);
    hi_void (*hal_hdmi_phy_output_enable_get)(const struct hdmi_hal_ *hal, hi_bool *enable);
    hi_void (*hal_hdmi_video_mute_set)(const struct hdmi_hal_ *hal, hi_bool video_mute);
    hi_void (*hal_hdmi_black_data_set)(const struct hdmi_hal_ *hal, hdmi_black_frame_info *black_pram);
    hi_void (*hal_hdmi_debug)(const struct hdmi_hal_ *hal, hdmi_debug_cmd debug_cmd, hi_void *data);
    hi_u32 *(*hal_hdmi_base_addr_get)(const struct hdmi_hal_ *hal);
#ifdef HDMI_HDR_SUPPORT
    hi_void (*hal_hdmi_hdr_timer_set)(const struct hdmi_hal_ *hal, hdmi_timer_config *timer_cfg);
#endif
#ifdef HDMI_SCDC_SUPPORT
    hi_void (*hal_hdmi_scdc_config)(const struct hdmi_hal_ *hal, hdmi_scdc_config *scdc_config);
    hi_void (*hal_hdmi_scdc_status_get)(const struct hdmi_hal_ *hal, hdmi_scdc_status *scdc_status);
    hi_void (*hal_hdmi_scdc_status_set)(const struct hdmi_hal_ *hal, hdmi_scdc_status *scdc_status);
    hi_s32  (*hal_hdmi_scdc_process)(const struct hdmi_hal_ *hal, scdc_cmd scdc_cmd, hi_void *data);
#endif
#ifdef HDMI_FRL_SUPPORT
    hi_s32 (*hal_hdmi_frl_get_train_status)(const struct hdmi_hal_ *hal, hdmi_frl_train *frl_status);
    hi_s32 (*hal_hdmi_frl_train_config)(const struct hdmi_hal_ *hal, hdmi_frl_train_config *frl_config);
    hi_s32 (*hal_hdmi_frl_train_start)(const struct hdmi_hal_ *hal);
    hi_s32 (*hal_hdmi_frl_enable)(const struct hdmi_hal_ *hal, hi_bool enable);
#endif
} hdmi_hal;

hi_s32 hal_hdmi_open(hdmi_hal_init *hal_init, hdmi_hal **hal_handle);

void hal_hdmi_close(hdmi_hal *hal);

#endif /* __HDMI_HAL_H__ */


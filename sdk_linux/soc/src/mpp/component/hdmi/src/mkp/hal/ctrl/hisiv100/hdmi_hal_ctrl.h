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
#ifndef __HDMI_HAL_CTRL_H__
#define __HDMI_HAL_CTRL_H__

#include "hi_type.h"
#include "hdmi_hal_machine.h"
#include "drv_hdmi_common.h"

#define HDMI_INFOFRMAE_MAX_SIZE 31

typedef enum {
    HDMI_COLORMETRY_BT709,
    HDMI_COLORMETRY_BT601,
    HDMI_COLORMETRY_BT2020,
    HDMI_COLORMETRY_BT2020_CONST,
    HDMI_COLORMETRY_BUTT
} hdmi_hal_colorimetry;

typedef struct {
    hi_bool h_pol_inver;
    hi_bool v_pol_inver;
    hi_bool de_inver;
} hdmi_sync_pol_type;

typedef struct {
    hdmi_video_timing       timing;
    hi_u32                  in_pixel_clk; /* in Khz */
    hi_u32                  out_tmds_clk; /* in Khz */
    hdmi_colorspace         in_color_space;
    hdmi_colorspace         out_color_space;
    hdmi_hal_colorimetry    in_colormetry;
    hdmi_hal_colorimetry    out_colormetry;
    hdmi_deep_color         in_deep_color;
    hdmi_deep_color         out_deep_color;
    hdmi_sync_pol_type      out_hv_sync_pol;
    hdmi_quantization_range in_quantization;
    hdmi_quantization_range out_quantization;
} hdmi_video_path;

typedef struct {
    hi_bool sync_sw_enable;
    hi_bool vsync_polarity; /* when sync_sw_enable==0, indicates hw; or, indicates sw */
    hi_bool hsync_polarity; /* when sync_sw_enable==0, indicates hw; or, indicates sw */
    hi_bool progressive;
    hi_u32  hsync_total;
    hi_u32  hactive_cnt;
    hi_u32  vsync_total;
    hi_u32  vactive_cnt;
} ctrl_timming_detect;

typedef struct {
    hi_u32                  out_tmds_clk; /* in khz */
    hdmi_colorspace         in_color_space;
    hdmi_colorspace         out_color_space;
    hdmi_hal_colorimetry    in_colormetry;
    hdmi_hal_colorimetry    out_colormetry;
    hdmi_deep_color         in_deep_color;
    hdmi_deep_color         out_deep_color;
    hdmi_sync_pol_type      out_hv_sync_pol;
    hdmi_quantization_range in_quantization;
    hdmi_quantization_range out_quantization;
    hi_bool                 csc_enable;
    hi_bool                 y422_enable;
    hi_bool                 y420_enable;
    hi_bool                 dwsm_hori_enable;
    hi_bool                 dwsm_vert_enable;
    hdmi_video_dither       dither_mode;
    ctrl_timming_detect     timing;
} ctrl_video_status;

typedef struct {
    hdmi_audio_interface sound_intf;
    hdmi_sample_rate     sample_rate;
    hdmi_audio_ch        channel_num;
    hdmi_audio_bit_depth sample_bit;
    hi_u32               pixel_clk; /* it is used to config N value */
} hdmi_audio_path;

typedef struct {
    hi_bool              enable_audio;
    hi_bool              audio_mute;
    hdmi_audio_interface sound_intf;
    hdmi_sample_rate     sample_rate;
    hdmi_audio_ch        channel_num;
    hdmi_audio_bit_depth sample_bit;
    hi_u32               pixel_clk; /* it is used to config N value */
    hi_u32               ref_n;
    hi_u32               reg_n;
    hi_u32               ref_cts;
    hi_u32               reg_cts;
} ctrl_audio_status;

typedef struct {
    hi_bool hpd_on;
    hi_bool rsen_on;
} hdmi_hpd_rsen;

typedef struct {
    hi_bool         enable;      // extern thread cfg
    hdmi_timer_type timer_type;  // extern thread cfg
    hi_u64          time_length; // extern thread cfg, in ms
    hi_u64          start_time;  // extern thread cfg, in ms
} ctrl_hdr_timer;

typedef struct {
    hi_bool         init;
    hi_u32          mach_id;
    hdmi_tmds_mode  tmds_mode;
    hi_bool         avmute;
    hi_bool         audio_mute;
    hi_bool         video_mute;
    hi_bool         if_enable[HDMI_INFOFRAME_TYPE_BUTT - HDMI_INFOFRAME_TYPE_VENDOR];
    hi_bool         if_data[HDMI_INFOFRAME_TYPE_BUTT - HDMI_INFOFRAME_TYPE_VENDOR][HDMI_INFOFRMAE_MAX_SIZE];
    hdmi_audio_path audio_path;
    hdmi_video_path video_path;
    hi_void        *event_data;
    hdmi_callback   event_callback;
#ifdef HDMI_HDR_SUPPORT
    ctrl_hdr_timer     hdr_timer[HDMI_TIMER_TYPE_BUTT];
#endif
    mach_callback     *hpd_callback;
    hi_bool            hpd;
    hi_bool            rsen;
} hdmi_ctrl_info;

typedef struct {
    hi_bool init;
    hdmi_tmds_mode  tmds_mode;
    hi_bool         avmute;
    hi_bool         audio_mute;
    hi_bool         video_mute;
    hi_bool         if_enable[HDMI_INFOFRAME_TYPE_BUTT - HDMI_INFOFRAME_TYPE_VENDOR];
    hi_bool         if_data[HDMI_INFOFRAME_TYPE_BUTT - HDMI_INFOFRAME_TYPE_VENDOR][HDMI_INFOFRMAE_MAX_SIZE];
    hdmi_audio_path audio_path;
    hdmi_video_path video_path;
    hdmi_hpd_rsen   hpd_rsen;
} hdmi_ctrl_status;

hi_s32 hal_hdmi_ctrl_init(hdmi_device_id hdmi, const hdmi_hal_init *hal_init);

hi_s32 hal_hdmi_ctrl_deinit(hdmi_device_id hdmi);

hi_s32 hal_hdmi_ctrl_avmute_set(hdmi_device_id hdmi, const hdmi_avmute_cfg *avmute_cfg);

hi_s32 hal_hdmi_ctrl_avmute_get(hdmi_device_id hdmi, hi_bool *enable);

hi_s32 hal_hdmi_ctrl_audio_mute_set(hdmi_device_id hdmi, hi_bool enable);

hi_s32 hal_hdmi_ctrl_audio_path_enable_set(hdmi_device_id hdmi, hi_bool enable);

hi_s32 hal_hdmi_ctrl_audio_path_set(hdmi_device_id hdmi, const hdmi_audio_path *audio_path);

hi_s32 hal_hdmi_ctrl_audio_path_get(hdmi_device_id hdmi, ctrl_audio_status *audio_path);

hi_s32 hal_hdmi_ctrl_video_mute_set(hdmi_device_id hdmi, hi_bool enable);

hi_s32 hal_hdmi_ctrl_video_mute_get(hdmi_device_id hdmi, hi_bool *enable);

hi_s32 hal_hdmi_ctrl_video_path_set(hdmi_device_id hdmi, hdmi_video_path *video_path);

hi_void hal_hdmi_ctrl_video_path_get(hdmi_device_id hdmi, ctrl_video_status *video_stat);

hi_s32 hal_hdmi_ctrl_tmds_mode_set(hdmi_device_id hdmi, hdmi_tmds_mode tmds_mode);

hi_s32 hal_hdmi_ctrl_tmds_mode_get(hdmi_device_id hdmi, hdmi_tmds_mode *tmds_mode);

hi_s32 hal_hdmi_ctrl_infoframe_en_set(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id, hi_bool enable);

hi_s32 hal_hdmi_ctrl_infoframe_en_get(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id, hi_bool *enable);

hi_s32 hal_hdmi_ctrl_infoframe_data_set(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id,
                                        const hi_u8 *if_data, hi_u32 data_len);

hi_s32 hal_hdmi_ctrl_infoframe_data_get(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id, hi_u8 *if_data);

hi_s32 hal_hdmi_ctrl_hpd_rsen_get(hdmi_device_id hdmi, hdmi_hpd_rsen *hpd_rsen);

hi_s32 hal_hdmi_ctrl_hdr_timer_set(hdmi_device_id hdmi, const hdmi_timer_config *hdr_timer_cfg);

hi_void hal_hdmi_ctrl_dither_dbg_set(hdmi_device_id hdmi, hdmi_video_dither dither_mode);

hi_s32 hal_hdmi_ctrl_csc_set(hdmi_device_id hdmi, const hdmi_video_config *video_cfg);

hi_s32 hal_hdmi_ctrl_data_reset(hdmi_device_id hdmi, hi_bool debug_mode, hi_u32 delay_ms);

hi_s32 hal_hdmi_ctrl_tmds_stable_get(hdmi_device_id hdmi, hi_bool *stable);

#endif /* __HDMI_HAL_CTRL_H__ */


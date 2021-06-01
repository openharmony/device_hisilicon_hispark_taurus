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
#ifndef __DRV_HDMI_DFM_H__
#define __DRV_HDMI_DFM_H__

#include "hi_type.h"

#define AUDIO_SAMPLE_PACKET         0x02
#define ONE_BIT_AUDIO_SAMPLE_PACKET 0x07
#define DTS_AUDIO_PACKET            0x08
#define HBR_AUDIO_PACKET            0x09
#define AUDIO_3D_SAMPLE_PACKET      0x0B
#define ONE_BIT_AUDIO_3D_SAMPLE_PACKET           0x0C
#define MULTI_STREAM_AUDIO_SAMPLE_PACKET         0x0E
#define ONE_BIT_MULTI_STREAM_AUDIO_SAMPLE_PACKET 0x0F
#define AUDIO_CHANNEL_ALLOC_TYPE1   1
#define AUDIO_CHANNEL_ALLOC_TYPE2   2
#define AUDIO_CHANNEL_ALLOC_TYPE3   3

typedef struct {
    hi_bool audio_support;
    hi_bool video_support;
    hi_bool uncompress_support;
    hi_bool is_extra_mode;
    hi_bool canbe_trans;
    hi_s32 htotal;
    hi_s32 vtotal;
    hi_s32 bpp;
    hi_s32 c_frl_sb;
    hi_s32 overhead_sb;
    hi_s32 overhead_rs;
    hi_s32 overhead_map;
    hi_s32 overhead_min;
    hi_s32 overhead_m;
    hi_s32 overhead_max;
    hi_s32 min_time_line;
    hi_s32 max_time_line;
    hi_s32 min_frl_chars_per_line;
    hi_s32 max_frl_chars_per_line;
    hi_s32 audio_ap;
    hi_s32 audio_rap;
    hi_s32 avg_audio_packets;
    hi_s32 audio_packets;
    hi_s32 blank_audio_min;
    hi_s32 c_frl_free;
    hi_s32 c_frl_rc_margin;
    hi_s32 c_frl_rc_savings;
    hi_s32 active_bytes_per_line;
    hi_s32 active_tb_per_line;
    hi_s32 blank_tb_per_line;
    hi_s32 active_time_ref;
    hi_s32 blank_time_ref;
    hi_s32 active_time_min;
    hi_s32 blank_time_min;
    hi_s32 t_borrow;
    hi_s32 tb_borrow;
    hi_s32 c_frl_actual_payload;
    hi_s32 utilization;
    hi_s32 margin;
    hi_s64 avg_tb_rate;
    hi_s64 pixel_clk;
    hi_s64 max_pixel_clk;
    hi_s64 min_pixel_clk;
    hi_s64 max_bit_rate;
    hi_s64 min_bit_rate;
    hi_s64 max_frl_char_rate;
    hi_s64 min_frl_char_rate;
} dfm_info;

typedef struct {
    hi_s32 hactive;
    hi_s32 vactive;
    hi_s32 hblank;
    hi_s32 vblank;
    hi_s32 hsync;
    hi_s32 hback;
    hi_s32 hfront;
    hi_s32 vsync;
    hi_s32 vback;
    hi_s32 vfront;
    hi_s32 v_freq;
    hi_s32 color_depth;
    hi_s32 pixel_format; /* 1:420, 2:422, 3:RGB444 */
    hi_s32 lane_num;
    hi_s32 bit_rate;
    hi_s32 audio_rate;
    hi_s32 packet_type;
    hi_s32 layout;
    hi_s32 acat;
} dfm_in;

hi_bool drv_hdmi_dfm_format_support(dfm_in *dfm);

#endif  /* __DRV_HDMI_DFM_H__ */


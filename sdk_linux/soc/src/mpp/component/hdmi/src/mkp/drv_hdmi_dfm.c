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
#include "drv_hdmi_dfm.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_common.h"

#define FRLCHAR_PER_CB_NUM  510
#define RS_PER_CB_NUM       8
#define CB_PER_SB_NUM       4
#define PIXELCLK_TOLERANCE  5
#define AUDIOCLK_TOLERANCE  1
#define BITRATE_TOLERANCE   3
#define TB_BORROW_MAX       400
#define AUDIO_AP_SIZE_25    25
#define AUDIO_AP_SIZE_100   100
#define AUDIO_AP_SIZE_200   200
#define AUDIO_AP_SIZE_300   300
#define AUDIO_AP_SIZE_400   400
#define DFM_MAGNIFICATION_8 8
#define RATE_MAGNIFICATION  100000
#define OVERHEAD_SIZE       300

#define get_k420(pixelformat)            (((pixelformat) == 1) ? 2 : 1)
#define get_kcd(pixelformat, colorspace) (((pixelformat) == 2) ? 8 : (colorspace))

static dfm_info g_dfm_info;

static hi_s64 dfm_div(hi_s64 div_a, hi_s64 div_b)
{
    hi_s64 div_result;
    div_result = osal_div64_s64(div_a, div_b);
    return div_result;
}

static dfm_info *get_dfm_info(hi_void)
{
    return &g_dfm_info;
}

static hi_s32 dfm_get_audio_ap(dfm_in *in)
{
    hi_s32 audio_ap = 0;

    if (in->packet_type == AUDIO_SAMPLE_PACKET || in->packet_type == ONE_BIT_AUDIO_SAMPLE_PACKET) {
        if (in->layout == 0) {
            audio_ap = AUDIO_AP_SIZE_25; /* AP eq audio_ap / 100 */
        } else {
            audio_ap = AUDIO_AP_SIZE_100; /* AP eq audio_ap / 100 */
        }
    } else if (in->packet_type == DTS_AUDIO_PACKET) {
        audio_ap = AUDIO_AP_SIZE_100; /* AP eq audio_ap / 100 */
    } else if (in->packet_type == HBR_AUDIO_PACKET ||
               in->packet_type == MULTI_STREAM_AUDIO_SAMPLE_PACKET ||
               in->packet_type == ONE_BIT_MULTI_STREAM_AUDIO_SAMPLE_PACKET) {
        audio_ap = AUDIO_AP_SIZE_100; /* AP eq audio_ap / 100 */
    } else if (in->packet_type == AUDIO_3D_SAMPLE_PACKET || in->packet_type == ONE_BIT_AUDIO_3D_SAMPLE_PACKET) {
        if (in->acat == AUDIO_CHANNEL_ALLOC_TYPE1) {
            audio_ap = AUDIO_AP_SIZE_200; /* AP eq audio_ap / 100 */
        } else if (in->acat == AUDIO_CHANNEL_ALLOC_TYPE2) {
            audio_ap = AUDIO_AP_SIZE_300; /* AP eq audio_ap / 100 */
        } else if (in->acat == AUDIO_CHANNEL_ALLOC_TYPE3) {
            audio_ap = AUDIO_AP_SIZE_400; /* AP eq audio_ap / 100 */
        }
    }

    return audio_ap;
}

#ifdef DEBUG
static hi_void dfm_info_debug(const dfm_info *info)
{
    hdmi_info("start\n");
    hdmi_info("htotal = %d\nvtotal = %d\n", info->htotal, info->vtotal);
    hdmi_info("pixel_clk = %lld\n", info->pixel_clk);
    hdmi_info("bpp = %d\n", info->bpp);
    hdmi_info("c_frl_sb = %d\n", info->c_frl_sb);
    hdmi_info("overhead_sb = %d\n", info->overhead_sb);
    hdmi_info("overhead_rs = %d\n", info->overhead_rs);
    hdmi_info("overhead_map = %d\n", info->overhead_map);
    hdmi_info("overhead_min = %d\n", info->overhead_min);
    hdmi_info("overhead_m = %d\n", info->overhead_m);
    hdmi_info("overhead_max = %d\n", info->overhead_max);
    hdmi_info("max_pixel_clk = %lld\n", info->max_pixel_clk);
    hdmi_info("min_pixel_clk = %lld\n", info->min_pixel_clk);
    hdmi_info("max_time_line = %d\n", info->max_time_line);
    hdmi_info("min_time_line = %d\n", info->min_time_line);
    hdmi_info("max_bit_rate = %lld\n", info->max_bit_rate);
    hdmi_info("min_bit_rate = %lld\n", info->min_bit_rate);
    hdmi_info("max_frl_char_rate = %lld\n", info->max_frl_char_rate);
    hdmi_info("min_frl_char_rate = %lld\n", info->min_frl_char_rate);
    hdmi_info("max_frl_chars_per_line = %d\n", info->max_frl_chars_per_line);
    hdmi_info("min_frl_chars_per_line = %d\n", info->min_frl_chars_per_line);
    hdmi_info("audio_ap = %d\naudio_rap = %d\n", info->audio_ap, info->audio_rap);
    hdmi_info("avg_audio_packets = %d\n", info->avg_audio_packets);
    hdmi_info("audio_packets = %d\n", info->audio_packets);
    hdmi_info("blank_audio_min = %d\n", info->blank_audio_min);
    hdmi_info("c_frl_free = %d\n", info->c_frl_free);
    hdmi_info("c_frl_rc_margin = %d\n", info->c_frl_rc_margin);
    hdmi_info("c_frl_rc_savings = %d\n", info->c_frl_rc_savings);
    hdmi_info("active_bytes_per_line = %d\n", info->active_bytes_per_line);
    hdmi_info("active_tb_per_line = %d\n", info->active_tb_per_line);
    hdmi_info("blank_tb_per_line = %d\n", info->blank_tb_per_line);
    hdmi_info("audio_support = %s\n", info->audio_support ? "TRUE" : "FALSE");
    hdmi_info("avg_tb_rate = %lld\n", info->avg_tb_rate);
    hdmi_info("active_time_ref = %d\n", info->active_time_ref);
    hdmi_info("blank_time_ref = %d\n", info->blank_time_ref);
    hdmi_info("active_time_min = %d\n", info->active_time_min);
    hdmi_info("blank_time_min = %d\n", info->blank_time_min);
    hdmi_info("t_borrow = %d\ntb_borrow = %d\n", info->t_borrow, info->tb_borrow);
    hdmi_info("video_support = %s\n", info->video_support ? "TRUE" : "FALSE");
    hdmi_info("c_frl_actual_payload = %d\n", info->c_frl_actual_payload);
    hdmi_info("utilization = %d\n", info->utilization);
    hdmi_info("margin = %d\n", info->margin);
    hdmi_info("uncompress_support = %s\n", info->uncompress_support ? "TRUE" : "FALSE");
    hdmi_info("canbe_trans = %s\n", info->canbe_trans ? "TRUE" : "FALSE");
    hdmi_info("is_extra_mode = %s\n", info->is_extra_mode ? "TRUE" : "FALSE");

    return;
}
#endif

static hi_void dfm_base_info_init(const dfm_in *in, dfm_info *info)
{
    info->htotal = in->hactive + in->hblank;
    info->vtotal = in->vactive + in->vblank;
    info->pixel_clk = dfm_div((hi_s64)info->htotal * info->vtotal * in->v_freq, HDMI_THOUSAND);

    /* specification assume, c_frl_sb eq 4 * C_FRL_CB + Lanes, in protocol  */
    info->c_frl_sb = FRLCHAR_PER_CB_NUM * 4 + in->lane_num;
    hdmi_if_zero_return_void(info->c_frl_sb);

    info->overhead_sb = in->lane_num * 100000 / info->c_frl_sb;                      /* step1.01 x100000 */
    info->overhead_rs = RS_PER_CB_NUM * CB_PER_SB_NUM * 100000 / info->c_frl_sb;     /* step1.02 x100000 */
    info->overhead_map = 25 * 10000 / info->c_frl_sb; /* 25, 10000: step1.03 x100000, 2.5 FRL map chars per blk. */
    info->overhead_min = info->overhead_sb + info->overhead_rs + info->overhead_map; /* step1.04 x100000 */
    info->overhead_m = OVERHEAD_SIZE;                                                /* step1.05 x100000 */
    info->overhead_max = info->overhead_min + info->overhead_m;                      /* step1.06 x100000 */
    info->max_pixel_clk = dfm_div(info->pixel_clk * (HDMI_THOUSAND + PIXELCLK_TOLERANCE), HDMI_THOUSAND); /* step1.07 */

    hdmi_if_zero_return_void(info->max_pixel_clk);
    info->min_pixel_clk = dfm_div(info->pixel_clk * (HDMI_THOUSAND - PIXELCLK_TOLERANCE), HDMI_THOUSAND);
    hdmi_if_zero_return_void(info->min_pixel_clk);
    info->max_time_line = dfm_div((hi_s64)info->htotal * 1000000000000, info->min_pixel_clk); /* x1000000000000 */
    /* step1.08 x1000000000000 */
    info->min_time_line = dfm_div((hi_s64)info->htotal * 1000000000000, info->max_pixel_clk);
    /* 1000000000,10000: step1.09, Determine the worst-case slow FRL bit rate, bit_rate * (1 - tolerance / 1000000 ) */
    info->min_bit_rate = dfm_div((hi_s64)in->bit_rate * 1000000000 * (10000 - BITRATE_TOLERANCE), 10000); /* x10000 */
    /* 1000000000,10000: x10000, bit_rate calculate 1000000000 / 10000 = 100000bit = 100M */
    info->max_bit_rate = dfm_div((hi_s64)in->bit_rate * 1000000000 * (10000 + BITRATE_TOLERANCE), 10000);
    info->max_frl_char_rate = dfm_div(info->max_bit_rate, 18); /* max_frl_char_rate, max_bit_rate / 18 */
    info->min_frl_char_rate = dfm_div(info->min_bit_rate, 18); /* min_frl_char_rate, min_bit_rate / 18 */

    info->max_frl_chars_per_line = (hi_s32)dfm_div(info->max_time_line *
        info->max_frl_char_rate * in->lane_num, 1000000000000); /* x1000000000000 */
    info->min_frl_chars_per_line = (hi_s32)dfm_div(info->min_time_line *
        info->min_frl_char_rate * in->lane_num, 1000000000000); /* step1.11 x1000000000000 */

    hdmi_if_zero_return_void(info->min_frl_chars_per_line);

    return;
}

static hi_void dfm_borrow_info_init(dfm_info *info)
{
    if (info->active_time_ref >= info->active_time_min && info->blank_time_ref >= info->blank_time_min) {
        info->t_borrow = 0; /* step1.30 */
    } else if (info->active_time_ref < info->active_time_min && info->blank_time_ref >= info->blank_time_min) {
        info->t_borrow = info->active_time_min - info->active_time_ref; /* step1.31 */
    } else {
        info->t_borrow = -1;
    }
    if (info->t_borrow == -1) {
        info->tb_borrow = -1;
    } else {
        /* step1.32 x100000000000, tb_borrow is the number of 3 Bytes required to be transmitted during blk period. */
        info->tb_borrow = ((hi_s32)dfm_div(info->t_borrow * info->avg_tb_rate, 100000000000) + 10 - 1) / 10;
    }

    return;
}

static hi_void dfm_info_init(dfm_in *in, dfm_info *info)
{
    hi_s32 kcd;
    hi_s32 k420;

    kcd = get_kcd(in->pixel_format, in->color_depth);
    k420 = get_k420(in->pixel_format);

    dfm_base_info_init(in, info); /* step1.01 - step1.11 */
    info->audio_ap = dfm_get_audio_ap(in); /* step1.12 */
     /* step1.13 x1000, rap eq audio_rate * (1 + tolerance_audioclock / 1000000) * AP */
    info->audio_rap = (hi_s32)dfm_div((hi_s64)in->audio_rate * (1000 + AUDIOCLK_TOLERANCE) * info->audio_ap, 100000);
    /* step1.14 x1000, 1000000000 is 1000 *1000000, avg_audio_packets is rap * time_line */
    info->avg_audio_packets = (hi_s32)dfm_div((hi_s64)info->audio_rap * info->min_time_line, 1000000000);
    info->audio_packets = (info->avg_audio_packets + HDMI_THOUSAND - 1) / HDMI_THOUSAND; /* step1.15 */
    /* step1.16, hblank_min includes 64(guard bands, two 12-character control periods) + 32 * audio_packets */
    info->blank_audio_min = 64 + 32 * info->audio_packets;
    /* step1.17, 32 is FRL characters each packet, 7 is guard bands island(4 FRL char) + video(3 FRL char) */
    info->c_frl_free = max(((in->hblank * kcd) / k420) / DFM_MAGNIFICATION_8 - 32 * (1 + info->audio_packets) - 7, 0);
    info->c_frl_rc_margin = 4; /* step1.18, add 1 character each for RC break caused by 4 */
    /* step1.19, RC compression transmit control characters reduce 7/8th total characters. */
    info->c_frl_rc_savings = max((7 * info->c_frl_free / DFM_MAGNIFICATION_8 - info->c_frl_rc_margin), 0);
    info->bpp = ((24 * kcd) / k420) / DFM_MAGNIFICATION_8; /* step1.20, dpp eq (24 * kcd) / k420) in protocol */

    info->active_bytes_per_line = info->bpp * in->hactive / DFM_MAGNIFICATION_8; /* step1.21 */
    /* step1.22, 3 is means active_bytes_per_line need 3 characters */
    info->active_tb_per_line = (info->active_bytes_per_line + 3 - 1) / 3;
    info->blank_tb_per_line = (in->hblank * kcd / k420 + DFM_MAGNIFICATION_8 - 1) / DFM_MAGNIFICATION_8; /* step1.23 */
    info->audio_support = (info->blank_tb_per_line >= info->blank_audio_min) ? HI_TRUE : HI_FALSE; /* step1.24 */

    hdmi_if_zero_return_void(info->htotal);
    /* step1.25 */
    info->avg_tb_rate =
        dfm_div(info->max_pixel_clk * (info->active_tb_per_line + info->blank_tb_per_line), info->htotal);
    /* step1.26 x1000000000000 */
    info->active_time_ref = dfm_div((hi_s64)info->min_time_line * in->hactive, info->htotal);
    /* step1.27 x1000000000000 */
    info->blank_time_ref = dfm_div((hi_s64)info->min_time_line * in->hblank, info->htotal);
    /* step1.28, 100000000000000 is 100000000000 * 1000 multiple, (3 / 2) is active_tb coefficient in protocol */
    info->active_time_min = dfm_div((hi_s64)info->active_tb_per_line * 3 * 100000000000000,
        dfm_div(2 * in->lane_num * info->min_frl_char_rate * (RATE_MAGNIFICATION - info->overhead_max), HDMI_THOUSAND));
    /* step1.29 100000000000000 is 100000000000 * 1000 blank_tb_per_line active coefficient multiple */
    info->blank_time_min = dfm_div((hi_s64)info->blank_tb_per_line * 100000000000000,
        dfm_div(in->lane_num * info->min_frl_char_rate * (RATE_MAGNIFICATION - info->overhead_max), HDMI_THOUSAND));
    dfm_borrow_info_init(info); /* step1.30 step1.32 */
    if (info->tb_borrow == -1) {
        info->video_support = HI_FALSE;
    } else {
        info->video_support = (info->tb_borrow <= TB_BORROW_MAX) ? HI_TRUE : HI_FALSE; /* step1.33 */
    }
    /* step1.34, 3, 2: (3 / 2) is active_tb coefficient in protocol */
    info->c_frl_actual_payload =
        (3 * info->active_tb_per_line + 2 - 1) / 2 + info->blank_tb_per_line - info->c_frl_rc_savings;
    /* step1.35  x100000 */
    info->utilization = dfm_div((hi_s64)info->c_frl_actual_payload * 100000, (hi_s64)info->min_frl_chars_per_line);
    info->margin = 100000 - (info->utilization + info->overhead_max); /* step1.36 x100000 */
    info->uncompress_support = (info->margin >= 0) ? HI_TRUE : HI_FALSE; /* step1.37 */
    info->canbe_trans = (info->audio_support && info->video_support && info->uncompress_support) ? HI_TRUE : HI_FALSE;
    info->is_extra_mode = (info->canbe_trans && (info->tb_borrow > 0)) ? HI_TRUE : HI_FALSE;

#ifdef DEBUG
    dfm_info_debug(info);
#endif

    return;
}

hi_bool drv_hdmi_dfm_format_support(dfm_in *dfm)
{
    hi_bool ret = HI_FALSE;
    dfm_info *info = HI_NULL;

    hdmi_if_null_return(dfm, HI_FALSE);
    info = get_dfm_info();
    hdmi_if_null_return(info, HI_FALSE);

    dfm_info_init(dfm, info);
    ret = (info->audio_support && info->video_support && info->uncompress_support) ? HI_TRUE : HI_FALSE;

    return ret;
}


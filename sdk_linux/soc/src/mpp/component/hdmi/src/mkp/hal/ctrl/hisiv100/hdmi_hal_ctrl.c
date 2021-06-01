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
#include "hdmi_hal_ctrl.h"
#include "hdmi_reg_tx.h"
#include "hdmi_reg_audio_path.h"
#include "hdmi_reg_video_path.h"
#include "hdmi_reg_aon.h"
#include "hdmi_reg_ctrl.h"
#include "hdmi_hal_ncts.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_intf.h"
#include "hdmi_reg_crg.h"
#include "hdmi_product_define.h"
#include "hdmi_hal_machine.h"
#include "drv_hdmi_common.h"

#define CTRL_CHANNEL0_Y          0x0
#define CTRL_CHANNEL0_Y422       0x3
#define CTRL_CHANNEL1_CB         0x1
#define CTRL_CHANNEL1_Y422       0x4
#define CTRL_CHANNEL2_CR         0x2
#define CTRL_CHANNEL2_Y422       0x3
#define CTRL_COLORMETRY_OUT_MASK 0xfc
#define CTRL_COLORMETRY_OUT_BIT  0   /* out colormetry offset in reg_csc_mode */
#define CTRL_COLORMETRY_IN_MASK  0xcf
#define CTRL_COLORMETRY_IN_BIT   0x4 /* in colormetry offset in reg_csc_mode */
#define CTRL_COLORMETRY_MASK     0x3
#define CTRL_RGB_OUT_BIT         0x3 /* out color space offset in reg_csc_mode */
#define CTRL_RGB_IN_BIT          0x7 /* in color space offset in reg_csc_mode */
#define CTRL_QUANTIZAION_OUT_BIT 0x2 /* out quantization offset in reg_csc_mode */
#define CTRL_QUANTIZAION_IN_BIT  0x6 /* in quantization offset in reg_csc_mode */
#define CTRL_SYCN_POL_V_BIT      0   /* vsync offset in reg_inver_sync */
#define CTRL_SYCN_POL_H_BIT      1   /* hsync offset in reg_inver_sync */
#define CTRL_SYCN_POL_DE_BIT     0x3 /* DE offset in reg_inver_sync */
#define CTRL_BLACK_Y_CB_CR       0x000000
#define CTRL_BLACK_DATA_YUV_CR   0x200 /* cr val for yuv black */
#define CTRL_BLACK_DATA_YUV_Y    0x40  /* y val for yuv black */
#define CTRL_BLACK_DATA_YUV_CB   0x200 /* cb val for yuv black */
#define CTRL_BLACK_DATA_RGB_R    0x40  /* r val for rgb black */
#define CTRL_BLACK_DATA_RGB_G    0x40  /* g val for rgb black */
#define CTRL_BLACK_DATA_RGB_B    0x40  /* b val for rgb black */
#define CTRL_AUDIO_INVALID_CFG   0xff
#define CTRL_AUDIO_INVALID_RATE  0xffffffff
#define CTRL_REAET_WAIT_TIME     5
#define HDMI_INFOFRAME_DATA_SIZE 31

typedef struct {
    hi_bool ver_down_samp_bypass;
    hi_bool ver_down_samp_enable;
    hi_bool hor_down_samp_enable;
    hi_bool demux_y420_enable;
    hi_bool pxl_div_enable;
    hi_bool hor_filter_enable;
} hdmi_video_down_samp;

typedef enum {
    HDMI_AUDIO_FIFO_MAP_SD0,
    HDMI_AUDIO_FIFO_MAP_SD1,
    HDMI_AUDIO_FIFO_MAP_SD2,
    HDMI_AUDIO_FIFO_MAP_SD3,
    HDMI_AUDIO_FIFO_MAP_BUTT
} hdmi_audio_fifo_map;

typedef enum {
    HDMI_AUDIO_I2S_SD_NONE,
    HDMI_AUDIO_I2S_SD_0,
    HDMI_AUDIO_I2S_SD_2,
    HDMI_AUDIO_I2S_SD_3 = 0x4,
    HDMI_AUDIO_I2S_SD_4 = 0x8,
    HDMI_AUDIO_I2S_SD_ALL = 0xf
} hdmi_audio_i2s_sd;

typedef enum {
    HDMI_AUDIO_HBRA_MASK_ALL,
    HDMI_AUDIO_HBRA_MASK_NONE = 0xf
} hdmi_i2s_hbra_mask;

typedef enum {
    HDMI_AUDIO_CLK_ACCURACY_LEVEL2,
    HDMI_AUDIO_CLK_ACCURACY_LEVEL1,
    HDMI_AUDIO_CLK_ACCURACY_LEVEL3,
    HDMI_AUDIO_CLK_ACCURACY_NOTMATCH
} hdmi_audio_clk_accuracy;

typedef struct {
    hi_bool test;
    hdmi_i2s_hbra_mask hbra_mask;
} ctrl_fifo;

typedef struct {
    hi_bool                 professional_app;
    hi_bool                 compress;
    hi_u32                  samp_rate;
    hi_u32                  org_rate;
    hi_u32                  out_bit_length;
    hdmi_audio_clk_accuracy clk_accuracy;
    hdmi_audio_fifo_map     fifo_map;
} ctrl_i2s_status;

typedef struct {
    hi_bool channel_lr_swap;
    hi_u32  in_bit_length;
    hi_bool vbit_compress;
    hi_bool data_dir_msb_first;
    hi_bool justify_ws_right;
    hi_bool polarity_ws_msb_left;
    hi_bool sck_shift_one;
    hi_bool hbra_enable;
} ctrl_i2s;

typedef struct {
    hi_u32 bit_value;
    hi_u32 bit_reg;
} hdmi_audio_samp_bit;

typedef struct {
    hi_u32 rate_value;
    hi_u32 rate_reg;
    hi_u32 rate_org_reg;
} hdmi_i2s_samp_rate;

typedef struct {
    hi_u32 rate_value;
    hi_u32 rate_reg;
} hdmi_spdif_samp_rate;

typedef struct {
    hi_bool in_rgb;
    hi_bool out_rgb;
    hi_bool csc_enable;
    hi_bool y422_enable;
    hi_bool y420_enable;
    hi_bool dwsm_hori_enable;
    hi_bool dwsm_vert_enable;
} ctrl_colorspace_en;

static hdmi_audio_samp_bit g_samp_bit[] = {
    /* bit reg */
    { 16, 0x2 },
    { 17, 0xc },
    { 18, 0x4 },
    { 19, 0x8 },
    { 20, 0xa },
    { 21, 0xd },
    { 22, 0x5 },
    { 23, 0x9 },
    { 24, 0xb }
};

static hdmi_i2s_samp_rate g_i2s_samp_rate[] = {
    /* sample rate value   reg  org_reg */
    { HDMI_SAMPLE_RATE_32K, 0x03, 0xc },
    { HDMI_SAMPLE_RATE_44K, 0x00, 0xf },
    { HDMI_SAMPLE_RATE_48K, 0x02, 0xd }
};

static hdmi_ctrl_info g_ctrl_info[HDMI_DEVICE_ID_BUTT];

static hdmi_ctrl_info *ctrl_info_ptr_get(hdmi_device_id hdmi)
{
    if (hdmi < HDMI_DEVICE_ID_BUTT) {
        return &g_ctrl_info[hdmi];
    }
    return HI_NULL;
}

static hi_u32 ctrl_i2s_rate_reg_get(hi_u32 rate_value)
{
    hi_u32 i = 0;
    hi_u32 ret = 0x2; /* 48Khz */
    hdmi_i2s_samp_rate *i2s_rate = HI_NULL;

    for (i2s_rate = &g_i2s_samp_rate[0]; i2s_rate && (i < hdmi_array_size(g_i2s_samp_rate)); i2s_rate++, i++) {
        if (i2s_rate->rate_value == rate_value) {
            ret = i2s_rate->rate_reg;
            break;
        }
    }

    if (i >= hdmi_array_size(g_i2s_samp_rate)) {
        hdmi_err("can't find aduio rate = %u, force 48k\n", rate_value);
    }

    return ret;
}

static hi_u32 ctrl_i2s_rate_org_reg_get(hi_u32 rate_value)
{
    hi_u32 i = 0;
    hi_u32 ret = 0xd; /* 48Khz */
    hdmi_i2s_samp_rate *i2s_rate = HI_NULL;

    for (i2s_rate = &g_i2s_samp_rate[0]; i2s_rate && (i < hdmi_array_size(g_i2s_samp_rate)); i2s_rate++, i++) {
        if (i2s_rate->rate_value == rate_value) {
            ret = i2s_rate->rate_org_reg;
            break;
        }
    }

    if (i >= hdmi_array_size(g_i2s_samp_rate)) {
        hdmi_err("can't find aduio rate = %u, force 48k\n", rate_value);
    }

    return ret;
}

static hi_u32 ctrl_i2s_rate_value_get(hi_void)
{
    hi_u32 i = 0;
    hi_u32 rate_reg, rate_org_reg;
    hdmi_i2s_samp_rate *i2s_rate = HI_NULL;

    rate_reg = hdmi_reg_chst_byte3_fs_get();
    rate_org_reg = hdmi_reg_chst_byte4_org_fs_get();

    for (i2s_rate = &g_i2s_samp_rate[0]; i2s_rate && (i < hdmi_array_size(g_i2s_samp_rate)); i2s_rate++, i++) {
        if ((i2s_rate->rate_reg == rate_reg) && (i2s_rate->rate_org_reg == rate_org_reg)) {
            return i2s_rate->rate_value;
        }
    }

    return HDMI_SAMPLE_RATE_UNKNOWN;
}

static hi_u32 ctrl_audio_bit_reg_get(hi_u32 bit_value)
{
    hi_u32 i = 0;
    hi_u32 ret = 0x2; /* 16bit */
    hdmi_audio_samp_bit *audio_bit = HI_NULL;

    for (audio_bit = &g_samp_bit[0]; audio_bit && (i < hdmi_array_size(g_samp_bit)); audio_bit++, i++) {
        if (audio_bit->bit_value == bit_value) {
            ret =  audio_bit->bit_reg;
            break;
        }
    }

    if (i >= hdmi_array_size(g_samp_bit)) {
        hdmi_err("can't find audio bit= %u, force 16\n", bit_value);
    }

    return ret;
}

static hi_u32 ctrl_audio_bit_value_get(hi_bool spdif)
{
    hi_u32 i = 0;
    hi_u32 bit_reg;
    hdmi_audio_samp_bit *audio_bit = HI_NULL;

    if (spdif) {
        bit_reg = hdmi_reg_aud_length_get();
    } else {
        bit_reg = hdmi_reg_chst_byte4_length_get();
    }

    for (audio_bit = &g_samp_bit[0]; audio_bit && (i < hdmi_array_size(g_samp_bit)); audio_bit++, i++) {
        if (audio_bit->bit_reg == bit_reg) {
            return audio_bit->bit_value;
        }
    }

    return HDMI_AUDIO_BIT_DEPTH_UNKNOWN;
}

static hi_void ctrl_audio_mute_set(hi_bool enable)
{
    hdmi_reg_aud_mute_en_set(enable);
    return;
}

static hi_bool ctrl_audio_mute_get(hi_void)
{
    if (hdmi_reg_aud_mute_en_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_audio_i2s_enable_set(hi_bool enable)
{
    hdmi_audio_i2s_sd audio_i2s_enable;
    audio_i2s_enable = enable ? HDMI_AUDIO_I2S_SD_ALL : HDMI_AUDIO_I2S_SD_NONE;
    hdmi_reg_aud_i2s_en_set(audio_i2s_enable);
    return;
}

static hi_bool ctrl_audio_i2s_enable_get(hi_void)
{
    if (hdmi_reg_aud_i2s_en_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_audio_spdif_enable_set(hi_bool spdif_enable)
{
    hdmi_reg_aud_spdif_en_set(spdif_enable);
    return;
}

static hi_bool ctrl_audio_spdif_enable_get(hi_void)
{
    if (hdmi_reg_aud_spdif_en_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_audio_path_enable_set(hi_bool audio_enable)
{
    hdmi_reg_aud_in_en_set(audio_enable);
    return;
}

static hi_bool ctrl_audio_path_enable_get(hi_void)
{
    if (hdmi_reg_aud_in_en_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_audio_path_n_value_set(hi_u32 audio_n)
{
    hdmi_reg_acr_n_val_sw_set(audio_n);
    return;
}

static hi_u32 ctrl_audio_path_n_value_get(hi_void)
{
    return hdmi_reg_acr_n_val_sw_get();
}

static hi_void ctrl_audio_path_cts_value_set(hi_bool sw_cts)
{
    hdmi_reg_acr_cts_hw_sw_sel_set(sw_cts);
    return;
}

static hi_u32 ctrl_audio_path_cts_value_get(hi_void)
{
    hi_u32 cts_value;

    /* wait reg valid */
    osal_msleep(1);
    /* sw */
    if (hdmi_reg_acr_cts_hw_sw_sel_get()) {
        cts_value = hdmi_reg_acr_cts_val_sw_get();
    } else { /* hw */
        cts_value = hdmi_reg_acr_cts_val_hw_get();
    }

    return cts_value;
}

static hi_void ctrl_audio_path_layout_set(hi_bool muti_layout)
{
    hdmi_reg_aud_layout_set(muti_layout);
    return;
}

static hi_u32 ctrl_audio_path_layout_get(hi_void)
{
    return hdmi_reg_aud_layout_get();
}

static hi_void ctrl_audio_hbra_enable_set(hi_bool hbra_enable)
{
    hdmi_reg_i2s_hbra_on_set(hbra_enable);
    return;
}

static hi_bool ctrl_audio_hbra_enable_get(hi_void)
{
    if (hdmi_reg_i2s_hbra_on_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_audio_i2s_ctrl_set(const ctrl_i2s *i2s_ctrl)
{
    hdmi_reg_i2s_1st_shift_set(i2s_ctrl->sck_shift_one);
    hdmi_reg_i2s_ws_polarity_set(i2s_ctrl->polarity_ws_msb_left);
    hdmi_reg_i2s_justify_set(i2s_ctrl->justify_ws_right);
    hdmi_reg_i2s_data_dir_set(i2s_ctrl->data_dir_msb_first);
    hdmi_reg_i2s_vbit_set(i2s_ctrl->vbit_compress);
    hdmi_reg_i2s_length_set(i2s_ctrl->in_bit_length);
    hdmi_reg_i2s_ch_swap_set(i2s_ctrl->channel_lr_swap);

    return;
}

static hi_void ctrl_audio_i2s_status_set(const ctrl_i2s_status *i2s_status)
{
    /* byte0_a config (0: consumer applications, 1: professional applications) */
    hdmi_reg_chst_byte0_aset(i2s_status->professional_app);
    /* byte0_b config (0: PCM, 1:compress) */
    hdmi_reg_chst_byte0_bset(i2s_status->compress);
    hdmi_reg_chst_byte3_fs_set(i2s_status->samp_rate);
    hdmi_reg_chst_byte3_clock_accuracy_set(i2s_status->clk_accuracy);
    hdmi_reg_chst_byte4_org_fs_set(i2s_status->org_rate);
    hdmi_reg_chst_byte4_length_set(i2s_status->out_bit_length);

    return;
}

static hi_void ctrl_audio_fifo_ctrl_set(const ctrl_fifo *fifo_ctrl)
{
    hdmi_reg_aud_fifo_test_set(fifo_ctrl->test);
    hdmi_reg_aud_fifo_hbr_mask_set(fifo_ctrl->hbra_mask);
    return;
}

static hi_void ctrl_audio_reset(hi_void)
{
    hdmi_reg_tx_afifo_srst_req_set(HI_TRUE);
    hdmi_reg_tx_acr_srst_req_set(HI_TRUE);
    hdmi_reg_tx_aud_srst_req_set(HI_TRUE);
    hdmi_reg_tx_afifo_srst_req_set(HI_FALSE);
    hdmi_reg_tx_acr_srst_req_set(HI_FALSE);
    hdmi_reg_tx_aud_srst_req_set(HI_FALSE);

    return;
}

static hi_void ctrl_avmute_set(const hdmi_avmute_cfg *avmute_cfg)
{
    hdmi_reg_cea_cp_en_set(HI_FALSE);
    /* set GCP set mute & clr mute */
    hdmi_reg_cp_set_avmute_set(avmute_cfg->mute_set);
    hdmi_reg_cp_clr_avmute_set(avmute_cfg->mute_clr);
    /* set GCP avmute function enable, sent loop */
    hdmi_reg_cea_cp_rpt_cnt_set(avmute_cfg->rpt_cnt);
    hdmi_reg_cea_cp_rpt_en_set(avmute_cfg->mute_pkg_en);
    hdmi_reg_cea_cp_en_set(avmute_cfg->mute_pkg_en);

    return;
}

static hi_s32 ctrl_avmute_get(hi_bool *enable)
{
    *enable = HI_FALSE;

    if (hdmi_reg_cp_set_avmute_get() || hdmi_reg_cea_cp_rpt_en_get()) {
        if (hdmi_reg_cp_set_avmute_get()) {
            *enable = HI_TRUE;
        }
    }

    return HI_SUCCESS;
}

static hi_void ctrl_video_path_deep_clr_set(hi_bool enable_gcp, hdmi_deep_color deep_color_bit)
{
    hdmi_reg_tmds_pack_mode_set(deep_color_bit);
    hdmi_reg_dc_pkt_en_set(enable_gcp);
    return;
}

static hi_s32 ctrl_video_path_polarity_set(const hdmi_sync_pol_type *out_sync_pol)
{
    hi_u32 sync_pol_cfg = 0;

    if (out_sync_pol->h_pol_inver) {
        hdmi_set_bit(sync_pol_cfg, CTRL_SYCN_POL_H_BIT);
    } else {
        hdmi_clr_bit(sync_pol_cfg, CTRL_SYCN_POL_H_BIT);
    }

    if (out_sync_pol->v_pol_inver) {
        hdmi_set_bit(sync_pol_cfg, CTRL_SYCN_POL_V_BIT);
    } else {
        hdmi_clr_bit(sync_pol_cfg, CTRL_SYCN_POL_V_BIT);
    }

    if (out_sync_pol->de_inver) {
        hdmi_set_bit(sync_pol_cfg, CTRL_SYCN_POL_DE_BIT);
    } else {
        hdmi_clr_bit(sync_pol_cfg, CTRL_SYCN_POL_DE_BIT);
    }

    hdmi_reg_inver_sync_set(sync_pol_cfg);
    hdmi_reg_syncmask_en_set(HI_FALSE);

    return HI_SUCCESS;
}

static hi_void ctrl_video_path_dither_set(hi_bool enable, hdmi_video_dither dither_mode)
{
    hdmi_reg_dither_rnd_bypass_set((!enable));
    hdmi_reg_dither_mode_set(dither_mode);
    return;
}

static hi_void ctrl_video_color_rgb_set(hi_bool rgb_in, hi_bool rgb_out)
{
    hi_u32 reg_value;

    reg_value = hdmi_reg_csc_mode_get();

    if (rgb_in) {
        hdmi_set_bit(reg_value, CTRL_RGB_IN_BIT);
    } else {
        hdmi_clr_bit(reg_value, CTRL_RGB_IN_BIT);
    }

    if (rgb_out) {
        hdmi_set_bit(reg_value, CTRL_RGB_OUT_BIT);
    } else {
        hdmi_clr_bit(reg_value, CTRL_RGB_OUT_BIT);
    }
    hdmi_reg_csc_mode_set(reg_value);

    return;
}

static hi_void ctrl_video_color_csc_set(hi_bool csc_enable)
{
    hdmi_reg_csc_en_set(csc_enable);
    return;
}

static hi_void ctrl_video_color_ycbcr422_set(hi_bool enable)
{
    if (enable) {
        hdmi_reg_vmux_y_sel_set(CTRL_CHANNEL0_Y422);
        hdmi_reg_vmux_cb_sel_set(CTRL_CHANNEL1_Y422);
        hdmi_reg_vmux_cr_sel_set(CTRL_CHANNEL2_Y422);
    } else {
        hdmi_reg_vmux_y_sel_set(CTRL_CHANNEL0_Y);
        hdmi_reg_vmux_cb_sel_set(CTRL_CHANNEL1_CB);
        hdmi_reg_vmux_cr_sel_set(CTRL_CHANNEL2_CR);
    }
    return;
}

static hi_void ctrl_video_color_ycbcr420_set(hi_bool enable)
{
    hdmi_reg_demux_420_en_set(enable);
    hdmi_reg_pxl_div_en_set(enable);
    return;
}

static hi_void ctrl_video_color_dwsm_vert_set(hi_bool dwsm_vert)
{
    hdmi_reg_dwsm_vert_bypass_set(!dwsm_vert);
    hdmi_reg_dwsm_vert_en_set(dwsm_vert);
    return;
}

static hi_void ctrl_video_color_dwsm_hori_set(hi_bool dwsm_hori)
{
    hdmi_reg_hori_filter_en_set(dwsm_hori);
    hdmi_reg_dwsm_hori_en_set(dwsm_hori);
    return;
}

static hi_void ctrl_video_path_colorimetry_set(hdmi_hal_colorimetry in_colormetry, hdmi_hal_colorimetry out_colormetry)
{
    hi_u32 csc_mode;

    csc_mode = hdmi_reg_csc_mode_get();

    out_colormetry = (hi_u32)out_colormetry & CTRL_COLORMETRY_MASK;
    csc_mode &= CTRL_COLORMETRY_OUT_MASK;
    csc_mode |= (hi_u32)out_colormetry << CTRL_COLORMETRY_OUT_BIT;

    in_colormetry = (hi_u32)in_colormetry & CTRL_COLORMETRY_MASK;
    csc_mode &= CTRL_COLORMETRY_IN_MASK;
    csc_mode |= (hi_u32)in_colormetry << CTRL_COLORMETRY_IN_BIT;

    hdmi_reg_csc_mode_set(csc_mode);

    return;
}

static hi_void videopath_colorspace_param_get(hdmi_colorspace in, hdmi_colorspace out, ctrl_colorspace_en *en)
{
    if (in == HDMI_COLORSPACE_YCBCR444) {
        switch (out) {
            case HDMI_COLORSPACE_YCBCR422:
                en->y422_enable = HI_TRUE;
                en->dwsm_hori_enable = HI_TRUE;
                break;
            case HDMI_COLORSPACE_YCBCR420:
                en->dwsm_hori_enable = HI_TRUE;
                en->y420_enable = HI_TRUE;
                en->dwsm_vert_enable = HI_TRUE;
                break;
            case HDMI_COLORSPACE_RGB:
                en->out_rgb = HI_TRUE;
                en->csc_enable = HI_TRUE;
                break;
            default:
                break;
        }
    } else if (in == HDMI_COLORSPACE_RGB) {
        switch (out) {
            case HDMI_COLORSPACE_YCBCR444:
                en->in_rgb = HI_TRUE;
                en->csc_enable = HI_TRUE;
                break;
            case HDMI_COLORSPACE_YCBCR422:
                en->in_rgb = HI_TRUE;
                en->csc_enable = HI_TRUE;
                en->y422_enable = HI_TRUE;
                en->dwsm_hori_enable = HI_TRUE;
                break;
            case HDMI_COLORSPACE_YCBCR420:
                en->in_rgb = HI_TRUE;
                en->csc_enable = HI_TRUE;
                en->dwsm_hori_enable = HI_TRUE;
                en->y420_enable = HI_TRUE;
                en->dwsm_vert_enable = HI_TRUE;
                break;
            default:
                en->in_rgb = HI_TRUE;
                en->out_rgb = HI_TRUE;
                break;
        }
    } else {
        hdmi_warn("un-expected in_color_space=%u\n", in);
    }

    return;
}

static hi_bool ctrl_video_path_color_space_set(hdmi_colorspace in_color_space, hdmi_colorspace out_color_space)
{
    ctrl_colorspace_en colorspace_en = {0};

    videopath_colorspace_param_get(in_color_space, out_color_space, &colorspace_en);

    hdmi_info("ctrl csc set:\n"
              "\tin_rgb=%u, out_rgb=%u, csc_enable=%u \n"
              "\ty422_enable=%u, dwsm_hori_enable=%u \n"
              "\ty420_enable=%u, dwsm_vert_enable=%u \n",
              colorspace_en.in_rgb, colorspace_en.out_rgb, colorspace_en.csc_enable,
              colorspace_en.y422_enable, colorspace_en.dwsm_hori_enable,
              colorspace_en.y420_enable, colorspace_en.dwsm_vert_enable);

    ctrl_video_color_rgb_set(colorspace_en.in_rgb, colorspace_en.out_rgb);
    ctrl_video_color_ycbcr422_set(colorspace_en.y422_enable);
    ctrl_video_color_dwsm_hori_set(colorspace_en.dwsm_hori_enable);
    ctrl_video_color_ycbcr420_set(colorspace_en.y420_enable);
    ctrl_video_color_dwsm_vert_set(colorspace_en.dwsm_vert_enable);
    ctrl_video_color_csc_set(colorspace_en.csc_enable);

    return colorspace_en.csc_enable;
}

static hi_void ctrl_video_path_quantization_set(hdmi_quantization_range in_quantization,
    hdmi_quantization_range out_quantization)
{
    hi_u32 csc_mode;

    csc_mode = hdmi_reg_csc_mode_get();

    if (in_quantization != HDMI_QUANTIZATION_RANGE_LIMITED) {
        hdmi_set_bit(csc_mode, CTRL_QUANTIZAION_IN_BIT);
    } else {
        hdmi_clr_bit(csc_mode, CTRL_QUANTIZAION_IN_BIT);
    }

    if (out_quantization != HDMI_QUANTIZATION_RANGE_LIMITED) {
        hdmi_set_bit(csc_mode, CTRL_QUANTIZAION_OUT_BIT);
    } else {
        hdmi_clr_bit(csc_mode, CTRL_QUANTIZAION_OUT_BIT);
    }

    hdmi_reg_csc_mode_set(csc_mode);
    hdmi_reg_csc_saturate_en_set(HI_TRUE);

    return;
}

static hi_s32 ctrl_vendor_infoframe_en_set(hi_bool enable)
{
    hdmi_reg_cea_vsif_rpt_en_set(enable);
    hdmi_reg_cea_vsif_en_set(enable);
    return HI_SUCCESS;
}

static hi_s32 ctrl_vendor_infoframe_data_set(const hi_u8 *if_data)
{
    /* HB config */
    hdmi_reg_vsif_header_set(if_data[0], if_data[1], if_data[2]);
    /* pkt0 config */
    hdmi_reg_vsif_pkt0_low_set(if_data[3], if_data[4], if_data[5], if_data[6]);
    hdmi_reg_vsif_pkt0_high_set(if_data[7], if_data[8], if_data[9]);
    /* pkt1 config */
    hdmi_reg_vsif_pkt1_low_set(if_data[10], if_data[11], if_data[12], if_data[13]);
    hdmi_reg_vsif_pkt1_high_set(if_data[14], if_data[15], if_data[16]);
    /* pkt2 config */
    hdmi_reg_vsif_pkt2_low_set(if_data[17], if_data[18], if_data[19], if_data[20]);
    hdmi_reg_vsif_pkt2_high_set(if_data[21], if_data[22], if_data[23]);
    /* pkt3 config */
    hdmi_reg_vsif_pkt3_low_set(if_data[24], if_data[25], if_data[26], if_data[27]);
    hdmi_reg_vsif_pkt3_high_set(if_data[28], if_data[29], if_data[30]);

    return HI_SUCCESS;
}

static hi_s32 ctrl_avi_infoframe_en_set(hi_bool enable)
{
    /* repeat enable */
    hdmi_reg_cea_avi_rpt_en_set(enable);
    /* info_frame enable */
    hdmi_reg_cea_avi_en_set(enable);

    return HI_SUCCESS;
}

static hi_s32 ctrl_avi_infoframe_data_set(const hi_u8 *if_data)
{
    /* HB config */
    hdmi_reg_avi_pkt_header_hb_set(if_data[0], if_data[1], if_data[2]);
    hdmi_reg_avi_pkt0_low_set(if_data[3], if_data[4], if_data[5], if_data[6]);
    hdmi_reg_avi_pkt0_high_set(if_data[7], if_data[8], if_data[9]);
    hdmi_reg_avi_pkt1_low_set(if_data[10], if_data[11], if_data[12], if_data[13]);
    hdmi_reg_avi_pkt1_high_set(if_data[14], if_data[15], if_data[16]);
    hdmi_reg_avi_pkt2_low_set(if_data[17], if_data[18], if_data[19], if_data[20]);
    hdmi_reg_avi_pkt2_high_set(if_data[21], if_data[22], if_data[23]);
    hdmi_reg_avi_pkt3_low_set(if_data[24], if_data[25], if_data[26], if_data[27]);
    hdmi_reg_avi_pkt3_high_set(if_data[28], if_data[29], if_data[30]);

    return HI_SUCCESS;
}

static hi_s32 ctrl_tmds_mode_set(hdmi_tmds_mode tmds_mode)
{
    switch (tmds_mode) {
        case HDMI_TMDS_MODE_DVI:
            hdmi_reg_hdmi_mode_set(HI_FALSE);
#if defined(HDMI_PRODUCT_HI3519A)
            /*
             * in case of HDMI_TMDS_MODE_DVI,hdmi_dvi_sel should be set 0,
             * eess_mode_en should be set the default value
             */
            hdmi_reg_hdmi_dvi_sel_set(HI_FALSE);
            hdmi_reg_eess_mode_en_set(HI_TRUE);
#endif
            break;
        case HDMI_TMDS_MODE_HDMI_1_4:
            hdmi_reg_hdmi_mode_set(HI_TRUE);
            hdmi_reg_enc_hdmi2_on_set(HI_FALSE);
#if defined(HDMI_PRODUCT_HI3519A)
            /*
             * in case of HDMI_TMDS_HDMI_1_4,hdmi_dvi_sel should be set 1,
             * eess_mode_en should be set 1
             */
            hdmi_reg_hdmi_dvi_sel_set(HI_TRUE);
            hdmi_reg_eess_mode_en_set(HI_TRUE);
#endif
            break;
        case HDMI_TMDS_MODE_HDMI_2_0:
            hdmi_reg_hdmi_mode_set(HI_TRUE);
            hdmi_reg_enc_hdmi2_on_set(HI_TRUE);
            hdmi_reg_enc_bypass_set(HI_FALSE);
#if defined(HDMI_PRODUCT_HI3519A)
            /*
             * in case of HDMI_TMDS_MODE_HDMI_2_0,hdmi_dvi_sel should be set 1,
             * eess_mode_en should be set 1
             */
            hdmi_reg_hdmi_dvi_sel_set(HI_TRUE);
            hdmi_reg_eess_mode_en_set(HI_TRUE);
#endif
            break;
        default:
            hdmi_warn("un-known tmds mode:%u\n", tmds_mode);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_s32 ctrl_video_path_deep_clr_get(hi_bool *enable_gcp, hdmi_deep_color *deep_color_bit)
{
    *enable_gcp = hdmi_reg_dc_pkt_en_get() ? HI_TRUE : HI_FALSE;
    *deep_color_bit = hdmi_reg_tmds_pack_mode_get();
    return HI_SUCCESS;
}

static hi_s32 ctrl_video_path_polarity_get(hdmi_sync_pol_type *out_sync_pol)
{
    hi_u32 sync_pol_cfg;

    sync_pol_cfg = hdmi_reg_inver_sync_get();
    out_sync_pol->h_pol_inver = is_bit_set(sync_pol_cfg, CTRL_SYCN_POL_H_BIT) ? HI_TRUE : HI_FALSE;
    out_sync_pol->v_pol_inver = is_bit_set(sync_pol_cfg, CTRL_SYCN_POL_V_BIT) ? HI_TRUE : HI_FALSE;
    out_sync_pol->de_inver = is_bit_set(sync_pol_cfg, CTRL_SYCN_POL_DE_BIT) ? HI_TRUE : HI_FALSE;

    return HI_SUCCESS;
}

static hi_s32 ctrl_video_path_dither_get(hi_bool *enable, hdmi_video_dither *dither_mode)
{
    *enable = hdmi_reg_dither_rnd_bypass_get() ? HI_FALSE : HI_TRUE;
    *dither_mode = hdmi_reg_dither_mode_get();
    return HI_SUCCESS;
}

static hi_void ctrl_video_color_rgb_get(hi_bool *rgb_in, hi_bool *rgb_out)
{
    hi_u32 reg_value;

    reg_value = hdmi_reg_csc_mode_get();
    *rgb_in = is_bit_set(reg_value, CTRL_RGB_IN_BIT);
    *rgb_out = is_bit_set(reg_value, CTRL_RGB_OUT_BIT);

    return;
}

static hi_bool ctrl_video_color_csc_get(hi_void)
{
    if (hdmi_reg_csc_en_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_bool ctrl_video_color_ycbcr422_get(hi_void)
{
    if ((hdmi_reg_vmux_y_sel_get() == CTRL_CHANNEL0_Y422) &&
        (hdmi_reg_vmux_cb_sel_get() == CTRL_CHANNEL1_Y422) &&
        (hdmi_reg_vmux_cr_sel_get() == CTRL_CHANNEL2_Y422)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_bool ctrl_video_color_ycbcr420_get(hi_void)
{
    if ((hdmi_reg_demux_420_en_get()) && (hdmi_reg_pxl_div_en_get())) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_bool ctrl_video_color_dwsm_vert_get(hi_void)
{
    if (hdmi_reg_dwsm_vert_en_get() && (!hdmi_reg_dwsm_vert_bypass_get())) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_bool ctrl_video_color_dwsm_hori_get(hi_void)
{
    if (hdmi_reg_hori_filter_en_get() && hdmi_reg_dwsm_hori_en_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_video_path_color_space_get(hdmi_colorspace *in_color_space, hdmi_colorspace *out_color_space)
{
    hi_bool in_rgb = HI_FALSE;
    hi_bool out_rgb = HI_FALSE;

    ctrl_video_color_rgb_get(&in_rgb, &out_rgb);
    *in_color_space = in_rgb ? HDMI_COLORSPACE_RGB : HDMI_COLORSPACE_YCBCR444;
    *out_color_space = out_rgb ? HDMI_COLORSPACE_RGB : HDMI_COLORSPACE_YCBCR444;

    if (ctrl_video_color_ycbcr422_get() && ctrl_video_color_dwsm_hori_get()) {
        *out_color_space = HDMI_COLORSPACE_YCBCR422;
    }

    if (ctrl_video_color_ycbcr420_get() && ctrl_video_color_dwsm_vert_get()) {
        *out_color_space = HDMI_COLORSPACE_YCBCR420;
    }

    return;
}

static hi_s32 ctrl_video_path_colormetry_get(hdmi_hal_colorimetry *in_colormetry, hdmi_hal_colorimetry *out_colormetry)
{
    hi_u32 csc_mode;

    csc_mode = hdmi_reg_csc_mode_get();
    *out_colormetry = (csc_mode & CTRL_COLORMETRY_OUT_MASK) >> CTRL_COLORMETRY_OUT_BIT;
    *in_colormetry = (csc_mode & CTRL_COLORMETRY_OUT_MASK) >> CTRL_COLORMETRY_IN_BIT;

    return HI_SUCCESS;
}

static hi_s32 ctrl_video_path_quantization_get(hdmi_quantization_range *in_quantization,
    hdmi_quantization_range *out_quantization)
{
    hi_u32 csc_mode;

    csc_mode = hdmi_reg_csc_mode_get();
    *in_quantization =
        is_bit_set(csc_mode, CTRL_QUANTIZAION_IN_BIT) ? HDMI_QUANTIZATION_RANGE_FULL : HDMI_QUANTIZATION_RANGE_LIMITED;
    *out_quantization =
        is_bit_set(csc_mode, CTRL_QUANTIZAION_OUT_BIT) ? HDMI_QUANTIZATION_RANGE_FULL : HDMI_QUANTIZATION_RANGE_LIMITED;

    return HI_SUCCESS;
}

static hi_s32 ctrl_tmds_mode_get(hdmi_tmds_mode *tmds_mode)
{
    hi_bool hdmi_mode = HI_FALSE;
    hi_bool hdmi2x_enable = HI_FALSE;

    hdmi_mode = hdmi_reg_hdmi_mode_get();
    hdmi2x_enable = hdmi_reg_enc_hdmi2_on_get();

    if (!hdmi_mode) {
        *tmds_mode = HDMI_TMDS_MODE_DVI;
    } else if (hdmi_mode && (!hdmi2x_enable)) {
        *tmds_mode = HDMI_TMDS_MODE_HDMI_1_4;
    } else if (hdmi_mode && hdmi2x_enable) {
        *tmds_mode = HDMI_TMDS_MODE_HDMI_2_0;
    } else {
        hdmi_warn("un-config tmds mode!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_s32 ctrl_vendor_infoframe_en_get(hi_bool *enable)
{
    *enable = hdmi_reg_cea_vsif_rpt_en_get() ? HI_TRUE : HI_FALSE;
    return HI_SUCCESS;
}

static hi_s32 ctrl_vendor_infoframe_data_get(hi_u8 *if_data)
{
    vsif_pkt_header    vsif_pkt_head;
    vsif_sub_pkt0_low  vsif_sub_pkt0_l;
    vsif_sub_pkt0_high vsif_sub_pkt0_h;
    vsif_sub_pkt1_low  vsif_sub_pkt1_l;
    vsif_sub_pkt1_high vsif_sub_pkt1_h;
    vsif_sub_pkt2_low  vsif_sub_pkt2_l;
    vsif_sub_pkt2_high vsif_sub_pkt2_h;
    vsif_sub_pkt3_low  vsif_sub_pkt3_l;
    vsif_sub_pkt3_high vsif_sub_pkt3_h;

    hdmi_reg_vsif_pkt_header_get(&vsif_pkt_head);
    hdmi_reg_vsif_pkt0_low_get(&vsif_sub_pkt0_l);
    hdmi_reg_vsif_pkt0_high_get(&vsif_sub_pkt0_h);
    hdmi_reg_vsif_pkt1_low_get(&vsif_sub_pkt1_l);
    hdmi_reg_vsif_pkt1_high_get(&vsif_sub_pkt1_h);
    hdmi_reg_vsif_pkt2_low_get(&vsif_sub_pkt2_l);
    hdmi_reg_vsif_pkt2_high_get(&vsif_sub_pkt2_h);
    hdmi_reg_vsif_pkt3_low_get(&vsif_sub_pkt3_l);
    hdmi_reg_vsif_pkt3_high_get(&vsif_sub_pkt3_h);

    if_data[0] = vsif_pkt_head.bits.vsif_pkt_hb0;
    if_data[1] = vsif_pkt_head.bits.vsif_pkt_hb1;
    if_data[2] = vsif_pkt_head.bits.vsif_pkt_hb2;
    if_data[3] = vsif_sub_pkt0_l.bits.vsif_sub_pkt0_pb0;
    if_data[4] = vsif_sub_pkt0_l.bits.vsif_sub_pkt0_pb1;
    if_data[5] = vsif_sub_pkt0_l.bits.vsif_sub_pkt0_pb2;
    if_data[6] = vsif_sub_pkt0_l.bits.vsif_sub_pkt0_pb3;
    if_data[7] = vsif_sub_pkt0_h.bits.vsif_sub_pkt0_pb4;
    if_data[8] = vsif_sub_pkt0_h.bits.vsif_sub_pkt0_pb5;
    if_data[9] = vsif_sub_pkt0_h.bits.vsif_sub_pkt0_pb6;
    if_data[10] = vsif_sub_pkt1_l.bits.vsif_sub_pkt1_pb0;
    if_data[11] = vsif_sub_pkt1_l.bits.vsif_sub_pkt1_pb1;
    if_data[12] = vsif_sub_pkt1_l.bits.vsif_sub_pkt1_pb2;
    if_data[13] = vsif_sub_pkt1_l.bits.vsif_sub_pkt1_pb3;
    if_data[14] = vsif_sub_pkt1_h.bits.vsif_sub_pkt1_pb4;
    if_data[15] = vsif_sub_pkt1_h.bits.vsif_sub_pkt1_pb5;
    if_data[16] = vsif_sub_pkt1_h.bits.vsif_sub_pkt1_pb6;
    if_data[17] = vsif_sub_pkt2_l.bits.vsif_sub_pkt2_pb0;
    if_data[18] = vsif_sub_pkt2_l.bits.vsif_sub_pkt2_pb1;
    if_data[19] = vsif_sub_pkt2_l.bits.vsif_sub_pkt2_pb2;
    if_data[20] = vsif_sub_pkt2_l.bits.vsif_sub_pkt2_pb3;
    if_data[21] = vsif_sub_pkt2_h.bits.vsif_sub_pkt2_pb4;
    if_data[22] = vsif_sub_pkt2_h.bits.vsif_sub_pkt2_pb5;
    if_data[23] = vsif_sub_pkt2_h.bits.vsif_sub_pkt2_pb6;
    if_data[24] = vsif_sub_pkt3_l.bits.vsif_sub_pkt3_pb0;
    if_data[25] = vsif_sub_pkt3_l.bits.vsif_sub_pkt3_pb1;
    if_data[26] = vsif_sub_pkt3_l.bits.vsif_sub_pkt3_pb2;
    if_data[27] = vsif_sub_pkt3_l.bits.vsif_sub_pkt3_pb3;
    if_data[28] = vsif_sub_pkt3_h.bits.vsif_sub_pkt3_pb4;
    if_data[29] = vsif_sub_pkt3_h.bits.vsif_sub_pkt3_pb5;
    if_data[30] = vsif_sub_pkt3_h.bits.vsif_sub_pkt3_pb6;

    return HI_SUCCESS;
}

static hi_s32 ctrl_avi_infoframe_en_get(hi_bool *enable)
{
    *enable = hdmi_reg_cea_avi_en_get() ? HI_TRUE : HI_FALSE;
    return HI_SUCCESS;
}

static hi_s32 ctrl_avi_infoframe_data_get(hi_u8 *if_data)
{
    avi_pkt_header    avi_pkt_head;
    avi_sub_pkt0_low  avi_sub_pkt0_l;
    avi_sub_pkt0_high avi_sub_pkt0_h;
    avi_sub_pkt1_low  avi_sub_pkt1_l;
    avi_sub_pkt1_high avi_sub_pkt1_h;
    avi_sub_pkt2_low  avi_sub_pkt2_l;
    avi_sub_pkt2_high avi_sub_pkt2_h;
    avi_sub_pkt3_low  avi_sub_pkt3_l;
    avi_sub_pkt3_high avi_sub_pkt3_h;

    hdmi_reg_avi_pkt_header_hb_get(&avi_pkt_head);
    hdmi_reg_avi_pkt0_low_get(&avi_sub_pkt0_l);
    hdmi_reg_avi_pkt0_high_get(&avi_sub_pkt0_h);
    hdmi_reg_avi_pkt1_low_get(&avi_sub_pkt1_l);
    hdmi_reg_avi_pkt1_high_get(&avi_sub_pkt1_h);
    hdmi_reg_avi_pkt2_low_get(&avi_sub_pkt2_l);
    hdmi_reg_avi_pkt2_high_get(&avi_sub_pkt2_h);
    hdmi_reg_avi_pkt3_low_get(&avi_sub_pkt3_l);
    hdmi_reg_avi_pkt3_high_get(&avi_sub_pkt3_h);

    if_data[0] = avi_pkt_head.bits.avi_pkt_hb0;
    if_data[1] = avi_pkt_head.bits.avi_pkt_hb1;
    if_data[2] = avi_pkt_head.bits.avi_pkt_hb2;
    if_data[3] = avi_sub_pkt0_l.bits.avi_sub_pkt0_pb0;
    if_data[4] = avi_sub_pkt0_l.bits.avi_sub_pkt0_pb1;
    if_data[5] = avi_sub_pkt0_l.bits.avi_sub_pkt0_pb2;
    if_data[6] = avi_sub_pkt0_l.bits.avi_sub_pkt0_pb3;
    if_data[7] = avi_sub_pkt0_h.bits.avi_sub_pkt0_pb4;
    if_data[8] = avi_sub_pkt0_h.bits.avi_sub_pkt0_pb5;
    if_data[9] = avi_sub_pkt0_h.bits.avi_sub_pkt0_pb6;
    if_data[10] = avi_sub_pkt1_l.bits.avi_sub_pkt1_pb0;
    if_data[11] = avi_sub_pkt1_l.bits.avi_sub_pkt1_pb1;
    if_data[12] = avi_sub_pkt1_l.bits.avi_sub_pkt1_pb2;
    if_data[13] = avi_sub_pkt1_l.bits.avi_sub_pkt1_pb3;
    if_data[14] = avi_sub_pkt1_h.bits.avi_sub_pkt1_pb4;
    if_data[15] = avi_sub_pkt1_h.bits.avi_sub_pkt1_pb5;
    if_data[16] = avi_sub_pkt1_h.bits.avi_sub_pkt1_pb6;
    if_data[17] = avi_sub_pkt2_l.bits.avi_sub_pkt2_pb0;
    if_data[18] = avi_sub_pkt2_l.bits.avi_sub_pkt2_pb1;
    if_data[19] = avi_sub_pkt2_l.bits.avi_sub_pkt2_pb2;
    if_data[20] = avi_sub_pkt2_l.bits.avi_sub_pkt2_pb3;
    if_data[21] = avi_sub_pkt2_h.bits.avi_sub_pkt2_pb4;
    if_data[22] = avi_sub_pkt2_h.bits.avi_sub_pkt2_pb5;
    if_data[23] = avi_sub_pkt2_h.bits.avi_sub_pkt2_pb6;
    if_data[24] = avi_sub_pkt3_l.bits.avi_sub_pkt3_pb0;
    if_data[25] = avi_sub_pkt3_l.bits.avi_sub_pkt3_pb1;
    if_data[26] = avi_sub_pkt3_l.bits.avi_sub_pkt3_pb2;
    if_data[27] = avi_sub_pkt3_l.bits.avi_sub_pkt3_pb3;
    if_data[28] = avi_sub_pkt3_h.bits.avi_sub_pkt3_pb4;
    if_data[29] = avi_sub_pkt3_h.bits.avi_sub_pkt3_pb5;
    if_data[30] = avi_sub_pkt3_h.bits.avi_sub_pkt3_pb6;

    return HI_SUCCESS;
}

static hi_s32 ctrl_timming_decect_get(ctrl_timming_detect *timing)
{
    timing->sync_sw_enable = HI_FALSE;
    hdmi_reg_sync_polarity_force_set(timing->sync_sw_enable);

    timing->vsync_polarity = hdmi_reg_vsync_polarity_get() ? HI_TRUE : HI_FALSE;
    timing->hsync_polarity = hdmi_reg_hsync_polarity_get() ? HI_TRUE : HI_FALSE;
    timing->progressive    = hdmi_reg_interlaced_get() ? HI_FALSE : HI_TRUE;
    timing->hsync_total    = hdmi_reg_hsync_total_cnt_get();
    timing->hactive_cnt    = hdmi_reg_hsync_active_cnt_get();
    timing->vsync_total    = hdmi_reg_vsync_total_cnt_get();
    timing->vactive_cnt    = hdmi_reg_vsync_active_cnt_get();

    /* clear format detect status */
    hdmi_reg_fdt_status_clear_set(HI_FALSE);
    hdmi_reg_fdt_status_clear_set(HI_TRUE);

    return HI_SUCCESS;
}

static hi_void ctrl_video_mute_set(hi_bool enable)
{
    hi_u32 data_value;
    hdmi_colorspace in_color_space = HDMI_COLORSPACE_RGB;
    hdmi_colorspace out_color_space = HDMI_COLORSPACE_RGB;

    ctrl_video_path_color_space_get(&in_color_space, &out_color_space);

    data_value = (in_color_space == HDMI_COLORSPACE_RGB) ? CTRL_BLACK_DATA_RGB_R : CTRL_BLACK_DATA_YUV_CR;
    hdmi_reg_solid_pattern_cr_set(data_value);

    data_value = (in_color_space == HDMI_COLORSPACE_RGB) ? CTRL_BLACK_DATA_RGB_G : CTRL_BLACK_DATA_YUV_Y;
    hdmi_reg_solid_pattern_y_set(data_value);

    data_value = (in_color_space == HDMI_COLORSPACE_RGB) ? CTRL_BLACK_DATA_RGB_B : CTRL_BLACK_DATA_YUV_CB;
    hdmi_reg_solid_pattern_cb_set(data_value);

    hdmi_reg_video_blank_en_set(enable);
    hdmi_reg_solid_pattern_en_set(enable);

    return;
}

static hi_s32 ctrl_video_mute_get(hi_bool *enable)
{
    if (hdmi_reg_video_blank_en_get() && hdmi_reg_solid_pattern_en_get()) {
        *enable = HI_TRUE;
    } else {
        *enable = HI_FALSE;
    }

    return HI_SUCCESS;
}

static hi_s32 ctrl_audio_infoframe_en_set(hi_bool enable)
{
    /* repeat enable */
    hdmi_reg_cea_aud_rpt_en_set(enable);
    /* info_frame enable */
    hdmi_reg_cea_aud_en_set(enable);
    return HI_SUCCESS;
}

static hi_s32 ctrl_audio_infoframe_en_get(hi_bool *enable)
{
    *enable = hdmi_reg_cea_aud_en_get() ? HI_TRUE : HI_FALSE;
    return HI_SUCCESS;
}

static hi_s32 ctrl_audio_infoframe_data_set(const hi_u8 *if_data)
{
    hdmi_reg_audio_pkt_header_set(if_data[0], if_data[1], if_data[2]);
    hdmi_reg_audio_pkt0_low_set(if_data[3], if_data[4], if_data[5], if_data[6]);
    hdmi_reg_audio_pkt0_high_set(if_data[7], if_data[8], if_data[9]);
    hdmi_reg_audio_pkt1_low_set(if_data[10], if_data[11], if_data[12], if_data[13]);
    hdmi_reg_audio_pkt1_high_set(if_data[14], if_data[15], if_data[16]);
    hdmi_reg_audio_pkt2_low_set(if_data[17], if_data[18], if_data[19], if_data[20]);
    hdmi_reg_audio_pkt2_high_set(if_data[21], if_data[22], if_data[23]);
    hdmi_reg_audio_pkt3_low_set(if_data[24], if_data[25], if_data[26], if_data[27]);
    hdmi_reg_audio_pkt3_high_set(if_data[28], if_data[29], if_data[30]);

    return HI_SUCCESS;
}

static hi_s32 ctrl_audio_infoframe_data_get(hi_u8 *if_data)
{
    aif_pkt_header    aif_pkt_head;
    aif_sub_pkt0_low  aif_sub_pkt0_l;
    aif_sub_pkt0_high aif_sub_pkt0_h;
    aif_sub_pkt1_low  aif_sub_pkt1_l;
    aif_sub_pkt1_high aif_sub_pkt1_h;
    aif_sub_pkt2_low  aif_sub_pkt2_l;
    aif_sub_pkt2_high aif_sub_pkt2_h;
    aif_sub_pkt3_low  aif_sub_pkt3_l;
    aif_sub_pkt3_high aif_sub_pkt3_h;

    hdmi_reg_aif_pkt_header_get(&aif_pkt_head);
    hdmi_reg_aif_pkt0_low_get(&aif_sub_pkt0_l);
    hdmi_reg_aif_pkt0_high_get(&aif_sub_pkt0_h);
    hdmi_reg_aif_pkt1_low_get(&aif_sub_pkt1_l);
    hdmi_reg_aif_pkt1_high_get(&aif_sub_pkt1_h);
    hdmi_reg_aif_pkt2_low_get(&aif_sub_pkt2_l);
    hdmi_reg_aif_pkt2_high_get(&aif_sub_pkt2_h);
    hdmi_reg_aif_pkt3_low_get(&aif_sub_pkt3_l);
    hdmi_reg_aif_pkt3_high_get(&aif_sub_pkt3_h);

    if_data[0] = aif_pkt_head.bits.aif_pkt_hb0;
    if_data[1] = aif_pkt_head.bits.aif_pkt_hb1;
    if_data[2] = aif_pkt_head.bits.aif_pkt_hb2;
    if_data[3] = aif_sub_pkt0_l.bits.aif_sub_pkt0_pb0;
    if_data[4] = aif_sub_pkt0_l.bits.aif_sub_pkt0_pb1;
    if_data[5] = aif_sub_pkt0_l.bits.aif_sub_pkt0_pb2;
    if_data[6] = aif_sub_pkt0_l.bits.aif_sub_pkt0_pb3;
    if_data[7] = aif_sub_pkt0_h.bits.aif_sub_pkt0_pb4;
    if_data[8] = aif_sub_pkt0_h.bits.aif_sub_pkt0_pb5;
    if_data[9] = aif_sub_pkt0_h.bits.aif_sub_pkt0_pb6;
    if_data[10] = aif_sub_pkt1_l.bits.aif_sub_pkt1_pb0;
    if_data[11] = aif_sub_pkt1_l.bits.aif_sub_pkt1_pb1;
    if_data[12] = aif_sub_pkt1_l.bits.aif_sub_pkt1_pb2;
    if_data[13] = aif_sub_pkt1_l.bits.aif_sub_pkt1_pb3;
    if_data[14] = aif_sub_pkt1_h.bits.aif_sub_pkt1_pb4;
    if_data[15] = aif_sub_pkt1_h.bits.aif_sub_pkt1_pb5;
    if_data[16] = aif_sub_pkt1_h.bits.aif_sub_pkt1_pb6;
    if_data[17] = aif_sub_pkt2_l.bits.aif_sub_pkt2_pb0;
    if_data[18] = aif_sub_pkt2_l.bits.aif_sub_pkt2_pb1;
    if_data[19] = aif_sub_pkt2_l.bits.aif_sub_pkt2_pb2;
    if_data[20] = aif_sub_pkt2_l.bits.aif_sub_pkt2_pb3;
    if_data[21] = aif_sub_pkt2_h.bits.aif_sub_pkt2_pb4;
    if_data[22] = aif_sub_pkt2_h.bits.aif_sub_pkt2_pb5;
    if_data[23] = aif_sub_pkt2_h.bits.aif_sub_pkt2_pb6;
    if_data[24] = aif_sub_pkt3_l.bits.aif_sub_pkt3_pb0;
    if_data[25] = aif_sub_pkt3_l.bits.aif_sub_pkt3_pb1;
    if_data[26] = aif_sub_pkt3_l.bits.aif_sub_pkt3_pb2;
    if_data[27] = aif_sub_pkt3_l.bits.aif_sub_pkt3_pb3;
    if_data[28] = aif_sub_pkt3_h.bits.aif_sub_pkt3_pb4;
    if_data[29] = aif_sub_pkt3_h.bits.aif_sub_pkt3_pb5;
    if_data[30] = aif_sub_pkt3_h.bits.aif_sub_pkt3_pb6;

    return HI_SUCCESS;
}

static hi_s32 ctrl_gbd_infoframe_en_set(hi_bool enable)
{
    hdmi_reg_cea_gamut_rpt_en_set(enable);
    hdmi_reg_cea_gamut_en_set(enable);
    return HI_SUCCESS;
}

static hi_s32 ctrl_gbd_infoframe_en_get(hi_bool *enable)
{
    *enable = hdmi_reg_cea_gamut_en_get() ? HI_TRUE : HI_FALSE;
    return HI_SUCCESS;
}

static hi_s32 ctrl_gbd_infoframe_data_get(hi_u8 *if_data)
{
    gamut_pkt_header    gamut_pkt_head;
    gamut_sub_pkt0_low  gamut_sub_pkt0_l;
    gamut_sub_pkt0_high gamut_sub_pkt0_h;
    gamut_sub_pkt1_low  gamut_sub_pkt1_l;
    gamut_sub_pkt1_high gamut_sub_pkt1_h;
    gamut_sub_pkt2_low  gamut_sub_pkt2_l;
    gamut_sub_pkt2_high gamut_sub_pkt2_h;
    gamut_sub_pkt3_low  gamut_sub_pkt3_l;
    gamut_sub_pkt3_high gamut_sub_pkt3_h;

    hdmi_reg_gamut_pkt_header_get(&gamut_pkt_head);
    hdmi_reg_gamut_pkt0_low_get(&gamut_sub_pkt0_l);
    hdmi_reg_gamut_pkt0_high_get(&gamut_sub_pkt0_h);
    hdmi_reg_gamut_pkt1_low_get(&gamut_sub_pkt1_l);
    hdmi_reg_gamut_pkt1_high_get(&gamut_sub_pkt1_h);
    hdmi_reg_gamut_pkt2_low_get(&gamut_sub_pkt2_l);
    hdmi_reg_gamut_pkt2_high_get(&gamut_sub_pkt2_h);
    hdmi_reg_gamut_pkt3_low_get(&gamut_sub_pkt3_l);
    hdmi_reg_gamut_pkt3_high_get(&gamut_sub_pkt3_h);

    if_data[0] = gamut_pkt_head.bits.gamut_pkt_hb0;
    if_data[1] = gamut_pkt_head.bits.gamut_pkt_hb1;
    if_data[2] = gamut_pkt_head.bits.gamut_pkt_hb2;
    if_data[3] = gamut_sub_pkt0_l.bits.gamut_sub_pkt0_pb0;
    if_data[4] = gamut_sub_pkt0_l.bits.gamut_sub_pkt0_pb1;
    if_data[5] = gamut_sub_pkt0_l.bits.gamut_sub_pkt0_pb2;
    if_data[6] = gamut_sub_pkt0_l.bits.gamut_sub_pkt0_pb3;
    if_data[7] = gamut_sub_pkt0_h.bits.gamut_sub_pkt0_pb4;
    if_data[8] = gamut_sub_pkt0_h.bits.gamut_sub_pkt0_pb5;
    if_data[9] = gamut_sub_pkt0_h.bits.gamut_sub_pkt0_pb6;
    if_data[10] = gamut_sub_pkt1_l.bits.gamut_sub_pkt1_pb0;
    if_data[11] = gamut_sub_pkt1_l.bits.gamut_sub_pkt1_pb1;
    if_data[12] = gamut_sub_pkt1_l.bits.gamut_sub_pkt1_pb2;
    if_data[13] = gamut_sub_pkt1_l.bits.gamut_sub_pkt1_pb3;
    if_data[14] = gamut_sub_pkt1_h.bits.gamut_sub_pkt1_pb4;
    if_data[15] = gamut_sub_pkt1_h.bits.gamut_sub_pkt1_pb5;
    if_data[16] = gamut_sub_pkt1_h.bits.gamut_sub_pkt1_pb6;
    if_data[17] = gamut_sub_pkt2_l.bits.gamut_sub_pkt2_pb0;
    if_data[18] = gamut_sub_pkt2_l.bits.gamut_sub_pkt2_pb1;
    if_data[19] = gamut_sub_pkt2_l.bits.gamut_sub_pkt2_pb2;
    if_data[20] = gamut_sub_pkt2_l.bits.gamut_sub_pkt2_pb3;
    if_data[21] = gamut_sub_pkt2_h.bits.gamut_sub_pkt2_pb4;
    if_data[22] = gamut_sub_pkt2_h.bits.gamut_sub_pkt2_pb5;
    if_data[23] = gamut_sub_pkt2_h.bits.gamut_sub_pkt2_pb6;
    if_data[24] = gamut_sub_pkt3_l.bits.gamut_sub_pkt3_pb0;
    if_data[25] = gamut_sub_pkt3_l.bits.gamut_sub_pkt3_pb1;
    if_data[26] = gamut_sub_pkt3_l.bits.gamut_sub_pkt3_pb2;
    if_data[27] = gamut_sub_pkt3_l.bits.gamut_sub_pkt3_pb3;
    if_data[28] = gamut_sub_pkt3_h.bits.gamut_sub_pkt3_pb4;
    if_data[29] = gamut_sub_pkt3_h.bits.gamut_sub_pkt3_pb5;
    if_data[30] = gamut_sub_pkt3_h.bits.gamut_sub_pkt3_pb6;

    return HI_SUCCESS;
}

static hi_s32 ctrl_drm_infoframe_en_set(hi_bool enable)
{
    hdmi_reg_cea_gen_rpt_en_set(enable);
    hdmi_reg_cea_gen_en_set(enable);
    return HI_SUCCESS;
}

static hi_s32 ctrl_drm_infoframe_en_get(hi_bool *enable)
{
    *enable = hdmi_reg_cea_gen_en_get() ? HI_TRUE : HI_FALSE;
    return HI_SUCCESS;
}

static hi_s32 ctrl_drm_infoframe_data_set(const hi_u8 *if_data)
{
    hdmi_reg_gen_pkt_header_set(if_data[0], if_data[1], if_data[2]);
    hdmi_reg_gen_pkt0_low_set(if_data[3], if_data[4], if_data[5], if_data[6]);
    hdmi_reg_gen_pkt0_high_set(if_data[7], if_data[8], if_data[9]);
    hdmi_reg_gen_pkt1_low_set(if_data[10], if_data[11], if_data[12], if_data[13]);
    hdmi_reg_gen_pkt1_high_set(if_data[14], if_data[15], if_data[16]);
    hdmi_reg_gen_pkt2_low_set(if_data[17], if_data[18], if_data[19], if_data[20]);
    hdmi_reg_gen_pkt2_high_set(if_data[21], if_data[22], if_data[23]);
    hdmi_reg_gen_pkt3_low_set(if_data[24], if_data[25], if_data[26], if_data[27]);
    hdmi_reg_gen_pkt3_high_set(if_data[28], if_data[29], if_data[30]);

    return HI_SUCCESS;
}

static hi_s32 ctrl_drm_infoframe_data_get(hi_u8 *if_data)
{
    gen_pkt_header    gen_pkt_head;
    gen_sub_pkt0_low  gen_sub_pkt0_l;
    gen_sub_pkt0_high gen_sub_pkt0_h;
    gen_sub_pkt1_low  gen_sub_pkt1_l;
    gen_sub_pkt1_high gen_sub_pkt1_h;
    gen_sub_pkt2_low  gen_sub_pkt2_l;
    gen_sub_pkt2_high gen_sub_pkt2_h;
    gen_sub_pkt3_low  gen_sub_pkt3_l;
    gen_sub_pkt3_high gen_sub_pkt3_h;

    hdmi_reg_gen_pkt_header_get(&gen_pkt_head);
    hdmi_reg_gen_pkt0_low_get(&gen_sub_pkt0_l);
    hdmi_reg_gen_pkt0_high_get(&gen_sub_pkt0_h);
    hdmi_reg_gen_pkt1_low_get(&gen_sub_pkt1_l);
    hdmi_reg_gen_pkt1_high_get(&gen_sub_pkt1_h);
    hdmi_reg_gen_pkt2_low_get(&gen_sub_pkt2_l);
    hdmi_reg_gen_pkt2_high_get(&gen_sub_pkt2_h);
    hdmi_reg_gen_pkt3_low_get(&gen_sub_pkt3_l);
    hdmi_reg_gen_pkt3_high_get(&gen_sub_pkt3_h);

    if_data[0] = gen_pkt_head.bits.gen_pkt_hb0;
    if_data[1] = gen_pkt_head.bits.gen_pkt_hb1;
    if_data[2] = gen_pkt_head.bits.gen_pkt_hb2;
    if_data[3] = gen_sub_pkt0_l.bits.gen_sub_pkt0_pb0;
    if_data[4] = gen_sub_pkt0_l.bits.gen_sub_pkt0_pb1;
    if_data[5] = gen_sub_pkt0_l.bits.gen_sub_pkt0_pb2;
    if_data[6] = gen_sub_pkt0_l.bits.gen_sub_pkt0_pb3;
    if_data[7] = gen_sub_pkt0_h.bits.gen_sub_pkt0_pb4;
    if_data[8] = gen_sub_pkt0_h.bits.gen_sub_pkt0_pb5;
    if_data[9] = gen_sub_pkt0_h.bits.gen_sub_pkt0_pb6;
    if_data[10] = gen_sub_pkt1_l.bits.gen_sub_pkt1_pb0;
    if_data[11] = gen_sub_pkt1_l.bits.gen_sub_pkt1_pb1;
    if_data[12] = gen_sub_pkt1_l.bits.gen_sub_pkt1_pb2;
    if_data[13] = gen_sub_pkt1_l.bits.gen_sub_pkt1_pb3;
    if_data[14] = gen_sub_pkt1_h.bits.gen_sub_pkt1_pb4;
    if_data[15] = gen_sub_pkt1_h.bits.gen_sub_pkt1_pb5;
    if_data[16] = gen_sub_pkt1_h.bits.gen_sub_pkt1_pb6;
    if_data[17] = gen_sub_pkt2_l.bits.gen_sub_pkt2_pb0;
    if_data[18] = gen_sub_pkt2_l.bits.gen_sub_pkt2_pb1;
    if_data[19] = gen_sub_pkt2_l.bits.gen_sub_pkt2_pb2;
    if_data[20] = gen_sub_pkt2_l.bits.gen_sub_pkt2_pb3;
    if_data[21] = gen_sub_pkt2_h.bits.gen_sub_pkt2_pb4;
    if_data[22] = gen_sub_pkt2_h.bits.gen_sub_pkt2_pb5;
    if_data[23] = gen_sub_pkt2_h.bits.gen_sub_pkt2_pb6;
    if_data[24] = gen_sub_pkt3_l.bits.gen_sub_pkt3_pb0;
    if_data[25] = gen_sub_pkt3_l.bits.gen_sub_pkt3_pb1;
    if_data[26] = gen_sub_pkt3_l.bits.gen_sub_pkt3_pb2;
    if_data[27] = gen_sub_pkt3_l.bits.gen_sub_pkt3_pb3;
    if_data[28] = gen_sub_pkt3_h.bits.gen_sub_pkt3_pb4;
    if_data[29] = gen_sub_pkt3_h.bits.gen_sub_pkt3_pb5;
    if_data[30] = gen_sub_pkt3_h.bits.gen_sub_pkt3_pb6;

    return HI_SUCCESS;
}

static hi_bool ctrl_hpd_get(hi_void)
{
    hi_s32 hot_plug;
    hi_bool hpd = HI_FALSE;

    hot_plug = hdmi_reg_hotplug_state_get();

    if (hdmi_reg_hpd_polarity_ctl_get() > 0) {
        hpd = (hot_plug > 0) ? HI_FALSE : HI_TRUE;
    } else {
        hpd = (hot_plug > 0) ? HI_TRUE : HI_FALSE;
    }

    return hpd;
}

static hi_bool ctrl_rsen_get(hi_void)
{
    if (hdmi_reg_phy_rx_sense_get()) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

static hi_void ctrl_hpd_intr_enable(hi_bool enable)
{
    hdmi_reg_aon_intr_stat0_set(HI_TRUE);
    hdmi_reg_aon_intr_mask0_set(enable);
    return;
}

#ifdef HDMI_HDR_SUPPORT
static hi_void ctrl_hpd_timer_invoke(const hdmi_ctrl_info *ctrl_info)
{
    hi_u32 i;
    hi_u64 timeout;
    ctrl_hdr_timer *hdr_timer = HI_NULL;

    for (i = 0; i < HDMI_TIMER_TYPE_BUTT; i++) {
        hdr_timer = &ctrl_info->hdr_timer[i];
        if (hdr_timer->enable != HI_TRUE) {
            continue;
        }
        timeout = hal_hdmi_mach_ms_get() - hdr_timer->start_time;
        if (timeout >= hdr_timer->time_length) {
            hdr_timer->enable = HI_FALSE;
            hdmi_info("ctrl timer %u disable!\n", hdr_timer->timer_type);
            switch (hdr_timer->timer_type) {
                case HDMI_TIMER_ZERO_DRMIF:
                    ctrl_info->event_callback(ctrl_info->event_data, HDMI_EVENT_ZERO_DRMIF_TIMEOUT);
                    break;
                case HDMI_TIMER_SDR_TO_HDR10:
                    ctrl_info->event_callback(ctrl_info->event_data, HDMI_EVENT_SWITCH_TO_HDRMODE_TIMEOUT);
                    break;
                default:
                    break;
            }
        }
    }

    return;
}
#endif

hi_void ctrl_mach_ppd_callback(hi_void *data)
{
    hi_bool event = HI_FALSE;
    hdmi_ctrl_info *ctrl_info = (hdmi_ctrl_info *)data;

    hdmi_if_null_return_void(data);
    hdmi_if_null_return_void(ctrl_info->event_data);
    hdmi_if_null_return_void(ctrl_info->event_callback);

    if (hdmi_reg_aon_intr_stat0_get()) {
        /* clr intr */
        hdmi_reg_aon_intr_stat0_set(HI_TRUE);
        /* event */
        event = ctrl_hpd_get();
        if (event != ctrl_info->hpd) {
            ctrl_info->event_callback(ctrl_info->event_data,
                (event == HI_TRUE) ? HDMI_EVENT_HOTPLUG : HDMI_EVENT_HOTUNPLUG);
            ctrl_info->hpd = event;
        } else {
            hdmi_warn("HPD event not change: %u!\n", event);
        }
    }

    if (hdmi_reg_aon_intr_stat1_get()) {
        /* clr intr */
        hdmi_reg_aon_intr_stat1_set(HI_TRUE);
        /* event */
        event = ctrl_rsen_get();
        if (event != ctrl_info->rsen) {
            ctrl_info->event_callback(ctrl_info->event_data,
                (event == HI_TRUE) ? HDMI_EVENT_RSEN_CONNECT : HDMI_EVENT_RSEN_DISCONNECT);
            ctrl_info->rsen = event;
        } else {
            hdmi_warn("RSEN event not change: %u!\n", event);
        }
    }

#ifdef HDMI_HDR_SUPPORT
    ctrl_hpd_timer_invoke(ctrl_info);
#endif

    return;
}

static hi_s32 ctrl_null_packet_set(hi_bool enable)
{
    hdmi_reg_null_pkt_en_set(enable);
    return HI_SUCCESS;
}

static hi_void ctrl_video_dither_set(hdmi_video_path *videopath)
{
    hi_bool enable_gcp = HI_FALSE;
    hi_bool enable_dither = HI_FALSE;
    hdmi_video_dither dither_mode = HDMI_VIDEO_DITHER_DISABLE;

    if (videopath->out_deep_color != HDMI_DEEP_COLOR_30BIT && videopath->out_deep_color != HDMI_DEEP_COLOR_36BIT) {
        videopath->out_deep_color = HDMI_DEEP_COLOR_24BIT;
    }

    /* dither */
    switch (videopath->out_deep_color) {
        case HDMI_DEEP_COLOR_24BIT:
            switch (videopath->in_deep_color) {
                case HDMI_DEEP_COLOR_30BIT:
                    enable_dither = HI_TRUE;
                    dither_mode = HDMI_VIDEO_DITHER_10_8;
                    break;
                case HDMI_DEEP_COLOR_36BIT:
                    enable_dither = HI_TRUE;
                    dither_mode = HDMI_VIDEO_DITHER_12_8;
                    break;
                default:
                    break;
            }
            break;
        case HDMI_DEEP_COLOR_30BIT:
            enable_gcp = HI_TRUE;
            if (videopath->in_deep_color == HDMI_DEEP_COLOR_36BIT) {
                enable_dither = HI_TRUE;
                dither_mode = HDMI_VIDEO_DITHER_12_10;
            }
            break;
        case HDMI_DEEP_COLOR_36BIT:
            enable_gcp = HI_TRUE;
            break;
        default:
            break;
    }

    if (videopath->timing == HDMI_VIDEO_TIMING_1440X480I_60000 ||
        videopath->timing == HDMI_VIDEO_TIMING_1440X576I_50000 ||
        videopath->out_color_space == HDMI_COLORSPACE_YCBCR422) {
        enable_dither = HI_FALSE;
        dither_mode = HDMI_VIDEO_DITHER_DISABLE;
    }

    ctrl_video_path_dither_set(enable_dither, dither_mode);
    ctrl_video_path_deep_clr_set(enable_gcp, videopath->out_deep_color);

    return;
}

hi_s32 hal_hdmi_ctrl_init(hdmi_device_id hdmi, const hdmi_hal_init *hal_init)
{
    hi_s32 ret = HI_SUCCESS;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);
    hdmi_mach_ctrl mach_ctrl = {0};

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(hal_init, HI_FAILURE);

    if (!ctrl_info->init) {
        ret += hdmi_reg_crg_init();
        hdmi_if_null_return(hal_init->base_addr, HI_FAILURE);
        ret += hdmi_reg_aon_regs_init(hal_init->base_addr);
        ret += hdmi_reg_audio_path_regs_init(hal_init->base_addr);
        /* this functions will not be used at BVT */
        ret += hdmi_reg_tx_ctrl_regs_init(hal_init->base_addr);
        ret += hdmi_reg_tx_hdmi_regs_init(hal_init->base_addr);
        ret += hdmi_reg_video_path_regs_init(hal_init->base_addr);

        drv_hdmi_prod_crg_gate_set(HI_TRUE);
        ctrl_info->event_callback = hal_init->event_callback;
        ctrl_info->event_data = hal_init->event_data;
        ctrl_info->init       = HI_TRUE;
        mach_ctrl.data        = ctrl_info;
        mach_ctrl.callback    = ctrl_mach_ppd_callback;
        mach_ctrl.interval    = HDMI_MACH_DEFUALT_INTERVAL;
        mach_ctrl.name        = "HPD";
        ret += hal_hdmi_mach_register(&mach_ctrl, &ctrl_info->mach_id);
        ret += hal_hdmi_mach_start(ctrl_info->mach_id);

        ctrl_hpd_intr_enable(HI_TRUE);
    }

    return (ret != HI_SUCCESS) ? HI_FAILURE : HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_deinit(hdmi_device_id hdmi)
{
    hi_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    ctrl_hpd_intr_enable(HI_FALSE);
    ret = hal_hdmi_mach_stop(ctrl_info->mach_id);
    ret += hal_hdmi_mach_unregister(ctrl_info->mach_id);
    ret += hdmi_reg_audio_path_regs_deinit();
    ret += hdmi_reg_aon_regs_deinit();
    ret += hdmi_reg_tx_ctrl_regs_deinit();
    ret += hdmi_reg_tx_hdmi_regs_deinit();
    ret += hdmi_reg_video_path_regs_deinit();

    (hi_void)memset_s(ctrl_info, sizeof(hdmi_ctrl_info), 0, sizeof(hdmi_ctrl_info));
    ctrl_info->init = HI_FALSE;

    return (ret != HI_SUCCESS) ? HI_FAILURE : HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_avmute_set(hdmi_device_id hdmi, const hdmi_avmute_cfg *avmute_cfg)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(avmute_cfg, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    ctrl_avmute_set(avmute_cfg);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_avmute_get(hdmi_device_id hdmi, hi_bool *enable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);
    hdmi_if_null_return(enable, HI_FAILURE);

    return ctrl_avmute_get(enable);
}

hi_s32 hal_hdmi_ctrl_csc_set(hdmi_device_id hdmi, const hdmi_video_config *video_cfg)
{
    hi_unused(hdmi);
    hdmi_if_null_return(video_cfg, HI_FAILURE);

    ctrl_video_path_colorimetry_set(video_cfg->conv_std, video_cfg->conv_std);
    ctrl_video_path_quantization_set(video_cfg->out_csc_quantization, video_cfg->out_csc_quantization);
    ctrl_video_path_color_space_set(video_cfg->in_color_space, video_cfg->out_color_space);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_video_path_set(hdmi_device_id hdmi, hdmi_video_path *video_path)
{
    errno_t errnumber;
    hi_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(video_path, HI_FAILURE);
    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    errnumber = memcpy_s(&ctrl_info->video_path, sizeof(ctrl_info->video_path), video_path, sizeof(hdmi_video_path));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);

    /* dither */
    ctrl_video_dither_set(video_path);
    ret = ctrl_video_path_polarity_set(&video_path->out_hv_sync_pol);
    ctrl_video_path_colorimetry_set(video_path->in_colormetry, video_path->out_colormetry);
    ctrl_video_path_quantization_set(video_path->in_quantization, video_path->out_quantization);
    ctrl_video_path_color_space_set(video_path->in_color_space, video_path->out_color_space);

    return ret;
}

hi_s32 hal_hdmi_ctrl_tmds_mode_set(hdmi_device_id hdmi, hdmi_tmds_mode tmds_mode)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    return ctrl_tmds_mode_set(tmds_mode);
}

hi_s32 hal_hdmi_ctrl_infoframe_en_set(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id, hi_bool enable)
{
    hi_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_NULL:
            ret = ctrl_null_packet_set(enable);
            break;
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ret = ctrl_vendor_infoframe_en_set(enable);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ret = ctrl_avi_infoframe_en_set(enable);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            ret = ctrl_audio_infoframe_en_set(enable);
            break;
        case HDMI_INFOFRAME_TYPE_GBD:
            ret = ctrl_gbd_infoframe_en_set(enable);
            break;
        case HDMI_INFOFRAME_TYPE_DRM:
            ret = ctrl_drm_infoframe_en_set(enable);
            break;
        default:
            hdmi_warn("un-support infoframe type:%u!\n", infoframe_id);
            ret = HI_FAILURE;
            break;
    }

    return ret;
}

hi_s32 hal_hdmi_ctrl_infoframe_data_set(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id,
    const hi_u8 *if_data, hi_u32 data_len)
{
    hi_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(if_data, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    if (data_len < HDMI_INFOFRAME_DATA_SIZE) {
        hdmi_warn("infoframe data size err, is %u!\n", data_len);
    }

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ret = ctrl_vendor_infoframe_data_set(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ret = ctrl_avi_infoframe_data_set(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            ret = ctrl_audio_infoframe_data_set(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_DRM:
            ret = ctrl_drm_infoframe_data_set(if_data);
            break;
        default:
            hdmi_warn("un-support infoframe type:%u!\n", infoframe_id);
            ret = HI_FAILURE;
            break;
    }

    return ret;
}

hi_s32 hal_hdmi_ctrl_data_reset(hdmi_device_id hdmi, hi_bool debug_mode, hi_u32 delay_ms)
{
    static hi_u32 delay;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    hdmi_reg_tx_acr_srst_req_set(HI_TRUE);
    hdmi_reg_tx_afifo_srst_req_set(HI_TRUE);
    hdmi_reg_tx_aud_srst_req_set(HI_TRUE);
    hdmi_reg_tx_hdmi_srst_req_set(HI_TRUE);
    osal_udelay(CTRL_REAET_WAIT_TIME);
    hdmi_reg_tx_acr_srst_req_set(HI_FALSE);
    hdmi_reg_tx_afifo_srst_req_set(HI_FALSE);
    hdmi_reg_tx_aud_srst_req_set(HI_FALSE);
    /* TOP rst: will also pack_fifo_ctrl reg_fifo_manu_rst */
    hdmi_reg_tx_hdmi_srst_req_set(HI_FALSE);

    if (debug_mode) {
        delay = delay_ms;
    }

    if (delay) {
        osal_msleep(delay);
        hdmi_info("data_rst %u ms\n", delay);
    }

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_video_mute_set(hdmi_device_id hdmi, hi_bool enable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    ctrl_video_mute_set(enable);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_video_mute_get(hdmi_device_id hdmi, hi_bool *enable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(enable, HI_FAILURE);
    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    return ctrl_video_mute_get(enable);
}

hi_s32 hal_hdmi_ctrl_audio_mute_set(hdmi_device_id hdmi, hi_bool enable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    ctrl_audio_mute_set(enable);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_audio_path_enable_set(hdmi_device_id hdmi, hi_bool enable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    ctrl_audio_path_enable_set(enable);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_audio_path_set(hdmi_device_id hdmi, const hdmi_audio_path *audio_path)
{
    errno_t ret;
    hi_u32          cts_value, n_value, org_reg_rate_cfg;
    hi_bool         muti_layout  = HI_FALSE;
    hi_bool         spdif_enable = HI_FALSE;
    hi_bool         hbra_enable  = HI_FALSE;
    ctrl_i2s_status i2s_status   = {0};
    ctrl_i2s        i2s_ctrl     = {0};
    ctrl_fifo       fifo_ctrl    = {0};
    hdmi_ctrl_info *ctrl_info    = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(audio_path, HI_FAILURE);
    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    ret = memcpy_s(&ctrl_info->audio_path, sizeof(ctrl_info->audio_path), audio_path, sizeof(hdmi_audio_path));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    ctrl_audio_path_enable_set(HI_FALSE);
    ctrl_audio_reset();

    org_reg_rate_cfg = audio_path->sample_rate;
    /* I2S layout */
    if (audio_path->channel_num == HDMI_AUDIO_FORMAT_8CH) {
        muti_layout = HI_TRUE;
    } else if (audio_path->channel_num < HDMI_AUDIO_FORMAT_8CH) {
        muti_layout = HI_FALSE;
    } else {
        hdmi_warn("error audio channel number:%u\n", audio_path->channel_num);
    }
    ctrl_audio_path_layout_set(muti_layout);

    /* i2s ctrl */
    i2s_ctrl.in_bit_length = ctrl_audio_bit_reg_get(audio_path->sample_bit);
    ctrl_audio_i2s_ctrl_set(&i2s_ctrl);

    /* i2s status */
    i2s_status.clk_accuracy     = HDMI_AUDIO_CLK_ACCURACY_LEVEL2;
    i2s_status.samp_rate        = ctrl_i2s_rate_reg_get(org_reg_rate_cfg);
    i2s_status.org_rate         = ctrl_i2s_rate_org_reg_get(audio_path->sample_rate);
    i2s_status.out_bit_length   = ctrl_audio_bit_reg_get(audio_path->sample_bit);
    ctrl_audio_i2s_status_set(&i2s_status);

    fifo_ctrl.hbra_mask = HDMI_AUDIO_HBRA_MASK_NONE;
    ctrl_audio_fifo_ctrl_set(&fifo_ctrl);

    /* CTS & N value */
    ctrl_audio_path_cts_value_set(HI_FALSE);
    n_value = hal_hdmi_n_value_get(audio_path->sample_rate, audio_path->pixel_clk);
    ctrl_audio_path_n_value_set(n_value);

    cts_value = hal_hdmi_cts_value_get(audio_path->sample_rate, audio_path->pixel_clk);
    hdmi_info("\n fs=%u,bit=%u,intf=%u,tmds=%u\n"
              "reference N=%u,real N=%u,reference cts=%u,real_cts=%u\n",
              audio_path->sample_rate, audio_path->sample_bit, audio_path->sound_intf, audio_path->pixel_clk,
              n_value, ctrl_audio_path_n_value_get(), cts_value, ctrl_audio_path_cts_value_get());

    /* enable */
    ctrl_audio_i2s_enable_set(HI_TRUE);
    ctrl_audio_spdif_enable_set(spdif_enable);
    ctrl_audio_hbra_enable_set(hbra_enable); /* when spdif, it must false */

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ctrl_audio_path_get(hdmi_device_id hdmi, ctrl_audio_status *audio_stat)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);
    hdmi_audio_path *audio_path = HI_NULL;

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);
    hdmi_if_null_return(audio_stat, HI_FAILURE);

    audio_path = &ctrl_info->audio_path;
    hdmi_if_null_return(audio_path, HI_FAILURE);

    if (ctrl_audio_spdif_enable_get()) {
        audio_stat->sound_intf = HDMI_AUDIO_INTF_SPDIF;
    } else if (ctrl_audio_hbra_enable_get()) {
        audio_stat->sound_intf = HDMI_AUDIO_INTF_HBRA;
    } else if (ctrl_audio_i2s_enable_get()) {
        audio_stat->sound_intf = HDMI_AUDIO_INTF_I2S;
    } else {
        audio_stat->sound_intf = HDMI_AUDIO_INTF_BUTT;
    }

    audio_stat->enable_audio = ctrl_audio_path_enable_get();
    audio_stat->audio_mute   = ctrl_audio_mute_get();
    audio_stat->sample_bit   = ctrl_audio_bit_value_get(HI_FALSE);
    audio_stat->sample_rate  = ctrl_i2s_rate_value_get();
    audio_stat->channel_num  = ctrl_audio_path_layout_get() ? HDMI_AUDIO_FORMAT_8CH : HDMI_AUDIO_FORMAT_2CH;
    audio_stat->ref_cts      = hal_hdmi_cts_value_get(audio_path->sample_rate, audio_path->pixel_clk);
    audio_stat->reg_cts      = ctrl_audio_path_cts_value_get();
    audio_stat->ref_n        = hal_hdmi_n_value_get(audio_path->sample_rate, audio_path->pixel_clk);
    audio_stat->reg_n        = ctrl_audio_path_n_value_get();

    return HI_SUCCESS;
}

hi_void hal_hdmi_ctrl_video_path_get(hdmi_device_id hdmi, ctrl_video_status *video_stat)
{
    hi_s32          ret;
    hi_bool         enable_gcp    = HI_FALSE;
    hi_bool         enable_dither = HI_FALSE;
    hdmi_ctrl_info *ctrl_info     = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return_void(video_stat);
    hdmi_if_null_return_void(ctrl_info);
    hdmi_if_false_return_void(ctrl_info->init);

    ret = ctrl_timming_decect_get(&video_stat->timing);
    ret += ctrl_video_path_polarity_get(&video_stat->out_hv_sync_pol);
    ctrl_video_path_color_space_get(&video_stat->in_color_space, &video_stat->out_color_space);
    ret += ctrl_video_path_dither_get(&enable_dither, &video_stat->dither_mode);
    ret += ctrl_video_path_colormetry_get(&video_stat->in_colormetry, &video_stat->out_colormetry);
    ret += ctrl_video_path_quantization_get(&video_stat->in_quantization, &video_stat->out_quantization);
    ret += ctrl_video_path_deep_clr_get(&enable_gcp, &video_stat->out_deep_color);
    video_stat->csc_enable = ctrl_video_color_csc_get();

    if (!enable_gcp) {
        video_stat->out_deep_color = HDMI_DEEP_COLOR_OFF;
    }

    switch (video_stat->dither_mode) {
        case HDMI_VIDEO_DITHER_12_10:
        case HDMI_VIDEO_DITHER_12_8:
            video_stat->in_deep_color = HDMI_DEEP_COLOR_36BIT;
            break;
        case HDMI_VIDEO_DITHER_10_8:
            video_stat->in_deep_color = HDMI_DEEP_COLOR_30BIT;
            break;
        default:
            video_stat->in_deep_color = video_stat->out_deep_color;
            break;
    }

    video_stat->y422_enable = ctrl_video_color_dwsm_hori_get();
    video_stat->y420_enable = ctrl_video_color_ycbcr420_get();

    return;
}

hi_s32 hal_hdmi_ctrl_tmds_mode_get(hdmi_device_id hdmi, hdmi_tmds_mode *tmds_mode)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(tmds_mode, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    return ctrl_tmds_mode_get(tmds_mode);
}

hi_s32 hal_hdmi_ctrl_infoframe_en_get(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id, hi_bool *enable)
{
    hi_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(enable, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ret = ctrl_vendor_infoframe_en_get(enable);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ret = ctrl_avi_infoframe_en_get(enable);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            ret = ctrl_audio_infoframe_en_get(enable);
            break;
        case HDMI_INFOFRAME_TYPE_GBD:
            ret = ctrl_gbd_infoframe_en_get(enable);
            break;
        case HDMI_INFOFRAME_TYPE_DRM:
            ret = ctrl_drm_infoframe_en_get(enable);
            break;
        default:
            hdmi_warn("un-support infoframe type:%u!\n", infoframe_id);
            ret = HI_FAILURE;
            break;
    }

    return ret;
}

hi_s32 hal_hdmi_ctrl_infoframe_data_get(hdmi_device_id hdmi, hdmi_infoframe_id infoframe_id, hi_u8 *if_data)
{
    hi_s32 ret;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(if_data, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ret = ctrl_vendor_infoframe_data_get(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ret = ctrl_avi_infoframe_data_get(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            ret = ctrl_audio_infoframe_data_get(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_GBD:
            ret = ctrl_gbd_infoframe_data_get(if_data);
            break;
        case HDMI_INFOFRAME_TYPE_DRM:
            ret = ctrl_drm_infoframe_data_get(if_data);
            break;
        default:
            hdmi_warn("un-support infoframe type:%u!\n", infoframe_id);
            ret = HI_FAILURE;
            break;
    }

    return ret;
}

#ifdef HDMI_HDR_SUPPORT
hi_s32 hal_hdmi_ctrl_hdr_timer_set(hdmi_device_id hdmi, const hdmi_timer_config *hdr_timer_cfg)
{
    hi_u32 i;
    ctrl_hdr_timer *hdr_timer = HI_NULL;
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(hdr_timer_cfg, HI_FAILURE);
    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    if (hdr_timer_cfg->timer_type >= HDMI_TIMER_TYPE_BUTT) {
        hdmi_warn("invalid timer type=%u,fail\n", hdr_timer_cfg->timer_type);
        return HI_FAILURE;
    }

    for (i = 0; i < HDMI_TIMER_TYPE_BUTT; i++) {
        hdr_timer = &ctrl_info->hdr_timer[i];
        if ((hdr_timer_cfg->timer_start) && (!hdr_timer->enable)) {
            hdr_timer->enable      = HI_TRUE;
            hdr_timer->timer_type  = hdr_timer_cfg->timer_type;
            hdr_timer->start_time  = hal_hdmi_mach_ms_get();
            hdr_timer->time_length = hdr_timer_cfg->time;
            break;
        } else if (hdr_timer_cfg->timer_type == hdr_timer->timer_type) {
            (hi_void)memset_s(hdr_timer, sizeof(ctrl_hdr_timer), 0, sizeof(ctrl_hdr_timer));
            break;
        }
    }

    if (i >= HDMI_TIMER_TYPE_BUTT) {
        hdmi_warn("can't find valid timer type=%u, fail\n", hdr_timer_cfg->timer_type);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
#endif

hi_s32 hal_hdmi_ctrl_hpd_rsen_get(hdmi_device_id hdmi, hdmi_hpd_rsen *hpd_rsen)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_null_return(hpd_rsen, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    hpd_rsen->rsen_on = ctrl_rsen_get();
    hpd_rsen->hpd_on = ctrl_hpd_get();

    return HI_SUCCESS;
}

hi_void hal_hdmi_ctrl_dither_dbg_set(hdmi_device_id hdmi, hdmi_video_dither dither_mode)
{
    hi_bool enable_dither = HI_FALSE;

    hi_unused(hdmi);
    if (dither_mode >= HDMI_VIDEO_DITHER_DISABLE) {
        enable_dither = HI_FALSE;
        dither_mode = HDMI_VIDEO_DITHER_DISABLE;
    } else {
        enable_dither = HI_TRUE;
    }
    ctrl_video_path_dither_set(enable_dither, dither_mode);

    return;
}

hi_s32 hal_hdmi_ctrl_tmds_stable_get(hdmi_device_id hdmi, hi_bool *stable)
{
    hdmi_ctrl_info *ctrl_info = ctrl_info_ptr_get(hdmi);

    hdmi_if_null_return(stable, HI_FAILURE);
    hdmi_if_null_return(ctrl_info, HI_FAILURE);
    hdmi_if_false_return(ctrl_info->init, HI_FAILURE);

    *stable = hdmi_reg_pclk2tclk_stable_get() ? HI_TRUE : HI_FALSE;
    return HI_SUCCESS;
}


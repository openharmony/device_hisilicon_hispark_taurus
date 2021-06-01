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
#include "hdmi_hal_intf.h"
#include "hdmi_hal_phy.h"
#include "hdmi_reg_crg.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_intf.h"

#define TX_MAX_TMDS_CLK 600
#define CTRL_RESET_WAIT 20

typedef hi_void *hdmi_handle;

static hdmi_hal_cfg g_hal_cfg[HDMI_DEVICE_ID_BUTT];

static hdmi_deep_color depth_convert_to_deep_color(hdmi_video_bit_depth video_bit_depth)
{
    hdmi_deep_color ret;

    switch (video_bit_depth) {
        case HDMI_VIDEO_BITDEPTH_8:
            ret = HDMI_DEEP_COLOR_24BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_10:
            ret = HDMI_DEEP_COLOR_30BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_12:
            ret = HDMI_DEEP_COLOR_36BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_16:
            ret = HDMI_DEEP_COLOR_24BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_OFF:
            ret = HDMI_DEEP_COLOR_OFF;
            break;
        default:
            ret = HDMI_DEEP_COLOR_OFF;
            break;
    }

    return ret;
}

static hdmi_hal_cfg *intf_hal_info_ptr_get(hdmi_device_id hdmi)
{
    if (hdmi < HDMI_DEVICE_ID_BUTT) {
        return &g_hal_cfg[hdmi];
    }
    return HI_NULL;
}

#ifdef HDMI_DEBUG_SUPPORT
static hi_void hal_hdmi_cbar_enable(hi_bool enable)
{
    hi_u32 *reg_addr = HI_NULL;
    hi_u32  reg_value;

    reg_addr = (hi_u32 *)osal_ioremap_nocache(HDMI_COLOR_BAR_BASE, 4); /* 4: register size */
    hdmi_if_null_return_void(reg_addr);
    reg_value = hdmi_reg_read(reg_addr);

    if (enable) {
        reg_value |= HDMI_COLOR_BAR_MASK;
        reg_value |= HDMI_COLOR_BAR_UPDATE_MASK;
    } else {
        reg_value &= ~HDMI_COLOR_BAR_MASK;
        reg_value |= HDMI_COLOR_BAR_UPDATE_MASK;
    }
    hdmi_reg_write(reg_addr, reg_value);
    osal_iounmap((void *)reg_addr, 0x4);
}
#endif

static hi_void intf_tx_capability_init(hdmi_device_id hdmi_id)
{
    hdmi_hal_cfg *hal_cfg = intf_hal_info_ptr_get(hdmi_id);

    hdmi_if_null_return_void(hal_cfg);

    /* tx capability */
    hal_cfg->tx_capability.tx_hdmi_14        = HI_TRUE;
    hal_cfg->tx_capability.tx_hdmi_20        = HI_TRUE;
    hal_cfg->tx_capability.tx_hdcp_14        = HI_TRUE;
    hal_cfg->tx_capability.tx_hdcp_22        = HI_TRUE;
    hal_cfg->tx_capability.tx_rgb444         = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr444       = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr422       = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr420       = HI_TRUE;
    hal_cfg->tx_capability.tx_deep_clr10_bit = HI_TRUE;
    hal_cfg->tx_capability.tx_deep_clr12_bit = HI_TRUE;
    hal_cfg->tx_capability.tx_deep_clr16_bit = HI_FALSE;
    hal_cfg->tx_capability.tx_rgb_ycbcr444   = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr444_422   = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr422_420   = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr420_422   = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr422_444   = HI_TRUE;
    hal_cfg->tx_capability.tx_ycbcr444_rgb   = HI_TRUE;
    hal_cfg->tx_capability.tx_scdc           = HI_TRUE;
    hal_cfg->tx_capability.tx_max_tmds_clk   = TX_MAX_TMDS_CLK;
    return;
}

static hi_void hal_hdmi_hardware_init(const struct hdmi_hal_ *hal)
{
    hi_unused(hal);
    drv_hdmi_prod_crg_init();
    return;
}

static hi_void hal_hdmi_tmds_mode_set(const struct hdmi_hal_ *hal, hdmi_tmds_mode tmds_mode)
{
    hdmi_if_null_return_void(hal);
    hal_hdmi_ctrl_tmds_mode_set(hal->hal_ctx.hdmi_id, tmds_mode);
    return;
}

static hi_void hal_hdmi_avmute_set(const struct hdmi_hal_ *hal, hi_bool avmute)
{
    hdmi_avmute_cfg avmute_cfg = {0};

    hdmi_if_null_return_void(hal);

    drv_hdmi_compat_avmute_get(hal->hal_ctx.hdmi_id, avmute, &avmute_cfg);
    hal_hdmi_ctrl_avmute_set(hal->hal_ctx.hdmi_id, &avmute_cfg);

    return;
}

static hi_void hal_hdmi_emi_status_get(const struct hdmi_hal_ *hal, hdmi_emi_status *emi_status)
{
#ifndef HDMI_FPGA_SUPPORT
    hdmi_phy_info phy_info = {0};

    hdmi_if_null_return_void(emi_status);

    hal_hdmi_phy_info_get(&phy_info);
    emi_status->sw_emi_enable = phy_info.ssc_cfg.ssc_enable;
    emi_status->debug_enable  = phy_info.ssc_cfg.ssc_debug_en;
    hal_hdmi_phy_ssc_get(&emi_status->hw_emi_enable);
#endif
    hi_unused(hal);
    return;
}

static hi_void hal_hdmi_csc_param_set(struct hdmi_hal_ *hal, hdmi_video_config *video_cfg)
{
    /* colorspace in & out, quantization, CONVERSION_STD */
    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(video_cfg);

    hal_hdmi_ctrl_csc_set(hal->hal_ctx.hdmi_id, video_cfg);
    hal->hal_ctx.video_cfg.in_color_space       = video_cfg->in_color_space;
    hal->hal_ctx.video_cfg.out_color_space      = video_cfg->out_color_space;
    hal->hal_ctx.video_cfg.out_csc_quantization = video_cfg->out_color_space;
    hal->hal_ctx.video_cfg.conv_std             = video_cfg->conv_std;

    return;
}

static hi_void hal_hdmi_infoframe_set(const struct hdmi_hal_ *hal,
    hdmi_infoframe_id infoframe_id, hi_u8 *in_buffer, hi_u32 buf_len)
{
    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(in_buffer);
    hal_hdmi_ctrl_infoframe_data_set(hal->hal_ctx.hdmi_id, infoframe_id, in_buffer, buf_len);
    return;
}

static hi_void hal_hdmi_infoframe_enable_set(const struct hdmi_hal_ *hal,
    hdmi_infoframe_id infoframe_id, hi_bool enable)
{
    hdmi_if_null_return_void(hal);
    hal_hdmi_ctrl_infoframe_en_set(hal->hal_ctx.hdmi_id, infoframe_id, enable);
    return;
}

static hi_s32 hal_hdmi_video_path_set(const struct hdmi_hal_ *hal, hdmi_video_config *video_cfg)
{
    hi_s32 ret;
    hdmi_video_path video_path = {0};
    hdmi_hal_cfg *hal_cfg = HI_NULL;

    hdmi_if_null_return(hal, HI_FAILURE);
    hdmi_if_null_return(video_cfg, HI_FAILURE);

    hal_cfg = intf_hal_info_ptr_get(hal->hal_ctx.hdmi_id);
    hdmi_if_null_return(hal_cfg, HI_FAILURE);
    hal_cfg->tmds_clk          = video_cfg->tmds_clk;
    video_path.in_color_space  = video_cfg->in_color_space;
    video_path.out_color_space = video_cfg->out_color_space;
    video_path.in_deep_color   = depth_convert_to_deep_color(video_cfg->in_bit_depth);
    video_path.out_deep_color  = video_cfg->deep_color;
    video_path.out_hv_sync_pol.de_inver    = video_cfg->de_pol;
    video_path.out_hv_sync_pol.h_pol_inver = video_cfg->h_sync_pol;
    video_path.out_hv_sync_pol.v_pol_inver = video_cfg->v_sync_pol;

    switch (video_cfg->conv_std) {
        case HDMI_CONV_STD_BT_601:
            video_path.out_colormetry = HDMI_COLORMETRY_BT601;
            break;
        case HDMI_CONV_STD_BT_709:
            video_path.out_colormetry = HDMI_COLORMETRY_BT709;
            break;
        case HDMI_CONV_STD_BT_2020_NON_CONST_LUMINOUS:
            video_path.out_colormetry = HDMI_COLORMETRY_BT2020;
            break;
        case HDMI_CONV_STD_BT_2020_CONST_LUMINOUS:
            video_path.out_colormetry = HDMI_COLORMETRY_BT2020_CONST;
            break;
        default:
            video_path.out_colormetry = HDMI_COLORMETRY_BT709;
            break;
    }
    video_path.in_colormetry    = video_path.out_colormetry;
    video_path.in_quantization  = video_cfg->out_csc_quantization;
    video_path.out_quantization = video_cfg->out_csc_quantization;
    video_path.in_pixel_clk     = video_cfg->pixel_clk;
    video_path.out_tmds_clk     = video_cfg->tmds_clk;
    video_path.timing           = video_cfg->timing;
    ret = hal_hdmi_ctrl_video_path_set(hal->hal_ctx.hdmi_id, &video_path);

    return ret;
}

static hi_void hal_hdmi_ctrl_reset(const struct hdmi_hal_ *hal)
{
    hi_u32 i;
    hi_bool tmds_stable = HI_FALSE;
    hi_bool output = HI_FALSE;

    hdmi_if_null_return_void(hal);

    hal_hdmi_phy_oe_get(&output);
    if (output == HI_TRUE) {
        hdmi_info("oe enable, do not reset!\n");
        return;
    }

    hal_hdmi_ctrl_data_reset(hal->hal_ctx.hdmi_id, HI_FALSE, 0);

    for (i = 0; (!tmds_stable) && (i < CTRL_RESET_WAIT); i++) {
        osal_msleep(1);
        hal_hdmi_ctrl_tmds_stable_get(hal->hal_ctx.hdmi_id, &tmds_stable);
    }

    hdmi_info("wait %ums, tmds_stable=%u\n", i, tmds_stable);

    return;
}

static hi_s32 hal_hdmi_phy_hw_spec_set(const struct hdmi_hal_ *hal, hi_u32 tmds_clk, const hdmi_hw_spec *hw_spec)
{
    hi_s32 ret = HI_SUCCESS;

#ifndef HDMI_FPGA_SUPPORT
    hdmi_if_null_return(hw_spec, HI_ERR_NULL_PTR);
    ret = hal_hdmi_phy_params_set(tmds_clk, hw_spec);
#else
    hi_unused(tmds_clk);
    hi_unused(hw_spec);
#endif
    hi_unused(hal);
    return ret;
}

static hi_s32 hal_hdmi_phy_hw_spec_get(const struct hdmi_hal_ *hal, hdmi_hw_spec *hw_spec)
{
    hi_s32 ret = HI_SUCCESS;

#ifndef HDMI_FPGA_SUPPORT
    errno_t errnumber;
    hdmi_phy_hw_param phy_hw_param = {0};

    hdmi_if_null_return(hw_spec, HI_ERR_NULL_PTR);
    ret = hal_hdmi_phy_params_get(&phy_hw_param);

    errnumber = memcpy_s(hw_spec, sizeof(*hw_spec), &phy_hw_param.hw_spec_cfg, sizeof(hdmi_hw_spec));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
#endif
    hi_unused(hal);
    return ret;
}

static hi_void hal_hdmi_phy_output_enable_set(const struct hdmi_hal_ *hal, hi_bool enable)
{
    hdmi_if_null_return_void(hal);

#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_oe_set(enable);
#else
    hi_unused(enable);
#endif
    return;
}

static hi_void hal_hdmi_phy_power_enable_set(const struct hdmi_hal_ *hal, hi_bool enable)
{
#ifndef HDMI_FPGA_SUPPORT
    hi_bool old_eable = HI_FALSE;

    hal_hdmi_phy_power_get(&old_eable);
    if (enable != old_eable) {
        hal_hdmi_phy_power_set(enable);
    }
#else
    hi_unused(enable);
#endif
    hi_unused(hal);
    return;
}

static hi_void hal_hdmi_phy_set(const struct hdmi_hal_ *hal, hdmi_phy_cfg *phy_cfg)
{
#ifndef HDMI_FPGA_SUPPORT
    hdmi_phy_tmds_cfg cfg = {0};

    hdmi_if_null_return_void(phy_cfg);

    cfg.deep_color = phy_cfg->deep_color;
    cfg.emi_enable = phy_cfg->emi_enable;
    cfg.mode_cfg   = phy_cfg->mode_cfg;
    cfg.pixel_clk  = phy_cfg->pixel_clk;
    cfg.tmds_clk   = phy_cfg->tmds_clk;
    cfg.trace_len  = phy_cfg->trace_len;

    hal_hdmi_phy_tmds_set(&cfg);
#else
    hi_unused(phy_cfg);
#endif
    hi_unused(hal);
    return;
}

static hi_void hal_hdmi_tx_capability_get(const struct hdmi_hal_ *hal, hdmi_tx_capability_data *tx_capability)
{
    errno_t ret;
    hdmi_hal_cfg *hal_cfg = HI_NULL;

    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(tx_capability);

    hal_cfg = intf_hal_info_ptr_get(hal->hal_ctx.hdmi_id);
    hdmi_if_null_return_void(hal_cfg);
    hdmi_if_false_return_void(hal_cfg->init);
    ret = memcpy_s(tx_capability, sizeof(hdmi_tx_capability_data),
        &hal_cfg->tx_capability, sizeof(hdmi_tx_capability_data));
    hdmi_unequal_eok_return_void(ret);

    return;
}

#ifdef HDMI_HDR_SUPPORT
static hi_void hal_hdmi_hdr_timer_set(const struct hdmi_hal_ *hal, hdmi_timer_config *timer_cfg)
{
    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(timer_cfg);
    hal_hdmi_ctrl_hdr_timer_set(hal->hal_ctx.hdmi_id, timer_cfg);
    return;
}
#endif

static hi_void hal_hdmi_sequencer_handler_process(const struct hdmi_hal_ *hal)
{
    hi_unused(hal);
    hal_hdmi_mach_invoke();
    return;
}

static hi_void hal_hdmi_hw_audio_status_get(hi_u32 hdmi_id, hdmi_hardware_status *hw_status)
{
    ctrl_audio_status audio_stat = {0};

    hal_hdmi_ctrl_audio_path_get(hdmi_id, &audio_stat);
    hw_status->audio_status.audio_mute   = audio_stat.audio_mute;
    hw_status->audio_status.audio_enable = audio_stat.enable_audio;
    hw_status->audio_status.sample_fs    = audio_stat.sample_rate;
    hw_status->audio_status.layout       = audio_stat.channel_num;
    hw_status->audio_status.sound_intf   = audio_stat.sound_intf;
    hw_status->audio_status.sample_depth = audio_stat.sample_bit;
    hw_status->audio_status.down_sample  = HI_FALSE;
    hw_status->audio_status.ref_cts      = audio_stat.ref_cts;
    hw_status->audio_status.reg_cts      = audio_stat.reg_cts;
    hw_status->audio_status.ref_n        = audio_stat.ref_n;
    hw_status->audio_status.reg_n        = audio_stat.reg_n;

    return;
}

static hi_void hal_hdmi_hw_video_status_get(hi_u32 hdmi_id, hdmi_hardware_status *hw_status)
{
    ctrl_video_status video_stat = {0};

    hal_hdmi_ctrl_video_path_get(hdmi_id, &video_stat);
    hal_hdmi_ctrl_video_mute_get(hdmi_id, &hw_status->video_status.video_mute);
    hw_status->video_status.ycbcr2rgb       =
        video_stat.csc_enable && (video_stat.out_color_space == HDMI_COLORSPACE_RGB);
    hw_status->video_status.rgb2ycbcr       =
        video_stat.csc_enable && (video_stat.in_color_space == HDMI_COLORSPACE_RGB);
    hw_status->video_status.ycbcr444_422    = video_stat.y422_enable;
    hw_status->video_status.ycbcr422_420    = video_stat.y420_enable;
    hw_status->video_status.ycbcr420_422    = HI_FALSE; // not support
    hw_status->video_status.ycbcr422_444    = HI_FALSE; // not support
    hw_status->video_status.in420_ydemux    = video_stat.dwsm_vert_enable;
    hw_status->video_status.out420_ydemux   = HI_FALSE; // not support
    hw_status->video_status.dither          = video_stat.dither_mode;
    hw_status->video_status.v_sync_pol      = video_stat.out_hv_sync_pol.h_pol_inver;
    hw_status->video_status.h_sync_pol      = video_stat.out_hv_sync_pol.v_pol_inver;
    hw_status->video_status.sync_pol        = HI_FALSE; // not support
    hw_status->video_status.de_pol          = video_stat.out_hv_sync_pol.de_inver;
    hw_status->video_status.swap_hs_cs      = HI_FALSE; // not support
    hw_status->video_status.in_color_space  = video_stat.in_color_space;
    hw_status->video_status.out_color_space = video_stat.out_color_space;
    hw_status->video_status.out_bit_depth   =
        (video_stat.out_deep_color == HDMI_DEEP_COLOR_OFF) ? HDMI_VIDEO_BITDEPTH_OFF : video_stat.out_deep_color;
    hw_status->video_status.sync_sw_enable  = video_stat.timing.sync_sw_enable;
    hw_status->video_status.vsync_polarity  = video_stat.timing.vsync_polarity;
    hw_status->video_status.hsync_polarity  = video_stat.timing.hsync_polarity;
    hw_status->video_status.progressive     = video_stat.timing.progressive;
    hw_status->video_status.hsync_total     = video_stat.timing.hsync_total;
    hw_status->video_status.hactive_cnt     = video_stat.timing.hactive_cnt;
    hw_status->video_status.vsync_total     = video_stat.timing.vsync_total;
    hw_status->video_status.vactive_cnt     = video_stat.timing.vactive_cnt;
    hw_status->video_status.out_csc_quantization = video_stat.out_quantization;

    return;
}

static hi_void hal_hdmi_phy_hwparam_get(hi_u32 hdmi_id, hdmi_hwspec *hwspec)
{
    errno_t ret;
    hdmi_phy_hw_param hw_param = {0};

    hi_unused(hdmi_id);
    hal_hdmi_phy_params_get(&hw_param);
    ret = memcpy_s(&hwspec->hwspec_user, sizeof(hwspec->hwspec_user),
        &hw_param.hw_spec_cfg, sizeof(hdmi_hw_spec));
    hdmi_unequal_eok_return_void(ret);
    ret = memcpy_s(&hwspec->hwspec_def, sizeof(hwspec->hwspec_def),
        &hw_param.hw_spec_def, sizeof(hdmi_hw_spec));
    hdmi_unequal_eok_return_void(ret);
    ret = memcpy_s(&hwspec->hwparam_cur, sizeof(hwspec->hwparam_cur),
        &hw_param.hw_param_cur, sizeof(hdmi_hw_param));
    hdmi_unequal_eok_return_void(ret);

    return;
}

static hi_void hal_hdmi_hardware_status_get(const struct hdmi_hal_ *hal, hdmi_hardware_status *hw_status)
{
    hdmi_hpd_rsen hpd_rsen = {0};

    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(hw_status);

#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_oe_get(&hw_status->phy_status.phy_oe);
    hal_hdmi_phy_power_get(&hw_status->phy_status.phy_power_on);
#endif
    hal_hdmi_ctrl_infoframe_en_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_AVI, &hw_status->info_frame_status.avi_enable);
    hal_hdmi_ctrl_infoframe_en_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_AUDIO, &hw_status->info_frame_status.audio_enable);
    hal_hdmi_ctrl_infoframe_en_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_VENDOR, &hw_status->info_frame_status.vsif_enable);
    hal_hdmi_ctrl_infoframe_en_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_GBD, &hw_status->info_frame_status.gbd_enable);
    hal_hdmi_ctrl_infoframe_en_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_DRM, &hw_status->info_frame_status.drm_enable);
    hal_hdmi_ctrl_infoframe_data_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_AVI, hw_status->info_frame_status.avi);
    hal_hdmi_ctrl_infoframe_data_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_AUDIO, hw_status->info_frame_status.audio);
    hal_hdmi_ctrl_infoframe_data_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_VENDOR, hw_status->info_frame_status.vsif);
    hal_hdmi_ctrl_infoframe_data_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_GBD, hw_status->info_frame_status.gdb);
    hal_hdmi_ctrl_infoframe_data_get(hal->hal_ctx.hdmi_id,
        HDMI_INFOFRAME_TYPE_DRM, hw_status->info_frame_status.drm);

    /* HPD, avmute, tmds_mode */
    hal_hdmi_ctrl_hpd_rsen_get(hal->hal_ctx.hdmi_id, &hpd_rsen);
    hw_status->common_status.hotplug = hpd_rsen.hpd_on;
    hw_status->common_status.rsen    = hpd_rsen.rsen_on;
    hal_hdmi_ctrl_avmute_get(hal->hal_ctx.hdmi_id, &hw_status->common_status.avmute);
    hal_hdmi_ctrl_tmds_mode_get(hal->hal_ctx.hdmi_id, &hw_status->common_status.tmds_mode);

    hal_hdmi_hw_audio_status_get(hal->hal_ctx.hdmi_id, hw_status);
    hal_hdmi_hw_video_status_get(hal->hal_ctx.hdmi_id, hw_status);
    hal_hdmi_phy_hwparam_get(hal->hal_ctx.hdmi_id, &hw_status->phy_hwspec);

    return;
}

static hi_void hal_hdmi_hdp_intr_status_get(const struct hdmi_hal_ *hal, hi_bool *intr_status)
{
    hi_unused(hal);
    *intr_status = (hdmi_reg_aon_intr_stat0_get() == 0) ? HI_FALSE : HI_TRUE;
    return;
}

static hi_void hal_hdmi_hot_plug_status_get(const struct hdmi_hal_ *hal, hi_bool *hot_plug)
{
    hdmi_hpd_rsen hpd_rsen = {0};

    if (hot_plug != HI_NULL) {
        hdmi_if_null_return_void(hal);
        hal_hdmi_ctrl_hpd_rsen_get(hal->hal_ctx.hdmi_id, &hpd_rsen);
        *hot_plug = hpd_rsen.hpd_on;
    } else {
        hdmi_err("ctrl hot_plug = null!\n");
    }

    return;
}

static hi_void hal_hdmi_audio_mute_set(const struct hdmi_hal_ *hal, hi_bool audio_mute)
{
    hdmi_if_null_return_void(hal);
    hal_hdmi_ctrl_audio_mute_set(hal->hal_ctx.hdmi_id, audio_mute);
    return;
}

static hi_s32 hal_hdmi_audio_path_set(const struct hdmi_hal_ *hal, hdmi_audio_config *audio_cfg)
{
    hdmi_audio_path audio_path = {0};

    hdmi_if_null_return(hal, HI_FAILURE);
    hdmi_if_null_return(audio_cfg, HI_FAILURE);

    audio_path.sound_intf  = audio_cfg->sound_intf;
    audio_path.sample_rate = audio_cfg->sample_fs;
    audio_path.channel_num = audio_cfg->layout;
    audio_path.sample_bit  = audio_cfg->sample_depth;
    audio_path.pixel_clk   = audio_cfg->tmds_clk;
    hal_hdmi_ctrl_audio_path_set(hal->hal_ctx.hdmi_id, &audio_path);

    return HI_SUCCESS;
}

static hi_void hal_hdmi_audio_path_enable_set(const struct hdmi_hal_ *hal, hi_bool enable)
{
    hdmi_if_null_return_void(hal);
    hal_hdmi_ctrl_audio_path_enable_set(hal->hal_ctx.hdmi_id, enable);
    return;
}

static hi_s32 hal_hdmi_edid_raw_data_read(const struct hdmi_hal_ *hal, hi_u32 size, hi_u8 out_buffer[])
{
    hi_s32 ret;

    hdmi_if_null_return(hal, HI_FAILURE);
    hdmi_if_null_return(out_buffer, HI_FAILURE);
    ret = hal_hdmi_ddc_edid_raw_get(hal->hal_ctx.hdmi_id, size, out_buffer);

    return ret;
}

static hi_void hal_hdmi_phy_output_enable_get(const struct hdmi_hal_ *hal, hi_bool *enable)
{
#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_oe_get(enable);
#endif
    hi_unused(hal);
    return;
}

static hi_void hal_hdmi_video_mute_set(const struct hdmi_hal_ *hal, hi_bool v_mute)
{
    hdmi_if_null_return_void(hal);
    hal_hdmi_ctrl_video_mute_set(hal->hal_ctx.hdmi_id, v_mute);
    return;
}

static hi_void hal_hdmi_black_data_set(const struct hdmi_hal_ *hal, hdmi_black_frame_info *black)
{
    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(black);
    hal_hdmi_ctrl_video_mute_set(hal->hal_ctx.hdmi_id, black->black_enable);

    return;
}

#ifdef HDMI_DEBUG_SUPPORT
static hi_void hal_hdmi_debug(const struct hdmi_hal_ *hal, hdmi_debug_cmd debug_cmd, hi_void *data)
{
    hdmi_if_null_return_void(data);
    hdmi_if_null_return_void(hal);

    switch (debug_cmd) {
        case HDMI_DEBUG_CMD_COLOR_BAR:
            hal_hdmi_cbar_enable(*(hi_bool *)data);
            break;
        case HDMI_DEBUG_CMD_DITHER:
            hal_hdmi_ctrl_dither_dbg_set(hal->hal_ctx.hdmi_id, *(hdmi_video_dither *)data);
            break;
        default:
            hdmi_err("debug_cmd = %u\n", debug_cmd);
            break;
    }

    return;
}
#endif

static hi_u32 *hal_hdmi_base_addr_get(const struct hdmi_hal_ *hal)
{
    hi_u32 *base_addr = HI_NULL;

    if (hal == HI_NULL) {
        hdmi_err("NULL ptr!\n");
        return HI_NULL;
    }

    base_addr = (hi_u32 *)hal->hal_ctx.base_addr;

    return base_addr;
}

#ifdef HDMI_SCDC_SUPPORT
static hi_void hal_hdmi_scdc_status_set(const struct hdmi_hal_ *hal, hdmi_scdc_status *status)
{
    scdc_attr attr = {0};

    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(status);

    attr.sink_read_quest   = HI_FALSE;
    attr.sink_scramble     = status->sink_scramble_on;
    attr.src_scramble      = status->source_scramble_on;
    attr.tmds_clk_ratio40x = status->tmds_bit_clk_ratio == SCDC_TMDS_BIT_CLK_RATIO_40X ? HI_TRUE : HI_FALSE;
    attr.scramble_interval = status->scramble_interval;
    attr.scramble_timeout  = status->scramble_timeout;
    hal_hdmi_scdc_attr_set(hal->hal_ctx.hdmi_id, &attr);

    return;
}

static hi_void hal_hdmi_scdc_status_get(const struct hdmi_hal_ *hal, hdmi_scdc_status *status)
{
    scdc_attr attr = {0};

    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(status);

    hal_hdmi_scdc_attr_get(hal->hal_ctx.hdmi_id, &attr);
    status->sink_read_quest    = HI_FALSE;
    status->sink_scramble_on   = attr.sink_scramble;
    status->source_scramble_on = attr.src_scramble;
    status->tmds_bit_clk_ratio = attr.tmds_clk_ratio40x ? SCDC_TMDS_BIT_CLK_RATIO_40X : SCDC_TMDS_BIT_CLK_RATIO_10X;
    status->scramble_interval  = attr.scramble_interval;
    status->scramble_timeout   = attr.scramble_timeout;

    return;
}

static hi_void hal_hdmi_scdc_config(const struct hdmi_hal_ *hal, hdmi_scdc_config *scdc_config)
{
    hi_unused(hal);
    hi_unused(scdc_config);
    return;
}
#endif

static hi_void hal_pfn_init(hdmi_hal *hal)
{
    hal->hal_hdmi_hardware_init             = hal_hdmi_hardware_init;
    hal->hal_hdmi_avmute_set                = hal_hdmi_avmute_set;
    hal->hal_hdmi_tmds_mode_set             = hal_hdmi_tmds_mode_set;
    hal->hal_hdmi_infoframe_set             = hal_hdmi_infoframe_set;
    hal->hal_hdmi_infoframe_enable_set      = hal_hdmi_infoframe_enable_set;
    hal->hal_hdmi_video_path_set            = hal_hdmi_video_path_set;
    hal->hal_hdmi_phy_power_enable_set      = hal_hdmi_phy_power_enable_set;
    hal->hal_hdmi_phy_output_enable_set     = hal_hdmi_phy_output_enable_set;
    hal->hal_hdmi_tx_capability_get         = hal_hdmi_tx_capability_get;
    hal->hal_hdmi_emi_status_get            = hal_hdmi_emi_status_get;
    hal->hal_hdmi_csc_param_set             = hal_hdmi_csc_param_set;
    hal->hal_hdmi_phy_set                   = hal_hdmi_phy_set;
    hal->hal_hdmi_ctrl_reset                = hal_hdmi_ctrl_reset;
    hal->hal_hdmi_phy_hw_spec_set           = hal_hdmi_phy_hw_spec_set;
    hal->hal_hdmi_phy_hw_spec_get           = hal_hdmi_phy_hw_spec_get;
    hal->hal_hdmi_hardware_status_get       = hal_hdmi_hardware_status_get;
    hal->hal_hdmi_sequencer_handler_process = hal_hdmi_sequencer_handler_process;
    hal->hal_hdmi_audio_mute_set            = hal_hdmi_audio_mute_set;
    hal->hal_hdmi_audio_path_set            = hal_hdmi_audio_path_set;
    hal->hal_hdmi_audio_path_enable_set     = hal_hdmi_audio_path_enable_set;
    hal->hal_hdmi_edid_raw_data_read        = hal_hdmi_edid_raw_data_read;
    hal->hal_hdmi_phy_output_enable_get     = hal_hdmi_phy_output_enable_get;
    hal->hal_hdmi_hot_plug_status_get       = hal_hdmi_hot_plug_status_get;
    hal->hal_hdmi_video_mute_set            = hal_hdmi_video_mute_set;
    hal->hal_hdmi_hdp_intr_status_get       = hal_hdmi_hdp_intr_status_get;
    hal->hal_hdmi_black_data_set            = hal_hdmi_black_data_set;
#ifdef HDMI_DEBUG_SUPPORT
    hal->hal_hdmi_debug                     = hal_hdmi_debug;
#endif
    hal->hal_hdmi_base_addr_get             = hal_hdmi_base_addr_get;
#ifdef HDMI_HDR_SUPPORT
    hal->hal_hdmi_hdr_timer_set             = hal_hdmi_hdr_timer_set;
#endif
#ifdef HDMI_SCDC_SUPPORT
    hal->hal_hdmi_scdc_config               = hal_hdmi_scdc_config;
    hal->hal_hdmi_scdc_status_get           = hal_hdmi_scdc_status_get;
    hal->hal_hdmi_scdc_status_set           = hal_hdmi_scdc_status_set;
#endif

    return;
}

hi_s32 hal_hdmi_open(hdmi_hal_init *hal_init, hdmi_hal **hal_handle)
{
    hdmi_hal     *hal = HI_NULL;
    hdmi_hal_cfg *hal_cfg = HI_NULL;

    if (hal_handle == HI_NULL) {
        hdmi_err("hal_handle == NULL!\n");
        return HI_FAILURE;
    }

    hal = (hdmi_hal *)osal_vmalloc(sizeof(hdmi_hal));
    if (hal == HI_NULL) {
        hdmi_err("alloc hdmi_hal struct memory fail\n");
        return HI_FAILURE;
    }
    (hi_void)memset_s(hal, sizeof(hdmi_hal), 0, sizeof(hdmi_hal));

    if (hal_init != HI_NULL) {
        hal->hal_ctx.callback  = hal_init->event_callback;
        hal->hal_ctx.hdmi_dev  = hal_init->event_data;
        hal->hal_ctx.hdmi_id   = hal_init->hdmi_dev_id;
        hal->hal_ctx.base_addr = hal_init->base_addr;
        hal->hal_ctx.phy_addr  = hal_init->phy_addr;
    } else {
        hdmi_info("hal_init null! open in boot!\n");
    }

    hal_cfg = intf_hal_info_ptr_get(hal->hal_ctx.hdmi_id);
    if (hal_cfg == HI_NULL) {
        osal_vfree(hal);
        hdmi_err("hal_cfg null, fail!\n");
        return HI_FAILURE;
    }

    intf_tx_capability_init(hal->hal_ctx.hdmi_id);
#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_init(hal_init);
#endif
    hal_hdmi_mach_init();
    hal_hdmi_ctrl_init(hal->hal_ctx.hdmi_id, hal_init);
    hal_hdmi_ddc_init(hal->hal_ctx.hdmi_id);
#ifdef HDMI_SCDC_SUPPORT
    hal_hdmi_scdc_init(hal->hal_ctx.hdmi_id);
#endif
    hal_pfn_init(hal);

    *hal_handle = hal;
    hal_cfg->init = HI_TRUE;

    return HI_SUCCESS;
}

void hal_hdmi_close(struct hdmi_hal_ *hal)
{
    hdmi_hal_cfg *hal_cfg = (hdmi_hal_cfg *)HI_NULL;

    hdmi_if_null_return_void(hal);
    hal_cfg = intf_hal_info_ptr_get(hal->hal_ctx.hdmi_id);
    hdmi_if_null_return_void(hal_cfg);

#ifdef HDMI_SCDC_SUPPORT
    hal_hdmi_scdc_deinit(hal->hal_ctx.hdmi_id);
#endif
    hal_hdmi_ddc_deinit(hal->hal_ctx.hdmi_id);
    hal_hdmi_ctrl_deinit(hal->hal_ctx.hdmi_id);
    hal_hdmi_mach_deinit();
#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_deinit();
#endif
    hdmi_reg_crg_deinit();

    hal->hal_ctx.base_addr = HI_NULL;
    hal->hal_ctx.phy_addr  = HI_NULL;
    (hi_void)memset_s(hal, sizeof(hdmi_hal), 0, sizeof(hdmi_hal));
    osal_vfree(hal);
    hal = HI_NULL;

    hal_cfg->init = HI_FALSE;

    return;
}


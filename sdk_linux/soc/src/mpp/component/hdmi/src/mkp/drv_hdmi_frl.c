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
#include "drv_hdmi_frl.h"
#include "drv_hdmi_compatibility.h"
#include "hdmi_osal.h"
#include "drv_hdmi_edid.h"
#include "hdmi_hal_frl.h"

#define DATAFLOW_METERING_THR 492
#define LINK_TRAINING_TIMEOUT 200
#define FRL_MACH_START        0
#define FRL_MACH_STOP         1
#define FRL_MACH_BUTT         2

static hi_u32 g_frl_stop = FRL_MACH_BUTT;

static hi_void frl_rate_info_get(hi_u32 frl_rate, hi_u32 *frl_bit_rate, hi_u32 *frl_lane_num)
{
    if (frl_rate <= FRL_WORK_MODE_3L6G) {
        *frl_lane_num = FRL_3_LANES;
    } else {
        *frl_lane_num = FRL_4_LANES;
    }

    switch (frl_rate) {
        case FRL_WORK_MODE_3L3G:
            *frl_bit_rate = DFM_BIT_RATE_3;
            break;
        case FRL_WORK_MODE_3L6G:
        case FRL_WORK_MODE_4L6G:
            *frl_bit_rate = DFM_BIT_RATE_6;
            break;
        case FRL_WORK_MODE_4L8G:
            *frl_bit_rate = DFM_BIT_RATE_8;
            break;
        case FRL_WORK_MODE_4L10G:
            *frl_bit_rate = DFM_BIT_RATE_10;
            break;
        case FRL_WORK_MODE_4L12G:
            *frl_bit_rate = DFM_BIT_RATE_12;
            break;
        default:
            break;
    }

    return;
}

static hi_s32 sample_fs_convert_to_audio_rate(hdmi_sample_rate sample_fs)
{
    hi_s32 audio_rate;

    switch (sample_fs) {
        case HDMI_SAMPLE_RATE_32K:
            audio_rate = AUDIO_SAMPLE_RATE_32K;
            break;
        case HDMI_SAMPLE_RATE_44K:
            audio_rate = AUDIO_SAMPLE_RATE_44K;
            break;
        case HDMI_SAMPLE_RATE_48K:
            audio_rate = AUDIO_SAMPLE_RATE_48K;
            break;
        default:
            hdmi_warn("error sample fs: %u\n", sample_fs);
            audio_rate = AUDIO_SAMPLE_RATE_UNKNOWN;
            break;
    }

    return audio_rate;
}

static hi_s32 deep_clolor_convert_to_color_depth(hdmi_deep_color deep_color)
{
    hi_s32 color_depth;

    switch (deep_color) {
        case HDMI_DEEP_COLOR_24BIT:
            color_depth = DFM_COLOR_DEPTH_8;
            break;
        case HDMI_DEEP_COLOR_30BIT:
            color_depth = DFM_COLOR_DEPTH_10;
            break;
        case HDMI_DEEP_COLOR_36BIT:
            color_depth = DFM_COLOR_DEPTH_12;
            break;
        case HDMI_DEEP_COLOR_48BIT:
            color_depth = DFM_COLOR_DEPTH_16;
            break;
        default:
            color_depth = DFM_COLOR_DEPTH_8;
            break;
    }

    return color_depth;
}

static hi_s32 color_space_convert_to_pixel_format(hdmi_colorspace color_space)
{
    hi_s32 pixel_format;

    switch (color_space) {
        case HDMI_COLORSPACE_RGB:
            pixel_format = DFM_PIXEL_FORMAT_MODE_0;
            break;
        case HDMI_COLORSPACE_YCBCR420:
            pixel_format = DFM_PIXEL_FORMAT_MODE_1;
            break;
        case HDMI_COLORSPACE_YCBCR422:
            pixel_format = DFM_PIXEL_FORMAT_MODE_2;
            break;
        case HDMI_COLORSPACE_YCBCR444:
            pixel_format = DFM_PIXEL_FORMAT_MODE_3;
            break;
        default:
            pixel_format = DFM_PIXEL_FORMAT_MODE_0;
            break;
    }

    return pixel_format;
}

static hdmi_video_def *frl_get_format_param(hdmi_video_timing timing,
                                            hdmi_picture_aspect aspect, hi_bool _3d_enable)
{
    hdmi_video_code_vic vic;
    hdmi_video_def *tmp_fmt = HI_NULL;

    if (timing < HDMI_VIDEO_TIMING_VESA_DEFINE) {
        vic = drv_hdmi_vic_search(timing, aspect, _3d_enable);
        tmp_fmt = drv_hdmi_comm_format_param_get(vic);
    } else if (timing < HDMI_VIDEO_TIMING_USER_DEFINE) {
        tmp_fmt = drv_hdmi_vesa_format_param_get(timing);
    }

    return tmp_fmt;
}

static hi_s32 frl_tx_ffe_set(const hdmi_device *hdmi_dev)
{
    hi_s32 ret = HI_SUCCESS;
    hdmi_phy_cfg phy_cfg = {0};

    phy_cfg.mode_cfg = HDMI_PHY_MODE_CFG_TXFFE;
    phy_cfg.rate = (hdmi_work_mode)hdmi_dev->frl_info.rate_info.cur_rate;

#ifndef HDMI_FPGA_SUPPORT
    hal_call_void(hal_hdmi_phy_set, hdmi_dev->hal, &phy_cfg);
#endif

    return ret;
}

static hi_void frl_phy_set(hdmi_device *hdmi_dev)
{
    hdmi_phy_cfg   phy_cfg  = {0};
    hdmi_app_attr *app_attr = HI_NULL;
    hdmi_vo_attr  *vo_attr  = HI_NULL;

    app_attr = &hdmi_dev->attr.app_attr;
    vo_attr = &hdmi_dev->attr.vo_attr;

    /* PHY config */
    phy_cfg.emi_enable  = hdmi_dev->emi_enable;
    phy_cfg.pixel_clk   = vo_attr->clk_fs;
    phy_cfg.tmds_clk    = vo_attr->hdmi_adapt_pix_clk;
    phy_cfg.deep_color  = app_attr->deep_color_mode;
    phy_cfg.color_space = app_attr->out_color_space;
    phy_cfg.trace_len   = hdmi_dev->mode_param.trace_len;

    hdmi_info("start mode: %u\n", hdmi_dev->frl_info.mode);
    if (hdmi_dev->frl_info.mode == HDMI_FRL_MODE_FRL) {
        phy_cfg.mode_cfg = HDMI_PHY_MODE_CFG_FRL;
        (hi_void)memset_s(phy_cfg.aen_tx_ffe, sizeof(phy_cfg.aen_tx_ffe), FRL_TXFFE_MODE_0, sizeof(phy_cfg.aen_tx_ffe));
        phy_cfg.rate = (hdmi_work_mode)hdmi_dev->frl_info.rate_info.cur_rate;
    }

    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_FALSE);
    hal_call_void(hal_hdmi_phy_set, hdmi_dev->hal, &phy_cfg);
    if (hdmi_dev->frl_info.mode == HDMI_FRL_MODE_TMDS) {
        hdmi_mode_strategy(hdmi_dev);
    }
    hal_call_void(hal_hdmi_ctrl_reset, hdmi_dev->hal);
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_TRUE);

    return;
}

static hi_void frl_dfm_construct(dfm_in *dfm, const hdmi_ao_attr *ao_attr,
                                 const hdmi_app_attr *app_attr, const hdmi_video_def *video_def)
{
    dfm->hactive = video_def->hactive;
    dfm->vactive = video_def->vactive;
    dfm->hblank  = video_def->hblank;
    dfm->vblank  = video_def->vblank;
    dfm->hfront  = video_def->hfront;
    dfm->hsync   = video_def->hsync;
    dfm->hback   = video_def->hback;
    dfm->vfront  = video_def->vfront;
    dfm->vsync   = video_def->vsync;
    dfm->vback   = video_def->vback;
    dfm->v_freq  = video_def->rate;
    dfm->color_depth  = deep_clolor_convert_to_color_depth(app_attr->deep_color_mode);
    dfm->pixel_format = color_space_convert_to_pixel_format(app_attr->out_color_space);
    dfm->audio_rate   = sample_fs_convert_to_audio_rate(ao_attr->sample_fs);
    dfm->layout       = (ao_attr->channels > HDMI_AUDIO_FORMAT_2CH) ? HI_TRUE : HI_FALSE;
    /* ACAT packet_type */
    dfm->acat = AUDIO_CHANNEL_ALLOC_TYPE3;
    dfm->packet_type = AUDIO_SAMPLE_PACKET;

    return;
}

static hi_u8 frl_get_sink_version(hdmi_device_id hdmi_id)
{
    hi_s32       ret;
    hi_u8        sink_ver = 0;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hal_call_ret(ret, hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_GET_SINK_VER, &sink_ver);
    sink_ver = (ret != HI_SUCCESS) ? 0 : sink_ver;

    return sink_ver;
}

static hi_bool frl_get_flt_update(hdmi_device *hdmi_dev)
{
    hi_s32  ret;
    hi_bool flt_update = HI_FALSE;

    hal_call_ret(ret, hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_GET_FLT_UPDATE, &flt_update);
    flt_update = (ret != HI_SUCCESS) ? HI_FALSE : flt_update;

    return flt_update;
}

static hi_void frl_set_flt_update(hdmi_device *hdmi_dev, hi_bool flt_update)
{
    hal_call_void(hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_SET_FLT_UPDATE, &flt_update);
    return;
}

static hi_bool frl_get_frl_start(hdmi_device *hdmi_dev)
{
    hi_bool flt_start = HI_FALSE;
    hal_call_void(hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_GET_FRL_START, &flt_start);
    return flt_start;
}

static hi_void frl_set_frl_start(hdmi_device *hdmi_dev, hi_bool flt_start)
{
    hal_call_void(hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_SET_FRL_START, &flt_start);
    return;
}

static hi_void frl_get_test_config(hdmi_device *hdmi_dev, scdc_test_config *test_cfg)
{
    hal_call_void(hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_GET_TEST_CONFIG, test_cfg);
    return;
}

static hi_void frl_set_config1(hdmi_device *hdmi_dev, scdc_config1 *cfg)
{
    hal_call_void(hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_SET_CONFIG1, cfg);
    return;
}

static hi_bool frl_get_flt_ready(hdmi_device *hdmi_dev)
{
    hi_bool flt_ready = HI_FALSE;
    hal_call_void(hal_hdmi_scdc_process, hdmi_dev->hal, SCDC_CMD_GET_FLT_READY, &flt_ready);
    return flt_ready;
}

static hi_s32 frl_check_capability(hdmi_device *hdmi_dev)
{
    hdmi_tx_capability_data tx_cap        = {0};
    hdmi_sink_capability   *sink_cap      = HI_NULL;
    hdmi_frl_info          *curr_frl_info = HI_NULL;
    hdmi_edid_data          edid_ret;
    hi_s32                  ret;
    hi_bool                 support_frl = HI_FALSE;
    hi_u32                  sink_scdc_ver;

    curr_frl_info = &hdmi_dev->frl_info;
    hal_call_void(hal_hdmi_tx_capability_get, hdmi_dev->hal, &tx_cap);
    edid_ret = drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap);
    if (edid_ret == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("invalid edid_capability from sink!\n");
        return (hi_s32)edid_ret;
    }
    sink_scdc_ver = frl_get_sink_version(hdmi_dev->hdmi_dev_id);
    hdmi_info("tx_hdmi_21:%u, max_frl_rate:%u, support_scdc:%u, scdc_version:%u\n",
              tx_cap.tx_hdmi_21, sink_cap->max_frl_rate, sink_cap->support_scdc, sink_scdc_ver);

    support_frl = ((tx_cap.tx_hdmi_21) && (sink_cap->max_frl_rate > 0) &&
        (sink_cap->support_scdc) && (sink_scdc_ver != 0)) ? HI_TRUE : HI_FALSE;

    if (support_frl || hdmi_dev->attr.app_attr.auth_mode == HI_TRUE) {
        if (hdmi_dev->attr.app_attr.auth_mode == HI_TRUE) {
            curr_frl_info->rate_info.max_rate = tx_cap.tx_max_frl_rate;
        } else {
            curr_frl_info->rate_info.max_rate = (sink_cap->max_frl_rate > tx_cap.tx_max_frl_rate) ?
                tx_cap.tx_max_frl_rate : sink_cap->max_frl_rate;
        }
        ret = HI_SUCCESS;
        hdmi_info("frl check capability success\n");
    } else {
        curr_frl_info->rate_info.max_rate = 0;
        ret = HI_FAILURE;
        hdmi_info("frl check capability fail\n");
    }
    curr_frl_info->scdc_present = sink_cap->support_scdc;

    return ret;
}

static hi_bool frl_strategy_check_fmt(hdmi_frl_strategy_mode mode, const hdmi_vo_attr *vo_attr,
    hdmi_video_format_type type)
{
    hi_bool frl_fmt = HI_TRUE;

    switch (mode) {
        case HDMI_FRL_STRATEGY_MODE_1:
            if (vo_attr->hdmi_adapt_pix_clk <= HDMI_EDID_MAX_HDMI20_TMDS_RATE &&
                vo_attr->clk_fs <= HDMI_EDID_MAX_HDMI20_TMDS_RATE) {
                hdmi_info("hdmi_adapt_pix_clk(%u) pixel clk(%u)\n", vo_attr->hdmi_adapt_pix_clk, vo_attr->clk_fs);
                frl_fmt = HI_FALSE;
            }
            break;
        case HDMI_FRL_STRATEGY_MODE_2:
            if (vo_attr->hdmi_adapt_pix_clk <= HDMI_EDID_MAX_HDMI14_TMDS_RATE) {
                frl_fmt = HI_FALSE;
            }
            break;
        case HDMI_FRL_STRATEGY_MODE_3:
            if (type == HDMI_VIDEO_INTERLACE) {
                frl_fmt = HI_FALSE;
            }
            break;
        default:
            break;
    }

    return frl_fmt;
}

static hi_bool frl_check_format(hdmi_frl_info *curr_frl_info, const hdmi_attr *attr)
{
    hi_u32  i;
    hi_u8   max_rate;
    hi_bool _3d_enable = HI_FALSE;
    hi_bool capable    = HI_FALSE;
    dfm_in  dfm        = {0};
    const hdmi_vo_attr  *vo_attr  = HI_NULL;
    const hdmi_ao_attr  *ao_attr  = HI_NULL;
    const hdmi_app_attr *app_attr = HI_NULL;
    hdmi_video_def *video_def     = HI_NULL;

    vo_attr = &attr->vo_attr;
    ao_attr = &attr->ao_attr;
    app_attr = &attr->app_attr;
    max_rate = curr_frl_info->rate_info.max_rate;

    _3d_enable = (vo_attr->stereo_mode == HDMI_3D_BUTT) ? HI_FALSE : HI_TRUE;
    video_def = frl_get_format_param(vo_attr->video_timing, vo_attr->picture_aspect, _3d_enable);
    if (video_def == HI_NULL) {
        hdmi_warn("no this fmt's param, transmit by tmds.\n");
        return HI_FALSE;
    }

    hdmi_info("video_code: %u\n", video_def->video_code);
    curr_frl_info->tmds_clk = vo_attr->hdmi_adapt_pix_clk;
    if (frl_strategy_check_fmt(curr_frl_info->strategy_mode, vo_attr, video_def->pi_type) == HI_FALSE) {
        return HI_FALSE;
    }

    frl_dfm_construct(&dfm, ao_attr, app_attr, video_def);
    if (max_rate > FRL_WORK_MODE_4L12G) {
        max_rate = FRL_WORK_MODE_4L12G;
        hdmi_info("sink's max FRL rate maybe wrong(%u)\n", curr_frl_info->rate_info.max_rate);
        return HI_TRUE;
    }

    for (i = max_rate; i > 0; i--) {
        frl_rate_info_get(i, &dfm.bit_rate, &dfm.lane_num);
        if (drv_hdmi_dfm_format_support(&dfm) == HI_TRUE) {
            curr_frl_info->rate_info.min_rate = i;
            hdmi_info("min_rate: %u\n", curr_frl_info->rate_info.min_rate);
        } else {
            break;
        }
    }

    capable = (i == max_rate) ? HI_FALSE : HI_TRUE;
    hdmi_info("max FRL rate(%u), video_code(%u) can %s be transmitted.\n",
              max_rate, video_def->video_code, (capable == HI_FALSE) ? "not" : "");

    return capable;
}

static hi_void frl_set_train_rate(hdmi_device *hdmi_dev)
{
    hdmi_frl_info *curr_frl_info = HI_NULL;
    scdc_config1  cfg            = {0};

    curr_frl_info = &hdmi_dev->frl_info;

    if (curr_frl_info->mode == HDMI_FRL_MODE_FRL) {
        cfg.ffe_levels = FRL_TXFFE_MODE_0;
        cfg.frl_rate = curr_frl_info->rate_info.cur_rate;
    } else {
        cfg.ffe_levels = 0;
        cfg.frl_rate = 0;
    }
    frl_set_config1(hdmi_dev, &cfg);

    return;
}

static hi_bool frl_is_cts_mode(hdmi_device *hdmi_dev)
{
    hi_bool ret = HI_FALSE;
    scdc_test_config test_cfg = {0};

    frl_get_test_config(hdmi_dev, &test_cfg);
    ret = ((test_cfg.frl_max == 1 && test_cfg.dsc_frl_max == 0) ||
           (test_cfg.frl_max == 0 && test_cfg.dsc_frl_max == 1)) ? HI_TRUE : HI_FALSE;

    return ret;
}

static hi_s32 frl_get_cur_rate(hdmi_frl_info *curr_frl_info)
{
    hi_s32       ret = HI_SUCCESS;
    hi_u8        cur_frl_rate;
    static hi_u8 pre_rate;

    cur_frl_rate = curr_frl_info->rate_info.cur_rate;
    if (curr_frl_info->cts_mode == HI_TRUE) {
        pre_rate = cur_frl_rate;
        cur_frl_rate = curr_frl_info->rate_info.max_rate;
    } else {
        if (pre_rate != 0) {
            cur_frl_rate = pre_rate;
            pre_rate = 0;
        } else {
            if (curr_frl_info->rate_select == HDMI_FRL_RATE_SELECT_BIG) {
                cur_frl_rate--;
            } else {
                cur_frl_rate++;
            }
            if ((cur_frl_rate > curr_frl_info->rate_info.max_rate) ||
                (cur_frl_rate < curr_frl_info->rate_info.min_rate)) {
                return HI_FAILURE;
            }
        }
    }
    curr_frl_info->rate_info.cur_rate = cur_frl_rate;

    return ret;
}

static hi_void frl_n_cts_config(hdmi_device *hdmi_dev)
{
    hdmi_audio_ncts audio_cfg = {0};

    audio_cfg.hdmi_mode = hdmi_dev->frl_info.mode;
    audio_cfg.sample_rate = hdmi_dev->attr.ao_attr.sample_fs;

    if (audio_cfg.hdmi_mode == HDMI_FRL_MODE_FRL) {
        audio_cfg.frl_rate = hdmi_dev->frl_info.rate_info.cur_rate;
    } else {
        audio_cfg.pixel_clk = hdmi_dev->attr.vo_attr.clk_fs;
    }
    hal_call_void(hal_hdmi_audio_n_cts_set, hdmi_dev->hal, &audio_cfg);

    return;
}

static hi_void frl_train_exception(hdmi_device *hdmi_dev)
{
    hdmi_phy_cfg              phy_cfg       = {0};
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_app_attr            *app_attr      = HI_NULL;
    hdmi_vo_attr             *vo_attr       = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;
    app_attr = &hdmi_dev->attr.app_attr;
    vo_attr = &hdmi_dev->attr.vo_attr;

    hdmi_err("FRL error!\n");
#ifndef HDMI_FPGA_SUPPORT
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_FALSE);
#endif
    mach_info->train_status = HDMI_FRL_TRAIN_STEP_STOP;
    curr_frl_info->start = HI_FALSE;
    mach_info->start_time = hdmi_osal_get_time_in_ms();

    if (curr_frl_info->fail_strategy != HDMI_FRL_FAIL_TMDS) {
        hdmi_err("FRL error, nothing to do!\n");
        return;
    }
    /* PHY configs */
    phy_cfg.emi_enable = hdmi_dev->emi_enable;
    phy_cfg.pixel_clk = vo_attr->clk_fs;
    phy_cfg.tmds_clk = vo_attr->hdmi_adapt_pix_clk;
    phy_cfg.deep_color = app_attr->deep_color_mode;
    phy_cfg.mode_cfg = HDMI_FRL_MODE_TMDS;
    phy_cfg.color_space = app_attr->out_color_space;
    hdmi_err("FRL error, change to TMDS!\n");
    if (phy_cfg.tmds_clk > HDMI_EDID_MAX_HDMI20_TMDS_RATE) {
        hdmi_err("TMDS clk > 600M. can't work TMDS mode. \n");
        return;
    }
    curr_frl_info->mode = HDMI_FRL_MODE_TMDS;
    if (curr_frl_info->scdc_present == HI_TRUE) {
        frl_set_train_rate(hdmi_dev);
    }
    frl_n_cts_config(hdmi_dev);
#ifndef HDMI_FPGA_SUPPORT
    hal_call_void(hal_hdmi_phy_set, hdmi_dev->hal, &phy_cfg);
    hdmi_mode_strategy(hdmi_dev);
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, HI_TRUE);
#endif

    return;
}

static hi_void frl_config_and_start_train(hdmi_device *hdmi_dev)
{
    hi_u32 frl_rate;
    hdmi_frl_train_config train_config  = {0};
    hdmi_frl_info        *curr_frl_info = HI_NULL;
    scdc_test_config      test_cfg      = {0};
    compatibility_info   *compat_info   = HI_NULL;

    compat_info = compat_info_get(hdmi_dev->hdmi_dev_id);
    hdmi_if_null_return_void(compat_info);

    curr_frl_info = &hdmi_dev->frl_info;
    frl_rate = curr_frl_info->rate_info.cur_rate;

    train_config.frl_rate = frl_rate;
    train_config.ffe_levels = FRL_TXFFE_MODE_0;
    frl_get_test_config(hdmi_dev, &test_cfg);
    train_config.frl_no_timeout = test_cfg.flt_no_timeout;
    hdmi_info("frl_rate %u, frl_no_timeout %u.\n", frl_rate, train_config.frl_no_timeout);
    train_config.mach_mode = hdmi_dev->frl_info.state_mach_info.mach_mode;
    train_config.train_timeout = hdmi_dev->frl_info.state_mach_info.train_timeout;
    train_config.sw_train_mode = hdmi_dev->frl_info.state_mach_info.sw_train_mode;
    train_config.ctl_type_config = compat_info->ctl_type_config;

    hal_call_void(hal_hdmi_frl_train_config, hdmi_dev->hal, &train_config);
    hal_call_void(hal_hdmi_frl_enable, hdmi_dev->hal, HI_FALSE);
    hal_call_void(hal_hdmi_frl_train_start, hdmi_dev->hal);

    return;
}

static hi_void frl_get_train_result(hdmi_device *hdmi_dev, hdmi_frl_train *train_status)
{
    hdmi_if_null_return_void(hdmi_dev);
    hdmi_if_null_return_void(train_status);
    hal_call_void(hal_hdmi_frl_get_train_status, hdmi_dev->hal, train_status);
    return;
}

static hi_void frl_enable_output(hdmi_device *hdmi_dev, hi_bool enable)
{
    hdmi_if_null_return_void(hdmi_dev);
    hal_call_void(hal_hdmi_frl_enable, hdmi_dev->hal, enable);
    return;
}

hi_void drv_hdmi_frl_mach_init(hdmi_device_id hdmi_id)
{
    hdmi_device              *hdmi_dev      = HI_NULL;
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return_void(hdmi_dev);

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    if (mach_info->start_mach != HI_TRUE) {
        curr_frl_info->strategy_mode = HDMI_FRL_STRATEGY_MODE_1;
        mach_info->start_mach = HI_TRUE;
        mach_info->start_time = hdmi_osal_get_time_in_ms();
        mach_info->wait_handle_ms = 100;  /* 100: FRL training machine RESULT_HANDLE timeout interval */
        mach_info->wait_ready_ms = 20;    /* 20: FRL training machine READY_CHECK timeout interval */
        mach_info->wait_retrain_ms = 500; /* 500: FRL training machine RETRAIN_CHECK timeout interval */
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_BUTT;
        curr_frl_info->rate_select = HDMI_FRL_RATE_SELECT_LITTLE;
        curr_frl_info->max_fail_times = 3; /* 3: max training times if training failed */
        curr_frl_info->work_en = HI_FALSE;
        curr_frl_info->fail_strategy = HDMI_FRL_FAIL_NONE;
        curr_frl_info->state_mach_info.mach_mode = HDMI_FRL_MACH_MODE_TIMEOUT;
        curr_frl_info->state_mach_info.train_timeout = LINK_TRAINING_TIMEOUT;
        curr_frl_info->state_mach_info.sw_train_mode = FRL_SW_TRAIN_TIMER;
        curr_frl_info->rate_info.debug_rate = FRL_WORK_MODE_NONE;
        curr_frl_info->mode = HDMI_FRL_MODE_TMDS;
        (hi_void)memset_s(curr_frl_info->aen_cur_tx_ffe, sizeof(curr_frl_info->aen_cur_tx_ffe),
            FRL_TXFFE_MODE_0, sizeof(curr_frl_info->aen_cur_tx_ffe));
    }

    return;
}

hi_void drv_hdmi_frl_enable(hdmi_device_id hdmi_id, hi_bool enable)
{
    hdmi_device              *hdmi_dev      = HI_NULL;
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return_void(hdmi_dev);

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    curr_frl_info->start = enable;
    if (curr_frl_info->rate_info.debug_rate != 0) {
        hdmi_info("debug mode, use the rate:%u\n", curr_frl_info->rate_info.cur_rate);
        curr_frl_info->rate_info.cur_rate = curr_frl_info->rate_info.debug_rate;
    }

    curr_frl_info->cts_mode = frl_is_cts_mode(hdmi_dev);
    if (curr_frl_info->cts_mode == HI_TRUE) {
        frl_get_cur_rate(curr_frl_info);
        hdmi_info("cts mode, use the maxmum rate:%u\n", curr_frl_info->rate_info.cur_rate);
    }

    g_frl_stop = (enable == HI_TRUE) ? FRL_MACH_START : FRL_MACH_STOP;
    mach_info->start_time = hdmi_osal_get_time_in_ms();
    hdmi_info("cur_rate(%u), enable: %u, train_status:%u, start_time:%llu\n",
              curr_frl_info->rate_info.cur_rate, enable,
              mach_info->train_status, mach_info->start_time);

    return;
}

hi_s32 drv_hdmi_frl_mode_strategy(hdmi_device_id hdmi_id)
{
    hi_s32 ret;
    hdmi_device   *hdmi_dev      = HI_NULL;
    hdmi_attr     *attr          = HI_NULL;
    hdmi_frl_info *curr_frl_info = HI_NULL;
    hi_u8         *cur_frl_rate  = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    attr = &hdmi_dev->attr;
    curr_frl_info = &hdmi_dev->frl_info;
    cur_frl_rate = &curr_frl_info->rate_info.cur_rate;

    ret = frl_check_capability(hdmi_dev);
    if (ret == HDMI_EDID_DATA_INVALID) {
        return ret;
    } else if (ret != HI_SUCCESS) {
        hdmi_info("frl_check_capability fail, change to TMDS\n");
        curr_frl_info->mode = HDMI_FRL_MODE_TMDS;
    } else {
        if (frl_check_format(curr_frl_info, attr) != HI_TRUE) {
            hdmi_info("frl_check_format fail, change to TMDS\n");
            curr_frl_info->mode = HDMI_FRL_MODE_TMDS;
        } else {
            hdmi_info("frl_check_capability success, FRL mode\n");
            curr_frl_info->mode = HDMI_FRL_MODE_FRL;
        }
    }

    if (curr_frl_info->mode == HDMI_FRL_MODE_TMDS) {
        if (attr->vo_attr.hdmi_adapt_pix_clk > HDMI_EDID_MAX_HDMI20_TMDS_RATE) {
            hdmi_err("this format not support!\n");
            return HI_FAILURE;
        }
        if (curr_frl_info->scdc_present == HI_TRUE) {
            frl_set_train_rate(hdmi_dev);
        }
    } else {
        if (curr_frl_info->rate_info.debug_rate != FRL_WORK_MODE_NONE) {
            *cur_frl_rate = curr_frl_info->rate_info.debug_rate;
        } else {
            *cur_frl_rate = (curr_frl_info->rate_select == HDMI_FRL_RATE_SELECT_BIG) ?
                curr_frl_info->rate_info.max_rate : curr_frl_info->rate_info.min_rate;
        }
        hdmi_dev->tmds_mode = HDMI_TMDS_MODE_HDMI_2_0;
        hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, hdmi_dev->tmds_mode);
        curr_frl_info->cts_mode = HI_FALSE;
    }

    return HI_SUCCESS;
}

static hi_void frl_training_result_check_step(hdmi_device *hdmi_dev, hdmi_frl_train_step *train_step)
{
    hdmi_frl_info  *curr_frl_info = HI_NULL;
    hdmi_frl_train *train_status  = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    train_status = &curr_frl_info->train_status;

    frl_get_train_result(hdmi_dev, train_status);
    if (train_status->frl_train_status == HDMI_FRL_TRAIN_SUCCESS) {
        *train_step = HDMI_FRL_TRAIN_STEP_RESULT_HANDLE;
        curr_frl_info->fail_count = 0;
    } else if (train_status->train_fail_res == HDMI_FRL_TRAIN_FAIL_FFECHANGE) {
        hdmi_err("TXFFE change\n");
        frl_tx_ffe_set(hdmi_dev);
        *train_step = HDMI_FRL_TRAIN_STEP_TRAIN_START;
        curr_frl_info->fail_count = 0;
    } else if (train_status->train_fail_res == HDMI_FRL_TRAIN_FAIL_RATECHANGE) {
        *train_step = HDMI_FRL_TRAIN_STEP_RATE_CHANGE;
        curr_frl_info->fail_count = 0;
    } else {
        curr_frl_info->fail_count++;
    }

    return;
}

static hi_void frl_training_rate_change_step(hdmi_device *hdmi_dev, hdmi_frl_train_step *train_step)
{
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    curr_frl_info->cts_mode = frl_is_cts_mode(hdmi_dev);
    if (frl_get_cur_rate(curr_frl_info) != HI_SUCCESS) {
        hdmi_err("FRL rate %u get failed!\n", curr_frl_info->rate_info.cur_rate);
        frl_train_exception(hdmi_dev);
        *train_step = mach_info->train_status;
        return;
    }
    /* FRL rate change */
    frl_phy_set(hdmi_dev);
    frl_set_train_rate(hdmi_dev);
    *train_step = HDMI_FRL_TRAIN_STEP_TRAIN_START;

    return;
}

static hi_void frl_start_check_step(hdmi_device *hdmi_dev, hi_u32 *timeout, hdmi_frl_train_step *train_step)
{
    hdmi_frl_info *curr_frl_info = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    if (frl_get_frl_start(hdmi_dev) == HI_TRUE) {
        hdmi_info("frl_start get...\n");
        if (curr_frl_info->start) {
            hdmi_info("training pass...\n");
            curr_frl_info->work_en = HI_TRUE;
            /* n/cts config */
            frl_n_cts_config(hdmi_dev);
            osal_msleep(1);
            frl_enable_output(hdmi_dev, curr_frl_info->work_en);
            osal_msleep(1);
            frl_set_frl_start(hdmi_dev, HI_TRUE);
            *timeout = 0;
            *train_step = HDMI_FRL_TRAIN_STEP_RETRAIN_CHECK;
        }
    }

    return;
}

static hi_void frl_retraining_check_step(hdmi_device *hdmi_dev)
{
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;
    if (frl_get_flt_update(hdmi_dev) == HI_TRUE) {
        hdmi_info("RETRAIN_CHECK flt_update get...\n");
        frl_set_flt_update(hdmi_dev, HI_TRUE);
        curr_frl_info->work_en = HI_FALSE;
        frl_enable_output(hdmi_dev, curr_frl_info->work_en);
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
    }

    return;
}

static hi_void frl_training_ready_check_step(hdmi_device *hdmi_dev)
{
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    mach_info = &hdmi_dev->frl_info.state_mach_info;
    if (frl_get_flt_ready(hdmi_dev) == HI_TRUE) {
        hdmi_info("READY_CHECK, start training.\n");
        frl_phy_set(hdmi_dev);
        frl_set_train_rate(hdmi_dev);
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
    }

    return;
}

static hi_void frl_training_step_mode(hdmi_device *hdmi_dev)
{
    hi_u64                    start_time;
    static hi_u32             timeout;
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;
    start_time = mach_info->start_time;
    switch (mach_info->train_status) {
        case HDMI_FRL_TRAIN_STEP_READY_CHECK:
            frl_training_ready_check_step(hdmi_dev);
            break;
        case HDMI_FRL_TRAIN_STEP_TRAIN_START:
            frl_config_and_start_train(hdmi_dev);
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_RESULT_CHECK;
            break;
        case HDMI_FRL_TRAIN_STEP_RESULT_CHECK:
            frl_training_result_check_step(hdmi_dev, &mach_info->train_status);
            break;
        case HDMI_FRL_TRAIN_STEP_RATE_CHANGE:
            frl_training_rate_change_step(hdmi_dev, &mach_info->train_status);
            break;
        case HDMI_FRL_TRAIN_STEP_RESULT_HANDLE:
            frl_start_check_step(hdmi_dev, &timeout, &mach_info->train_status);
            break;
        case HDMI_FRL_TRAIN_STEP_RETRAIN_CHECK:
            frl_retraining_check_step(hdmi_dev);
            break;
        case HDMI_FRL_TRAIN_STEP_STOP:
            curr_frl_info->work_en = HI_FALSE;
            frl_enable_output(hdmi_dev, curr_frl_info->work_en);
            start_time = hdmi_osal_get_time_in_ms();
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_BUTT;
            break;
        default:
            break;
    }
    mach_info->start_time = start_time;

    return;
}

static hi_void frl_training_ready_check_timeout(hdmi_device *hdmi_dev, hi_u64 *start_time, hi_u32 *timeout)
{
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    if ((hdmi_osal_get_time_in_ms() - *start_time) > mach_info->wait_ready_ms) {
        if (frl_get_flt_ready(hdmi_dev) == HI_TRUE) {
            hdmi_info("start training\n");
            frl_phy_set(hdmi_dev);
            frl_set_train_rate(hdmi_dev);
            *timeout = 0;
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
        } else {
            *timeout += 1;
        }
        *start_time = hdmi_osal_get_time_in_ms();
    } else {
        /* wait for a maximum of 10 times, (wait_ready_ms * 10) ms */
        if (*timeout < 10) {
            return;
        }

        hdmi_info("READY_CHECK exception: %u\n", *timeout);
        *timeout = 0;
        if (curr_frl_info->bypass & FRL_BYPASS_READY_CHECK) {
            hdmi_info("READY_CHECK bypass, goto TRAIN_START!\n");
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
            return;
        }
        frl_train_exception(hdmi_dev);
    }

    return;
}

static hi_void frl_training_result_check_timeout(hdmi_device *hdmi_dev)
{
    hdmi_frl_info  *curr_frl_info       = HI_NULL;
    hdmi_frl_train *train_status        = HI_NULL;
    hdmi_frl_state_mach_info *mach_info = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;
    train_status = &curr_frl_info->train_status;

    frl_get_train_result(hdmi_dev, train_status);
    if (train_status->frl_train_status == HDMI_FRL_TRAIN_SUCCESS) {
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_RESULT_HANDLE;
        curr_frl_info->fail_count = 0;
    } else if (train_status->train_fail_res == HDMI_FRL_TRAIN_FAIL_FFECHANGE) {
        hdmi_err("TXFFE change\n");
        frl_tx_ffe_set(hdmi_dev);
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
        curr_frl_info->fail_count = 0;
    } else if (train_status->train_fail_res == HDMI_FRL_TRAIN_FAIL_RATECHANGE) {
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_RATE_CHANGE;
        curr_frl_info->fail_count = 0;
    } else {
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
        curr_frl_info->fail_count++;
        if (curr_frl_info->max_fail_times <= curr_frl_info->fail_count) {
            curr_frl_info->fail_count = 0;
            if (curr_frl_info->bypass & FRL_BYPASS_RESULT_CHECK) {
                hdmi_info("RESULT_CHECK bypass, goto RESULT_HANDLE!\n");
                mach_info->train_status = HDMI_FRL_TRAIN_STEP_RESULT_HANDLE;
                return;
            }
            frl_train_exception(hdmi_dev);
        }
    }

    return;
}

static hi_void frl_training_rate_change_timeout(hdmi_device *hdmi_dev, hi_u64 *start_time)
{
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    curr_frl_info->cts_mode = frl_is_cts_mode(hdmi_dev);
    if (frl_get_cur_rate(curr_frl_info) != HI_SUCCESS) {
        hdmi_err("FRL rate(%u) get failed!\n", curr_frl_info->rate_info.cur_rate);
        frl_train_exception(hdmi_dev);
        return;
    }
    /* FRL rate change */
    frl_phy_set(hdmi_dev);
    frl_set_train_rate(hdmi_dev);
    mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
    *start_time = hdmi_osal_get_time_in_ms();

    return;
}

static hi_void frl_start_check_timeout(hdmi_device *hdmi_dev, hi_u32 *timeout, hi_u64 *start_time)
{
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    do {
        if (frl_get_frl_start(hdmi_dev) == HI_TRUE) {
            if (curr_frl_info->start) {
                curr_frl_info->work_en = HI_TRUE;
                /* n/cts config */
                frl_n_cts_config(hdmi_dev);
                frl_enable_output(hdmi_dev, curr_frl_info->work_en);
                frl_set_frl_start(hdmi_dev, HI_TRUE);
                *timeout = 0;
                mach_info->train_status = HDMI_FRL_TRAIN_STEP_RETRAIN_CHECK;
                break;
            }
        } else if (frl_get_flt_update(hdmi_dev) == HI_TRUE) {
            *timeout = 0;
            frl_set_flt_update(hdmi_dev, HI_TRUE);
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
            break;
        }

        if ((hdmi_osal_get_time_in_ms() - *start_time) > mach_info->wait_handle_ms) {
            *timeout += 1;
            break;
        }
        /* 2000: check the sink's status every 2ms required by protocol */
        osal_udelay(2000);
    } while (1);

    *start_time = hdmi_osal_get_time_in_ms();
    /* Maximum check 3 times */
    if (*timeout >= 3) {
        hdmi_err("training timeout!\n");
        *timeout = 0;
        frl_train_exception(hdmi_dev);
    }

    return;
}

static hi_void frl_retraining_check_timeout(hdmi_device *hdmi_dev, hi_u64 *start_time)
{
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;

    if ((hdmi_osal_get_time_in_ms() - *start_time) <= mach_info->wait_retrain_ms) {
        return;
    }
    if (frl_get_flt_update(hdmi_dev) != HI_TRUE) {
        return;
    }
    if (curr_frl_info->bypass & FRL_BYPASS_RETRAIN_CHECK) {
        hdmi_info("RETRAIN_CHECK bypass!\n");
        return;
    }

    frl_set_flt_update(hdmi_dev, HI_TRUE);
    curr_frl_info->work_en = HI_FALSE;
    frl_enable_output(hdmi_dev, curr_frl_info->work_en);
    mach_info->train_status = HDMI_FRL_TRAIN_STEP_TRAIN_START;
    *start_time = hdmi_osal_get_time_in_ms();

    return;
}

static hi_void frl_training_timeout_mode(hdmi_device *hdmi_dev)
{
    hi_u64                    start_time;
    static hi_u32             timeout;
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;
    start_time = mach_info->start_time;
    switch (mach_info->train_status) {
        case HDMI_FRL_TRAIN_STEP_READY_CHECK:
            frl_training_ready_check_timeout(hdmi_dev, &start_time, &timeout);
            break;
        case HDMI_FRL_TRAIN_STEP_TRAIN_START:
            frl_config_and_start_train(hdmi_dev);
            start_time = hdmi_osal_get_time_in_ms();
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_RESULT_CHECK;
            break;
        case HDMI_FRL_TRAIN_STEP_RESULT_CHECK:
            frl_training_result_check_timeout(hdmi_dev);
            start_time = hdmi_osal_get_time_in_ms();
            break;
        case HDMI_FRL_TRAIN_STEP_RATE_CHANGE:
            frl_training_rate_change_timeout(hdmi_dev, &start_time);
            break;
        case HDMI_FRL_TRAIN_STEP_RESULT_HANDLE:
            frl_start_check_timeout(hdmi_dev, &timeout, &start_time);
            break;
        case HDMI_FRL_TRAIN_STEP_RETRAIN_CHECK:
            frl_retraining_check_timeout(hdmi_dev, &start_time);
            break;
        case HDMI_FRL_TRAIN_STEP_STOP:
            curr_frl_info->work_en = HI_FALSE;
            frl_enable_output(hdmi_dev, curr_frl_info->work_en);
            start_time = hdmi_osal_get_time_in_ms();
            mach_info->train_status = HDMI_FRL_TRAIN_STEP_BUTT;
            break;
        default:
            break;
    }
    mach_info->start_time = start_time;

    return;
}

hi_void drv_hdmi_frl_train_mach(hdmi_device_id hdmi_id)
{
    hdmi_device              *hdmi_dev      = HI_NULL;
    hdmi_frl_info            *curr_frl_info = HI_NULL;
    hdmi_frl_train           *train_status  = HI_NULL;
    hdmi_frl_state_mach_info *mach_info     = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return_void(hdmi_dev);

    curr_frl_info = &hdmi_dev->frl_info;
    mach_info = &curr_frl_info->state_mach_info;
    train_status = &curr_frl_info->train_status;
    if (g_frl_stop == FRL_MACH_STOP) {
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_STOP;
        g_frl_stop = FRL_MACH_BUTT;
        train_status->frl_train_status = HDMI_FRL_TRAIN_NONE;
    } else if (g_frl_stop == FRL_MACH_START) {
        mach_info->train_status = HDMI_FRL_TRAIN_STEP_READY_CHECK;
        g_frl_stop = FRL_MACH_BUTT;
    }

    if (mach_info->mach_mode == HDMI_FRL_MACH_MODE_STEP) {
        frl_training_step_mode(hdmi_dev);
    } else {
        frl_training_timeout_mode(hdmi_dev);
    }

    return;
}

hi_bool dfm_calculate(hdmi_frl_info *curr_frl_info, hdmi_attr *attr)
{
    hi_u8   max_rate;
    dfm_in  dfm        = {0};
    hi_bool _3d_enable = HI_FALSE;
    hi_bool capable    = HI_FALSE;
    const hdmi_vo_attr  *vo_attr  = HI_NULL;
    const hdmi_ao_attr  *ao_attr  = HI_NULL;
    const hdmi_app_attr *app_attr = HI_NULL;
    hdmi_video_def *video_def     = HI_NULL;

    hdmi_if_null_return(curr_frl_info, HI_FALSE);
    hdmi_if_null_return(attr, HI_FALSE);

    vo_attr = &attr->vo_attr;
    ao_attr = &attr->ao_attr;
    app_attr = &attr->app_attr;
    max_rate = curr_frl_info->rate_info.max_rate;
    _3d_enable = (vo_attr->stereo_mode == HDMI_3D_BUTT) ? HI_FALSE : HI_TRUE;
    video_def  = frl_get_format_param(vo_attr->video_timing, vo_attr->picture_aspect, _3d_enable);
    hdmi_if_null_return(video_def, HI_FALSE);

    hdmi_info("video_code: %u\n", video_def->video_code);
    curr_frl_info->tmds_clk = vo_attr->hdmi_adapt_pix_clk;
    frl_dfm_construct(&dfm, ao_attr, app_attr, video_def);

    if (max_rate > FRL_WORK_MODE_4L12G) {
        capable = HI_FALSE;
        goto finish;
    }

    frl_rate_info_get(curr_frl_info->rate_info.cur_rate, &dfm.bit_rate, &dfm.lane_num);
    if (drv_hdmi_dfm_format_support(&dfm) == HI_TRUE) {
        capable = HI_TRUE;
    } else {
        capable = HI_FALSE;
    }
    hdmi_err("this fmt can %s be trans\n", (capable == HI_TRUE) ? "" : "not");

finish:
    return capable;
}

hi_s32 drv_hdmi_frl_mode_change(hdmi_device_id hdmi_id, const hdmi_frl_debug *debug)
{
    hdmi_device          *hdmi_dev      = HI_NULL;
    hdmi_frl_info        *curr_frl_info = HI_NULL;
    frl_debug             debug_cfg     = {0};
    hdmi_frl_train_config train_config  = {0};
    hdmi_tx_capability_data tx_cap      = {0};
    compatibility_info   *compat_info   = HI_NULL;

    hdmi_if_null_return(debug, HI_FAILURE);

    compat_info = compat_info_get(hdmi_id);
    hdmi_if_null_return(compat_info, HI_FAILURE);

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    curr_frl_info = &hdmi_dev->frl_info;

    hdmi_dev->frl_info.mode = debug->hdmi_mode;
    if (debug->hdmi_mode == HDMI_FRL_MODE_FRL) {
        hal_call_void(hal_hdmi_tx_capability_get, hdmi_dev->hal, &tx_cap);
        curr_frl_info->rate_info.cur_rate = debug->rate;
        curr_frl_info->rate_info.max_rate = tx_cap.tx_max_frl_rate;
        if (dfm_calculate(curr_frl_info, &(hdmi_dev->attr)) == HI_FALSE) {
            hdmi_err("can not trans\n");
            return HI_FAILURE;
        }
        train_config.frl_rate = debug->rate;
        train_config.ffe_levels = FRL_TXFFE_MODE_0;
        train_config.frl_no_timeout = HI_FALSE;
        train_config.ctl_type_config = compat_info->ctl_type_config;
        /* set frl rate to rx by scdc. */
        frl_set_train_rate(hdmi_dev);
        hal_call_void(hal_hdmi_frl_train_config, hdmi_dev->hal, &train_config);
        frl_phy_set(hdmi_dev);
        /* LM config & channel config */
        debug_cfg.debug_cmd = FRL_DEBUG_RATE;
        debug_cfg.rate = train_config.frl_rate;
        hal_call_void(hal_hdmi_debug, hdmi_dev->hal, HDMI_DEBUG_CMD_FRL, &debug_cfg);
        hal_call_void(hal_hdmi_frl_enable, hdmi_dev->hal, HI_TRUE); /* enable worken */
    } else {
        /* TMDS mode */
        frl_phy_set(hdmi_dev);
    }
    frl_n_cts_config(hdmi_dev);

    return HI_SUCCESS;
}


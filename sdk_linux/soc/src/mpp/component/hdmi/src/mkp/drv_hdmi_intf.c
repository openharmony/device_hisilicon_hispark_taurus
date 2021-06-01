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

#include "drv_hdmi_intf.h"
#ifndef HDMI_LITEOS_SUPPORT
#include <linux/kthread.h>
#endif
#include "hdmi_hal.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_intf_k.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_event.h"
#include "drv_hdmi_ioctl.h"
#include "drv_hdmi_debug.h"
#include "drv_hdmi_compatibility.h"
#include "hi_defines.h"
#ifdef HDMI_CHIP_HI3559AV100
/* 59a need vo dither */
#include "vou_dev_ext.h"
#endif
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
#include "drv_hdmi_proc.h"
#endif
#ifdef CONFIG_DRM_HISI_HISMART
#include "drm_hdmi_func_ext.h"
#endif

static osal_semaphore_t g_hdmi_mutex;

hi_char *g_hdmi_reg = NULL;
hi_char *g_hdmi_phy = NULL;
hi_s32 g_hdmi_irq_timer = -1;

#define hdmi_multiple_2p0(x)         ((x) *= 2)
#define hdmi_multiple_1p5(x)         ((x) = ((x) * 3) >> 1)
#define hdmi_multiple_1p25(x)        ((x) = ((x) * 5) >> 2)
#define hdmi_multiple_0p5(x)         ((x) = (x) >> 1)
#define MAX_DELAY_TIME_MS            10000
#define SCDC_ENABLE_TMDS_CHR_RATE    (600 * 1000000)
#define SCDC_DESABLE_TMDS_CHR_RATE   (300 * 1000000)
#define SCDC_SCRAMBLE_INTERVAL_RESET 20
#define SCDC_SCRAMBLE_TIMEOUT_RESET  200
#define DEV_HPD_ASSERT_WAIT_TIME     50
#define HDMI_THREAD_STATE_WAIT_TIME  90
#define HDMI_THREAD_DELAY            10
#define HDMI_DEV_FMT_DELAY           500
#define HDMI_DEV_MUTE_DELAY          120
#define HDMI_VIDEO_ATTR_CLK_FS       74250
#define ATTR_LOCK_WAIT_TIME          2
#define ATTR_LOCK_WAIT_TIMEOUT       5
#define HW_PARAM_ARRAY_COUNT         4
#define HDMI_READ_HPD_STATUS_DELAY   110

#define hdmi_check_open_return(state)           \
    do {                                        \
        if (!((state) & HDMI_RUN_STATE_OPEN)) { \
            hdmi_warn("device not open\n");     \
            return HI_ERR_HDMI_DEV_NOT_OPEN;    \
        }                                       \
    } while (0)

static osal_dev_t *g_hdmi_device = HI_NULL;
static osal_atomic_t g_hdmi_count = OSAL_ATOMIC_INIT(0);
static hi_s32 hdmi_file_open(void *private_data);
static hi_s32 hdmi_file_close(void *private_data);
static hi_s32 hdmi_release(hdmi_device *hdmi_dev);
static hi_s32 hdmi_video_path_set(hdmi_device *hdmi_dev, hdmi_vo_attr *vo_attr);
static hi_s32 hdmi_event_callback(hi_void *data, hdmi_event event);
static hi_bool vo_attr_is_changed(const hdmi_vo_attr *hw_vo_attr, hdmi_vo_attr *sw_vo_attr);
static hi_bool app_attr_is_changed(const hdmi_app_attr *hw_app_attr, const hdmi_app_attr *sw_app_attr);
static hi_void drv_hdmi_black_data_set(const hdmi_device *hdmi_dev, hi_bool enable);
#ifdef HDMI_HDR_SUPPORT
static hi_void hdmi_zero_drm_infoframe_timer_set(hdmi_device *hdmi_dev, hdmi_timer_config *timer);
static hi_void hdmi_zero_drm_infoframe_stop(hdmi_device *hdmi_dev);
static hi_void hdmi_hdr_mode_change_timer_set(const hdmi_device *hdmi_dev, hdmi_timer_config *timer);
static hi_void hdmi_zero_drm_infoframe_stop(hdmi_device *hdmi_dev);
static hi_s32 hdmi_hdr_mode_change_timeout(const hdmi_device *hdmi_dev);
hi_s32 hi_drv_hdmi_set_hdr_attr(hdmi_dev_id hdmi, const hdmi_hdr_attr *hdr_attr);
#endif

static hdmi_device g_hdmi_ctrl[HDMI_DEVICE_ID_BUTT];

typedef struct {
    hi_u32 cmd;
    hi_s32 (*hdmi_ioctrl_func)(hi_void *arg, hi_bool user);
} hdmi_ioctrl_func;

hi_void hdmi_set_reg(hi_char *reg)
{
    g_hdmi_reg = reg;
}

hi_void hdmi_set_phy(hi_char *phy)
{
    g_hdmi_phy = phy;
}

hi_void hdmi_set_init_irq(hi_s32 hdmi_irq_timer)
{
    g_hdmi_irq_timer = hdmi_irq_timer;
}

hi_s32 get_global_hdmi_irq_timer(hi_void)
{
    return g_hdmi_irq_timer;
}

hdmi_device *get_hdmi_device(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        g_hdmi_ctrl[hdmi_id].hdmi_dev_id = hdmi_id;
        return &g_hdmi_ctrl[hdmi_id];
    }
    return HI_NULL;
}

static hi_s32 hdmi_capability_inter_section(hdmi_sink_capability *dest_cap,
                                            const hdmi_tx_capability_data *tx_cap, hi_bool auth_mode)
{
    dest_cap->support_hdmi = dest_cap->support_hdmi && tx_cap->tx_hdmi_14;
    dest_cap->support_hdmi_20 = dest_cap->support_hdmi_20 && tx_cap->tx_hdmi_20;
    dest_cap->support_scdc = dest_cap->support_scdc && tx_cap->tx_scdc;
    dest_cap->color_space.rgb444 = dest_cap->color_space.rgb444 && tx_cap->tx_rgb444;
    dest_cap->color_space.ycbcr444 = dest_cap->color_space.ycbcr444 && tx_cap->tx_ycbcr444;
    dest_cap->color_space.ycbcr422 = dest_cap->color_space.ycbcr422 && tx_cap->tx_ycbcr422;
    dest_cap->color_space.ycbcr420 = (dest_cap->color_space.ycbcr420 && tx_cap->tx_ycbcr420) || auth_mode;
    dest_cap->deep_color.deep_color30_bit = dest_cap->deep_color.deep_color30_bit && tx_cap->tx_deep_clr10_bit;
    dest_cap->deep_color.deep_color36_bit = dest_cap->deep_color.deep_color36_bit && tx_cap->tx_deep_clr12_bit;
    dest_cap->deep_color.deep_color48_bit = dest_cap->deep_color.deep_color48_bit && tx_cap->tx_deep_clr16_bit;
    dest_cap->deep_color_y420.deep_color30_bit = (dest_cap->deep_color_y420.deep_color30_bit &&
        (tx_cap->tx_ycbcr420 && tx_cap->tx_deep_clr10_bit)) || auth_mode;
    dest_cap->deep_color_y420.deep_color36_bit = (dest_cap->deep_color_y420.deep_color36_bit &&
        (tx_cap->tx_ycbcr420 && tx_cap->tx_deep_clr12_bit)) || auth_mode;
    dest_cap->deep_color_y420.deep_color48_bit = (dest_cap->deep_color_y420.deep_color48_bit &&
        (tx_cap->tx_ycbcr420 && tx_cap->tx_deep_clr16_bit)) || auth_mode;
    dest_cap->max_tmds_clock = dest_cap->max_tmds_clock < tx_cap->tx_max_tmds_clk ? dest_cap->max_tmds_clock :
        tx_cap->tx_max_tmds_clk;
    dest_cap->hdcp_support.hdcp14_support = dest_cap->hdcp_support.hdcp14_support && tx_cap->tx_hdcp_14;
    dest_cap->hdcp_support.hdcp22_support = dest_cap->hdcp_support.hdcp22_support && tx_cap->tx_hdcp_22;

    return HI_SUCCESS;
}

static hi_void hdmi_phy_output_enable(const hdmi_device *hdmi_dev, hi_bool phy_output_enable)
{
    hal_call_void(hal_hdmi_phy_output_enable_set, hdmi_dev->hal, phy_output_enable);
    return;
}

#if !defined(HDMI_SUPPORT_LOGIC_HISIV100) && defined(HDMI_SCDC_SUPPORT)
static hi_s32 hdmi_scramble_enable_set(const hdmi_device *hdmi_dev, hi_bool enable)
{
    hdmi_scdc_config scdc_config = {0};
    hdmi_scdc_status scdc_status = {0};

    hal_call_void(hal_hdmi_scdc_status_get, hdmi_dev->hal, &scdc_status);

    if ((scdc_status.source_scramble_on && scdc_status.sink_scramble_on &&
        scdc_status.tmds_bit_clk_ratio == SCDC_TMDS_BIT_CLK_RATIO_40X && enable) ||
        (!scdc_status.source_scramble_on && !scdc_status.sink_scramble_on &&
        scdc_status.tmds_bit_clk_ratio == SCDC_TMDS_BIT_CLK_RATIO_10X && !enable)) {
        hdmi_info("scramble status is not change, so no need to config scramble\n");
        return HI_SUCCESS;
    }
    hdmi_info("source_scramble_on:%u, sink_scramble_on:%u, tmds_bit_clk_ratio:%u\n",
        scdc_status.source_scramble_on, scdc_status.sink_scramble_on, scdc_status.tmds_bit_clk_ratio);
    scdc_config.scdc_enable = enable;

    if (enable) {
        scdc_config.max_tmds_character_rate = SCDC_ENABLE_TMDS_CHR_RATE;
    } else {
        scdc_config.max_tmds_character_rate = SCDC_DESABLE_TMDS_CHR_RATE;
    }

    scdc_config.scdc_present         = HI_TRUE;
    scdc_config.rr_capable           = HI_FALSE;
    scdc_config.lte340_mcsc_scramble = HI_FALSE;
    hdmi_phy_output_enable(hdmi_dev, HI_FALSE);
    hdmi_info("scdc_enable:%u\n", scdc_config.scdc_enable);
    hdmi_info("scdc config: scdc_present(%u), rr_capable(%u), lte340_mcsc_scramble(%u), max_tmds_character_rate(%u)\n",
              scdc_config.scdc_present, scdc_config.rr_capable,
              scdc_config.lte340_mcsc_scramble, scdc_config.max_tmds_character_rate);
    hal_call_void(hal_hdmi_video_mute_set, hdmi_dev->hal, HI_TRUE);
    hal_call_void(hal_hdmi_scdc_config, hdmi_dev->hal, &scdc_config);
    hal_call_void(hal_hdmi_video_mute_set, hdmi_dev->hal, HI_FALSE);

    return HI_SUCCESS;
}
#endif

static hi_bool hdmi_ycbcr420_fmt_check(hdmi_device *hdmi_dev)
{
    hi_u32                i;
    hdmi_video_code_vic   vic;
    hi_bool               search_out = HI_FALSE;
    hdmi_vo_attr         *vo_attr    = HI_NULL;
    hdmi_sink_capability *sink_cap   = HI_NULL;

    /* auth mode donot do cb_cr420_fmt_check and return HI_TRUE */
    if (hdmi_dev->attr.app_attr.auth_mode == HI_TRUE) {
        hdmi_info("auth_mode: %u \n", hdmi_dev->attr.app_attr.auth_mode);
        return HI_TRUE;
    }
    vo_attr = &hdmi_dev->attr.vo_attr;
    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
    }
    vic = drv_hdmi_vic_search(vo_attr->video_timing, vo_attr->picture_aspect, HI_FALSE);
    if (vic != 0) {
        for (i = 0;
             i < sink_cap->support_y420_vic_num && i < hdmi_array_size(sink_cap->support_y420_format);
             i++) {
            if (vic == sink_cap->support_y420_format[i]) {
                search_out = HI_TRUE;
                break;
            }
        }
        for (i = 0;
             (!search_out) &&
             i < sink_cap->only_support_y420_vic_num &&
             i < hdmi_array_size(sink_cap->only_support_y420_format);
             i++) {
            if (vic == sink_cap->only_support_y420_format[i]) {
                search_out = HI_TRUE;
                break;
            }
        }
    }
    hdmi_info("is Y420 support vic=%u :%s\n", vic, search_out ? "YES" : "NO");

    return search_out;
}

static hi_s32 hdmi_color_space_check(hdmi_device *hdmi_dev, hdmi_app_attr *user_app)
{
    hdmi_tx_capability_data tx_cap            = {0};
    hi_bool                 support_clr_space = HI_FALSE;
    hdmi_sink_capability   *sink_cap          = HI_NULL;

    hal_call_void(hal_hdmi_tx_capability_get, hdmi_dev->hal, &tx_cap);
    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
    }

    switch (user_app->out_color_space) {
        case HDMI_COLORSPACE_RGB:
            support_clr_space = HI_TRUE;
            if (!(sink_cap->color_space.rgb444 && tx_cap.tx_rgb444)) {
                hdmi_warn("sink or source not support RGB!\n");
            }
            break;
        case HDMI_COLORSPACE_YCBCR422:
            support_clr_space = sink_cap->color_space.ycbcr422 && tx_cap.tx_ycbcr422;
            break;
        case HDMI_COLORSPACE_YCBCR444:
            support_clr_space = sink_cap->color_space.ycbcr444 && tx_cap.tx_ycbcr444;
            break;
        case HDMI_COLORSPACE_YCBCR420:
            support_clr_space = tx_cap.tx_ycbcr420 && hdmi_ycbcr420_fmt_check(hdmi_dev);
            break;
        default:
            hdmi_err("un-know color_space=%u!\n", user_app->out_color_space);
            return HI_FAILURE;
            break;
    }

    if (!support_clr_space) {
#if defined(CONFIG_HI_PLATFORM_H8)
        hdmi_err("fail, not support color space:%u\n", user_app->out_color_space);
        return HI_FAILURE;
#else
        if (user_app->enable_clr_space_adapt) {
            hdmi_warn("fail,clr(%u) adapt to RGB!\n", user_app->out_color_space);
            user_app->out_color_space = HDMI_COLORSPACE_RGB;
        } else {
            hdmi_err("fail,adapt clrspace fail!\n");
            return HI_FAILURE;
        }
#endif
    }

    return HI_SUCCESS;
}

static hi_void check_deep_color_10bit(const hdmi_sink_capability *sink_cap, const hdmi_app_attr *user_app,
    const hdmi_tx_capability_data *tx_cap, hi_u32 *fmt_pix_clk, hi_bool *support_deep_clr)
{
    hi_u32 pix_clk;

    hdmi_if_null_return_void(sink_cap);
    hdmi_if_null_return_void(user_app);
    hdmi_if_null_return_void(tx_cap);
    hdmi_if_null_return_void(fmt_pix_clk);
    hdmi_if_null_return_void(support_deep_clr);

    pix_clk = *fmt_pix_clk;

    switch (user_app->out_color_space) {
        case HDMI_COLORSPACE_RGB:
            hdmi_multiple_1p25(pix_clk);
            *support_deep_clr = (sink_cap->deep_color.deep_color30_bit &&
                tx_cap->tx_deep_clr10_bit) ? HI_TRUE : HI_FALSE;
            break;
        case HDMI_COLORSPACE_YCBCR444:
            hdmi_multiple_1p25(pix_clk);
            *support_deep_clr = (sink_cap->deep_color.deep_color30_bit && tx_cap->tx_deep_clr10_bit &&
                sink_cap->deep_color.deep_color_y444) ? HI_TRUE : HI_FALSE;
            break;
        case HDMI_COLORSPACE_YCBCR420:
            hdmi_multiple_1p25(pix_clk);
            *support_deep_clr = (sink_cap->deep_color_y420.deep_color30_bit &&
                tx_cap->tx_deep_clr10_bit) ? HI_TRUE : HI_FALSE;
            break;
        default:
            /* Y422, ignore deepclr */
            *support_deep_clr = HI_TRUE;
            break;
    }
    *fmt_pix_clk = pix_clk;

    return;
}

static hi_void check_deep_color_12bit(const hdmi_sink_capability *sink_cap, const hdmi_app_attr *user_app,
    const hdmi_tx_capability_data *tx_cap, hi_u32 *fmt_pix_clk, hi_bool *support_deep_clr)
{
    hi_u32 pix_clk;

    hdmi_if_null_return_void(sink_cap);
    hdmi_if_null_return_void(user_app);
    hdmi_if_null_return_void(tx_cap);
    hdmi_if_null_return_void(fmt_pix_clk);
    hdmi_if_null_return_void(support_deep_clr);

    pix_clk = *fmt_pix_clk;

    switch (user_app->out_color_space) {
        case HDMI_COLORSPACE_RGB:
            hdmi_multiple_1p5(pix_clk);
            *support_deep_clr = (sink_cap->deep_color.deep_color36_bit &&
                tx_cap->tx_deep_clr12_bit) ? HI_TRUE : HI_FALSE;
            break;
        case HDMI_COLORSPACE_YCBCR444:
            hdmi_multiple_1p5(pix_clk);
            *support_deep_clr = (sink_cap->deep_color.deep_color36_bit && tx_cap->tx_deep_clr12_bit &&
                sink_cap->deep_color.deep_color_y444) ? HI_TRUE : HI_FALSE;
            break;
        case HDMI_COLORSPACE_YCBCR420:
            hdmi_multiple_1p5(pix_clk);
            *support_deep_clr = (sink_cap->deep_color_y420.deep_color36_bit &&
                tx_cap->tx_deep_clr12_bit) ? HI_TRUE : HI_FALSE;
            break;
        default:
            /* Y422, ignore deepclr */
            *support_deep_clr = HI_TRUE;
            break;
    }
    *fmt_pix_clk = pix_clk;

    return;
}

static hi_u32 hdmi_current_id_get(const hdmi_device *hdmi_dev)
{
    hi_u32 id;
#ifdef HDMI_LITEOS_SUPPORT
    hi_u32 total = 0;
#endif
    id = hdmi_get_current_id();

#ifdef HDMI_LITEOS_SUPPORT
    /*
     * lite_os only support thread, each thread id is different(such as sample and HDMI poll event thread).
     * so return pool[0]'s ID when get current ID.
     */
    drv_hdmi_event_pool_total_get(hdmi_dev->hdmi_dev_id, &total);
    if (total > 0) {
        drv_hdmi_event_pool_id_get(hdmi_dev->hdmi_dev_id, &id);
    }
#else
    hi_unused(hdmi_dev);
#endif

    return id;
}

static hi_void check_deep_color(const hdmi_sink_capability *sink_cap, hdmi_app_attr *user_app,
    const hdmi_tx_capability_data *tx_cap, hi_u32 *fmt_pix_clk, hi_bool *support_deep_clr)
{
    switch (user_app->deep_color_mode) {
        case HDMI_DEEP_COLOR_30BIT:
            check_deep_color_10bit(sink_cap, user_app, tx_cap, fmt_pix_clk, support_deep_clr);
            break;
        case HDMI_DEEP_COLOR_36BIT:
            check_deep_color_12bit(sink_cap, user_app, tx_cap, fmt_pix_clk, support_deep_clr);
            break;
        default:
#ifndef HDMI_SUPPORT_LOGIC_HISIV100
            user_app->deep_color_mode = HDMI_DEEP_COLOR_24BIT;
#endif
            *support_deep_clr = HI_TRUE;
            break;
    }

    return;
}

static hi_s32 hdmi_deep_color_check(hdmi_device *hdmi_dev, hdmi_app_attr *user_app, hi_u32 max_tmds_clk)
{
    hi_u32                  fmt_pix_clk;
    hdmi_vo_attr           *vo_attr          = HI_NULL;
    hdmi_sink_capability   *sink_cap         = HI_NULL;
    hdmi_tx_capability_data tx_cap           = {0};
    hi_bool                 adapt_to_y422    = HI_FALSE;
    hi_bool                 support_deep_clr = HI_FALSE;

    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);

    vo_attr = &hdmi_dev->attr.vo_attr;
    hdmi_dev->hal->hal_hdmi_tx_capability_get(hdmi_dev->hal, &tx_cap);
    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
    }

    fmt_pix_clk = vo_attr->clk_fs;
    if (user_app->out_color_space == HDMI_COLORSPACE_YCBCR420) {
        hdmi_multiple_0p5(fmt_pix_clk);
    }
    check_deep_color(sink_cap, user_app, &tx_cap, &fmt_pix_clk, &support_deep_clr);

    if (support_deep_clr && (fmt_pix_clk < max_tmds_clk)) {
        vo_attr->hdmi_adapt_pix_clk = fmt_pix_clk;
        return HI_SUCCESS;
    } else {
        vo_attr->hdmi_adapt_pix_clk = vo_attr->clk_fs;
        if (user_app->out_color_space == HDMI_COLORSPACE_YCBCR420) {
            hdmi_multiple_0p5(vo_attr->hdmi_adapt_pix_clk);
        }

        adapt_to_y422 = ((user_app->out_color_space == HDMI_COLORSPACE_YCBCR444) &&
                         (user_app->enable_clr_space_adapt) &&
                         (user_app->deep_color_mode != HDMI_DEEP_COLOR_24BIT) &&
                         (user_app->deep_color_mode != HDMI_DEEP_COLOR_OFF) &&
                         (user_app->deep_color_mode != HDMI_DEEP_COLOR_48BIT) &&
                         (sink_cap->color_space.ycbcr422 && tx_cap.tx_ycbcr422));

        /* Y444 12/10bit */
        if (adapt_to_y422) {
            hdmi_warn("fail, Y444 adapt to Y422!\n");
            user_app->out_color_space = HDMI_COLORSPACE_YCBCR422;
        } else if (user_app->enable_deep_clr_adapt) { /* Y444 16bit/ RGB / Y420 */
            hdmi_warn("fail, deepclr(%u) adapt to 8bit!\n", user_app->deep_color_mode);
            user_app->deep_color_mode = HDMI_DEEP_COLOR_24BIT;
        } else {
            hdmi_err("fail, adapt deepclr fail!\n");
            return HI_FAILURE;
        }

        return HI_SUCCESS;
    }
}

static hi_s32 adapt_to_420(const hdmi_vo_attr *vo_attr, hdmi_app_attr *user_app, hi_u32 max_tmds_clk)
{
    if (vo_attr->clk_fs <= max_tmds_clk) {
        return HI_SUCCESS;
    }

    hdmi_warn("pix_clk(%u) > max_tmds_clk(%u)\n", vo_attr->clk_fs, max_tmds_clk);
    // add user_app->out_color_space to avoid set yuv420 failed after the timing set success
#if defined(CONFIG_HI_PLATFORM_H8)
    /* in h8, hdmi can adapt to yuv420 just in yuv444 */
    if (user_app->out_color_space == HDMI_COLORSPACE_YCBCR444) {
#else
    if (user_app->out_color_space != HDMI_COLORSPACE_YCBCR420) {
#endif
        if (user_app->enable_clr_space_adapt) {
            hdmi_warn("clr(%u) adapt to Y420 or user clr is Y420!\n", user_app->out_color_space);
            user_app->out_color_space = HDMI_COLORSPACE_YCBCR420;
        } else {
            hdmi_err("adapt Y420 fail!\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

#ifdef HDMI_FRL_SUPPORT
static hi_void tmds_clk_calc(hdmi_device *hdmi_dev, const hdmi_app_attr *user_app)
{
    hdmi_vo_attr *vo_attr = HI_NULL;

    vo_attr = &hdmi_dev->attr.vo_attr;
    vo_attr->tmds_clk =
        (user_app->out_color_space == HDMI_COLORSPACE_YCBCR420) ? (vo_attr->clk_fs >> 1) : vo_attr->clk_fs;

    if (user_app->out_color_space != HDMI_COLORSPACE_YCBCR422) {
        switch (user_app->deep_color_mode) {
            case HDMI_DEEP_COLOR_48BIT:
                hdmi_multiple_2p0(vo_attr->tmds_clk);
                break;
            case HDMI_DEEP_COLOR_36BIT:
                hdmi_multiple_1p5(vo_attr->tmds_clk);
                break;
            case HDMI_DEEP_COLOR_30BIT:
                hdmi_multiple_1p25(vo_attr->tmds_clk);
                break;
            default:
                break;
        }
    }

    return;
}
#endif

static hi_void edid_invalid_or_auth_mode(hdmi_device *hdmi_dev, hdmi_app_attr *user_app)
{
    hdmi_vo_attr *vo_attr = HI_NULL;

    vo_attr = &hdmi_dev->attr.vo_attr;
    vo_attr->hdmi_adapt_pix_clk =
        (user_app->out_color_space == HDMI_COLORSPACE_YCBCR420) ? (vo_attr->clk_fs >> 1) : vo_attr->clk_fs;

    if (user_app->out_color_space != HDMI_COLORSPACE_YCBCR422) {
        switch (user_app->deep_color_mode) {
            case HDMI_DEEP_COLOR_48BIT:
                hdmi_multiple_2p0(vo_attr->hdmi_adapt_pix_clk);
                break;
            case HDMI_DEEP_COLOR_36BIT:
                hdmi_multiple_1p5(vo_attr->hdmi_adapt_pix_clk);
                break;
            case HDMI_DEEP_COLOR_30BIT:
                hdmi_multiple_1p25(vo_attr->hdmi_adapt_pix_clk);
                break;
            default:
                break;
        }
    } else {
        if ((user_app->deep_color_mode != HDMI_DEEP_COLOR_OFF) &&
            (user_app->deep_color_mode != HDMI_DEEP_COLOR_24BIT)) {
            hdmi_info("Y422 foce deepcolor 8bit");
            user_app->deep_color_mode = HDMI_DEEP_COLOR_24BIT;
        }
        if (hdmi_dev->attr.vo_attr.video_timing == HDMI_VIDEO_TIMING_1440X480I_60000 ||
            hdmi_dev->attr.vo_attr.video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) {
            hdmi_warn("Y422 is not support at pal and ntsc, force adapt to rgb!\n");
            user_app->out_color_space = HDMI_COLORSPACE_RGB;
        }
    }

    return;
}

static hi_s32 dvi_color_and_bit_strategy(hdmi_device *hdmi_dev, hdmi_app_attr *user_app)
{
    hdmi_vo_attr *vo_attr = HI_NULL;

    vo_attr = &hdmi_dev->attr.vo_attr;
#if (defined(CONFIG_HI_PLATFORM_H8))
    if (user_app->out_color_space != HDMI_COLORSPACE_RGB) {
        hdmi_err("DVI mode, but the color space is not RGB!");
        return HI_FAILURE;
    }
#else
    hdmi_info("DVI mode, force deep_color %u -> OFF, color_space %u -> RGB!\n",
              user_app->deep_color_mode, user_app->out_color_space);
    user_app->out_color_space = HDMI_COLORSPACE_RGB;
#endif
    user_app->deep_color_mode = HDMI_DEEP_COLOR_OFF;
    vo_attr->hdmi_adapt_pix_clk = vo_attr->clk_fs;

    return HI_SUCCESS;
}

static hi_s32 hdmi_deep_color_adapt(const hdmi_device *hdmi_dev, hdmi_app_attr *user_app)
{
    /* Y422 default 12bit output, deep_color force adapt to 8bit(24bit). */
    if (user_app->out_color_space == HDMI_COLORSPACE_YCBCR422) {
        if (hdmi_dev->attr.vo_attr.video_timing == HDMI_VIDEO_TIMING_1440X480I_60000 ||
            hdmi_dev->attr.vo_attr.video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) {
#if defined(CONFIG_HI_PLATFORM_H8)
            hdmi_err("Y422 is not support at pal and ntsc!\n");
            return HI_FAILURE;
#else
            hdmi_warn("Y422 is not support at pal and ntsc!\n");
            if (user_app->enable_clr_space_adapt != HI_TRUE) {
                return HI_FAILURE;
            } else {
                hdmi_warn("force change colorspace to RGB!\n");
                user_app->out_color_space = HDMI_COLORSPACE_RGB;
            }
#endif
        }
        if ((user_app->deep_color_mode != HDMI_DEEP_COLOR_24BIT) &&
            (user_app->deep_color_mode != HDMI_DEEP_COLOR_OFF)) {
            user_app->deep_color_mode = HDMI_DEEP_COLOR_OFF;
            hdmi_warn("when Y422, deep_color not support 10/12bit!\n");
        }
    }

    return HI_SUCCESS;
}

static hi_s32 hdmi_color_and_bit_strategy(hdmi_device *hdmi_dev, hdmi_app_attr *user_app)
{
    hi_s32                  ret;
    hdmi_edid_data          edid_ret;
    hi_u32                  max_tmds_clk = 0;
    hdmi_vo_attr           *vo_attr      = HI_NULL;
    hdmi_sink_capability   *sink_cap     = HI_NULL;
    hdmi_tx_capability_data tx_cap       = {0};

    vo_attr = &hdmi_dev->attr.vo_attr;
    /* DVI mode, must set RGB & deep_color off */
    if (!user_app->enable_hdmi) {
        return dvi_color_and_bit_strategy(hdmi_dev, user_app);
    }
#ifdef HDMI_FRL_SUPPORT
    tmds_clk_calc(hdmi_dev, user_app);
    if (vo_attr->tmds_clk > HDMI_EDID_MAX_HDMI20_TMDS_RATE) {
        vo_attr->hdmi_adapt_pix_clk = vo_attr->tmds_clk;
        hdmi_warn("HDMI2.1 no hdmi adapt hdmi_adapt_pix_clk=%u\n", vo_attr->hdmi_adapt_pix_clk);
        return HI_SUCCESS;
    }
#endif
    hal_call_void(hal_hdmi_tx_capability_get, hdmi_dev->hal, &tx_cap);
    edid_ret = drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap);
    if (edid_ret == HDMI_EDID_DATA_INVALID || (user_app->auth_mode == HI_TRUE)) {
        edid_invalid_or_auth_mode(hdmi_dev, user_app);
        /* get edid_capability fail means cannot do strategy, according to appatrr */
        hdmi_warn("get sink capability fail or auth_mode=%u, adapt hdmi_adapt_pix_clk=%u\n",
                  hdmi_dev->attr.app_attr.auth_mode, vo_attr->hdmi_adapt_pix_clk);
        return HI_SUCCESS;
    } else {
        /* get max_tmds_clk, in k_hz. */
        if (sink_cap->max_tmds_clock == 0) {
            /*
             * but when the sink's max_tmds_clock is 0_m_hz, app can not set attr success.
             * then set max_tmds_clk = 300M default.
             */
            max_tmds_clk = 300;
        } else {
            max_tmds_clk =
                sink_cap->max_tmds_clock < tx_cap.tx_max_tmds_clk ? sink_cap->max_tmds_clock : tx_cap.tx_max_tmds_clk;
        }
        max_tmds_clk *= 1000; /* k_hz*1000, M_hz */
    }
    /* whether adapt to ycbcr420 */
    ret = adapt_to_420(vo_attr, user_app, max_tmds_clk);
    hdmi_if_failure_return(ret, HI_FAILURE);
    /* color_space check & adapt */
    ret = hdmi_color_space_check(hdmi_dev, user_app);
    hdmi_if_failure_return(ret, HI_FAILURE);
    /* deep_color check & adapt */
    ret = hdmi_deep_color_check(hdmi_dev, user_app, max_tmds_clk);
    hdmi_if_failure_return(ret, HI_FAILURE);
    /* deep color adapt */
    ret = hdmi_deep_color_adapt(hdmi_dev, user_app);
    hdmi_if_failure_return(ret, HI_FAILURE);
    hdmi_info("clr_space_adapt=%u, deep_clr_adapt=%u, adptclrspace=%u, adptbit=%u, adptclk=%u, max_clk=%u\n",
              user_app->enable_clr_space_adapt, user_app->enable_deep_clr_adapt, user_app->out_color_space,
              user_app->deep_color_mode, vo_attr->hdmi_adapt_pix_clk, max_tmds_clk);

    return HI_SUCCESS;
}

#ifdef HDMI_SCDC_SUPPORT
static hi_s32 hdmi_scdc_status_get(const hdmi_app_attr *app_attr, const hdmi_vo_attr *vo_attr,
                                   hdmi_scdc_status *scdc_status, hdmi_tmds_mode *tmds_mode)
{
    if (!app_attr->enable_hdmi) {
        /* DVI mode */
        *tmds_mode = HDMI_TMDS_MODE_DVI;
        scdc_status->sink_scramble_on   = HI_FALSE;
        scdc_status->source_scramble_on = HI_FALSE;
        scdc_status->tmds_bit_clk_ratio = SCDC_TMDS_BIT_CLK_RATIO_10X;
        if ((vo_attr->hdmi_adapt_pix_clk > HDMI_EDID_MAX_HDMI14_TMDS_RATE) && (!app_attr->enable_clr_space_adapt)) {
            hdmi_err("tmds_clk=%u can't support in DVI mode. \n", vo_attr->hdmi_adapt_pix_clk);
            return HI_FAILURE;
        }
    } else if (vo_attr->hdmi_adapt_pix_clk > HDMI_EDID_MAX_HDMI14_TMDS_RATE) {
        *tmds_mode = HDMI_TMDS_MODE_HDMI_2_0;
        scdc_status->sink_scramble_on   = HI_TRUE;
        scdc_status->source_scramble_on = HI_TRUE;
        scdc_status->tmds_bit_clk_ratio = SCDC_TMDS_BIT_CLK_RATIO_40X;
    } else {
        *tmds_mode = HDMI_TMDS_MODE_HDMI_1_4;
        scdc_status->sink_scramble_on   = HI_FALSE;
        scdc_status->source_scramble_on = HI_FALSE;
        scdc_status->tmds_bit_clk_ratio = SCDC_TMDS_BIT_CLK_RATIO_10X;
    }

    return HI_SUCCESS;
}
#endif

#if defined HDMI_SUPPORT_LOGIC_HISIV100
hi_s32 hdmi_mode_strategy(hdmi_device *hdmi_dev)
{
    hdmi_tmds_mode tmds_mode;
    hdmi_app_attr *app_attr = HI_NULL;
#ifdef HDMI_SCDC_SUPPORT
    hdmi_edid_data        edid_ret;
    hdmi_scdc_status      scdc_get    = {0};
    hdmi_scdc_status      scdc_status = {0};
    hdmi_vo_attr         *vo_attr     = HI_NULL;
    hdmi_sink_capability *sink_cap    = HI_NULL;
#endif

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    app_attr = &hdmi_dev->attr.app_attr;
    hdmi_if_null_return(app_attr, HI_ERR_NULL_PTR);

#ifdef HDMI_SCDC_SUPPORT
    vo_attr = &hdmi_dev->attr.vo_attr;
    edid_ret = drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap);
    if (edid_ret == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("invalid edid_capability!\n");
    }
    if (hdmi_scdc_status_get(app_attr, vo_attr, &scdc_status, &tmds_mode) != HI_SUCCESS) {
        return HI_FAILURE;
    }
#else
    tmds_mode = (app_attr->enable_hdmi == HI_TRUE) ? HDMI_TMDS_MODE_HDMI_1_4 : HDMI_TMDS_MODE_DVI;
#endif

    hdmi_info("tmds mode %u->%u.\n", hdmi_dev->tmds_mode, tmds_mode);
    hdmi_dev->tmds_mode = tmds_mode;
    hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, hdmi_dev->tmds_mode);
    /* reset controller when the controller and phy configuration is completed. */
    hal_call_void(hal_hdmi_ctrl_reset, hdmi_dev->hal);

#ifdef HDMI_SCDC_SUPPORT
    if (sink_cap->support_scdc == HI_TRUE || hdmi_dev->attr.app_attr.auth_mode == HI_TRUE) {
        hal_call_void(hal_hdmi_scdc_status_get, hdmi_dev->hal, &scdc_get);
        if ((scdc_get.sink_scramble_on != scdc_status.sink_scramble_on ||
            scdc_get.source_scramble_on != scdc_status.source_scramble_on ||
            scdc_get.tmds_bit_clk_ratio != scdc_status.tmds_bit_clk_ratio) ||
            (hdmi_dev->attr.app_attr.auth_mode == HI_TRUE)) {
            scdc_status.scramble_interval = SCDC_SCRAMBLE_INTERVAL_RESET;
            scdc_status.scramble_timeout = SCDC_SCRAMBLE_TIMEOUT_RESET;
            scdc_status.sink_read_quest = HI_FALSE;
            hal_call_void(hal_hdmi_scdc_status_set, hdmi_dev->hal, &scdc_status);
        }
    } else {
        hdmi_info("sink not support SCDC\n");
    }
#endif

    return HI_SUCCESS;
}

#else
hi_s32 hdmi_mode_strategy(hdmi_device *hdmi_dev)
{
    hdmi_vo_attr  *vo_attr  = HI_NULL;
    hdmi_app_attr *app_attr = HI_NULL;
    hdmi_tmds_mode tmds_mode;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);

    vo_attr  = &hdmi_dev->attr.vo_attr;
    app_attr = &hdmi_dev->attr.app_attr;

    hdmi_if_null_return(app_attr, HI_ERR_NULL_PTR);

    if (!app_attr->enable_hdmi) {  // DVI
        tmds_mode = HDMI_TMDS_MODE_DVI;
        if (vo_attr->hdmi_adapt_pix_clk > HDMI_EDID_MAX_HDMI14_TMDS_RATE) {
            if (app_attr->enable_clr_space_adapt) {
                app_attr->enable_hdmi = HI_TRUE;
                tmds_mode = HDMI_TMDS_MODE_HDMI_2_0;
                hdmi_warn("tmds_clk=%u can't support in DVI, adapt to hdmi2.0\n", vo_attr->hdmi_adapt_pix_clk);
            } else {
                hdmi_err("tmds_clk=%u can't support in DVI mode", vo_attr->hdmi_adapt_pix_clk);
                return HI_FAILURE;
            }
        }
    } else {  // HDMI
        tmds_mode = vo_attr->hdmi_adapt_pix_clk < HDMI_EDID_MAX_HDMI14_TMDS_RATE ?
            HDMI_TMDS_MODE_HDMI_1_4 : HDMI_TMDS_MODE_HDMI_2_0;
    }

    hdmi_info("tmds mode %u->%u.\n", hdmi_dev->tmds_mode, tmds_mode);
    hdmi_dev->tmds_mode = tmds_mode;
    hal_call_void(hal_hdmi_tmds_mode_set, hdmi_dev->hal, hdmi_dev->tmds_mode);

#ifdef HDMI_SCDC_SUPPORT
    if (hdmi_dev->edid_info.capability.support_scdc == HI_TRUE || hdmi_dev->attr.app_attr.auth_mode == HI_TRUE) {
        if (hdmi_dev->tmds_mode == HDMI_TMDS_MODE_HDMI_2_0) {
            hdmi_scramble_enable_set(hdmi_dev, HI_TRUE);
        } else {
            hdmi_scramble_enable_set(hdmi_dev, HI_FALSE);
        }
    } else {
        hdmi_info("can't support SCDC \n");
    }
#endif

    return HI_SUCCESS;
}
#endif

hi_s32 drv_hdmi_init(hi_void)
{
    return HI_SUCCESS;
}

hi_s32 drv_hdmi_deinit(hi_void)
{
    return HI_SUCCESS;
}

static hdmi_deep_color depth_convert_to_deep_color(hdmi_video_bit_depth bit_depth)
{
    hdmi_deep_color deep_color;

    switch (bit_depth) {
        case HDMI_VIDEO_BITDEPTH_8:
            deep_color = HDMI_DEEP_COLOR_24BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_10:
            deep_color = HDMI_DEEP_COLOR_30BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_12:
            deep_color = HDMI_DEEP_COLOR_36BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_16:
            deep_color = HDMI_DEEP_COLOR_48BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_OFF:
            deep_color = HDMI_DEEP_COLOR_OFF;
            break;
        default:
            deep_color = HDMI_DEEP_COLOR_BUTT;
            break;
    }

    return deep_color;
}

static hi_void hdmi_user_attr_construct(hdmi_device *hdmi_dev, hdmi_attr *attr, const hdmi_hardware_status *hw_status)
{
    hdmi_app_attr       *app_attr   = HI_NULL;
#ifdef HDMI_HDR_SUPPORT
    hi_bool is_dolby = HI_FALSE;
#endif

    app_attr   = &attr->app_attr;
    hdmi_dev->tmds_mode = hw_status->common_status.tmds_mode;
    switch (hdmi_dev->tmds_mode) {
        case HDMI_TMDS_MODE_HDMI_1_4:
        case HDMI_TMDS_MODE_HDMI_2_0:
            app_attr->enable_hdmi = HI_TRUE;
            app_attr->enable_video = HI_TRUE;
            break;
        case HDMI_TMDS_MODE_DVI:
            app_attr->enable_hdmi = HI_FALSE;
            app_attr->enable_video = HI_TRUE;
            break;
        default:
            app_attr->enable_hdmi = HI_FALSE;
            app_attr->enable_video = HI_FALSE;
            break;
    }

    app_attr->enable_audio = (hw_status->audio_status.audio_enable && hw_status->info_frame_status.audio_enable);
#ifdef HDMI_HDR_SUPPORT
    /* out colorspace is yuv444 when the hdr mode is dolby. */
    is_dolby = ((hdmi_dev->attr.hdr_attr.user_hdr_mode == HDMI_HDR_USERMODE_DOLBY) &&
                ((!hw_status->video_status.rgb2ycbcr) && (!hw_status->video_status.ycbcr2rgb) &&
                (!hw_status->video_status.ycbcr444_422) && (!hw_status->video_status.ycbcr422_420) &&
                (!hw_status->video_status.ycbcr422_444) && (!hw_status->video_status.ycbcr420_422)));
    app_attr->out_color_space =
        (is_dolby == HI_TRUE) ? HDMI_COLORSPACE_YCBCR444 : hw_status->video_status.out_color_space;
#else
    app_attr->out_color_space = hw_status->video_status.out_color_space;
#endif
    app_attr->deep_color_mode      = depth_convert_to_deep_color(hw_status->video_status.out_bit_depth);
    app_attr->enable_avi_infoframe = hw_status->info_frame_status.avi_enable;
    app_attr->enable_aud_infoframe = hw_status->info_frame_status.audio_enable;
    app_attr->xvycc_mode = hw_status->info_frame_status.gbd_enable;

    return;
}

static hi_void hdmi_video_attr_construct(hdmi_attr *attr, const hdmi_hardware_status *hw_status)
{
    hdmi_app_attr       *app_attr   = HI_NULL;
    hdmi_vo_attr        *video_attr = HI_NULL;
    hi_bool in_clr_is_rgb = HI_FALSE;
    hi_bool aspect_is256  = HI_FALSE;

    app_attr   = &attr->app_attr;
    video_attr = &attr->vo_attr;
    video_attr->in_color_space = app_attr->out_color_space;
    video_attr->v_sync_pol     = hw_status->video_status.v_sync_pol;
    video_attr->h_sync_pol     = hw_status->video_status.h_sync_pol;
    video_attr->de_pol         = hw_status->video_status.de_pol;

    in_clr_is_rgb = ((hw_status->video_status.rgb2ycbcr) || ((!hw_status->video_status.ycbcr2rgb) &&
                     (hw_status->video_status.out_color_space == HDMI_COLORSPACE_RGB)));
    video_attr->in_color_space = (in_clr_is_rgb == HI_TRUE) ? HDMI_COLORSPACE_RGB : HDMI_COLORSPACE_YCBCR444;
    if (hw_status->info_frame_status.avi_enable) {
        hdmi_video_code_vic video_code = hw_status->info_frame_status.avi[7];
        /*
         * when the timing is 4096*2160, the aspect ratio in AVI infoframe is 0
         * (but the real aspect ratio is 256:135<0x04>, the video_code is 0)
         */
        aspect_is256 = (((video_code == 0) && (hw_status->info_frame_status.vsif[8] == 0x04)) ||
                        ((video_code >= HDMI_4096X2160P25_256_135) && (video_code <= HDMI_4096X2160P60_256_135)));
        video_attr->picture_aspect = (aspect_is256 == HI_TRUE) ?
            HDMI_PICTURE_ASPECT_256_135 : ((hw_status->info_frame_status.avi[5] >> 4) & 0x3); /* 4'b, BIT[2:1] */
        video_attr->active_aspect        = hw_status->info_frame_status.avi[5] & 0xf;
        video_attr->colorimetry          = (hw_status->info_frame_status.avi[5] >> 6) & 0x3; /* 6'b, BIT[2:1] */
        video_attr->rgb_quantization     = (hw_status->info_frame_status.avi[6] >> 2) & 0x3; /* 2'b, BIT[2:1] */
        video_attr->ycc_quantization     = (hw_status->info_frame_status.avi[8] >> 6) & 0x3; /* 6'b, BIT[2:1] */
        video_attr->pixel_repeat         = (hw_status->info_frame_status.avi[8] & 0xf) + 1;
        video_attr->extended_colorimetry = (hw_status->info_frame_status.avi[6] >> 4) & 0x07; /* 4'b, BIT[3:1] */
        video_attr->video_timing         = drv_hdmi_video_timing_lookup(video_code, video_attr->picture_aspect);
        hdmi_info("video_timing: %u, video_code: %u, picture_aspect: %u\n",
            video_attr->video_timing, video_code, video_attr->picture_aspect);
        if ((!hw_status->info_frame_status.vsif_enable) && (!video_code)) {
            video_attr->video_timing = HDMI_VIDEO_TIMING_UNKNOWN;
        }
        app_attr->out_csc_quantization = (app_attr->out_color_space == HDMI_COLORSPACE_RGB) ?
            video_attr->rgb_quantization : (video_attr->ycc_quantization + 1);
        hdmi_info("get hw out_csc_quantization: %u\n", app_attr->out_csc_quantization);
    }
    video_attr->stereo_mode = HDMI_3D_BUTT;
    if (hw_status->info_frame_status.vsif_enable) {
        enum hdmi_video_format format = (hw_status->info_frame_status.vsif[7] >> 5) & 0x3; /* 5'b, BIT[2:1] */
        if (format == HDMI_VIDEO_FORMAT_4K) {
            hdmi_vsif_vic video_code = hw_status->info_frame_status.vsif[8];
            video_attr->video_timing = drv_hdmi_vsif_video_timing_lookup(video_code);
        } else if (format == HDMI_VIDEO_FORMAT_3D) {
            video_attr->stereo_mode = (hw_status->info_frame_status.vsif[8] >> 4) & 0xf; /* 4'b, BIT[4:1] */
        }
    }

    return;
}

static hi_s32 hdmi_attr_construct(hdmi_device *hdmi_dev, hdmi_attr *attr)
{
    hdmi_hardware_status hw_status  = {0};
    hdmi_ao_attr        *audio_attr = HI_NULL;

    audio_attr = &attr->ao_attr;
    hal_call_void(hal_hdmi_hardware_status_get, hdmi_dev->hal, &hw_status);

    /* user attribute construct */
    hdmi_user_attr_construct(hdmi_dev, attr, &hw_status);
    /* video attribute construct */
    hdmi_video_attr_construct(attr, &hw_status);
    /* audio attrbute construct */
    audio_attr->down_sample  = hw_status.audio_status.down_sample;
    audio_attr->channels     = hw_status.audio_status.layout;
    audio_attr->sample_depth = hw_status.audio_status.sample_depth;
    audio_attr->sample_fs    = hw_status.audio_status.sample_fs;
    audio_attr->sound_intf   = hw_status.audio_status.sound_intf;
    if (hw_status.info_frame_status.audio_enable) {
        audio_attr->audio_code = (hw_status.info_frame_status.audio[4] >> 4) & 0xf; /* 4'b, BIT[4:1] */
    }

    return HI_SUCCESS;
}

static hi_s32 hpd_event(hdmi_device *hdmi_dev)
{
    hi_s32 ret         = HI_SUCCESS;
    hi_u32 timeout_cnt = 5;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);

    /* update edid from sink */
    if (hdmi_dev->hpd_detected == HI_FALSE) {
        drv_hdmi_edid_reset(&hdmi_dev->edid_info);
        do {
            /*
             * 1.for some TV ,EDID need to read more times;
             * 2.for some repeater, EDID is readable after HPD assert 250ms.
             */
            ret = drv_hdmi_edid_update(&hdmi_dev->edid_info, HDMI_EDID_UPDATE_SINK);
            if (ret == HI_SUCCESS) {
                hdmi_dev->hpd_detected = HI_TRUE;
                break;
            }
            osal_msleep(DEV_HPD_ASSERT_WAIT_TIME);
        } while (timeout_cnt--);

        if (ret != HI_SUCCESS) {
            hdmi_warn("update EDID fail,timeout_cnt=%u\n", timeout_cnt);
        }
    }
    compatibility_info_update(hdmi_dev->hdmi_dev_id);

    return ret;
}

static hi_void event_notify(hdmi_device *hdmi_dev, hdmi_event event)
{
    if (event <= HDMI_EVENT_HDCP_USERSETTING) {
        hdmi_mutex_unlock(g_hdmi_mutex);
        hdmi_mutex_lock(hdmi_dev->mutex_thread);
        if (hdmi_dev->k_callback == HI_NULL) { /* notify to user */
            hdmi_info("notify event(0x%x) to user\n", event);
            drv_hdmi_event_pool_write(hdmi_dev->hdmi_dev_id, event);
        } else { /* notify to kernel */
            hdmi_info("notify event(0x%x) to kernel\n", event);
            if (hdmi_dev->k_callback != HI_NULL) {
                hdmi_dev->k_callback((hi_void *)&(hdmi_dev->hdmi_dev_id), event);
            }
            if (event == HDMI_EVENT_HOTPLUG) {
                hdmi_dev->hpd_notified = HI_TRUE;
            }
        }
        hdmi_mutex_unlock(hdmi_dev->mutex_thread);
        hdmi_mutex_lock(g_hdmi_mutex);
    }

    return;
}

static hi_s32 hdmi_event_callback(hi_void *data, hdmi_event event)
{
    hi_s32 ret = HI_SUCCESS;
    hdmi_device *hdmi_dev = (hdmi_device *)data;

    switch (event) {
        case HDMI_EVENT_HOTPLUG:
            hpd_event(hdmi_dev);
            break;
        case HDMI_EVENT_HOTUNPLUG:
            hdmi_dev->hpd_detected = HI_FALSE;
            break;
#ifdef HDMI_HDR_SUPPORT
        case HDMI_EVENT_ZERO_DRMIF_TIMEOUT:
            hdmi_zero_drm_infoframe_stop(hdmi_dev);
            break;
        case HDMI_EVENT_SWITCH_TO_HDRMODE_TIMEOUT:
            hdmi_hdr_mode_change_timeout(hdmi_dev);
            break;
#endif
        default:
            break;
    }
    event_notify(hdmi_dev, event);

    return ret;
}

static hi_s32 hdmi_kthread_timer(void *data)
{
    hdmi_device *hdmi_dev = (hdmi_device *)data;
#ifdef HDMI_FRL_SUPPORT
    hdmi_frl_info *frl_info = HI_NULL;
#endif
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
#ifdef HDMI_FRL_SUPPORT
    frl_info = &hdmi_dev->frl_info;
#endif

    while (1) {
#ifdef HDMI_LITEOS_SUPPORT
        if (hdmi_dev->thread_info.thread_run == HI_FALSE) {
            hdmi_dev->thread_info.thread_exit = HI_TRUE;
            osal_wakeup(&hdmi_dev->thread_info.wait_destroy);
            break;
        }
#else
        if (kthread_should_stop()) {
            break;
        }
#endif
        if (hdmi_dev->thread_info.thread_timer_sate == HDMI_THREAD_STATE_STOP) {
            osal_msleep(HDMI_THREAD_STATE_WAIT_TIME);
            continue;
        }

#ifdef HDMI_FRL_SUPPORT
        if (frl_info->state_mach_info.start_mach) {
            drv_hdmi_frl_train_mach(hdmi_dev->hdmi_dev_id);
        }
#endif
        if (hdmi_dev->hal != HI_NULL) {
            if (hdmi_dev->hal->hal_hdmi_sequencer_handler_process != HI_NULL &&
                (((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_OPEN) ||
                ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_START) ||
                ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_STOP))) {
                hdmi_dev->hal->hal_hdmi_sequencer_handler_process((struct hdmi_hal_ *)hdmi_dev);
            }
        } else {
            hdmi_info("input param is NULL\n");
        }

#if defined(HDMI_SUPPORT_LOGIC_HISIV100)
        osal_msleep(HDMI_THREAD_DELAY);
#endif
    }

    return HI_SUCCESS;
}

static hi_void hdmi_vo_attr_init(hdmi_vo_attr *video_attr)
{
    hi_bool colorimetry_is601;

    colorimetry_is601 = ((video_attr->video_timing == HDMI_VIDEO_TIMING_720X480P_60000) ||
                         (video_attr->video_timing == HDMI_VIDEO_TIMING_720X576P_50000) ||
                         (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X240P_60000) ||
                         (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X480I_60000) ||
                         (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) ||
                         (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X576P_50000) ||
                         (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X576I_60000));

    if (video_attr->video_timing == HDMI_VIDEO_TIMING_640X480P_60000) {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_4_3;
    } else if (colorimetry_is601 == HI_TRUE) {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_601;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_4_3;
    } else if (video_attr->video_timing <= HDMI_VIDEO_TIMING_4096X2160P_120000 &&
               video_attr->video_timing >= HDMI_VIDEO_TIMING_4096X2160P_24000) {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_256_135;
    } else {
        video_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
        video_attr->picture_aspect = HDMI_PICTURE_ASPECT_16_9;
    }

    video_attr->pixel_repeat = 1;
    if (video_attr->video_timing == HDMI_VIDEO_TIMING_1440X480I_60000 ||
        video_attr->video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) {
        video_attr->pixel_repeat = 2; /* 2, pixel repeat count */
    }

    if (video_attr->video_timing <= HDMI_VIDEO_TIMING_640X480P_60000) {
        // cts1.4 test_id 7-24 required rgb_quantization is default or limited when the timing is 640x480p60
        video_attr->rgb_quantization = HDMI_QUANTIZATION_RANGE_DEFAULT;
    } else {
        video_attr->ycc_quantization = HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
    }

    if (video_attr->video_timing != HDMI_VIDEO_TIMING_UNKNOWN &&
        video_attr->video_timing != HDMI_VIDEO_TIMING_640X480P_60000) {
        video_attr->in_color_space = HDMI_COLORSPACE_YCBCR444;
    } else {
        video_attr->in_color_space = HDMI_COLORSPACE_RGB;
    }

    video_attr->stereo_mode   = HDMI_3D_BUTT;
    video_attr->in_bit_depth  = HDMI_VIDEO_BITDEPTH_10;
    video_attr->active_aspect = HDMI_ACTIVE_ASPECT_PICTURE;
}

static hi_void hdmi_user_attr_init(hdmi_device *hdmi_dev)
{
    hdmi_app_attr     *app_attr = HI_NULL;
#ifdef HDMI_HDR_SUPPORT
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;
#endif

    app_attr = &hdmi_dev->attr.app_attr;
#ifdef HDMI_HDR_SUPPORT
    hdr_attr = &hdmi_dev->attr.hdr_attr;
#endif
    app_attr->enable_hdmi            = HI_TRUE;
    app_attr->enable_video           = HI_TRUE;
    app_attr->enable_audio           = HI_TRUE;
    app_attr->enable_aud_infoframe   = HI_TRUE;
    app_attr->enable_avi_infoframe   = HI_TRUE;
    app_attr->deep_color_mode        = HDMI_DEEP_COLOR_24BIT;
    app_attr->out_color_space        = HDMI_COLORSPACE_YCBCR444;
    app_attr->enable_clr_space_adapt = HI_TRUE;
    app_attr->enable_deep_clr_adapt  = HI_TRUE;
    app_attr->out_csc_quantization   = HDMI_QUANTIZATION_RANGE_LIMITED;
#ifdef HDMI_HDR_SUPPORT
#if defined(HDMI_CHIP_HI3559AV100)
    hdr_attr->user_hdr_mode = HDMI_HDR_USERMODE_HDR10;
#else
    hdr_attr->user_hdr_mode = HDMI_HDR_USERMODE_SDR;
#endif
    hdr_attr->colorimetry   = HDMI_HDR_COLORIMETRY_ITU_601;

    app_attr->enable_drm_infoframe    = HI_TRUE;
    hdmi_dev->hdr_mutex.attr_busy     = HI_FALSE;
    hdmi_dev->hdr_mutex.attr_set      = HI_FALSE;
    hdmi_dev->hdr_info.hdr_debug_mode = HDMI_HDR_DEBUG_MODE_NONE;
    hdmi_dev->hdr_info.hdr_mode_chn_timer.timer_start = HI_FALSE;
    hdmi_dev->hdr_info.hdr_mode_chn_timer.timer_type  = HDMI_TIMER_SDR_TO_HDR10;
    hdmi_dev->hdr_info.hdr_mode_chn_timer.time        = HDRMODE_CHANGE_TIME;
    hdmi_dev->hdr_info.zero_drm_if_timer.timer_start  = HI_FALSE;
    hdmi_dev->hdr_info.zero_drm_if_timer.timer_type   = HDMI_TIMER_ZERO_DRMIF;
    hdmi_dev->hdr_info.zero_drm_if_timer.time         = ZERO_DRMIF_SEND_TIME;
#endif

    return;
}

static hi_s32 hdmi_device_init(hdmi_device *hdmi_dev)
{
    hdmi_ao_attr *audio_attr = HI_NULL;
    hdmi_vo_attr *video_attr = HI_NULL;

    audio_attr = &hdmi_dev->attr.ao_attr;
    video_attr = &hdmi_dev->attr.vo_attr;
    get_hdmi_default_action_set(hdmi_dev, HDMI_DEFAULT_ACTION_HDMI);

#ifdef HDMI_EMI_ENABLE
    hdmi_dev->emi_enable = HI_TRUE;
#else
    hdmi_dev->emi_enable = HI_FALSE;
#endif
    /* common para init */
    hdmi_dev->debug.stop_delay = 0;
    hdmi_dev->delay.fmt_delay  = HDMI_DEV_FMT_DELAY;
    hdmi_dev->delay.mute_delay = HDMI_DEV_MUTE_DELAY;
    /* user attribute init */
    hdmi_user_attr_init(hdmi_dev);
    /* video attribute init */
    video_attr->video_timing = HDMI_VIDEO_TIMING_1280X720P_50000;
    video_attr->clk_fs       = HDMI_VIDEO_ATTR_CLK_FS;
    video_attr->h_sync_pol   = HI_FALSE;
    video_attr->v_sync_pol   = HI_FALSE;
    video_attr->de_pol       = HI_FALSE;
    (hi_void)memset_s(&(hdmi_dev->debug.intf_status), sizeof(hdmi_dev->debug.intf_status), 0, sizeof(hdmi_intf_status));
    hdmi_vo_attr_init(video_attr);

#if defined(CONFIG_HI_PLATFORM_H8)
    hdmi_dev->attr.app_attr.out_csc_quantization = hdmi_dev->csc_param.quantization;
    hdmi_dev->attr.app_attr.out_color_space = hdmi_dev->csc_param.pixel_encoding;
    video_attr->in_color_space = hdmi_dev->csc_param.pixel_encoding;
    video_attr->colorimetry = hdmi_dev->csc_param.colorimetry;
#endif
    /* audio attribute init */
    audio_attr->sound_intf   = HDMI_AUDIO_INTF_I2S;
    audio_attr->sample_fs    = HDMI_SAMPLE_RATE_48K;
    audio_attr->sample_depth = HDMI_AUDIO_BIT_DEPTH_16;
    audio_attr->channels     = HDMI_AUDIO_FORMAT_2CH;
#if defined(HDMI_CHIP_HI3519AV100)
    hdmi_dev->mode_param.trace_len = HDMI_TRACE_LEN_2;
#elif defined(HDMI_CHIP_HI3559AV100)
    hdmi_dev->mode_param.trace_len = HDMI_TRACE_LEN_1;
#elif defined(HDMI_CHIP_HI3516CV500)
    hdmi_dev->mode_param.trace_len = HDMI_TRACE_LEN_0;
#elif defined(PHY_CRAFT_S28)
    hdmi_dev->mode_param.trace_len = HDMI_TRACE_BUTT;
#endif

    compatibility_info_default_set(hdmi_dev->hdmi_dev_id);
    hdmi_info("video_timing:%u\n", video_attr->video_timing);

    return HI_SUCCESS;
}

static hi_void hdmi_dev_deinit(hdmi_device *hdmi_dev)
{
    hdmi_dev->emi_enable        = HI_FALSE;
    hdmi_dev->hpd_detected      = HI_FALSE;
    hdmi_dev->hpd_notified      = HI_FALSE;
    hdmi_dev->run_state         = HDMI_RUN_STATE_NONE;
    hdmi_dev->tmds_mode         = HDMI_TMDS_MODE_NONE;
    hdmi_dev->transition_state  = HDMI_TRANSITION_NONE;
    hdmi_dev->k_callback        = HI_NULL;
    hdmi_dev->hal               = HI_NULL;
    hdmi_dev->hdmi_dev_id       = 0;
    hdmi_dev->kernel_cnt        = 0;
    hdmi_dev->user_callback_cnt = 0;
    hdmi_dev->user_cnt          = 0;

    (hi_void)memset_s(&hdmi_dev->attr, sizeof(hdmi_dev->attr), 0, sizeof(hdmi_dev->attr));
    (hi_void)memset_s(&hdmi_dev->debug, sizeof(hdmi_dev->debug), 0, sizeof(hdmi_dev->debug));
    (hi_void)memset_s(&hdmi_dev->delay, sizeof(hdmi_dev->delay), 0, sizeof(hdmi_dev->delay));
    (hi_void)memset_s(&hdmi_dev->edid_info, sizeof(hdmi_dev->edid_info), 0, sizeof(hdmi_dev->edid_info));
    (hi_void)memset_s(&hdmi_dev->info_frame, sizeof(hdmi_dev->info_frame), 0, sizeof(hdmi_dev->info_frame));
    (hi_void)memset_s(&hdmi_dev->thread_info, sizeof(hdmi_dev->thread_info), 0, sizeof(hdmi_dev->thread_info));
#ifdef HDMI_HDR_SUPPORT
    hdmi_dev->hdr_mutex.attr_busy = HI_FALSE;
    (hi_void)memset_s(&hdmi_dev->hdr_info, sizeof(hdmi_dev->hdr_info), 0, sizeof(hdmi_dev->hdr_info));
#endif

    return;
}

static hi_s32 create_task(hdmi_device *hdmi_dev, hdmi_hal_init *hal_cfg)
{
    hi_s32 ret = HI_SUCCESS;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    if (hdmi_dev->thread_info.thread_timer == HI_NULL) {
        /* open hal module */
        hal_cfg->base_addr      = g_hdmi_reg;
        hal_cfg->phy_addr       = g_hdmi_phy;
        hal_cfg->event_callback = hdmi_event_callback;
        hal_cfg->event_data     = hdmi_dev;
        hal_cfg->hdmi_dev_id    = hdmi_dev->hdmi_dev_id;

        ret = hal_hdmi_open(hal_cfg, &hdmi_dev->hal);
        if (hdmi_dev->hal == HI_NULL || ret != HI_SUCCESS) {
            hdmi_err("hal_hdmi_open fail\n");
            return HI_FAILURE;
        }
        hdmi_dev->hal->hal_ctx.hdmi_dev = hdmi_dev;
        hdmi_dev->hal->hal_ctx.hdmi_id = hdmi_dev->hdmi_dev_id;

        hdmi_info("create HDMI timer thread \n");
#ifdef HDMI_LITEOS_SUPPORT
        hdmi_dev->thread_info.thread_run = HI_TRUE;
        osal_wait_init(&hdmi_dev->thread_info.wait_destroy);
        hdmi_dev->thread_info.thread_exit = HI_FALSE;
#endif
        hdmi_dev->thread_info.thread_timer = osal_kthread_create(hdmi_kthread_timer, hdmi_dev, "hi_hdmi_k_thread");
        if (hdmi_dev->thread_info.thread_timer == HI_NULL) {
            hdmi_err("create HDMI timer thread fail\n");
            return HI_FAILURE;
        }

        hdmi_device_init(hdmi_dev);
    }

    return ret;
}

static hi_s32 hdmi_oe_transition_state_set(hdmi_device *hdmi_dev, hi_bool user, hi_bool hdmi_on)
{
    hi_s32 ret = HI_SUCCESS;

#ifndef HDMI_FPGA_SUPPORT
    if (hdmi_on == HI_FALSE) {
        hdmi_info("hdmi_on:%u, hardware init...\n", hdmi_on);
        hdmi_dev->transition_state = HDMI_TRANSITION_NONE;
        hal_call_void(hal_hdmi_hardware_init, hdmi_dev->hal);
    } else {
        hdmi_info("hdmi_on:%u...\n", hdmi_on);
        if (user) {
            hdmi_dev->transition_state = (hdmi_dev->kernel_cnt > 0) ?
                HDMI_TRANSITION_MCE_APP : HDMI_TRANSITION_BOOT_APP;
        } else {
            hdmi_dev->transition_state = HDMI_TRANSITION_BOOT_MCE;
        }
    }
#else
    hdmi_info("hdmi_on:%u...\n", hdmi_on);
    if (user) {
        hdmi_dev->transition_state = (hdmi_dev->kernel_cnt > 0) ?
            HDMI_TRANSITION_MCE_APP : HDMI_TRANSITION_BOOT_APP;
    } else {
        hdmi_dev->transition_state = HDMI_TRANSITION_BOOT_MCE;
    }
#endif

    return ret;
}

static hi_void drv_hdmi_hpd_status_delay_get (hdmi_device *hdmi_dev, hi_bool *hotplug)
{
    hi_u32 i;

    for (i = 0; i < HDMI_READ_HPD_STATUS_DELAY; i++) {
        hal_call_void(hal_hdmi_hot_plug_status_get, hdmi_dev->hal, hotplug);
        if (*hotplug) {
            break;
        }
        osal_msleep(1);
    }
    hdmi_info("delay %d times, hot plug status is 0X%x\n", i, (*hotplug ? HDMI_EVENT_HOTPLUG : HDMI_EVENT_HOTUNPLUG));

    return;
}

static hi_void notify_event_in_open(hdmi_device *hdmi_dev, hi_bool hdmi_on, hi_bool hotplug)
{
    if (hdmi_on == HI_TRUE) { /* need to updata event in open, when there is boot logo. */
        hdmi_event_callback(hdmi_dev, hotplug ? HDMI_EVENT_HOTPLUG : HDMI_EVENT_HOTUNPLUG);
    } else {
        if (hotplug == HI_TRUE) {
            hpd_event(hdmi_dev);
        } else {
            hdmi_dev->hpd_detected = HI_FALSE;
        }
    }

    return;
}

hi_s32 drv_hdmi_open(hdmi_device *hdmi_dev, hi_bool user)
{
    hi_s32 ret;
    hdmi_hal_init hal_cfg       = {0};
    hi_u32        event_pool_id = 0;
    hi_bool       hotplug       = HI_FALSE;
    hi_bool       hdmi_on       = HI_FALSE;

    /* create hdmi task, every hdmi device only create once */
    if (create_task(hdmi_dev, &hal_cfg) != HI_SUCCESS) {
        return HI_ERR_HDMI_CREATE_TESK_FAILED;
    }

    drv_hdmi_event_init(hdmi_dev->hdmi_dev_id);
#ifdef HDMI_FRL_SUPPORT
    drv_hdmi_frl_mach_init(hdmi_dev->hdmi_dev_id);
#endif

    if (user == HI_TRUE) {
        /* must create event queue first */
        ret = drv_hdmi_event_pool_malloc(hdmi_dev->hdmi_dev_id, &event_pool_id);
        if (ret != HI_SUCCESS) {
            if (ret != HDMI_EVENT_ID_EXIST) {
                hdmi_err("drv_hdmi_event_pool_malloc fail\n");
                return HI_ERR_HDMI_MALLOC_FAILED;
            }
        } else {
            hdmi_dev->user_cnt++;
        }
        hdmi_info("create event queue for process:%u \n", hdmi_current_id_get(hdmi_dev));
    } else {
        hdmi_dev->kernel_cnt++;
    }

    if (hdmi_dev->user_callback_cnt == 0) {
        hdmi_mutex_unlock(g_hdmi_mutex);
        hdmi_mutex_lock(hdmi_dev->mutex_thread);
        hdmi_dev->k_callback = hi_drv_hdmi_kernel_event_callback;
        hdmi_mutex_unlock(hdmi_dev->mutex_thread);
        hdmi_mutex_lock(g_hdmi_mutex);
    }
    hdmi_info("user_cnt: %u, kernel_cnt: %u, user_call_back_cnt: %u\n",
              hdmi_dev->user_cnt, hdmi_dev->kernel_cnt, hdmi_dev->user_callback_cnt);

    hal_call_void(hal_hdmi_phy_output_enable_get, hdmi_dev->hal, &hdmi_on);
    ret = hdmi_oe_transition_state_set(hdmi_dev, user, hdmi_on);
    hdmi_if_failure_return(ret, HI_FAILURE);
    /* insure that get sink caps success immediately after open HDMI when no boot or no output. */
    drv_hdmi_hpd_status_delay_get(hdmi_dev, &hotplug);
    notify_event_in_open(hdmi_dev, hdmi_on, hotplug);
    hdmi_info("transition_state: %u\n", hdmi_dev->transition_state);
    hdmi_dev->run_state = HDMI_RUN_STATE_OPEN;
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

#ifdef HDMI_LITEOS_SUPPORT
static hi_s32 hdmi_wait_thread_exit_callback(const hi_void *param)
{
    hi_s32 result = HI_FALSE;
    const hdmi_device *hdmi_dev = (const hdmi_device *)param;

    if (hdmi_dev == HI_NULL) {
        return result;
    }
    result = (hdmi_dev->thread_info.thread_exit == HI_TRUE);

    return result;
}
#endif

static hi_s32 hdmi_release(hdmi_device *hdmi_dev)
{
    hdmi_info(">>> in...\n");

    if ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_START) {
        drv_hdmi_stop(hdmi_dev);
    }

    if (hdmi_dev->user_cnt == 0) {
        if (hdmi_dev->kernel_cnt == 0) {
            if (hdmi_dev->thread_info.thread_timer) {
                hdmi_info("stop hdmi kernel thread timer\n");
#ifdef HDMI_LITEOS_SUPPORT
                hdmi_dev->thread_info.thread_run = HI_FALSE;
                osal_wait_event_uninterruptible(&hdmi_dev->thread_info.wait_destroy,
                    hdmi_wait_thread_exit_callback, (hi_void *)hdmi_dev);
                /*
                 * note: in liteos,
                 * when call the kthread_stop function,
                 * the thread function should be finished,
                 * if task(hdmi_kthread_timer) get a mutex is deleted or automatically deleted before release
                 * this mutex, other tasks pended this mutex maybe never be scheduled.
                 */
                osal_kthread_destroy(hdmi_dev->thread_info.thread_timer, HI_TRUE);
                osal_wait_destroy(&hdmi_dev->thread_info.wait_destroy);
#else
                /*
                 * note: in linux,
                 * when call the kthread_stop function,
                 * the thread function cannot be finished, otherwise it will oops.
                 */
                osal_kthread_destroy(hdmi_dev->thread_info.thread_timer, HI_TRUE);
#endif
                hdmi_dev->thread_info.thread_timer = HI_NULL;
                get_hdmi_thread_state_set(hdmi_dev, HDMI_THREAD_STATE_STOP);
            }
            drv_hdmi_event_deinit(hdmi_dev->hdmi_dev_id);
            hdmi_mutex_lock(hdmi_dev->mutex_proc);
            if (hdmi_dev->hal != HI_NULL) {
                hal_hdmi_close(hdmi_dev->hal);
                hdmi_dev->hal = HI_NULL;
            }
            hdmi_dev_deinit(hdmi_dev);
            hdmi_mutex_unlock(hdmi_dev->mutex_proc);
        } else {
            drv_hdmi_event_deinit(hdmi_dev->hdmi_dev_id);
            hdmi_mutex_unlock(g_hdmi_mutex);
            hdmi_mutex_lock(hdmi_dev->mutex_thread);
            hdmi_dev->k_callback = hi_drv_hdmi_kernel_event_callback;
            hdmi_mutex_unlock(hdmi_dev->mutex_thread);
            hdmi_mutex_lock(g_hdmi_mutex);
        }
    }
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_close(hdmi_device *hdmi_dev, hi_bool user)
{
    hi_s32 ret;
#if defined(HDMI_FPGA_SUPPORT) && defined(HDMI_HDR_SUPPORT)
    /* to avoid HDR mode change error when app exit. delete it after ASIC back. */
    drv_hdmi_hdr_attr hdr_attr = {0};
#endif

    hdmi_info(">>> in...\n");
    hdmi_check_open_return((hi_u32)hdmi_dev->run_state);

    hdmi_info("user: %u\n", user);
    hdmi_info("user_cnt: %u, kernel_cnt: %u\n", hdmi_dev->user_cnt, hdmi_dev->kernel_cnt);

    hdmi_dev->hpd_notified = HI_FALSE;

    if (user && hdmi_dev->user_cnt > 0) {
        hdmi_info("delete event queue for process:%u \n", hdmi_current_id_get(hdmi_dev));
        ret = drv_hdmi_event_pool_free(hdmi_dev->hdmi_dev_id, hdmi_current_id_get(hdmi_dev));
        if (ret != HI_SUCCESS) {
            hdmi_err("drv_hdmi_event_pool_free fail\n");
            return HI_ERR_HDMI_FREE_FAILED;
        }
#if defined(HDMI_FPGA_SUPPORT) && defined(HDMI_HDR_SUPPORT)
        /* to avoid HDR mode change error when app exit. delete it after ASIC back. */
        if (hdmi_dev->user_cnt == 1) {
            hdr_attr.hdr_mode = HI_HDMI_HDR_MODE_DISABLE;
            hdr_attr.eotf_type = HI_HDMI_EOTF_SDR_LUMIN;
            hdr_attr.colorimetry = HI_HDMI_COLORIMETRY_ITU709;
            hi_drv_hdmi_set_hdr_attr(HI_HDMI_DEV_ID_0, &hdr_attr);
        }
#endif
        hdmi_dev->user_cnt--;
        if (hdmi_dev->user_cnt == 0 && hdmi_dev->kernel_cnt > 0) {
            hdmi_mutex_unlock(g_hdmi_mutex);
            hdmi_mutex_lock(hdmi_dev->mutex_thread);
            hdmi_dev->k_callback = hi_drv_hdmi_kernel_event_callback;
            hdmi_mutex_unlock(hdmi_dev->mutex_thread);
            hdmi_mutex_lock(g_hdmi_mutex);
        } else {
            hdmi_mutex_unlock(g_hdmi_mutex);
            hdmi_mutex_lock(hdmi_dev->mutex_thread);
            hdmi_dev->k_callback = HI_NULL;
            hdmi_mutex_unlock(hdmi_dev->mutex_thread);
            hdmi_mutex_lock(g_hdmi_mutex);
        }
    } else if (!user && hdmi_dev->kernel_cnt > 0) {
        hdmi_dev->kernel_cnt--;
    }

    if (hdmi_dev->user_cnt == 0 && hdmi_dev->kernel_cnt == 0) {
        hdmi_release(hdmi_dev);
        hdmi_dev->run_state = HDMI_RUN_STATE_CLOSE;
    }

    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_start(hdmi_device *hdmi_dev)
{
    hi_bool audio_enable = HI_FALSE;
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_check_open_return((hi_u32)hdmi_dev->run_state);

    app_attr = &hdmi_dev->attr.app_attr;
    audio_enable = (hdmi_dev->attr.app_attr.enable_audio) && (app_attr->enable_hdmi);
    /* enable clk */
    drv_hdmi_low_power_set(HI_FALSE);

#ifdef HDMI_FRL_SUPPORT
    if (hdmi_dev->frl_info.mode != HDMI_FRL_MODE_FRL) {
        if (hdmi_mode_strategy(hdmi_dev) != HI_SUCCESS) {
            hdmi_err(" hdmi_mode_strategy fail\n");
        }
    }
#else
    if (hdmi_mode_strategy(hdmi_dev) != HI_SUCCESS) {
        hdmi_err(" hdmi_mode_strategy fail\n");
    }
#endif
    /*
     * 20160415, fix SAM doesn't show at 10bit/12bit.
     * this TV clear mute must after phy enable when the deepcolor is 10bit and 12bit.
     */
    hal_call_void(hal_hdmi_avmute_set, hdmi_dev->hal, HI_FALSE);

#ifdef HDMI_FRL_SUPPORT
    if (hdmi_dev->frl_info.mode != HDMI_FRL_MODE_TMDS) {
        hdmi_info("start frl...\n");
        drv_hdmi_frl_enable(hdmi_dev->hdmi_dev_id, HI_TRUE);
    } else {
        hdmi_info("TMDS phy config\n");
        hdmi_phy_output_enable(hdmi_dev, HI_TRUE);
    }
#else
    hdmi_info("TMDS phy config\n");
    hdmi_phy_output_enable(hdmi_dev, HI_TRUE);
#endif
    drv_hdmi_avmute_set(hdmi_dev, HI_FALSE);
    drv_hdmi_audio_path_enable(hdmi_dev, audio_enable);

    if (hdmi_dev->attr.app_attr.enable_video == HI_FALSE) {
        /* if video is disable, send black video. */
        hdmi_info("video was disable by user, so send blackframe.\n");
        drv_hdmi_black_data_set(hdmi_dev, HI_TRUE);
    } else if (hdmi_dev->attr.app_attr.hdcp_enable == HI_FALSE) {
        /* stop black frame here if hdcp is disable, otherwise stop it when the hdcp auth success. */
        drv_hdmi_black_data_set(hdmi_dev, HI_FALSE);
    }
    hdmi_dev->run_state = (hi_u32)hdmi_dev->run_state & (~(HDMI_RUN_STATE_STOP));
    hdmi_dev->run_state = (hi_u32)hdmi_dev->run_state | (HDMI_RUN_STATE_START);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

static hi_void hdmi_debug_delay(hdmi_device *hdmi_dev, const hi_char *info)
{
    hdmi_if_null_return_void(info);

    osal_msleep(hdmi_dev->debug.stop_delay);

    if (hdmi_dev->debug.stop_delay > 0) {
        hdmi_info("%s, delay %ums...\n", info, hdmi_dev->debug.stop_delay);
    }
}

hi_s32 drv_hdmi_stop(hdmi_device *hdmi_dev)
{
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);

    hdmi_debug_delay(hdmi_dev, "drv_hdmi_stop debug start");
    if (!((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_START) || ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_STOP)) {
        return HI_SUCCESS;
    }
    hdmi_debug_delay(hdmi_dev, "drv_hdmi_stop audio path disable");
    drv_hdmi_audio_path_enable(hdmi_dev, HI_FALSE);
    hdmi_debug_delay(hdmi_dev, "drv_hdmi_stop black data enable");
    drv_hdmi_black_data_set(hdmi_dev, HI_TRUE);
    hdmi_debug_delay(hdmi_dev, "drv_hdmi_stop avmute enable");
    drv_hdmi_avmute_set(hdmi_dev, HI_TRUE);
    hdmi_debug_delay(hdmi_dev, "drv_hdmi_stop phy disable");
#ifdef HDMI_FRL_SUPPORT
    if (hdmi_dev->frl_info.mode != HDMI_FRL_MODE_TMDS) {
        drv_hdmi_frl_enable(hdmi_dev->hdmi_dev_id, HI_FALSE);
    } else {
        hdmi_phy_output_enable(hdmi_dev, HI_FALSE);
    }
#else
    hdmi_phy_output_enable(hdmi_dev, HI_FALSE);
#endif

    /* disable clk for low power */
    drv_hdmi_low_power_set(HI_TRUE);
    hdmi_dev->run_state = (hi_u32)hdmi_dev->run_state  & (~(HDMI_RUN_STATE_START));
    hdmi_dev->run_state = (hi_u32)hdmi_dev->run_state | HDMI_RUN_STATE_STOP;
    drv_hdmi_compat_stop_delay(hdmi_dev->hdmi_dev_id);
    hdmi_debug_delay(hdmi_dev, "stop delay");
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_void drv_hdmi_avmute_set(const hdmi_device *hdmi_dev, hi_bool av_mute)
{
    hdmi_delay delay = {0};

    hdmi_info(">>> in..., avmute: %u\n", av_mute);

    hdmi_if_null_return_void(hdmi_dev);
    hdmi_if_null_return_void(hdmi_dev->hal);

    delay.fmt_delay  = hdmi_dev->delay.fmt_delay;
    delay.mute_delay = hdmi_dev->delay.mute_delay;
    drv_hdmi_compat_delay_get(hdmi_dev->hdmi_dev_id, &delay);

    if (hdmi_dev->attr.app_attr.auth_mode != HI_TRUE) {
        /* fix compatibility problem */
        if (!av_mute) {
            hdmi_info("format_delay %u ms \n", delay.fmt_delay);
            osal_msleep(delay.fmt_delay);
        }
    }
    hal_call_void(hal_hdmi_avmute_set, hdmi_dev->hal, av_mute);

    if (hdmi_dev->attr.app_attr.auth_mode != HI_TRUE) {
        /* fix compatibility problem */
        if (av_mute) {
            hdmi_info("mute_delay %u ms \n", delay.mute_delay);
            osal_msleep(delay.mute_delay);
        }
    }
    hdmi_info("<<< out...\n");

    return;
}

static hi_void drv_hdmi_black_data_set(const hdmi_device *hdmi_dev, hi_bool enable)
{
    hdmi_black_frame_info black_info;
    hdmi_if_null_return_void(hdmi_dev);

    black_info.black_enable = enable;
    black_info.in_bit_depth = hdmi_dev->attr.vo_attr.in_bit_depth;
    black_info.in_color_space = hdmi_dev->attr.vo_attr.in_color_space;
    black_info.in_quantization = (black_info.in_color_space == HDMI_COLORSPACE_RGB) ?
        hdmi_dev->attr.vo_attr.rgb_quantization : hdmi_dev->attr.vo_attr.ycc_quantization;
    hal_call_void(hal_hdmi_black_data_set, hdmi_dev->hal, &black_info);

    return;
}

hi_s32 drv_hdmi_deep_color_set(hdmi_device *hdmi_dev, hdmi_deep_color deep_color)
{
    hi_s32 ret;
    hdmi_attr attr = {0};

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);

    hdmi_info("deep_color: %u\n", deep_color);

    ret = drv_hdmi_attr_get(hdmi_dev, &attr);
    if (attr.app_attr.out_color_space == HDMI_COLORSPACE_YCBCR422) {
        if ((deep_color != HDMI_DEEP_COLOR_24BIT) && (deep_color != HDMI_DEEP_COLOR_OFF)) {
            hdmi_err("not support deepcolor:%u when colorspace is ycbcr422. \n", deep_color);
            return HI_ERR_HDMI_INVALID_PARA;
        }
    }
#ifdef HDMI_HDR_SUPPORT
    /* if the current hdmi mode is dolby, deep_color_set is invalid. */
    if (attr.hdr_attr.hdr_mode == HDMI_HDR_MODE_DOLBY_NORMAL ||
        attr.hdr_attr.hdr_mode == HDMI_HDR_MODE_DOLBY_TUNNELING) {
        hdmi_warn("don't change deepcolor when dolby mode is:%u \n", attr.hdr_attr.hdr_mode);
        return HI_FAILURE;
    }
#endif
    attr.app_attr.deep_color_mode = deep_color;
    drv_hdmi_avmute_set(hdmi_dev, HI_TRUE);
    ret = drv_hdmi_attr_set(hdmi_dev, &attr);
    drv_hdmi_avmute_set(hdmi_dev, HI_FALSE);
    hdmi_info("<<< out...\n");

    return ret;
}

hi_s32 drv_hdmi_deep_color_get(hdmi_device *hdmi_dev, hdmi_deep_color *deep_color)
{
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(deep_color, HI_ERR_NULL_PTR);

    app_attr = &hdmi_dev->attr.app_attr;
    *deep_color = app_attr->deep_color_mode;

    hdmi_info("deep_color: %u\n", *deep_color);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

static hi_s32 check_audio_attr(const hdmi_ao_attr *ao_attr)
{
    if (ao_attr->sound_intf != HDMI_AUDIO_INTF_I2S &&
        ao_attr->sound_intf != HDMI_AUDIO_INTF_SPDIF &&
        ao_attr->sound_intf != HDMI_AUDIO_INTF_HBRA) {
        hdmi_err("the audio interface(%u) is invalid\n", ao_attr->sound_intf);
        return HI_FAILURE;
    }

    if (ao_attr->channels < HDMI_AUDIO_FORMAT_2CH || ao_attr->channels > HDMI_AUDIO_FORMAT_8CH) {
        hdmi_err("the audio channel number(%u) is invalid\n", ao_attr->channels);
        return HI_FAILURE;
    }

    if (ao_attr->sample_fs < HDMI_SAMPLE_RATE_32K || ao_attr->sample_fs > HDMI_SAMPLE_RATE_768K) {
        hdmi_err("the input audio frequency(%u) is invalid\n", ao_attr->sample_fs);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_ao_attr_set(hdmi_device *hdmi_dev, const hdmi_ao_attr *ao_attr)
{
    errno_t errnumber;
    hi_s32            ret        = HI_SUCCESS;
    hdmi_attr         hw_attr    = {0};
    hdmi_audio_config audio_cfg  = {0};
    hdmi_app_attr    *app_attr   = HI_NULL;
    hdmi_ao_attr     *audio_attr = HI_NULL;
    hdmi_vo_attr     *video_attr = HI_NULL;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(ao_attr, HI_ERR_NULL_PTR);

    if (check_audio_attr(ao_attr) != HI_SUCCESS) {
        hdmi_err("check_audio_attr fail\n");
        return HI_FAILURE;
    }
    app_attr   = &hdmi_dev->attr.app_attr;
    video_attr = &hdmi_dev->attr.vo_attr;
    audio_attr = &hdmi_dev->attr.ao_attr;

    errnumber = memcpy_s(audio_attr, sizeof(hdmi_dev->attr.ao_attr), ao_attr, sizeof(hdmi_ao_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    hdmi_attr_construct(hdmi_dev, &hw_attr);

    hdmi_info("down_sample: %u\n", audio_attr->down_sample);
    hdmi_info("sample_fs: %u\n", audio_attr->sample_fs);
    hdmi_info("channels: %u\n", audio_attr->channels);
    hdmi_info("sound_intf: %u\n", audio_attr->sound_intf);
    hdmi_info("sample_depth: %u\n", audio_attr->sample_depth);
    hdmi_info("audio_code: %u\n", audio_attr->audio_code);

    audio_cfg.down_sample  = ao_attr->down_sample;
    audio_cfg.layout       = ao_attr->channels;
    audio_cfg.sample_depth = ao_attr->sample_depth;
    audio_cfg.sample_fs    = ao_attr->sample_fs;
    audio_cfg.sound_intf   = ao_attr->sound_intf;
    audio_cfg.enable_audio = (hdmi_dev->attr.app_attr.enable_audio) && (app_attr->enable_hdmi);
    audio_cfg.tmds_clk     = video_attr->hdmi_adapt_pix_clk;
    audio_cfg.pixel_repeat = video_attr->pixel_repeat;
    hal_call_void(hal_hdmi_audio_path_enable_set, hdmi_dev->hal, HI_FALSE);
    hal_call_ret(ret, hal_hdmi_audio_path_set, hdmi_dev->hal, &audio_cfg);
    drv_hdmi_audio_infoframe_send(&hdmi_dev->info_frame, (app_attr->enable_audio && app_attr->enable_aud_infoframe));
    hal_call_void(hal_hdmi_audio_path_enable_set, hdmi_dev->hal, audio_cfg.enable_audio);
    hdmi_info("<<< out...\n");

    return ret;
}

hi_void drv_hdmi_audio_path_enable(const hdmi_device *hdmi_dev, hi_bool enable)
{
    hal_call_void(hal_hdmi_audio_path_enable_set, hdmi_dev->hal, enable);
    return;
}

hi_s32 drv_hdmi_ao_attr_get(const hdmi_device *hdmi_dev, hdmi_ao_attr *ao_attr)
{
    errno_t errnumber;
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(ao_attr, HI_ERR_NULL_PTR);

    errnumber = memcpy_s(ao_attr, sizeof(*ao_attr), &hdmi_dev->attr.ao_attr, sizeof(hdmi_ao_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_audio_output_enable_set(const hdmi_device *hdmi_dev, hi_bool enable)
{
    hi_bool mute = enable ? HI_FALSE : HI_TRUE;
    hal_call_void(hal_hdmi_audio_mute_set, hdmi_dev->hal, mute);
    return HI_SUCCESS;
}

hi_s32 get_hdmi_thread_state_set(hdmi_device *hdmi_dev, hdmi_thread_state state)
{
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_info("state: %u\n", state);
    hdmi_dev->thread_info.thread_timer_sate = state;
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

static hi_s32 check_video_attr(const hdmi_vo_attr *vo_attr)
{
    hdmi_if_null_return(vo_attr, HI_ERR_NULL_PTR);

    if (vo_attr->video_timing >= HDMI_VIDEO_TIMING_BUTT) {
        hdmi_err("video timing(%u) is wrong\n", vo_attr->video_timing);
        return HI_FAILURE;
    }

    if (vo_attr->in_color_space >= HDMI_COLORSPACE_BUTT) {
        hdmi_err("video in_color_space(%u) is wrong\n", vo_attr->in_color_space);
        return HI_FAILURE;
    }

    if (vo_attr->colorimetry != HDMI_COLORIMETRY_ITU_601 &&
        vo_attr->colorimetry != HDMI_COLORIMETRY_ITU_709 &&
        vo_attr->colorimetry != HDMI_COLORIMETRY_EXTEND) {
        hdmi_err("video colorimetry(%u) is wrong\n", vo_attr->colorimetry);
        return HI_FAILURE;
    }

    if (vo_attr->rgb_quantization > HDMI_QUANTIZATION_RANGE_FULL) {
        hdmi_err("video rgb_quantization(%u) is wrong\n", vo_attr->rgb_quantization);
        return HI_FAILURE;
    }

    if (vo_attr->picture_aspect != HDMI_PICTURE_ASPECT_4_3 &&
        vo_attr->picture_aspect != HDMI_PICTURE_ASPECT_16_9 &&
        vo_attr->picture_aspect != HDMI_PICTURE_ASPECT_64_27 &&
        vo_attr->picture_aspect != HDMI_PICTURE_ASPECT_256_135) {
        hdmi_err("video picture_aspect(%u) is wrong\n", vo_attr->picture_aspect);
        return HI_FAILURE;
    }

    if (vo_attr->in_bit_depth != HDMI_VIDEO_BITDEPTH_8 &&
        vo_attr->in_bit_depth != HDMI_VIDEO_BITDEPTH_10 &&
        vo_attr->in_bit_depth != HDMI_VIDEO_BITDEPTH_12) {
        hdmi_err("video in_bit_depth(%u) is wrong\n", vo_attr->in_bit_depth);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void hdmi_vo_colorimetry_cfg(const hdmi_vo_attr *vo_attr, hdmi_video_config *video_cfg)
{
    switch (vo_attr->colorimetry) {
        case HDMI_COLORIMETRY_ITU_601:
            video_cfg->conv_std = HDMI_CONV_STD_BT_601;
            break;
        case HDMI_COLORIMETRY_ITU_709:
            video_cfg->conv_std = HDMI_CONV_STD_BT_709;
            break;
        case HDMI_COLORIMETRY_EXTEND:
            if (vo_attr->extended_colorimetry == HDMI_EXTENDED_COLORIMETRY_2020_NON_CONST_LUMINOUS) {
                video_cfg->conv_std = HDMI_CONV_STD_BT_2020_NON_CONST_LUMINOUS;
            } else if (vo_attr->extended_colorimetry == HDMI_EXTENDED_COLORIMETRY_2020_CONST_LUMINOUS) {
                video_cfg->conv_std = HDMI_CONV_STD_BT_2020_CONST_LUMINOUS;
            }
            break;
        default:
            video_cfg->conv_std = HDMI_CONV_STD_BT_709;
            break;
    }

    return;
}

static hi_void hdmi_attr_to_vid_cfg(hdmi_vo_attr *vo_attr, const hdmi_app_attr *app_attr, hdmi_video_config *video_cfg)
{
    hdmi_if_null_return_void(vo_attr);
    hdmi_if_null_return_void(app_attr);

    video_cfg->pixel_clk =
        (app_attr->out_color_space == HDMI_COLORSPACE_YCBCR420) ? (vo_attr->clk_fs >> 1) : vo_attr->clk_fs;
    video_cfg->tmds_clk        = vo_attr->hdmi_adapt_pix_clk;
    video_cfg->in_bit_depth    = vo_attr->in_bit_depth;
    video_cfg->quantization    = vo_attr->rgb_quantization;
    video_cfg->in_color_space  = vo_attr->in_color_space;
    video_cfg->deep_color      = app_attr->deep_color_mode;
    video_cfg->out_color_space = app_attr->out_color_space;
    video_cfg->v_sync_pol      = vo_attr->v_sync_pol;
    video_cfg->h_sync_pol      = vo_attr->h_sync_pol;
    video_cfg->de_pol          = vo_attr->de_pol;

    if (video_cfg->out_color_space == HDMI_COLORSPACE_RGB) {
        vo_attr->rgb_quantization = app_attr->out_csc_quantization;
        vo_attr->ycc_quantization = (vo_attr->rgb_quantization == HDMI_QUANTIZATION_RANGE_FULL) ?
            HDMI_YCC_QUANTIZATION_RANGE_FULL : HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
    } else {
        vo_attr->ycc_quantization = (app_attr->out_csc_quantization == HDMI_QUANTIZATION_RANGE_FULL) ?
            HDMI_YCC_QUANTIZATION_RANGE_FULL : HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
        if (vo_attr->ycc_quantization == HDMI_YCC_QUANTIZATION_RANGE_FULL) {
            vo_attr->rgb_quantization = HDMI_QUANTIZATION_RANGE_FULL;
        } else {
            vo_attr->rgb_quantization = HDMI_QUANTIZATION_RANGE_DEFAULT;
        }
    }

    if (app_attr->out_color_space == HDMI_COLORSPACE_RGB) {
        video_cfg->out_csc_quantization = vo_attr->rgb_quantization;
    } else {
        switch (vo_attr->ycc_quantization) {
            case HDMI_YCC_QUANTIZATION_RANGE_FULL:
                video_cfg->out_csc_quantization = HDMI_QUANTIZATION_RANGE_FULL;
                break;
            default:
                video_cfg->out_csc_quantization = HDMI_QUANTIZATION_RANGE_LIMITED;
                break;
        }
    }
    hdmi_vo_colorimetry_cfg(vo_attr, video_cfg);

    return;
}

static hi_s32 hdmi_phy_set(hdmi_device *hdmi_dev, const hdmi_vo_attr *vo_attr)
{
    hi_s32         ret      = HI_SUCCESS;
#ifndef HDMI_FPGA_SUPPORT
    hdmi_phy_cfg   phy_cfg  = {0};
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    app_attr = &hdmi_dev->attr.app_attr;

    /* PHY config */
    phy_cfg.pixel_clk  =
        (app_attr->out_color_space == HDMI_COLORSPACE_YCBCR420) ? (vo_attr->clk_fs >> 1) : vo_attr->clk_fs;
    phy_cfg.emi_enable = hdmi_dev->emi_enable;
    phy_cfg.tmds_clk   = vo_attr->hdmi_adapt_pix_clk;
    phy_cfg.deep_color = app_attr->deep_color_mode;
    phy_cfg.trace_len  = hdmi_dev->mode_param.trace_len;
#ifdef HDMI_FRL_SUPPORT
    phy_cfg.color_space = app_attr->out_color_space;
    if (hdmi_dev->frl_info.mode == HDMI_FRL_MODE_FRL) {
        phy_cfg.mode_cfg = HDMI_PHY_MODE_CFG_FRL;
        (hi_void)memset_s(phy_cfg.aen_tx_ffe, sizeof(phy_cfg.aen_tx_ffe), FRL_TXFFE_MODE_0, sizeof(phy_cfg.aen_tx_ffe));
        phy_cfg.rate = (hdmi_work_mode)hdmi_dev->frl_info.rate_info.cur_rate;
    }
#endif
    hal_call_void(hal_hdmi_phy_set, hdmi_dev->hal, &phy_cfg);
#endif

    return ret;
}

static hi_s32 hdmi_csc_set(hdmi_device *hdmi_dev, hdmi_vo_attr *vo_attr)
{
    hi_s32 ret = HI_SUCCESS;
    hdmi_video_config video_cfg = {0};
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    app_attr = &hdmi_dev->attr.app_attr;
    hdmi_attr_to_vid_cfg(vo_attr, app_attr, &video_cfg);
    hal_call_void(hal_hdmi_csc_param_set, hdmi_dev->hal, &video_cfg);

    return ret;
}

static hi_s32 hdmi_video_path_set(hdmi_device *hdmi_dev, hdmi_vo_attr *vo_attr)
{
    hdmi_video_config video_cfg = {0};
    hdmi_app_attr *app_attr = HI_NULL;
    hi_s32 ret = HI_SUCCESS;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    app_attr = &hdmi_dev->attr.app_attr;
    video_cfg.emi_enable = hdmi_dev->emi_enable;

    hdmi_attr_to_vid_cfg(vo_attr, app_attr, &video_cfg);
    hal_call_ret(ret, hal_hdmi_video_path_set, hdmi_dev->hal, &video_cfg);
    hdmi_csc_set(hdmi_dev, vo_attr);

#ifdef HDMI_FRL_SUPPORT
    if (hdmi_dev->frl_info.mode == HDMI_FRL_MODE_TMDS) {
        hdmi_phy_set(hdmi_dev, vo_attr);
    }
#else
    hdmi_phy_set(hdmi_dev, vo_attr);
#endif
    return ret;
}

static hi_bool app_attr_is_changed(const hdmi_app_attr *hw_app_attr, const hdmi_app_attr *sw_app_attr)
{
    hdmi_if_null_return(hw_app_attr, HI_FALSE);
    hdmi_if_null_return(sw_app_attr, HI_FALSE);

    if (hw_app_attr->enable_hdmi != sw_app_attr->enable_hdmi) {
        hdmi_info("hdmi enable change, old(%u)->new(%u)\n", hw_app_attr->enable_hdmi, sw_app_attr->enable_hdmi);
        return HI_TRUE;
    }

    if (hw_app_attr->out_color_space != sw_app_attr->out_color_space) {
        hdmi_info("out color_space change,old(%u)->new(%u)\n",
            hw_app_attr->out_color_space, sw_app_attr->out_color_space);
        return HI_TRUE;
    }

    if (((sw_app_attr->deep_color_mode == HDMI_DEEP_COLOR_OFF) ||
        (sw_app_attr->deep_color_mode == HDMI_DEEP_COLOR_24BIT)) &&
        ((hw_app_attr->deep_color_mode == HDMI_DEEP_COLOR_OFF) ||
        (hw_app_attr->deep_color_mode == HDMI_DEEP_COLOR_24BIT))) {
        hdmi_info("deepcolor mode not change: %u\n", hw_app_attr->deep_color_mode);
    } else if (hw_app_attr->deep_color_mode != sw_app_attr->deep_color_mode) {
        hdmi_info("deepcolor mode change,old(%u)->new(%u)\n",
            hw_app_attr->deep_color_mode, sw_app_attr->deep_color_mode);
        return HI_TRUE;
    }

    if (hw_app_attr->xvycc_mode != sw_app_attr->xvycc_mode) {
        hdmi_info("xvycc_mode mode change,old(%u)->new(%u)\n",
            hw_app_attr->xvycc_mode, sw_app_attr->xvycc_mode);
        return HI_TRUE;
    }

    if (hw_app_attr->enable_avi_infoframe != sw_app_attr->enable_avi_infoframe) {
        hdmi_info("avi infoframe enable change, old(%u)->new(%u)\n",
            hw_app_attr->enable_avi_infoframe, sw_app_attr->enable_avi_infoframe);
        return HI_TRUE;
    }

    if (hw_app_attr->out_csc_quantization != sw_app_attr->out_csc_quantization) {
        hdmi_info("out_csc_quantization change, old(%u)->new(%u)\n",
            hw_app_attr->out_csc_quantization, sw_app_attr->out_csc_quantization);
        return HI_TRUE;
    }

    return HI_FALSE;
}

static hi_bool vo_attr_is_changed(const hdmi_vo_attr *hw_vo_attr, hdmi_vo_attr *sw_vo_attr)
{
    hdmi_if_null_return(hw_vo_attr, HI_FALSE);
    hdmi_if_null_return(sw_vo_attr, HI_FALSE);

    if (hw_vo_attr->video_timing != sw_vo_attr->video_timing) {
        hdmi_info("video timing change,old(%u)->new(%u)\n", hw_vo_attr->video_timing, sw_vo_attr->video_timing);
        return HI_TRUE;
    } else {
        /*
         * if timing is change, the sync param need according to user config, or not,
         * should not change the sync param.
         */
        sw_vo_attr->v_sync_pol = hw_vo_attr->v_sync_pol;
        sw_vo_attr->h_sync_pol = hw_vo_attr->h_sync_pol;
        sw_vo_attr->de_pol     = hw_vo_attr->de_pol;
    }

    if (hw_vo_attr->in_color_space != sw_vo_attr->in_color_space) {
        hdmi_info("input colorspace change, old(%u)->new(%u)\n",
            hw_vo_attr->in_color_space, sw_vo_attr->in_color_space);
        return HI_TRUE;
    }
    if (hw_vo_attr->stereo_mode != sw_vo_attr->stereo_mode) {
        hdmi_info("3d mode change, old(%u)->new(%u)\n", hw_vo_attr->stereo_mode, sw_vo_attr->stereo_mode);
        return HI_TRUE;
    }
    if (hw_vo_attr->pixel_repeat != sw_vo_attr->pixel_repeat) {
        hdmi_info("pixel repeation change, old(%u)->new(%u)\n", hw_vo_attr->pixel_repeat, sw_vo_attr->pixel_repeat);
        return HI_TRUE;
    }
    if (hw_vo_attr->colorimetry != sw_vo_attr->colorimetry) {
        hdmi_info("colorimetry change, old(%u)->new(%u)\n", hw_vo_attr->colorimetry, sw_vo_attr->colorimetry);
        return HI_TRUE;
    }
    if (hw_vo_attr->extended_colorimetry != sw_vo_attr->extended_colorimetry) {
        hdmi_info("extended_colorimetry change, old(%u)->new(%u)\n",
            hw_vo_attr->extended_colorimetry, sw_vo_attr->extended_colorimetry);
        return HI_TRUE;
    }
    if (hw_vo_attr->rgb_quantization != sw_vo_attr->rgb_quantization) {
        hdmi_info("RGB quantization change, old(%u)->new(%u)\n",
            hw_vo_attr->rgb_quantization, sw_vo_attr->rgb_quantization);
        return HI_TRUE;
    }
    if (hw_vo_attr->ycc_quantization != sw_vo_attr->ycc_quantization) {
        hdmi_info("YCC quantization change, old(%u)->new(%u)\n",
            hw_vo_attr->ycc_quantization, sw_vo_attr->ycc_quantization);
        return HI_TRUE;
    }
    if (hw_vo_attr->picture_aspect != sw_vo_attr->picture_aspect) {
        hdmi_info("picture aspect change, old(%u)->new(%u)\n", hw_vo_attr->picture_aspect, sw_vo_attr->picture_aspect);
        return HI_TRUE;
    }

    return HI_FALSE;
}

static hi_s32 check_app_attr(const hdmi_app_attr *app_attr)
{
    hi_s32 ret = HI_SUCCESS;

    if (app_attr->out_color_space != HDMI_COLORSPACE_RGB &&
        app_attr->out_color_space != HDMI_COLORSPACE_YCBCR422 &&
        app_attr->out_color_space != HDMI_COLORSPACE_YCBCR444 &&
        app_attr->out_color_space != HDMI_COLORSPACE_YCBCR420) {
        hdmi_err("out_color_space=%u is wrong\n", app_attr->out_color_space);
        ret += HI_FAILURE;
    }

    if (app_attr->deep_color_mode != HDMI_DEEP_COLOR_24BIT &&
        app_attr->deep_color_mode != HDMI_DEEP_COLOR_30BIT &&
        app_attr->deep_color_mode != HDMI_DEEP_COLOR_36BIT &&
        app_attr->deep_color_mode != HDMI_DEEP_COLOR_48BIT &&
        app_attr->deep_color_mode != HDMI_DEEP_COLOR_OFF) {
        hdmi_err("deep_clr_mode=%u is wrong\n", app_attr->deep_color_mode);
        ret += HI_FAILURE;
    }

    if (app_attr->hdcp_mode != HDMI_HDCP_MODE_1_4 &&
        app_attr->hdcp_mode != HDMI_HDCP_MODE_2_2 &&
        app_attr->hdcp_mode != HDMI_HDCP_MODE_AUTO) {
        hdmi_err("hdcp_mode=%u is wrong\n", app_attr->hdcp_mode);
        ret += HI_FAILURE;
    }

    return (ret != HI_SUCCESS) ? HI_FAILURE : HI_SUCCESS;
}

#ifdef HDMI_HDR_SUPPORT
static hi_void colormetry_hdr2vo(hdmi_hdr_colormetry hdr_colorimetry, hdmi_colormetry *vo_colorimetry)
{
    switch (hdr_colorimetry) {
        case HDMI_HDR_COLORIMETRY_NONE:
            *vo_colorimetry = HDMI_COLORIMETRY_NONE;
            break;
        case HDMI_HDR_COLORIMETRY_ITU_601:
            *vo_colorimetry = HDMI_COLORIMETRY_ITU_601;
            break;
        case HDMI_HDR_COLORIMETRY_ITU_709:
            *vo_colorimetry = HDMI_COLORIMETRY_ITU_709;
            break;
        case HDMI_HDR_COLORIMETRY_EXTENDED:
            *vo_colorimetry = HDMI_COLORIMETRY_EXTEND;
            break;
        default:
            break;
    }

    return;
}

static hi_void hdr_mode_check(hdmi_device *hdmi_dev, hdmi_vo_attr *vo_attr, hdmi_app_attr *app_attr)
{
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;

    hdmi_if_null_return_void(hdmi_dev);
    hdmi_if_null_return_void(vo_attr);

    hdr_attr = &(hdmi_dev->attr.hdr_attr);
    switch (hdr_attr->user_hdr_mode) {
        case HDMI_HDR_USERMODE_DOLBY:
            vo_attr->ycc_quantization   = HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
            app_attr->out_color_space   = HDMI_COLORSPACE_YCBCR444;
            vo_attr->hdmi_adapt_pix_clk = vo_attr->clk_fs;
            app_attr->deep_color_mode   = HDMI_DEEP_COLOR_24BIT;
            vo_attr->rgb_quantization   = HDMI_QUANTIZATION_RANGE_DEFAULT;
            break;
        case HDMI_HDR_USERMODE_HDR10:
            if (hdr_attr->hdr_mode == HDMI_HDR_MODE_CEA_861_3) {
                if (hdr_attr->colorimetry > HDMI_HDR_COLORIMETRY_EXTENDED) {
                    vo_attr->colorimetry = HDMI_COLORIMETRY_EXTEND;
                    vo_attr->extended_colorimetry =
                        hdr_attr->colorimetry - HDMI_HDR_EXTENDED_COLORIMETRY_XV_YCC_601;
                } else {
                    colormetry_hdr2vo(hdr_attr->colorimetry, &vo_attr->colorimetry);
                    vo_attr->extended_colorimetry = HDMI_EXTENDED_COLORIMETRY_BUTT;
                }
                app_attr->xvycc_mode = HI_FALSE;
            }
            break;
        default:
            break;
    }
    hdmi_info("user hdr mode:%u, actual hdr mode:%u \n", hdr_attr->user_hdr_mode, hdr_attr->hdr_mode);

    return;
}
#endif

static hi_void drv_hdmi_vou_dither_set(const hdmi_app_attr *app_attr)
{
#ifdef HDMI_CHIP_HI3559AV100
    vou_export_func *vo_export_func = HI_NULL;
    vo_dither_out_bitwidth vou_dither_mode;

    vo_export_func = func_entry(vou_export_func, HI_ID_VO);
    if (vo_export_func != HI_NULL && vo_export_func->pfn_vou_set_dither_out_bit_width != HI_NULL) {
        if (app_attr->out_color_space == HDMI_COLORSPACE_YCBCR422) {
            /* 422 must disable dither */
            vou_dither_mode = DITHER_OUT_BITWIDTH_10;
        } else {
            vou_dither_mode = DITHER_OUT_BITWIDTH_10;
            if (app_attr->deep_color_mode == HDMI_DEEP_COLOR_24BIT ||
                app_attr->deep_color_mode == HDMI_DEEP_COLOR_OFF) {
                vou_dither_mode = DITHER_OUT_BITWIDTH_8;
            }
        }
        vo_export_func->pfn_vou_set_dither_out_bit_width(HI_ID_HDMI, 0, vou_dither_mode);
        hdmi_info("set VOU dither mode(%u)\n", vou_dither_mode);
    } else {
        hdmi_err("vo_export_func is NULL, VOU dither set error\n");
    }
#else
    hi_unused(app_attr);
#endif

    return;
}

static hi_bool hdmi_mode_change(const hdmi_device *hdmi_dev)
{
#ifdef HDMI_FRL_SUPPORT
    static hdmi_frl_mode pre_mode = HDMI_FRL_MODE_TMDS;

    if (pre_mode != hdmi_dev->frl_info.mode) {
        hdmi_info("hdmi mode change, %u to %u\n", pre_mode, hdmi_dev->frl_info.mode);
        pre_mode = hdmi_dev->frl_info.mode;
        return HI_TRUE;
    }
#else
    hi_unused(hdmi_dev);
#endif

    return HI_FALSE;
}

static hi_void hdmi_vo_attr_info(const hdmi_device *hdmi_dev, const hdmi_vo_attr *video_attr)
{
    hdmi_info("clk_fs: %u\n", video_attr->clk_fs);
    hdmi_info("pixel_repeat: %u\n", video_attr->pixel_repeat);
    hdmi_info("video_timing: %u\n", video_attr->video_timing);
    hdmi_info("stereo_mode: %u\n", video_attr->stereo_mode);
    hdmi_info("in_color_space: %u\n", video_attr->in_color_space);
    hdmi_info("colorimetry: %u\n", video_attr->colorimetry);
    hdmi_info("extended_colorimetry: %u\n", video_attr->extended_colorimetry);
    hdmi_info("rgb_quantization: %u\n", video_attr->rgb_quantization);
    hdmi_info("ycc_quantization: %u\n", video_attr->ycc_quantization);
    hdmi_info("picture_aspect: %u\n", video_attr->picture_aspect);
    hdmi_info("active_aspect: %u\n", video_attr->active_aspect);
    hdmi_info("picture_scaling: %u\n", video_attr->picture_scaling);
    hdmi_info("in_bit_depth: %u\n", video_attr->in_bit_depth);
    hdmi_info("disp_fmt: %u\n", video_attr->disp_fmt);
    hdmi_info("v_sync_pol: %u\n", video_attr->v_sync_pol);
    hdmi_info("h_sync_pol: %u\n", video_attr->h_sync_pol);
    hdmi_info("de_pol: %u\n", video_attr->de_pol);
#ifdef HDMI_HDR_SUPPORT
    hdmi_info("user_hdr_mode: %u\n", hdmi_dev->attr.hdr_attr.user_hdr_mode);
#else
    hi_unused(hdmi_dev);
#endif

    return;
}

static hi_s32 hdmi_format_is_change(hdmi_device *hdmi_dev, const hdmi_vo_attr *video_attr)
{
    errno_t ret;
    hdmi_app_attr           tmp_app                     = {0};
    hdmi_app_attr          *app_attr                    = HI_NULL;
    hi_bool                 enable_clr_space_adapt_back = HI_TRUE;
    hi_bool                 enable_deep_clr_adapt_back  = HI_TRUE;
    static hdmi_disp_format disp_fmt                    = HDMI_VIDEO_FORMAT_BUTT;

    app_attr = &hdmi_dev->attr.app_attr;
    enable_clr_space_adapt_back = app_attr->enable_clr_space_adapt;
    enable_deep_clr_adapt_back = app_attr->enable_deep_clr_adapt;
    ret = memcpy_s(&tmp_app, sizeof(tmp_app), app_attr, sizeof(*app_attr));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    /* when format is changed, force to enable  hdmi_color_and_bit_strategy */
    if (disp_fmt != video_attr->disp_fmt) {
        tmp_app.enable_clr_space_adapt = HI_TRUE;
        tmp_app.enable_deep_clr_adapt = HI_TRUE;
        disp_fmt = video_attr->disp_fmt;
    }
    if (hdmi_color_and_bit_strategy(hdmi_dev, &tmp_app) != HI_SUCCESS) {
        hdmi_err(" hdmi_color_and_bit_strategy fail\n");
        return HI_ERR_HDMI_STRATEGY_FAILED;
    }

    ret = memcpy_s(app_attr, sizeof(hdmi_dev->attr.app_attr), &tmp_app, sizeof(tmp_app));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    app_attr->enable_clr_space_adapt = enable_clr_space_adapt_back;
    app_attr->enable_deep_clr_adapt  = enable_deep_clr_adapt_back;

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_vo_attr_set(hdmi_device *hdmi_dev, const hdmi_vo_attr *vo_attr)
{
    errno_t ret;
    hdmi_attr hw_attr = {0};
    hdmi_vo_attr *video_attr = HI_NULL;
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(vo_attr, HI_ERR_NULL_PTR);

    if (check_video_attr(vo_attr) != HI_SUCCESS) {
        hdmi_err("check_video_attr fail\n");
        return HI_ERR_HDMI_INVALID_PARA;
    }
    video_attr = &hdmi_dev->attr.vo_attr;
    app_attr   = &hdmi_dev->attr.app_attr;
    ret = memcpy_s(video_attr, sizeof(*video_attr), vo_attr, sizeof(hdmi_vo_attr));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    hdmi_vo_attr_info(hdmi_dev, video_attr);
    if (hdmi_format_is_change(hdmi_dev, video_attr) != HI_SUCCESS) {
        return HI_ERR_HDMI_STRATEGY_FAILED;
    }
#ifdef HDMI_HDR_SUPPORT
    hdmi_dev->hdr_mutex.attr_set = HI_TRUE;
    hdr_mode_check(hdmi_dev, video_attr, app_attr);
#endif
#ifdef HDMI_FRL_SUPPORT
    ret = drv_hdmi_frl_mode_strategy(hdmi_dev->hdmi_dev_id);
    if (ret == HDMI_EDID_DATA_INVALID) {
        return HI_SUCCESS;
    } else if (ret != HI_SUCCESS) {
        hdmi_err("FRL strategy fail\n");
        return HI_ERR_HDMI_FEATURE_NO_SUPPORT;
    }
#endif
    hdmi_attr_construct(hdmi_dev, &hw_attr);
    if (!vo_attr_is_changed(&hw_attr.vo_attr, video_attr) &&
        !app_attr_is_changed(&hw_attr.app_attr, app_attr) && hdmi_mode_change(hdmi_dev) == HI_FALSE) {
        hdmi_info("the video and app attr is not changed\n");
        return HI_SUCCESS;
    }
    if (app_attr->enable_video) {
        hdmi_video_path_set(hdmi_dev, video_attr);
    }
    /*
     * HDMI is not support dither on hi3559_av100.
     * set output bit to VO. VO decide to whether enable dither or not.
     */
    drv_hdmi_vou_dither_set(app_attr);
    drv_hdmi_avi_infoframe_send(&hdmi_dev->info_frame, (app_attr->enable_hdmi && app_attr->enable_avi_infoframe));
    drv_hdmi_vendor_infoframe_send(&hdmi_dev->info_frame, app_attr->enable_hdmi, app_attr->enable_vsif_dolby);
#ifdef HDMI_HDR_SUPPORT
    /* the drm infoframe is stop because software was reset in videopath, so re-enable it if the mode is HDR10. */
    if (hdmi_dev->attr.hdr_attr.hdr_mode == HDMI_HDR_MODE_CEA_861_3) {
        drv_hdmi_drm_infoframe_send(&hdmi_dev->info_frame, HI_FALSE);
        drv_hdmi_drm_infoframe_send(&hdmi_dev->info_frame, HI_TRUE);
    }
#endif

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_vo_attr_get(hdmi_device *hdmi_dev, hdmi_vo_attr *vo_attr)
{
    errno_t ret;
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(vo_attr, HI_ERR_NULL_PTR);

    ret = memcpy_s(vo_attr, sizeof(*vo_attr), &hdmi_dev->attr.vo_attr, sizeof(hdmi_vo_attr));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

#ifdef HDMI_HDR_SUPPORT
hi_s32 hdmi_hdr_attr_lock(hdmi_device *hdmi_dev)
{
    hi_s32 ret = HI_FAILURE;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    osal_spin_lock(&hdmi_dev->hdr_mutex.attr_spin);
    if ((hdmi_dev->hdr_mutex.attr_busy == HI_TRUE) || (hdmi_dev->hdr_mutex.attr_set == HI_FALSE)) {
        osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);
        hdmi_warn("attr busy(%u), attr set(%u)!\n", hdmi_dev->hdr_mutex.attr_busy, hdmi_dev->hdr_mutex.attr_set);
        goto exit;
    }

    hdmi_dev->hdr_mutex.attr_busy = HI_TRUE;
    osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);
    ret = HI_SUCCESS;

exit:
    return ret;
}

hi_s32 hdmi_attr_un_lock(hdmi_device *hdmi_dev)
{
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    osal_spin_lock(&hdmi_dev->hdr_mutex.attr_spin);
    hdmi_dev->hdr_mutex.attr_busy = HI_FALSE;
    osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);

    return HI_SUCCESS;
}

hi_s32 hdmi_attr_lock(hdmi_device *hdmi_dev)
{
    hi_u32 timeout = ATTR_LOCK_WAIT_TIMEOUT;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    osal_spin_lock(&hdmi_dev->hdr_mutex.attr_spin);
    if (hdmi_dev->hdr_mutex.attr_busy == HI_FALSE) {
        hdmi_dev->hdr_mutex.attr_busy = HI_TRUE;
        osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);
        goto exit;
    }
    osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);
    while (timeout-- > 0) {
        osal_msleep(ATTR_LOCK_WAIT_TIME);
        osal_spin_lock(&hdmi_dev->hdr_mutex.attr_spin);
        if (hdmi_dev->hdr_mutex.attr_busy == HI_FALSE) {
            hdmi_dev->hdr_mutex.attr_busy = HI_TRUE;
            osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);
            goto exit;
        }
        osal_spin_unlock(&hdmi_dev->hdr_mutex.attr_spin);
    }
    hdmi_dev->hdr_mutex.attr_busy = HI_TRUE;
    hdmi_warn("time out(%u), force set attr!\n", timeout);

exit:
    return HI_SUCCESS;
}

static hi_bool hdr_attr_is_change(const drv_hdmi_hdr *app_hdr_attr, const drv_hdmi_hdr_attr *hdr_attr)
{
    hdmi_check_is_change_return(hdr_attr->hdr_mode, app_hdr_attr->hdr_attr.hdr_mode, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->eotf_type, app_hdr_attr->hdr_attr.eotf_type, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->metadata_id, app_hdr_attr->hdr_attr.metadata_id, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->colorimetry, app_hdr_attr->hdr_attr.colorimetry, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.primaries0_x,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.primaries0_x, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.primaries0_y,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.primaries0_y, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.primaries1_x,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.primaries1_x, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.primaries1_y,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.primaries1_y, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.primaries2_x,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.primaries2_x, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.primaries2_y,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.primaries2_y, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.max_luminance,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.max_luminance, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.min_luminance,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.min_luminance, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.max_light_level,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.max_light_level, HI_TRUE);
    hdmi_check_is_change_return(hdr_attr->un_descriptor.type1.average_light_level,
                                app_hdr_attr->hdr_attr.un_descriptor.type1.average_light_level, HI_TRUE);

    return HI_FALSE;
}

static hi_s32 hdr_attr_check(const drv_hdmi_hdr *app_hdr_attr)
{
    hdmi_if_null_return(app_hdr_attr, HI_ERR_NULL_PTR);

    if (app_hdr_attr->hdr_attr.hdr_mode >= HDMI_HDR_MODE_BUTT) {
        hdmi_err("hdmi hdr hdr_mode(%u) is wrong\n", app_hdr_attr->hdr_attr.hdr_mode);
        return HI_FAILURE;
    }
    if (app_hdr_attr->hdr_attr.eotf_type >= HDMI_EOTF_BUTT) {
        hdmi_err("hdmi hdr eotf_type(%u) is wrong\n", app_hdr_attr->hdr_attr.eotf_type);
        return HI_FAILURE;
    }
    if (app_hdr_attr->hdr_attr.metadata_id >= HDMI_HDR_METADATA_ID_BUTT) {
        hdmi_err("hdmi hdr metadata_id(%u) is wrong\n", app_hdr_attr->hdr_attr.metadata_id);
        return HI_FAILURE;
    }
    if (app_hdr_attr->hdr_attr.colorimetry > HDMI_HDR_EXTENDED_COLORIMETRY_2020_NON_CONST_LUMINOUW) {
        hdmi_err("hdmi hdr colorimetry(%u) is wrong\n", app_hdr_attr->hdr_attr.colorimetry);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void hdmi_zero_drm_infoframe_stop(hdmi_device *hdmi_dev)
{
    if (hdmi_dev->attr.hdr_attr.hdr_mode != HDMI_HDR_MODE_CEA_861_3) {
        drv_hdmi_drm_infoframe_send(&hdmi_dev->info_frame, HI_FALSE);
        hdmi_info("zero DRM infoframe send timeout.\n");
    }
    return;
}

static hi_s32 hdmi_hdr_mode_change_timeout(const hdmi_device *hdmi_dev)
{
    hi_s32 ret = HI_SUCCESS;

    if (hdmi_dev->hdr_info.hdr_debug_mode == HDMI_HDR_DEBUG_MODE_OE) {
        hdmi_phy_output_enable(hdmi_dev, HI_TRUE);
    }
    hal_call_void(hal_hdmi_avmute_set, hdmi_dev->hal, HI_FALSE);
    if (hdmi_dev->hdr_info.hdr_debug_mode == HDMI_HDR_DEBUG_MODE_OE) {
        drv_hdmi_black_data_set(hdmi_dev, HI_FALSE);
        ret = drv_hdmi_audio_output_enable_set(hdmi_dev, HI_TRUE);
    }
    hdmi_info("HDR mode change timeout, debug mode: %u.\n", hdmi_dev->hdr_info.hdr_debug_mode);

    return ret;
}

static hi_void hdmi_zero_drm_infoframe_timer_set(hdmi_device *hdmi_dev, hdmi_timer_config *timer)
{
    hdmi_attr *attr = HI_NULL;
    hdmi_timer_config timer_cfg = {0};

    hdmi_if_null_return_void(hdmi_dev);

    attr = &hdmi_dev->attr;
    timer_cfg.timer_type  = HDMI_TIMER_ZERO_DRMIF;
    timer_cfg.timer_start = HI_FALSE;
    timer_cfg.time        = ZERO_DRMIF_SEND_TIME;
    hal_call_void(hal_hdmi_hdr_timer_set, hdmi_dev->hal, &timer_cfg);
    if (timer->timer_start == HI_FALSE) {
        return;
    }
    attr->hdr_attr.eotf_type   = HDMI_EOTF_SDR_LUMIN;
    attr->hdr_attr.metadata_id = HDMI_HDR_METADATA_ID_0;
    (hi_void)memset_s(&(attr->hdr_attr.un_descriptor), sizeof(attr->hdr_attr.un_descriptor),
        0, sizeof(hdmi_meta_descriptor));
    drv_hdmi_drm_infoframe_send(&hdmi_dev->info_frame, HI_TRUE);

    timer->timer_type = HDMI_TIMER_ZERO_DRMIF;
    hal_call_void(hal_hdmi_hdr_timer_set, hdmi_dev->hal, timer);

    return;
}

static hi_void hdmi_hdr_mode_change_timer_set(const hdmi_device *hdmi_dev, hdmi_timer_config *timer)
{
    hdmi_timer_config timer_cfg = {0};

    hdmi_if_null_return_void(hdmi_dev);

    timer_cfg.timer_type  = HDMI_TIMER_SDR_TO_HDR10;
    timer_cfg.time        = HDRMODE_CHANGE_TIME;
    timer_cfg.timer_start = HI_FALSE;

    /* stop timer first. */
    hal_call_void(hal_hdmi_hdr_timer_set, hdmi_dev->hal, &timer_cfg);
    if (timer->timer_start == HI_FALSE) {
        return;
    }
    timer->timer_type = HDMI_TIMER_SDR_TO_HDR10;
    hal_call_void(hal_hdmi_hdr_timer_set, hdmi_dev->hal, timer);

    return;
}

static hi_void hdmi_timer_set(hdmi_device *hdmi_dev, hdmi_timer_config *timer)
{
    hdmi_if_null_return_void(hdmi_dev);

    switch (timer->timer_type) {
        case HDMI_TIMER_ZERO_DRMIF:
            hdmi_zero_drm_infoframe_timer_set(hdmi_dev, timer);
            break;
        case HDMI_TIMER_SDR_TO_HDR10:
            hdmi_hdr_mode_change_timer_set(hdmi_dev, timer);
            break;
        default:
            break;
    }

    return;
}

static hi_void hdr_colorimetry_conv(hdmi_hdr_colormetry colorimetry, hdmi_vo_attr *vo_attr)
{
    if (colorimetry >= HDMI_HDR_COLORIMETRY_EXTENDED) {
        vo_attr->colorimetry = HDMI_COLORIMETRY_EXTEND;
        vo_attr->extended_colorimetry = colorimetry - HDMI_HDR_EXTENDED_COLORIMETRY_XV_YCC_601;
    } else {
        switch (colorimetry) {
            case HDMI_HDR_COLORIMETRY_NONE:
                vo_attr->colorimetry = HDMI_COLORIMETRY_NONE;
                break;
            case HDMI_HDR_COLORIMETRY_ITU_601:
                vo_attr->colorimetry = HDMI_COLORIMETRY_ITU_601;
                break;
            case HDMI_HDR_COLORIMETRY_ITU_709:
                vo_attr->colorimetry = HDMI_COLORIMETRY_ITU_709;
                break;
            default:
                break;
        }
        vo_attr->extended_colorimetry = HDMI_EXTENDED_COLORIMETRY_XV_YCC_601;
    }

    return;
}

static hi_s32 disable_hdr(hdmi_device *hdmi_dev, const drv_hdmi_hdr *app_hdr_attr)
{
    hi_s32             ret      = HI_SUCCESS;
    hdmi_attr         *attr     = HI_NULL;
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;
    drv_hdmi_hdr_info *hdr_info = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(app_hdr_attr, HI_ERR_NULL_PTR);

    attr     = &(hdmi_dev->attr);
    hdr_attr = &(attr->hdr_attr);
    hdr_info = &(hdmi_dev->hdr_info);

    switch (hdr_attr->hdr_mode) {
        case HDMI_HDR_MODE_DISABLE:
            break;
        case HDMI_HDR_MODE_CEA_861_3:
            ret = memcpy_s(&attr->hdr_attr, sizeof(attr->hdr_attr),
                &(app_hdr_attr->hdr_attr), sizeof(drv_hdmi_hdr_attr));
            hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
            if (hdr_info->hdr_debug_mode != HDMI_HDR_DEBUG_MODE_NONE) {
                /* to solve the problem of some TV can not switch smoothly from HDR10 to non-HDR10. */
                if (hdr_info->hdr_debug_mode == HDMI_HDR_DEBUG_MODE_OE) {
                    drv_hdmi_audio_output_enable_set(hdmi_dev, HI_FALSE);
                    drv_hdmi_black_data_set(hdmi_dev, HI_TRUE);
                }
                hal_call_void(hal_hdmi_avmute_set, hdmi_dev->hal, HI_TRUE);

                if (hdr_info->hdr_debug_mode == HDMI_HDR_DEBUG_MODE_OE) {
                    hdmi_phy_output_enable(hdmi_dev, HI_FALSE);
                }
                hdr_info->hdr_mode_chn_timer.timer_start = HI_TRUE;
                hdmi_timer_set(hdmi_dev, &(hdr_info->hdr_mode_chn_timer));
                drv_hdmi_drm_infoframe_send(&hdmi_dev->info_frame, HI_FALSE);
            } else {
                /* start timer, send zero DRMIF and stop after 2 seconds. */
                hdr_info->zero_drm_if_timer.timer_start = HI_TRUE;
                hdmi_timer_set(hdmi_dev, &(hdr_info->zero_drm_if_timer));
            }
            break;
        case HDMI_HDR_MODE_CEA_861_3_AUTHEN:
        case HDMI_HDR_MODE_DOLBY_NORMAL:
        case HDMI_HDR_MODE_DOLBY_TUNNELING:
            if (hdr_attr->hdr_mode != HDMI_HDR_MODE_DOLBY_NORMAL) {
                hdmi_dev->attr.app_attr.out_color_space = HDMI_COLORSPACE_YCBCR444;
            }
            attr->vo_attr.rgb_quantization = HDMI_QUANTIZATION_RANGE_DEFAULT;
            attr->vo_attr.ycc_quantization = HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
            break;
        default:
            ret = HI_FAILURE;
            break;
    }

    return ret;
}

hi_s32 change_to_hdr(hdmi_device *hdmi_dev, const drv_hdmi_hdr *app_hdr_attr)
{
    hi_s32             ret      = HI_SUCCESS;
    hdmi_attr         *attr     = HI_NULL;
    hdmi_app_attr     *app_attr = HI_NULL;
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;
    drv_hdmi_hdr_info *hdr_info = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(app_hdr_attr, HI_ERR_NULL_PTR);

    attr     = &(hdmi_dev->attr);
    hdr_attr = &(attr->hdr_attr);
    app_attr = &(hdmi_dev->attr.app_attr);
    hdr_info = &(hdmi_dev->hdr_info);

    hdr_info->zero_drm_if_timer.timer_start = HI_FALSE;
    hdmi_zero_drm_infoframe_timer_set(hdmi_dev, &(hdr_info->zero_drm_if_timer));
    if (hdr_attr->user_hdr_mode != HDMI_HDR_USERMODE_HDR10) {
        hdmi_err("usermode is not HDR10/HLG.\n");
        return HI_FAILURE;
    }
    app_attr->enable_drm_infoframe = HI_TRUE;

    if (hdr_info->hdr_debug_mode != HDMI_HDR_DEBUG_MODE_NONE) {
        /* to solve the problem of some TV can not switch smoothly from non-HDR10 to HDR10. */
        if (hdr_info->hdr_debug_mode == HDMI_HDR_DEBUG_MODE_OE) {
            ret = drv_hdmi_audio_output_enable_set(hdmi_dev, HI_FALSE);
            drv_hdmi_black_data_set(hdmi_dev, HI_TRUE);
        }
        hal_call_void(hal_hdmi_avmute_set, hdmi_dev->hal, HI_TRUE);
        if (hdr_info->hdr_debug_mode == HDMI_HDR_DEBUG_MODE_OE) {
            hdmi_phy_output_enable(hdmi_dev, HI_FALSE);
        }
        hdr_info->hdr_mode_chn_timer.timer_start = HI_TRUE;
        hdmi_timer_set(hdmi_dev, &(hdr_info->hdr_mode_chn_timer));
    }

    return ret;
}

static hi_s32 hdmi_hdr_strategy(hdmi_device *hdmi_dev, const drv_hdmi_hdr *app_hdr_attr)
{
    hdmi_attr         *attr     = HI_NULL;
    hdmi_app_attr     *app_attr = HI_NULL;
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;

    attr     = &hdmi_dev->attr;
    hdr_attr = &attr->hdr_attr;
    app_attr = &attr->app_attr;

    switch (app_hdr_attr->hdr_attr.hdr_mode) {
        case HDMI_HDR_MODE_CEA_861_3:
            change_to_hdr(hdmi_dev, app_hdr_attr);
            app_attr->enable_vsif_dolby = HI_FALSE;
            hdmi_info("HDR10 mode, colorimetry(%u), extended_colorimetry(%u)\n",
                      attr->vo_attr.colorimetry, attr->vo_attr.extended_colorimetry);
            break;
        case HDMI_HDR_MODE_CEA_861_3_AUTHEN:
            /* HDR10 authen mode, HDMI must bypass. */
            if (hdr_attr->user_hdr_mode != HDMI_HDR_USERMODE_DOLBY) {
                hdmi_err("usermode is not dolby.\n");
                return HI_FAILURE;
            }
            app_attr->enable_drm_infoframe = HI_TRUE;
            app_attr->enable_vsif_dolby = HI_FALSE;
            hdmi_info("hdr authen mode is enable! \n");
            break;
        case HDMI_HDR_MODE_DOLBY_NORMAL:
        case HDMI_HDR_MODE_DOLBY_TUNNELING:
            if (hdr_attr->user_hdr_mode != HDMI_HDR_USERMODE_DOLBY) {
                hdmi_err("usermode is not dolby.\n");
                return HI_FAILURE;
            }
            app_attr->enable_drm_infoframe = HI_FALSE;
            app_attr->enable_vsif_dolby = HI_TRUE;
            hdmi_info("dolby mode(%u) is enable! \n", app_hdr_attr->hdr_attr.hdr_mode);
            break;
        default:
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_s32 drv_hdmi_hdr_attr_set(hdmi_device *hdmi_dev, const drv_hdmi_hdr *app_hdr_attr)
{
    errno_t errnumber;
    hi_s32 ret = HI_FAILURE;
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);

    app_attr = &hdmi_dev->attr.app_attr;

    if (hdr_attr_check(app_hdr_attr) != HI_SUCCESS) {
        hdmi_err("app_hdr_attr check failed!\n");
        return HI_FAILURE;
    }
    if (hdr_attr_is_change(app_hdr_attr, &hdmi_dev->attr.hdr_attr) == HI_FALSE) {
        hdmi_info("hdr attr not change, return.\n");
        return HI_SUCCESS;
    }

    if (hdmi_hdr_attr_lock(hdmi_dev) == HI_FAILURE) {
        goto exit;
    }

    if (app_hdr_attr->hdr_attr.colorimetry != hdmi_dev->attr.hdr_attr.colorimetry) {
        hdr_colorimetry_conv(app_hdr_attr->hdr_attr.colorimetry, &hdmi_dev->attr.vo_attr);
        hdmi_info("change colorimetry = %u, ext_colorimety = %u\n",
            hdmi_dev->attr.vo_attr.colorimetry, hdmi_dev->attr.vo_attr.extended_colorimetry);
    }
    /* change to SDR */
    if (app_hdr_attr->hdr_attr.hdr_mode == HDMI_HDR_MODE_DISABLE) {
        if (disable_hdr(hdmi_dev, app_hdr_attr) == HI_SUCCESS) {
            app_attr->enable_vsif_dolby = HI_FALSE;
            errnumber = memcpy_s(&hdmi_dev->attr.hdr_attr, sizeof(hdmi_dev->attr.hdr_attr),
                &(app_hdr_attr->hdr_attr), sizeof(drv_hdmi_hdr_attr));
            hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
            goto send_inforame;
        } else {
            goto exit;
        }
    }
    /* change to HDR */
    errnumber = memcpy_s(&hdmi_dev->attr.hdr_attr, sizeof(hdmi_dev->attr.hdr_attr),
        &(app_hdr_attr->hdr_attr), sizeof(drv_hdmi_hdr_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    ret = hdmi_hdr_strategy(hdmi_dev, app_hdr_attr);
    if (ret == HI_FAILURE) {
        goto exit;
    }
    drv_hdmi_drm_infoframe_send(&hdmi_dev->info_frame, app_attr->enable_hdmi && app_attr->enable_drm_infoframe);

send_inforame:
    drv_hdmi_avi_infoframe_send(&hdmi_dev->info_frame, (app_attr->enable_hdmi && app_attr->enable_avi_infoframe));
    drv_hdmi_vendor_infoframe_send(&hdmi_dev->info_frame, app_attr->enable_hdmi, app_attr->enable_vsif_dolby);
    hdmi_csc_set(hdmi_dev, &hdmi_dev->attr.vo_attr);

exit:
    hdmi_attr_un_lock(hdmi_dev);

    return ret;
}
#endif

hi_s32 drv_hdmi_attr_get(const hdmi_device *hdmi_dev, hdmi_attr *attr)
{
    errno_t errnumber;
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(attr, HI_ERR_NULL_PTR);

    errnumber = memcpy_s(attr, sizeof(*attr), &hdmi_dev->attr, sizeof(hdmi_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_attr_set(hdmi_device *hdmi_dev, hdmi_attr *attr)
{
    errno_t errnumber;
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);
    hdmi_if_null_return(attr, HI_ERR_NULL_PTR);

    app_attr = &hdmi_dev->attr.app_attr;
    hdmi_info("enable_hdmi: %u\n", attr->app_attr.enable_hdmi);
    hdmi_info("enable_video: %u\n", attr->app_attr.enable_video);
    hdmi_info("enable_audio: %u\n", attr->app_attr.enable_audio);
    hdmi_info("out_color_space: %u\n", attr->app_attr.out_color_space);
    hdmi_info("deep_color_mode: %u\n", attr->app_attr.deep_color_mode);
    hdmi_info("bxv_ycc_mode: %u\n", attr->app_attr.xvycc_mode);
    hdmi_info("enable_avi_info_frame: %u\n", attr->app_attr.enable_avi_infoframe);
    hdmi_info("enable_spd_info_frame: %u\n", attr->app_attr.enable_spd_infoframe);
    hdmi_info("enable_mpeg_info_frame: %u\n", attr->app_attr.enable_mpeg_infoframe);
    hdmi_info("enable_aud_info_frame: %u\n", attr->app_attr.enable_aud_infoframe);
    hdmi_info("debug_flag: %u\n", attr->app_attr.debug_flag);
    hdmi_info("hdcp_enable: %u\n", attr->app_attr.hdcp_enable);
    hdmi_info("hdcp_mode: %u\n", attr->app_attr.hdcp_mode);
    hdmi_info("enable_clr_space_adapt: %u\n", attr->app_attr.enable_clr_space_adapt);
    hdmi_info("enable_deep_clr_adapt: %u\n", attr->app_attr.enable_deep_clr_adapt);
    hdmi_info("auth_mode: %u\n", attr->app_attr.auth_mode);
    hdmi_info("out_csc_quantization: %u\n", attr->app_attr.out_csc_quantization);

    if (check_app_attr(&attr->app_attr) != HI_SUCCESS) {
        hdmi_err("check hdmi attr fail\n");
        return HI_ERR_HDMI_INVALID_PARA;
    }
    if (hdmi_color_and_bit_strategy(hdmi_dev, &attr->app_attr) != HI_SUCCESS) {
        hdmi_err(" hdmi_color_and_bit_strategy fail\n");
        return HI_ERR_HDMI_STRATEGY_FAILED;
    }
    errnumber = memcpy_s(app_attr, sizeof(*app_attr), &attr->app_attr, sizeof(hdmi_app_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    if (drv_hdmi_ao_attr_set(hdmi_dev, &attr->ao_attr) != HI_SUCCESS) {
        hdmi_err("drv_hdmi_ao_attr_set fail\n");
        return HI_ERR_HDMI_SET_ATTR_FAILED;
    }
    if (drv_hdmi_vo_attr_set(hdmi_dev, &attr->vo_attr) != HI_SUCCESS) {
        hdmi_err("drv_hdmi_vo_attr_set fail\n");
        return HI_ERR_HDMI_SET_ATTR_FAILED;
    }
    hdmi_info(">>> out...\n");

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_infoframe_get(const hdmi_device *hdmi_dev, hdmi_infoframe_id infoframe_id, hdmi_infoframe *info_frame)
{
    errno_t ret;
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(info_frame, HI_ERR_NULL_PTR);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            ret = memcpy_s(&info_frame->vendor_infoframe, sizeof(info_frame->vendor_infoframe),
                &hdmi_dev->info_frame.vendor_infoframe, sizeof(hdmi_vendor_infoframe));
            hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            ret = memcpy_s(&info_frame->avi_infoframe, sizeof(info_frame->avi_infoframe),
                &hdmi_dev->info_frame.avi_infoframe, sizeof(hdmi_avi_infoframe));
            hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            ret = memcpy_s(&info_frame->audio_infoframe, sizeof(info_frame->audio_infoframe),
                &hdmi_dev->info_frame.audio_infoframe, sizeof(hdmi_audio_infoframe));
            hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
            break;
        default:
            return HI_ERR_HDMI_FEATURE_NO_SUPPORT;
    }
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

static hi_void infoframe_set_vendor(hdmi_device *hdmi_dev, hdmi_infoframe *info_frame)
{
    errno_t ret;
    hdmi_user_vsif_content *src_vsif_content = HI_NULL;
    hdmi_user_vsif_content *dest_vsif_content = HI_NULL;

    src_vsif_content = &info_frame->vendor_infoframe.vsif_content.cea861_vsif;
    dest_vsif_content = &hdmi_dev->info_frame.vendor_infoframe.vsif_content.cea861_vsif;
    /*
     * save the user data to pstHdmiDev->puInfoFrame,
     * because it will get the data form it when user call get infoframe.
     */
    dest_vsif_content->len = src_vsif_content->len;
    ret = memcpy_s(dest_vsif_content->data, HDMI_VENDOR_USER_DATA_MAX_LEN,
        src_vsif_content->data, src_vsif_content->len);
    hdmi_unequal_eok_return_void(ret);
    /* first: save user data to pstHdmiDev->vendor_data */
    hdmi_dev->vendor_data.len = src_vsif_content->len;
    (hi_void)memset_s(hdmi_dev->vendor_data.data, HDMI_VENDOR_USER_DATA_MAX_LEN, 0, HDMI_VENDOR_USER_DATA_MAX_LEN);
    ret = memcpy_s(hdmi_dev->vendor_data.data, HDMI_VENDOR_USER_DATA_MAX_LEN,
        src_vsif_content->data, src_vsif_content->len);
    hdmi_unequal_eok_return_void(ret);
    /* second: copy other information to puInfoFrame, except user data */
    src_vsif_content->ieee             = dest_vsif_content->ieee;
    src_vsif_content->format           = dest_vsif_content->format;
    src_vsif_content->vic              = dest_vsif_content->vic;
    src_vsif_content->_3d_meta_present = dest_vsif_content->_3d_meta_present;
    src_vsif_content->_3d_structure    = dest_vsif_content->_3d_structure;

    return;
}

static hi_void infoframe_set_avi_print(hdmi_avi_infoframe *src_avi_info)
{
    hdmi_info("type: %u\n", src_avi_info->type);
    hdmi_info("version: %u\n", src_avi_info->version);
    hdmi_info("length: %u\n", src_avi_info->length);
    hdmi_info("colorspace: %u\n", src_avi_info->colorspace);
    hdmi_info("active_info_valid: %u\n", src_avi_info->active_info_valid);
    hdmi_info("horizontal_bar_valid: %u\n", src_avi_info->horizontal_bar_valid);
    hdmi_info("vertical_bar_valid: %u\n", src_avi_info->vertical_bar_valid);
    hdmi_info("scan_mode: %u\n", src_avi_info->scan_mode);
    hdmi_info("colorimetry: %u\n", src_avi_info->colorimetry);
    hdmi_info("picture_aspect: %u\n", src_avi_info->picture_aspect);
    hdmi_info("active_aspect: %u\n", src_avi_info->active_aspect);
    hdmi_info("itc: %u\n", src_avi_info->itc);
    hdmi_info("extended_colorimetry: %u\n", src_avi_info->extended_colorimetry);
    hdmi_info("quantization_range: %u\n", src_avi_info->quantization_range);
    hdmi_info("nups: %u\n", src_avi_info->nups);
    hdmi_info("video_timing: %u\n", src_avi_info->video_timing);
    hdmi_info("disp_fmt: %u\n", src_avi_info->disp_fmt);
    hdmi_info("ycc_quantization_range: %u\n", src_avi_info->ycc_quantization_range);
    hdmi_info("content_type: %u\n", src_avi_info->content_type);
    hdmi_info("pixel_repeat: %u\n", src_avi_info->pixel_repeat);
    hdmi_info("top_bar: %u\n", src_avi_info->top_bar);
    hdmi_info("bottom_bar: %u\n", src_avi_info->bottom_bar);
    hdmi_info("left_bar: %u\n", src_avi_info->left_bar);
    hdmi_info("right_bar: %u\n", src_avi_info->right_bar);
}

static hi_void infoframe_set_avi(hdmi_device *hdmi_dev, hdmi_infoframe *info_frame)
{
    errno_t ret;
    hdmi_vo_attr       *vo_attr = &hdmi_dev->attr.vo_attr;
    hdmi_avi_infoframe *src_avi_info = &info_frame->avi_infoframe;
    hdmi_avi_infoframe *dest_avi_info = &hdmi_dev->info_frame.avi_infoframe;

    infoframe_set_avi_print(src_avi_info);

    if ((src_avi_info->type != dest_avi_info->type) || (src_avi_info->video_timing != dest_avi_info->video_timing) ||
        (src_avi_info->colorspace != dest_avi_info->colorspace) ||
        (src_avi_info->disp_fmt != dest_avi_info->disp_fmt) ||
        (src_avi_info->quantization_range != dest_avi_info->quantization_range) ||
        (src_avi_info->ycc_quantization_range != dest_avi_info->ycc_quantization_range) ||
        (src_avi_info->pixel_repeat != dest_avi_info->pixel_repeat)) {
        hdmi_warn("The current infoframe does not match the original infoframe!\n");
    }
    ret = memcpy_s(dest_avi_info, sizeof(*dest_avi_info), src_avi_info, sizeof(hdmi_avi_infoframe));
    hdmi_unequal_eok_return_void(ret);

    if (hdmi_dev->attr.app_attr.auth_mode == HI_TRUE) {
        // set aspect and colorimetry by attr
        vo_attr->picture_aspect       = dest_avi_info->picture_aspect;
        vo_attr->active_aspect        = dest_avi_info->active_aspect;
        vo_attr->colorimetry          = dest_avi_info->colorimetry;
        vo_attr->extended_colorimetry = dest_avi_info->extended_colorimetry;
        vo_attr->rgb_quantization     = dest_avi_info->quantization_range;
        vo_attr->ycc_quantization     = dest_avi_info->ycc_quantization_range;
        (hi_void) drv_hdmi_vo_attr_set(hdmi_dev, vo_attr);
    }

    return;
}

hi_s32 drv_hdmi_infoframe_set(hdmi_device *hdmi_dev, hdmi_infoframe_id infoframe_id, hdmi_infoframe *info_frame)
{
    errno_t ret;
    hdmi_info(">>> in..., infoframe_id: %u\n", infoframe_id);

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(info_frame, HI_ERR_NULL_PTR);

    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            infoframe_set_vendor(hdmi_dev, info_frame);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            infoframe_set_avi(hdmi_dev, info_frame);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO: {
            hdmi_audio_infoframe *src_audio_info = &info_frame->audio_infoframe;
            hdmi_audio_infoframe *dest_audio_info = &hdmi_dev->info_frame.audio_infoframe;
            ret = memcpy_s(dest_audio_info, sizeof(hdmi_dev->info_frame.audio_infoframe),
                src_audio_info, sizeof(hdmi_audio_infoframe));
            hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

            hdmi_info("type: %u\n", dest_audio_info->type);
            hdmi_info("version: %u\n", dest_audio_info->version);
            hdmi_info("length: %u\n", dest_audio_info->length);
            hdmi_info("channels: %u\n", dest_audio_info->channels);
            hdmi_info("coding_type: %u\n", dest_audio_info->coding_type);
            hdmi_info("sample_size: %u\n", dest_audio_info->sample_size);
            hdmi_info("sample_frequency: %u\n", dest_audio_info->sample_frequency);
            hdmi_info("coding_type_ext: %u\n", dest_audio_info->coding_type_ext);
            hdmi_info("channel_allocation: %u\n", dest_audio_info->channel_allocation);
            hdmi_info("lfe_playback_level: %u\n", dest_audio_info->lfe_playback_level);
            hdmi_info("level_shift_value: %u\n", dest_audio_info->level_shift_value);
            hdmi_info("downmix_inhibit: %u\n", dest_audio_info->downmix_inhibit);
            hdmi_info("level_shift_value: %u\n", dest_audio_info->level_shift_value);
            break;
        }
        default:
            return HI_ERR_HDMI_INVALID_PARA;
    }
    if ((infoframe_id != HDMI_INFOFRAME_TYPE_AVI) || (hdmi_dev->attr.app_attr.auth_mode == HI_FALSE)) {
        drv_hdmi_infoframe_send(&hdmi_dev->info_frame, infoframe_id, info_frame);
    }
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

static hi_s32 drv_hdmi_status_get(const hdmi_device *hdmi_dev, hdmi_status *status)
{
    hdmi_hardware_status hw_status = {0};

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(status, HI_ERR_NULL_PTR);

    hal_call_void(hal_hdmi_hardware_status_get, hdmi_dev->hal, &hw_status);

    /* some TV sometimes has no hot_plug but has rsen */
    status->connected = (hw_status.common_status.hotplug || hw_status.common_status.rsen);
    status->sink_power_on = hw_status.common_status.rsen;
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_play_status_get(const hdmi_device *hdmi_dev, hi_u32 *play_status)
{
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(play_status, HI_ERR_NULL_PTR);
    *play_status = ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_START) ? 1 : 0;
    return HI_SUCCESS;
}

hi_s32 get_hdmi_default_action_set(hdmi_device *hdmi_dev, hdmi_default_action action)
{
    hdmi_info(">>> in..., action: %u\n", action);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_dev->attr.app_attr.hdmi_action = action;
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 get_hdmi_delay_get(const hdmi_device *hdmi_dev, hdmi_delay *delay)
{
    errno_t ret;
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(delay, HI_ERR_NULL_PTR);

    ret = memcpy_s(delay, sizeof(*delay), &hdmi_dev->delay, sizeof(hdmi_delay));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

hi_s32 get_hdmi_delay_set(hdmi_device *hdmi_dev, hdmi_delay *delay)
{
    errno_t ret;
    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(delay, HI_ERR_NULL_PTR);

    if ((delay->fmt_delay > MAX_DELAY_TIME_MS) || (delay->mute_delay > MAX_DELAY_TIME_MS)) {
        hdmi_warn("delay time fmt: %u mute: %u over range: %u \n",
            delay->fmt_delay, delay->mute_delay, MAX_DELAY_TIME_MS);
        return HI_FAILURE;
    }
    ret = memcpy_s(&hdmi_dev->delay, sizeof(hdmi_delay), delay, sizeof(*delay));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    hdmi_info("force_mute_delay: %u, mute_delay: %u\n", delay->force_mute_delay, delay->mute_delay);
    hdmi_info("force_fmt_delay:  %u, fmt_delay:  %u\n", delay->force_fmt_delay, delay->fmt_delay);
    hdmi_info("<<< out...\n");

    return HI_SUCCESS;
}

static hi_s32 drv_hdmi_mod_param_set(hdmi_device *hdmi_dev, const drv_hdmi_mod_param *mode_param)
{
    hdmi_mode_param *param = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(mode_param, HI_ERR_NULL_PTR);

    param = &hdmi_dev->mode_param;
    hdmi_info("set trace length: %u->%u\n", param->trace_len, mode_param->trace_len);
    param->trace_len = mode_param->trace_len;

    return HI_SUCCESS;
}

static hi_s32 drv_hdmi_mod_param_get(hdmi_device *hdmi_dev, drv_hdmi_mod_param *mode_param)
{
    hdmi_mode_param *param = HI_NULL;

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(mode_param, HI_ERR_NULL_PTR);

    param = &hdmi_dev->mode_param;
    mode_param->trace_len = param->trace_len;

    return HI_SUCCESS;
}

#if defined(CONFIG_HI_PLATFORM_H8)
static hi_s32 drv_hdmi_hw_spec_get(hdmi_device *hdmi_dev, hdmi_hw_spec *hw_spec)
{
    hi_s32 ret;

    hdmi_info(">>> in...\n");
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hw_spec, HI_ERR_NULL_PTR);

    hal_call_ret(ret, hal_hdmi_phy_hw_spec_get, hdmi_dev->hal, hw_spec);
    hdmi_info("<<< out...\n");

    return ret;
}

hi_s32 drv_hdmi_hw_spec_set(hdmi_device *hdmi_dev, const hdmi_hw_spec *hw_spec)
{
    hi_u8 i;
    hi_u32 tmds_clk;
    hi_s32 ret;

    hdmi_info(">>> in...\n");

    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hdmi_dev->hal, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hw_spec, HI_ERR_NULL_PTR);

    tmds_clk = hdmi_dev->attr.vo_attr.hdmi_adapt_pix_clk;
    hdmi_info("tmds_clk : %u\n", tmds_clk);
    for (i = 0; i < HW_PARAM_ARRAY_COUNT; i++) {
        hdmi_info("stage[%u]: i_de_main_clk:%u i_de_main_data:%u\
            i_main_clk:%u i_main_data:%u ft_cap_clk:%u ft_cap_data:%u\n", i,
            hw_spec->hw_param[i].i_de_main_clk, hw_spec->hw_param[i].i_de_main_data,
            hw_spec->hw_param[i].i_main_clk, hw_spec->hw_param[i].i_main_data,
            hw_spec->hw_param[i].ft_cap_clk, hw_spec->hw_param[i].ft_cap_data);
    }
    hal_call_ret(ret, hal_hdmi_phy_hw_spec_set, hdmi_dev->hal, tmds_clk, hw_spec);
    hdmi_info("<<< out...\n");

    return ret;
}
#endif

static hi_s32 hdmi_file_open(void *private_data)
{
    hi_unused(private_data);
    osal_atomic_inc_return(&g_hdmi_count);

    return HI_SUCCESS;
}

static hi_s32 hdmi_file_close(void *private_data)
{
    hi_u32 hdmi_id = 0;
    hdmi_device *hdmi_dev = HI_NULL;

    hi_unused(private_data);
    if (osal_atomic_read(&g_hdmi_count)) {
        osal_atomic_dec_return(&g_hdmi_count);
        for (; hdmi_id < HDMI_DEVICE_ID_BUTT; hdmi_id++) {
            hdmi_dev = get_hdmi_device(hdmi_id);
            hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
            hdmi_mutex_lock(g_hdmi_mutex);
            drv_hdmi_close(hdmi_dev, HI_TRUE);
            hdmi_mutex_unlock(g_hdmi_mutex);
        }
    }

    return HI_SUCCESS;
}

static hi_s32 hdmi21_caps_check(hdmi_device *hdmi_dev, const hdmi_property *attr)
{
#ifndef HDMI_FRL_SUPPORT
    hi_bool is_4k = HI_FALSE;

    is_4k = (attr->disp_fmt == HDMI_VIDEO_FORMAT_3840X2160P_50) ||
            (attr->disp_fmt == HDMI_VIDEO_FORMAT_3840X2160P_60) ||
            (attr->disp_fmt == HDMI_VIDEO_FORMAT_4096X2160P_50) ||
            (attr->disp_fmt == HDMI_VIDEO_FORMAT_4096X2160P_60);

    if (is_4k == HI_TRUE &&
        (attr->out_color_space == HDMI_COLORSPACE_RGB || attr->out_color_space == HDMI_COLORSPACE_YCBCR444) &&
        (attr->deep_color_mode == HDMI_DEEP_COLOR_30BIT || attr->deep_color_mode == HDMI_DEEP_COLOR_36BIT)) {
        hdmi_err("param is invalid, fmt = %u, colorspace = %u, deepcolor = %u\n",
            attr->disp_fmt, attr->out_color_space, attr->deep_color_mode);
        return HI_ERR_HDMI_FEATURE_NO_SUPPORT;
    }
#else
    hi_unused(attr);
#endif

    hi_unused(hdmi_dev);
    return HI_SUCCESS;
}

static hi_void hdmi_property_vo_attr_adapt(hdmi_vo_attr *vo_attr, const hdmi_property *prop)
{
    hi_bool aspect_is43 = HI_FALSE;

    vo_attr->disp_fmt     = prop->disp_fmt;
    vo_attr->video_timing = prop->video_timing;
#if (!defined(CONFIG_HI_PLATFORM_H8))
    vo_attr->in_color_space = prop->in_color_space;
#endif
    vo_attr->clk_fs = prop->pix_clk;

    aspect_is43 = ((vo_attr->video_timing == HDMI_VIDEO_TIMING_640X480P_60000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_720X480P_60000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_720X576P_50000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_1440X240P_60000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_1440X288P_50000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_1440X480I_60000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_1440X576I_50000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_1440X576P_50000) ||
                   (vo_attr->video_timing == HDMI_VIDEO_TIMING_1440X576I_60000));
    if (aspect_is43 == HI_TRUE) {
        vo_attr->picture_aspect = HDMI_PICTURE_ASPECT_4_3;
    } else if (vo_attr->video_timing <= HDMI_VIDEO_TIMING_4096X2160P_120000 &&
               vo_attr->video_timing >= HDMI_VIDEO_TIMING_4096X2160P_24000) {
        vo_attr->picture_aspect = HDMI_PICTURE_ASPECT_256_135;
    } else {
        vo_attr->picture_aspect = HDMI_PICTURE_ASPECT_16_9;
    }

    return;
}

static hi_void drv_hdmi_property_app_attr_init(hdmi_app_attr *app_attr, const hdmi_property *prop)
{
    app_attr->enable_hdmi           = prop->enable_hdmi;
    app_attr->enable_video          = prop->enable_video;
    app_attr->enable_audio          = prop->enable_audio;
    app_attr->auth_mode             = prop->auth_mode;
    app_attr->hdmi_action           = prop->hdmi_action;
    app_attr->deep_color_mode       = prop->deep_color_mode;
    app_attr->enable_avi_infoframe  = prop->enable_avi_infoframe;
    app_attr->enable_aud_infoframe  = prop->enable_aud_infoframe;
    app_attr->enable_deep_clr_adapt = prop->enable_deep_clr_adapt;
#if (!defined(CONFIG_HI_PLATFORM_H8))
    app_attr->enable_clr_space_adapt = prop->enable_vid_mode_adapt;
    app_attr->out_csc_quantization   = prop->out_csc_quantization;
    app_attr->out_color_space        = prop->out_color_space;
#endif
}

static hi_s32 drv_hdmi_property_set(hdmi_device_id hdmi_id, const hdmi_property *prop)
{
    errno_t errnumber;
    hi_s32 ret;
    hdmi_app_attr tmp = {0};
    hdmi_ao_attr  ao_attr = {0};
    hdmi_vo_attr  vo_attr = {0};
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_if_null_return(prop, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
    ret = hdmi21_caps_check(hdmi_dev, prop);
    hdmi_if_failure_return(ret, HI_ERR_HDMI_FEATURE_NO_SUPPORT);

    app_attr = &hdmi_dev->attr.app_attr;
    errnumber = memcpy_s(&tmp, sizeof(tmp), app_attr, sizeof(hdmi_app_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    errnumber = memcpy_s(&vo_attr, sizeof(vo_attr), &hdmi_dev->attr.vo_attr, sizeof(hdmi_vo_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    errnumber = memcpy_s(&ao_attr, sizeof(ao_attr), &hdmi_dev->attr.ao_attr, sizeof(hdmi_ao_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);

    drv_hdmi_property_app_attr_init(app_attr, prop);
    hdmi_property_vo_attr_adapt(&vo_attr, prop);
    ret = drv_hdmi_vo_attr_set(hdmi_dev, &vo_attr);
    if (ret != HI_SUCCESS) {
        hdmi_err("set voattr error\n");
        goto err;
    }

    ao_attr.sample_fs    = prop->sample_rate;
    ao_attr.sample_depth = prop->bit_depth;
    ret = drv_hdmi_ao_attr_set(hdmi_dev, &ao_attr);
    if (ret != HI_SUCCESS) {
        hdmi_err("set aoattr error\n");
        goto err;
    }

    return HI_SUCCESS;

err:
    /* property set failed, recover the app_attr */
    errnumber = memcpy_s(app_attr, sizeof(hdmi_dev->attr.app_attr), &tmp, sizeof(hdmi_app_attr));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);

    return ret;
}

static hi_s32 drv_hdmi_property_get(hdmi_device_id hdmi_id, hdmi_property *prop)
{
    hdmi_attr *attr = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    attr = &hdmi_dev->attr;
    prop->enable_hdmi           = attr->app_attr.enable_hdmi;
    prop->enable_video          = attr->app_attr.enable_video;
    prop->enable_audio          = attr->app_attr.enable_audio;
    prop->video_timing          = attr->vo_attr.video_timing;
    prop->disp_fmt              = attr->vo_attr.disp_fmt;
    prop->enable_avi_infoframe  = attr->app_attr.enable_avi_infoframe;
    prop->enable_aud_infoframe  = attr->app_attr.enable_aud_infoframe;
    prop->enable_vid_mode_adapt = attr->app_attr.enable_clr_space_adapt;
    prop->enable_deep_clr_adapt = attr->app_attr.enable_deep_clr_adapt;
    prop->auth_mode             = attr->app_attr.auth_mode;
    prop->in_color_space        = attr->vo_attr.in_color_space;
    prop->out_color_space       = attr->app_attr.out_color_space;
    prop->deep_color_mode       = attr->app_attr.deep_color_mode;
    prop->out_csc_quantization  = attr->app_attr.out_csc_quantization;
    prop->hdmi_action           = attr->app_attr.hdmi_action;
    prop->sample_rate           = attr->ao_attr.sample_fs;
    prop->pix_clk               = attr->vo_attr.clk_fs;
    prop->bit_depth             = attr->ao_attr.sample_depth;

    return HI_SUCCESS;
}

#ifdef CONFIG_COMPAT
hi_s32 drv_hdmi_compat_cmd_process(unsigned int cmd, hi_void *arg, hi_bool user)
{
    hi_void __user *argp = (hi_void __user *)arg;
    return drv_hdmi_cmd_process(cmd, argp, user);
}
#endif

static hi_s32 hdmi_ioctrl_init(hi_void *arg, hi_bool user)
{
    hi_unused(arg);
    hi_unused(user);
    return drv_hdmi_init();
}

static hi_s32 hdmi_ioctrl_deinit(hi_void *arg, hi_bool user)
{
    hi_unused(arg);
    hi_unused(user);
    return drv_hdmi_deinit();
}

static hi_s32 hdmi_ioctrl_open(hi_void *arg, hi_bool user)
{
    hi_s32 ret;

    hdmi_open *open = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_OPEN user=%u\n", user);

    open = (hdmi_open *)arg;
    hdmi_if_null_return(open, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(open->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    get_hdmi_default_action_set(hdmi_dev, open->default_mode);
    ret = drv_hdmi_open(hdmi_dev, user);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_close(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_CLOSE user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(*(hdmi_device_id *)arg);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_close(hdmi_dev, user);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_start(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_START user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(*(hdmi_device_id *)arg);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_start(hdmi_dev);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_stop(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_STOP user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(*(hdmi_device_id *)arg);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_stop(hdmi_dev);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_get_sink_capability(hi_void *arg, hi_bool user)
{
    errno_t errnumber;
    hi_s32 ret;
    hdmi_tx_capability_data tx_cap = {0};
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_sink_capability *sink_cap = HI_NULL;
    drv_hdmi_sink_capability *drv_sink_cap = HI_NULL;

    hdmi_info("CMD_HDMI_GET_SINK_CAPABILITY user=%u\n", user);

    hdmi_if_null_return(arg, HI_FAILURE);
    drv_sink_cap = (drv_hdmi_sink_capability *)arg;
    hdmi_dev = get_hdmi_device(drv_sink_cap->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    (hi_void)memset_s(&drv_sink_cap->cap, sizeof(drv_sink_cap->cap), 0, sizeof(hdmi_sink_capability));
    hdmi_mutex_lock(g_hdmi_mutex);
    if (hdmi_dev->hal == HI_NULL || hdmi_dev->hal->hal_hdmi_tx_capability_get == HI_NULL) {
        hdmi_err("null pointer!\n");
        ret = HI_ERR_NULL_PTR;
        hdmi_mutex_unlock(g_hdmi_mutex);
        return ret;
    }
    hdmi_dev->hal->hal_hdmi_tx_capability_get(hdmi_dev->hal, &tx_cap);
    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) != HDMI_EDID_DATA_INVALID) {
        errnumber = memcpy_s(&drv_sink_cap->cap, sizeof(drv_sink_cap->cap), sink_cap, sizeof(hdmi_sink_capability));
        hdmi_unlock_unequal_eok_return(errnumber, g_hdmi_mutex, HI_ERR_HDMI_INVALID_PARA);
        ret = hdmi_capability_inter_section(&drv_sink_cap->cap, &tx_cap, hdmi_dev->attr.app_attr.auth_mode);
    } else {
        ret = HI_ERR_HDMI_READ_SINK_FAILED;
        hdmi_warn("no HPD, get sink capabity fail\n");
    }
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_poll_event(hi_void *arg, hi_bool user)
{
    hi_s32 ret = HI_FAILURE;
    drv_hdmi_event *event = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;

    hi_unused(user);
    hdmi_if_null_return(arg, ret);
    event = (drv_hdmi_event *)arg;
    hdmi_dev = get_hdmi_device(event->hdmi_id);
    if (hdmi_dev != HI_NULL) {
        ret = drv_hdmi_event_pool_read(event->hdmi_id, hdmi_current_id_get(hdmi_dev), &event->event);
    }

    return ret;
}

static hi_s32 hdmi_ioctrl_set_attr(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    drv_hdmi_property *prop = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_SET_ATTR user=%u\n", user);

    prop = (drv_hdmi_property *)arg;
    hdmi_if_null_return(prop, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(prop->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);
#ifdef HDMI_HDR_SUPPORT
    hdmi_attr_lock(hdmi_dev);
#endif
    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_property_set(prop->hdmi_id, &prop->prop);
    hdmi_mutex_unlock(g_hdmi_mutex);
#ifdef HDMI_HDR_SUPPORT
    hdmi_attr_un_lock(hdmi_dev);
#endif

    return ret;
}

static hi_s32 hdmi_ioctrl_get_attr(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    drv_hdmi_property *prop = HI_NULL;

    hdmi_info("CMD_HDMI_GET_ATTR user=%u\n", user);

    prop = (drv_hdmi_property *)arg;
    hdmi_if_null_return(prop, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_property_get(prop->hdmi_id, &prop->prop);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_get_vo_attr(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_vo_attr *attr = HI_NULL;

    hdmi_info("CMD_HDMI_GET_VO_ATTR user=%u\n", user);

    attr = (drv_hdmi_vo_attr *)arg;
    hdmi_if_null_return(attr, HI_FAILURE);
    hdmi_dev = get_hdmi_device(attr->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    (hi_void)memset_s(&attr->vo_attr, sizeof(attr->vo_attr), 0, sizeof(hdmi_vo_attr));

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_vo_attr_get(hdmi_dev, &attr->vo_attr);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_set_vo_attr(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_vo_attr *attr = HI_NULL;

    hdmi_info("CMD_HDMI_SET_VO_ATTR user=%u\n", user);

    attr = (drv_hdmi_vo_attr *)arg;
    hdmi_if_null_return(attr, HI_FAILURE);
    hdmi_dev = get_hdmi_device(attr->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_vo_attr_set(hdmi_dev, &attr->vo_attr);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_get_infoframe(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_infoframe *infoframe = HI_NULL;

    hdmi_info("CMD_HDMI_GET_INFOFRAME user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    infoframe = (drv_hdmi_infoframe *)arg;
    hdmi_dev  = get_hdmi_device(infoframe->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_infoframe_get(hdmi_dev, infoframe->infoframe_id, &(infoframe->infoframe));
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_set_infoframe(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_infoframe *infoframe = HI_NULL;

    hdmi_info("CMD_HDMI_SET_INFOFRAME user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    infoframe = (drv_hdmi_infoframe *)arg;
    hdmi_dev = get_hdmi_device(infoframe->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_infoframe_set(hdmi_dev, infoframe->infoframe_id, &(infoframe->infoframe));
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_avmute(hi_void *arg, hi_bool user)
{
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_avmute *drv_avmute = HI_NULL;
    hi_bool audio_enable = HI_FALSE;

    hdmi_info("CMD_HDMI_SET_AVMUTE user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    drv_avmute = (drv_hdmi_avmute *)arg;
    hdmi_dev = get_hdmi_device(drv_avmute->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    audio_enable = (hdmi_dev->attr.app_attr.enable_hdmi) && (hdmi_dev->attr.app_attr.enable_audio);

    hdmi_mutex_lock(g_hdmi_mutex);
    drv_hdmi_avmute_set(hdmi_dev, drv_avmute->avmute);
    if (drv_avmute->avmute) {
        drv_hdmi_audio_path_enable(hdmi_dev, !(drv_avmute->avmute));
    } else {
        drv_hdmi_audio_path_enable(hdmi_dev, audio_enable);
    }
    hdmi_mutex_unlock(g_hdmi_mutex);

    return HI_SUCCESS;
}

static hi_s32 hdmi_ioctrl_update_edid(hi_void *arg, hi_bool user)
{
    hi_s32 ret, edid_len;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_edid_raw_data *drv_edid_raw = HI_NULL;

    hdmi_info("CMD_HDMI_UPDATE_EDID user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    drv_edid_raw = (drv_hdmi_edid_raw_data *)arg;
    hdmi_dev = get_hdmi_device(drv_edid_raw->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    (hi_void)memset_s(&drv_edid_raw->edid_raw, sizeof(drv_edid_raw->edid_raw), 0, sizeof(hdmi_edid_raw_data));
    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_edid_update(&hdmi_dev->edid_info, HDMI_EDID_UPDATE_SINK);
    if (ret == HI_SUCCESS) {
        edid_len = drv_hdmi_edid_raw_get(&hdmi_dev->edid_info, drv_edid_raw->edid_raw.edid, HDMI_EDID_SIZE);
        if (edid_len != HI_ERR_HDMI_NUL_PTR && edid_len != HI_ERR_HDMI_INVALID_PARA && edid_len != HI_FAILURE) {
            drv_edid_raw->edid_raw.edid_len = (hi_u32)edid_len;
            drv_edid_raw->edid_raw.edid_valid = 1;
        }
    }
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_get_status(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_status *hdmi_state = HI_NULL;

    hdmi_info("CMD_HDMI_GET_STATUS user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    hdmi_state = (drv_hdmi_status *)arg;
    hdmi_dev = get_hdmi_device(hdmi_state->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    (hi_void)memset_s(&hdmi_state->status, sizeof(hdmi_state->status), 0, sizeof(hdmi_status));
    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_status_get(hdmi_dev, &hdmi_state->status);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_register_callback(hi_void *arg, hi_bool user)
{
    hi_bool hpd = HI_FALSE;
    hi_bool intr_status = HI_FALSE;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_REGISTER_CALLBACK user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(*(hdmi_device_id *)arg);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    if ((hdmi_dev->user_cnt != 0) || (hdmi_dev->kernel_cnt != 0)) {
        hdmi_dev->user_callback_cnt++;
        hdmi_mutex_unlock(g_hdmi_mutex);
        hdmi_mutex_lock(hdmi_dev->mutex_thread);
        hdmi_dev->k_callback = HI_NULL;
        /*
         * HPD needs to be reported when the following conditions are met:
         * 1. the driver has reported HPD before registering the callback.
         * 2. there is currently no pending hpd interrupt.
         * 3. hotplug pin level is high.
         */
        hal_call_void(hal_hdmi_hdp_intr_status_get, hdmi_dev->hal, &intr_status);
        if (hdmi_dev->hpd_notified == HI_TRUE && intr_status == HI_FALSE) {
            hal_call_void(hal_hdmi_hot_plug_status_get, hdmi_dev->hal, &hpd);
            if (hpd == HI_TRUE) {
                drv_hdmi_event_pool_write(hdmi_dev->hdmi_dev_id, HDMI_EVENT_HOTPLUG);
            }
        }
        hdmi_mutex_unlock(hdmi_dev->mutex_thread);
        hdmi_mutex_lock(g_hdmi_mutex);
    }
    hdmi_mutex_unlock(g_hdmi_mutex);

    return HI_SUCCESS;
}

static hi_s32 hdmi_ioctrl_ungister_callback(hi_void *arg, hi_bool user)
{
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_UNREGISTER_CALLBACK user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(*(hdmi_device_id *)arg);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    if (hdmi_dev->user_callback_cnt > 0) {
        hdmi_dev->user_callback_cnt--;
    }
    if (hdmi_dev->user_callback_cnt == 0) {
        hdmi_mutex_unlock(g_hdmi_mutex);
        hdmi_mutex_lock(hdmi_dev->mutex_thread);
        hdmi_dev->k_callback = hi_drv_hdmi_kernel_event_callback;
        hdmi_mutex_unlock(hdmi_dev->mutex_thread);
        hdmi_mutex_lock(g_hdmi_mutex);
    }
    hdmi_mutex_unlock(g_hdmi_mutex);

    return HI_SUCCESS;
}

#ifdef HDMI_HDR_SUPPORT
static hi_s32 hdmi_ioctrl_set_hdr_attr(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    drv_hdmi_hdr *hdr_attr = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_info("CMD_HDMI_SET_HDR_ATTR user=%u\n", user);

    hdmi_if_null_return(arg, HI_FAILURE);
    hdr_attr = (drv_hdmi_hdr *)arg;                 // from intf_k.c
    hdmi_dev = get_hdmi_device(hdr_attr->hdmi_id);  // find hdmi_device by hdmi_id
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    ret = drv_hdmi_hdr_attr_set(hdmi_dev, hdr_attr);

    return ret;
}
#endif

static hi_s32 hdmi_ioctrl_set_mod_param(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_mod_param *mod_param = HI_NULL;

    hdmi_info("CMD_HDMI_SET_MOD_PARAM user=%u\n", user);

    hdmi_if_null_return(arg, HI_FAILURE);
    mod_param = (drv_hdmi_mod_param *)arg;
    hdmi_dev = get_hdmi_device(mod_param->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_mod_param_set(hdmi_dev, mod_param);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_get_mod_param(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_mod_param *mod_param = HI_NULL;

    hdmi_info("CMD_HDMI_GET_MOD_PARAM user=%u\n", user);

    hdmi_if_null_return(arg, HI_FAILURE);
    mod_param = (drv_hdmi_mod_param *)arg;
    hdmi_dev = get_hdmi_device(mod_param->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_mod_param_get(hdmi_dev, mod_param);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

#if defined(CONFIG_HI_PLATFORM_H8)
static hi_s32 hdmi_ioctrl_get_hw_spec(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_hw_spec *spec = HI_NULL;

    hdmi_info("CMD_HDMI_GET_HW_SPEC user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    spec = (drv_hdmi_hw_spec *)arg;
    hdmi_dev = get_hdmi_device(spec->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    (hi_void)memset_s(&spec->hw_spec, sizeof(hdmi_hw_spec), 0, sizeof(hdmi_hw_spec));
    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_hw_spec_get(hdmi_dev, &spec->hw_spec);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}

static hi_s32 hdmi_ioctrl_set_hw_spec(hi_void *arg, hi_bool user)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_hw_spec *spec = HI_NULL;

    hdmi_info("CMD_HDMI_SET_HW_SPEC user=%u\n", user);

    hdmi_if_null_return(arg, HI_ERR_NULL_PTR);
    spec = (drv_hdmi_hw_spec *)arg;
    hdmi_dev = get_hdmi_device(spec->hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    hdmi_mutex_lock(g_hdmi_mutex);
    ret = drv_hdmi_hw_spec_set(hdmi_dev, &spec->hw_spec);
    hdmi_mutex_unlock(g_hdmi_mutex);

    return ret;
}
#endif

static const hdmi_ioctrl_func g_hdmi_cmd_func_tab[] = {
    { CMD_HDMI_INIT, hdmi_ioctrl_init },
    { CMD_HDMI_DEINIT, hdmi_ioctrl_deinit },
    { CMD_HDMI_OPEN, hdmi_ioctrl_open },
    { CMD_HDMI_CLOSE, hdmi_ioctrl_close },
    { CMD_HDMI_START, hdmi_ioctrl_start },
    { CMD_HDMI_STOP, hdmi_ioctrl_stop },
    { CMD_HDMI_GET_SINK_CAPABILITY, hdmi_ioctrl_get_sink_capability },
    { CMD_HDMI_POLL_EVENT, hdmi_ioctrl_poll_event },
    { CMD_HDMI_SET_ATTR, hdmi_ioctrl_set_attr },
    { CMD_HDMI_GET_ATTR, hdmi_ioctrl_get_attr },
    { CMD_HDMI_GET_VO_ATTR, hdmi_ioctrl_get_vo_attr },
    { CMD_HDMI_SET_VO_ATTR, hdmi_ioctrl_set_vo_attr },
    { CMD_HDMI_GET_INFOFRAME, hdmi_ioctrl_get_infoframe },
    { CMD_HDMI_SET_INFOFRAME, hdmi_ioctrl_set_infoframe },
    { CMD_HDMI_SET_AVMUTE, hdmi_ioctrl_avmute },
    { CMD_HDMI_UPDATE_EDID, hdmi_ioctrl_update_edid },
    { CMD_HDMI_GET_STATUS, hdmi_ioctrl_get_status },
    { CMD_HDMI_REGISTER_CALLBACK, hdmi_ioctrl_register_callback },
    { CMD_HDMI_UNREGISTER_CALLBACK, hdmi_ioctrl_ungister_callback },
#ifdef HDMI_HDR_SUPPORT
    { CMD_HDMI_SET_HDR_ATTR, hdmi_ioctrl_set_hdr_attr },
#endif
    { CMD_HDMI_SET_MOD_PARAM, hdmi_ioctrl_set_mod_param },
    { CMD_HDMI_GET_MOD_PARAM, hdmi_ioctrl_get_mod_param },
#if defined(CONFIG_HI_PLATFORM_H8)
    { CMD_HDMI_GET_HW_SPEC, hdmi_ioctrl_get_hw_spec },
    { CMD_HDMI_SET_HW_SPEC, hdmi_ioctrl_set_hw_spec },
#endif
};

hi_s32 drv_hdmi_cmd_process(unsigned int cmd, hi_void *arg, hi_bool user)
{
    hi_s32 ret = HI_FAILURE;
    hi_u32 index;

    for (index = 0; index < hdmi_array_size(g_hdmi_cmd_func_tab); index++) {
        if ((g_hdmi_cmd_func_tab[index].cmd == cmd) && (g_hdmi_cmd_func_tab[index].hdmi_ioctrl_func != HI_NULL)) {
            ret = g_hdmi_cmd_func_tab[index].hdmi_ioctrl_func(arg, user);
            break;
        }
    }

    if (index == hdmi_array_size(g_hdmi_cmd_func_tab)) {
        hdmi_err("unknown cmd:0x%x\n", cmd);
        return HI_ERR_HDMI_UNKNOWN_COMMAND;
    }
    if (ret != HI_SUCCESS) {
        hdmi_warn("ret != HI_SUCCESS!\n");
    }

    return ret;
}

#ifndef CONFIG_COMPAT
static long hdmi_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    hi_unused(private_data);
    return drv_hdmi_cmd_process(cmd, (hi_void *)(uintptr_t)arg, HI_TRUE);
}
#else
static long hdmi_compact_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    hi_unused(private_data);
    return drv_hdmi_compat_cmd_process(cmd, (hi_void *)(uintptr_t)arg, HI_TRUE);
}
#endif

hi_s32 hdmi_init(hi_void *args)
{
    hi_unused(args);
    osal_atomic_inc_return(&g_hdmi_count);
    return 0;
}

hi_void hdmi_exit(hi_void)
{
    /* deinit HDMI by user. */
    if (osal_atomic_dec_return(&g_hdmi_count)) {
        hdmi_info("exit...");
    }
    return;
}

static hi_void hdmi_notify(mod_notice_id notice)
{
    hi_unused(notice);
    return;
}

static hi_void hdmi_query_state(mod_state *state)
{
    *state = MOD_STATE_FREE;
    return;
}

static hi_u32 hdmi_get_ver_magic(hi_void)
{
    return VERSION_MAGIC;
}

static hdmi_export_func g_hdmi_export_funcs = {
    .pfn_stop = hi_drv_hdmi_stop,
#if (defined(CONFIG_HI_PLATFORM_H8))
    .pfn_csc_param_set = hi_drv_hdmi_csc_param_set,
    .pfn_video_param_set = hi_drv_hdmi_video_param_set,
#endif
#ifdef HDMI_HDR_SUPPORT
    .pfn_set_hdr_attr = hi_drv_hdmi_set_hdr_attr,
#endif
};

static umap_module g_module = {
    .export_funcs    = &g_hdmi_export_funcs,
    .mod_id          = HI_ID_HDMI,
    .mod_name        = "hdmi",
    .pfn_init        = hdmi_init,
    .pfn_exit        = hdmi_exit,
    .pfn_query_state = hdmi_query_state,
    .pfn_notify      = hdmi_notify,
    .pfn_ver_checker = hdmi_get_ver_magic,
    .data            = HI_NULL,
};

static long hdmi_file_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
#ifndef CONFIG_COMPAT
    return (long)hdmi_ioctl(cmd, arg, private_data);
#else
    return (long)hdmi_compact_ioctl(cmd, arg, private_data);
#endif
}

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
hdmi_proc_item g_hdmi_proc_item = {
#ifdef CONFIG_HI_PLATFORM_H8
    .fn_read = drv_hdmi_proc_show,
#else
    .fn_read = drv_hdmi_read_proc,
#endif
#ifdef HDMI_DEBUG_SUPPORT
    .fn_write = drv_hdmi_debug_source_write,
#else
    .fn_write = HI_NULL,
#endif
};

hdmi_proc_item g_vo_proc_item = {
#ifdef CONFIG_HI_PLATFORM_H8
    .fn_read = drv_hdmi_vo_proc_show,
#else
    .fn_read = drv_hdmi_vo_read_proc,
#endif
    .fn_write = HI_NULL,
};

hdmi_proc_item g_ao_proc_item = {
#ifdef CONFIG_HI_PLATFORM_H8
    .fn_read = drv_hdmi_ao_proc_show,
#else
    .fn_read = drv_hdmi_ao_read_proc,
#endif
    .fn_write = HI_NULL,
};

hdmi_proc_item g_sink_proc_item = {
#ifdef CONFIG_HI_PLATFORM_H8
    .fn_read = drv_hdmi_sink_proc_show,
#else
    .fn_read = drv_hdmi_sink_read_proc,
#endif
    .fn_write = HI_NULL,
};

static hi_void drv_hdmi_proc_add_module(hi_char *entry_name, hdmi_proc_item *proc_item, hi_void *data)
{
    osal_proc_entry_t *proc = NULL;

    hi_unused(data);
    proc = osal_create_proc_entry(entry_name, NULL);
    hdmi_if_null_return_void(proc);
    proc->read = (hi_void *)proc_item->fn_read;
    proc->write = (hi_void *)proc_item->fn_write;

    return;
}

static hi_void drv_hdmi_proc_remove_module(hi_char *entry_name)
{
    osal_remove_proc_entry(entry_name, NULL);
    return;
}

hi_void drv_hdmi_proc_register(hi_u32 hdmi_id)
{
    errno_t ret;
    hi_char proc_name[16];

    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u", "hdmi", hdmi_id);
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_add_module(proc_name, &g_hdmi_proc_item, HI_NULL);
    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u%s", "hdmi", hdmi_id, "_vo");
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_add_module(proc_name, &g_vo_proc_item, HI_NULL);
    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u%s", "hdmi", hdmi_id, "_ao");
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_add_module(proc_name, &g_ao_proc_item, HI_NULL);
    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u%s", "hdmi", hdmi_id, "_sink");
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_add_module(proc_name, &g_sink_proc_item, HI_NULL);

    return;
}

hi_void drv_hdmi_proc_un_register(hi_u32 hdmi_id)
{
    hi_s32 ret;
    hi_char proc_name[16];

    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u", "hdmi", hdmi_id);
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_remove_module(proc_name);
    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u%s", "hdmi", hdmi_id, "_vo");
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_remove_module(proc_name);
    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u%s", "hdmi", hdmi_id, "_ao");
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_remove_module(proc_name);
    (hi_void)memset_s(proc_name, sizeof(proc_name), 0, sizeof(proc_name));
    ret = snprintf_s(proc_name, sizeof(proc_name), sizeof(proc_name) - 1, "%s%u%s", "hdmi", hdmi_id, "_sink");
    if (ret < 0) {
        hdmi_err("snprintf_s err.\n");
        return;
    }
    drv_hdmi_proc_remove_module(proc_name);

    return;
}
#endif

static struct osal_fileops g_hdmi_file_ops = {
    .open           = hdmi_file_open,
    .unlocked_ioctl = hdmi_file_ioctl,
    .release        = hdmi_file_close,
#ifdef CONFIG_COMPAT
    .compat_ioctl   = hdmi_file_ioctl,
#endif
};

static struct osal_pmops g_hdmi_drv_ops = {0};

static hi_s32 hdmi_dev_register(hi_void)
{
    errno_t ret;
    hi_char ch_devfs_name[20] = {0};

    if (cmpi_register_module(&g_module)) {
        hdmi_warn("cmpi_register_module hdmi fail \n");
        return HI_FAILURE;
    }

    /* register hdmi device */
    ret = snprintf_s(ch_devfs_name, sizeof(ch_devfs_name), sizeof(ch_devfs_name) - 1, "%s", UMAP_DEVNAME_HDMI_BASE);
    if (ret < 0) {
        hdmi_err("snprintf_s err\n");
        return HI_FAILURE;
    }
    g_hdmi_device = osal_createdev(ch_devfs_name);
    hdmi_if_null_return(g_hdmi_device, HI_FAILURE);
    g_hdmi_device->fops       = &g_hdmi_file_ops;
    g_hdmi_device->osal_pmops = &g_hdmi_drv_ops;
    g_hdmi_device->minor      = UMAP_HDMI_MINOR_BASE;
    if (osal_registerdevice(g_hdmi_device) < 0) {
        hdmi_warn("g_hdmi_device register failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
static hi_void hdmi_proc_register(hi_void)
{
    hi_s32 hdmi_id;
    hdmi_device *hdmi_dev = HI_NULL;

        /* register hdmi proc */
    for (hdmi_id = 0; hdmi_id < HDMI_DEVICE_ID_BUTT; hdmi_id++) {
        hdmi_dev = get_hdmi_device(hdmi_id);
        if (hdmi_dev != HI_NULL) {
            drv_hdmi_proc_register(hdmi_id);
        }
    }

    return;
}
#endif

hi_s32 hdmi_drv_mod_init(hi_void)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_dev = get_hdmi_device(HDMI_DEVICE_ID0);
    if (hdmi_dev != HI_NULL) {
        /* init csc param. */
        hdmi_dev->csc_param.colorimetry = HI_HDMI_COLORIMETRY_ITU709;
        hdmi_dev->csc_param.pixel_encoding = HI_HDMI_COLORSPACE_YCBCR444;
        hdmi_dev->csc_param.quantization = HI_HDMI_QUANT_RANGE_LIMITED;

        osal_sema_init(&hdmi_dev->mutex_proc, 1);
        osal_sema_init(&hdmi_dev->mutex_thread, 1);
#ifdef HDMI_HDR_SUPPORT
        osal_spin_lock_init(&hdmi_dev->hdr_mutex.attr_spin);
#endif
    }

    osal_sema_init(&g_hdmi_mutex, 1);
    osal_atomic_init(&g_hdmi_count);
    osal_atomic_set(&g_hdmi_count, 0);

    ret = hdmi_dev_register();
    if (ret == HI_FAILURE) {
        goto exit_dev;
    }

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    hdmi_proc_register();
#endif

#ifdef CONFIG_DRM_HISI_HISMART
    drm_hal_hdmitx_dev_register();
#endif

#if (defined(CONFIG_HI_PLATFORM_H8))
    HI_PRINT("load hdmi.ko ....OK!\n");
#else
    HI_PRINT("load hdmi.ko for %s...OK!\n", CHIP_NAME);
#endif

#if defined(HDMI_LITEOS_SUPPORT)
#ifdef HDMI_PRODUCT_HI3516CV500
    g_hdmi_reg = (hi_char *)osal_ioremap_nocache(BASE_ADDR_TX_DPHY, HDMI_TX_REG_LEN);
#else
    g_hdmi_reg = (hi_char *)osal_ioremap_nocache(HDMI_TX_BASE_ADDR, HDMI_TX_REG_LEN);
#endif
    if (g_hdmi_reg == HI_NULL) {
        hdmi_err("ioremap_nocache failed!\n");
        return HI_FAILURE;
    }
#endif

    return HI_SUCCESS;

exit_dev:
    osal_destroydev(g_hdmi_device);
    cmpi_unregister_module(HI_ID_HDMI);

    return HI_FAILURE;
}

hi_void hdmi_drv_mod_exit(hi_void)
{
    hi_u32 hdmi_id;
    hdmi_device *hdmi_dev = HI_NULL;

#ifdef CONFIG_DRM_HISI_HISMART
    drm_hal_hdmitx_dev_unregister();
#endif

    for (hdmi_id = 0; hdmi_id < HDMI_DEVICE_ID_BUTT; hdmi_id++) {
        hdmi_dev = get_hdmi_device(hdmi_id);
        if (hdmi_dev != HI_NULL) {
            if ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_OPEN) {
                hdmi_dev->user_cnt = 0;
                hdmi_dev->kernel_cnt = 0;
                hdmi_release(hdmi_dev);
            }
            osal_sema_destroy(&hdmi_dev->mutex_proc);
#ifdef HDMI_HDR_SUPPORT
            osal_spin_lock_destroy(&hdmi_dev->hdr_mutex.attr_spin);
#endif
            osal_sema_destroy(&hdmi_dev->mutex_thread);
        }
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
        drv_hdmi_proc_un_register(hdmi_id);
#endif
    }
    drv_hdmi_hardware_reset();
    cmpi_unregister_module(HI_ID_HDMI);
    osal_deregisterdevice(g_hdmi_device);
    osal_destroydev(g_hdmi_device);
    osal_atomic_destroy(&g_hdmi_count);
    osal_sema_destroy(&g_hdmi_mutex);

#if defined(HDMI_LITEOS_SUPPORT)
    if (g_hdmi_reg != HI_NULL) {
        osal_iounmap((void *)g_hdmi_reg, HDMI_TX_REG_LEN);
        g_hdmi_reg = HI_NULL;
    }
#endif
}


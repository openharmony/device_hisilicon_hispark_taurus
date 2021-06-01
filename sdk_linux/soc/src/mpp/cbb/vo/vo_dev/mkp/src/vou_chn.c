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

#include "vou_chn.h"
#include "hi_osal.h"
#include "hi_debug_adapt.h"
#include "hi_math_adapt.h"
#include "mkp_vo_dev.h"
#include "hi_board.h"
#include "valg_plat.h"
#include "graphics_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VOU_DEBUG                0

#define VO_ROTATION_TASK_MAX_CNT 2

#define VO_WBC_MAX_VGS_TASK      200
#define VO_WBC_MAX_COVEREX_NUM   VO_WBC_MAX_VGS_TASK

#define VO_DEV_MAX_FRMRATE       240

volatile hi_bool g_wbc_bottom = HI_FALSE;

vo_sync_basic_info g_vo_sync_basic_info[VO_OUTPUT_BUTT] = {
    { VO_OUTPUT_PAL,          "PAL",          720,  576,  25 },
    { VO_OUTPUT_NTSC,         "NTSC",         720,  480,  30 },
    { VO_OUTPUT_1080P24,      "1080P@24",     1920, 1080, 24 },
    { VO_OUTPUT_1080P25,      "1080P@25",     1920, 1080, 25 },
    { VO_OUTPUT_1080P30,      "1080P@30",     1920, 1080, 30 },
    { VO_OUTPUT_720P50,       "720P@50",      1280, 720,  50 },
    { VO_OUTPUT_720P60,       "720P@60",      1280, 720,  60 },
    { VO_OUTPUT_1080I50,      "1080I50",      1920, 1080, 25 },
    { VO_OUTPUT_1080I60,      "1080I60",      1920, 1080, 30 },
    { VO_OUTPUT_1080P50,      "1080P@50",     1920, 1080, 50 },
    { VO_OUTPUT_1080P60,      "1080P@60",     1920, 1080, 60 },
    { VO_OUTPUT_576P50,       "576p@50",      720,  576,  50 },
    { VO_OUTPUT_480P60,       "480p@60",      720,  480,  60 },
    { VO_OUTPUT_800x600_60,   "800x600@60",   800,  600,  60 },
    { VO_OUTPUT_1024x768_60,  "1024x768@60",  1024, 768,  60 },
    { VO_OUTPUT_1280x1024_60, "1280x1024@60", 1280, 1024, 60 },
    { VO_OUTPUT_1366x768_60,  "1366x768@60",  1366, 768,  60 },
    { VO_OUTPUT_1440x900_60,  "1440x900@60",  1440, 900,  60 },
    { VO_OUTPUT_1280x800_60,  "1280x800@60",  1280, 800,  60 },
    { VO_OUTPUT_1600x1200_60, "1600x1200@60", 1600, 1200, 60 },
    { VO_OUTPUT_1680x1050_60, "1680x1050@60", 1680, 1050, 60 },
    { VO_OUTPUT_1920x1200_60, "1920x1200@60", 1920, 1200, 60 },
    { VO_OUTPUT_640x480_60,   "640x480@60",   640,  480,  60 },
    { VO_OUTPUT_960H_PAL,     "960H_PAL",     960,  576,  25 },
    { VO_OUTPUT_960H_NTSC,    "960H_NTSC",    960,  480,  30 },
    { VO_OUTPUT_1920x2160_30, "1920x2160@30", 1920, 2160, 30 },
    { VO_OUTPUT_2560x1440_30, "2560x1440@30", 2560, 1440, 30 },
    { VO_OUTPUT_2560x1440_60, "2560x1440@60", 2560, 1440, 60 },
    { VO_OUTPUT_2560x1600_60, "2560x1600@60", 2560, 1600, 60 },
    { VO_OUTPUT_3840x2160_24, "3840x2160@24", 3840, 2160, 24 },
    { VO_OUTPUT_3840x2160_25, "3840x2160@25", 3840, 2160, 25 },
    { VO_OUTPUT_3840x2160_30, "3840x2160@30", 3840, 2160, 30 },
    { VO_OUTPUT_3840x2160_50, "3840x2160@50", 3840, 2160, 50 },
    { VO_OUTPUT_3840x2160_60, "3840x2160@60", 3840, 2160, 60 },
    { VO_OUTPUT_4096x2160_24, "4096x2160@24", 4096, 2160, 24 },
    { VO_OUTPUT_4096x2160_25, "4096x2160@25", 4096, 2160, 25 },
    { VO_OUTPUT_4096x2160_30, "4096x2160@30", 4096, 2160, 30 },
    { VO_OUTPUT_4096x2160_50, "4096x2160@50", 4096, 2160, 50 },
    { VO_OUTPUT_4096x2160_60, "4096x2160@60", 4096, 2160, 60 },
    { VO_OUTPUT_320x240_60,   "320x240@60",   320,  240,  60 },
    { VO_OUTPUT_320x240_50,   "320x240@50",   320,  240,  50 },
    { VO_OUTPUT_240x320_50,   "240x320@50",   240,  320,  50 },
    { VO_OUTPUT_240x320_60,   "240x320@60",   240,  320,  60 },
    { VO_OUTPUT_800x600_50,   "800x600@50",   800,  600,  50 },
    { VO_OUTPUT_720x1280_60,  "720x1280@60",  720,  1280, 60 },
    { VO_OUTPUT_1080x1920_60, "1080x1920@60", 1080, 1920, 60 },
    { VO_OUTPUT_7680x4320_30, "7680x4320@30", 7680, 4320, 30 },
    { VO_OUTPUT_USER,         "USER",         0,    0,    0 },
};

vo_sync_basic_info *vo_get_sync_basic_info(hi_void)
{
    return g_vo_sync_basic_info;
}

hi_void vou_set_disp_max_size(hi_vo_dev dev, hi_vo_intf_sync intf_sync, hi_vo_sync_info sync_info)
{
    hi_u32 max_width = 720;
    hi_u32 max_height = 576;
    vo_dev_info *dev_ctx = HI_NULL;

    if (intf_sync >= VO_OUTPUT_PAL && intf_sync < VO_OUTPUT_USER) {
        if (g_vo_sync_basic_info[intf_sync].index == intf_sync) {
            max_width = g_vo_sync_basic_info[intf_sync].width;
            max_height = g_vo_sync_basic_info[intf_sync].height;
        } else {
            max_width = VO_DISP_1080_WIDTH;
            max_height = VO_DISP_1080_HEIGHT;
        }
    } else if (intf_sync == VO_OUTPUT_USER) {
        max_width = sync_info.hact;
        max_height = (sync_info.iop) ? sync_info.vact : sync_info.vact * 2;
    } else {
        max_width = VO_DISP_1080_WIDTH;
        max_height = VO_DISP_1080_HEIGHT;
    }

    dev_ctx = vo_get_dev_ctx(dev);
    dev_ctx->max_width = max_width;
    dev_ctx->max_height = max_height;

    vou_drv_set_disp_max_size(dev, max_width, max_height);
}

hi_s32 vo_set_pub_attr_check(hi_vo_dev dev, hi_vo_pub_attr *pub_attr)
{
    hi_s32 ret;
    vo_dev_info *dev_ctx = HI_NULL;

    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->vo_enable == HI_TRUE) {
        vo_err_trace("vo %d doesn't disabled!\n", dev);
        return HI_ERR_VO_DEV_HAS_ENABLED;
    }

    ret = vou_drv_check_dev_pub_attr(dev, pub_attr);
    if (ret != HI_SUCCESS) {
        vo_err_trace("vo%d's pub attr is illegal!\n", dev);
        return ret;
    }

    if (!vou_drv_is_virt_dev(dev)) {
        if (pub_attr->intf_sync == VO_OUTPUT_USER) {
            ret = vou_drv_check_usr_sync_timing(dev, &pub_attr->sync_info);
            if (ret != HI_SUCCESS) {
                vo_err_trace("vo%d's usr sync_timing is illegal!\n", dev);
                return ret;
            }
        }
    }

    return ret;
}

hi_void vo_set_phy_dev_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr)
{
    hi_u32 vtth = 0;

    vou_drv_set_dev_intf_type(dev, pub_attr->intf_type);
    vou_drv_set_dev_out_sync(dev, pub_attr->intf_sync);

    vou_drv_set_dev_bk_grd(dev, pub_attr->bg_color);

    vou_drv_get_dev_vtth(dev, &vtth);
    vou_drv_set_dev_vtth(dev, vtth);
    if (pub_attr->intf_sync == VO_OUTPUT_USER) {
        vou_drv_set_usr_sync_timing(&pub_attr->sync_info);
    }
}

hi_void vo_set_full_frame_rate(hi_vo_pub_attr *pub_attr, vo_dev_info *dev_ctx)
{
    if (pub_attr->intf_sync >= VO_OUTPUT_PAL && pub_attr->intf_sync < VO_OUTPUT_USER) {
        if (g_vo_sync_basic_info[pub_attr->intf_sync].index == pub_attr->intf_sync) {
            dev_ctx->full_frame_rate = g_vo_sync_basic_info[pub_attr->intf_sync].frame_rate;
        } else {
            dev_ctx->full_frame_rate = VO_DISP_FREQ_VGA;
        }
    } else {
        dev_ctx->full_frame_rate = VO_DISP_FREQ_VGA;
    }
}

hi_s32 vou_set_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr)
{
    hi_s32 ret;
    vo_dev_info *dev_ctx = HI_NULL;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(pub_attr);

    vo_down_sem_return();

    ret = vo_set_pub_attr_check(dev, pub_attr);
    if (ret != HI_SUCCESS) {
        vo_up_sem();
        return ret;
    }

    if (!vou_drv_is_virt_dev(dev)) {
        vo_set_phy_dev_pub_attr(dev, pub_attr);
    }

    dev_ctx = vo_get_dev_ctx(dev);

#ifndef FPSCTRL_BYTIME
    vo_set_full_frame_rate(pub_attr, dev_ctx);
#endif

    vou_set_disp_max_size(dev, pub_attr->intf_sync, pub_attr->sync_info);
    /* copy to drv level. */
    vou_drv_set_dev_full_frame_rate(dev, dev_ctx->full_frame_rate);
    vou_drv_set_pub_attr(dev, pub_attr);
    (hi_void)memcpy_s(&dev_ctx->vou_attr, sizeof(hi_vo_pub_attr), pub_attr, sizeof(hi_vo_pub_attr));
    dev_ctx->config = HI_TRUE;

    vo_up_sem();

    return HI_SUCCESS;
}

hi_s32 vou_get_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr)
{
    vo_dev_info *dev_ctx = HI_NULL;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(pub_attr);

    vo_down_sem_return();

    dev_ctx = vo_get_dev_ctx(dev);
    (hi_void)memcpy_s(pub_attr, sizeof(hi_vo_pub_attr), &dev_ctx->vou_attr, sizeof(hi_vo_pub_attr));

    vo_up_sem();

    return HI_SUCCESS;
}

hi_s32 vou_set_dev_frame_rate(hi_vo_dev dev, hi_u32 *frm_rate)
{
    vo_dev_info *dev_ctx = HI_NULL;
    hi_u32 frame_rate;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(frm_rate);

    frame_rate = *frm_rate;

    if (frame_rate <= 0 || frame_rate > VO_DEV_MAX_FRMRATE) {
        vo_err_trace("vo %d framerate should be (0, %d] which is %u!\n", dev, VO_DEV_MAX_FRMRATE, frame_rate);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    vo_down_sem_return();
    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->vo_enable == HI_TRUE) {
        vo_err_trace("vo %d doesn't disabled!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_HAS_ENABLED;
    }

    if (dev_ctx->config == HI_FALSE) {
        vo_err_trace("vo %d doesn't set pub attr!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_NOT_CONFIG;
    }

    if (dev_ctx->vou_attr.intf_sync != VO_OUTPUT_USER) {
        vo_err_trace("vo %d doesn't set user intf_sync!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_NOT_CONFIG;
    }

    dev_ctx->full_frame_rate = frame_rate;
    vou_drv_set_dev_full_frame_rate(dev, dev_ctx->full_frame_rate);

    vo_up_sem();

    return HI_SUCCESS;
}

hi_s32 vou_get_dev_frame_rate(hi_vo_dev dev, hi_u32 *frame_rate)
{
    vo_dev_info *dev_ctx = HI_NULL;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(frame_rate);

    vo_down_sem_return();
    dev_ctx = vo_get_dev_ctx(dev);

    *frame_rate = dev_ctx->full_frame_rate;

    vo_up_sem();

    return HI_SUCCESS;
}

hi_s32 vou_set_vtth(hi_vo_dev dev, hi_u32 *vtth_set)
{
    hi_s32 ret;
    hi_u32 vtth;
    vo_dev_info *dev_ctx = HI_NULL;
    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(vtth_set);
    vo_down_sem_return();

    vtth = *vtth_set;

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
    if (vou_drv_is_virt_dev(dev)) {
        vo_err_trace("invalid dev id %d! should set vtth for physic vo dev\n", dev);
        vo_up_sem();
        return HI_ERR_VO_INVALID_DEVID;
    }
#endif

    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->vo_enable == HI_TRUE) {
        vo_err_trace("can't set dev(%d)'s vtth after vo enabled!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_HAS_ENABLED;
    }

    if (vo_drv_is_phy_dev_hd(dev) && ((vtth < VO_HD_MIN_VTTH_WATERLINE) || (vtth > VO_HD_MAX_VTTH_WATERLINE))) {
        vo_err_trace("dev(%d) vtth %u illegal\n", dev, vtth);
        vo_up_sem();
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if (vo_drv_is_phy_dev_sd(dev) && ((vtth < VO_SD_MIN_VTTH_WATERLINE) || (vtth > VO_SD_MAX_VTTH_WATERLINE))) {
        vo_err_trace("dev(%d) vtth %u illegal\n", dev, vtth);
        vo_up_sem();
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    ret = vou_drv_set_dev_vtth(dev, *vtth_set);
    vo_up_sem();
    return ret;
}

hi_s32 vou_get_vtth(hi_vo_dev dev, hi_u32 *vtth)
{
    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(vtth);
    vo_down_sem_return();

    if (!(vo_drv_is_phy_dev_hd(dev) || vo_drv_is_phy_dev_sd(dev))) {
        vo_err_trace("invalid dev id %d! only physic vo dev support vtth\n", dev);
        vo_up_sem();
        return HI_ERR_VO_INVALID_DEVID;
    }

    vou_drv_get_dev_vtth(dev, vtth);
    vo_up_sem();
    return HI_SUCCESS;
}

hi_s32 vou_set_vtth2(hi_vo_dev dev, hi_u32 *vtth)
{
    hi_s32 ret;
    hi_u32 vtth1;
    hi_u32 vtth2;
    hi_vo_mod_param *vo_mod_param = HI_NULL;
    vo_dev_info *dev_ctx = HI_NULL;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(vtth);

    if (vou_drv_is_support_save_buf_mode() == HI_FALSE) {
        vo_err_trace("not support setting vtth2!\n");
        return HI_ERR_VO_NOT_SUPPORT;
    }

    vtth2 = *vtth;
    vo_down_sem_return();

    if (dev >= VO_MAX_PHY_DEV_NUM) {
        vo_err_trace("dev(%d) is not physical device!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_INVALID_DEVID;
    }

    vo_mod_param = vo_get_vo_mod_param();
    if (vo_mod_param->save_buf_mode[dev] == HI_FALSE) {
        vo_err_trace("save_buf_mode must be true\n");
        vo_up_sem();
        return HI_ERR_VO_NOT_PERMIT;
    }

    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->vo_enable == HI_TRUE) {
        vo_err_trace("can't set dev(%d)'s vtth after vo enabled!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_HAS_ENABLED;
    }

    vou_drv_get_dev_vtth(dev, &vtth1);

    if (vo_drv_is_phy_dev_hd(dev) && ((vtth2 < VO_HD_MIN_VTTH_WATERLINE2) || (vtth2 > vtth1))) {
        vo_err_trace("dev(%d) vtth2(%u),vtth1(%u) illegal\n", dev, vtth2, vtth1);
        vo_up_sem();
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if (vo_drv_is_phy_dev_sd(dev) && ((vtth2 < VO_SD_MIN_VTTH_WATERLINE2) || (vtth2 > vtth1))) {
        vo_err_trace("dev(%d) vtth2(%u),vtth1(%u) illegal\n", dev, vtth2, vtth1);
        vo_up_sem();
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    ret = vou_drv_set_dev_vtth2(dev, *vtth);

    vo_up_sem();
    return ret;
}

hi_s32 vou_get_vtth2(hi_vo_dev dev, hi_u32 *vtth)
{
    hi_vo_mod_param *vo_mod_param = HI_NULL;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(vtth);

    if (vou_drv_is_support_save_buf_mode() == HI_FALSE) {
        vo_err_trace("not support getting vtth2!\n");
        return HI_ERR_VO_NOT_SUPPORT;
    }

    vo_down_sem_return();

    if (dev >= VO_MAX_PHY_DEV_NUM) {
        vo_err_trace("dev(%d) is not physical device!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_INVALID_DEVID;
    }

    vo_mod_param = vo_get_vo_mod_param();
    if (vo_mod_param->save_buf_mode[dev] == HI_FALSE) {
        vo_err_trace("save_buf_mode must be true\n");
        vo_up_sem();
        return HI_ERR_VO_NOT_PERMIT;
    }

    if (!(vo_drv_is_phy_dev_hd(dev) || vo_drv_is_phy_dev_sd(dev))) {
        vo_err_trace("invalid dev id %d! should set vtth for physic vo dev\n", dev);
        vo_up_sem();
        return HI_ERR_VO_INVALID_DEVID;
    }

    vou_drv_get_dev_vtth2(dev, vtth);

    vo_up_sem();
    return HI_SUCCESS;
}

hi_s32 vou_set_user_intf_sync_info(hi_vo_dev dev, hi_vo_user_intfsync_info *user_info)
{
    hi_s32 ret;
    vo_dev_info *dev_ctx = HI_NULL;
    hi_u32 flags = 0;
    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(user_info);

    vo_down_sem_return();

    vo_dev_spin_lock(&flags);
    vo_drv_set_all_crg_clk(HI_TRUE);
    vo_dev_spin_unlock(&flags);

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
    if (vou_drv_is_virt_dev(dev)) {
        vo_err_trace("dev:%d is a virtual device, not support!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_NOT_PERMIT;
    }
#endif

    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->config != HI_TRUE) {
        vo_err_trace("vo device %d doesn't configured!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_NOT_CONFIG;
    }

    if (dev_ctx->vou_attr.intf_sync != VO_OUTPUT_USER) {
        vo_err_trace("vo device %d only support this in user sync.\n", dev);
        vo_up_sem();
        return HI_ERR_VO_NOT_SUPPORT;
    }

    if (dev_ctx->vo_enable == HI_TRUE) {
        vo_err_trace("vo device %d has enabled!\n", dev);
        vo_up_sem();
        return HI_ERR_VO_DEV_HAS_ENABLED;
    }

    ret = vo_drv_check_dev_user_sync_info(dev, user_info);
    if (ret != HI_SUCCESS) {
        vo_err_trace("VO%d's user sync info is illegal!\n", dev);
        vo_up_sem();
        return ret;
    }

    vo_drv_set_dev_user_intf_sync_attr(dev, &user_info->user_intf_sync_attr);

    vo_drv_set_dev_div(dev, user_info->dev_div);

    vo_drv_set_hdmi_div(dev, user_info->pre_div);

    vo_drv_set_clk_reverse(user_info->clk_reverse);

    vou_drv_set_user_sync_info(dev, user_info);
    (hi_void)memcpy_s(&dev_ctx->vo_user_sync_info, sizeof(hi_vo_user_intfsync_info),
                      user_info, sizeof(hi_vo_user_intfsync_info));
    vo_up_sem();
    return HI_SUCCESS;
}

static hi_void vou_reset_dev_vars(hi_vo_dev dev)
{
    vo_dev_info *dev_ctx = HI_NULL;
    dev_ctx = vo_get_dev_ctx(dev);

    (hi_void)memset_s(&dev_ctx->debug_info, sizeof(vo_debug_info), 0, sizeof(vo_debug_info));

    return;
}

static hi_void vo_enable_clk(hi_void)
{
    vo_set_low_power_ctrl_clk_en(HI_TRUE);
    vo_lpw_bus_reset(HI_FALSE);
    vo_drv_set_all_crg_clk(HI_TRUE);
}

static hi_s32 vo_enable_check(hi_vo_dev dev)
{
    vo_dev_info *dev_ctx = HI_NULL;
    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->config != HI_TRUE) {
        vo_err_trace("vo device %d doesn't configured!\n", dev);
        return HI_ERR_VO_DEV_NOT_CONFIG;
    }

    if (dev_ctx->vo_enable == HI_TRUE) {
        vo_err_trace("vo device %d has enabled!\n", dev);
        return HI_ERR_VO_DEV_HAS_ENABLED;
    }

    return HI_SUCCESS;
}

static hi_s32 vo_do_enable(hi_vo_dev dev)
{
    hi_s32 ret;

    ret = vou_drv_set_dev_clk(dev);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    vo_drv_open(dev);

    return HI_SUCCESS;
}

static hi_void vo_had_enable(hi_vo_dev dev)
{
    vo_dev_info *dev_ctx = HI_NULL;
    dev_ctx = vo_get_dev_ctx(dev);

    if (dev == HAL_DISP_CHANNEL_DHD0) {
        vo_drv_v_set_v0_zme_coef(VO_RM_COEF_MODE_TYP);
    }

    vo_drv_enable(dev);
    dev_ctx->vo_enable = HI_TRUE;
}

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
static hi_s32 vo_enable_virt_dev(hi_vo_dev dev)
{
    vo_dev_info *dev_ctx = HI_NULL;
    dev_ctx = vo_get_dev_ctx(dev);
    if (vou_drv_is_virt_dev(dev)) {
        dev_ctx->vo_enable = HI_TRUE;
        return HI_SUCCESS;
    }
    return HI_FAILURE;
}
#endif

hi_s32 vou_enable(hi_vo_dev dev)
{
    hi_s32 ret;
    hi_u32 flags = 0;

    vo_check_dev_id_return(dev);

    vo_down_sem_return();

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
    ret = vo_enable_virt_dev(dev);
    if (ret == HI_SUCCESS) {
        goto return_ret;
    }
#endif

    vo_dev_spin_lock(&flags);
    vo_enable_clk();
    vo_dev_spin_unlock(&flags);

    ret = vo_enable_check(dev);
    if (ret != HI_SUCCESS) {
        goto return_ret;
    }

    if (vou_drv_get_dev_enable(dev) == HI_TRUE) {
        vo_info_trace("vo device %d has enabled in uboot!\n", dev);
        vo_drv_dev_int_enable(dev, HI_TRUE);
        ret = HI_SUCCESS;
        goto load_detect;
    }

    ret = vo_do_enable(dev);
    if (ret != HI_SUCCESS) {
        goto return_ret;
    }

load_detect:
    vo_had_enable(dev);

return_ret:
    vo_up_sem();

    return ret;
}

#ifdef CONFIG_HI_VO_WBC
static hi_s32 vo_wbc_enable_check(hi_vo_dev dev)
{
    return HI_SUCCESS;
}
#endif

static hi_s32 vo_layer_enable_check(hi_vo_dev dev)
{
    return HI_SUCCESS;
}

static hi_s32 vo_disable_check(hi_vo_dev dev)
{
    hi_s32 ret;

    ret = vo_layer_enable_check(dev);
    if (ret != HI_SUCCESS) {
        return ret;
    }

#ifdef CONFIG_HI_VO_WBC
    ret = vo_wbc_enable_check(dev);
    if (ret != HI_SUCCESS) {
        return ret;
    }
#endif

    return HI_SUCCESS;
}

static hi_void vo_reset_vtth(hi_vo_dev dev)
{
    hi_vo_mod_param *vo_mod_param = HI_NULL;

    if (!vou_drv_is_virt_dev(dev)) {
        vou_drv_set_dev_default_vtth(dev);
    }

    vo_mod_param = vo_get_vo_mod_param();
    if ((dev < VO_MAX_PHY_DEV_NUM) && vo_mod_param->save_buf_mode[dev]) {
        vou_drv_set_dev_default_vtth2(dev);
    }
}

static hi_void vo_disable_clk(hi_void)
{
    hi_s32 i;
    vo_dev_info *dev_ctx = HI_NULL;

    for (i = 0; i < VO_MAX_PHY_DEV_NUM; i++) {
        dev_ctx = vo_get_dev_ctx(i);
        if (dev_ctx->vo_enable || (vou_drv_get_dev_enable(i) == HI_TRUE)) {
            break;
        }
    }

    if (i == VO_MAX_PHY_DEV_NUM) {
        vo_set_low_power_ctrl_clk_en(HI_FALSE);
        vo_drv_set_all_crg_clk(HI_FALSE);
    }
}

hi_s32 vou_disable(hi_vo_dev dev)
{
    hi_s32 ret;
    hi_u32 flags = 0;
    vo_dev_info *dev_ctx = HI_NULL;

    vo_check_dev_id_return(dev);

    vo_down_sem_return();

    vo_dev_spin_lock(&flags);
    vo_drv_set_all_crg_clk(HI_TRUE);
    vo_dev_spin_unlock(&flags);

    ret = vo_disable_check(dev);
    if (ret != HI_SUCCESS) {
        vo_up_sem();
        return ret;
    }

    if (!vou_drv_is_virt_dev(dev)) {
        vo_drv_close(dev);
    }

    vou_drv_int_clear(VOU_INTCLEAR_ALL);

    vo_drv_disable(dev);

    dev_ctx = vo_get_dev_ctx(dev);
    dev_ctx->config = HI_FALSE;
    dev_ctx->vo_enable = HI_FALSE;

    vou_reset_dev_vars(dev);

    vo_reset_vtth(dev);

    vo_dev_spin_lock(&flags);
    vo_disable_clk();
    vo_dev_spin_unlock(&flags);

    vo_up_sem();

    return HI_SUCCESS;
}

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
void vou_virt_dev_timer_func(unsigned long data)
{
    return;
}
#endif

hi_s32 vo_check_mod_param(const hi_vo_mod_param *mod_param)
{
    hi_u32 i = 0;

    vo_dev_info *dev_ctx = HI_NULL;

    if ((mod_param->transparent_transmit != HI_TRUE) && (mod_param->transparent_transmit != HI_FALSE)) {
        vo_err_trace("the transparent_transmit %d is illegal!\n", mod_param->transparent_transmit);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((mod_param->exit_dev != HI_TRUE) && (mod_param->exit_dev != HI_FALSE)) {
        vo_err_trace("the exit_dev %d is illegal!\n", mod_param->exit_dev);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((mod_param->wbc_bg_black_en != HI_TRUE) && (mod_param->wbc_bg_black_en != HI_FALSE)) {
        vo_err_trace("the wbc_bg_black_en %d is illegal!\n", mod_param->wbc_bg_black_en);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((mod_param->dev_clk_ext_en != HI_TRUE) && (mod_param->dev_clk_ext_en != HI_FALSE)) {
        vo_err_trace("the dev_clk_ext_en %d is illegal!\n", mod_param->dev_clk_ext_en);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    for (i = 0; i < VO_MAX_PHY_DEV_NUM; i++) {
        vo_down_sem_return();

        dev_ctx = vo_get_dev_ctx(i);
        if (dev_ctx->vo_enable == HI_TRUE) {
            vo_err_trace("vo device %u has enabled!\n", i);
            vo_up_sem();
            return HI_ERR_VO_DEV_HAS_ENABLED;
        }

        vo_up_sem();

        if (vou_drv_is_support_save_buf_mode()) {
            if ((mod_param->save_buf_mode[i] != HI_TRUE) && (mod_param->save_buf_mode[i] != HI_FALSE)) {
                vo_err_trace("the save_buf_mode[%u] %d is illegal!\n", i, mod_param->save_buf_mode[i]);
                return HI_ERR_VO_ILLEGAL_PARAM;
            }
        } else {
            if (mod_param->save_buf_mode[i] != HI_FALSE) {
                vo_err_trace("the chip not support save buf mode, save_buf_mode[%u]:%d should be HI_FALSE!\n", i,
                             mod_param->save_buf_mode[i]);
                return HI_ERR_VO_ILLEGAL_PARAM;
            }
        }
    }
    return HI_SUCCESS;
}

hi_s32 vo_set_mod_param(const hi_vo_mod_param *mod_param)
{
    hi_u32 flags = 0;
    hi_s32 ret;
    hi_vo_mod_param *vo_mod_param = vo_get_vo_mod_param();

    vo_check_null_ptr_return(mod_param);

    ret = vo_check_mod_param(mod_param);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    vo_down_sem_return();

    vo_mod_param->transparent_transmit = mod_param->transparent_transmit;
    vo_mod_param->exit_dev = mod_param->exit_dev;
    vo_mod_param->wbc_bg_black_en = mod_param->wbc_bg_black_en;
    vo_mod_param->dev_clk_ext_en = mod_param->dev_clk_ext_en;
    (hi_void)memcpy_s(&vo_mod_param->save_buf_mode, sizeof(hi_bool) * VO_MAX_PHY_DEV_NUM,
        &mod_param->save_buf_mode, sizeof(hi_bool) * VO_MAX_PHY_DEV_NUM);
    vo_dev_spin_lock(&flags);
    vo_drv_transparent_transmit_setting(vo_mod_param->transparent_transmit);
    vo_dev_spin_unlock(&flags);

    vo_up_sem();

    return HI_SUCCESS;
}

hi_s32 vo_get_mod_param(hi_vo_mod_param *mod_param)
{
    vo_check_null_ptr_return(mod_param);

    vo_down_sem_return();
    (hi_void)memcpy_s(mod_param, sizeof(hi_vo_mod_param), vo_get_vo_mod_param(), sizeof(hi_vo_mod_param));
    vo_up_sem();

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

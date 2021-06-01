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

#include "drm_hal_display.h"
#include <linux/module.h>
#include <linux/slab.h>
#include "vou_chn.h"
#include "vou_graphics.h"
#include "hi_drm_func_ext.h"

#define COLOR_BLUE 0x0000FF

#define hal_disp_check_open(chan_param) do { \
    if (chan_param->open_cnt <= 0) { \
        drm_hal_err("disp channel not opened!\n"); \
        return -1; \
    } \
} while (0)

#define hal_disp_check_ptr(ptr) do { \
    if (ptr == NULL) { \
        drm_hal_err("null ptr!\n"); \
        return -1; \
    } \
} while (0)

struct disp_chan_params {
    int open_cnt;
    hi_vo_dev vo_dev;
    hi_vo_pub_attr vo_attr;
    hi_vo_user_intfsync_info user_info;
    hi_graphic_layer layer;
};

struct hal_disp_dev_params {
    struct disp_chan_params chan_param[DRM_HAL_DISP_MAX];
};

static struct drm_hal_disp_dev g_disp_dev = {0};

static hi_vo_dev disp_chan_to_dev(enum drm_hal_disp_chn disp)
{
    switch (disp) {
        case DRM_HAL_DISP_0:
            return 0;
        case DRM_HAL_DISP_1:
            return 1;
        default:
            drm_hal_err("invalid disp chn: %u\n", disp);
            return -1;
    }
    return -1;
}

static hi_graphic_layer disp_layer_translate(enum drm_hal_gfx_layer layer)
{
    switch (layer) {
        case DRM_HAL_GFX_G0:
            return 0;
        case DRM_HAL_GFX_G1:
            return 1;
        case DRM_HAL_GFX_G3:
            return 2; /* 2:G3 */
        default:
            drm_hal_err("invalid layer id: %u\n", layer);
            return -1;
    }
}

static struct disp_chan_params *disp_get_chan_param(enum drm_hal_disp_chn disp)
{
    int ret;
    if (disp >= DRM_HAL_DISP_MAX) {
        return NULL;
    }
    /* has init */
    if (g_disp_dev.params != NULL) {
        return &(((struct hal_disp_dev_params *)g_disp_dev.params)->chan_param[disp]);
    }

    g_disp_dev.params = kzalloc(sizeof(struct hal_disp_dev_params), GFP_KERNEL);
    if (g_disp_dev.params == NULL) {
        drm_hal_err("kzalloc error!\n");
        return NULL;
    }

    ret = vo_graphics_init();
    if (ret != 0) {
        drm_hal_err("vo_graphics_init err, ret=%#x\n", ret);
        kfree(g_disp_dev.params);
        g_disp_dev.params = NULL;
        return NULL;
    }

    ret = osal_spin_lock_init(&(g_disp_dev.lock));
    if (ret < 0) {
        drm_hal_err("spinlock init failed!\n");
        (void)vo_graphics_de_init();
        kfree(g_disp_dev.params);
        g_disp_dev.params = NULL;
        return NULL;
    }
    return &(((struct hal_disp_dev_params *)g_disp_dev.params)->chan_param[disp]);
}

static void drm_hal_disp_deinit(void)
{
    (void)vo_graphics_de_init();
    if (g_disp_dev.params != NULL) {
        kfree(g_disp_dev.params);
        g_disp_dev.params = NULL;
    }
    osal_spin_lock_destroy(&(g_disp_dev.lock));
    return;
}

static int drm_hal_disp_open(enum drm_hal_disp_chn disp)
{
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    if (param->open_cnt > 0) {
        drm_hal_info("already opened, cnt=%d\n", param->open_cnt);
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return 0;
    }

    param->vo_dev = disp_chan_to_dev(disp);

    if (param->vo_dev == -1) {
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    param->open_cnt++;
    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_disp_close(enum drm_hal_disp_chn disp)
{
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        drm_hal_info("already closed, cnt=%d\n", param->open_cnt);
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return 0;
    }

    param->open_cnt--;
    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_disp_enable(enum drm_hal_disp_chn disp)
{
    int ret;
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        drm_hal_err("disp channel not opened!\n");
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    drm_hal_info("intf %d, %u\n", param->vo_attr.intf_sync, param->vo_attr.intf_type);

    param->vo_attr.bg_color = COLOR_BLUE;

    ret = vou_set_pub_attr(param->vo_dev, &param->vo_attr);
    if (ret != 0) {
        drm_hal_err("vou_set_pub_attr err, ret=%#x\n", ret);
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return ret;
    }

    if (param->vo_attr.intf_type == VO_INTF_MIPI && param->vo_attr.intf_sync == VO_OUTPUT_USER) {
        ret = vou_set_user_intf_sync_info(param->vo_dev, &(param->user_info));
        if (ret != 0) {
            drm_hal_err("vou_set_user_intf_sync_info err, ret=%#x\n", ret);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return ret;
        }
    }

    ret = vou_enable(param->vo_dev);
    if (ret != 0) {
        drm_hal_err("vou_enable err, ret=%#x\n", ret);
    }

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return ret;
}

static int drm_hal_disp_disable(enum drm_hal_disp_chn disp)
{
    int ret;
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        drm_hal_err("disp channel not opened!\n");
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    ret = vou_disable(param->vo_dev);
    if (ret != 0) {
        drm_hal_err("vou_disable err, ret=%#x\n", ret);
    }

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return ret;
}

static int drm_hal_disp_get_attr(enum drm_hal_disp_chn disp, enum drm_hal_disp_attr_type type, const void *param)
{
    (void)disp;
    (void)type;
    (void)param;
    return 0;
}

struct hal_disp_timing_map {
    enum drm_hal_timing_fmt hal_fmt;
    VO_INTF_SYNC_E vo_fmt;
};

struct hal_disp_timing_map g_hal_disp_timing_map[] = {
    {DRM_HAL_TIMING_FMT_1080P_60, VO_OUTPUT_1080P60},
    {DRM_HAL_TIMING_FMT_1080P_50, VO_OUTPUT_1080P50},
    {DRM_HAL_TIMING_FMT_1080P_30, VO_OUTPUT_1080P30},
    {DRM_HAL_TIMING_FMT_1080P_25, VO_OUTPUT_1080P25},
    {DRM_HAL_TIMING_FMT_1080P_24, VO_OUTPUT_1080P24},
    {DRM_HAL_TIMING_FMT_1080I_60, VO_OUTPUT_1080I60},
    {DRM_HAL_TIMING_FMT_1080I_50, VO_OUTPUT_1080I50},
    {DRM_HAL_TIMING_FMT_720P_60, VO_OUTPUT_720P60},
    {DRM_HAL_TIMING_FMT_720P_50, VO_OUTPUT_720P50},
    {DRM_HAL_TIMING_FMT_576P_50, VO_OUTPUT_576P50},
    {DRM_HAL_TIMING_FMT_480P_60, VO_OUTPUT_480P60},
    {DRM_HAL_TIMING_FMT_PAL, VO_OUTPUT_PAL},
    {DRM_HAL_TIMING_FMT_NTSC, VO_OUTPUT_NTSC},
    {DRM_HAL_TIMING_FMT_VESA_800X600_60, VO_OUTPUT_800x600_60},
    {DRM_HAL_TIMING_FMT_VESA_1024X768_60, VO_OUTPUT_1024x768_60},
    {DRM_HAL_TIMING_FMT_VESA_1280X800_60, VO_OUTPUT_1280x800_60},
    {DRM_HAL_TIMING_FMT_VESA_1280X1024_60, VO_OUTPUT_1280x1024_60},
    {DRM_HAL_TIMING_FMT_VESA_1366X768_60, VO_OUTPUT_1366x768_60},
    {DRM_HAL_TIMING_FMT_VESA_1440X900_60, VO_OUTPUT_1440x900_60},
    {DRM_HAL_TIMING_FMT_VESA_1600X1200_60, VO_OUTPUT_1600x1200_60},
    {DRM_HAL_TIMING_FMT_VESA_1680X1050_60, VO_OUTPUT_1680x1050_60},
    {DRM_HAL_TIMING_FMT_VESA_1920X1200_60, VO_OUTPUT_1920x1200_60},
    {DRM_HAL_TIMING_FMT_2560X1440_60, VO_OUTPUT_2560x1440_60},
    {DRM_HAL_TIMING_FMT_2560X1600_60, VO_OUTPUT_2560x1600_60},
    {DRM_HAL_TIMING_FMT_3840X2160P_24, VO_OUTPUT_3840x2160_24},
    {DRM_HAL_TIMING_FMT_3840X2160P_25, VO_OUTPUT_3840x2160_25},
    {DRM_HAL_TIMING_FMT_3840X2160P_30, VO_OUTPUT_3840x2160_30},
    {DRM_HAL_TIMING_FMT_3840X2160P_50, VO_OUTPUT_3840x2160_50},
    {DRM_HAL_TIMING_FMT_3840X2160P_60, VO_OUTPUT_3840x2160_60},
    {DRM_HAL_TIMING_FMT_4096X2160P_24, VO_OUTPUT_4096x2160_24},
    {DRM_HAL_TIMING_FMT_4096X2160P_25, VO_OUTPUT_4096x2160_25},
    {DRM_HAL_TIMING_FMT_4096X2160P_30, VO_OUTPUT_4096x2160_30},
    {DRM_HAL_TIMING_FMT_4096X2160P_50, VO_OUTPUT_4096x2160_50},
    {DRM_HAL_TIMING_FMT_4096X2160P_60, VO_OUTPUT_4096x2160_60},
    {DRM_HAL_TIMING_FMT_USER, VO_OUTPUT_USER}
};

static void disp_timing_translate(enum drm_hal_timing_fmt *hal_fmt, hi_vo_intf_sync *vo_fmt)
{
    int i;
    for (i = 0; i < sizeof(g_hal_disp_timing_map) / sizeof(struct hal_disp_timing_map); i++) {
        if (g_hal_disp_timing_map[i].hal_fmt == *hal_fmt) {
            break;
        }
    }

    /* can not find, use the default */
    if (i == sizeof(g_hal_disp_timing_map) / sizeof(struct hal_disp_timing_map)) {
        drm_hal_err("can not find the matched format, hal_fmt = %u!\n", *hal_fmt);
        *vo_fmt = VO_OUTPUT_1080P60;
        return;
    }

    drm_hal_info("get matched fmt: %d\n", i);

    *vo_fmt = g_hal_disp_timing_map[i].vo_fmt;
}

static void disp_set_timing(struct disp_chan_params *chan_param, enum drm_hal_timing_fmt *hal_fmt)
{
    disp_timing_translate(hal_fmt, &chan_param->vo_attr.intf_sync);
}

static int drm_hal_disp_set_attr(enum drm_hal_disp_chn disp, enum drm_hal_disp_attr_type type, const void *param)
{
    int ret = 0;
    unsigned long lock_flag;
    struct disp_chan_params *chan_param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);
    hal_disp_check_ptr(chan_param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    /* user timing config,from hos L1 */
    if (*(enum drm_hal_timing_fmt *)param == DRM_HAL_TIMING_FMT_USER) {
        chan_param->vo_attr.sync_info.synm = 0;
        chan_param->vo_attr.sync_info.iop = 1;
        chan_param->vo_attr.sync_info.intfb = 0;
        chan_param->vo_attr.sync_info.vact = 960; /* 960 vact */
        chan_param->vo_attr.sync_info.vbb = 16; /* 16 vbb */
        chan_param->vo_attr.sync_info.vfb = 16; /* 16 vfb */
        chan_param->vo_attr.sync_info.hact = 480; /* 480 hact */
        chan_param->vo_attr.sync_info.hbb = 30; /* 30 hbb */
        chan_param->vo_attr.sync_info.hfb = 20; /* 20 hfb */
        chan_param->vo_attr.sync_info.hmid = 1;
        chan_param->vo_attr.sync_info.bvact = 1;
        chan_param->vo_attr.sync_info.bvbb = 1;
        chan_param->vo_attr.sync_info.bvfb = 1;
        chan_param->vo_attr.sync_info.hpw = 10; /* 10 hpw */
        chan_param->vo_attr.sync_info.vpw = 2; /* 2 vpw */
        chan_param->vo_attr.sync_info.idv = 0;
        chan_param->vo_attr.sync_info.ihs = 0;
        chan_param->vo_attr.sync_info.ivs = 0;
    }

    switch (type) {
        case DRM_HAL_DISP_ATTR_TIMING:
            disp_set_timing(chan_param, (enum drm_hal_timing_fmt *)param);
            break;
        default:
            drm_hal_err("attr type error: %#x\n", type);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return -1;
    }

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return ret;
}

int drm_hal_disp_bind_layer(enum drm_hal_disp_chn disp, enum drm_hal_gfx_layer layer)
{
    int ret = 0;
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        drm_hal_err("disp channel not opened!\n");
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    param->layer = disp_layer_translate(layer);
    if (param->layer == -1) {
        drm_hal_err("layer %u is invalid\n", layer);
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    if (DRM_HAL_GFX_G3 == layer) {
        ret = vou_graphics_un_bind_dev(param->layer, param->vo_dev);
        if (ret != 0) {
            drm_hal_err("vou_graphics_bind_dev err, ret=%#x\n", ret);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return ret;
        }

        ret = vou_graphics_bind_dev(param->layer, param->vo_dev);
        if (ret != 0) {
            drm_hal_err("vou_graphics_bind_dev err, ret=%#x\n", ret);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return ret;
        }
    }

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return ret;
}

int drm_hal_disp_unbind_layer(enum drm_hal_disp_chn disp, enum drm_hal_gfx_layer layer)
{
    int ret = 0;
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        drm_hal_err("disp channel not opened!\n");
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    if (param->layer != disp_layer_translate(layer)) {
        drm_hal_err("layer %u is invalid\n", layer);
        osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
        return -1;
    }

    if (DRM_HAL_GFX_G3 == layer) {
        ret = vou_graphics_un_bind_dev(param->layer, param->vo_dev);
        if (ret != 0) {
            drm_hal_err("vou_graphics_bind_dev err, ret=%#x\n", ret);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return ret;
        }
    }

    param->layer = -1;

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return ret;
}


static int drm_hal_disp_attach_intf(enum drm_hal_disp_chn disp, enum drm_hal_disp_intf_type intf, const void *intf_para)
{
    struct disp_chan_params *param = disp_get_chan_param(disp);
    unsigned long lock_flag;
    hal_disp_check_ptr(param);
    (void)intf_para;

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);

    switch (intf) {
        case DRM_HAL_DISP_INTF_HDMITX:
            param->vo_attr.intf_type = VO_INTF_HDMI;
            break;
        case DRM_HAL_DISP_INTF_MIPITX:
            param->vo_attr.intf_type = VO_INTF_MIPI;
            break;
        default:
            drm_hal_err("invalid interface %u!\n", intf);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return -1;
    }

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_disp_detach_intf(enum drm_hal_disp_chn disp, enum drm_hal_disp_intf_type intf, const void *intf_para)
{
    struct disp_chan_params *param = disp_get_chan_param(disp);
    unsigned long lock_flag;
    hal_disp_check_ptr(param);
    (void)intf_para;

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    switch (intf) {
        case DRM_HAL_DISP_INTF_HDMITX:
            param->vo_attr.intf_type = -1;
            break;
        case DRM_HAL_DISP_INTF_MIPITX:
            param->vo_attr.intf_type = -1;
            break;
        default:
            drm_hal_err("invalid interface %u!\n", intf);
            osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
            return -1;
    }

    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_disp_attach_user_intf_sync(enum drm_hal_disp_chn disp, hi_vo_user_intfsync_info *intf_sync_attr,
    const void *intf_para)
{
    unsigned long lock_flag;
    struct disp_chan_params *param = disp_get_chan_param(disp);
    hal_disp_check_ptr(param);
    hal_disp_check_ptr(intf_sync_attr);
    (void)intf_para;

    osal_spin_lock_irqsave(&(g_disp_dev.lock), &lock_flag);
    param->user_info.pre_div = intf_sync_attr->pre_div;
    param->user_info.dev_div = intf_sync_attr->dev_div;
    param->user_info.clk_reverse = intf_sync_attr->clk_reverse;
    param->user_info.user_intf_sync_attr.clk_source = intf_sync_attr->user_intf_sync_attr.clk_source;
    param->user_info.user_intf_sync_attr.user_sync_pll.fbdiv = intf_sync_attr->user_intf_sync_attr.user_sync_pll.fbdiv;
    param->user_info.user_intf_sync_attr.user_sync_pll.frac = intf_sync_attr->user_intf_sync_attr.user_sync_pll.frac;
    param->user_info.user_intf_sync_attr.user_sync_pll.refdiv =
        intf_sync_attr->user_intf_sync_attr.user_sync_pll.refdiv;
    param->user_info.user_intf_sync_attr.user_sync_pll.postdiv1 =
        intf_sync_attr->user_intf_sync_attr.user_sync_pll.postdiv1;
    param->user_info.user_intf_sync_attr.user_sync_pll.postdiv2 =
        intf_sync_attr->user_intf_sync_attr.user_sync_pll.postdiv2;
    osal_spin_unlock_irqrestore(&(g_disp_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_disp_set_csc(hi_u32 layer, hi_vo_csc *csc)
{
    return vou_graphics_set_csc(layer, csc);
}

static int drm_hal_disp_get_csc(hi_u32 layer, hi_vo_csc *csc)
{
    return vou_graphics_get_csc(layer, csc);
}

hi_void drm_hal_disp_dev_register(hi_void)
{
    hi_drm_export_func *drm_ext_func = HI_NULL;
    drm_ext_func = drm_export_func_register();
    if (drm_ext_func == HI_NULL) {
        drm_hal_err("err:drm_ext_func is NULL!\n");
        return;
    }

    drm_ext_func->disp_func = &g_disp_dev;
    g_disp_dev.deinit = drm_hal_disp_deinit;
    g_disp_dev.open = drm_hal_disp_open;
    g_disp_dev.close = drm_hal_disp_close;
    g_disp_dev.enable = drm_hal_disp_enable;
    g_disp_dev.disable = drm_hal_disp_disable;
    g_disp_dev.get_attr = drm_hal_disp_get_attr;
    g_disp_dev.set_attr = drm_hal_disp_set_attr;
    g_disp_dev.bind_layer = drm_hal_disp_bind_layer;
    g_disp_dev.unbind_layer = drm_hal_disp_unbind_layer;
    g_disp_dev.attach_intf = drm_hal_disp_attach_intf;
    g_disp_dev.detach_intf = drm_hal_disp_detach_intf;
    g_disp_dev.attach_user_intf_sync = drm_hal_disp_attach_user_intf_sync;
    g_disp_dev.set_csc = drm_hal_disp_set_csc;
    g_disp_dev.get_csc = drm_hal_disp_get_csc;

    return;
}

hi_void drm_hal_disp_dev_unregister(hi_void)
{
    hi_drm_export_func *drm_ext_func = HI_NULL;
    drm_ext_func = drm_export_func_register();
    if (drm_ext_func == HI_NULL) {
        drm_hal_err("err:drm_ext_func is NULL!\n");
        return;
    }

    g_disp_dev.deinit = NULL;
    g_disp_dev.open = NULL;
    g_disp_dev.close = NULL;
    g_disp_dev.enable = NULL;
    g_disp_dev.disable = NULL;
    g_disp_dev.get_attr = NULL;
    g_disp_dev.set_attr = NULL;
    g_disp_dev.bind_layer = NULL;
    g_disp_dev.unbind_layer = NULL;
    g_disp_dev.attach_intf = NULL;
    g_disp_dev.detach_intf = NULL;
    g_disp_dev.attach_user_intf_sync = NULL;
    g_disp_dev.set_csc = NULL;
    g_disp_dev.get_csc = NULL;
    drm_ext_func->disp_func = NULL;

    return;
}


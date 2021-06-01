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

#include "drm_hal_gfx.h"
#include <linux/module.h>
#include "hifb_vou_drv.h"
#include "hifb_vou_graphics.h"
#include "hifb_main.h"
#include "hi_drm_func_ext.h"

#define hal_gfx_check_open(layer_param) do {     \
    if (layer_param == NULL || layer_param->open_cnt <= 0) { \
        drm_hal_err("layer not opened!\n"); \
        return -1; \
    } \
} while (0)

#define hal_gfx_check_ptr(ptr) do {     \
    if (ptr == NULL) { \
        drm_hal_err("null ptr!\n"); \
        return -1; \
    } \
} while (0)

struct gfx_layer_params {
    unsigned int layer_id;
    int open_cnt;
    drm_hal_gfx_cb gfx_cb;
};

struct hal_gfx_dev_params {
    hifb_drv_ops gfx_api;
    struct gfx_layer_params layer_param[DRM_HAL_GFX_MAX];
};

static struct drm_hal_gfx_dev gfx_dev = {0};

static hifb_drv_ops *gfx_get_api(void)
{
    if (gfx_dev.params == NULL) {
        return NULL;
    }
    return &(((struct hal_gfx_dev_params *)gfx_dev.params)->gfx_api);
}

static struct gfx_layer_params *gfx_get_layer_param(enum drm_hal_gfx_layer layer)
{
    int ret;

    if (layer >= DRM_HAL_GFX_MAX) {
        return NULL;
    }
    /* has init */
    if (gfx_dev.params != NULL) {
        return &(((struct hal_gfx_dev_params *)gfx_dev.params)->layer_param[layer]);
    }

    gfx_dev.params = kzalloc(sizeof(struct hal_gfx_dev_params), GFP_KERNEL);
    if (gfx_dev.params == NULL) {
        drm_hal_err("kzalloc err!\n");
        return NULL;
    }

    hifb_drv_get_ops(gfx_get_api());
    /*
     * can not init here, because hifb ko has inited
     * gfx_api->hifb_drv_init()
     */
    ret = osal_spin_lock_init(&(gfx_dev.lock));
    if (ret < 0) {
        drm_hal_err("spinlock init failed!\n");
        kfree(gfx_dev.params);
        gfx_dev.params = NULL;
        return NULL;
    }

    return &(((struct hal_gfx_dev_params *)gfx_dev.params)->layer_param[layer]);
}

static int gfx_layer_translate(enum drm_hal_gfx_layer layer)
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
    return -1;
}

static void drm_hal_gfx_deinit(void)
{
    /*
     * can not deinit here, because hifb ko has deinited
     * gfx_api->hifb_drv_deinit()
     */
    if (gfx_dev.params != NULL) {
        kfree(gfx_dev.params);
        gfx_dev.params = NULL;
    }
    osal_spin_lock_destroy(&(gfx_dev.lock));
    return;
}

static int drm_hal_gfx_open(enum drm_hal_gfx_layer layer)
{
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    int layer_id;
    hal_gfx_check_ptr(param);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt > 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_info("already opened, cnt=%d\n", param->open_cnt);
        return 0;
    }

    layer_id = gfx_layer_translate(layer);
    if (layer_id == -1) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        return -1;
    }

    param->layer_id = layer_id;
    param->open_cnt++;
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_gfx_close(enum drm_hal_gfx_layer layer)
{
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    hal_gfx_check_ptr(param);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_info("already closed, cnt=%d\n", param->open_cnt);
        return 0;
    }

    param->open_cnt--;
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return 0;
}


static int gfx_callback(hi_u32 layer_id, hifb_main_intf_type type, void *para)
{
    int ret = 0;
    int i;
    unsigned long lock_flag;
    enum drm_hal_gfx_layer layer;
    struct hal_gfx_dev_params *dev_param = (struct hal_gfx_dev_params *)gfx_dev.params;
    hal_gfx_check_ptr(dev_param);

    switch (layer_id) {
        case 0:
            layer = DRM_HAL_GFX_G0;
            break;
        case 1:
            layer = DRM_HAL_GFX_G1;
            break;
        case 2:
            layer = DRM_HAL_GFX_G3;
            break;
        default:
            return -1;
    }
    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    for (i = 0; i < DRM_HAL_GFX_MAX; i++) {
        if ((dev_param->layer_param[i].layer_id == layer_id) &&
            (dev_param->layer_param[i].open_cnt > 0) &&
            (dev_param->layer_param[i].gfx_cb != NULL)) {
            ret = dev_param->layer_param[i].gfx_cb(layer, DRM_HAL_GFX_CB_INTR_100, NULL);
            break;
        }
    }

    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return ret;
}

static int drm_hal_gfx_enable(enum drm_hal_gfx_layer layer)
{
    int ret;
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    hifb_drv_ops *gfx_api = gfx_get_api();
    hal_gfx_check_ptr(param);
    hal_gfx_check_ptr(gfx_api);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("layer not opened!\n");
        return -1;
    }

    /* this api should be called after vo enable, so we put it here */
    ret = vou_graphics_init();
    if (ret != 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("err, ret=%#x\n", ret);
        return ret;
    }

    ret = gfx_api->hifb_open_layer(param->layer_id);
    if (ret != 0) {
        drm_hal_err("err, ret=%#x, id=%u\n", ret, param->layer_id);
        goto err_vou_gfx_deinit;
    }

    ret = gfx_api->hifb_drv_layer_default_setting(param->layer_id);
    if (ret != 0) {
        drm_hal_err("err, ret=%#x, id=%u\n", ret, param->layer_id);
        goto err_close_layer;
    }

    hifb_main_reg_callback(param->layer_id, HIFB_INTTYPE_VO, gfx_callback);

    ret = gfx_api->hifb_drv_enable_layer(param->layer_id, HI_TRUE);
    if (ret != 0) {
        drm_hal_err("err, ret=%#x\n", ret);
        goto err_unreg_callback;
    }
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return 0;

err_unreg_callback:
    hifb_main_reg_callback(param->layer_id, HIFB_INTTYPE_VO, HI_NULL);
err_close_layer:
    (void)gfx_api->hifb_close_layer(param->layer_id);
err_vou_gfx_deinit:
    (void)vou_graphics_deinit();
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return ret;
}

static int drm_hal_gfx_disable(enum drm_hal_gfx_layer layer)
{
    int ret;
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    hifb_drv_ops *gfx_api = gfx_get_api();
    hal_gfx_check_ptr(param);
    hal_gfx_check_ptr(gfx_api);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("layer not opened!\n");
        return -1;
    }

    ret = gfx_api->hifb_drv_enable_layer(param->layer_id, HI_FALSE);
    if (ret != 0) {
        drm_hal_err("err, ret=%#x\n", ret);
    }

    /* to be optimizing */
    hifb_main_reg_callback(param->layer_id, HIFB_INTTYPE_VO, HI_NULL);

    ret = gfx_api->hifb_close_layer(param->layer_id);
    if (ret != 0) {
        drm_hal_err("err, ret=%#x\n", ret);
    }

    ret = vou_graphics_deinit();
    if (ret != 0) {
        drm_hal_err("err, ret=%#x\n", ret);
    }

    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return ret;
}

static int drm_hal_gfx_get_attr(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_attr_type type, const void *param)
{
    (void)layer;
    (void)type;
    (void)param;
    return 0;
}

static int gfx_set_size(unsigned int id, struct drm_hal_rect *out_rect)
{
    HIFB_RECT in = {0};
    HIFB_RECT out;
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(gfx_api);

    out.x = out_rect->x;
    out.y = out_rect->y;
    out.w = out_rect->w;
    out.h = out_rect->h;

    return gfx_api->hifb_drv_set_layer_rect(id, &in, &out);
}

static int gfx_set_alpha(unsigned int id, unsigned int alpha)
{
    HIFB_ALPHA_S hifb_alpha = {0};
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(gfx_api);

    return gfx_api->hifb_drv_set_layer_alpha(id, hifb_alpha);
}

static int gfx_set_zpos(unsigned int id, unsigned int zpos)
{
    (void)id;
    (void)zpos;
    drm_hal_err("unsupported yet!\n");
    return -1;
}

static int gfx_set_format(unsigned int id, enum drm_hal_color_fmt fmt)
{
    HIFB_COLOR_FMT_E hifb_fmt;
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(gfx_api);
    hal_gfx_check_ptr(gfx_api->hifb_drv_set_layer_data_fmt);

    switch (fmt) {
        case DRM_HAL_FMT_ARGB8888:
            hifb_fmt = DRM_HAL_FMT_ARGB8888;
            break;
        case DRM_HAL_FMT_ARGB1555:
            hifb_fmt = DRM_HAL_FMT_ARGB1555;
            break;
        default:
            hifb_fmt = DRM_HAL_FMT_ARGB8888;
            break;
    }

    return gfx_api->hifb_drv_set_layer_data_fmt(id, hifb_fmt);
}

static int gfx_set_buffer(unsigned int id, unsigned long long addr)
{
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(gfx_api);

    return gfx_api->hifb_drv_set_layer_addr(id, addr);
}

static int gfx_set_stride(unsigned int id, unsigned int stride)
{
    hifb_drv_ops *gfx_api = gfx_get_api();
    hal_gfx_check_ptr(gfx_api);
    return gfx_api->hifb_drv_set_layer_stride(id, stride);
}

static int drm_hal_gfx_set_attr(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_attr_type type, const void *param)
{
    int ret;
    unsigned long lock_flag;
    struct gfx_layer_params *layer_param = gfx_get_layer_param(layer);
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(layer_param);
    hal_gfx_check_ptr(gfx_api);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (layer_param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("layer not opened!\n");
        return -1;
    }

    switch (type) {
        case DRM_HAL_GFX_ATTR_SIZE:
            ret = gfx_set_size(layer_param->layer_id, (struct drm_hal_rect *)param);
            break;
        case DRM_HAL_GFX_ATTR_ALPHA:
            ret = gfx_set_alpha(layer_param->layer_id, *(unsigned int *)param);
            break;
        case DRM_HAL_GFX_ATTR_ZPOS:
            ret = gfx_set_zpos(layer_param->layer_id, *(unsigned int *)param);
            break;
        case DRM_HAL_GFX_ATTR_FORMAT:
            ret = gfx_set_format(layer_param->layer_id, *(enum drm_hal_color_fmt *)param);
            break;
        case DRM_HAL_GFX_ATTR_STRIDE:
            ret = gfx_set_stride(layer_param->layer_id, *(unsigned int *)param);
            break;
        case DRM_HAL_GFX_ATTR_BUFFER:
            ret = gfx_set_buffer(layer_param->layer_id, *(unsigned long long *)param);
            break;
        default:
            drm_hal_err("attr type %d not support!\n", type);
            ret = -1;
    }

    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return ret;
}

static int drm_hal_gfx_register_cb(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_cb_type type, drm_hal_gfx_cb cb)
{
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(param);
    hal_gfx_check_ptr(gfx_api);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("layer not opened!\n");
        return -1;
    }

    param->gfx_cb = cb;
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_disp_unregister_cb(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_cb_type type)
{
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(param);
    hal_gfx_check_ptr(gfx_api);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("layer not opened!\n");
        return -1;
    }

    param->gfx_cb = NULL;
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return 0;
}

static int drm_hal_gfx_refresh(enum drm_hal_gfx_layer layer)
{
    int ret;
    unsigned long lock_flag;
    struct gfx_layer_params *param = gfx_get_layer_param(layer);
    hifb_drv_ops *gfx_api = gfx_get_api();

    hal_gfx_check_ptr(param);
    hal_gfx_check_ptr(gfx_api);

    osal_spin_lock_irqsave(&(gfx_dev.lock), &lock_flag);
    if (param->open_cnt <= 0) {
        osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
        drm_hal_err("layer not opened!\n");
        return -1;
    }

    ret = gfx_api->hifb_drv_updata_layer_reg(param->layer_id);
    if (ret != 0) {
        drm_hal_err("err, ret=%#x\n", ret);
    }
    osal_spin_unlock_irqrestore(&(gfx_dev.lock), &lock_flag);
    return 0;
}

hi_void drm_hal_gfx_dev_register(hi_void)
{
    hi_drm_export_func *drm_ext_func = HI_NULL;
    drm_ext_func = drm_export_func_register();
    if (drm_ext_func == HI_NULL) {
        drm_hal_err("err:drm_ext_func is NULL!\n");
        return;
    }
    drm_ext_func->gfx_func = &gfx_dev;
    gfx_dev.deinit = drm_hal_gfx_deinit;
    gfx_dev.open = drm_hal_gfx_open;
    gfx_dev.close = drm_hal_gfx_close;
    gfx_dev.enable = drm_hal_gfx_enable;
    gfx_dev.disable = drm_hal_gfx_disable;
    gfx_dev.get_attr = drm_hal_gfx_get_attr;
    gfx_dev.set_attr = drm_hal_gfx_set_attr;
    gfx_dev.register_cb = drm_hal_gfx_register_cb;
    gfx_dev.unregister_cb = drm_hal_disp_unregister_cb;
    gfx_dev.refresh = drm_hal_gfx_refresh;

    return;
}

hi_void drm_hal_gfx_dev_unregister(hi_void)
{
    hi_drm_export_func *drm_ext_func = HI_NULL;
    drm_ext_func = drm_export_func_register();
    if (drm_ext_func == HI_NULL) {
        drm_hal_err("err:drm_ext_func is NULL!\n");
        return;
    }

    gfx_dev.deinit = NULL;
    gfx_dev.open = NULL;
    gfx_dev.close = NULL;
    gfx_dev.enable = NULL;
    gfx_dev.disable = NULL;
    gfx_dev.get_attr = NULL;
    gfx_dev.set_attr = NULL;
    gfx_dev.register_cb = NULL;
    gfx_dev.unregister_cb = NULL;
    gfx_dev.refresh = NULL;
    drm_ext_func->gfx_func = NULL;

    return;
}


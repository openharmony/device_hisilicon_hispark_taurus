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

#ifndef _DRM_HAL_GFX_H__
#define _DRM_HAL_GFX_H__

#include "drm_hal_common.h"
#include "hi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define DRM_HAL_GFX_MAX_FORMAT_NUM 16

enum drm_hal_gfx_attr_type {
    DRM_HAL_GFX_ATTR_SIZE, /* param is pointer to struct drm_hal_rect */
    DRM_HAL_GFX_ATTR_ALPHA,
    DRM_HAL_GFX_ATTR_ZPOS,
    DRM_HAL_GFX_ATTR_FORMAT, /* param is pointer to enum drm_hal_color_fmt */
    DRM_HAL_GFX_ATTR_CS, /* color space */
    DRM_HAL_GFX_ATTR_STRIDE, /* param is pointer to unsigned int */
    DRM_HAL_GFX_ATTR_BUFFER, /* buffer address */
    DRM_HAL_GFX_ATTR_MAX
};

struct drm_hal_gfx_layer_capability {
    int available; /* does this layer available for this SOC */
    int max_w;
    int max_h;
    enum drm_hal_color_fmt formats[DRM_HAL_GFX_MAX_FORMAT_NUM]; /* supported formats of this layer */
    int format_num;
    enum drm_hal_disp_chn connected_disp_chn; /* which disp channel does this layer belongs to */
};

struct drm_hal_gfx_capability {
    struct drm_hal_gfx_layer_capability layer_cap[DRM_HAL_GFX_MAX];
};

enum drm_hal_gfx_cb_type {
    DRM_HAL_GFX_CB_INTR_0,
    DRM_HAL_GFX_CB_INTR_90,
    DRM_HAL_GFX_CB_INTR_100,
    DRM_HAL_GFX_CB_INTR_MAX
};

typedef int (*drm_hal_gfx_cb)(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_cb_type type, void *para);

struct drm_hal_gfx_dev {
    void (*deinit)(void);
    int (*open)(enum drm_hal_gfx_layer layer);
    int (*close)(enum drm_hal_gfx_layer layer);
    int (*enable)(enum drm_hal_gfx_layer layer);
    int (*disable)(enum drm_hal_gfx_layer layer);
    int (*get_attr)(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_attr_type type, const void *param);
    int (*set_attr)(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_attr_type type, const void *param);
    int (*register_cb)(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_cb_type type, drm_hal_gfx_cb cb);
    int (*unregister_cb)(enum drm_hal_gfx_layer layer, enum drm_hal_gfx_cb_type type);
    int (*refresh)(enum drm_hal_gfx_layer layer);
    void *params; /* for internal use */
    osal_spinlock_t lock;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _DRM_HAL_GFX_H__ */


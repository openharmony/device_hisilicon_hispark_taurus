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

#ifndef _DRM_HAL_DISPLAY_H__
#define _DRM_HAL_DISPLAY_H__

#include "drm_hal_common.h"
#include "hi_comm_vo_adapt.h"
#include "hi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum drm_hal_disp_attr_type {
    DRM_HAL_DISP_ATTR_TIMING, /* param: enum drm_hal_timing_fmt */
    DRM_HAL_DISP_ATTR_MAX
};

enum drm_hal_disp_intf_type {
    DRM_HAL_DISP_INTF_HDMITX,
    DRM_HAL_DISP_INTF_MIPITX,
    DRM_HAL_DISP_INTF_MAX
};

struct drm_hal_disp_capability {
    int channel_num;
};

struct drm_hal_disp_dev {
    void (*deinit)(void);
    int (*open)(enum drm_hal_disp_chn disp);
    int (*close)(enum drm_hal_disp_chn disp);
    int (*enable)(enum drm_hal_disp_chn disp);
    int (*disable)(enum drm_hal_disp_chn disp);
    int (*set_attr)(enum drm_hal_disp_chn disp, enum drm_hal_disp_attr_type type, const void *param);
    int (*get_attr)(enum drm_hal_disp_chn disp, enum drm_hal_disp_attr_type type, const void *param);
    int (*bind_layer)(enum drm_hal_disp_chn disp, enum drm_hal_gfx_layer layer);
    int (*unbind_layer)(enum drm_hal_disp_chn disp, enum drm_hal_gfx_layer layer);
    int (*attach_intf)(enum drm_hal_disp_chn disp, enum drm_hal_disp_intf_type intf, const void *intf_para);
    int (*detach_intf)(enum drm_hal_disp_chn disp, enum drm_hal_disp_intf_type intf, const void *intf_para);
    int (*attach_user_intf_sync)(enum drm_hal_disp_chn disp, hi_vo_user_intfsync_info *intf_sync_attr,
        const void *intf_para);
    int (*set_csc)(hi_u32 gfx_layer, hi_vo_csc *csc);
    int (*get_csc)(hi_u32 gfx_layer, hi_vo_csc *csc);
    void *params; /* for internal use */
    osal_spinlock_t lock;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _DRM_HAL_DISPLAY_H__ */

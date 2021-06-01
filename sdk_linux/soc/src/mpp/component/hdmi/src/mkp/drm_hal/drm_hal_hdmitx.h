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

#ifndef __HI_DRM_HAL_HDMITX_H__
#define __HI_DRM_HAL_HDMITX_H__

#include "drm_hal_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

struct drm_hal_hdmitx_attr {
    enum drm_hal_timing_fmt fmt;
};

enum drm_hal_hdmitx_status_type {
    DRM_HAL_HDMITX_STAT_CONNECTOR, /* param: enum drm_hal_hdmitx_connect_status */
    DRM_HAL_HDMITX_STAT_MAX
};

enum drm_hal_hdmitx_connect_status {
    DRM_HAL_HDMITX_CONNECTED,
    DRM_HAL_HDMITX_DISCONNECTED,
    DRM_HAL_HDMITX_UNKNOWN
};

struct drm_hal_hdmitx_capability {
    int hdmitx_num;
};

struct drm_hal_hdmitx_dev {
    void (*get_capability)(struct drm_hal_hdmitx_capability *cap);
    int (*open)(enum drm_hal_hdmitx_id id);
    int (*close)(enum drm_hal_hdmitx_id id);
    int (*enable)(enum drm_hal_hdmitx_id id);
    int (*disable)(enum drm_hal_hdmitx_id id);
    int (*get_attr)(enum drm_hal_hdmitx_id id, struct drm_hal_hdmitx_attr *attr);
    int (*set_attr)(enum drm_hal_hdmitx_id id, struct drm_hal_hdmitx_attr *attr);
    int (*get_status)(enum drm_hal_hdmitx_id id, enum drm_hal_hdmitx_status_type type,
        const enum drm_hal_hdmitx_connect_status *param);
    int (*read_edid)(enum drm_hal_hdmitx_id id, unsigned char *buffer, int buf_len);
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DRM_HAL_HDMITX_H__ */

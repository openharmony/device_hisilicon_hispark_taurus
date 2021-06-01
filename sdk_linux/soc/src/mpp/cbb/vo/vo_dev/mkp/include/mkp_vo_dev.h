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

#ifndef __MKP_VO_DEV_H__
#define __MKP_VO_DEV_H__

#include "mkp_ioctl.h"
#include "hi_common_adapt.h"
#include "hi_comm_video_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define GFX_MASK                           0x80000000

typedef enum {
    VO_LAYER_V0 = 0,
    VO_LAYER_V1 = 1,
    VO_LAYER_V2 = 2,
    VO_LAYER_V3 = 3,
    VO_LAYER_G0 = 4,
    VO_LAYER_G1 = 5,
    VO_LAYER_G2 = 6,
    VO_LAYER_G3 = 7,
    VO_LAYER_BUTT
} vo_layer_id;

#define vo_id_gfx_fd(gfx) (GFX_MASK | (((gfx)&0xf) << 28))

/* for mkp */
#define vo_get_dev(f) (((*((hi_u32 *)(f))) >> 16) & 0xff)
#define vo_get_gfx(f) ((((*((hi_u32 *)(f))) & (~(GFX_MASK))) >> 28) & 0xf)

#define vo_emerg_trace(fmt, ...) \
    HI_EMERG_TRACE(HI_ID_VO_DEV, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define vo_err_trace(fmt, ...) \
    HI_ERR_TRACE(HI_ID_VO_DEV, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define vo_warn_trace(fmt, ...) \
    HI_WARN_TRACE(HI_ID_VO_DEV, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define vo_info_trace(fmt, ...) \
    HI_INFO_TRACE(HI_ID_VO_DEV, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define vo_alert_trace(fmt, ...) \
    HI_ALERT_TRACE(HI_ID_VO_DEV, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define vo_debug_trace(fmt, ...) \
    HI_DEBUG_TRACE(HI_ID_VO_DEV, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

hi_s32 vo_def_check_dev_id(hi_vo_dev dev);

/* macro definition for check status */
#define vo_check_null_ptr_return(ptr)                                     \
    do {                                                           \
        if ((ptr) == HI_NULL) {                                       \
            vo_warn_trace("null ptr!\n");                          \
            return HI_ERR_VO_NULL_PTR;                             \
        }                                                          \
    } while (0)

/* vo device relative */
#define vo_check_dev_id_return(dev)                                \
    do {                                                    \
        if (vo_def_check_dev_id(dev) != HI_SUCCESS) {       \
            vo_err_trace("dev %d invalid!\n", dev);         \
            return HI_ERR_VO_INVALID_DEVID;                 \
        }                                                   \
    } while (0)

typedef enum {
    /* file operation */
    IOC_NR_VOU_SET_FD,
    IOC_NR_VOU_CLOSE_FD,

    /* device operation */
    IOC_NR_VOU_OPEN,
    IOC_NR_VOU_CLOSE,
    IOC_NR_VOU_PUB_ATTR_SET,
    IOC_NR_VOU_PUB_ATTR_GET,
    IOC_NR_VOU_FPS_SET,
    IOC_NR_VOU_FPS_GET,

    IOC_NR_VOU_VTTH_SET,
    IOC_NR_VOU_VTTH_GET,

    IOC_NR_VOU_USER_INTFSYNC_INFO_SET,

    /* graphics bind operation */
    IOC_NR_VOU_GFX_LAYER_BIND,
    IOC_NR_VOU_GFX_LAYER_UNBIND,
    IOC_NR_VOU_GFX_CSC_SET,
    IOC_NR_VOU_GFX_CSC_GET,

    /* module param settings */
    IOC_NR_VO_MOD_PARAM_SET,
    IOC_NR_VO_MOD_PARAM_GET,

    IOC_NR_VOU_VTTH2_SET,
    IOC_NR_VOU_VTTH2_GET,

    IOC_NR_VOU_BUTT
} ioc_nr_vou;

#define VOU_OPEN_CTRL                      _IO(IOC_TYPE_VOU_DEV, IOC_NR_VOU_OPEN)
#define VOU_CLOSE_CTRL                     _IO(IOC_TYPE_VOU_DEV, IOC_NR_VOU_CLOSE)
#define VOU_PUB_ATTR_SET_CTRL              _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_PUB_ATTR_SET, hi_vo_pub_attr)
#define VOU_PUB_ATTR_GET_CTRL              _IOR(IOC_TYPE_VOU_DEV, IOC_NR_VOU_PUB_ATTR_GET, hi_vo_pub_attr)

#define VOU_DEV_FPS_SET_CTRL               _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_FPS_SET, hi_u32)
#define VOU_DEV_FPS_GET_CTRL               _IOR(IOC_TYPE_VOU_DEV, IOC_NR_VOU_FPS_GET, hi_u32)

#define VOU_DEV_VTTH_SET_CTRL              _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_VTTH_SET, hi_u32)
#define VOU_DEV_VTTH_GET_CTRL              _IOR(IOC_TYPE_VOU_DEV, IOC_NR_VOU_VTTH_GET, hi_u32)
#define VOU_DEV_VTTH2_SET_CTRL             _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_VTTH2_SET, hi_u32)
#define VOU_DEV_VTTH2_GET_CTRL             _IOR(IOC_TYPE_VOU_DEV, IOC_NR_VOU_VTTH2_GET, hi_u32)

#define VOU_USER_INTFSYNC_INFO_SET_CTRL    _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_USER_INTFSYNC_INFO_SET, \
    hi_vo_user_intfsync_info)

/* the following IOCTL is used for vou fd operation */
#define VOU_DEV_SET_FD                     _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_SET_FD, hi_s32)
#define VOU_DEV_CLOSE_FD                   _IO(IOC_TYPE_VOU_DEV, IOC_NR_VOU_CLOSE_FD)

/* graphics layer bind-relation */
#define VOU_GFX_LAYER_BIND_CTRL            _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_GFX_LAYER_BIND, hi_s32)
#define VOU_GFX_LAYER_UNBIND_CTRL          _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_GFX_LAYER_UNBIND, hi_s32)
#define VOU_GFX_LAYER_CSC_SET_CTRL         _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VOU_GFX_CSC_SET, hi_vo_csc)
#define VOU_GFX_LAYER_CSC_GET_CTRL         _IOR(IOC_TYPE_VOU_DEV, IOC_NR_VOU_GFX_CSC_GET, hi_vo_csc)

/* module param settings */
#define VO_MOD_PARAM_SET_CTRL              _IOW(IOC_TYPE_VOU_DEV, IOC_NR_VO_MOD_PARAM_SET, hi_vo_mod_param)
#define VO_MOD_PARAM_GET_CTRL              _IOR(IOC_TYPE_VOU_DEV, IOC_NR_VO_MOD_PARAM_GET, hi_vo_mod_param)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __MKP_VO_DEV_H__ */

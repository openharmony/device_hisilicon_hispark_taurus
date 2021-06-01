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

#ifndef __HI_DRV_TDE_H__
#define __HI_DRV_TDE_H__

#include "drv_tde_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Use 't' as magic number */
#define TDE_IOC_MAGIC 't'

typedef struct {
    hi_u64 phy_addr; /* <Header address of a bitmap or the Y component */
    hi_tde_color_fmt color_fmt; /* <Color format */

    hi_u32 height; /* <Bitmap height */

    hi_u32 width; /* <Bitmap width */
    hi_u32 stride; /* <Stride of a bitmap or the Y component */

    /* <Address of the color look-up table (CLUT), for color extension or color correction */
    hi_u64 clut_phy_addr;

    hi_bool is_ycbcr_clut; /* <Whether the CLUT is in the YCbCr space. */
    hi_bool alpha_max_is_255; /* <The maximum alpha value of a bitmap is 255 or 128. */

    hi_bool support_alpha_ext_1555; /* <Whether to enable the alpha extension of an ARGB1555 bitmap. */
    hi_u8 alpha0; /* <Values of alpha0 and alpha1, used as the ARGB1555 format */
    hi_u8 alpha1; /* <Values of alpha0 and alpha1, used as the ARGB1555 format */
    hi_u64 cbcr_phy_addr; /* Address of the CbCr component, pilot */
    hi_u32 cbcr_stride; /* <Stride of the CbCr component, pilot */
} drv_tde_ioctl_surface;

/* blit info */
typedef struct {
    hi_s32 handle;                             /* < TDE handle */
    drv_tde_ioctl_surface back_ground_surface; /* < background surface */
    hi_tde_rect back_ground_rect;              /* < background surface operating rect */
    drv_tde_ioctl_surface fore_ground_surface; /* < foreground surface */
    hi_tde_rect fore_ground_rect;              /* < foreground surface operating rect */
    drv_tde_ioctl_surface dst_surface;         /* < target surface */
    hi_tde_rect dst_rect;                      /* < target surface operating rect */
    hi_tde_opt option;                         /* < operating option */
    hi_u32 null_indicator;                     /* < flag of mask , 1: valid ,0: invalid */
} drv_tde_bitblit_cmd;

/* color fill info */
typedef struct {
    hi_s32 handle;                             /* < TDE handle */
    drv_tde_ioctl_surface fore_ground_surface; /* < background surface */
    hi_tde_rect fore_ground_rect;              /* < background surface operating rect */
    drv_tde_ioctl_surface dst_surface;         /* < foreground surface */
    hi_tde_rect dst_rect;                      /* < foreground surface operating rect */
    hi_tde_fill_color fill_color;              /* < fill color */
    hi_tde_opt option;                         /* < operating option */
    hi_u32 null_indicator;                     /* < flag of mask , 1: valid ,0: invalid */
} drv_tde_solid_draw_cmd;

/* quick copy info */
typedef struct {
    hi_s32 handle;                     /* < TDE handle */
    drv_tde_ioctl_surface src_surface; /* < src surface */
    hi_tde_rect src_rect;              /* < src surface rect */
    drv_tde_ioctl_surface dst_surface; /* < target surface */
    hi_tde_rect dst_rect;              /* < target rect */
} drv_tde_quick_copy_cmd;

/* quick fill info */
typedef struct {
    hi_s32 handle;                     /* < TDE handle */
    drv_tde_ioctl_surface dst_surface; /* < target surface */
    hi_tde_rect dst_rect;              /* < target surface rect */
    hi_u32 fill_data;                  /* < fill color */
} drv_tde_quick_fill_cmd;

/* quick defilicker info */
typedef struct {
    hi_s32 handle;                     /* < TDE handle */
    drv_tde_ioctl_surface src_surface; /* < src surface */
    hi_tde_rect src_rect;              /* < src rect */
    drv_tde_ioctl_surface dst_surface; /* < target surface */
    hi_tde_rect dst_rect;              /* < target rect */
} drv_tde_quick_deflicker_cmd;

/* quick scale info */
typedef struct {
    hi_s32 handle;                     /* < TDE handle */
    drv_tde_ioctl_surface src_surface; /* < src surface */
    hi_tde_rect src_rect;              /* < src rect */
    drv_tde_ioctl_surface dst_surface; /* < target surface */
    hi_tde_rect dst_rect;              /* < target rect */
} drv_tde_quick_resize_cmd;

/* semi-planar YUV to RGB info */
typedef struct {
    hi_s32 handle;                     /* < TDE handle */
    hi_tde_mb_surface mb_src_surface;  /* < src surface */
    hi_tde_rect mb_src_rect;           /* < src rect */
    drv_tde_ioctl_surface dst_surface; /* < target surface */
    hi_tde_rect dst_rect;              /* < target rect */
    hi_tde_mb_opt mb_option;           /* < operating option */
} drv_tde_mb_blit_cmd;

/* mask blit rop blend info */
typedef struct {
    hi_s32 handle;                             /* < TDE handle */
    drv_tde_ioctl_surface back_ground_surface; /* < bk surface */
    hi_tde_rect back_ground_rect;              /* < bk rect */
    drv_tde_ioctl_surface fore_ground_surface; /* < fore surface */
    hi_tde_rect fore_ground_rect;              /* < fore rect */
    drv_tde_ioctl_surface mask_surface;        /* < mask surface  */
    hi_tde_rect mask_rect;                     /* < mask surface rect */
    drv_tde_ioctl_surface dst_surface;         /* < target surface */
    hi_tde_rect dst_rect;                      /* < target rect */
    hi_tde_rop_mode rop_color;                 /* < RGB rop type */
    hi_tde_rop_mode rop_alpha;                 /* < alpha rop type */
} drv_tde_bitmap_maskrop_cmd;

/* mask blit alpha blend info */
typedef struct {
    hi_s32 handle;                             /* < TDE handle */
    drv_tde_ioctl_surface back_ground_surface; /* < bk surface */
    hi_tde_rect back_ground_rect;              /* < bk rect */
    drv_tde_ioctl_surface fore_ground_surface; /* < fore surface */
    hi_tde_rect fore_ground_rect;              /* < fore rect */
    drv_tde_ioctl_surface mask_surface;        /* < mask surface  */
    hi_tde_rect mask_rect;                     /* < mask surface rect */
    drv_tde_ioctl_surface dst_surface;         /* < target surface */
    hi_tde_rect dst_rect;                      /* < target rect */
    hi_u8 alpha;                               /* < global alpha */
    hi_u8 reserved0;
    hi_u8 reserved1;
    hi_u8 reserved2;
    hi_tde_alpha_blending blend_mode; /* < blend opt */
} drv_tde_bitmap_maskblend_cmd;

/* pattern fill */
typedef struct {
    hi_s32 handle;                             /* < TDE handle */
    drv_tde_ioctl_surface back_ground_surface; /* < bk surface */
    hi_tde_rect back_ground_rect;              /* < bk rect */
    drv_tde_ioctl_surface fore_ground_surface; /* < fore surface */
    hi_tde_rect fore_ground_rect;              /* < fore rect */
    drv_tde_ioctl_surface dst_surface;         /* < target surface */
    hi_tde_rect dst_rect;                      /* < target rect */
    hi_tde_pattern_fill_opt option;            /* < option */
    hi_u32 null_indicator;                     /* < flag of mask , 1: valid ,0: invalid */
} drv_tde_pattern_fill_cmd;

typedef struct {
    hi_s32 handle;
    drv_tde_ioctl_surface src_surface;
    hi_tde_rect src_rect;
    drv_tde_ioctl_surface dst_surface;
    hi_tde_rect dst_rect;
    hi_tde_rotate_angle rotate_angle;
} tde_rotate_cmd;

#define TDE_BEGIN_JOB _IOR(TDE_IOC_MAGIC, 1, hi_s32)
#define TDE_BIT_BLIT _IOW(TDE_IOC_MAGIC, 2, drv_tde_bitblit_cmd)
#define TDE_SOLID_DRAW _IOW(TDE_IOC_MAGIC, 3, drv_tde_solid_draw_cmd)
#define TDE_QUICK_COPY _IOW(TDE_IOC_MAGIC, 4, drv_tde_quick_copy_cmd)
#define TDE_QUICK_RESIZE _IOW(TDE_IOC_MAGIC, 5, drv_tde_quick_resize_cmd)
#define TDE_QUICK_FILL _IOW(TDE_IOC_MAGIC, 6, drv_tde_quick_fill_cmd)
#define TDE_QUICK_DEFLICKER _IOW(TDE_IOC_MAGIC, 7, drv_tde_quick_deflicker_cmd)
#define TDE_MB_BITBLT _IOW(TDE_IOC_MAGIC, 8, drv_tde_mb_blit_cmd)
#define TDE_END_JOB _IOW(TDE_IOC_MAGIC, 9, drv_tde_end_job_cmd)
#define TDE_WAITFORDONE _IOW(TDE_IOC_MAGIC, 10, hi_s32)
#define TDE_CANCEL_JOB _IOW(TDE_IOC_MAGIC, 11, hi_s32)
#define TDE_BITMAP_MASKROP _IOW(TDE_IOC_MAGIC, 12, drv_tde_bitmap_maskrop_cmd)
#define TDE_BITMAP_MASKBLEND _IOW(TDE_IOC_MAGIC, 13, drv_tde_bitmap_maskblend_cmd)
#define TDE_WAITALLDONE _IO(TDE_IOC_MAGIC, 14)
#define TDE_RESET _IO(TDE_IOC_MAGIC, 15)
#define TDE_SET_DEFLICKERLEVEL _IOW(TDE_IOC_MAGIC, 17, hi_tde_deflicker_level)
#define TDE_GET_DEFLICKERLEVEL _IOR(TDE_IOC_MAGIC, 18, hi_tde_deflicker_level)
#define TDE_SET_ALPHATHRESHOLD_VALUE _IOW(TDE_IOC_MAGIC, 19, hi_u8)
#define TDE_GET_ALPHATHRESHOLD_VALUE _IOR(TDE_IOC_MAGIC, 20, hi_u8)
#define TDE_SET_ALPHATHRESHOLD_STATE _IOW(TDE_IOC_MAGIC, 21, hi_bool)
#define TDE_GET_ALPHATHRESHOLD_STATE _IOR(TDE_IOC_MAGIC, 22, hi_bool)
#define TDE_PATTERN_FILL _IOW(TDE_IOC_MAGIC, 23, drv_tde_pattern_fill_cmd)
#define TDE_ENABLE_REGIONDEFLICKER _IOW(TDE_IOC_MAGIC, 24, hi_bool)
#define TDE_ROTATE _IOW(TDE_IOC_MAGIC, 25, tde_rotate_cmd)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __HI_DRV_TDE_H__ */

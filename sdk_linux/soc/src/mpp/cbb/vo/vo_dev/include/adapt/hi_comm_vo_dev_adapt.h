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

#ifndef __HI_COMM_VO_DEV_ADAPT_H__
#define __HI_COMM_VO_DEV_ADAPT_H__

#include "hi_type.h"
#include "hi_comm_video_adapt.h"
#include "hi_comm_vo_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef EN_VOU_ERR_CODE_E hi_vou_err_code;

typedef hi_u32 hi_vo_intf_type;

typedef VO_INTF_SYNC_E hi_vo_intf_sync;

typedef VO_CSC_MATRIX_E hi_vo_csc_matrix;

typedef struct {
    hi_bool synm; /* RW; sync mode(0:timing,as BT.656; 1:signal,as LCD) */
    hi_bool iop; /* RW; interlaced or progressive display(0:i; 1:p) */
    hi_u8 intfb; /* RW; interlace bit width while output */

    hi_u16 vact; /* RW; vertical active area */
    hi_u16 vbb; /* RW; vertical back blank porch */
    hi_u16 vfb; /* RW; vertical front blank porch */

    hi_u16 hact; /* RW; horizontal active area */
    hi_u16 hbb; /* RW; horizontal back blank porch */
    hi_u16 hfb; /* RW; horizontal front blank porch */
    hi_u16 hmid; /* RW; bottom horizontal active area */

    hi_u16 bvact; /* RW; bottom vertical active area */
    hi_u16 bvbb; /* RW; bottom vertical back blank porch */
    hi_u16 bvfb; /* RW; bottom vertical front blank porch */

    hi_u16 hpw; /* RW; horizontal pulse width */
    hi_u16 vpw; /* RW; vertical pulse width */

    hi_bool idv; /* RW; inverse data valid of output */
    hi_bool ihs; /* RW; inverse horizontal synch signal */
    hi_bool ivs; /* RW; inverse vertical synch signal */
} hi_vo_sync_info;

typedef struct {
    hi_u32 bg_color; /* RW; background color of a device, in RGB format. */
    hi_vo_intf_type intf_type; /* RW; type of a VO interface */
    hi_vo_intf_sync intf_sync; /* RW; type of a VO interface timing */
    hi_vo_sync_info sync_info; /* RW; information about VO interface timings */
} hi_vo_pub_attr;

typedef struct {
    hi_vo_csc_matrix csc_matrix; /* CSC matrix */
    hi_u32 luma; /* RW; range:    [0, 100]; luminance, default: 50 */
    hi_u32 contrast; /* RW; range:    [0, 100]; contrast, default: 50 */
    hi_u32 hue; /* RW; range:    [0, 100]; hue, default: 50 */
    hi_u32 satuature; /* RW; range:    [0, 100]; satuature, default: 50 */
} hi_vo_csc;

typedef struct {
    hi_bool transparent_transmit; /* RW, range: [0, 1];  YC(luminance and chrominance) changes or not
                                            when passing through VO */
    hi_bool exit_dev;
    hi_bool wbc_bg_black_en;
    hi_bool dev_clk_ext_en;
    hi_bool save_buf_mode[VO_MAX_PHY_DEV_NUM]; /* save buff mode */
} hi_vo_mod_param;

typedef VO_CLK_SOURCE_E hi_vo_clk_source;

typedef struct {
    hi_u32 fbdiv;
    hi_u32 frac;
    hi_u32 refdiv;
    hi_u32 postdiv1;
    hi_u32 postdiv2;
} hi_vo_user_intfsync_pll;

typedef struct {
    hi_vo_clk_source clk_source;

    union {
        hi_vo_user_intfsync_pll user_sync_pll;
        hi_u32 lcd_m_clk_div;
    };
} hi_vo_user_intfsync_attr;

typedef struct {
    hi_vo_user_intfsync_attr user_intf_sync_attr;
    hi_u32 pre_div;
    hi_u32 dev_div;
    hi_bool clk_reverse;
} hi_vo_user_intfsync_info;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __HI_COMM_VO_DEV_ADAPT_H__ */

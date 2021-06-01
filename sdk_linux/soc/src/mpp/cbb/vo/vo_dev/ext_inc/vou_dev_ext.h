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

#ifndef __HI_VOU_DEV_EXT_H__
#define __HI_VOU_DEV_EXT_H__

#include <hi_common.h>
#include "hi_comm_vo_adapt.h"
#include "hi_comm_video_adapt.h"
typedef enum {
    DITHER_OUT_BITWIDTH_8 = 0x0, /* dither output 8bit */
    DITHER_OUT_BITWIDTH_10 = 0x1, /* dither output 10bit */

    DITHER_OUT_BITWIDTH_BUTT
} vo_dither_out_bitwidth;

typedef enum {
    VO_HIFB_INTTYPE_VO,
    VO_HIFB_INTTYPE_VO_DISP,
    VO_HIFB_INTTYPE_WBC,
    VO_HIFB_INTTYPE_BUTT,
} vo_fb_int_type;

typedef enum {
    VO_INPUTFMT_ARGB_4444 = 0x48,
    VO_INPUTFMT_ARGB_1555 = 0x49,
    VO_INPUTFMT_ARGB_8888 = 0x68,

    VO_DISP_PIXELFORMAT_BUTT
} vo_disp_pixel_format;

typedef struct {
    vo_disp_pixel_format pixel_fmt;  /* new */
    hi_u64 ar_phy_addr;
    hi_u64 gb_phy_addr;  /* new */
    hi_u32 frame_size0;  /* new */
    hi_u32 frame_size1;  /* new */
    hi_u32 width;  /* new */
    hi_u32 height;  /* new */
    hi_bool is_lossless_a;  /* new */
    hi_bool is_lossless;  /* new */
    hi_u32 offset;
    hi_u32 stride;
} vo_graphic_dcmp_info;


typedef hi_s32 (*vo_fb_int_call_back)(const hi_void *paraml, const hi_void *paramr);
/* set vo dither output bitwidth */
typedef hi_s32 fn_vou_set_dither_out_bit_width(hi_mod_id mod_id, hi_vo_dev dev, vo_dither_out_bitwidth out_bit_width);

typedef hi_s32 fn_vou_graphics_open_layer(hi_graphic_layer gfx_layer);

typedef hi_s32 fn_vou_graphics_set_callback(hi_graphic_layer gfx_layer, vo_fb_int_type int_type,
                                            vo_fb_int_call_back call_back, hi_void *arg);
typedef struct {
    fn_vou_set_dither_out_bit_width *pfn_vou_set_dither_out_bit_width;
    fn_vou_graphics_open_layer *pfn_vou_graphics_open_layer;
    fn_vou_graphics_set_callback *pfn_vou_graphics_set_callback;
} vou_export_func;
#endif /* __HI_VOU_DEV_EXT_H__ */

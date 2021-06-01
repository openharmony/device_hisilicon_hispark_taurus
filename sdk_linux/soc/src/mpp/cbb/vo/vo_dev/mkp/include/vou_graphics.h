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

#ifndef __VOU_GRAPHICS_H__
#define __VOU_GRAPHICS_H__

#include "hi_type.h"
#include "hi_common_adapt.h"
#include "vou_dev_ext.h"
#include "hi_comm_vo_adapt.h"
#include "graphics_drv.h"

#define HIFB_LINE_BUF 1920

typedef enum {
    VOU_BITEXT_LOW_ZERO = 0x0,
    VOU_BITEXT_LOW_HIGHBIT = 0x2,
    VOU_BITEXT_LOW_HIGHBITS = 0x3,
    VOU_BITEXT_BUTT
} vou_bitext_mode;

typedef enum {
    VOU_COLORKEY_IN = 0x0,
    VOU_COLORKEY_OUT = 0x1,
    VOU_COLORKEY_BUTT
} vou_colorkey_mode;

typedef enum {
    VOU_SCAN_MODE_INTERLACE = 0x0,
    VOU_SCAN_MODE_PROGRESSIVE = 0x1,
    VOU_SCAN_MODE_BUTT
} vou_scan_mode;

vo_gfxlayer_context *vou_graphics_get_layer_ctx(hi_graphic_layer gfx_layer);

hi_void vou_graphics_enable_wbc(hal_disp_layer gfx_layer, hi_bool enable);

hi_bool vou_graphics_set_gfx_palpha(hi_graphic_layer gfx_layer, hi_u32 alpha_en, hi_u32 arange, hi_u8 alpha0,
                                    hi_u8 alpha1);
hi_bool vou_graphics_set_layer_galpha(hi_graphic_layer gfx_layer, hi_u8 alpha0);
hi_bool vou_graphics_set_csc_en(hi_graphic_layer gfx_layer, hi_bool csc_en);
hi_bool vou_graphics_set_gfx_addr(hi_graphic_layer gfx_layer, hi_u64 l_addr);
hi_bool vou_graphics_set_gfx_stride(hi_graphic_layer gfx_layer, hi_u16 pitch);

hi_void vo_graphics_vtth_int_process(hi_vo_dev dev, hi_u32 int_status);

hi_s32 vou_graphics_bind_dev(hi_graphic_layer gfx_layer, hi_vo_dev dev);
hi_s32 vou_graphics_un_bind_dev(hi_graphic_layer gfx_layer, hi_vo_dev dev);

hi_s32 vou_graphics_set_csc(hi_u32 layer, hi_vo_csc *csc);
hi_s32 vou_graphics_get_csc(hi_u32 layer, hi_vo_csc *csc);

hi_s32 vo_graphics_init(hi_void);
hi_s32 vo_graphics_de_init(hi_void);
hi_s32 vo_graphics_open_layer(hi_graphic_layer gfx_layer);
hi_s32 vou_graphics_enable_layer(hi_graphic_layer gfx_layer, hi_bool enable);
hi_s32 vou_graphics_set_callback(hi_graphic_layer gfx_layer, vo_fb_int_type int_type, vo_fb_int_call_back call_back,
                                 hi_void *arg);

hi_s32 vou_graphics_set_csc_coef(hi_graphic_layer gfx_layer);

hi_s32 vou_graphics_show_proc(osal_proc_entry_t *s);

hi_s32 vou_graphics_lock_init(hi_void);
hi_s32 vou_graphics_lock_de_init(hi_void);

#endif /* __VOU_GRAPHICS_H__ */

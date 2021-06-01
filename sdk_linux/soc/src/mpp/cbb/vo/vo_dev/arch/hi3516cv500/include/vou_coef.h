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

#ifndef __VOU_COEF_H__
#define __VOU_COEF_H__

#include "hi_type.h"
#include "hi_debug_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    VOU_ZOOM_COEF_1 = 0,
    VOU_ZOOM_COEF_EQU_1,
    VOU_ZOOM_COEF_075,
    VOU_ZOOM_COEF_05,
    VOU_ZOOM_COEF_033,
    VOU_ZOOM_COEF_025,
    VOU_ZOOM_COEF_0,
    VOU_ZOOM_COEF_BUTT
} vou_zoom_coef;

typedef struct {
    hi_s32 csc_coef00;
    hi_s32 csc_coef01;
    hi_s32 csc_coef02;

    hi_s32 csc_coef10;
    hi_s32 csc_coef11;
    hi_s32 csc_coef12;

    hi_s32 csc_coef20;
    hi_s32 csc_coef21;
    hi_s32 csc_coef22;
} vdp_csc_coef;

typedef struct {
    hi_s32 csc_in_dc0;
    hi_s32 csc_in_dc1;
    hi_s32 csc_in_dc2;

    hi_s32 csc_out_dc0;
    hi_s32 csc_out_dc1;
    hi_s32 csc_out_dc2;
} vdp_csc_dc_coef;

typedef struct {
    /* for old version csc */
    hi_s32 csc_coef00;
    hi_s32 csc_coef01;
    hi_s32 csc_coef02;

    hi_s32 csc_coef10;
    hi_s32 csc_coef11;
    hi_s32 csc_coef12;

    hi_s32 csc_coef20;
    hi_s32 csc_coef21;
    hi_s32 csc_coef22;

    hi_s32 csc_in_dc0;
    hi_s32 csc_in_dc1;
    hi_s32 csc_in_dc2;

    hi_s32 csc_out_dc0;
    hi_s32 csc_out_dc1;
    hi_s32 csc_out_dc2;

    hi_s32 new_csc_scale2p;
    hi_s32 new_csc_clip_min;
    hi_s32 new_csc_clip_max;
} csc_coef;

typedef struct {
    hi_s32 csc_scale2p;
    hi_s32 csc_clip_min;
    hi_s32 csc_clip_max;
} csc_coef_param;

typedef struct {
    hi_s32 vga_hsp_tmp0;
    hi_s32 vga_hsp_tmp1;
    hi_s32 vga_hsp_tmp2;
    hi_s32 vga_hsp_tmp3;
    hi_u32 vga_hsp_coring;
    hi_s32 vga_hsp_gainneg;
    hi_s32 vga_hsp_gainpos;
    hi_s32 vga_hsp_adpshooten;
    hi_u32 vga_hsp_winsize;
    hi_u32 vga_hsp_mixratio;
    hi_u32 vga_hsp_underth;
    hi_u32 vga_hsp_overth;
} hsp_hf_coef;

typedef struct {
    hi_u32 vga_hsp_hf_shootdiv;
    hi_u32 vga_hsp_lti_ratio;
    hi_u32 vga_hsp_ldti_gain;
    hi_u32 vga_hsp_cdti_gain;
    hi_u32 vga_hsp_peak_ratio;
    hi_u32 vga_hsp_glb_overth;
    hi_u32 vga_hsp_glb_unferth;
} hsp_coef;

#define VO_CSC_DC_MAGIC 0x07c30180

/* CVFIR VCOEF */
typedef struct {
    hi_s32 vccoef00;
    hi_s32 vccoef01;
    hi_s32 vccoef02;
    hi_s32 vccoef03;
    hi_s32 vccoef10;
    hi_s32 vccoef11;
    hi_s32 vccoef12;
    hi_s32 vccoef13;
} cvfir_coef;

/* HFIR VCOEF */
typedef struct {
    hi_s32 coef0;
    hi_s32 coef1;
    hi_s32 coef2;
    hi_s32 coef3;
    hi_s32 coef4;
    hi_s32 coef5;
    hi_s32 coef6;
    hi_s32 coef7;
} hfir_coef;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __VOU_COEF_H__ */

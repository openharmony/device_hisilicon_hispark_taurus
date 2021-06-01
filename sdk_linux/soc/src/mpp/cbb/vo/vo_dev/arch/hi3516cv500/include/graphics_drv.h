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

#ifndef __GRAPHICS_DRV_H__
#define __GRAPHICS_DRV_H__

#include "hi_osal.h"
#include "hi_type.h"
#include "hi_common_adapt.h"
#include "hi_comm_vo_adapt.h"
#include "mkp_vo_dev.h"
#include "vou_def.h"
#include "vou_coef.h"

typedef enum {
    HIFB_INTTYPE_VO,
    HIFB_INTTYPE_VO_DISP,
    HIFB_INTTYPE_WBC,
    HIFB_INTTYPE_BUTT,
} fb_int_tpye;

typedef hi_s32 (*fb_int_call_back)(hi_void *paraml, hi_void *paramr);

#define GFX_SPIN_LOCK_S osal_spinlock_t
#define gfx_spin_lock_init(lock) osal_spin_lock_init(lock)
#define gfx_spin_lock_deinit(lock) osal_spin_lock_destroy(lock)
#define gfx_spin_lock_irqsave(lock, flag) osal_spin_lock_irqsave(lock, flag)
#define gfx_spin_unlock_irqrestore(lock, flag) osal_spin_unlock_irqrestore(lock, flag)

typedef struct {
    hi_u32 start_phy_addr;
    hi_u32 end_phy_addr;
} mddrc_zone_addr;

typedef struct {
    hi_u32 ar_phy_addr;
    hi_u32 gb_phy_addr;
    hi_u32 offset;
    hi_u32 stride;
    hi_rect in_rect;
    hi_rect out_rect;
} graphic_wbc_info;

typedef struct {
    hal_disp_pixel_format pixel_fmt;  /* new */
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
} graphic_dcmp_info;

typedef struct {
    hi_dynamic_range dst_dynamic_range; /* destination dynamic range type */
} vo_graphic_attr;

typedef struct {
    hal_disp_layer layer_id;
    hi_bool opened;
    hi_bool binded;
    hi_vo_dev binded_dev;

    fb_int_call_back vo_call_back; /* VSYNC interrupt callback function */
    hi_void *vo_call_back_arg;

    fb_int_call_back wbc_call_back; /* VSYNC interrupt callback function */
    hi_void *wbc_call_back_arg;

    hi_vo_csc gfx_csc;
    csc_coef_param csc_param;
    GFX_SPIN_LOCK_S spin_lock;

    hi_u32 start_section;
    hi_u32 zone_nums;
    vo_graphic_attr vo_gfx_attr; /* VO graphic layer attribute */
} vo_gfxlayer_context;

typedef enum {
    GF_HIHDR_G_TYP = 0,
    GF_HIHDR_G_TYP1,
    GF_HIHDR_G_RAND,
    GF_HIHDR_G_MAX,
    GF_HIHDR_G_MIN,
    GF_HIHDR_G_ZERO,
    GF_HIHDR_G_BUTT
} gf_hihdr_g_mode;

typedef enum {
    GF_CSC_G0_TYP = 0,
    GF_CSC_G0_TYP1,
    GF_CSC_G0_RAND,
    GF_CSC_G0_MAX,
    GF_CSC_G0_MIN,
    GF_CSC_G0_ZERO,
    GF_CSC_G0_BUTT
} gf_csc_g0_mode;

typedef enum {
    GF_SDR_IN_HDR10_OUT = 0,
    GF_SDR_IN_HLG_OUT,
    GF_SDR_PREMULT,
    GF_SDR709_IN_2020_OUT,
    GF_BUTT
} highdr_scene_mode;

typedef struct {
    hi_u32 hihdr_r2y_en;

    highdr_scene_mode ghdr_scene_mode;
} gf_csc_g0_cfg;

typedef enum {
    VDP_GP_ORDER_NULL = 0x0,
    VDP_GP_ORDER_CSC = 0x1,
    VDP_GP_ORDER_ZME = 0x2,
    VDP_GP_ORDER_CSC_ZME = 0x3,
    VDP_GP_ORDER_ZME_CSC = 0x4,

    VDP_GP_ORDER_BUTT
} vdp_gp_order;

typedef enum {
    VDP_GFX_BITEXTEND_1ST = 0,
    VDP_GFX_BITEXTEND_2ND = 0x2,
    VDP_GFX_BITEXTEND_3RD = 0x3,

    VDP_GFX_BITEXTEND_BUTT
} vdp_gfx_bitextend;

typedef struct {
    hi_u32 a_data;
    hi_u32 r_data;
    hi_u32 g_data;
    hi_u32 b_data;
} vdp_gfx_lut_data;

typedef struct {
    hi_u32 key_r_min;
    hi_u32 key_g_min;
    hi_u32 key_b_min;

    hi_u32 key_r_max;
    hi_u32 key_g_max;
    hi_u32 key_b_max;

    hi_u32 key_r_msk;
    hi_u32 key_g_msk;
    hi_u32 key_b_msk;

    hi_u32 key_mode;
} vdp_gfx_ckey;

typedef struct {
    hi_u32 mask_r;
    hi_u32 mask_g;
    hi_u32 mask_b;
} vdp_gfx_mask;

typedef enum {
    VDP_GFX_PARA_LUT = 0,

    VDP_GFX_PARA_BUTT
} vdp_gfx_para;

typedef enum {
    VDP_GP_PARA_ZME_HORL = 0,
    VDP_GP_PARA_ZME_HORC,
    VDP_GP_PARA_ZME_VERL,
    VDP_GP_PARA_ZME_VERC,

    VDP_GP_GTI_PARA_ZME_HORL,
    VDP_GP_GTI_PARA_ZME_HORC,
    VDP_GP_GTI_PARA_ZME_VERL,
    VDP_GP_GTI_PARA_ZME_VERC,

    VDP_GP_PARA_ZME_HOR,
    VDP_GP_PARA_ZME_VER,
    VDP_GP_PARA_HDR_DEGAMM,
    VDP_GP_PARA_HDR_GAMM,

    VDP_GP_PARA_BUTT
} vdp_gp_para;

typedef enum {
    VDP_REQ_CTRL_16BURST_1,
    VDP_REQ_CTRL_16BURST_2,
    VDP_REQ_CTRL_16BURST_4,
    VDP_REQ_CTRL_16BURST_8,
    VDP_REQ_CTRL_BUTT
} vdp_req_ctrl;

typedef enum {
    VDP_G0_ZME_TYP = 0,
    VDP_G0_ZME_TYP1,
    VDP_G0_ZME_RAND,
    VDP_G0_ZME_MAX,
    VDP_G0_ZME_MIN,
    VDP_G0_ZME_ZERO,
    VDP_G0_ZME_BUTT
} gf_g0_zme_mode;

typedef enum {
    VDP_G0_ZME_FIR_COPY = 0,
    VDP_G0_ZME_FIR_FILT = 1
} gf_g0_zme_fir_mode;

typedef struct {
    hi_u32 ck_gt_en;
    hi_u32 in_width;
    hi_u32 out_width;
    hi_u32 out_pro;

    hi_u32 hfir_en;
    hi_u32 lhmid_en;
    hi_u32 ahmid_en;
    hi_u32 lhfir_mode;
    hi_u32 ahfir_mode;

    hi_u32 in_height;
    hi_u32 out_height;

    hi_u32 vfir_en;
    hi_u32 lvmid_en;
    hi_u32 avmid_en;
    hi_u32 lvfir_mode;
    hi_u32 avfir_mode;
} gf_g0_zme_cfg;

typedef enum {
    GF_RM_COEF_MODE_TYP = 0x0,
    GF_RM_COEF_MODE_RAN = 0x1,
    GF_RM_COEF_MODE_MIN = 0x2,
    GF_RM_COEF_MODE_MAX = 0x3,
    GF_RM_COEF_MODE_ZRO = 0x4,
    GF_RM_COEF_MODE_CUS = 0x5,
    GF_RM_COEF_MODE_UP = 0x6,
    GF_RM_COEF_MODE_BUTT
} gf_rm_coef_mode;

typedef enum {
    GFX_DCMP_SRC_MODE_ARGB8888 = 0,
    GFX_DCMP_SRC_MODE_ARGB6888,
    GFX_DCMP_SRC_MODE_ARGB1555,
    GFX_DCMP_SRC_MODE_ARGB4444,

    GFX_DCMP_SRC_MODE_BUTT
} gfx_dcmp_src_mode;

hi_bool graphic_drv_set_mddr_detect_zone(hal_disp_layer layer, hi_u32 start_section, hi_u32 zone_nums);

hi_s32 graphic_drv_get_hal_layer(hi_u32 layer, hal_disp_layer *hal_layer);
hi_s32 graphic_drv_get_layer_index(hal_disp_layer hal_layer, hi_u32 *layer);

hi_void graphic_drv_clr_wbc_mask(hal_disp_layer layer);
hi_void graphic_drv_clear_wbc_int_status(hal_disp_layer layer);

hi_s32 graphic_drv_check_graphic_bind_support(hi_graphic_layer gfx_layer, hi_vo_dev dev);
hi_s32 graphic_drv_graphic_bind_dev(hi_graphic_layer gfx_layer, hi_vo_dev dev);
hi_s32 graphic_drv_graphic_un_bind_dev(hi_graphic_layer gfx_layer);

hi_s32 graphic_drv_init(hi_void);
hi_s32 graphic_drv_exit(hi_void);

hi_s32 graphic_drv_lock_init(hi_void);
hi_s32 graphic_drv_lock_exit(hi_void);

hi_s32 graphic_drv_set_csc_coef(hal_disp_layer gfx_layer, hi_vo_csc *gfx_csc, csc_coef_param *csc_param);
hi_bool graphic_drv_is_chip_support_csc(hi_void);
hi_bool graphic_drv_get_layer_enable(hal_disp_layer gfx_layer);
hi_s32 graphic_drv_check_csc_coef(hal_disp_layer gfx_layer, hi_vo_csc *gfx_csc);

hi_s32 graphic_drv_show_proc(osal_proc_entry_t *s);

hi_void gf_func_set_g0_zme_mode(hi_u32 layer, gf_g0_zme_mode g0_zme_mode, gf_g0_zme_cfg *cfg);

#endif /* __GRAPHICS_DRV_H__ */

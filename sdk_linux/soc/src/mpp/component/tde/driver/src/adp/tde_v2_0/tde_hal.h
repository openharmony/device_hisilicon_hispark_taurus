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

#ifndef __TDE_HAL_H__
#define __TDE_HAL_H__

#include "drv_tde_type.h"
#include "tde_adp.h"
#include "tde_define.h"
#include "tde_osictl_k.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* TDE pixel format, accord with register, value cannot modified freely */
typedef enum {
    TDE_DRV_COLOR_FMT_ARGB8888 = 0x0,
    TDE_DRV_COLOR_FMT_KRGB8888 = 0x1,
    TDE_DRV_COLOR_FMT_ARGB4444 = 0x2,
    TDE_DRV_COLOR_FMT_ARGB1555 = 0x3,
    TDE_DRV_COLOR_FMT_ARGB8565 = 0x4,
    TDE_DRV_COLOR_FMT_RGB888 = 0x5,
    TDE_DRV_COLOR_FMT_RGB444 = 0x6,
    TDE_DRV_COLOR_FMT_RGB555 = 0x7,
    TDE_DRV_COLOR_FMT_RGB565 = 0x8,
    TDE_DRV_COLOR_FMT_A1 = 0x9,
    TDE_DRV_COLOR_FMT_A8 = 0xa,
    TDE_DRV_COLOR_FMT_YCBCR888 = 0xb,
    TDE_DRV_COLOR_FMT_AYCBCR8888 = 0xc,
    TDE_DRV_COLOR_FMT_A1B = 0xd,
    TDE_DRV_COLOR_FMT_RABG8888 = 0xf,
    TDE_DRV_COLOR_FMT_CLUT1 = 0x10,
    TDE_DRV_COLOR_FMT_CLUT2 = 0x11,
    TDE_DRV_COLOR_FMT_CLUT4 = 0x12,
    TDE_DRV_COLOR_FMT_CLUT8 = 0x13,
    TDE_DRV_COLOR_FMT_ACLUT44 = 0x14,
    TDE_DRV_COLOR_FMT_ACLUT88 = 0x15,
    TDE_DRV_COLOR_FMT_CLUT1B = 0x16,
    TDE_DRV_COLOR_FMT_CLUT2B = 0x17,
    TDE_DRV_COLOR_FMT_CLUT4B = 0x18,
    TDE_DRV_COLOR_FMT_YCBCR422 = 0x21,
    TDE_DRV_COLOR_FMT_HALFWORD = 0x22,
    TDE_DRV_COLOR_FMT_PKGVYUY = 0x23,
    TDE_DRV_COLOR_FMT_YCBCR400MBP = 0x30,
    TDE_DRV_COLOR_FMT_YCBCR422MBH = 0x31,
    TDE_DRV_COLOR_FMT_YCBCR422MBV = 0x32,
    TDE_DRV_COLOR_FMT_YCBCR420MB = 0x33,
    TDE_DRV_COLOR_FMT_YCBCR444MB = 0x34,
    TDE_DRV_COLOR_FMT_BYTE = 0x100 + TDE_DRV_COLOR_FMT_CLUT8,
    TDE_DRV_COLOR_FMT_MAX
} tde_color_fmt;

/*
 * branch order
 * support 24 orders in ARGB, TDE driver can only expose 4 usual orders; if needed, can be added
 */
typedef enum {
    TDE_DRV_ORDER_ARGB = 0x0,
    TDE_DRV_ORDER_ABGR = 0x5,
    TDE_DRV_ORDER_RABG = 0x7,
    TDE_DRV_ORDER_RGBA = 0x9,
    TDE_DRV_ORDER_BGRA = 0x14,
    TDE_DRV_ORDER_MAX
} tde_argb_order_mode;

/* TDE basic operate mode */
typedef enum {
    /* Quick fill */
    TDE_QUIKE_FILL,

    /* Quick copy */
    TDE_QUIKE_COPY,

    /* Normal fill in single source */
    TDE_NORM_FILL_1OPT,

    /* Normal bilit in single source */
    TDE_NORM_BLIT_1OPT,

    /* Fill and Rop */
    TDE_NORM_FILL_2OPT,

    /* Normal bilit in double source */
    TDE_NORM_BLIT_2OPT,

    /* MB operation */
    TDE_MB_C_OPT, /* MB chroma zoom */
    TDE_MB_Y_OPT, /* MB brightness zoom */
    TDE_MB_2OPT,  /* MB combine operation */

    /* Fill operate in single source mode  */
    TDE_SINGLE_SRC_PATTERN_FILL_OPT,

    /* Fill operate in double source mode */
    TDE_DOUBLE_SRC_PATTERN_FILL_OPT
} tde_base_opt_mode;

/* Type definition in interrupted state */
typedef enum {
    TDE_DRV_LINK_COMPLD_STATS = 0x1,
    TDE_DRV_NODE_COMPLD_STATS = 0x2,
    TDE_DRV_LINE_SUSP_STATS = 0x4,
    TDE_DRV_RDY_START_STATS = 0x8,
    TDE_DRV_SQ_UPDATE_STATS = 0x10,
    TDE_DRV_INT_ALL_STATS = 0x800F001F
} tde_int_status_mode;

/* ColorKey mode is needed by hardware */
typedef enum {
    TDE_DRV_COLORKEY_BACKGROUND = 0,             /* color key in bkground bitmap */
    TDE_DRV_COLORKEY_FOREGROUND_BEFORE_CLUT = 2, /* color key in foreground bitmap,before CLUT */
    TDE_DRV_COLORKEY_FOREGROUND_AFTER_CLUT = 3   /* color key in bkground bitmap, after CLUT */
} tde_color_key_mode;

/* color key setting arguments */
typedef struct {
    tde_color_key_mode color_key_mode; /* color key mode */
    hi_tde_color_key color_key_value;  /* color key value */
} tde_color_key_cmd;

/* Deficker filting mode */
typedef enum {
    TDE_DRV_FIXED_COEF0 = 0, /* Deficker by fixed coefficient: 0 */
    TDE_DRV_AUTO_FILTER,     /* Deficker by auto filter */
    TDE_DRV_TEST_FILTER      /* Deficker by test filter */
} tde_flicker_mode;

/* Block type, equipped register note in numerical value reference */
typedef enum {
    TDE_NO_BLOCK_SLICE_TYPE = 0,      /* No block */
    TDE_FIRST_BLOCK_SLICE_TYPE = 0x3, /* First block */
    TDE_LAST_BLOCK_SLICE_TYPE = 0x5,  /* Last block */
    TDE_MID_BLOCK_SLICE_TYPE = 0x1    /* Middle block */
} tde_slice_type;

/* vertical/horizontal filt mode: available for zoom */
typedef enum {
    TDE_DRV_FILTER_NONE = 0, /* none filt */
    TDE_DRV_FILTER_ALL       /* filt on Alpha and color value */
} tde_filter_type;

typedef enum {
    TDE_ZME_FMT_YUV422 = 0, /* none filt */
    TDE_ZME_FMT_YUV420,     /* filt on color parameter */
    TDE_ZME_FMT_YUV444,     /* filt on Alpha value */
    TDE_ZME_FMT_ARGB        /* filt on Alpha and color value */
} tde_zme_fmt_mode;

/* Deflicker operate setting */
typedef struct {
    tde_flicker_mode dfe_mode;
    tde_filter_type filter_v;
    hi_u8 coef0_last_line;
    hi_u8 coef0_cur_line;
    hi_u8 coef0_next_line;
    hi_u8 coef1_last_line;
    hi_u8 coef1_cur_line;
    hi_u8 coef1_next_line;
    hi_u8 coef2_last_line;
    hi_u8 coef2_cur_line;
    hi_u8 coef2_next_line;
    hi_u8 coef3_last_line;
    hi_u8 coef3_cur_line;
    hi_u8 coef3_next_line;
    hi_u8 threshold0;
    hi_u8 threshold1;
    hi_u8 threshold2;
    hi_tde_deflicker_mode deflicker_mode;
} tde_flicker_cmd;

typedef struct {
    hi_u32 hratio : 24;    /* 24 is length[23..0]  */
    hi_u32 hfir_order : 1; /* [24]  */
    hi_u32 hchfir_en : 1;  /* [25]  */
    hi_u32 hlfir_en : 1;   /* [26]  */
    hi_u32 hafir_en : 1;   /* [27]  */
    hi_u32 hchmid_en : 1;  /* [28]  */
    hi_u32 hlmid_en : 1;   /* [29]  */
    hi_u32 hchmsc_en : 1;  /* [30]  */
    hi_u32 hlmsc_en : 1;   /* [31]  */
} tde_hsp_info;

/* Zoom operate settings */
typedef struct {
    hi_u32 offset_x;
    hi_u32 offset_y;
    hi_u32 step_h;
    hi_u32 step_v;
    hi_bool coef_sym;
    hi_bool vf_ring;
    hi_bool hf_ring;
    tde_filter_type filter_v;
    tde_filter_type filter_h;
    hi_bool first_line_out;
    hi_bool last_line_out;
} tde_resize_cmd;

/* Clip Setting */
typedef struct {
    hi_u16 clip_start_x;
    hi_u16 clip_start_y;
    hi_u16 clip_end_x;
    hi_u16 clip_end_y;
    hi_bool inside_clip;
} tde_clip_cmd;

/* clut mode */
typedef enum {
    /* color expand */
    TDE_COLOR_EXP_CLUT_MODE = 0,

    /* color correct */
    TDE_COLOR_CORRCT_CLUT_MODE
} tde_clut_mode;

/* clut setting */
typedef struct {
    tde_clut_mode clut_mode;
    hi_u64 phy_clut_addr;
} tde_clut_cmd;

/* MB Setting */
typedef enum {
    TDE_MB_Y_FILTER = 0,     /* brightness filt */
    TDE_MB_CBCR_FILTER = 2,  /* 2 chroma filt */
    TDE_MB_UPSAMP_CONCA = 4, /* 4 first upsample then contact in chroma and brightness */
    TDE_MB_CONCA_FILTER = 6, /* 6 first contact in chroma and brightness and then filt */
} tde_mb_opt_mode;

/* MB Command Setting */
typedef struct {
    tde_mb_opt_mode mb_mode; /* MB Operate Mode */
} tde_mb_cmd;

/* plane mask command setting */
typedef struct {
    hi_u32 mask;
} tde_mask_cmd;

/* Color zone convert setting */
typedef struct {
    /* Enable or unable conversion on importing color zone */
    hi_u8 in_conv;

    /* Enable or unable conversion on exporting color zone */
    hi_u8 out_conv;
    hi_u8 in_src1_conv;

    /* import color conversion direction */
    hi_u8 in_rgb2_yc;
} tde_conv_mode_cmd;

/* vertical scanning direction */
typedef enum {
    TDE_SCAN_UP_DOWN = 0, /* form up to down */
    TDE_SCAN_DOWN_UP = 1  /* form down to up */
} tde_vscan_mode;

/* horizontal scanning direction */
typedef enum {
    TDE_SCAN_LEFT_RIGHT = 0, /* form left to right */
    TDE_SCAN_RIGHT_LEFT = 1  /* form right to left */
} tde_hscan_mode;

/* Definition on scanning direction */
typedef struct {
    /* vertical scanning direction */
    tde_vscan_mode v_scan;

    /* horizontal scanning direction */
    tde_hscan_mode h_scan;
} tde_scandirection_mode;

/*
 * Between bitmap info struct set by driver , by user and hardware info is not all
 * the same. eg, bitmap info can be divided into two bitmap info: src1 and src2,
 * which is hardware needs, when user set for MB.
 * In MB mode(refer to TDE_INS register), pu8PhyCbCr is not used in driver, but divided into
 * head addr of src1 and src2.
 * TDEV240 version:
 * 1. In nonMB mod, you can support MB. Because adding two membet variables:u32CbCrphy_addr、u32CbCrPitch,
 * which for Src1 and Src2 add one assistant channel by each.
 * 2.Support component order in ARGB/RGB format(24 kinds in toal)
 * ,add component order register and member variables:enRgbOrder.
 */
typedef struct {
    /* Bitmap head addr */
    hi_u64 phy_addr;

    /* color format */
    tde_color_fmt color_fmt;

    /* ARGB component order */
    tde_argb_order_mode rgb_order;

    /* Position X at first */
    hi_u32 xpos;

    /* Position Y at first */
    hi_u32 ypos;

    /* Bitmap Height */
    hi_u32 height;

    /* Bitmap Width */
    hi_u32 width;

    /* Bitmap Pitch */
    hi_u32 pitch;

    /* CbCr component addr */
    hi_u64 cbcr_phy_addr;

    /* CbCr pitch */
    hi_u32 cb_cr_pitch;

    /* alpha max value is 255?or 128? */
    hi_bool alpha_max_is_255;

    /* Vertical scanning direction */
    tde_vscan_mode v_scan;

    /* Horizontal scanning direction */
    tde_hscan_mode h_scan;
    hi_bool cma;
} tde_surface_msg;

/* ALU mode */
typedef enum {
    TDE_SRC1_BYPASS = 0,
    TDE_ALU_NONE, /* register has no setting, used in flag */
    TDE_ALU_ROP,
    TDE_SRC2_BYPASS,
    TDE_ALU_MASK_ROP1,
    TDE_ALU_MASK_ROP2,
    TDE_ALU_MASK_BLEND,
    TDE_ALU_BLEND,
    TDE_ALU_BLEND_SRC2,
    TDE_ALU_CONCA,
    TDE_CLIP,
    TDE_BUTT
} tde_alu_mode;

/* Zoom mode in subnode */
typedef enum {
    TDE_CHILD_SCALE_NORM = 0,
    TDE_CHILD_SCALE_MBY,
    TDE_CHILD_SCALE_MBC,
    TDE_CHILD_SCALE_MB_CONCA_H,
    TDE_CHILD_SCALE_MB_CONCA_M,
    TDE_CHILD_SCALE_MB_CONCA_L,
    TDE_CHILD_SCALE_MB_CONCA_CUS,
} tde_child_scale_mode;

/* Info needed in MB format when Y/CbCr change */
typedef struct {
    hi_u32 start_in_x; /* Start X,Y imported after MB adjust */
    hi_u32 start_in_y;
    hi_u32 start_out_x; /* Start X,Y exported after MB adjust */
    hi_u32 start_out_y;
    tde_color_fmt fmt; /* color format, MB use it to renew position of Y and CbCr */
    tde_child_scale_mode scale_mode;
} tde_mb_start_adj_info;

/* Adjusting info when double source dispart */
typedef struct {
    hi_bool double_source;
    hi_s32 diff_x; /* differences between S1x and Tx */
    hi_s32 diff_y; /* differences between S1y and Ty */
} tde_double_src_adj_info;

/*
 * Configure info when set child node
 * u64Update :
 * _________________________________________
 * |    |    |    |    |    |    |    |    |
 * | ...| 0  | 0  | 1  | 1  | 1  | 1  |  1 |
 * |____|____|____|____|____|____|____|____|
 *                   |    |    |    |    |
 *                  \/   \/   \/   \/   \/
 *                u32Wo u32Xo HOfst u32Wi u32Xi
 *                u32Ho u32Yo VOfst u32Hi u32Yi
 */
typedef struct {
    hi_u32 slice_width;
    hi_u32 slice_wi;
    hi_s32 slice_c_ofst;

    hi_s32 slice_l_ofst;
    hi_u32 slice_hoffset;
    hi_u32 slice_wo;
    hi_u32 slice_w_hpzme;
    hi_u32 xi;
    hi_u32 yi;
    hi_u32 wi;
    hi_u32 hi;
    hi_u32 h_ofst;
    hi_u32 v_ofst;
    hi_u32 xo;
    hi_u32 yo;
    hi_u32 wo;
    hi_u32 ho;
    hi_u32 slice_dst_width;
    hi_u32 slice_dst_height;
    hi_u32 slice_dst_hoffset;
    hi_u64 update;
    tde_mb_start_adj_info adj_info;
    tde_double_src_adj_info ds_adj_info;
    tde_slice_type slice_type;
    hi_u32 des_crop_en;
    hi_u32 des_crop_start_x;
    hi_u32 des_crop_end_x;
} tde_child_info;

typedef struct {
    tde_color_fmt drv_color_fmt;
    hi_u32 fill_data;
} tde_color_fill;

typedef enum {
    TDE_DRV_SRC_NONE = 0,
    TDE_DRV_SRC_S1 = 0x1,
    TDE_DRV_SRC_S2 = 0x2,
    TDE_DRV_SRC_T = 0x4,
} tde_src_mode;

#define TDE_AQ_COMP_NODE_MASK_EN 4 /* 0100：Enable to interrupt when complete current node in AQ */
#define TDE_AQ_COMP_LIST_MASK_EN 8 /* 1000：Enable to interrupt  in complete AQ */

/*
 * Handle responded with node
 * Add 4 byte pointer in physical buffer header,to save software node;
 * For need to consult current executing software node,but register can only
 * give the physical addr of it.
 */
#define TDE_NODE_HEAD_BYTE 16

/* Next node addr、update info、occupied bytes */
#define TDE_NODE_TAIL_BYTE 12

/*
 * Function:      tde_hal_init
 * Description:   main used in mapping TDE basic addr
 * Input:         base_addr:Register basic addr
 * Return:        Success/Failure
 */
hi_s32 tde_hal_init(hi_u32 base_addr);

hi_s32 tde_hal_open(hi_void);
hi_s32 tde_hal_close(hi_void);

/*
 * Function:      tde_hal_release
 * Description:   main used in release TDE basic addr by mapping
 * Return:        Success/Failure
 */
hi_void tde_hal_release(hi_void);

/*
 * Function:      tde_hal_ctl_is_idle
 * Description:   Query if TDE is in IDLE state or not
 * Return:        True: Idle/False: Busy
 */
hi_bool tde_hal_ctl_is_idle(hi_void);

/*
 * Function:      tde_hal_ctl_is_idle_safely
 * Description:   cycle many times, to make sure TDE is in IDLE state
 * Return:        True: Idle/False: Busy
 */
hi_bool tde_hal_ctl_is_idle_safely(hi_void);

hi_u32 tde_hal_ctl_int_status(hi_void);

hi_void tde_hal_ctl_reset(hi_void);

hi_void tde_hal_ctl_int_clear(hi_u32 stats);

hi_s32 tde_hal_node_init_nd(tde_hw_node **hw_node);

hi_void tde_hal_free_node_buf(tde_hw_node *hw_node);
#if ((HICHIP != HI3516C_V500) && (HICHIP != HI3519A_V100))
hi_void tde_hal_calc_src1_filter_opt(tde_hw_node *node, hi_tde_color_fmt in_fmt,
                                     hi_tde_rect *in_rect, hi_tde_rect *out_rect, hi_tde_deflicker_mode filter_mode);
#endif
hi_s32 tde_hal_calc_src2_filter_opt(tde_hw_node *node, tde_rect_opt *rect_opt, hi_bool defilicker,
                                    hi_tde_deflicker_mode filter_mode);

/*
 * Function:      tde_hal_node_enable_complete_int
 * Description:   Complete interrupt by using node's operate
 * Input:         buf: Buffer need node be operated
 */
hi_void tde_hal_node_enable_complete_int(hi_void *buf);

hi_void tde_hal_next_node_addr(hi_void *buf, hi_u64 phy_addr);

/*
 * Function:      tde_hal_node_set_src1
 * Description:   Set Src1 bitmap info
 * Input:         hw_node: Node struct pointer used in cache by software
 *                drv_surface: bitmap info used in setting
 */
hi_void tde_hal_node_set_src1(tde_hw_node *hw_node, tde_surface_msg *drv_surface);

/*
 * Function:      tde_hal_node_set_src2
 * Description:   Set Src2 bitmap info
 * Input:         hw_node: Node struct pointer used in cache by software
 *                drv_surface: bitmap info used in setting
 */
hi_void tde_hal_node_set_src2(tde_hw_node *hw_node, tde_surface_msg *drv_surface);

hi_void tde_hal_node_enable_sync(hi_void *buf);

hi_void tde_hal_node_set_src_to_decompress(tde_hw_node *hw_node, tde_surface_msg *drv_surface);

hi_void tde_hal_node_set_compress(tde_hw_node *hw_node);

hi_void tde_hal_node_set_compress_tqt(tde_hw_node *hw_node, tde_surface_msg *drv_surface,
                                      hi_tde_out_alpha_from alpha_from);

hi_void tde_hal_node_set_rotate(tde_hw_node *hw_node, hi_tde_rotate_angle rotate_angle);

/*
 * Function:      tde_hal_node_set_tqt
 * Description:   Set target bitmap information
 * Input:         hw_node: Node struct pointer used in cache by software
 *                drv_surface: bitmap information used in setting
 */
hi_void tde_hal_node_set_tqt(tde_hw_node *hw_node, tde_surface_msg *drv_surface, hi_tde_out_alpha_from alpha_from);

/*
 * Function:      tde_hal_node_set_base_operate
 * Description:   Set basic operate type
 * Input:         hw_node: Node struct pointer used in cache by software
 *                mode: basic operate mode
 *                alu: ALU mode
 *                color_fill: if basic mode have fill operate ,read this value
 */
hi_s32 tde_hal_node_set_base_operate(tde_hw_node *hw_node, tde_base_opt_mode mode, tde_alu_mode alu,
                                     tde_color_fill *color_fill);

/*
 * Function:      tde_hal_node_set_global_alpha
 * Description:   Set Alpha mixed arguments
 * Input:         hw_node: Node struct pointer used in cache by software
 *                alpha: Alpha mixed setting value
 *                enable: Enable to use global alpha
 */
hi_void tde_hal_node_set_global_alpha(tde_hw_node *hw_node, hi_u8 alpha, hi_bool enable);

hi_void tde_hal_node_set_src1_alpha(tde_hw_node *hw_node);

hi_void tde_hal_node_set_src2_alpha(tde_hw_node *hw_node);

/*
 * Function:      tde_hal_node_set_exp_alpha
 * Description:   When expand Alpha in RGB5551. to alpha0 and alpha1
 * Input:         hw_node: Node struct pointer used in cache by software
 *                alpha: Alpha mixed setting value
 */
hi_void tde_hal_node_set_exp_alpha(tde_hw_node *hw_node, tde_src_mode src, hi_u8 alpha0, hi_u8 alpha1);

/*
 * Function:      tde_hal_node_set_rop
 * Description:   Set ROP arguments
 * Input:         hw_node: Node struct pointer used in cache by software
 *                rgb_rop: ROP operator
 */
hi_s32 tde_hal_node_set_rop(tde_hw_node *hw_node, hi_tde_rop_mode rgb_rop, hi_tde_rop_mode alpha_rop);

/*
 * Function:      tde_hal_node_set_blend
 * Description:   Set blend operate arguments
 * Input:         hw_node: Node struct pointer used in cache by software
 *                blend_opt:blend operate option
 */
hi_s32 tde_hal_node_set_blend(tde_hw_node *hw_node, hi_tde_blend_opt *blend_opt);

/*
 * Function:      tde_hal_node_set_colorize
 * Description:   Set blend operate arguments
 * Input:         hw_node: Node struct pointer used in cache by software
 *                color_resize:Co
 */
hi_s32 tde_hal_node_set_colorize(tde_hw_node *hw_node, hi_u32 colorize);

/*
 * Function:      tde_hal_node_enable_alpha_rop
 * Description:   Enable to blend Rop operate
 * Input:         hw_node: Node struct pointer used in cache by software
 */
hi_void tde_hal_node_enable_alpha_rop(tde_hw_node *hw_node);

/*
 * Function:      tde_hal_node_set_clut_opt
 * Description:   Set color expand or adjust argument
 * Input:         hw_node: Node struct pointer used in cache by software
 *                clut_cmd: Clut operate atguments
 */
hi_s32 tde_hal_node_set_clut_opt(tde_hw_node *hw_node, tde_clut_cmd *clut_cmd, hi_bool reload);

/*
 * Function:      tde_hal_node_set_colorkey
 * Description:   Set arguments needed by color key,according current color format
 * Input:         hw_node: Node struct pointer used in cache by software
 *                fmt_cat: color format
 *                color_key: color key pointer
 */
hi_s32 tde_hal_node_set_colorkey(tde_hw_node *hw_node, tde_colorfmt_category fmt_cat,
    tde_color_key_cmd *color_key);

/*
 * Function:      tde_hal_node_set_clipping
 * Description:   Set rectangle's clip operated arguments
 * Input:         hw_node: Node struct pointer used in cache by software
 *                clip: Clip rectangle range
 */
hi_s32 tde_hal_node_set_clipping(tde_hw_node *hw_node, tde_clip_cmd *clip);

hi_s32 tde_hal_node_set_color_convert(tde_hw_node *hw_node, tde_conv_mode_cmd *conv);

/*
* Function:      tde_set_node_csc
* Description:   Set CSC's first optional argument
* Input:         hw_node:Node struct pointer used in cache by software
                 csc_opt:CSC first optional argument
*/
hi_s32 tde_set_node_csc(tde_hw_node *hw_node, hi_tde_csc_opt csc_opt);

hi_s32 tde_hal_set_deflicer_level(hi_tde_deflicker_level deflicker_level);

hi_s32 tde_hal_get_deflicer_level(hi_tde_deflicker_level *deflicer_level);

hi_s32 tde_hal_set_alpha_threshold(hi_u8 threshold_value);

hi_s32 tde_hal_get_alpha_threshold(hi_u8 *threshold_value);

/*
 * Function:      tde_hal_get_alpha_threshold_state
 * Description:   Set alpha threshold to judge if open or close
 * Input:         alpha_threshold_en:alpha switch status
 * Return:        Success / Fail
 */
hi_s32 tde_hal_set_alpha_threshold_state(hi_bool alpha_threshold_en);

/*
 * Function:      tde_hal_get_alpha_threshold_state
 * Description:   Get alpha threshold to judge if open or close
 * Output:        en_alpha_threshold:alpha switch status
 * Return:        Success / Fail
 */
hi_s32 tde_hal_get_alpha_threshold_state(hi_bool *alpha_threshold_en);

/*
 * Function:      tde_hal_node_execute
 * Description:   Start TDE list
 * Input:
 *                nodephy_addr: list first node address
 *                update: first node updating flag
 *                aq_use_buff: if use temporary buffer
 * Return:        Success / Fail
 */
hi_s32 tde_hal_node_execute(hi_u64 nodephy_addr, hi_u64 update, hi_bool aq_use_buff);

hi_u64 tde_hal_cur_node(hi_void);
/*
 * Function:      tde_hal_resume_init
 * Description:   Resume the hardware by software ,initialize the TDE device
 */
hi_void tde_hal_resume_init(hi_void);

hi_void tde_hal_suspend(hi_void);

hi_void tde_hal_set_clock(hi_bool enable);

#ifdef CONFIG_GFX_MMU_SUPPORT
hi_void tde_hal_free_tmp_buf(tde_hw_node *hw_node);
#endif

osal_proc_entry_t *tde_hal_node_print_info(osal_proc_entry_t *p, hi_u32 *cur_node);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TDE_HAL_H_ */

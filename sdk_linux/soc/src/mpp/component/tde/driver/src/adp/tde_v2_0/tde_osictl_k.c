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

#include "tde_osictl_k.h"
#ifndef HI_BUILD_IN_BOOT
#include "tde_handle.h"
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
#include "tde_proc.h"
#endif
#else
#include "exports.h"
#include "tde_handle.h"
#endif
#include "tde_osictl.h"
#include "tde_osilist.h"
#include "tde_hal.h"
#include "wmalloc.h"
#include "hi_math.h"
#include "securec.h"
#include "tde_adp.h"
#include "osal_list.h"

/* TDE osi ctl macro definition */
#define TDE_CLUT_SIZE (256 * 4)

#define tde_min(a, b) \
    (((a) > (b)) ? (b) : (a))

#define drv_tde_mb_src_val(mb_src, theight, twidth)        \
    do {                                                   \
        theight = (mb_src)->src_rect->height;              \
        twidth = (mb_src)->src_rect->width;                \
    } while (0)

#define drv_tde_mb_src_addr(mb_src, typhy_addr, tcb_crphy_addr, tphy)    \
        do {                                                             \
            typhy_addr = (mb_src)->mb_surface->y_addr;                   \
            tcb_crphy_addr = (mb_src)->mb_surface->cbcr_phy_addr;        \
            tphy = (mb_src)->dst_surface->phy_addr;                      \
        } while (0)

#define tde_get_yc422r_fillvalue(value)                                          \
    (((value) & 0xffffff) | ((((value) >> 8) & 0xff) << 24))

#define ycc2rgb(y, cb, cr, r, g, b)                                                            \
    do {                                                                                       \
        r = (((298 * ((y) - 16) + 409 * ((cr) - 128)) >> 7) + 1) >> 1;                         \
        g = (((298 * ((y) - 16) - 100 * ((cb) - 128) - 208 * ((cr) - 128)) >> 7) + 1) >> 1;    \
        b = (((298 * ((y) - 16) + 517 * ((cb) - 128)) >> 7) + 1) >> 1;                         \
    } while (0)

#define rgb2ycc(r, g, b, y, cb, cr)                                          \
    do {                                                                     \
        y  = ((((263 * (r) + 516 * (g) + 100 * (b)) >> 9) + 1) >> 1) + 16;   \
        cb = ((((-152 * (r) - 298 * (g) + 450 * (b)) >> 9) + 1) >> 1) + 128; \
        cr = ((((450 * (r) - 377 * (g) - 73 * (b)) >> 9) + 1) >> 1) + 128;   \
    } while (0)

#define TDE_FOUR_BITS_SHIFT  4
#define TDE_THREE_BITS_SHIFT 3
#define TDE_TWO_BITS_SHIFT   2
#define TDE_ONE_BIT_SHIFT    1
#define TDE_ZERO_BIT_SHIFT   0
#define TDE_MAX_PATTERNWIDTH 256

/* TDE osi ctl struct definition */
#if (TDE_CAPABILITY & SLICE)
typedef struct {
    hi_u32 des_width;
    hi_u32 des_hoffset_pix;
    hi_u32 des_h_scan_ord;
    hi_u32 des_crop_en;
    hi_u32 des_crop_start_x;
    hi_u32 des_crop_end_x;

    hi_u32 xdpos;
    hi_u32 h_scan_ord;
    hi_u32 width;

    hi_u32 hlmsc_en;
    hi_u32 hchmsc_en;
    hi_u32 hratio;
    hi_s32 hor_loffset;
    hi_s32 hor_coffset;
    hi_u32 zme_ow;
    hi_u32 zme_iw;
    hi_u32 hpzme_en;
    hi_u32 hpzme_mode;
    hi_u32 hpzme_width;

    hi_u32 des_xst_pos_blk;
    hi_u32 des_xed_pos_blk;
    hi_u32 xed_pos_blk;
    hi_u32 xst_pos_cord;
    hi_u32 xed_pos_cord;
    hi_u32 xst_pos_cord_in;
    hi_u32 xed_pos_cord_in;
    hi_u32 hor_loffset_cfg_int_comp;
    hi_s32 hor_loffset_cfg_int;
    hi_s32 xst_pos_cord_in_offset;
    hi_s32 xed_pos_cord_in_offset;
    hi_u32 xst_pos_cord_in_tap_rgb;
    hi_u32 xed_pos_cord_in_tap_rgb;
    hi_u32 node_cfg_zme_iw_rgb;
    hi_u32 hor_loffset_cfg_fraction;
    hi_u32 hor_loffset_pix_fraction;
    hi_u32 hor_loffset_fraction;
    hi_s32 hor_loffset_int;
    hi_s32 node_cfg_hor_loffset_rgb;
    hi_u32 xst_pos_cord_in_tap_luma;
    hi_u32 xed_pos_cord_in_tap_luma;
    hi_u32 xst_pos_cord_in_chroma;
    hi_u32 xed_pos_cord_in_chroma;
    hi_u32 hor_coffset_cfg_int_comp;
    hi_s32 hor_coffset_cfg_int;
    hi_s32 xst_pos_cord_in_offset_chroma;
    hi_s32 xed_pos_cord_in_offset_chroma;
    hi_u32 xst_pos_cord_in_tap_chroma;
    hi_u32 xed_pos_cord_in_tap_chroma;
    hi_u32 xst_pos_cord_in_tap_chroma_x2;
    hi_u32 xed_pos_cord_in_tap_chroma_x2;
    hi_u32 xst_pos_cord_in_tap_sp;
    hi_u32 xed_pos_cord_in_tap_sp;
    hi_u32 node_cfg_zme_iw_sp;
    hi_u32 hor_coffset_cfg_fraction;
    hi_u32 hor_coffset_pix_fraction;
    hi_u32 hor_coffset_fraction;
    hi_s32 hor_loffset_int_sp;
    hi_s32 hor_coffset_int_sp;
    hi_u32 hor_loffset_int_sp_complent;
    hi_u32 hor_coffset_int_sp_complent;
    hi_s32 node_cfg_hor_loffset_sp;
    hi_s32 node_cfg_hor_coffset_sp;
    hi_u32 xst_pos_cord_in_tap;
    hi_u32 xed_pos_cord_in_tap;

    hi_u32 hor_loffset_int_complement;
    hi_u32 node_num;

    hi_u32 xst_pos_cord_in_tap_hpzme;
    hi_u32 xed_pos_cord_in_tap_hpzme;
    hi_u32 xst_pos_cord_in_tap_hpzme_hso;
    hi_u32 xed_pos_cord_in_tap_hpzme_hso;
    hi_u32 u32422v_pro;
    hi_u32 hor_loffset_int_beyond;
    hi_u32 hor_loffset_int_beyond_complent;

    hi_u32 slice_width;
    hi_u32 slice_wi;
    hi_s32 slice_c_ofst;
    hi_s32 slice_l_ofst;
    hi_u32 slice_hoffset;
    hi_u32 slice_wo;
    hi_u32 slice_w_hpzme;
    hi_u32 slice_dst_width;
    hi_u32 slice_dst_hoffset;
    hi_u32 fmt;
} tde_slice_data;
#endif

/* pixel format transform type */
typedef enum {
    TDE_COLORFMT_TRANSFORM_ARGB_ARGB = 0,
    TDE_COLORFMT_TRANSFORM_ARGB_YCBCR,
    TDE_COLORFMT_TRANSFORM_CLUT_ARGB,
    TDE_COLORFMT_TRANSFORM_CLUT_YCBCR,
    TDE_COLORFMT_TRANSFORM_CLUT_CLUT,
    TDE_COLORFMT_TRANSFORM_YCBCR_ARGB,
    TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR,
    TDE_COLORFMT_TRANSFORM_AN_AN,
    TDE_COLORFMT_TRANSFORM_ARGB_AN,
    TDE_COLORFMT_TRANSFORM_YCBCR_AN,
    TDE_COLORFMT_TRANSFORM_BUTT,
} tde_colorfmt_transform;

/* CLUT table use */
typedef enum {
    TDE_CLUT_COLOREXPENDING = 0, /* color expend */
    TDE_CLUT_COLORCORRECT,       /* color correct */
    TDE_CLUT_CLUT_BYPASS,
    TDE_CLUT_USAGE_BUTT
} tde_clut_usage;

typedef enum {
    TDE_OPERATION_SINGLE_SRC1 = 0,
    TDE_OPERATION_SINGLE_SRC2,
    TDE_OPERATION_DOUBLE_SRC,
    TDE_OPERATION_BUTT
} tde_operation_category;

typedef enum {
    TDE_PATTERN_OPERATION_SINGLE_SRC = 0,
    TDE_PATTERN_OPERATION_DOUBLE_SRC,
    TDE_PATTERN_OPERATION_BUTT
} tde_pattern_operation_category;

typedef struct {
    hi_u8 alpha_bits;
    hi_u8 red_bits;
    hi_u8 green_bits;
    hi_u8 blue_bits;
    hi_u8 alpha_offset;
    hi_u8 red_offset;
    hi_u8 green_offset;
    hi_u8 blue_offset;
} tde2_fmt_bitoffset;

#ifndef HI_BUILD_IN_BOOT
#if (TDE_CAPABILITY & RESIZE)
typedef struct {
    hi_s32 ori_in_width;    /* original image width */
    hi_s32 ori_in_height;   /* original image height */
    hi_s32 zme_out_width;   /* output full image width */
    hi_s32 zme_out_height;  /* output full image height */

    hi_s32 update_instart_w;  /* the start_x of update area in original image */
    hi_s32 update_instart_h;  /* the start_y of update area in original image */
    hi_s32 update_in_width;   /* the width of update area in original image */
    hi_s32 update_in_height;  /* the height of update area in original image */
} tde_update_config;

typedef struct {
    hi_s32 zme_instart_w;  /* the start_x of needed readin area in original image */
    hi_s32 zme_instart_h;  /* the start_y of needed readin area in original image */
    hi_s32 zme_in_width;   /* the width of needed readin area in original image */
    hi_s32 zme_in_height;  /* the height of needed readin area in original image */

    hi_s32 zme_outstart_w;  /* the start_x of needed update area in output image */
    hi_s32 zme_outstart_h;  /* the start_y of needed update area in output image */
    hi_s32 zme_out_width;   /* the width of needed update area in output image */
    hi_s32 zme_out_height;  /* the height of needed update area in output image */

    hi_s32 zme_hphase;      /* the start phase of horizontal scale */
    hi_s32 zme_vphase;      /* the start phase of vertical scale */
    hi_s32 def_offsetup;    /* the up offset of deflicker */
    hi_s32 def_offsetdown;  /* the down offset of deflicker */
} tde_update_info;
#endif
#endif

/* TDE osi ctl inner variables definition */
tde_color_fmt g_tde_common_drv_color_fmt[HI_TDE_COLOR_FMT_MAX + 1] = {
    TDE_DRV_COLOR_FMT_RGB444, TDE_DRV_COLOR_FMT_RGB444,
    TDE_DRV_COLOR_FMT_RGB555, TDE_DRV_COLOR_FMT_RGB555,
    TDE_DRV_COLOR_FMT_RGB565, TDE_DRV_COLOR_FMT_RGB565,
    TDE_DRV_COLOR_FMT_RGB888, TDE_DRV_COLOR_FMT_RGB888,
    TDE_DRV_COLOR_FMT_ARGB4444, TDE_DRV_COLOR_FMT_ARGB4444, TDE_DRV_COLOR_FMT_ARGB4444, TDE_DRV_COLOR_FMT_ARGB4444,
    TDE_DRV_COLOR_FMT_ARGB1555, TDE_DRV_COLOR_FMT_ARGB1555, TDE_DRV_COLOR_FMT_ARGB1555, TDE_DRV_COLOR_FMT_ARGB1555,
    TDE_DRV_COLOR_FMT_ARGB8565, TDE_DRV_COLOR_FMT_ARGB8565, TDE_DRV_COLOR_FMT_ARGB8565, TDE_DRV_COLOR_FMT_ARGB8565,
    TDE_DRV_COLOR_FMT_ARGB8888, TDE_DRV_COLOR_FMT_ARGB8888, TDE_DRV_COLOR_FMT_ARGB8888, TDE_DRV_COLOR_FMT_ARGB8888,
    TDE_DRV_COLOR_FMT_RABG8888,
    TDE_DRV_COLOR_FMT_CLUT1, TDE_DRV_COLOR_FMT_CLUT2, TDE_DRV_COLOR_FMT_CLUT4,
    TDE_DRV_COLOR_FMT_CLUT8, /* TDE_DRV_COLOR_FMT_CLUT1B,TDE_DRV_COLOR_FMT_CLUT2B,TDE_DRV_COLOR_FMT_CLUT4B, */
    TDE_DRV_COLOR_FMT_ACLUT44, TDE_DRV_COLOR_FMT_ACLUT88,
    TDE_DRV_COLOR_FMT_A1, TDE_DRV_COLOR_FMT_A8, /* TDE_DRV_COLOR_FMT_A1B, */
    TDE_DRV_COLOR_FMT_YCBCR888, TDE_DRV_COLOR_FMT_AYCBCR8888, TDE_DRV_COLOR_FMT_YCBCR422, TDE_DRV_COLOR_FMT_PKGVYUY,
    TDE_DRV_COLOR_FMT_BYTE, TDE_DRV_COLOR_FMT_HALFWORD,
    TDE_DRV_COLOR_FMT_YCBCR400MBP,
    TDE_DRV_COLOR_FMT_YCBCR422MBH, TDE_DRV_COLOR_FMT_YCBCR422MBV,
    TDE_DRV_COLOR_FMT_YCBCR420MB, TDE_DRV_COLOR_FMT_YCBCR420MB, TDE_DRV_COLOR_FMT_YCBCR420MB,
    TDE_DRV_COLOR_FMT_YCBCR420MB,
    TDE_DRV_COLOR_FMT_YCBCR444MB, TDE_DRV_COLOR_FMT_MAX
};

static tde_argb_order_mode g_tde_argb_order[HI_TDE_COLOR_FMT_MAX + 1] = {
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR, TDE_DRV_ORDER_RGBA, TDE_DRV_ORDER_BGRA,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR, TDE_DRV_ORDER_RGBA, TDE_DRV_ORDER_BGRA,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR, TDE_DRV_ORDER_RGBA, TDE_DRV_ORDER_BGRA,
    TDE_DRV_ORDER_ARGB, TDE_DRV_ORDER_ABGR, TDE_DRV_ORDER_RGBA, TDE_DRV_ORDER_BGRA,
    TDE_DRV_ORDER_RABG,
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX, /* TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, */
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, /* TDE_DRV_ORDER_MAX, */
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX, TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX,
    TDE_DRV_ORDER_MAX
};

#ifndef HI_BUILD_IN_BOOT /* codecc */
/* if local deflicker flag */
static hi_bool g_region_deflicker = HI_FALSE;
static hi_bool g_is_resize_filter = HI_TRUE;
#endif

/* TDE osi ctl inner interface definition */
static hi_void tde_unify_rect(hi_tde_rect *src_rect, hi_tde_rect *dst_rect);
static tde_colorfmt_category tde_osi_get_fmt_category(hi_tde_color_fmt fmt);

static tde_colorfmt_transform tde_osi_get_fmt_trans_type(hi_tde_color_fmt src2_fmt, hi_tde_color_fmt dst_fmt);

static hi_s32 tde_osi_set_clut_opt(hi_tde_surface *clut_sur, hi_tde_surface *out_sur, tde_clut_usage *pen_clut_usage,
                                   hi_bool clut_reload, tde_hw_node *hw_node);

static tde_clut_usage tde_osi_get_clut_usage(hi_tde_color_fmt src_fmt, hi_tde_color_fmt dst_fmt);

static hi_s32 tde_osi_get_scan_info_ex(hi_tde_single_src *single_src, hi_tde_opt *opt,
                                       tde_scandirection_mode *src_direction,
                                       tde_scandirection_mode *dst_direction);

static hi_s32 tde_osi_get_inter_rect(hi_tde_rect *rect1, hi_tde_rect *rect2, hi_tde_rect *inter_rect);

static hi_s32 tde_osi_set_mb_para(hi_s32 handle, hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt);

static hi_s32 tde_osi_set_filter_node(hi_s32 handle, tde_hw_node *node, hi_tde_double_src *double_src,
                                      hi_tde_deflicker_mode deflicker_mode, hi_tde_deflicker_mode fliter_mode);

static hi_s32 tde_osi_1_source_fill(hi_s32 handle, hi_tde_surface *dst_surface,
                                    hi_tde_rect *dst_rect, hi_tde_fill_color *fill_color,
                                    hi_tde_opt *opt);

static hi_s32 tde_osi_single_src_2_blit(hi_s32 handle, hi_tde_single_src *single_src,
                                        hi_tde_opt *opt, hi_bool mmz_for_src, hi_bool mmz_for_dst);

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_2_source_fill(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_fill_color *fill_color,
                                    hi_tde_opt *opt);

static hi_s32 tde_osi_set_color_key(hi_tde_double_src *double_src,
                                    tde_hw_node *hw_node, hi_tde_color_key color_key_value,
                                    hi_tde_color_key_mode color_key_mode,
                                    tde_clut_usage clut_usage);
static hi_s32 tde_osi_set_blend(tde_hw_node *hw_node, hi_tde_alpha_blending alpha_blending_cmd,
                                hi_tde_blend_opt blend_opt,
                                tde_alu_mode *alu_mode, hi_bool check_blend);

#if (TDE_CAPABILITY & ROP)

static hi_s32 tde_osi_set_rop(tde_hw_node *hw_node, hi_tde_alpha_blending alpha_blending_cmd,
                              hi_tde_rop_mode rop_code_color,
                              hi_tde_rop_mode rop_alpha, tde_alu_mode *alu_mode, hi_bool single_sr2_rop);
#endif

#if (TDE_CAPABILITY & COLORIZE)

static hi_s32 tde_osi_set_colorize(tde_hw_node *hw_node, hi_tde_alpha_blending alpha_blending_cmd,
    hi_s32 color_resize);
#endif

static hi_s32 tde_osi_check_double_src_pattern_fill_para(hi_tde_double_src *double_src,
                                                         hi_tde_pattern_fill_opt *opt);

static hi_s32 tde_osi_check_single_src_pattern_fill_para(hi_tde_single_src *single_src,
                                                         hi_tde_pattern_fill_opt *opt);

static tde_pattern_operation_category tde_osi_check_single_src_pattern_operation(hi_tde_double_src *double_src,
    hi_tde_pattern_fill_opt *opt);

static tde_pattern_operation_category tde_osi_double_src_pattern_operation(hi_tde_double_src *double_src,
                                                                           hi_tde_pattern_fill_opt *opt);

static hi_s32 tde_osi_single_src_1_blit(hi_s32 handle, hi_tde_single_src *single_src,
                                        hi_bool mmz_for_src, hi_bool mmz_for_dst);

static hi_s32 tde_osi_set_foreground_color_key(tde_hw_node *hw_node, hi_tde_surface *src_surface, hi_tde_opt *opt,
                                               tde_clut_usage clut_usage);
#endif

static tde_operation_category tde_osi_single_src_operation(hi_tde_double_src *double_src, hi_tde_opt *opt);

static tde_operation_category tde_osi_double_src_operation(hi_tde_double_src *double_src, hi_tde_opt *opt);

static hi_s32 tde_osi_check_surface(hi_tde_surface *surface, hi_tde_rect *rect);

static tde_operation_category tde_osi_get_opt_category(hi_tde_double_src *double_src, hi_tde_opt *opt);

static hi_void tde_osi_convert_surface(hi_tde_surface *sur, hi_tde_rect *rect,
                                       tde_scandirection_mode *scan_info,
                                       tde_surface_msg *drv_sur);

static hi_s32 tde_osi_set_clip_para(hi_tde_double_src *double_src, hi_tde_opt *opt, tde_hw_node *hw_node);

static hi_s32 tde_osi_set_base_opt_para_for_blit(hi_tde_opt *opt, hi_tde_surface *src1, hi_tde_surface *src2,
                                                 tde_operation_category opt_category, tde_hw_node *hw_node);

static hi_s32 tde_osi_adj_clip_para(tde_hw_node *hw_node);

static hi_s32 tde_osi_set_node_finish(hi_s32 handle, tde_hw_node *hw_node,
                                      hi_u32 work_buf_num, tde_node_subm_type subm_type);

static hi_s32 tde_osi_check_resize_para(hi_u32 in_width, hi_u32 in_height,
                                        hi_u32 out_width, hi_u32 out_height);
static hi_bool tde_osi_whether_contain_alpha(hi_tde_color_fmt color_fmt);
static hi_void tde_osi_set_ext_alpha(hi_tde_surface *back_ground, hi_tde_surface *fore_ground,
                                     tde_hw_node *hw_node);

static hi_s32 tde_osi_pre_check_surface_ex(hi_tde_surface *surface, hi_tde_rect *rect);

static hi_s32 tde_osi_raster_fmt_check_align(hi_tde_surface *surface);

static hi_s32 tde_osi_check_mb_blit_para(hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt);

static hi_s32 tde_osi_get_double_cycle_data(hi_tde_rect *mb_rect, hi_u32 *i, hi_u32 *j);
static hi_s32 tde_osi_get_bpp_by_fmt_1(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_get_bpp_by_fmt_2(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_get_bpp_by_fmt_4(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_get_bpp_by_fmt_8(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_get_bpp_by_fmt_16(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_get_bpp_by_fmt_24(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_get_bpp_by_fmt_32(hi_tde_color_fmt fmt);
static hi_s32 tde_osi_check_single_src_to_para(hi_tde_surface *fore_ground, hi_tde_rect *fore_ground_rect,
                                               hi_tde_surface *dst_surface, hi_tde_rect *dst_rect,
                                               hi_tde_opt *opt);

static hi_s32 tde_osi_double_src_2_blit(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_opt *opt);

static hi_void tde_unify_rect(hi_tde_rect *src_rect, hi_tde_rect *dst_rect)
{
    if (src_rect->height != dst_rect->height) {
        src_rect->height = tde_min(src_rect->height, dst_rect->height);
        dst_rect->height = src_rect->height;
    }
    if (src_rect->width != dst_rect->width) {
        src_rect->width = tde_min(src_rect->width, dst_rect->width);
        dst_rect->width = src_rect->width;
    }

    return;
}

static tde2_fmt_bitoffset g_fmt_bit_and_offset_array[HI_TDE_COLOR_FMT_AYCBCR8888 + 1] = {
    { 0, 4, 4, 4, 12, 8,  4,  0 },  /* HI_TDE_COLOR_FMT_RGB444 */
    { 0, 4, 4, 4, 12, 0,  4,  8 },  /* HI_TDE_COLOR_FMT_BGR444 */
    { 0, 5, 5, 5, 15, 10, 5,  0 },  /* HI_TDE_COLOR_FMT_RGB555 */
    { 0, 5, 5, 5, 15, 0,  5,  10 }, /* HI_TDE_COLOR_FMT_BGR555 */
    { 0, 5, 6, 5, 16, 11, 5,  0 },  /* HI_TDE_COLOR_FMT_RGB565 */
    { 0, 5, 6, 6, 16, 0,  5,  11 }, /* HI_TDE_COLOR_FMT_BGR565 */
    { 0, 8, 8, 8, 24, 16, 8,  0 },  /* HI_TDE_COLOR_FMT_RGB888 */
    { 0, 8, 8, 8, 24, 0,  8,  16 }, /* HI_TDE_COLOR_FMT_BGR888 */
    { 4, 4, 4, 4, 12, 8,  4,  0 },  /* HI_TDE_COLOR_FMT_ARGB4444 */
    { 4, 4, 4, 4, 12, 0,  4,  8 },  /* HI_TDE_COLOR_FMT_ABGR4444 */
    { 4, 4, 4, 4, 0,  12, 8,  4 },  /* HI_TDE_COLOR_FMT_RGBA4444 */
    { 4, 4, 4, 4, 0,  4,  8,  12 }, /* HI_TDE_COLOR_FMT_BGRA4444 */
    { 1, 5, 5, 5, 15, 10, 5,  0 },  /* HI_TDE_COLOR_FMT_ARGB1555 */
    { 1, 5, 5, 5, 15, 0,  5,  10 }, /* HI_TDE_COLOR_FMT_ABGR1555 */
    { 1, 5, 5, 5, 0,  11, 6,  1 },  /* HI_TDE_COLOR_FMT_RGBA1555 */
    { 1, 5, 5, 5, 0,  1,  6,  11 }, /* HI_TDE_COLOR_FMT_BGRA1555 */
    { 8, 5, 6, 5, 16, 11, 5,  0 },  /* HI_TDE_COLOR_FMT_ARGB8565 */
    { 8, 5, 6, 5, 16, 0,  5,  11 }, /* HI_TDE_COLOR_FMT_ABGR8565 */
    { 8, 5, 6, 5, 0,  19, 13, 8 },  /* HI_TDE_COLOR_FMT_RGBA8565 */
    { 8, 5, 6, 6, 0,  8,  13, 19 }, /* HI_TDE_COLOR_FMT_BGRA8565 */
    { 8, 8, 8, 8, 24, 16, 8,  0 },  /* HI_TDE_COLOR_FMT_ARGB8888 */
    { 8, 8, 8, 8, 24, 0,  8,  16 }, /* HI_TDE_COLOR_FMT_ABGR8888 */
    { 8, 8, 8, 8, 0,  24, 16, 8 },  /* HI_TDE_COLOR_FMT_RGBA8888 */
    { 8, 8, 8, 8, 0,  8,  16, 24 }, /* HI_TDE_COLOR_FMT_BGRA8888 */
    { 8, 8, 8, 8, 16, 24, 0,  8 },  /* HI_TDE_COLOR_FMT_RABG8888 */

    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 0, 0, 0, 0,  0,  0, 0 },
    { 0, 8, 8, 8, 24, 16, 8, 0 }, /* HI_TDE_COLOR_FMT_YCbCr888 */
    { 8, 8, 8, 8, 24, 16, 8, 0 }, /* HI_TDE_COLOR_FMT_AYCbCr8888 */
};

tde_color_fmt tde_get_common_drv_color_fmt(hi_u32 count)
{
    return g_tde_common_drv_color_fmt[count];
}

static hi_s32 color_convert_check(hi_tde_fill_color *fill_color, hi_tde_surface *sur, hi_u32 *out_color,
    tde_colorfmt_transform *color_trans)
{
    if (fill_color == HI_NULL) {
        tde_error("pstFillColor is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (sur == HI_NULL) {
        tde_error("pstSur is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (out_color == HI_NULL) {
        tde_error("pu32OutColor is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    *color_trans = tde_osi_get_fmt_trans_type(fill_color->color_fmt, sur->color_fmt);

    if (((fill_color->color_fmt >= HI_TDE_COLOR_FMT_CLUT1) &&
         (fill_color->color_fmt <= HI_TDE_COLOR_FMT_A8)) ||
        (fill_color->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422)) {
        tde_error("Unsupported color!\n");
        return -1;
    }
    return 0;
}

static hi_s32 tde_osi_color_convert(hi_tde_fill_color *fill_color, hi_tde_surface *sur, hi_u32 *out_color)
{
    hi_u8 a, r, g, b, y, cb, cr;
    hi_s32 ret;
    tde_colorfmt_transform color_trans;

    ret = color_convert_check(fill_color, sur, out_color, &color_trans);
    if (ret != 0) {
        return ret;
    }
    a = (fill_color->color_value >> g_fmt_bit_and_offset_array[fill_color->color_fmt].alpha_offset) &
        /* 8 Data from the, 0xff mask */
        (0xff >> (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].alpha_bits));
    r = (fill_color->color_value >> g_fmt_bit_and_offset_array[fill_color->color_fmt].red_offset) &
        /* 8 Data from the, 0xff mask */
        (0xff >> (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].red_bits));
    g = (fill_color->color_value >> g_fmt_bit_and_offset_array[fill_color->color_fmt].green_offset) &
        /* 8 Data from the, 0xff mask */
        (0xff >> (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].green_bits));
    b = (fill_color->color_value >> g_fmt_bit_and_offset_array[fill_color->color_fmt].blue_offset) &
        /* 8 Data from the, 0xff mask */
        (0xff >> (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].blue_bits));

    if ((HI_TDE_COLOR_FMT_ARGB1555 <= fill_color->color_fmt) &&
        (fill_color->color_fmt <= HI_TDE_COLOR_FMT_BGRA1555)) {
        if (a) {
            a = sur->alpha1;
        } else {
            a = sur->alpha0;
        }
    } else {
        a = a << (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].alpha_bits); /* 8 Data from the */
    }

    r = r << (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].red_bits);   /* 8 Data from the */
    g = g << (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].green_bits); /* 8 Data from the */
    b = b << (8 - g_fmt_bit_and_offset_array[fill_color->color_fmt].blue_bits);  /* 8 Data from the */

    switch (color_trans) {
        case TDE_COLORFMT_TRANSFORM_ARGB_ARGB:
        case TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR:
            *out_color = (a << 24) + (r << 16) + (g << 8) + b;   /* 8 16 24 Data from the */
            return 0;
        case TDE_COLORFMT_TRANSFORM_ARGB_YCBCR:
            rgb2ycc(r, g, b, y, cb, cr);
            *out_color = (a << 24) + (y << 16) + (cb << 8) + cr; /* 8 16 24 Data from the */
            return 0;
        case TDE_COLORFMT_TRANSFORM_YCBCR_ARGB:
            ycc2rgb(r, g, b, y, cb, cr);
            *out_color = (a << 24) + (y << 16) + (cb << 8) + cr; /* 8 16 24 Data from the */
            return 0;
        default:
            tde_error("Unsupported color transport!\n");
            return -1;
    }
}

/*
 * Function:      tde_osi_check_resize_para
 * Description:   check zoom ratio limit
 * Return:        TDE_COLORFMT_CATEGORY_E   pixel format category
 */
static hi_s32 tde_osi_check_resize_para(hi_u32 in_width, hi_u32 in_height,
                                        hi_u32 out_width, hi_u32 out_height)
{
    if (((in_width > TDE_MAX_RECT_WIDTH)) ||
        (in_height > TDE_MAX_RECT_HEIGHT) || ((out_width > TDE_MAX_RECT_WIDTH)) ||
        (out_height > TDE_MAX_RECT_HEIGHT)) {
        if ((in_width != out_width) || (in_height != out_height)) {
            tde_error("Can not support resize!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }

    if (((out_width * TDE_MAX_MINIFICATION_H) < in_width) ||
        ((out_height * TDE_MAX_MINIFICATION_V) < in_height)) {
        tde_error("Resize parameter error!\n");
        return -1;
    } else {
        return 0;
    }
}

#if (TDE_CAPABILITY & ROTATE)
static hi_s32 tde_osi_check_rotate_para(hi_tde_single_src *single_src, hi_tde_rotate_angle rotate_angle)
{
    hi_bool is_unsupport_format = ((single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_RGB888) &&
                                   ((single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_YCBCR888)) &&
                                   (single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_YCBCR422) &&
                                   (single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_PKGVYUY) && (
                                   (single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_ARGB8888)) &&
                                   (single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_ARGB4444) &&
                                   (single_src->src_surface->color_fmt != HI_TDE_COLOR_FMT_ARGB1555));

    if (rotate_angle >= HI_TDE_ROTATE_MAX) {
        tde_error("enRotateMode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (single_src->src_surface->color_fmt != single_src->dst_surface->color_fmt) {
        tde_error("Rotate parameter error!\n");
        return -1;
    }
    if (is_unsupport_format) {
        tde_error("Rotate Operation can not support the format!\n");
        return -1;
    }

    if (((single_src->src_surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) ||
         (single_src->src_surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) &&
        ((single_src->src_rect->height & 0x1) || (single_src->src_rect->width & 0x1))) {
        tde_error("Height, width of YCbCr422R couldn't be odd!\n");
        return -1;
    }

    if ((single_src->src_surface->phy_addr % 4) || (single_src->dst_surface->phy_addr % 4)) { /* 4 alg data */
        tde_error("Bitmap address is not aligned!\n");
        return -1;
    }

    if ((single_src->src_surface->stride % 4) || (single_src->dst_surface->stride % 4)) { /* 4 alg data */
        tde_error("stride is not aligned!\n");
        return -1;
    }

    if ((rotate_angle != HI_TDE_ROTATE_CLOCKWISE_180) &&
        ((single_src->src_rect->height != single_src->dst_rect->width) ||
         (single_src->src_rect->width != single_src->dst_rect->height))) {
        tde_error("Rotate operation rect is wrong!\n");
        return -1;
    }

    if ((rotate_angle == HI_TDE_ROTATE_CLOCKWISE_180) &&
        ((single_src->src_rect->height != single_src->dst_rect->height) ||
         (single_src->src_rect->width != single_src->dst_rect->width))) {
        tde_error("Rotate 180 operation rect is wrong!\n");
        return -1;
    }

    return 0;
}
#endif

#if (TDE_CAPABILITY & COMPRESS)
static hi_s32 tde_osi_check_compress_para(hi_tde_surface *fore_ground, hi_tde_rect *fore_ground_rect,
                                          hi_tde_surface *dst_surface, hi_tde_rect *dst_rect, hi_tde_opt *opt)
{
    hi_unused(fore_ground);
    hi_unused(fore_ground_rect);
    hi_unused(dst_rect);
    if ((dst_surface->color_fmt != HI_TDE_COLOR_FMT_ARGB1555) &&
        (dst_surface->color_fmt != HI_TDE_COLOR_FMT_ARGB8888) &&
        (dst_surface->color_fmt != HI_TDE_COLOR_FMT_ARGB4444) &&
        (dst_surface->color_fmt != HI_TDE_COLOR_FMT_RGB888)   &&
        (dst_surface->color_fmt != HI_TDE_COLOR_FMT_YCBCR888) &&
        (dst_surface->color_fmt != HI_TDE_COLOR_FMT_RGB565)) {
        tde_error("Compress operation can not support the fmt !\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    if (opt->mirror) {
        tde_error("Could not support Mirror\n!");
        return -1;
    }
    if (opt->clip_mode != HI_TDE_CLIP_MODE_NONE) {
        tde_error("Could not support Clip\n!");
        return -1;
    }
    return 0;
}

static hi_s32 tde_osi_check_decompress_para(hi_tde_surface *fore_ground, hi_tde_rect *fore_ground_rect,
                                            hi_tde_surface *dst_surface, hi_tde_rect *dst_rect, hi_tde_opt *opt)
{
    hi_unused(fore_ground_rect);
    hi_unused(dst_surface);
    hi_unused(dst_rect);
    if ((fore_ground->color_fmt != HI_TDE_COLOR_FMT_ARGB1555) &&
        (fore_ground->color_fmt != HI_TDE_COLOR_FMT_ARGB8888) &&
        (fore_ground->color_fmt != HI_TDE_COLOR_FMT_ARGB4444) &&
        (fore_ground->color_fmt != HI_TDE_COLOR_FMT_RGB888)   &&
        (fore_ground->color_fmt != HI_TDE_COLOR_FMT_YCBCR888) &&
        (fore_ground->color_fmt != HI_TDE_COLOR_FMT_RGB565)) {
        tde_error("Compress operation can not support the fmt !\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    if (opt->mirror) {
        tde_error("Could not support Mirror\n!");
        return -1;
    }
    return 0;
}

#endif

hi_tde_color_fmt tde_osi_covert_mb_fmt(hi_tde_mb_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_MB_COLOR_FMT_JPG_YCBCR400MBP:
            return HI_TDE_COLOR_FMT_JPG_YCBCR400MBP;
        case HI_TDE_MB_COLOR_FMT_JPG_YCBCR422MBHP:
            return HI_TDE_COLOR_FMT_JPG_YCBCR422MBHP;
        case HI_TDE_MB_COLOR_FMT_JPG_YCBCR422MBVP:
            return HI_TDE_COLOR_FMT_JPG_YCBCR422MBVP;
        case HI_TDE_MB_COLOR_FMT_MP1_YCBCR420MBP:
            return HI_TDE_COLOR_FMT_MP1_YCBCR420MBP;
        case HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBP:
            return HI_TDE_COLOR_FMT_MP2_YCBCR420MBP;
        case HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBI:
            return HI_TDE_COLOR_FMT_MP2_YCBCR420MBI;
        case HI_TDE_MB_COLOR_FMT_JPG_YCBCR420MBP:
            return HI_TDE_COLOR_FMT_JPG_YCBCR420MBP;
        case HI_TDE_MB_COLOR_FMT_JPG_YCBCR444MBP:
            return HI_TDE_COLOR_FMT_JPG_YCBCR444MBP;
        default:
            return HI_TDE_COLOR_FMT_MAX;
    }
}

/*
 * Function:      tde_osi_get_fmt_category
 * Description:   get pixel format category info
 * Input:         fmt: pixel format
 * Return:        TDE_COLORFMT_CATEGORY_E  pixel format category
 */
static tde_colorfmt_category tde_osi_get_fmt_category(hi_tde_color_fmt fmt)
{
    /* target is ARGB format */
    if (fmt <= HI_TDE_COLOR_FMT_RABG8888) {
        return TDE_COLORFMT_CATEGORY_ARGB;
    } else if (fmt <= HI_TDE_COLOR_FMT_ACLUT88) {
        /* target is CLUT table format */
        return TDE_COLORFMT_CATEGORY_CLUT;
    } else if (fmt <= HI_TDE_COLOR_FMT_A8) {
        /* target is alpha CLUT table format */
        return TDE_COLORFMT_CATEGORY_AN;
    } else if (fmt <= HI_TDE_COLOR_FMT_PKGVYUY) {
        /* target is YCbCr format */
        return TDE_COLORFMT_CATEGORY_YCBCR;
    } else if (fmt == HI_TDE_COLOR_FMT_BYTE) {
        /* byte format */
        return TDE_COLORFMT_CATEGORY_BYTE;
    } else if (fmt == HI_TDE_COLOR_FMT_HALFWORD) {
        /* halfword  format */
        return TDE_COLORFMT_CATEGORY_HALFWORD;
    } else if (fmt <= HI_TDE_COLOR_FMT_JPG_YCBCR444MBP) {
        return TDE_COLORFMT_CATEGORY_YCBCR;
    } else {
        /* error format */
        return TDE_COLORFMT_CATEGORY_BUTT;
    }
}

/*
 * Function:      tde_osi_get_fmt_trans_type
 * Description:   get pixel format transform type
 * Input:         src2_fmt: foreground pixel format
                  dst_fmt: target pixel format
 * Return:        TDE_COLORFMT_TRANSFORM_E pixel format transform type
 */
static tde_colorfmt_transform tde_osi_get_fmt_trans_type(hi_tde_color_fmt src2_fmt, hi_tde_color_fmt dst_fmt)
{
    tde_colorfmt_category src_category;
    tde_colorfmt_category dst_category;

    /* get foreground pixel format category */
    src_category = tde_osi_get_fmt_category(src2_fmt);

    /* get target pixel format category */
    dst_category = tde_osi_get_fmt_category(dst_fmt);

    switch (src_category) {
        case TDE_COLORFMT_CATEGORY_ARGB:
            if (dst_category == TDE_COLORFMT_CATEGORY_ARGB) {
                return TDE_COLORFMT_TRANSFORM_ARGB_ARGB;
            } else if (dst_category == TDE_COLORFMT_CATEGORY_YCBCR) {
                return TDE_COLORFMT_TRANSFORM_ARGB_YCBCR;
            } else if (dst_category == TDE_COLORFMT_CATEGORY_AN) {
                return TDE_COLORFMT_TRANSFORM_ARGB_AN;
            }
            return TDE_COLORFMT_TRANSFORM_BUTT;

        case TDE_COLORFMT_CATEGORY_CLUT:
            if (dst_category == TDE_COLORFMT_CATEGORY_ARGB) {
                return TDE_COLORFMT_TRANSFORM_CLUT_ARGB;
            } else if (dst_category == TDE_COLORFMT_CATEGORY_YCBCR) {
                return TDE_COLORFMT_TRANSFORM_CLUT_YCBCR;
            } else if (dst_category == TDE_COLORFMT_CATEGORY_CLUT) {
                return TDE_COLORFMT_TRANSFORM_CLUT_CLUT;
            }
            return TDE_COLORFMT_TRANSFORM_BUTT;

        case TDE_COLORFMT_CATEGORY_YCBCR:
            if (dst_category == TDE_COLORFMT_CATEGORY_ARGB) {
                return TDE_COLORFMT_TRANSFORM_YCBCR_ARGB;
            } else if (dst_category == TDE_COLORFMT_CATEGORY_YCBCR) {
                return TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR;
            } else if (dst_category == TDE_COLORFMT_CATEGORY_AN) {
                return TDE_COLORFMT_TRANSFORM_ARGB_AN;
            }
            return TDE_COLORFMT_TRANSFORM_BUTT;

        case TDE_COLORFMT_CATEGORY_AN:
            if (dst_category == TDE_COLORFMT_CATEGORY_AN) {
                return TDE_COLORFMT_TRANSFORM_AN_AN;
            }
            return TDE_COLORFMT_TRANSFORM_BUTT;

        default:
            return TDE_COLORFMT_TRANSFORM_BUTT;
    }
}

/*
 * Function:      tde_osi_is_single_src_to_rop
 * Description:   query if ROP operate is if single source2 operate
 * Input:         rop: rop operate type
 * Return:        HI_TRUE: single ROP;HI_FALSE: non single ROP
 */
#if (TDE_CAPABILITY & ROP)
static hi_bool tde_osi_is_single_src_to_rop(hi_tde_rop_mode rop)
{
    switch (rop) {
        case HI_TDE_ROP_BLACK:
        case HI_TDE_ROP_NOTCOPYPEN:
        case HI_TDE_ROP_COPYPEN:
        case HI_TDE_ROP_WHITE:
            return HI_TRUE;

        default:
            return HI_FALSE;
    }
}
#endif

/*
 * Function:      tde_osi_get_clut_usage
 * Description:   get CLUT table usage
 * Input:         src_fmt foreground pixel format
                  dst_fmt  target pixel format
 * Return:        TDE_CLUT_USAGE_E:  clut  usage
 */
static tde_clut_usage tde_osi_get_clut_usage(hi_tde_color_fmt src_fmt, hi_tde_color_fmt dst_fmt)
{
    tde_colorfmt_transform color_trans_type;

    color_trans_type = tde_osi_get_fmt_trans_type(src_fmt, dst_fmt);

    switch (color_trans_type) {
        case TDE_COLORFMT_TRANSFORM_CLUT_ARGB:
        case TDE_COLORFMT_TRANSFORM_CLUT_YCBCR:
            return TDE_CLUT_COLOREXPENDING; /* color expand */

        case TDE_COLORFMT_TRANSFORM_ARGB_ARGB:
        case TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR:
            return TDE_CLUT_COLORCORRECT; /* color adjust */

        case TDE_COLORFMT_TRANSFORM_CLUT_CLUT:
            return TDE_CLUT_CLUT_BYPASS;

        default:
            return TDE_CLUT_USAGE_BUTT;
    }
}

static hi_bool tde_osi_whether_contain_alpha(hi_tde_color_fmt color_fmt)
{
    switch (color_fmt) {
        case HI_TDE_COLOR_FMT_ARGB4444:
        case HI_TDE_COLOR_FMT_ABGR4444:
        case HI_TDE_COLOR_FMT_RGBA4444:
        case HI_TDE_COLOR_FMT_BGRA4444:
        case HI_TDE_COLOR_FMT_ARGB1555:
        case HI_TDE_COLOR_FMT_ABGR1555:
        case HI_TDE_COLOR_FMT_RGBA1555:
        case HI_TDE_COLOR_FMT_BGRA1555:
        case HI_TDE_COLOR_FMT_ARGB8565:
        case HI_TDE_COLOR_FMT_ABGR8565:
        case HI_TDE_COLOR_FMT_RGBA8565:
        case HI_TDE_COLOR_FMT_BGRA8565:
        case HI_TDE_COLOR_FMT_ARGB8888:
        case HI_TDE_COLOR_FMT_ABGR8888:
        case HI_TDE_COLOR_FMT_RGBA8888:
        case HI_TDE_COLOR_FMT_BGRA8888:
        case HI_TDE_COLOR_FMT_AYCBCR8888:
        case HI_TDE_COLOR_FMT_RABG8888:
            return HI_TRUE;
        default:
            return HI_FALSE;
    }
}

/*
 * Function:      tde_osi_set_clut_opt
 * Description:   set clut parameter
 * Input:         clut_sur Clut bitmap info
 *                pen_clut_usage  return clut usage type: expand/adjust
 *                clut_reload   if reload clut
 *                hw_node     hardware node information
 * Return:        TDE_CLUT_USAGE_E: clut usage
 */
static hi_s32 tde_osi_set_clut_opt(hi_tde_surface *clut_sur, hi_tde_surface *out_sur, tde_clut_usage *pen_clut_usage,
                                   hi_bool clut_reload, tde_hw_node *hw_node)
{
    tde_colorfmt_category fmt_cate;
    tde_clut_cmd clut_cmd;
    hi_u64 clut_phyaddr;
    /* 0xffffffffffffffffUL flag value */
    if (!((clut_sur->clut_phy_addr != 0xffffffffffffffffUL) && (clut_sur->clut_phy_addr != 0))) {
        return HI_SUCCESS;
    }
    clut_phyaddr = clut_sur->clut_phy_addr;
    fmt_cate = tde_osi_get_fmt_category(out_sur->color_fmt);
    /* when user input the type of clut is not consistent with output format,return error */
    if ((!clut_sur->is_ycbcr_clut && (fmt_cate == TDE_COLORFMT_CATEGORY_YCBCR)) ||
        (clut_sur->is_ycbcr_clut && (fmt_cate == TDE_COLORFMT_CATEGORY_ARGB))) {
        tde_error("clut fmt not same\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    /*
     * Because of unsupported output CSC, input and background color zone is consistent.
     * In trine sources operation,clut need use background, so output color zone instand of background's
     */
    *pen_clut_usage = tde_osi_get_clut_usage(clut_sur->color_fmt, out_sur->color_fmt);

    if (*pen_clut_usage >= TDE_CLUT_CLUT_BYPASS) {
        return HI_SUCCESS;
    }

    if (*pen_clut_usage == TDE_CLUT_COLOREXPENDING) {
        clut_cmd.clut_mode = TDE_COLOR_EXP_CLUT_MODE;
    } else {
        clut_cmd.clut_mode = TDE_COLOR_CORRCT_CLUT_MODE;
    }

    if (osal_div_u64_rem(clut_phyaddr, 4)) { /* 4 size */
        tde_error("clut_phy_addr=0x%llx is not aligned!\n", clut_sur->clut_phy_addr);
        return HI_ERR_TDE_NOT_ALIGNED;
    }

    clut_cmd.phy_clut_addr = clut_sur->clut_phy_addr;

    if (tde_hal_node_set_clut_opt(hw_node, &clut_cmd, clut_reload) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_single_opt(hi_tde_opt *opt)
{
    hi_bool real = (((opt->blend_opt.global_alpha_en != HI_TRUE) && (opt->blend_opt.global_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.pixel_alpha_en != HI_TRUE) && (opt->blend_opt.pixel_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.src1_alpha_premulti != HI_TRUE) && (opt->blend_opt.src1_alpha_premulti != HI_FALSE)) ||
        ((opt->blend_opt.src2_alpha_premulti != HI_TRUE) && (opt->blend_opt.src2_alpha_premulti != HI_FALSE)) ||
        ((opt->clut_reload != HI_TRUE) && (opt->clut_reload != HI_FALSE)) ||
        ((opt->resize != HI_TRUE) && (opt->resize != HI_FALSE)) ||
        ((opt->is_compress != HI_TRUE) && (opt->is_compress != HI_FALSE)) ||
        ((opt->is_decompress != HI_TRUE) && (opt->is_decompress != HI_FALSE)));

    /* return error, if enable color key */
    if (opt->color_key_mode != HI_TDE_COLOR_KEY_MODE_NONE) {
        tde_error("It doesn't support colorkey in single source mode!\n");
        return -1;
    }

    if (opt->alpha_blending_cmd >= HI_TDE_ALPHA_BLENDING_MAX) {
        tde_error("alpha_blending_cmd error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (real) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->deflicker_mode >= HI_TDE_DEFLICKER_LEVEL_MODE_MAX) ||
        (opt->deflicker_mode < HI_TDE_DEFLICKER_LEVEL_MODE_NONE)) {
        tde_error("deflicker_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->mirror >= HI_TDE_MIRROR_MAX) || (opt->mirror < HI_TDE_MIRROR_NONE)) {
        tde_error("mirror error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->filter_mode >= HI_TDE_FILTER_MODE_MAX) || (opt->filter_mode < HI_TDE_FILTER_MODE_COLOR)) {
        tde_error("filter_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
static hi_s32 tde_osi_check_dst_fmt(hi_tde_color_fmt fmt)
{
    if (((fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) && (fmt <= HI_TDE_COLOR_FMT_JPG_YCBCR422MBVP)) ||
        (fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR444MBP) || ((fmt >= HI_TDE_COLOR_FMT_CLUT1) &&
        (fmt <= HI_TDE_COLOR_FMT_A8))) {
        tde_error("This enFmt %d doesn't support!\n", fmt);
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}
#else
static hi_s32 tde_osi_check_dst_fmt(hi_tde_color_fmt fmt)
{
    if (((fmt >= HI_TDE_COLOR_FMT_YCBCR888) && (fmt <= HI_TDE_COLOR_FMT_PKGVYUY)) ||
        ((fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) &&
        (fmt <= HI_TDE_COLOR_FMT_JPG_YCBCR422MBVP)) ||
        (fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR444MBP) ||
        ((fmt >= HI_TDE_COLOR_FMT_CLUT1) && (fmt <= HI_TDE_COLOR_FMT_A8))) {
        tde_error("This enFmt %d doesn't support!\n", fmt);
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_check_single_src_to_opt
 * Description:   check if valid of foreground single source operate
 * Input:         src2_fmt foreground pixel format
                  dst_fmt  target pixel format
                  opt     operate attribute pointer
 * Return:        0  valid parameter;
                  -1 invalid parameter;
 */
static hi_s32 tde_osi_check_single_src_to_opt(hi_tde_color_fmt src2_fmt, hi_tde_color_fmt dst_fmt, hi_tde_opt *opt)
{
    tde_colorfmt_transform color_trans_type;
    hi_s32 ret = tde_osi_check_single_opt(opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

#if (TDE_CAPABILITY & ROP)
    /* if operate type is ROP and it is not single operate,return error */
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) {
        if (((opt->rop_alpha >= HI_TDE_ROP_MAX) || (opt->rop_alpha < HI_TDE_ROP_BLACK)) ||
            ((opt->rop_color >= HI_TDE_ROP_MAX) || (opt->rop_color < HI_TDE_ROP_BLACK))) {
            return HI_ERR_TDE_INVALID_PARA;
        }

        if ((!tde_osi_is_single_src_to_rop(opt->rop_alpha)) || (!tde_osi_is_single_src_to_rop(opt->rop_color))) {
            tde_error("Only support single s2 rop!\n");
            return -1;
        }
    }
#endif
    /* single source can not do blend operate */
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_BLEND) {
        tde_error("Alu mode error!\n");
        return -1;
    }
    ret = tde_osi_check_dst_fmt(dst_fmt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    color_trans_type = tde_osi_get_fmt_trans_type(src2_fmt, dst_fmt);
    if (color_trans_type == TDE_COLORFMT_TRANSFORM_BUTT) {
        tde_error("Unknown color transport type!\n");
        return -1;
    }

    if (color_trans_type == TDE_COLORFMT_TRANSFORM_CLUT_CLUT) {
        /* unsupported deflicker,zoom, Rop, mirror,colorize */
        if ((opt->resize) || (opt->deflicker_mode != HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ||
            ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) ||
            ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_COLORIZE)) {
            tde_error("It doesn't support deflicker or ROP or mirror!\n");
            return -1;
        }
    }

    return 0;
}

static hi_s32 tde_osi_check_bool_single_opt(hi_tde_opt *opt)
{
    hi_bool real = (((opt->blend_opt.global_alpha_en != HI_TRUE) && (opt->blend_opt.global_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.pixel_alpha_en != HI_TRUE) && (opt->blend_opt.pixel_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.src1_alpha_premulti != HI_TRUE) && (opt->blend_opt.src1_alpha_premulti != HI_FALSE)) ||
        ((opt->blend_opt.src2_alpha_premulti != HI_TRUE) && (opt->blend_opt.src2_alpha_premulti != HI_FALSE)) ||
        ((opt->clut_reload != HI_TRUE) && (opt->clut_reload != HI_FALSE)) ||
        ((opt->resize != HI_TRUE) && (opt->resize != HI_FALSE)) ||
        ((opt->is_compress != HI_TRUE) && (opt->is_compress != HI_FALSE)) ||
        ((opt->is_decompress != HI_TRUE) && (opt->is_decompress != HI_FALSE)));
    if (real) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->deflicker_mode >= HI_TDE_DEFLICKER_LEVEL_MODE_MAX) ||
        (opt->deflicker_mode < HI_TDE_DEFLICKER_LEVEL_MODE_NONE)) {
        tde_error("deflicker_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->mirror >= HI_TDE_MIRROR_MAX) || (opt->mirror < HI_TDE_MIRROR_NONE)) {
        tde_error("mirror error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->filter_mode >= HI_TDE_FILTER_MODE_MAX) || (opt->filter_mode < HI_TDE_FILTER_MODE_COLOR)) {
        tde_error("filter_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_fmt_single_opt(hi_tde_color_fmt src1_fmt, hi_tde_opt *opt)
{
    hi_s32 ret;

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    if ((src1_fmt == HI_TDE_COLOR_FMT_YCBCR422) || (src1_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("This operation doesn't support PKG!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#else
    hi_unused(src1_fmt);
#endif
    if (opt->alpha_blending_cmd >= HI_TDE_ALPHA_BLENDING_MAX) {
        tde_error("alpha_blending_cmd error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    ret = tde_osi_check_bool_single_opt(opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_cmd_single_opt(hi_tde_opt *opt)
{
#if (TDE_CAPABILITY & ROP)
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) {
        if (((opt->rop_alpha >= HI_TDE_ROP_MAX) || (opt->rop_alpha < HI_TDE_ROP_BLACK)) ||
            ((opt->rop_color >= HI_TDE_ROP_MAX) || (opt->rop_color < HI_TDE_ROP_BLACK))) {
            tde_error("enRopCode error!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
#endif
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_BLEND) {
        if ((opt->blend_opt.blend_cmd >= HI_TDE_BLEND_CMD_MAX) ||
            (opt->blend_opt.blend_cmd < HI_TDE_BLEND_CMD_NONE)) {
            tde_error("Unknown blend cmd!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }

        if (opt->blend_opt.blend_cmd == HI_TDE_BLEND_CMD_CONFIG) {
            if ((opt->blend_opt.src1_blend_mode >= HI_TDE_BLEND_MAX) ||
                (opt->blend_opt.src2_blend_mode >= HI_TDE_BLEND_MAX)) {
                tde_error("Unknown blend mode!\n");
                return HI_ERR_TDE_INVALID_PARA;
            }
        }
    }
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_check_double_src_opt
 * Description:   check if valid of dual source operate
 * Input:         src1_fmt background pixel format
                  src2_fmt foreground pixel format
                  dst_fmt  target pixel format
                  opt    operate attribute operate
 * Return:        0  valid parameter;
                  -1 invalid parameter;
 */
static hi_s32 tde_osi_check_double_src_opt(hi_tde_color_fmt src1_fmt, hi_tde_color_fmt src2_fmt,
                                           hi_tde_color_fmt dst_fmt, hi_tde_opt *opt)
{
    tde_colorfmt_category src1_category, src2_category, dst_category;
    hi_bool temp_fmt = HI_FALSE;
    hi_s32 ret = tde_osi_check_dst_fmt(dst_fmt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    ret = tde_osi_check_fmt_single_opt(src1_fmt, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    /* get background pixel format category */
    src1_category = tde_osi_get_fmt_category(src1_fmt);

    /* get foreground pixel format category  */
    src2_category = tde_osi_get_fmt_category(src2_fmt);

    /* get target pixel format category  */
    dst_category = tde_osi_get_fmt_category(dst_fmt);
    if ((src1_category >= TDE_COLORFMT_CATEGORY_BYTE) || (src2_category >= TDE_COLORFMT_CATEGORY_BYTE) ||
        (dst_category >= TDE_COLORFMT_CATEGORY_BYTE)) {
        tde_error("unknown format!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    temp_fmt = tde_osi_whether_contain_alpha(dst_fmt);
    if (((src1_category == TDE_COLORFMT_CATEGORY_ARGB) || (src1_category == TDE_COLORFMT_CATEGORY_YCBCR)) &&
        (src2_category == TDE_COLORFMT_CATEGORY_AN) && (!temp_fmt)) {
        tde_error("Target must have alpha component!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((src1_category == TDE_COLORFMT_CATEGORY_CLUT) && ((src2_category != TDE_COLORFMT_CATEGORY_CLUT) ||
        (dst_category != TDE_COLORFMT_CATEGORY_CLUT))) {
        tde_error("Unsupported operation!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    if (((opt->deflicker_mode != HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ||
        (opt->resize) || (opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE)) &&
        (src1_category == TDE_COLORFMT_CATEGORY_CLUT)) {
        tde_error("It doesn't support deflicker or ROP or mirror!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if ((src1_category == TDE_COLORFMT_CATEGORY_AN) && (src2_category == TDE_COLORFMT_CATEGORY_AN) &&
        (dst_category == TDE_COLORFMT_CATEGORY_AN) && (opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE)) {
        tde_error("It doesn't support ROP or mirror!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = tde_osi_check_cmd_single_opt(opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    return HI_SUCCESS;
}

static hi_u16 tde_osi_get_code1(tde_colorfmt_transform color_trans_type)
{
    switch (color_trans_type) {
        case TDE_COLORFMT_TRANSFORM_ARGB_ARGB:
            return 0x0;
        case TDE_COLORFMT_TRANSFORM_ARGB_YCBCR:
            return 0x5;
        case TDE_COLORFMT_TRANSFORM_CLUT_ARGB:
            return 0x8;
        case TDE_COLORFMT_TRANSFORM_CLUT_YCBCR:
            return 0x8 | 0x10 | 0x1;
        case TDE_COLORFMT_TRANSFORM_YCBCR_ARGB:
            return 0x1;
        case TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR:
            return 0x0;
        default:
            return 0x8000;
    }
}

static hi_u16 tde_osi_get_code2(tde_colorfmt_transform color_trans_type, hi_tde_color_fmt src2_fmt)
{
    hi_u16 code2;
    switch (color_trans_type) {
        case TDE_COLORFMT_TRANSFORM_ARGB_ARGB:
            return 0x0;
        case TDE_COLORFMT_TRANSFORM_ARGB_YCBCR:
            return 0x2;
        case TDE_COLORFMT_TRANSFORM_CLUT_ARGB:
            return 0x8;
        case TDE_COLORFMT_TRANSFORM_CLUT_YCBCR:
            return 0xa;
        case TDE_COLORFMT_TRANSFORM_YCBCR_ARGB:
            code2 = 0x2 | 0x4;
            if ((src2_fmt >= HI_TDE_COLOR_FMT_CLUT1) && (src2_fmt <= HI_TDE_COLOR_FMT_ACLUT88)) {
                code2 = 0;
            }
            return code2;
        case TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR:
            code2 = 0x0;
            if ((src2_fmt >= HI_TDE_COLOR_FMT_CLUT1) && (src2_fmt <= HI_TDE_COLOR_FMT_ACLUT88)) {
                code2 = 0x2;
            }
            return code2;
        default:
            return 0x8000;
    }
}

/*
 * Function:      tde_osi_double_src_get_opt_code
 * Description:   get dual source operate encode
 * Input:         src1_fmt background pixel format
                  src2_fmt foreground pixel format
                  dst_fmt: target pixel format
 * Return:        code value
 */
static hi_u16 tde_osi_double_src_get_opt_code(hi_tde_color_fmt src1_fmt, hi_tde_color_fmt src2_fmt,
                                              hi_tde_color_fmt dst_fmt)
{
    hi_u16 code1;
    hi_u16 code2;
    tde_colorfmt_transform color_trans_type;

    color_trans_type = tde_osi_get_fmt_trans_type(src2_fmt, src1_fmt);
    code1 = tde_osi_get_code1(color_trans_type);
    if (code1 == 0x8000) { /* 0x8000 flag value */
        return code1;
    }

    color_trans_type = tde_osi_get_fmt_trans_type(src1_fmt, dst_fmt);

    code2 = tde_osi_get_code2(color_trans_type, src2_fmt);
    if (code2 == 0x8000) { /* 0x8000 flag value */
        return code2;
    }
    return (code1 | code2);
}

/*
 * Function:      tde_osi_single_src_to_get_opt_code
 * Description:   get dual source operate encode
 * Input:         src2_fmt foreground pixel format
                  dst_fmt  target pixel format
 * Return:        encode value, expression is fllowinf:

 |------0-----|------1-----|------2-------|------3-----|
 |-----ICSC---|-----OCSC---|(in)RGB->YCbCr|-----CLUT---|

 */
static hi_u16 tde_osi_single_src_to_get_opt_code(hi_tde_color_fmt src2_fmt, hi_tde_color_fmt dst_fmt)
{
    hi_u16 code;
    tde_colorfmt_transform color_trans_type;

    color_trans_type = tde_osi_get_fmt_trans_type(src2_fmt, dst_fmt);

    switch (color_trans_type) {
        case TDE_COLORFMT_TRANSFORM_ARGB_ARGB:
            code = 0x0;
            break;
        case TDE_COLORFMT_TRANSFORM_ARGB_YCBCR:
            code = 0x5;
            break;
        case TDE_COLORFMT_TRANSFORM_CLUT_ARGB:
            code = 0x8;
            break;
        case TDE_COLORFMT_TRANSFORM_CLUT_CLUT:
            code = 0x0;
            break;
        case TDE_COLORFMT_TRANSFORM_CLUT_YCBCR:
            code = 0xA;
            break;
        case TDE_COLORFMT_TRANSFORM_YCBCR_ARGB:
            code = 0x1;
            break;
        case TDE_COLORFMT_TRANSFORM_YCBCR_YCBCR:
            code = 0x0;
            break;
        default:
            code = 0x8000;
    }
    return code;
}

/*
 * Function:      tde_osi_get_conv_by_code
 * Description:   get format conversion manner by format conversion code
 * Input:         code  format conversion code
 *                conv  format conversion struct
 * Return:        encode value
 */
static hi_void tde_osi_get_conv_by_code(hi_u16 code, tde_conv_mode_cmd *conv)
{
    conv->in_conv = code & 0x1;
    conv->out_conv = (code >> 1) & 0x1;
    conv->in_rgb2_yc = ((code >> 2) & 0x1);   /* 2 Expand the digits */
    conv->in_src1_conv = ((code >> 4) & 0x1); /* 4 Expand the digits */

    return;
}

/*
 * Function:      tde_osi_get_bpp_by_fmt
 * Description:   get pixel bit of pixel format
 * Input:         fmt  target pixel format
 * Return:        -1 fail; other:pixel bit
 */
static hi_s32 tde_osi_get_bpp_by_fmt(hi_tde_color_fmt fmt)
{
    if (tde_osi_get_bpp_by_fmt_16(fmt) == 16) { /* 16 bpp fmt */
        return 16;
    } else if (tde_osi_get_bpp_by_fmt_24(fmt) == 24) { /* 24 bpp fmt */
        return 24;
    } else if (tde_osi_get_bpp_by_fmt_32(fmt) == 32) { /* 32 bpp fmt */
        return 32;
    } else if (tde_osi_get_bpp_by_fmt_1(fmt) == 1) {
        return 1;
    } else if (tde_osi_get_bpp_by_fmt_2(fmt) == 2) { /* 2 bpp fmt */
        return 2;
    } else if (tde_osi_get_bpp_by_fmt_4(fmt) == 4) { /* 4 bpp fmt */
        return 4;
    } else if (tde_osi_get_bpp_by_fmt_8(fmt) == 8) { /* 8 bpp fmt */
        return 8;
    } else {
        return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_16(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_RGB444:
        case HI_TDE_COLOR_FMT_BGR444:
        case HI_TDE_COLOR_FMT_RGB555:
        case HI_TDE_COLOR_FMT_BGR555:
        case HI_TDE_COLOR_FMT_RGB565:
        case HI_TDE_COLOR_FMT_BGR565:
        case HI_TDE_COLOR_FMT_ARGB4444:
        case HI_TDE_COLOR_FMT_ABGR4444:
        case HI_TDE_COLOR_FMT_RGBA4444:
        case HI_TDE_COLOR_FMT_BGRA4444:
        case HI_TDE_COLOR_FMT_ARGB1555:
        case HI_TDE_COLOR_FMT_ABGR1555:
        case HI_TDE_COLOR_FMT_RGBA1555:
        case HI_TDE_COLOR_FMT_BGRA1555:
        case HI_TDE_COLOR_FMT_ACLUT88:
        case HI_TDE_COLOR_FMT_YCBCR422:
        case HI_TDE_COLOR_FMT_HALFWORD:
        case HI_TDE_COLOR_FMT_PKGVYUY:
            return 16; /* 16 bpp fmt */
        default:
            return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_24(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_RGB888:
        case HI_TDE_COLOR_FMT_BGR888:
        case HI_TDE_COLOR_FMT_ARGB8565:
        case HI_TDE_COLOR_FMT_ABGR8565:
        case HI_TDE_COLOR_FMT_RGBA8565:
        case HI_TDE_COLOR_FMT_BGRA8565:
        case HI_TDE_COLOR_FMT_YCBCR888:
            return 24; /* 24 bpp fmt */
        default:
            return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_32(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_ARGB8888:
        case HI_TDE_COLOR_FMT_ABGR8888:
        case HI_TDE_COLOR_FMT_RGBA8888:
        case HI_TDE_COLOR_FMT_BGRA8888:
        case HI_TDE_COLOR_FMT_AYCBCR8888:
        case HI_TDE_COLOR_FMT_RABG8888:
            return 32; /* 32 bpp fmt */
        default:
            return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_8(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_CLUT8:
        case HI_TDE_COLOR_FMT_ACLUT44:
        case HI_TDE_COLOR_FMT_A8:
        case HI_TDE_COLOR_FMT_BYTE:
            return 8; /* 8 bpp fmt */
        default:
            return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_4(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_CLUT4:
            return 4; /* 4 bpp fmt */
        default:
            return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_2(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_CLUT2:
            return 2; /* 2 bpp fmt */
        default:
            return -1;
    }
}

static hi_s32 tde_osi_get_bpp_by_fmt_1(hi_tde_color_fmt fmt)
{
    switch (fmt) {
        case HI_TDE_COLOR_FMT_CLUT1:
        case HI_TDE_COLOR_FMT_A1:
            return 1;
        default:
            return -1;
    }
}

/*
 * Function:      tde_osi_check_src
 * Description:   get scanning direction, avoid lap
 * Input:         pSrc source bitmap
                  dst_surface target bitmap
                  mirror mirror type
 * Output:        pstSrcDirection source scanning information
                  pstDstDirection target scanning information
 * Return:        0  success
                  -1 fail
 * Others:        add  antiscan handle to YCbCr422R
 */
static hi_s32 tde_osi_check_src(hi_tde_single_src *single_src, tde_scandirection_mode *src_direction,
    tde_scandirection_mode *dst_direction)
{
    hi_s32 bpp;

    if (single_src->src_surface == HI_NULL) {
        tde_error("single_src->src_surface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    if (single_src->src_rect == HI_NULL) {
        tde_error("single_src->src_rect is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    if (single_src->dst_surface == HI_NULL) {
        tde_error("single_src->dst_surface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    if (single_src->dst_rect == HI_NULL) {
        tde_error("single_src->dst_rect is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    if (src_direction == HI_NULL) {
        tde_error("pstSrcDirection is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    if (dst_direction == HI_NULL) {
        tde_error("pstDstDirection is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    bpp = tde_osi_get_bpp_by_fmt(single_src->dst_surface->color_fmt);
    if (bpp < 0) {
        tde_error("Unknown color format!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (bpp < 8) { /* 8 is bpp */
        /* when writing, 8 subbyte format align ask start point byte align */
        if ((single_src->dst_rect->pos_x * bpp % 8) || (single_src->dst_rect->width * bpp % 8)) {
            tde_error("Subbyte's start or width for writing is not aligned!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    return HI_SUCCESS;
}

static hi_void tde_check_reverse_scan(hi_u32 src_addr, hi_u32 dst_addr, tde_scandirection_mode *src_direction,
    tde_scandirection_mode *dst_direction)
{
    if (src_addr <= dst_addr) {
        src_direction->v_scan = TDE_SCAN_DOWN_UP;
        dst_direction->v_scan = TDE_SCAN_DOWN_UP;

        src_direction->h_scan = TDE_SCAN_RIGHT_LEFT;
        dst_direction->h_scan = TDE_SCAN_RIGHT_LEFT;
    }
    return;
}

static hi_void tde_osi_get_src_direction(hi_tde_mirror_mode mirror, tde_scandirection_mode *src_direction)
{
    switch (mirror) {
        case HI_TDE_MIRROR_HORIZONTAL:
            src_direction->h_scan = !(src_direction->h_scan);
            break;
        case HI_TDE_MIRROR_VERTICAL:
            src_direction->v_scan = !(src_direction->v_scan);
            break;
        case HI_TDE_MIRROR_BOTH:
            src_direction->h_scan = !(src_direction->h_scan);
            src_direction->v_scan = !(src_direction->v_scan);
            break;
        default:
            break;
    }
}

static hi_s32 tde_osi_get_scan_info_ex(hi_tde_single_src *single_src, hi_tde_opt *opt,
    tde_scandirection_mode *src_direction, tde_scandirection_mode *dst_direction)
{
    hi_u32 src_addr, dst_addr;
    hi_tde_mirror_mode mirror = HI_TDE_MIRROR_NONE;
    hi_s32 srcd_bpp, dst_bpp, ret;

    ret = tde_osi_check_src(single_src, src_direction, dst_direction);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    /* default scanning direction */
    src_direction->h_scan = TDE_SCAN_LEFT_RIGHT;
    src_direction->v_scan = TDE_SCAN_UP_DOWN;
    dst_direction->h_scan = TDE_SCAN_LEFT_RIGHT;
    dst_direction->v_scan = TDE_SCAN_UP_DOWN;

    if (opt != HI_NULL) {
        mirror = opt->mirror;
    }

    if (mirror != HI_TDE_MIRROR_NONE) {
        tde_osi_get_src_direction(mirror, src_direction);
    } else {
        /* only if stride is the same, can be do conversion */
        if ((single_src->src_surface->stride != single_src->dst_surface->stride) ||
            (single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP)) {
            return 0;
        }
        srcd_bpp = tde_osi_get_bpp_by_fmt(single_src->src_surface->color_fmt);
        dst_bpp = tde_osi_get_bpp_by_fmt(single_src->dst_surface->color_fmt);
        if ((dst_bpp < 0) || (srcd_bpp < 0)) {
            tde_error("bits per pixel less than 0!src fmt:%d,dst fmt:%d\n",
                single_src->src_surface->color_fmt, single_src->dst_surface->color_fmt);
            return -1;
        }

        src_addr = single_src->src_surface->phy_addr + single_src->src_rect->pos_y *
            single_src->src_surface->stride + ((hi_u32)(single_src->src_rect->pos_x * srcd_bpp) >> 3); /* 3 bpp fmt */
        dst_addr = single_src->dst_surface->phy_addr + single_src->dst_rect->pos_y *
            /* 3 bpp fmt */
            single_src->dst_surface->stride  + ((hi_u32)(single_src->dst_rect->pos_x * dst_bpp) >> 3);

        /* clip and reverse cannot open at the same time, the effect will be abnormal */
        if ((opt != HI_NULL) && (opt->clip_mode != HI_TDE_CLIP_MODE_NONE) && (opt->resize)) {
            return 0;
        }

        /* source is above of target or on the left of the same direction */
        tde_check_reverse_scan(src_addr, dst_addr, src_direction, dst_direction);
    }
    return 0;
}

/*
 * Function:      tde_osi_get_inter_rect
 * Description:   get inter rect of two rectangles
 * Output:        inter_rect output inter rectangle
 * Return:        0  have inter zone
                  -1 no inter zone
 */
static hi_s32 tde_osi_get_inter_rect(hi_tde_rect *dst_rect, hi_tde_rect *clip_rect, hi_tde_rect *inter_rect)
{
    hi_s32 left, top, right, bottom;
    hi_s32 right1, bottom1, right2, bottom2;
    hi_s32 pos_x, pos_y;

    pos_x = dst_rect->pos_x;
    pos_y = dst_rect->pos_y;
    left = (pos_x > clip_rect->pos_x) ? pos_x : clip_rect->pos_x;
    top = (pos_y > clip_rect->pos_y) ? pos_y : clip_rect->pos_y;

    right1 = pos_x + dst_rect->width - 1;
    right2 = clip_rect->pos_x + clip_rect->width - 1;
    right = (right1 > right2) ? right2 : right1;

    bottom1 = pos_y + dst_rect->height - 1;
    bottom2 = clip_rect->pos_y + clip_rect->height - 1;
    bottom = (bottom1 > bottom2) ? bottom2 : bottom1;

    if ((left > right) || (top > bottom)) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    inter_rect->pos_x = left;
    inter_rect->pos_y = top;
    inter_rect->width = right - left + 1;
    inter_rect->height = bottom - top + 1;
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_is_rect1_in_rect2
 * Description:   Rect1 is if inside Rect2
 * Input:         rect1  input rectangle1
                  rect2  input rectangle2
 * Return:        1  have inter zone
                  0  no inter zone
 */
static hi_bool tde_osi_is_rect1_in_rect2(hi_tde_rect *rect1, hi_tde_rect *rect2)
{
    hi_s32 right1 = rect1->pos_x + rect1->width - 1;
    hi_s32 right2 = rect2->pos_x + rect2->width - 1;

    hi_s32 bottom1 = rect1->pos_y + rect1->height - 1;
    hi_s32 bottom2 = rect2->pos_y + rect2->height - 1;

    if ((rect1->pos_x >= rect2->pos_x) &&
        (rect1->pos_y >= rect2->pos_y) &&
        (right1 <= right2) &&
        (bottom1 <= bottom2)) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

/*
 * Function:      tde_osi_set_mb_para
 * Description:   MB operate setting parameter interface
 * Input:         handle: task handle
 *                dst_surface:  target bitmap information struct
 *                dst_rect: target bitmap operate zone
 *                mb_opt:  operate parameter setting struct
 */
static hi_s32 tde_osi_set_mb_para(hi_s32 handle, hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt)
{
    hi_tde_surface *fore_ground = HI_NULL;
    hi_tde_opt *opt = HI_NULL;
    hi_s32 ret;
    hi_tde_single_src single_src;

    fore_ground = (hi_tde_surface *)tde_malloc(sizeof(hi_tde_surface));
    if (fore_ground == HI_NULL) {
        tde_error ("malloc pstForeGround failed, size=%ld!\n", (unsigned long)(sizeof(hi_tde_surface)));
        return HI_ERR_TDE_NO_MEM;
    }
    opt = (hi_tde_opt *)tde_malloc(sizeof(hi_tde_opt));
    if (opt == HI_NULL) {
        tde_error ("malloc pstOpt failed, size=%ld!\n", (unsigned long)(sizeof(hi_tde_opt)));
        tde_free((hi_void *)fore_ground);
        return HI_ERR_TDE_NO_MEM;
    }
    fore_ground->phy_addr = mb_src->mb_surface->y_addr;
    fore_ground->stride = mb_src->mb_surface->y_stride;
    fore_ground->width = mb_src->mb_surface->y_width;
    fore_ground->color_fmt = tde_osi_covert_mb_fmt(mb_src->mb_surface->mb_color_fmt);
    fore_ground->height = mb_src->mb_surface->y_height;
    fore_ground->cbcr_phy_addr = mb_src->mb_surface->cbcr_phy_addr;
    fore_ground->cbcr_stride = mb_src->mb_surface->cbcr_stride;

#if (TDE_CAPABILITY & DEFLICKER)
    opt->deflicker_mode = (mb_opt->is_deflicker) ? HI_TDE_DEFLICKER_LEVEL_MODE_BOTH :
        HI_TDE_DEFLICKER_LEVEL_MODE_NONE;
#endif

    opt->out_alpha_from = (mb_opt->is_set_out_alpha) ? HI_TDE_OUT_ALPHA_FROM_GLOBALALPHA :
        HI_TDE_OUT_ALPHA_FROM_NORM;
    opt->resize = (mb_opt->resize_en == HI_TDE_MB_RESIZE_NONE) ? HI_FALSE : HI_TRUE;
    opt->clip_rect = mb_opt->clip_rect;
    opt->global_alpha = mb_opt->out_alpha;
    opt->clip_mode = mb_opt->clip_mode;

    single_src.src_surface      = fore_ground;
    single_src.src_rect         = mb_src->src_rect;
    single_src.dst_surface      = mb_src->dst_surface;
    single_src.dst_rect         = mb_src->dst_rect;
    ret = tde_osi_single_src_2_blit(handle, &single_src, opt, HI_FALSE, HI_FALSE);
    tde_free((hi_void *)fore_ground);
    tde_free((hi_void *)opt);
    return ret;
}

#if (TDE_CAPABILITY & SLICE)
/* data_in :positive or negetive  ;  bit:complement width */
static hi_u32 tde_true_value_to_complement(hi_s32 data_in, hi_u32 bit)
{
    hi_u32 data_out;
    hi_u32 data_in_tmp;

    if (data_in >> (bit - 1)) {
        data_in_tmp = 0 - data_in;
        data_out = ((1 << (bit - 1)) | (((~data_in_tmp) & ((1 << (bit - 1)) - 1)) + 1)) & ((1 << bit) - 1);
    } else {
        data_out = data_in;
    }

    return data_out;
}

/* data_in :positive or negetive  ;  bit:complement width */
static hi_s32 tde_complement_to_true_value(hi_u32 data_in, hi_u32 bit)
{
    hi_s32 data_out;
    hi_u32 data_in_tmp;

    if (data_in >> (bit - 1)) {
        data_in_tmp = data_in & ((1 << (bit - 1)) - 1);
        data_out = (((~data_in_tmp) & ((1 << (bit - 1)) - 1)) + 1) * (-1);
    } else {
        data_out = data_in;
    }

    return data_out;
}

static hi_void tde_osi_init_slice_data(tde_slice_data *slice_data, tde_hw_node *child_node, hi_u32 i)
{
    if (i == 0) {
        slice_data->fmt = child_node->src1_ctrl.bits.src1_fmt;
        slice_data->h_scan_ord = child_node->src1_ctrl.bits.src1_h_scan_ord;
        slice_data->width = child_node->src1_imgsize.bits.src1_width + 1;

        slice_data->u32422v_pro = child_node->src1_ctrl.bits.src1_422v_pro;

        slice_data->hlmsc_en = child_node->src1_hsp.bits.hlmsc_en;
        slice_data->hchmsc_en = child_node->src1_hsp.bits.hchmsc_en;
        slice_data->hratio = child_node->src1_hsp.bits.hratio;
        slice_data->hor_loffset = child_node->src1_hloffset;
        slice_data->hor_coffset = child_node->src1_hcoffset;
        slice_data->zme_ow = child_node->src1_zmeoreso.bits.ow + 1;
        slice_data->zme_iw = child_node->src1_zmeireso.bits.iw + 1;
        slice_data->hpzme_en = child_node->src1_hpzme.bits.src1_hpzme_en;
        slice_data->hpzme_mode = child_node->src1_hpzme.bits.src1_hpzme_mode;
        slice_data->hpzme_width = child_node->src1_hpzme_size.bits.src1_hpzme_width + 1;
    } else {
        slice_data->fmt = child_node->src2_ctrl.bits.src2_fmt;
        slice_data->h_scan_ord = child_node->src2_ctrl.bits.src2_h_scan_ord;
        slice_data->width = child_node->src2_imgsize.bits.src2_width + 1;
        slice_data->u32422v_pro = child_node->src2_ctrl.bits.src2_422v_pro;
        slice_data->hlmsc_en =  child_node->src2_hsp.bits.hlmsc_en;
        slice_data->hchmsc_en = child_node->src2_hsp.bits.hchmsc_en;
        slice_data->hratio = child_node->src2_hsp.bits.hratio;
        slice_data->hor_loffset =  child_node->src2_hloffset;
        slice_data->hor_coffset =  child_node->src2_hcoffset;
        slice_data->zme_ow = child_node->src2_zmeoreso.bits.ow + 1;
        slice_data->zme_iw = child_node->src2_zmeireso.bits.iw + 1;
        slice_data->hpzme_en =  child_node->src2_hpzme.bits.src2_hpzme_en;
        slice_data->hpzme_mode = child_node->src2_hpzme.bits.src2_hpzme_mode;
        slice_data->hpzme_width = child_node->src2_hpzme_size.bits.src2_hpzme_width + 1;
    }

    return;
}

/*
 * The constants used in this function are algorithm-specific values and are not part of the devil number
 * 0xfffff 0xfff00000 is mask
 */
static hi_void tde_osi_calc_slice_s1(tde_slice_data *slice_data)
{
    slice_data->xdpos = (slice_data->hlmsc_en | slice_data->hchmsc_en) ? (slice_data->zme_ow - 1) :
        (slice_data->hpzme_en ? (slice_data->hpzme_width - 1) : (slice_data->width - 1));
    slice_data->xed_pos_blk = MIN2(slice_data->des_xed_pos_blk, slice_data->xdpos);
    /* step2 : out pos relative to s1 disp start */
    slice_data->xst_pos_cord = slice_data->des_xst_pos_blk;
    slice_data->xed_pos_cord = slice_data->xed_pos_blk;

    /* setp2 : calc s1 zme parameter */
    slice_data->xst_pos_cord_in = (slice_data->xst_pos_cord * slice_data->hratio) >> 20; /* 20 alg data */
    slice_data->xed_pos_cord_in = (slice_data->xed_pos_cord * slice_data->hratio) >> 20; /* 20 alg data */
    slice_data->hor_loffset_cfg_int_comp = slice_data->hor_loffset >> 20; /* 20 alg data */
    /* 8 alg data */
    slice_data->hor_loffset_cfg_int = tde_complement_to_true_value(slice_data->hor_loffset_cfg_int_comp, 8);
    slice_data->hor_loffset_cfg_fraction = slice_data->hor_loffset & 0xfffff;
    slice_data->hor_loffset_pix_fraction = (slice_data->xst_pos_cord * slice_data->hratio) & 0xfffff;
    slice_data->hor_loffset_fraction = (slice_data->hor_loffset_cfg_fraction +
                                        slice_data->hor_loffset_pix_fraction) & 0xfffff;

    slice_data->xst_pos_cord_in_offset = slice_data->xst_pos_cord_in + slice_data->hor_loffset_cfg_int + (
        ((slice_data->hor_loffset_cfg_fraction + slice_data->hor_loffset_pix_fraction) & 0xfff00000) != 0);
    slice_data->xed_pos_cord_in_offset = slice_data->xed_pos_cord_in + slice_data->hor_loffset_cfg_int + (
        ((slice_data->hor_loffset_cfg_fraction + slice_data->hor_loffset_pix_fraction) & 0xfff00000) != 0);
    slice_data->xst_pos_cord_in_tap_rgb = slice_data->xst_pos_cord_in_offset < 0 ? 0 : (
        (slice_data->xst_pos_cord_in_offset >= (TDE_MAX_ZOOM_OUT_STEP / 2 - 1)) ?  /* 2 alg data */
        (slice_data->xst_pos_cord_in_offset - (TDE_MAX_ZOOM_OUT_STEP / 2 - 1)) : 0); /* 2 alg data */
    slice_data->xed_pos_cord_in_tap_rgb = (slice_data->xed_pos_cord_in_offset + TDE_MAX_ZOOM_OUT_STEP / 2) < 0 ?
        0 : (((slice_data->xed_pos_cord_in_offset + TDE_MAX_ZOOM_OUT_STEP / 2) >= /* 2 alg data */
        slice_data->zme_iw - 1) ? slice_data->zme_iw - 1 : (slice_data->xed_pos_cord_in_offset +
        TDE_MAX_ZOOM_OUT_STEP / 2)); /* 2 alg data */
    slice_data->node_cfg_zme_iw_rgb = slice_data->xed_pos_cord_in_tap_rgb - slice_data->xst_pos_cord_in_tap_rgb + 1;
    slice_data->hor_loffset_int = (slice_data->xst_pos_cord_in_offset - slice_data->xst_pos_cord_in_tap_rgb);
    /* 8 alg data */
    slice_data->hor_loffset_int_complement = tde_true_value_to_complement(slice_data->hor_loffset_int, 8);
    slice_data->node_cfg_hor_loffset_rgb = (slice_data->hor_loffset_int_complement << 20) + /* 20 alg data */
                                            slice_data->hor_loffset_fraction;
    slice_data->xst_pos_cord_in_tap_luma = ((slice_data->xst_pos_cord_in_tap_rgb % 2) == 1) ? /* 2 alg data */
        slice_data->xst_pos_cord_in_tap_rgb - 1 : slice_data->xst_pos_cord_in_tap_rgb;
    slice_data->xed_pos_cord_in_tap_luma = (((slice_data->xed_pos_cord_in_tap_rgb % 2) == 0) ? /* 2 alg data */
        slice_data->xed_pos_cord_in_tap_rgb + 1 : slice_data->xed_pos_cord_in_tap_rgb) > (
        /* 2 alg data */
        slice_data->zme_iw - 1) ? (slice_data->zme_iw - 1) : (((slice_data->xed_pos_cord_in_tap_rgb % 2) == 0) ?
        slice_data->xed_pos_cord_in_tap_rgb + 1 : slice_data->xed_pos_cord_in_tap_rgb);
    slice_data->xst_pos_cord_in_chroma = (slice_data->fmt == 50) ? (slice_data->xst_pos_cord * /* 50 alg data */
        (slice_data->hratio)) >> 20 : (slice_data->xst_pos_cord * (slice_data->hratio / 2)) >> 20; /* 20 alg data */
    slice_data->xed_pos_cord_in_chroma = (slice_data->fmt == 50) ? (slice_data->xed_pos_cord * /* 50 alg data */
        (slice_data->hratio)) >> 20 : (slice_data->xed_pos_cord * (slice_data->hratio / 2)) >> 20; /* 20 alg data */
    slice_data->hor_coffset_cfg_int_comp = slice_data->hor_coffset >> 20; /* 20 alg data */
    /* 8 alg data */
    slice_data->hor_coffset_cfg_int = tde_complement_to_true_value(slice_data->hor_coffset_cfg_int_comp, 8);
    slice_data->hor_coffset_cfg_fraction = slice_data->hor_coffset & 0xfffff;
    return;
}

/*
 * The constants used in this function are algorithm-specific values and are not part of the devil number
 * 0xfffff 0xfff00000 is mask
 */
static hi_void tde_osi_calc_slice_sp(tde_slice_data *slice_data)
{
    /* 50 alg data */
    slice_data->hor_coffset_pix_fraction = ((slice_data->fmt == 50) ? (slice_data->xst_pos_cord *
        (slice_data->hratio)) : (slice_data->xst_pos_cord * (slice_data->hratio / 2))) & 0xfffff; /* 2 alg data */
    slice_data->hor_coffset_fraction = (slice_data->hor_coffset_cfg_fraction +
        slice_data->hor_coffset_pix_fraction) & 0xfffff;
    slice_data->xst_pos_cord_in_offset_chroma = slice_data->xst_pos_cord_in_chroma +
        slice_data->hor_coffset_cfg_int + (((slice_data->hor_coffset_cfg_fraction +
        slice_data->hor_coffset_pix_fraction) & 0xfff00000) != 0);
    slice_data->xed_pos_cord_in_offset_chroma = slice_data->xed_pos_cord_in_chroma +
        slice_data->hor_coffset_cfg_int + (((slice_data->hor_coffset_cfg_fraction +
        slice_data->hor_coffset_pix_fraction) & 0xfff00000) != 0);
    slice_data->xst_pos_cord_in_tap_chroma = slice_data->xst_pos_cord_in_offset_chroma < 0 ? 0 :
        ((slice_data->xst_pos_cord_in_offset_chroma >= (TDE_MAX_ZOOM_OUT_STEP / 2 - 1)) ? /* 2 alg data */
        (slice_data->xst_pos_cord_in_offset_chroma - (TDE_MAX_ZOOM_OUT_STEP / 2 - 1)) : 0); /* 2 alg data */
    /* 2 alg data */
    slice_data->xed_pos_cord_in_tap_chroma = (slice_data->xed_pos_cord_in_offset_chroma + TDE_MAX_ZOOM_OUT_STEP / 2)
        < 0 ? 0 : (((slice_data->xed_pos_cord_in_offset_chroma + TDE_MAX_ZOOM_OUT_STEP / 2) >= /* 2 alg data */
        /* 2 50 alg data */
        ((slice_data->fmt == 50) ? ((slice_data->zme_iw - 1)) : ((slice_data->zme_iw - 1) / 2))) ? ((
        /* 2 50 alg data */
        slice_data->fmt == 50) ? ((slice_data->zme_iw - 1)) : ((slice_data->zme_iw - 1) / 2)) :
        (slice_data->xed_pos_cord_in_offset_chroma + TDE_MAX_ZOOM_OUT_STEP / 2)); /* 2 alg data */
    /* 50 alg data */
    slice_data->xst_pos_cord_in_tap_chroma_x2 = (slice_data->fmt == 50) ? slice_data->xst_pos_cord_in_tap_chroma :
                                                 slice_data->xst_pos_cord_in_tap_chroma * 2; /* 2 alg data */
    /* 50 alg data */
    slice_data->xed_pos_cord_in_tap_chroma_x2 = (slice_data->fmt == 50) ? slice_data->xed_pos_cord_in_tap_chroma :
                                                 slice_data->xed_pos_cord_in_tap_chroma * 2 + 1; /* 2 alg data */

    slice_data->xst_pos_cord_in_tap_sp = MIN2(slice_data->xst_pos_cord_in_tap_luma,
                                              slice_data->xst_pos_cord_in_tap_chroma_x2);
    slice_data->xed_pos_cord_in_tap_sp = MAX2(slice_data->xed_pos_cord_in_tap_luma,
                                              slice_data->xed_pos_cord_in_tap_chroma_x2);

    slice_data->node_cfg_zme_iw_sp = slice_data->xed_pos_cord_in_tap_sp - slice_data->xst_pos_cord_in_tap_sp + 1;
    slice_data->hor_loffset_int_sp = (slice_data->xst_pos_cord_in_offset - slice_data->xst_pos_cord_in_tap_sp);
    slice_data->hor_coffset_int_sp = (slice_data->xst_pos_cord_in_offset_chroma - (
        /* 50 2 alg data */
        (slice_data->fmt == 50) ? slice_data->xst_pos_cord_in_tap_sp : slice_data->xst_pos_cord_in_tap_sp / 2));
    /* 8 alg data */
    slice_data->hor_loffset_int_sp_complent = tde_true_value_to_complement(slice_data->hor_loffset_int_sp, 8);
    /* 8 alg data */
    slice_data->hor_coffset_int_sp_complent = tde_true_value_to_complement(slice_data->hor_coffset_int_sp, 8);
    slice_data->node_cfg_hor_loffset_sp = (slice_data->hor_loffset_int_sp_complent << 20) + /* 20 alg data */
                                           slice_data->hor_loffset_fraction;
    slice_data->node_cfg_hor_coffset_sp = (slice_data->hor_coffset_int_sp_complent << 20) + /* 20 alg data */
                                           slice_data->hor_coffset_fraction;
    return;
}

/* The constants used in this function are algorithm-specific values and are not part of the devil number */
static hi_void tde_osi_calc_slice_node(tde_slice_data *slice_data, tde_hw_node *child_node, hi_u32 i)
{
    hi_bool set_slice_size = HI_FALSE;
    /* 0 2 1 alg data */
    set_slice_size = ((slice_data->hpzme_en == 0) && (slice_data->slice_width % 2 == 1) &&
        ((slice_data->fmt == 48) || ((slice_data->fmt == 50) && !slice_data->u32422v_pro) || /* 48 50 alg data */
        (slice_data->fmt == 52)) && (slice_data->xed_pos_cord_in_tap_hpzme_hso == /* 52 alg data */
        slice_data->width - 1) && (slice_data->width % 2 == 0)); /* 52 alg data */
    if (set_slice_size) {
        if (slice_data->h_scan_ord) {
            slice_data->slice_wi = slice_data->slice_wi + 1;
            slice_data->slice_width = slice_data->slice_width + 1;
            slice_data->slice_hoffset = slice_data->slice_hoffset - 1;
        } else {
            slice_data->slice_wi = slice_data->slice_wi + 1;
            slice_data->slice_width = slice_data->slice_width + 1;
            slice_data->slice_hoffset = slice_data->slice_hoffset - 1;
            slice_data->slice_l_ofst = slice_data->slice_l_ofst + (
            slice_data->hor_loffset_int_beyond_complent << 20); /* 20 alg data */
            slice_data->slice_c_ofst = slice_data->slice_c_ofst + (
            slice_data->hor_loffset_int_beyond_complent << 20); /* 20 alg data */
        }
    }

    if (i == 0) {
        child_node->src1_zmeoreso.bits.ow = slice_data->slice_wo;
        child_node->src1_hloffset = slice_data->slice_l_ofst;
        child_node->src1_hcoffset = slice_data->slice_c_ofst;
        child_node->src1_pix_offset.bits.src1_hoffset_pix = slice_data->slice_hoffset;
        child_node->src1_imgsize.bits.src1_width = slice_data->slice_width - 1;
        child_node->src1_zmeireso.bits.iw = slice_data->slice_wi;
        child_node->src1_hpzme_size.bits.src1_hpzme_width = slice_data->slice_w_hpzme - 1;

        return;
    }

    child_node->src2_zmeoreso.bits.ow = slice_data->slice_wo;
    child_node->src2_hloffset = slice_data->slice_l_ofst;
    child_node->src2_hcoffset = slice_data->slice_c_ofst;
    child_node->src2_pix_offset.bits.src2_hoffset_pix = slice_data->slice_hoffset;
    child_node->src2_imgsize.bits.src2_width = slice_data->slice_width - 1;
    child_node->src2_zmeireso.bits.iw = slice_data->slice_wi;
    child_node->src2_hpzme_size.bits.src2_hpzme_width = slice_data->slice_w_hpzme - 1;
}

/* The constants used in this function are algorithm-specific values and are not part of the devil number */
static hi_void tde_osi_calc_slice_mux(tde_slice_data *slice_data, tde_hw_node *child_node, hi_u32 i)
{
    hi_bool cfg_zme_iw_sp = HI_FALSE;
    hi_bool cfg_hor_loffset_sp = HI_FALSE;
    hi_bool cfg_hor_coffset_sp = HI_FALSE;

    slice_data->slice_wo = slice_data->xed_pos_cord - slice_data->xst_pos_cord + 1;
    /* 32 48 50 alg data */
    cfg_zme_iw_sp = ((slice_data->fmt >= 32) && (slice_data->fmt != 48) && (!((slice_data->fmt == 50) &&
                     !slice_data->u32422v_pro)) && (slice_data->fmt != 52)); /* 52 alg data */
    slice_data->slice_wi = (cfg_zme_iw_sp) ? slice_data->node_cfg_zme_iw_sp : ((slice_data->hlmsc_en ||
                            slice_data->hchmsc_en) ? slice_data->node_cfg_zme_iw_rgb : slice_data->slice_wo);
    /* 32 48 50 alg data */
    cfg_hor_loffset_sp = ((slice_data->fmt >= 32) && (slice_data->fmt != 48) && (!((slice_data->fmt == 50) &&
                          !slice_data->u32422v_pro)) && (slice_data->fmt != 52)); /* 52 alg data */
    slice_data->slice_l_ofst = (cfg_hor_loffset_sp) ? slice_data->node_cfg_hor_loffset_sp :
                                slice_data->node_cfg_hor_loffset_rgb;

    cfg_hor_coffset_sp = cfg_hor_loffset_sp;
    slice_data->slice_c_ofst = (cfg_hor_coffset_sp) ? slice_data->node_cfg_hor_coffset_sp :
                                slice_data->node_cfg_hor_loffset_rgb;

    slice_data->slice_w_hpzme = slice_data->slice_wi;
    slice_data->slice_wo = slice_data->slice_wo - 1;
    slice_data->slice_wi = slice_data->slice_wi - 1;

    slice_data->xst_pos_cord_in_tap = (slice_data->hlmsc_en || slice_data->hchmsc_en) ?
        /* 32 48 50 alg data */
        (((slice_data->fmt >= 32) && (slice_data->fmt != 48) && (!((slice_data->fmt == 50) &&
        /* 52 alg data */
        !slice_data->u32422v_pro)) && (slice_data->fmt != 52)) ? slice_data->xst_pos_cord_in_tap_sp :
        slice_data->xst_pos_cord_in_tap_rgb) : slice_data->xst_pos_cord;

    slice_data->xed_pos_cord_in_tap = (slice_data->hlmsc_en || slice_data->hchmsc_en) ?
        /* 32 48 50 alg data */
        (((slice_data->fmt >= 32) && (slice_data->fmt != 48) && (!((slice_data->fmt == 50) &&
        /* 52 alg data */
        !slice_data->u32422v_pro)) && (slice_data->fmt != 52)) ? slice_data->xed_pos_cord_in_tap_sp :
        slice_data->xed_pos_cord_in_tap_rgb) : slice_data->xed_pos_cord;

    slice_data->xst_pos_cord_in_tap_hpzme = slice_data->hpzme_en ? (slice_data->xst_pos_cord_in_tap *
       (slice_data->hpzme_mode + 1)) : slice_data->xst_pos_cord_in_tap;
    slice_data->xed_pos_cord_in_tap_hpzme = slice_data->hpzme_en ? ((slice_data->xed_pos_cord_in_tap + 1) *
       (slice_data->hpzme_mode + 1) - 1) : slice_data->xed_pos_cord_in_tap;

    if (slice_data->h_scan_ord) {
        slice_data->xst_pos_cord_in_tap_hpzme_hso = slice_data->width - 1 - slice_data->xed_pos_cord_in_tap_hpzme;
        slice_data->xed_pos_cord_in_tap_hpzme_hso = slice_data->width - 1 - slice_data->xst_pos_cord_in_tap_hpzme;
    } else {
        slice_data->xst_pos_cord_in_tap_hpzme_hso = slice_data->xst_pos_cord_in_tap_hpzme;
        slice_data->xed_pos_cord_in_tap_hpzme_hso = slice_data->xed_pos_cord_in_tap_hpzme;
    }

    slice_data->slice_width = (slice_data->xed_pos_cord_in_tap_hpzme_hso -
                               slice_data->xst_pos_cord_in_tap_hpzme_hso + 1);
    slice_data->slice_hoffset = slice_data->xst_pos_cord_in_tap_hpzme_hso;

    slice_data->hor_loffset_int_beyond = 1;
    /* 8 alg data */
    slice_data->hor_loffset_int_beyond_complent = tde_true_value_to_complement(slice_data->hor_loffset_int_beyond, 8);
}

static hi_void tde_osi_finalize_slice_calculation(tde_slice_data *slice_data, tde_hw_node *child_node)
{
    slice_data->slice_dst_width = slice_data->des_xed_pos_blk - slice_data->des_xst_pos_blk + 1;
    slice_data->slice_dst_hoffset = (slice_data->des_h_scan_ord ? (slice_data->des_width - 1 -
        slice_data->des_xed_pos_blk) : slice_data->des_xst_pos_blk) + slice_data->des_hoffset_pix;
    slice_data->des_crop_en = (!((slice_data->des_crop_start_x > slice_data->des_xed_pos_blk) ||
        (slice_data->des_crop_end_x < slice_data->des_xst_pos_blk))) && slice_data->des_crop_en;
    slice_data->des_crop_start_x = slice_data->des_crop_en ? (MAX2(slice_data->des_xst_pos_blk,
        slice_data->des_crop_start_x) - slice_data->des_xst_pos_blk) : 0;
    slice_data->des_crop_end_x = slice_data->des_crop_en ? (MIN2(slice_data->des_xed_pos_blk,
        slice_data->des_crop_end_x) - slice_data->des_xst_pos_blk) : 0;

    child_node->des_imgsize.bits.des_width = slice_data->slice_dst_width - 1;
    child_node->des_pix_offset.bits.des_hoffset_pix = slice_data->slice_dst_hoffset;
    child_node->des_alpha.bits.des_crop_en = slice_data->des_crop_en;
    child_node->des_crop_pos_st.bits.des_crop_start_x = slice_data->des_crop_start_x;
    child_node->des_crop_pos_ed.bits.des_crop_end_x = slice_data->des_crop_end_x;

    return;
}

hi_s32 tde_osi_calc_slice(hi_s32 handle, tde_hw_node *node)
{
    tde_slice_data slice_data = { 0 };
    hi_u32 n;
    hi_u32 i;
    hi_s32 ret;

    slice_data.des_width = node->des_imgsize.bits.des_width + 1;
    slice_data.des_hoffset_pix = 0;
    slice_data.des_h_scan_ord = node->des_ctrl.bits.des_h_scan_ord;

    slice_data.des_crop_en = node->des_alpha.bits.des_crop_en;
    slice_data.des_crop_start_x = node->des_crop_pos_st.bits.des_crop_start_x;
    slice_data.des_crop_end_x = node->des_crop_pos_ed.bits.des_crop_end_x;
    /* 256 alg data */
    slice_data.node_num = slice_data.des_width / 256 + ((slice_data.des_width % 256) != 0);

    for (n = 0; n < slice_data.node_num; n++) {
        tde_hw_node *child_node;
        hi_void *buf = HI_NULL;
        buf = (hi_void *)tde_malloc(sizeof(tde_hw_node) + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE);
        if (buf == HI_NULL) {
            tde_error("malloc (%d) failed, wgetfreenum(%d)!\n", (sizeof(tde_hw_node)), wgetfreenum());
            return HI_ERR_TDE_NO_MEM;
        }

        child_node = (tde_hw_node *)(buf + TDE_NODE_HEAD_BYTE);

        if (memcpy_s(child_node, sizeof(tde_hw_node), node, sizeof(tde_hw_node)) != EOK) {
            tde_hal_free_node_buf(child_node);
            return HI_ERR_TDE_INVALID_PARA;
        }

        slice_data.des_xst_pos_blk = n * TDE_MAX_SLICE_WIDTH;
        slice_data.des_xed_pos_blk = (((n + 1) * TDE_MAX_SLICE_WIDTH) <= slice_data.des_width) ? (
            ((n + 1) * TDE_MAX_SLICE_WIDTH) - 1) : slice_data.des_width - 1;

        for (i = 0; i < 2; i++) { /* 2 alg data */
            tde_osi_init_slice_data(&slice_data, child_node, i);

            tde_osi_calc_slice_s1(&slice_data);

            tde_osi_calc_slice_sp(&slice_data);

            tde_osi_calc_slice_mux(&slice_data, child_node, i);
        }

        tde_osi_finalize_slice_calculation(&slice_data, child_node);
        ret = tde_osi_set_node_finish(handle, child_node, 0, TDE_NODE_SUBM_ALONE);
        if (ret != HI_SUCCESS) {
            tde_hal_free_node_buf(child_node);
            return ret;
        }
    }
    return HI_SUCCESS;
}

#endif

#ifndef HI_BUILD_IN_BOOT /* codecc */

#if (TDE_CAPABILITY & RESIZE)
static hi_void tde_osi_set_value_to_clear(hi_s32 *zme_vin, hi_s32 *zme_vout, hi_s32 *zme_vphase)
{
    *zme_vin = 0;
    *zme_vout = 0;
    *zme_vphase = 0;
}
static hi_void tde_osi_get_info_zme(tde_update_config *reg, tde_update_info *info)
{
    info->zme_instart_w = reg->update_instart_w;
    info->zme_outstart_w = reg->update_instart_w;
    info->zme_in_width = reg->update_in_width;
    info->zme_out_width = reg->update_in_width;
    info->zme_hphase = 0;
}

/* The constants used in this function are algorithm-specific values and are not part of the devil number */
static hi_void tde_osi_get_hupdate_info(tde_update_config *reg, tde_update_info *info, hi_bool scaler)
{
    hi_s32 zme_hinstart, zme_hinstop, zme_houtstart, zme_houtstop, zme_hphase, ratio, dratio;
    hi_s32 update_hstart = reg->update_instart_w;
    hi_s32 update_hstop = update_hstart + reg->update_in_width - 1;

    if (((reg->zme_out_width - 1) == 0) || ((reg->ori_in_width - 1) == 0)) {
        return;
    }

    /* 4096 2 alg data */
    ratio = (hi_s32)(4096 * (reg->ori_in_width - 1) / (reg->zme_out_width - 1) + 1 / 2);
    /* 4096 alg data */
    dratio = 4096 * (reg->zme_out_width - 1) / (reg->ori_in_width - 1);

    if (scaler != HI_TRUE) { /* hor_scaler not enable */
        tde_osi_get_info_zme(reg, info);
        return;
    }

    if ((update_hstart >= 0) && (update_hstart < 3)) { /* 3 alg data */
        /* update outstretched area exceed left limit bordline */
        tde_osi_set_value_to_clear(&zme_hinstart, &zme_houtstart, &zme_hphase);
    } else {
        /* update outstretched area didn't exceed the left limit bordline */
        zme_hinstart = (update_hstart - 3) * dratio; /* 3 alg data */
        /* 12 4096 alg data */
        zme_houtstart = (zme_hinstart % 4096) == 0 ? ((hi_u32)zme_hinstart >> 12) : (((hi_u32)zme_hinstart >> 12) + 1);
        zme_hinstart = (hi_u32)(zme_houtstart * ratio) >> 12; /* 12 alg data */
        if (zme_hinstart - 2 < 0) { /* 2 alg data */
            /* 4096 the left few point need mirror pixels when scale */
            zme_hphase = (zme_houtstart * ratio) % 4096 + zme_hinstart * 4096;
            zme_hinstart = 0;
        } else {
            /* 4096 3 the left few point not need mirror pixels when scale */
            zme_hphase = (zme_houtstart * ratio) % 4096 + 3 * 4096;
            zme_hinstart = zme_hinstart - 2; /* 2 alg data */
        }
    }

    if ((update_hstop > (reg->ori_in_width - 3)) && (update_hstop < reg->ori_in_width)) { /* 3 alg data */
        /* update outstretched area exceed the right limit bordline */
        zme_hinstop = reg->ori_in_width - 1;
        zme_houtstop = reg->zme_out_width - 1;
    } else {
        /* 2 1 alg data, update outstretched area didn't exceed the right limit bordline */
        zme_hinstop = (update_hstop + 2 + 1) * dratio;
        /* 12 4096 alg data */
        zme_houtstop = (zme_hinstop % 4096) == 0 ? (((hi_u32)zme_hinstop >> 12) - 1) : ((hi_u32)zme_hinstop >> 12);
        zme_hinstop = (hi_u32)(zme_houtstop * ratio) >> 12;  /* 12 alg data */
        if (zme_hinstop + 3 > (reg->ori_in_width - 1)) { /* 3 alg data */
            /* the right few point need mirror pixels when scale */
            zme_hinstop = reg->ori_in_width - 1;
        } else {
            /* the right few point need mirror pixels when scale */
            zme_hinstop = zme_hinstop + 3; /* 3 alg data */
        }
    }

    info->zme_instart_w = zme_hinstart;
    info->zme_outstart_w = zme_houtstart;
    info->zme_in_width = zme_hinstop - zme_hinstart + 1;
    info->zme_out_width = zme_houtstop - zme_houtstart + 1;
    info->zme_hphase = zme_hphase;
}

static hi_void tde_osi_get_info_to_reg(tde_update_config *reg, tde_update_info *info)
{
    info->zme_instart_h = reg->update_instart_h;
    info->zme_in_height = reg->update_in_height;
    info->zme_outstart_h = reg->update_instart_h;
    info->zme_out_height = reg->update_in_height;
    info->zme_vphase = 0;
    info->def_offsetup = 0;
    info->def_offsetdown = 0;
}

static hi_void tde_osi_get_vupdate_info(tde_update_config *reg, tde_update_info *info, int scaler, int is_deflicker)
{
    hi_s32 zme_vinstart, zme_vinstop, zme_voutstart, zme_voutstop, zme_vphase, ratio, dratio;
    hi_s32 update_vstart = reg->update_instart_h;
    hi_s32 update_vstop = update_vstart + reg->update_in_height - 1;
    hi_unused(is_deflicker);

    if ((reg->zme_out_height == 0) || (reg->ori_in_height == 0)) {
        return;
    }

    ratio = (hi_s32)(4096 * (reg->ori_in_height) / (reg->zme_out_height)); /* 4096 alg data */
    dratio = 4096 * (reg->zme_out_height) / (reg->ori_in_height); /* 4096 alg data */

    if (scaler != HI_TRUE) {
        tde_osi_get_info_to_reg(reg, info);
        return;
    }

    /* ver_scale enable & deflicker not enable */
    if ((update_vstart >= 0) && (update_vstart < 2)) { /* 2 Vstart */
        /* the update outstreatched area exceed the up limit bordline */
        tde_osi_set_value_to_clear(&zme_vinstart, &zme_voutstart, &zme_vphase);
    } else {
        /* the update outstreatched area didn't exceed the up limit bordline */
        zme_vinstart = (update_vstart - 2) * dratio; /* 2 alg data */
        /* 4096 12 alg data */
        zme_voutstart = (zme_vinstart % 4096) == 0 ? ((hi_u32)zme_vinstart >> 12) : (((hi_u32)zme_vinstart >> 12) + 1);
        zme_vinstart = (hi_u32)(zme_voutstart * ratio) >> 12; /* 12 alg data */
        if (zme_vinstart - 1 < 0) {
            /* the up few point need mirror pixels when scale */
            zme_vphase = (zme_voutstart * ratio) % 4096 + zme_vinstart * 4096; /* 4096 alg data */
            zme_vinstart = 0;
        } else {
            /* the up few point not need mirror pixels when scale */
            zme_vphase = (zme_voutstart * ratio) % 4096 + 1 * 4096; /* 4096 alg data */
            zme_vinstart = zme_vinstart - 1;
        }
    }
    /* 2 alg data */
    if ((update_vstop > (reg->ori_in_height - 2)) && (update_vstop < reg->ori_in_height)) {
        /* the update outstreatched area exceed the down limit bordline */
        zme_vinstop = reg->ori_in_height - 1;
        zme_voutstop = reg->zme_out_height - 1;
    } else {
        /* the update outstreatched area didn't exceed the down limit bordline */
        zme_vinstop = (update_vstop + 1 + 1) * dratio;
        /* 4096 12 alg data */
        zme_voutstop = (zme_vinstop % 4096) == 0 ? (((hi_u32)zme_vinstop >> 12) - 1) : ((hi_u32)zme_vinstop >> 12);
        zme_vinstop = (hi_u32)(zme_voutstop * ratio) >> 12; /* 12 alg data */
        if (zme_vinstop + 2 > (reg->ori_in_height - 1)) { /* 2 alg data */
            /* the down few point need mirror pixels when scale */
            zme_vinstop = reg->ori_in_height - 1;
        } else {
            /* the down few point not need mirror pixels when scale */
            zme_vinstop = zme_vinstop + 2; /* 2 alg data */
        }
    }

    info->zme_in_height = zme_vinstop - zme_vinstart + 1;
    info->zme_instart_h = zme_vinstart;
    info->zme_outstart_h = zme_voutstart;
    info->zme_out_height = zme_voutstop - zme_voutstart + 1;
    info->zme_vphase = zme_vphase;
    info->def_offsetup = 0;
    info->def_offsetdown = 0;
}
#endif
#endif

#ifndef HI_BUILD_IN_BOOT
static hi_void tde_osi_src2_filter_reg(hi_tde_double_src *double_src, tde_update_config *reg)
{
    reg->ori_in_height = double_src->fg_surface->height;
    reg->ori_in_width = double_src->fg_surface->width;
    reg->zme_out_height = double_src->dst_surface->height;
    reg->zme_out_width = double_src->dst_surface->width;
    reg->update_instart_w = double_src->fg_rect->pos_x;
    reg->update_instart_h = double_src->fg_rect->pos_y;
    reg->update_in_width = double_src->fg_rect->width;
    reg->update_in_height = double_src->fg_rect->height;
}

static hi_void tde_osi_src2_filter_rect_opt(tde_rect_opt *rect_opt, tde_update_info *info)
{
    rect_opt->in_rect->pos_x = info->zme_instart_w;
    rect_opt->in_rect->pos_y = info->zme_instart_h;
    rect_opt->in_rect->width = info->zme_in_width;
    rect_opt->in_rect->height = info->zme_in_height;

    rect_opt->out_rect->pos_x = info->zme_outstart_w;
    rect_opt->out_rect->pos_y = info->zme_outstart_h;
    rect_opt->out_rect->width = info->zme_out_width;
    rect_opt->out_rect->height = info->zme_out_height;
}

static hi_s32 tde_osi_src2_filter_opt(tde_hw_node *node, hi_tde_double_src *double_src, tde_rect_opt *rect_opt,
    hi_tde_deflicker_mode deflicker_mode, hi_tde_deflicker_mode filter_mode)
{
    hi_s32 ret;
    tde_update_config reg = { 0 };
    tde_update_info info = { 0 };
    hi_bool is_deflicker = HI_FALSE;
    hi_bool scale = HI_FALSE;

#if (TDE_CAPABILITY & DEFLICKER)
    is_deflicker = (deflicker_mode == HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ? HI_FALSE : HI_TRUE;
#else
    hi_unused(deflicker_mode);
#endif
    tde_osi_src2_filter_reg(double_src, &reg);
    if (node->src2_hsp.bits.hratio != TDE_NO_SCALE_HSTEP) {
        scale = HI_TRUE;
    }

    tde_info("bHScale:%x\n", scale);
    tde_osi_get_hupdate_info(&reg, &info, scale);

    scale = HI_FALSE;
    if (node->src2_vsr.bits.vratio != TDE_NO_SCALE_VSTEP) {
        scale = HI_TRUE;
    }
    tde_info("bVScale:%x\n", scale);
    tde_osi_get_vupdate_info(&reg, &info, scale, is_deflicker);

    node->src2_imgsize.bits.src2_width = info.zme_in_width - 1;
    node->src2_zmeireso.bits.iw = info.zme_in_width - 1;

    node->src2_imgsize.bits.src2_height = info.zme_in_height - 1;
    node->src2_zmeireso.bits.ih = info.zme_in_height - 1;

    double_src->dst_rect->pos_x = info.zme_outstart_w;
    double_src->dst_rect->pos_y = info.zme_outstart_h;
    double_src->dst_rect->width = info.zme_out_width;
    double_src->dst_rect->height = info.zme_out_height;

    node->src2_hcoffset = info.zme_hphase;
    node->src2_voffset.bits.vluma_offset = info.zme_vphase;

    tde_osi_src2_filter_rect_opt(rect_opt, &info);
#if ((HICHIP == HI3516C_V500) || (HICHIP == HI3519A_V100))
    if ((rect_opt->in_rect->width > MAX_LINE_BUFFER) && (rect_opt->out_rect->width > MAX_LINE_BUFFER)) {
    tde_error("it does not support local resize in width %d && out width %d over max line buffer %d!! \n",
              rect_opt->in_rect->width, rect_opt->out_rect->width, MAX_LINE_BUFFER);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
#endif
    ret = tde_hal_calc_src2_filter_opt(node, rect_opt, is_deflicker, filter_mode);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_filter_reg(tde_hw_node *node, hi_tde_double_src *double_src,
    hi_tde_deflicker_mode deflicker_mode, hi_tde_deflicker_mode filter_mode)
{
    hi_s32 ret;
    tde_rect_opt rect_opt;
    hi_tde_rect in_rect, out_rect;
    hi_u64 src2_addr, dst_addr;
    hi_tde_color_fmt out_fmt = double_src->dst_surface->color_fmt;
    hi_bool back_ground_operation = ((double_src->bg_surface != HI_NULL) && (double_src->bg_rect != HI_NULL));

    rect_opt.in_fmt = double_src->fg_surface->color_fmt;
    rect_opt.in_rect = &in_rect;
    rect_opt.out_rect = &out_rect;
    if (((double_src->fg_surface->width != double_src->fg_rect->width) ||
         (double_src->fg_surface->height != double_src->fg_rect->height)) && g_region_deflicker) {
        ret = tde_osi_src2_filter_opt(node, double_src, &rect_opt, deflicker_mode, filter_mode);
        if (ret != HI_SUCCESS) {
            return ret;
        }
        if (double_src->dst_surface->width <= 1) {
            node->src2_hsp.bits.hratio = 0;
        } else {
            node->src2_hsp.bits.hratio = osal_div_u64(
                ((hi_u64)(double_src->fg_surface->width) << TDE_HAL_HSTEP_FLOATLEN),
                (double_src->dst_surface->width));
        }

        if (double_src->dst_surface->height <= 1) {
            node->src2_vsr.bits.vratio = 0;
        } else {
            node->src2_vsr.bits.vratio = osal_div_u64(((hi_u64)(double_src->fg_surface->height) << TDE_FLOAT_BITLEN),
                                                      (double_src->dst_surface->height));
        }
        src2_addr = double_src->fg_surface->phy_addr;
        dst_addr = double_src->dst_surface->phy_addr;
        src2_addr += rect_opt.in_rect->pos_y * node->src2_ch0_stride.bits.src2_ch0_stride  +
            ((hi_u32)(rect_opt.in_rect->pos_x * tde_osi_get_bpp_by_fmt(rect_opt.in_fmt)) >> 3); /* 2^3 pos_x data */
        dst_addr += rect_opt.out_rect->pos_y * node->des_ch0_stride.bits.des_ch0_stride +
            ((hi_u32)(rect_opt.out_rect->pos_x * tde_osi_get_bpp_by_fmt(out_fmt)) >> 3); /* 2^3 pos_x data */

        node->src2_ch0_addr_low.bits.src2_ch0_addr_low = GetLowAddr(src2_addr);
        node->src2_ch0_addr_high.bits.src2_ch0_addr_high = GetHighAddr(src2_addr);

        node->des_ch0_addr_low.bits.des_ch0_addr_low = GetLowAddr(dst_addr);
        node->des_ch0_addr_high.bits.des_ch0_addr_high = GetHighAddr(dst_addr);
        if (back_ground_operation) {
            node->src1_imgsize.bits.src1_width = node->des_imgsize.bits.des_width;
            node->src1_imgsize.bits.src1_height = node->des_imgsize.bits.des_height;
        }
    }
    return HI_SUCCESS;
}
#endif

static hi_s32 tde_osi_filter_opera(tde_hw_node *node, hi_tde_double_src *double_src,
    hi_tde_deflicker_mode deflicker_mode, hi_tde_deflicker_mode filter_mode)
{
    hi_s32 ret;
    hi_bool is_deflicker = HI_FALSE;
    tde_rect_opt rect_opt;
    hi_bool fore_ground_operation = ((double_src->fg_surface != HI_NULL) && (double_src->fg_rect != HI_NULL));

#if (TDE_CAPABILITY & DEFLICKER)
    is_deflicker = (deflicker_mode == HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ? HI_FALSE : HI_TRUE;
#endif
    if (fore_ground_operation) {
#if ((HICHIP == HI3516C_V500) || (HICHIP == HI3519A_V100))
        if ((double_src->fg_rect->width > MAX_LINE_BUFFER) && (double_src->dst_rect->width > MAX_LINE_BUFFER)) {
            tde_error("it does not support resize in width %d && out width %d over max line buffer %d!! \n",
                      double_src->fg_rect->width, double_src->dst_rect->width, MAX_LINE_BUFFER);
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
#endif
        rect_opt.in_fmt = double_src->fg_surface->color_fmt;
        rect_opt.in_rect = double_src->fg_rect;
        rect_opt.out_rect = double_src->dst_rect;
        ret = tde_hal_calc_src2_filter_opt(node, &rect_opt, is_deflicker, filter_mode);
        if (ret != HI_SUCCESS) {
            return ret;
        }
#ifndef HI_BUILD_IN_BOOT
        ret = tde_osi_filter_reg(node, double_src, deflicker_mode, filter_mode);
        if (ret != HI_SUCCESS) {
            return ret;
        }
#endif
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_set_filter_node(hi_s32 handle, tde_hw_node *node, hi_tde_double_src *double_src,
    hi_tde_deflicker_mode deflicker_mode, hi_tde_deflicker_mode filter_mode)
{
    hi_s32 ret;
    hi_bool back_ground_operation = ((double_src->bg_surface != HI_NULL) && (double_src->bg_rect != HI_NULL));

    if (back_ground_operation == HI_TRUE && g_region_deflicker == HI_FALSE) {
#if ((HICHIP == HI3516C_V500) || (HICHIP == HI3519A_V100))
        if ((double_src->bg_rect->width != double_src->dst_rect->width) ||
            (double_src->bg_rect->height != double_src->dst_rect->height)) {
            tde_error("it does not support  src1 resize!! \n");
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
#else
        tde_hal_calc_src1_filter_opt(node, double_src->bg_surface->color_fmt, double_src->bg_rect,
            double_src->dst_rect, filter_mode);
#endif
    }
    ret = tde_osi_filter_opera(node, double_src, deflicker_mode, filter_mode);
    if (ret != HI_SUCCESS) {
        return ret;
    }
#if (TDE_CAPABILITY & SLICE)
    return tde_osi_calc_slice(handle, node);
#else
    ret = tde_osi_set_node_finish(handle, node, 0, TDE_NODE_SUBM_ALONE);
    if (ret < 0) {
        return ret;
    }
    return HI_SUCCESS;
#endif
}

/*
 * Function:      tde_osi_adj_para4_ycb_cr422_r
 * Description:   when fill color is YCbCr422R, fill by word
 * Input:         dst_surface: target bitmap info struct
 *                dst_rect: target operate zone
 *                fill_color: fill color
 * Others:        add software fill, when YCbCr422, fill function by word
 */
static hi_void tde_osi_adj_para4_ycb_cr422_r(hi_tde_surface *dst_surface, hi_tde_rect *dst_rect,
                                             hi_tde_fill_color *fill_color)
{
    if ((dst_surface->color_fmt != HI_TDE_COLOR_FMT_YCBCR422) &&
        (dst_surface->color_fmt != HI_TDE_COLOR_FMT_PKGVYUY)) {
        return;
    }

    dst_surface->color_fmt = HI_TDE_COLOR_FMT_AYCBCR8888;
    dst_surface->alpha_max_is_255 = HI_TRUE;
    fill_color->color_fmt = HI_TDE_COLOR_FMT_AYCBCR8888;

    dst_surface->width /= 2; /* 2 width */

    dst_rect->width /= 2; /* 2 width */
    dst_rect->pos_x /= 2; /* 2 pos_x */

    fill_color->color_value = tde_get_yc422r_fillvalue(fill_color->color_value);
}

static hi_s32 tde_osi_check_fill_opt(hi_tde_opt *opt)
{
    hi_bool real = (((opt->blend_opt.global_alpha_en != HI_TRUE) && (opt->blend_opt.global_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.pixel_alpha_en != HI_TRUE) && (opt->blend_opt.pixel_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.src1_alpha_premulti != HI_TRUE) && (opt->blend_opt.src1_alpha_premulti != HI_FALSE)) ||
        ((opt->blend_opt.src2_alpha_premulti != HI_TRUE) && (opt->blend_opt.src2_alpha_premulti != HI_FALSE)) ||
        ((opt->clut_reload != HI_TRUE) && (opt->clut_reload != HI_FALSE)) ||
        ((opt->is_compress != HI_TRUE) && (opt->is_compress != HI_FALSE)) ||
        ((opt->is_decompress != HI_TRUE) && (opt->is_decompress != HI_FALSE)));

    if ((opt->color_key_mode >= HI_TDE_COLOR_KEY_MODE_MAX) ||
        (opt->color_key_mode < HI_TDE_COLOR_KEY_MODE_NONE)) {
        tde_error("color_key_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (real) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->deflicker_mode >= HI_TDE_DEFLICKER_LEVEL_MODE_MAX) ||
        (opt->deflicker_mode < HI_TDE_DEFLICKER_LEVEL_MODE_NONE)) {
        tde_error("deflicker_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->filter_mode >= HI_TDE_FILTER_MODE_MAX) || (opt->filter_mode < HI_TDE_FILTER_MODE_COLOR)) {
        tde_error("filter_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->mirror >= HI_TDE_MIRROR_MAX) || (opt->mirror < HI_TDE_MIRROR_NONE)) {
        tde_error("mirror error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->alpha_blending_cmd >= HI_TDE_ALPHA_BLENDING_MAX ||
        opt->alpha_blending_cmd < HI_TDE_ALPHA_BLENDING_NONE) {
        tde_error("alpha_blending_cmd error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE) {
        tde_error("invalid alu command!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->resize) {
        tde_error("Not support resize in single source fill operation!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_1_source_src1(hi_tde_surface *dst_surface, hi_tde_opt *opt, tde_hw_node *hw_node,
                                    hi_tde_rect *dst_rect)
{
    hi_s32 ret;
    hi_tde_double_src double_src;
    ret = tde_osi_check_fill_opt(opt);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }
    if ((opt->out_alpha_from >= HI_TDE_OUT_ALPHA_FROM_MAX) ||
        (opt->out_alpha_from == HI_TDE_OUT_ALPHA_FROM_FOREGROUND)) {
        tde_error("out_alpha_from error!\n");
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }

    tde_hal_node_set_global_alpha(hw_node, opt->global_alpha, opt->blend_opt.global_alpha_en);

    double_src.bg_surface = HI_NULL;
    double_src.bg_rect = HI_NULL;
    double_src.fg_surface = dst_surface;
    double_src.fg_rect = dst_rect;
    double_src.dst_surface = dst_surface;
    double_src.dst_rect = dst_rect;
    if (tde_osi_set_clip_para(&double_src, opt, hw_node) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_CLIP_AREA;
    }
    return HI_SUCCESS;
}
#endif

static hi_s32 tde_osi_1_source_check(hi_tde_surface *dst_surface, hi_tde_rect *dst_rect,
                                     hi_tde_fill_color *fill_color, hi_tde_rect *rect)
{
    hi_bool null_ptr = ((dst_surface == HI_NULL) || (dst_rect == HI_NULL) || (fill_color == HI_NULL));
    hi_s32 bpp;

    if (null_ptr) {
        tde_error("HI_NULL pointer!\n");
        return HI_ERR_TDE_NULL_PTR;
    }
    if (memcpy_s(rect, sizeof(hi_tde_rect), dst_rect, sizeof(hi_tde_rect)) != EOK) {
        tde_error("secure function failure\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((dst_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
        (fill_color->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP)) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

#ifdef TDE_BACKGROUND_COLORFOMATSUPPORT_YCBCR422
    if (dst_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422) {
        tde_error("Background  doesn't support the colorfmt!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#endif

    if (tde_osi_check_surface(dst_surface, rect) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    bpp = tde_osi_get_bpp_by_fmt(dst_surface->color_fmt);
    if (bpp < 0) {
        tde_error("Unknown color format!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (bpp < 8) { /* 8 is bpp */
        /* when writing, 8 subbyte format align ask start point byte align */
        if ((rect->pos_x % 8) || (rect->width * bpp % 8)) {
            tde_error("Subbyte's start or width for writing is not aligned!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    tde_osi_adj_para4_ycb_cr422_r(dst_surface, rect, fill_color);
    return HI_SUCCESS;
}

static hi_void tde_osi_1_source_drv(hi_tde_surface *dst_surface, tde_base_opt_mode base_mode, hi_tde_rect *dst_rect,
                                    tde_hw_node *hw_node, hi_tde_out_alpha_from out_alpha_from)
{
    tde_scandirection_mode scan_info = { 0 };
    tde_surface_msg drv_surface = { 0 };
    scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    scan_info.v_scan = TDE_SCAN_UP_DOWN;
    tde_osi_convert_surface(dst_surface, dst_rect, &scan_info, &drv_surface);

    if (base_mode == TDE_NORM_FILL_1OPT) {
        tde_hal_node_set_src1(hw_node, &drv_surface);
    }

    tde_hal_node_set_tqt(hw_node, &drv_surface, out_alpha_from);

    tde_osi_set_ext_alpha(dst_surface, HI_NULL, hw_node);
    return;
}

/*
 * Function:      tde_osi_1_source_fill
 * Description:   single source fill operate,source1 is fill color,target bitmap is dst_surface,
                  support source1 and fill color do ROP or alpha blending to target bitmap,
                  unsupported mirror,colorkey
 *                src bitmap is not support MB color format
 * Input:         handle: task handle
 *                dst_surface: foreground bitmap info struct
 *                fill_color:  target bitmap info struct
 *                opt: operate parameter setting struct
 * Return:        HI_SUCCESS/HI_FAILURE
 */
static hi_s32 tde_osi_1_source_fill(hi_s32 handle, hi_tde_surface *dst_surface, hi_tde_rect *dst_rect,
                                    hi_tde_fill_color *fill_color, hi_tde_opt *opt)
{
    tde_base_opt_mode base_mode = { 0 };
    tde_hw_node *hw_node = HI_NULL;
    hi_tde_out_alpha_from out_alpha_from = HI_TDE_OUT_ALPHA_FROM_NORM;
    hi_tde_rect rect = { 0 };
    tde_color_fill drv_color_fill = { 0 };
    hi_s32 ret = tde_osi_1_source_check(dst_surface, dst_rect, fill_color, &rect);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    if (tde_hal_node_init_nd(&hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    if (opt == HI_NULL) {
        base_mode = (fill_color->color_fmt == dst_surface->color_fmt) ? TDE_QUIKE_FILL : TDE_NORM_FILL_1OPT;
        out_alpha_from = HI_TDE_OUT_ALPHA_FROM_NORM;
    } else {
#ifndef HI_BUILD_IN_BOOT
        ret = tde_osi_1_source_src1(dst_surface, opt, hw_node, &rect);
        if (ret != HI_SUCCESS) {
            return ret;
        }
        out_alpha_from = opt->out_alpha_from;
        base_mode = (fill_color->color_fmt == dst_surface->color_fmt) ? TDE_QUIKE_FILL : TDE_NORM_FILL_1OPT;
#endif
    }
    if (base_mode == TDE_NORM_FILL_1OPT) {
        if (tde_osi_color_convert(fill_color, dst_surface, &drv_color_fill.fill_data) != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            return HI_ERR_TDE_INVALID_PARA;
        }
        drv_color_fill.drv_color_fmt = TDE_DRV_COLOR_FMT_ARGB8888;
    } else {
        drv_color_fill.fill_data = fill_color->color_value;
        drv_color_fill.drv_color_fmt = g_tde_common_drv_color_fmt[fill_color->color_fmt];
    }
    if (tde_hal_node_set_base_operate(hw_node, base_mode, TDE_ALU_NONE, &drv_color_fill) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    tde_osi_1_source_drv(dst_surface, base_mode, &rect, hw_node, out_alpha_from);
    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_2_check_opt(hi_tde_opt *opt)
{
    hi_bool real = (((opt->blend_opt.global_alpha_en != HI_TRUE) && (opt->blend_opt.global_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.pixel_alpha_en != HI_TRUE) && (opt->blend_opt.pixel_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.src1_alpha_premulti != HI_TRUE) && (opt->blend_opt.src1_alpha_premulti != HI_FALSE)) ||
        ((opt->blend_opt.src2_alpha_premulti != HI_TRUE) && (opt->blend_opt.src2_alpha_premulti != HI_FALSE)) ||
        ((opt->clut_reload != HI_TRUE) && (opt->clut_reload != HI_FALSE)) ||
        ((opt->is_compress != HI_TRUE) && (opt->is_compress != HI_FALSE)) ||
        ((opt->is_decompress != HI_TRUE) && (opt->is_decompress != HI_FALSE)));

    if (opt->out_alpha_from >= HI_TDE_OUT_ALPHA_FROM_MAX) {
        tde_error("out_alpha_from error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->alpha_blending_cmd >= HI_TDE_ALPHA_BLENDING_MAX) {
        tde_error("alpha_blending_cmd error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->color_key_mode >= HI_TDE_COLOR_KEY_MODE_MAX) ||
        (opt->color_key_mode < HI_TDE_COLOR_KEY_MODE_NONE)) {
        tde_error("color_key_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (real) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->deflicker_mode >= HI_TDE_DEFLICKER_LEVEL_MODE_MAX) ||
        (opt->deflicker_mode < HI_TDE_DEFLICKER_LEVEL_MODE_NONE)) {
        tde_error("deflicker_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->filter_mode >= HI_TDE_FILTER_MODE_MAX) || (opt->filter_mode < HI_TDE_FILTER_MODE_COLOR)) {
        tde_error("filter_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->mirror >= HI_TDE_MIRROR_MAX) || (opt->mirror < HI_TDE_MIRROR_NONE)) {
        tde_error("mirror error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_2_source_check(hi_tde_single_src *single_src, hi_tde_fill_color *fill_color,
                                     hi_tde_opt *opt, hi_tde_single_src *single_surface_dou)
{
    hi_s32 ret;
    hi_bool null_ptr = ((single_src->dst_surface == HI_NULL) || (single_src->dst_rect == HI_NULL) ||
                        (fill_color == HI_NULL) || ((opt == HI_NULL)) ||
                        (single_src->src_surface == HI_NULL) || (single_src->src_rect == HI_NULL));

    if (null_ptr) {
        return HI_ERR_TDE_NULL_PTR;
    }

    ret = memcpy_s(single_surface_dou->src_rect, sizeof(hi_tde_rect), single_src->src_rect, sizeof(hi_tde_rect));
    tde_unequal_eok_return(ret);
    ret = memcpy_s(single_surface_dou->dst_rect, sizeof(hi_tde_rect), single_src->dst_rect, sizeof(hi_tde_rect));
    tde_unequal_eok_return(ret);
    if (fill_color->color_fmt >= HI_TDE_COLOR_FMT_MAX) {
        tde_error("color format error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    ret = tde_osi_check_dst_fmt(single_src->dst_surface->color_fmt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    if ((single_src->src_surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) ||
        (single_src->src_surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("This operation doesn't support PKG!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#endif

    ret = tde_osi_2_check_opt(opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    if (tde_osi_check_surface(single_src->dst_surface, single_surface_dou->dst_rect) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_surface(single_src->src_surface, single_surface_dou->src_rect) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->resize) {
        if (tde_osi_check_resize_para(single_src->src_rect->width, single_src->src_rect->height,
                                      single_src->dst_rect->width, single_src->dst_rect->height) != HI_SUCCESS) {
            return HI_ERR_TDE_MINIFICATION;
        }
    }

    if (!opt->resize) {
        tde_unify_rect(single_surface_dou->src_rect, single_surface_dou->dst_rect);
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_2_source_para(hi_tde_single_src *single_src, hi_tde_fill_color *fill_color,
    hi_tde_opt *opt, tde_hw_node *hw_node, hi_tde_single_src *single_surface_dou)
{
    hi_s32 ret;
    hi_tde_double_src double_src;
    tde_alu_mode alu_mode = TDE_ALU_NONE;
    tde_color_fill drv_color_fill = { 0 };

#if (TDE_CAPABILITY & ROP)
    ret = tde_osi_set_rop(hw_node, opt->alpha_blending_cmd, opt->rop_color, opt->rop_alpha, &alu_mode, HI_FALSE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
#endif

    ret = tde_osi_set_blend(hw_node, opt->alpha_blending_cmd, opt->blend_opt, &alu_mode, HI_TRUE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
#if (TDE_CAPABILITY & COLORIZE)
    ret = tde_osi_set_colorize(hw_node, opt->alpha_blending_cmd, opt->color_resize);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
#endif
    tde_hal_node_set_global_alpha(hw_node, opt->global_alpha, opt->blend_opt.global_alpha_en);

    double_src.bg_surface = HI_NULL;
    double_src.bg_rect = HI_NULL;
    double_src.fg_surface = single_src->src_surface;
    double_src.fg_rect = single_surface_dou->src_rect;
    double_src.dst_surface = single_src->dst_surface;
    double_src.dst_rect = single_surface_dou->dst_rect;
    if (tde_osi_set_clip_para(&double_src, opt, hw_node) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_CLIP_AREA;
    }

    if (tde_osi_color_convert(fill_color, single_src->src_surface, &drv_color_fill.fill_data) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_hal_node_set_base_operate(hw_node, TDE_NORM_FILL_2OPT, alu_mode, &drv_color_fill) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_2_source_node(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_opt *opt,
                                    tde_hw_node *hw_node, hi_tde_single_src *single_surface_dou)
{
    hi_tde_double_src double_src;
    tde_clut_usage clut_usage = TDE_CLUT_USAGE_BUTT;
    hi_bool set_file_node = HI_FALSE;
    hi_s32 ret = tde_osi_set_clut_opt(single_src->src_surface, single_src->dst_surface, &clut_usage,
        opt->clut_reload, hw_node);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    ret = tde_osi_set_foreground_color_key(hw_node, single_src->src_surface, opt, clut_usage);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    set_file_node = ((opt->resize) || (opt->deflicker_mode != HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ||
                     (single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422));
    if (set_file_node) {
        double_src.bg_surface = single_src->src_surface;
        double_src.bg_rect = single_surface_dou->src_rect;
        double_src.fg_surface = HI_NULL;
        double_src.fg_rect = HI_NULL;
        double_src.dst_surface = single_src->dst_surface;
        double_src.dst_rect = single_surface_dou->dst_rect;
        ret = tde_osi_set_filter_node(handle, hw_node, &double_src,
                                      opt->deflicker_mode, opt->filter_mode);
        if (ret != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            return ret;
        }
#if (TDE_CAPABILITY & SLICE)
        tde_hal_free_node_buf(hw_node);
#endif
        return HI_SUCCESS;
    }
    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_2_source_fill(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_fill_color *fill_color,
                                    hi_tde_opt *opt)
{
    tde_hw_node *hw_node = HI_NULL;
    tde_surface_msg drv_surface = { 0 };
    tde_scandirection_mode src_scan_info = { 0 };
    tde_scandirection_mode dst_scan_info = { 0 };
    hi_tde_rect src_rect = { 0 };
    hi_tde_rect dst_rect = { 0 };
    hi_s32 ret;
    hi_tde_single_src single_surface_dou;

    single_surface_dou.dst_rect = &dst_rect;
    single_surface_dou.dst_surface = HI_NULL;
    single_surface_dou.src_rect = &src_rect;
    single_surface_dou.src_surface = HI_NULL;
    ret = tde_osi_2_source_check(single_src, fill_color, opt, &single_surface_dou);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    if (tde_hal_node_init_nd(&hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    ret = tde_osi_2_source_para(single_src, fill_color, opt, hw_node, &single_surface_dou);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    tde_osi_set_ext_alpha(HI_NULL, single_src->src_surface, hw_node);
    single_surface_dou.src_surface = single_src->src_surface;
    single_surface_dou.dst_surface = single_src->dst_surface;
    if (tde_osi_get_scan_info_ex(&single_surface_dou, opt, &src_scan_info, &dst_scan_info) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }

    tde_osi_convert_surface(single_src->src_surface, single_surface_dou.src_rect, &src_scan_info, &drv_surface);

    tde_hal_node_set_src1(hw_node, &drv_surface);

    tde_osi_convert_surface(single_src->dst_surface, single_surface_dou.dst_rect, &dst_scan_info, &drv_surface);

    tde_hal_node_set_tqt(hw_node, &drv_surface, opt->out_alpha_from);
    ret = tde_osi_2_source_node(handle, single_src, opt, hw_node, &single_surface_dou);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_set_foreground_color_key(tde_hw_node *hw_node, hi_tde_surface *src_surface, hi_tde_opt *opt,
                                               tde_clut_usage clut_usage)
{
    tde_color_key_cmd color_key;
    tde_colorfmt_category fmt_category;
    hi_bool foreground_colorkey_after_clut = HI_FALSE;
    color_key.color_key_value = opt->color_key_value;
    foreground_colorkey_after_clut = ((clut_usage != TDE_CLUT_COLOREXPENDING) && (clut_usage != TDE_CLUT_CLUT_BYPASS));

    if (opt->color_key_mode == HI_TDE_COLOR_KEY_MODE_BACKGROUND) {
        tde_error("Unsupported solidraw colorkey in background mode!\n");

        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->color_key_mode != HI_TDE_COLOR_KEY_MODE_FOREGROUND) &&
        (opt->color_key_mode != HI_TDE_COLOR_KEY_MODE_BACKGROUND)) {
        return HI_SUCCESS;
    }

    color_key.color_key_mode = (foreground_colorkey_after_clut) ? TDE_DRV_COLORKEY_FOREGROUND_AFTER_CLUT :
                                TDE_DRV_COLORKEY_FOREGROUND_BEFORE_CLUT;

    fmt_category = tde_osi_get_fmt_category(src_surface->color_fmt);
    if (fmt_category >= TDE_COLORFMT_CATEGORY_BUTT) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if (tde_hal_node_set_colorkey(hw_node, fmt_category, &color_key) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_check_surface
 * Description:   adjust right operate zone, according by the size of bitmap and operate zone from user upload
 * Input:         surface: bitmap info
 *                rect: bitmap operate zone
 * Return:        success/fail
 */
static hi_s32 tde_osi_check_surface(hi_tde_surface *surface, hi_tde_rect *rect)
{
    hi_bool invalid_operation_area = HI_FALSE;
    hi_s32 ret = tde_osi_pre_check_surface_ex(surface, rect);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    invalid_operation_area = ((TDE_MAX_RECT_WIDTH < rect->width) || (TDE_MAX_RECT_HEIGHT < rect->height));

    if (invalid_operation_area) {
        tde_error("operation area is over maximum!Width:%d,Height:%d\n", rect->width, rect->height);
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_pre_check_surface(hi_tde_surface *surface, hi_tde_rect *rect)
{
    hi_bool real = ((surface->stride > TDE_MAX_SURFACE_PITCH) || (surface->stride == 0) || (rect->height == 0) ||
        (rect->width == 0) || (rect->pos_x < 0) || ((hi_u32)rect->pos_x >= surface->width) ||
        (rect->pos_y < 0) || ((hi_u32)rect->pos_y >= surface->height) || (surface->phy_addr == 0));

    if (surface->color_fmt >= HI_TDE_COLOR_FMT_MAX) {
        tde_error("color format error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (real) {
        tde_error("sw:%d, sh:%d, stride:%d, sphy:%llx, x:%d, y:%d, w:%d, h:%d\n", surface->width,
            surface->height, surface->stride, surface->phy_addr, rect->pos_x,
            rect->pos_y, rect->width, rect->height);
        tde_error("invalid surface phyaddr or invalid surface size or operation area!");
        return -1;
    }
    real = (((surface->support_alpha_ext_1555 != HI_TRUE) && (surface->support_alpha_ext_1555 != HI_FALSE)) ||
        ((surface->alpha_max_is_255 != HI_TRUE) && (surface->alpha_max_is_255 != HI_FALSE)) ||
        ((surface->is_ycbcr_clut != HI_TRUE) && (surface->is_ycbcr_clut != HI_FALSE)));
    if (real) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

#ifdef  CONFIG_DRM_HISI_HISMART
extern int drm_check_dumb_phy_addr(unsigned long long addr_start, unsigned long long size);
static hi_s32 tde_osi_pre_check_drm_dumb(hi_tde_surface *surface, hi_u32 size)
{
    hi_s32 ret;
    ret = drm_check_dumb_phy_addr(surface->phy_addr, size);
    if (ret != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }
    if (surface->cbcr_phy_addr != 0) {
        size = surface->cbcr_stride * surface->height;
        if ((surface->color_fmt == HI_TDE_COLOR_FMT_JPG_YCBCR422MBVP) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_MP1_YCBCR420MBP) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_MP2_YCBCR420MBP) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_MP2_YCBCR420MBI) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_JPG_YCBCR420MBP)) {
            size = surface->cbcr_stride * (surface->height / 2); /* 2 half data height */
        }
        ret = drm_check_dumb_phy_addr(surface->cbcr_phy_addr, size);
        if (ret != HI_SUCCESS) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    return HI_SUCCESS;
}
#endif

static hi_s32 tde_osi_pre_check_mmz(hi_tde_surface *surface, hi_u32 size)
{
    hi_s32 ret;
#ifdef  CONFIG_DRM_HISI_HISMART
    ret = tde_osi_pre_check_drm_dumb(surface, size);
    if (ret == HI_SUCCESS) {
        return HI_SUCCESS;
    }
#endif
    ret = cmpi_check_mmz_phy_addr(surface->phy_addr, size);
    if (ret != HI_SUCCESS) {
        tde_error("Check surface phy addr failed!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    if (surface->cbcr_phy_addr != 0) {
        size = surface->cbcr_stride * surface->height;
        if ((surface->color_fmt == HI_TDE_COLOR_FMT_JPG_YCBCR422MBVP) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_MP1_YCBCR420MBP) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_MP2_YCBCR420MBP) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_MP2_YCBCR420MBI) ||
            (surface->color_fmt == HI_TDE_COLOR_FMT_JPG_YCBCR420MBP)) {
            size = surface->cbcr_stride * (surface->height / 2); /* 2 half data height */
        }
        ret = cmpi_check_mmz_phy_addr(surface->cbcr_phy_addr, size);
        if (ret != HI_SUCCESS) {
            tde_error("Check MB surface cbcr_phy_addr addr failed!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    return HI_SUCCESS;
}

static hi_void tde_osi_get_rect_wh(hi_tde_surface *surface, hi_tde_rect *rect)
{
    if (rect->pos_x + rect->width > surface->width) {
        rect->width = surface->width - rect->pos_x;
    }

    if (rect->pos_y + rect->height > surface->height) {
        rect->height = surface->height - rect->pos_y;
    }
}

static hi_s32 tde_osi_pre_check_surface_ex(hi_tde_surface *surface, hi_tde_rect *rect)
{
    hi_bool unknown_color_fmt = HI_FALSE;
    hi_bool raster_fmt = HI_FALSE;
    hi_u32 size;
    hi_s32 bpp;
    hi_s32 ret = tde_osi_pre_check_surface(surface, rect);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    bpp = tde_osi_get_bpp_by_fmt(surface->color_fmt);
    /* CMPI_CheckMmzphy_addr Call a semaphore,Cannot be used in the interrupt */
    if (!osal_in_interrupt()) {
        if (bpp >= 8) { /* 8 bpp data */
            size = (surface->stride * (surface->height - 1)) + (surface->width * bpp / 8);
        } else {
            size = surface->stride * surface->height;
        }
        ret = tde_osi_pre_check_mmz(surface, size);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    }
    unknown_color_fmt = (surface->color_fmt >= HI_TDE_COLOR_FMT_MAX);
    if (unknown_color_fmt) {
        tde_error("Unknown color format!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    unknown_color_fmt = ((((hi_u32)(rect->pos_x) & 0x1) || (rect->width & 0x1)) &&
        ((surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) || (surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)));
    if (unknown_color_fmt) {
        tde_error("x, width of YCbCr422R couldn't be odd!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    raster_fmt = (surface->color_fmt <= HI_TDE_COLOR_FMT_HALFWORD);
    if (raster_fmt) {
        ret = tde_osi_raster_fmt_check_align(surface);
        if (ret != HI_SUCCESS) {
            return ret;
        }
    } else {
        if (((surface->cbcr_stride > TDE_MAX_SURFACE_PITCH) || (surface->cbcr_stride == 0)) &&
            (surface->color_fmt != HI_TDE_COLOR_FMT_JPG_YCBCR400MBP)) {
            tde_error("Invalid CbCr stride!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    tde_osi_get_rect_wh(surface, rect);
    return HI_SUCCESS;
}

static hi_s32 tde_osi_raster_fmt_check_align(hi_tde_surface *surface)
{
    hi_u32 byte_per_pixel;
    hi_s32 bpp = tde_osi_get_bpp_by_fmt(surface->color_fmt);
    if ((bpp >= 8) && (bpp != 24)) { /* 8 24 Bpp */
        byte_per_pixel = ((hi_u32)bpp >> 3); /* 3 Bpp /8 */

        if (osal_div_u64_rem(surface->phy_addr, byte_per_pixel)) {
            tde_error("Bitmap address is not aligned!\n");
            return HI_ERR_TDE_NOT_ALIGNED;
        }

        if (surface->stride % byte_per_pixel) {
            tde_error("stride is not aligned!\n");
            return HI_ERR_TDE_NOT_ALIGNED;
        }
    } else if (bpp == 24) { /* 24 Bpp */
        if (osal_div_u64_rem(surface->phy_addr, 4)) { /* 4 align_num */
            tde_error("Bitmap address is not aligned!\n");
            return HI_ERR_TDE_NOT_ALIGNED;
        }
        if (surface->stride % 4) { /* 4 align_num */
            tde_error("stride is not aligned!\n");
            return HI_ERR_TDE_NOT_ALIGNED;
        }
    }

    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
/*
 * Function:      tde_osi_check_mb_surface_ex
 * Description:   adjust right operate zone,
                  according by the size of bitmap is more than 4095*4095 and less than 8190*8190
 * Input:         mb_surface: bitmap info
 *                rect: bitmap operate zone
 * Return:        success/fail
 */
static hi_s32 tde_osi_check_mb_surface_ex(hi_tde_mb_surface *mb_surface, hi_tde_rect *rect)
{
    hi_bool real = ((rect->pos_x < 0) || (rect->width > TDE_MAX_RECT_WIDTH_EX) ||
        (rect->height > TDE_MAX_RECT_HEIGHT_EX) || (rect->height == 0) || (rect->width == 0) ||
        (mb_surface->y_stride > TDE_MAX_SURFACE_PITCH) || (mb_surface->cbcr_stride > TDE_MAX_SURFACE_PITCH) ||
        ((hi_u32)rect->pos_x >= mb_surface->y_width) || (rect->pos_y < 0) ||
        ((hi_u32)rect->pos_y >= mb_surface->y_height) || (mb_surface->y_addr == 0));

    if (mb_surface->mb_color_fmt >= HI_TDE_MB_COLOR_FMT_MAX) {
        tde_error("mb color format error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((mb_surface->mb_color_fmt > HI_TDE_MB_COLOR_FMT_JPG_YCBCR400MBP) &&
        (mb_surface->mb_color_fmt < HI_TDE_MB_COLOR_FMT_MAX)) {
        if (mb_surface->cbcr_stride == 0) {
            tde_error(" mb cbcr_stride is null !\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
        if (mb_surface->cbcr_phy_addr == 0) {
            tde_error("mb cbcr_phy_addr is 0!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    if (real) {
        tde_error("syw:%d, syh:%d, systride:%x, syphy:%llx, scbcrstride:%x, scbcrphy:%llx, x:%d, y:%d, w:%d, h:%d\n",
            mb_surface->y_width, mb_surface->y_height, mb_surface->y_stride, mb_surface->y_addr,
            mb_surface->cbcr_stride, mb_surface->cbcr_phy_addr, rect->pos_x, rect->pos_y, rect->width,
            rect->height);
        tde_error("invalid mbsurface phyaddr or invalid surface size or operation area!\n");
        return -1;
    }
    if ((mb_surface->y_stride % 4) || (mb_surface->cbcr_stride % 4)) { /* 4 align_num */
        tde_error("stride is not aligned!\n");
        return HI_ERR_TDE_NOT_ALIGNED;
    }

    if ((mb_surface->mb_color_fmt != HI_TDE_MB_COLOR_FMT_JPG_YCBCR400MBP) && (mb_surface->cbcr_phy_addr == 0)) {
        tde_error("MB fmt :%d cbcr_phy_addr is HI_NULL !!\n", mb_surface->mb_color_fmt);
        return HI_FAILURE;
    }
    if (rect->pos_x + rect->width > mb_surface->y_width) {
        rect->width = mb_surface->y_width - rect->pos_x;
    }

    if (rect->pos_y + rect->height > mb_surface->y_height) {
        rect->height = mb_surface->y_height - rect->pos_y;
    }

    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_get_opt_category
 * Description:   analyze TDE operate type
 * Return:        TDE operate type
 */
static tde_operation_category tde_osi_get_opt_category(hi_tde_double_src *double_src, hi_tde_opt *opt)
{
    if ((double_src->dst_surface == HI_NULL) || (double_src->dst_rect == HI_NULL)) {
        tde_error("dst is HI_NULL!\n");
        return TDE_OPERATION_BUTT;
    }

    if (double_src->dst_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_surface(double_src->dst_surface, double_src->dst_rect) != HI_SUCCESS) {
        return TDE_OPERATION_BUTT;
    }
    if ((double_src->bg_surface == HI_NULL) && (double_src->fg_surface == HI_NULL)) {
        tde_error("null pointer in single source 2 operation!");
        return TDE_OPERATION_BUTT;
    } else if ((double_src->bg_surface != HI_NULL) && (double_src->fg_surface != HI_NULL)) {
        return tde_osi_double_src_operation(double_src, opt);
    } else {
        return tde_osi_single_src_operation(double_src, opt);
    }
}

static tde_operation_category tde_osi_single_src_operation(hi_tde_double_src *double_src, hi_tde_opt *opt)
{
    hi_tde_surface *tmp_src2 = HI_NULL;
    hi_tde_rect *tmp_src2_rect = HI_NULL;

    if ((double_src->bg_surface != HI_NULL) && (double_src->fg_surface == HI_NULL)) {
        tmp_src2 = double_src->bg_surface;
        tmp_src2_rect = double_src->bg_rect;
    } else {
        tmp_src2 = double_src->fg_surface;
        tmp_src2_rect = double_src->fg_rect;
    }

    if (tmp_src2_rect == HI_NULL) {
        return TDE_OPERATION_BUTT;
    }

    if (tde_osi_check_surface(tmp_src2, tmp_src2_rect) != HI_SUCCESS) {
        return TDE_OPERATION_BUTT;
    }

    if ((opt == HI_NULL) || (!opt->resize)) {
        tde_unify_rect(tmp_src2_rect, double_src->dst_rect);
    }

    if ((opt == HI_NULL) && (tmp_src2->color_fmt == double_src->dst_surface->color_fmt)) {
        return TDE_OPERATION_SINGLE_SRC1;
    }

    if (opt == HI_NULL) {
        tde_error("source format must be the same with dst format!\n");
        return TDE_OPERATION_BUTT;
    }

    return TDE_OPERATION_SINGLE_SRC2;
}

static tde_operation_category tde_osi_double_src_operation(hi_tde_double_src *double_src, hi_tde_opt *opt)
{
    if ((double_src->bg_rect == HI_NULL) || (double_src->fg_rect == HI_NULL) ||
        (opt == HI_NULL)) {
        tde_error("Null pointer!\n");
        return TDE_OPERATION_BUTT;
    }

    if (double_src->bg_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

#ifdef TDE_BACKGROUND_COLORFOMATSUPPORT_YCBCR422
    if (double_src->bg_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422) {
        tde_error("Background  doesn't support the colorfmt!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#endif

    if (tde_osi_check_surface(double_src->bg_surface, double_src->bg_rect) != HI_SUCCESS) {
        return TDE_OPERATION_BUTT;
    }

    if (tde_osi_check_surface(double_src->fg_surface, double_src->fg_rect) != HI_SUCCESS) {
        return TDE_OPERATION_BUTT;
    }

    if ((double_src->bg_rect->height != double_src->dst_rect->height) ||
        (double_src->bg_rect->width != double_src->dst_rect->width)) {
        tde_error("BackGroundRect is not the same with DstRect!\n");
        return TDE_OPERATION_BUTT;
    }

    if (!opt->resize) {
        if (double_src->bg_rect->height != double_src->fg_rect->height) {
            double_src->bg_rect->height = tde_min(double_src->bg_rect->height,
                double_src->fg_rect->height);
            double_src->dst_rect->height = double_src->bg_rect->height;
            double_src->fg_rect->height = double_src->bg_rect->height;
        }

        if (double_src->bg_rect->width != double_src->fg_rect->width) {
            double_src->bg_rect->width = tde_min(double_src->bg_rect->width,
                double_src->fg_rect->width);
            double_src->dst_rect->width = double_src->bg_rect->width;
            double_src->fg_rect->width = double_src->bg_rect->width;
        }
    }

    return TDE_OPERATION_DOUBLE_SRC;
}

/*
 * Function:      tde_osi_convert_surface
 * Description:   raster bitmap info by user upload translate to bitmap info which driver and hardware need
 * Input:         sur: raster bitmap info by user upload
 *                rect: raster bitmap operate zone by user upload
 *                scan_info: scanning direction info
 *                drv_sur: bitmap info which driver and hardware need
 */
static hi_void tde_osi_convert_surface(hi_tde_surface *sur, hi_tde_rect *rect,
                                       tde_scandirection_mode *scan_info,
                                       tde_surface_msg *drv_sur)
{
    drv_sur->color_fmt = g_tde_common_drv_color_fmt[sur->color_fmt];
    drv_sur->width = rect->width;
    drv_sur->height = rect->height;
    drv_sur->pitch = sur->stride;
    drv_sur->alpha_max_is_255 = sur->alpha_max_is_255;
    drv_sur->h_scan = scan_info->h_scan;
    drv_sur->v_scan = scan_info->v_scan;
    drv_sur->phy_addr = sur->phy_addr;
    drv_sur->cbcr_phy_addr = sur->cbcr_phy_addr;
    drv_sur->cb_cr_pitch = sur->cbcr_stride;
    drv_sur->rgb_order = g_tde_argb_order[sur->color_fmt];

    drv_sur->xpos = (hi_u32)rect->pos_x;

    drv_sur->ypos = (hi_u32)rect->pos_y;

    return;
}

static hi_s32 tde_osi_check_clip_para(hi_tde_double_src *double_src, hi_tde_opt *opt, tde_hw_node *hw_node)
{
    if (double_src->fg_surface == HI_NULL) {
        tde_error("double_src->fg_surface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (double_src->dst_surface == HI_NULL) {
        tde_error("double_src->dst_surface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (opt == HI_NULL) {
        tde_error("pstOpt is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (hw_node == HI_NULL) {
        tde_error("pstHwNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (opt->clip_mode >= HI_TDE_CLIP_MODE_MAX) {
        tde_error("clip_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->clip_mode != HI_TDE_CLIP_MODE_NONE) {
        if ((opt->clip_rect.height == 0) || (opt->clip_rect.width == 0) || (opt->clip_rect.pos_x < 0) ||
            ((hi_u32)opt->clip_rect.pos_x >= TDE_MAX_RECT_WIDTH) ||
            (opt->clip_rect.pos_y < 0) || ((hi_u32)opt->clip_rect.pos_y >= TDE_MAX_RECT_HEIGHT) ||
            ((hi_u32)opt->clip_rect.width >= TDE_MAX_RECT_WIDTH) ||
            ((hi_u32)opt->clip_rect.height >= TDE_MAX_RECT_HEIGHT)) {
            tde_error(" Clip rect x:%d, y:%d, w:%d, h:%d error!\n", opt->clip_rect.pos_x,
                opt->clip_rect.pos_y, opt->clip_rect.width, opt->clip_rect.height);
            return -1;
        }
    }
    return HI_SUCCESS;
}

static hi_void tde_osi_double_src(hi_tde_double_src *double_src, hi_tde_rect *inter_rect)
{
    if (double_src->bg_surface != HI_NULL) {
        double_src->bg_rect->pos_x += inter_rect->pos_x - double_src->dst_rect->pos_x;
        double_src->bg_rect->pos_y += inter_rect->pos_y - double_src->dst_rect->pos_y;
        double_src->bg_rect->height = inter_rect->height;
        double_src->bg_rect->width = inter_rect->width;
    }

    double_src->fg_rect->pos_x += inter_rect->pos_x - double_src->dst_rect->pos_x;
    double_src->fg_rect->pos_y += inter_rect->pos_y - double_src->dst_rect->pos_y;
    double_src->fg_rect->height = inter_rect->height;
    double_src->fg_rect->width = inter_rect->width;

    *(double_src->dst_rect) = *inter_rect;
}

/*
 * Function:      tde_osi_set_clip_para
 * Description:   set clip zone parameter
 * Input:         double_src:  bitmap info
 *                opt: operate option
 *                hw_node: hardware operate node
 */
static hi_s32 tde_osi_set_clip_para(hi_tde_double_src *double_src, hi_tde_opt *opt, tde_hw_node *hw_node)
{
    tde_clip_cmd clip = { 0 };
    hi_tde_rect inter_rect = { 0 };
    hi_s32 ret;
    ret = tde_osi_check_clip_para(double_src, opt, hw_node);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    if ((opt->clip_mode == HI_TDE_CLIP_MODE_INSIDE) && (!opt->resize)) {
        if (tde_osi_get_inter_rect(double_src->dst_rect, &opt->clip_rect, &inter_rect) != HI_SUCCESS) {
            tde_error("clip and operation area have no inrerrect!\n");
            return HI_ERR_TDE_CLIP_AREA;
        }
        tde_osi_double_src(double_src, &inter_rect);
    } else if (opt->clip_mode == HI_TDE_CLIP_MODE_INSIDE) {
        if (tde_osi_get_inter_rect(double_src->dst_rect, &opt->clip_rect, &inter_rect) != HI_SUCCESS) {
            tde_error("clip and operation area have no inter-rect!\n");
            return HI_ERR_TDE_CLIP_AREA;
        }

        clip.inside_clip = HI_TRUE;
        clip.clip_start_x = opt->clip_rect.pos_x - double_src->dst_rect->pos_x;
        clip.clip_start_y = opt->clip_rect.pos_y - double_src->dst_rect->pos_y;
        clip.clip_end_x = (opt->clip_rect.pos_x - double_src->dst_rect->pos_x) + opt->clip_rect.width - 1;
        clip.clip_end_y = (opt->clip_rect.pos_y - double_src->dst_rect->pos_y) + opt->clip_rect.height - 1;
        if (tde_hal_node_set_clipping(hw_node, &clip) < 0) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    } else if (opt->clip_mode == HI_TDE_CLIP_MODE_OUTSIDE) {
        if (tde_osi_is_rect1_in_rect2(double_src->dst_rect, &opt->clip_rect)) {
            tde_error("clip and operation area have no inter-rect!\n");
            return HI_ERR_TDE_CLIP_AREA;
        }

        if (tde_osi_get_inter_rect(double_src->dst_rect, &opt->clip_rect, &inter_rect) != HI_SUCCESS) {
            return HI_SUCCESS;
        }

        clip.inside_clip = HI_FALSE;
        clip.clip_start_x = opt->clip_rect.pos_x - double_src->dst_rect->pos_x;
        clip.clip_start_y = opt->clip_rect.pos_y - double_src->dst_rect->pos_y;
        clip.clip_end_x = (opt->clip_rect.pos_x - double_src->dst_rect->pos_x) + opt->clip_rect.width - 1;
        clip.clip_end_y = (opt->clip_rect.pos_y - double_src->dst_rect->pos_y) + opt->clip_rect.height - 1;

        if (tde_hal_node_set_clipping(hw_node, &clip) != HI_SUCCESS) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    } else if (opt->clip_mode >= HI_TDE_CLIP_MODE_MAX) {
        tde_error("error clip mode!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_check_sre_para(hi_tde_double_src *double_src, hi_tde_pattern_fill_opt *opt,
                                     tde_hw_node *hw_node)
{
    if (double_src->fg_surface == HI_NULL) {
        tde_error("double_src->fg_surface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (double_src->dst_surface == HI_NULL) {
        tde_error("double_src->dst_surface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (opt == HI_NULL) {
        tde_error("opt is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (hw_node == HI_NULL) {
        tde_error("hw_node is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (opt->clip_mode >= HI_TDE_CLIP_MODE_MAX) {
        tde_error("clip_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->clip_mode != HI_TDE_CLIP_MODE_NONE) {
        if ((opt->clip_rect.height == 0) || (opt->clip_rect.width == 0) || (opt->clip_rect.pos_x < 0) ||
            ((hi_u32)opt->clip_rect.pos_x >= TDE_MAX_RECT_WIDTH) ||
            (opt->clip_rect.pos_y < 0) || ((hi_u32)opt->clip_rect.pos_y >= TDE_MAX_RECT_HEIGHT) ||
            ((hi_u32)opt->clip_rect.width >= TDE_MAX_RECT_WIDTH) ||
            ((hi_u32)opt->clip_rect.height >= TDE_MAX_RECT_HEIGHT)) {
            tde_error(" Clip rect x:%d, y:%d, w:%d, h:%d error!\n", opt->clip_rect.pos_x,
                opt->clip_rect.pos_y, opt->clip_rect.width, opt->clip_rect.height);
            return -1;
        }
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_set_pattern_clip_para(hi_tde_double_src *double_src, hi_tde_pattern_fill_opt *opt,
                                            tde_hw_node *hw_node)
{
    tde_clip_cmd clip = {0};
    hi_tde_rect inter_rect = {0};
    hi_s32 ret;
    ret = tde_osi_check_sre_para(double_src, opt, hw_node);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (opt->clip_mode == HI_TDE_CLIP_MODE_INSIDE) {
        if (tde_osi_get_inter_rect(double_src->dst_rect, &opt->clip_rect, &inter_rect) != HI_SUCCESS) {
            tde_error("clip and operation area have no inrerrect!\n");
            return HI_ERR_TDE_CLIP_AREA;
        }
        clip.inside_clip = HI_TRUE;
        clip.clip_start_x = opt->clip_rect.pos_x - double_src->dst_rect->pos_x;
        clip.clip_start_y = opt->clip_rect.pos_y - double_src->dst_rect->pos_y;
        clip.clip_end_x = (opt->clip_rect.pos_x - double_src->dst_rect->pos_x) + opt->clip_rect.width - 1;
        clip.clip_end_y = (opt->clip_rect.pos_y - double_src->dst_rect->pos_y) + opt->clip_rect.height - 1;

        if (tde_hal_node_set_clipping(hw_node, &clip) != HI_SUCCESS) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    } else if (opt->clip_mode == HI_TDE_CLIP_MODE_OUTSIDE) {
        if (tde_osi_is_rect1_in_rect2(double_src->dst_rect, &opt->clip_rect)) {
            tde_error("clip and operation area have no inter-rect!\n");
            return HI_ERR_TDE_CLIP_AREA;
        }

        if (tde_osi_get_inter_rect(double_src->dst_rect, &opt->clip_rect, &inter_rect) != HI_SUCCESS) {
            return HI_SUCCESS;
        }

        clip.inside_clip = HI_FALSE;
        clip.clip_start_x = opt->clip_rect.pos_x - double_src->dst_rect->pos_x;
        clip.clip_start_y = opt->clip_rect.pos_y - double_src->dst_rect->pos_y;
        clip.clip_end_x = (opt->clip_rect.pos_x - double_src->dst_rect->pos_x) + opt->clip_rect.width - 1;
        clip.clip_end_y = (opt->clip_rect.pos_y - double_src->dst_rect->pos_y) + opt->clip_rect.height - 1;

        if (tde_hal_node_set_clipping(hw_node, &clip) != HI_SUCCESS) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    } else if (opt->clip_mode >= HI_TDE_CLIP_MODE_MAX) {
        tde_error("error clip mode!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}
#endif

static hi_void tde_osi_set_ext_alpha(hi_tde_surface *back_ground, hi_tde_surface *fore_ground,
                                     tde_hw_node *hw_node)
{
    tde_src_mode src = TDE_DRV_SRC_NONE;
    hi_bool real = HI_FALSE;
    real = ((fore_ground != HI_NULL) && (fore_ground->color_fmt >= HI_TDE_COLOR_FMT_ARGB1555) &&
        (fore_ground->color_fmt <= HI_TDE_COLOR_FMT_BGRA1555));
    if (real) {
        tde_hal_node_set_src2_alpha(hw_node);
    }

    real = ((back_ground != HI_NULL) &&
        (back_ground->color_fmt >= HI_TDE_COLOR_FMT_ARGB1555) &&
        (back_ground->color_fmt <= HI_TDE_COLOR_FMT_BGRA1555));
    if (real) {
        tde_hal_node_set_src1_alpha(hw_node);
    }

    real = ((fore_ground != HI_NULL) && (fore_ground->support_alpha_ext_1555) &&
        (fore_ground->color_fmt >= HI_TDE_COLOR_FMT_ARGB1555) &&
        (fore_ground->color_fmt <= HI_TDE_COLOR_FMT_BGRA1555));
    if (real) {
        src = (hi_u32)src | TDE_DRV_SRC_S2;
    }

    real = ((back_ground != HI_NULL) && (back_ground->support_alpha_ext_1555) &&
        (back_ground->color_fmt >= HI_TDE_COLOR_FMT_ARGB1555) &&
        (back_ground->color_fmt <= HI_TDE_COLOR_FMT_BGRA1555));
    if (real) {
        src = (hi_u32)src | TDE_DRV_SRC_S1;
    }

    if ((hi_u32)src & TDE_DRV_SRC_S1) {
        tde_hal_node_set_exp_alpha(hw_node, src, back_ground->alpha0, back_ground->alpha1);
    } else if ((hi_u32)src & TDE_DRV_SRC_S2) {
        tde_hal_node_set_exp_alpha(hw_node, src, fore_ground->alpha0, fore_ground->alpha1);
    }
}

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_set_blend_opt_rop(hi_tde_opt *opt, tde_hw_node *hw_node, tde_alu_mode *alu)
{
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_BLEND) {
        *alu = TDE_ALU_BLEND;
        if (tde_hal_node_set_blend(hw_node, &opt->blend_opt) != HI_SUCCESS) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
#if (TDE_CAPABILITY & ROP)
        if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) {
            tde_hal_node_enable_alpha_rop(hw_node);
        }
#endif
    }
#if (TDE_CAPABILITY & COLORIZE)
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_COLORIZE) {
        if (tde_hal_node_set_colorize(hw_node, opt->color_resize) != HI_SUCCESS) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    }
#endif
    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_set_base_opt_para_for_blit
 * Description:   encapsulation function used to set operate type
 * Input:         opt: operate option
 *                opt_category: operate category
 *                hw_node: nareware operate node
 */
static hi_s32 tde_osi_set_base_opt_para_for_blit(hi_tde_opt *opt, hi_tde_surface *src1, hi_tde_surface *src2,
                                                 tde_operation_category opt_category, tde_hw_node *hw_node)
{
    tde_base_opt_mode base_opt = { 0 };
    tde_alu_mode alu = TDE_ALU_NONE;
    hi_s32 ret;

    if ((hw_node == HI_NULL) || (opt == HI_NULL)) {
        return HI_SUCCESS;
    }

    if ((src1 != HI_NULL) && (src2 != HI_NULL)) {
        if ((tde_osi_get_fmt_category(src2->color_fmt) == TDE_COLORFMT_CATEGORY_AN) &&
            ((tde_osi_get_fmt_category(src1->color_fmt) == TDE_COLORFMT_CATEGORY_ARGB) ||
            (tde_osi_get_fmt_category(src1->color_fmt) == TDE_COLORFMT_CATEGORY_YCBCR))) {
            alu = TDE_SRC1_BYPASS;
        }
    }
#if (TDE_CAPABILITY & ROP)
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) {
        alu = TDE_ALU_ROP;

        if (tde_hal_node_set_rop(hw_node, opt->rop_color, opt->rop_alpha) != HI_SUCCESS) {
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    }
#endif
#ifndef HI_BUILD_IN_BOOT
    ret = tde_osi_set_blend_opt_rop(opt, hw_node, &alu);
    if (ret != HI_SUCCESS) {
        return ret;
    }
#endif

    base_opt = (opt_category == TDE_OPERATION_DOUBLE_SRC) ? TDE_NORM_BLIT_2OPT : TDE_NORM_BLIT_1OPT;

    tde_hal_node_set_global_alpha(hw_node, opt->global_alpha, opt->blend_opt.global_alpha_en);

    if (tde_hal_node_set_base_operate(hw_node, base_opt, alu, 0) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = tde_set_node_csc(hw_node, opt->csc_opt);

    return ret;
}

/*
 * Function:      tde_osi_adj_clip_para
 * Description:   set clip parameter to hardware node
 * Input:         hw_node: node need to set
 */
static hi_s32 tde_osi_adj_clip_para(tde_hw_node *hw_node)
{
    hi_u32 clip_start_x, clip_start_y, clip_end_x, clip_end_y;
    hi_u32 out_start_x, out_start_y, out_end_x, out_end_y;
    hi_u32 start_x, start_y, end_x, end_y;

    if (!(hw_node->des_alpha.bits.des_crop_en)) {
        return HI_SUCCESS;
    }

    clip_start_x = hw_node->des_crop_pos_st.bits.des_crop_start_x & 0xfff;
    clip_start_y = hw_node->des_crop_pos_st.bits.des_crop_start_y & 0xfff;
    clip_end_x = hw_node->des_crop_pos_ed.bits.des_crop_end_x & 0xfff;
    clip_end_y = hw_node->des_crop_pos_ed.bits.des_crop_end_y & 0xfff;

    out_start_x = 0;
    out_start_y = 0;
    out_end_x = out_start_x + ((hw_node->des_imgsize.bits.des_width + 1) & 0xfff) - 1;
    out_end_y = out_start_y + ((hw_node->des_imgsize.bits.des_height + 1) & 0xfff) - 1;

    start_x = (clip_start_x > out_start_x) ? clip_start_x : out_start_x;
    start_y = (clip_start_y > out_start_y) ? clip_start_y : out_start_y;
    end_x = (clip_end_x < out_end_x) ? clip_end_x : out_end_x;
    end_y = (clip_end_y < out_end_y) ? clip_end_y : out_end_y;

    if ((start_x > end_x) || (start_y > end_y)) {
        hw_node->des_alpha.bits.des_crop_en = 0;
    } else {
        hw_node->des_crop_pos_st.bits.des_crop_start_x = start_x;
        hw_node->des_crop_pos_st.bits.des_crop_start_y = start_y;
        hw_node->des_crop_pos_ed.bits.des_crop_end_x = end_x;
        hw_node->des_crop_pos_ed.bits.des_crop_end_y = end_y;
    }

    return HI_SUCCESS;
}

static hi_void tde_osi_none_cmd(tde_hw_node *hw_node, tde_swnode *cmd, tde_swjob *job)
{
    tde_hw_node *hw_tail_node = HI_NULL;

#ifndef HI_BUILD_IN_BOOT
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    tdeproc_record_node(hw_node);
#endif
#endif

#ifndef __RTOS__
    cmd->node_buf.buf = (hi_void *)hw_node - TDE_NODE_HEAD_BYTE;
#else
    cmd->node_buf.buf = (hi_u8 *)hw_node - TDE_NODE_HEAD_BYTE;
#endif

    cmd->node_buf.node_sz = sizeof(tde_hw_node);
    cmd->node_buf.update = (0xffffffff) | ((hi_u64)0x000003ff << 32); /* 32 alg data */
    cmd->node_buf.phy_addr = wgetphy(cmd->node_buf.buf) + TDE_NODE_HEAD_BYTE;
    /*
     * If the tail node of the job is not null,add the current job node to the tail of the job list of hardware.
     * That is to say the next node address of the tail\
     * node is the current node address.
     */
    if (job->tail_node != HI_NULL) {
        hw_tail_node = (tde_hw_node *)((hi_u8 *)job->tail_node->node_buf.buf + TDE_NODE_HEAD_BYTE);
        hw_tail_node->tde_pnext_low.bits.next_low = GetLowAddr(cmd->node_buf.phy_addr);
        hw_tail_node->tde_pnext_hi.bits.next_hi = GetHighAddr(cmd->node_buf.phy_addr);
    }
    return;
}

/*
 * Function:      tde_osi_set_node_finish
 * Description:   complete node config, add node to list signed by handle
 * Input:         handle: task handle, which is sumbit list
 *                hw_node: set node
 *                work_buf_num: temporary buffer number
 *                subm_type: submit node type
 * Return:        return slice number
 * Others:        node struct is following:
 *                 ----------------------
 *                 |  software node pointer(4)   |
 *                 ----------------------
 *                 |  config parameter          |
 *                 ----------------------
 *                 |  physical address of next node(4) |
 *                 ----------------------
 *                 |  update flag of next node(4) |
 *                 ----------------------
 */
static hi_s32 tde_osi_set_node_finish(hi_s32 handle, tde_hw_node *hw_node,
                                      hi_u32 work_buf_num, tde_node_subm_type subm_type)
{
    hi_s32 ret;
    hi_handle_mgr *handle_mgr = HI_NULL;
    tde_swjob *job = HI_NULL;
    tde_swnode *cmd = HI_NULL;
    hi_bool valid = HI_FALSE;

    ret = tde_osi_adj_clip_para(hw_node);
    if (ret != HI_SUCCESS) {
        tde_error("Tde osi adj clip para failed, ret = 0x%x\n", ret);
    }
    valid = tde_query_handle(handle, &handle_mgr);
    if (!valid) {
        tde_error("invalid handle %d!\n", handle);
        return HI_ERR_TDE_INVALID_HANDLE;
    }
    job = (tde_swjob *)handle_mgr->res;
    if (job->submitted) {
        tde_error("job %d already submitted!\n", handle);
        return HI_ERR_TDE_INVALID_HANDLE;
    }
    cmd = (tde_swnode *)tde_malloc(sizeof(tde_swnode));
    if (cmd == HI_NULL) {
        tde_error("malloc failed!\n");
        return HI_ERR_TDE_NO_MEM;
    }
    if (subm_type != TDE_NODE_SUBM_CHILD) {
        job->cmd_num++;
        if (job->cmd_num == 1) {
            job->first_cmd = cmd;

            OSAL_INIT_LIST_HEAD(&cmd->list);
        }
        job->last_cmd = cmd;
    }

    tde_osi_none_cmd(hw_node, cmd, job);
    cmd->handle = job->handle;
    cmd->index = job->cmd_num;
    cmd->subm_type = subm_type;
    cmd->phy_buff_num = work_buf_num;
    *(((hi_u64 *)cmd->node_buf.buf) + 1) = handle;
    osal_list_add_tail(&cmd->list, &job->first_cmd->list); /* Add the cmd to the job list */
    job->tail_node = cmd;
    job->node_num++;
    if (cmd->phy_buff_num != 0) {
        job->aq_use_buff = HI_TRUE;
    }

    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_begin_job
 * Description:   get TDE task handle
 * Return:        created task handle
 */
hi_s32 tde_osi_begin_job(hi_s32 *handle, hi_void *private_data)
{
    if (handle == HI_NULL) {
        tde_error("Null ptr!\n");
        return HI_ERR_TDE_NULL_PTR;
    }

    return tde_osi_list_begin_job(handle, private_data);
}

hi_s32 tde_osi_begin_job_ex(hi_s32 *handle)
{
    if (handle == HI_NULL) {
        tde_error("Null ptr!\n");
        return HI_ERR_TDE_NULL_PTR;
    }

    return tde_osi_list_begin_job(handle, HI_NULL);
}

hi_s32 tde_osi_end_job(drv_tde_end_job_cmd *end_job, tde_func_cb func_compl_cb, hi_void *func_para)
{
#ifndef HI_BUILD_IN_BOOT
    tde_notify_mode noti_type;

    if (((end_job->is_block != HI_TRUE) && (end_job->is_block != HI_FALSE)) ||
        ((end_job->is_sync != HI_TRUE) && (end_job->is_sync != HI_FALSE))) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (end_job->is_block) {
        if (osal_in_interrupt()) {
            tde_error("can not be block in interrupt!\n");
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }

        noti_type = TDE_JOB_WAKE_NOTIFY;
    } else {
        noti_type = TDE_JOB_COMPL_NOTIFY;
    }

#if (TDE_CAPABILITY & SYNC)
    return tde_osi_list_submit_job(end_job, func_compl_cb, func_para, noti_type, end_job->is_sync);
#else
    return tde_osi_list_submit_job(end_job, func_compl_cb, func_para, noti_type);
#endif

#else
    return tde_osi_list_submit_job(end_job, func_compl_cb, func_para, TDE_JOB_WAKE_NOTIFY);
#endif
}

#ifndef HI_BUILD_IN_BOOT

/*
 * Function:      tde_osi_cancel_job
 * Description:   delete created TDE task, only effective for call before endjob
 *                use to release software resource of list of task
 * Input:         handle: task handle
 * Return:        success/fail
 */
hi_s32 tde_osi_cancel_job(hi_s32 handle)
{
    return tde_osi_list_cancel_job(handle);
}

/*
 * Function:      tde_osi_wait_for_done
 * Description:   wait for completion of submit TDE operate
 * Input:         handle: task handle
 * Return:        success/fail
 */
hi_s32 tde_osi_wait_for_done(hi_s32 handle, hi_u32 time_out)
{
    if (osal_in_interrupt()) {
        tde_error("can not be block in interrupt!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return tde_osi_list_wait_for_done(handle, time_out);
}

/*
 * Function:      tde_osi_wait_all_done
 * Description:   wait for all TDE operate completion
 * Return:        success/fail
 */
hi_s32 tde_osi_wait_all_done(hi_bool is_sync)
{
    hi_unused(is_sync);
    if (osal_in_interrupt()) {
        tde_error("can not wait in interrupt!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return tde_osi_list_wait_all_done();
}

/*
 * Function:      tde_osi_quick_copy
 * Description:   quick blit source to target, no any functional operate, the size of source and target are the same
 *                format is not MB format
 */
hi_s32 tde_osi_quick_copy(hi_s32 handle, hi_tde_single_src *single_src)
{
    hi_tde_double_src double_src;

    double_src.bg_surface  = HI_NULL;
    double_src.bg_rect  = HI_NULL;
    double_src.fg_surface = single_src->src_surface;
    double_src.fg_rect = single_src->src_rect;
    double_src.dst_surface = single_src->dst_surface;
    double_src.dst_rect = single_src->dst_rect;

    return tde_osi_blit(handle, &double_src, HI_NULL);
}

hi_void tde_set_resize_filter(hi_bool is_resize_filter)
{
    g_is_resize_filter = is_resize_filter;
}

#if (TDE_CAPABILITY & RESIZE)
/*
 * Function:      tde_osi_quick_resize
 * Description:   zoom the size of source bitmap to the size aasigned by target bitmap,
                  of which source and target can be the same
 * Input:         src_surface: source bitmap info struct
 *                dst_surface: target bitmap info struct
 * Others:        add support for YCbCr422
 */
hi_s32 tde_osi_quick_resize(hi_s32 handle, hi_tde_surface *src_surface, hi_tde_rect *src_rect,
                            hi_tde_surface *dst_surface, hi_tde_rect *dst_rect)
{
    hi_tde_opt option = { 0 };
    hi_tde_double_src double_src;
    option.resize = HI_TRUE;

    if (g_is_resize_filter) {
        option.filter_mode = HI_TDE_FILTER_MODE_COLOR;
    } else {
        option.filter_mode = HI_TDE_FILTER_MODE_NONE;
    }
    if (src_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    double_src.bg_surface  = HI_NULL;
    double_src.bg_rect  = HI_NULL;
    double_src.fg_surface = src_surface;
    double_src.fg_rect = src_rect;
    double_src.dst_surface = dst_surface;
    double_src.dst_rect = dst_rect;
    return tde_osi_blit(handle, &double_src, &option);
}
#endif

#if (TDE_CAPABILITY & ROTATE)
static hi_s32 tde_osi_quick_rotate_set(tde_hw_node *hw_node, hi_tde_single_src *single_src,
                                       hi_tde_rotate_angle rotate_angle)
{
    tde_scandirection_mode src_scan_info = {0};
    tde_scandirection_mode dst_scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};

    if (tde_hal_node_set_base_operate(hw_node, TDE_NORM_BLIT_1OPT, TDE_ALU_NONE, 0) < 0) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    src_scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    src_scan_info.v_scan = TDE_SCAN_UP_DOWN;
    dst_scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    dst_scan_info.v_scan = TDE_SCAN_UP_DOWN;
    if (rotate_angle == HI_TDE_ROTATE_CLOCKWISE_180) {
        src_scan_info.h_scan = TDE_SCAN_RIGHT_LEFT;
        src_scan_info.v_scan = TDE_SCAN_DOWN_UP;
    }

    tde_osi_convert_surface(single_src->src_surface, single_src->src_rect, &src_scan_info, &src_drv_surface);

    tde_hal_node_set_src2(hw_node, &src_drv_surface);

    tde_osi_convert_surface(single_src->dst_surface, single_src->dst_rect, &dst_scan_info, &dst_drv_surface);

    tde_hal_node_set_tqt(hw_node, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_NORM);

    if (rotate_angle != HI_TDE_ROTATE_CLOCKWISE_180) {
        tde_hal_node_set_rotate(hw_node, rotate_angle);
    }
    return HI_SUCCESS;
}

hi_s32 tde_osi_quick_rotate(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_rotate_angle rotate_angle)
{
    hi_s32 ret;
    tde_hw_node *hw_node = HI_NULL;
    hi_tde_double_src double_src = {0};

    if ((single_src == HI_NULL) || (single_src->src_surface == HI_NULL) || (single_src->src_rect == HI_NULL) ||
        (single_src->dst_surface == HI_NULL) || (single_src->dst_rect == HI_NULL)) {
        return HI_ERR_TDE_NULL_PTR;
    }
    if ((single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
        (tde_osi_check_surface(single_src->src_surface, single_src->src_rect) != HI_SUCCESS) ||
        (tde_osi_check_surface(single_src->dst_surface, single_src->dst_rect) != HI_SUCCESS) ||
        (tde_osi_check_rotate_para(single_src, rotate_angle) < 0)) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_hal_node_init_nd(&hw_node) < 0) {
        return HI_ERR_TDE_NO_MEM;
    }

    /*
     * set:
     * set opt
     * set src2
     * set tqt
     * set rotate
     */
    ret = tde_osi_quick_rotate_set(hw_node, single_src, rotate_angle);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    /* set filter node */
    if ((rotate_angle == HI_TDE_ROTATE_CLOCKWISE_180) && (single_src->src_surface->color_fmt >=
        HI_TDE_COLOR_FMT_YCBCR422)) {
        double_src.bg_surface = HI_NULL;
        double_src.bg_rect = HI_NULL;
        double_src.fg_surface = single_src->src_surface;
        double_src.fg_rect = single_src->src_rect;
        double_src.dst_surface = single_src->dst_surface;
        double_src.dst_rect = single_src->dst_rect;
        ret = tde_osi_set_filter_node(handle, hw_node, &double_src, HI_TDE_DEFLICKER_LEVEL_MODE_NONE,
                                      HI_TDE_FILTER_MODE_NONE);
        if (ret < 0) {
            tde_hal_free_node_buf(hw_node);
            return ret;
        }
#if (TDE_CAPABILITY & SLICE)
        tde_hal_free_node_buf(hw_node);
#endif
        return HI_SUCCESS;
    }

    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret < 0) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    return HI_SUCCESS;
}
#endif

#if (TDE_CAPABILITY & DEFLICKER)
/*
 * Function:      tde_osi_quick_flicker
 * Description:   deflicker source bitmap,output to target bitmap,source and target can be the same
 * Input:         src_surface: source bitmap info struct
 *                dst_surface: terget bitmap info struct
 */
hi_s32 tde_osi_quick_flicker(hi_s32 handle, hi_tde_surface *src_surface, hi_tde_rect *src_rect,
                             hi_tde_surface *dst_surface, hi_tde_rect *dst_rect)
{
    hi_s32 ret;
    hi_tde_opt option = { 0 };
    hi_tde_double_src double_src;

    option.deflicker_mode = HI_TDE_DEFLICKER_LEVEL_MODE_BOTH;
    option.resize = HI_TRUE;
    if (src_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    double_src.bg_surface  = HI_NULL;
    double_src.bg_rect  = HI_NULL;
    double_src.fg_surface = src_surface;
    double_src.fg_rect = src_rect;
    double_src.dst_surface = dst_surface;
    double_src.dst_rect = dst_rect;
    ret = tde_osi_blit(handle, &double_src, &option);
    if (ret < 0) {
        return ret;
    }
    return HI_SUCCESS;
}
#endif

#endif

/*
 * Function:      tde_osi_blit
 * Description:   operate pstBackGround with pstForeGround,which result output to dst_surface,
                  operate setting is in pstOpt
 * Input:         opt:  operate parameter setting struct
 */
hi_s32 tde_osi_blit(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_opt *opt)
{
    tde_operation_category opt_category;
    hi_s32 ret;
    hi_tde_single_src single_src;

    opt_category = tde_osi_get_opt_category(double_src, opt);
    switch (opt_category) {
#ifndef HI_BUILD_IN_BOOT
        case TDE_OPERATION_SINGLE_SRC1:
            if (double_src->bg_surface == HI_NULL) {
                single_src.src_surface = double_src->fg_surface;
                single_src.src_rect = double_src->fg_rect;
            } else {
                single_src.src_surface = double_src->bg_surface;
                single_src.src_rect = double_src->bg_rect;
            }
            single_src.dst_surface = double_src->dst_surface;
            single_src.dst_rect = double_src->dst_rect;
            return tde_osi_single_src_1_blit(handle, &single_src, HI_FALSE, HI_FALSE);
            break;
        case TDE_OPERATION_SINGLE_SRC2:
            if (double_src->bg_surface == HI_NULL) {
                single_src.src_surface = double_src->fg_surface;
                single_src.src_rect = double_src->fg_rect;
            } else {
                single_src.src_surface = double_src->bg_surface;
                single_src.src_rect = double_src->bg_rect;
            }
            single_src.dst_surface = double_src->dst_surface;
            single_src.dst_rect = double_src->dst_rect;
            ret = tde_osi_single_src_2_blit(handle, &single_src, opt, HI_FALSE, HI_FALSE);
            if (ret < 0) {
                return ret;
            }
            break;
#endif
        case TDE_OPERATION_DOUBLE_SRC:
            ret = tde_osi_double_src_2_blit(handle, double_src, opt);
            if (ret < 0) {
                return ret;
            }
            break;
        default:
            return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_quick_fill
 * Description:   quick fill fixed value to target bitmap, fill value is referred to target bitmap
 * Input:         dst_surface: target bitmap info struct
 *                fill_data: fill value
 *                pFuncComplCB: callback function pointer when operate is over;if null, to say to no need to notice
 */
hi_s32 tde_osi_quick_fill(hi_s32 handle, hi_tde_surface *dst_surface, hi_tde_rect *dst_rect, hi_u32 fill_data)
{
    hi_tde_fill_color fill_color;

    fill_color.color_fmt = dst_surface->color_fmt;
    fill_color.color_value = fill_data;

    return tde_osi_1_source_fill(handle, dst_surface, dst_rect, &fill_color, HI_NULL);
}

#ifndef HI_BUILD_IN_BOOT
/*
 * Function:      tde_osi_single_src_1_blit
 * Description:   source1 operate realization
 * Input:         dst_surface: target bitmap info struct
 *                dst_rect: target bitmap operate zone
 * Return:        success/fail
 */
static hi_s32 tde_osi_single_src_1_blit(hi_s32 handle, hi_tde_single_src *single_src,
                                        hi_bool mmz_for_src, hi_bool mmz_for_dst)
{
    hi_s32 ret;
    tde_hw_node *hw_node = HI_NULL;
    tde_surface_msg src_drv_surface = { 0 };
    tde_surface_msg dst_drv_surface = { 0 };
    tde_scandirection_mode src_scan_info = { 0 };
    tde_scandirection_mode dst_scan_info = { 0 };

    if ((single_src->src_surface == HI_NULL) || (single_src->src_rect == HI_NULL) ||
        (single_src->dst_surface == HI_NULL) || (single_src->dst_rect == HI_NULL)) {
        return HI_ERR_TDE_NULL_PTR;
    }

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    if ((single_src->src_surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) ||
        (single_src->src_surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("This operation doesn't support PKG!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#endif

    if (single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    tde_unify_rect(single_src->src_rect, single_src->dst_rect);

    if (tde_hal_node_init_nd(&hw_node) < 0) {
        return HI_ERR_TDE_NO_MEM;
    }
    if (tde_hal_node_set_base_operate(hw_node, TDE_QUIKE_COPY, TDE_SRC1_BYPASS, 0) < 0) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if (tde_osi_get_scan_info_ex(single_src, HI_NULL, &src_scan_info, &dst_scan_info) < 0) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }

    tde_osi_convert_surface(single_src->src_surface, single_src->src_rect, &src_scan_info, &src_drv_surface);
    src_drv_surface.cma = mmz_for_src;

    tde_hal_node_set_src1(hw_node, &src_drv_surface);

    tde_osi_set_ext_alpha(single_src->src_surface, HI_NULL, hw_node);

    tde_osi_convert_surface(single_src->dst_surface, single_src->dst_rect, &dst_scan_info, &dst_drv_surface);
    dst_drv_surface.cma = mmz_for_dst;

    tde_hal_node_set_tqt(hw_node, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_NORM);
    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret < 0) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    return HI_SUCCESS;
}
#endif

static hi_s32 tde_osi_single_para(hi_tde_single_src *single_src, hi_tde_opt *opt, tde_hw_node **hw_node,
    tde_scandirection_mode *src_scan_info, tde_scandirection_mode *dst_scan_info)
{
    hi_s32 ret;
    hi_tde_double_src double_src;

#if (TDE_CAPABILITY & COMPRESS)
    hi_bool compress_or_decompress = HI_FALSE;
#endif
    ret = tde_osi_check_single_src_to_para(single_src->src_surface, single_src->src_rect,
                                           single_src->dst_surface, single_src->dst_rect, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (tde_hal_node_init_nd(hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    double_src.bg_surface = HI_NULL;
    double_src.bg_rect = HI_NULL;
    double_src.fg_surface = single_src->src_surface;
    double_src.fg_rect = single_src->src_rect;
    double_src.dst_surface = single_src->dst_surface;
    double_src.dst_rect = single_src->dst_rect;
    if (tde_osi_set_clip_para(&double_src, opt, *hw_node) != HI_SUCCESS) {
        tde_hal_free_node_buf(*hw_node);
        return HI_ERR_TDE_CLIP_AREA;
    }
    if (tde_osi_get_scan_info_ex(single_src, opt, src_scan_info, dst_scan_info) != HI_SUCCESS) {
        tde_hal_free_node_buf(*hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }
#if (TDE_CAPABILITY & COMPRESS)
    compress_or_decompress = ((opt->is_compress) || (opt->is_decompress));
    if (compress_or_decompress) {
        src_scan_info->h_scan = TDE_SCAN_LEFT_RIGHT;
        src_scan_info->v_scan = TDE_SCAN_UP_DOWN;
        dst_scan_info->h_scan = TDE_SCAN_LEFT_RIGHT;
        dst_scan_info->v_scan = TDE_SCAN_UP_DOWN;
    }
#endif
    return HI_SUCCESS;
}

static hi_s32 tde_osi_single_opt(hi_tde_opt *opt, hi_tde_single_src *single_src, tde_surface_msg *drv_surface,
    tde_hw_node *hw_node)
{
    hi_u16 code;
    tde_conv_mode_cmd conv = { 0 };
    hi_s32 ret;
    tde_clut_usage clut_usage = TDE_CLUT_USAGE_BUTT;

#if (TDE_CAPABILITY & COMPRESS)
    if (opt->is_compress) {
        tde_hal_node_set_compress_tqt(hw_node, drv_surface, opt->out_alpha_from);
        tde_hal_node_set_compress(hw_node);
    } else {
        tde_hal_node_set_tqt(hw_node, drv_surface, opt->out_alpha_from);
    }
#else
    tde_hal_node_set_tqt(hw_node, drv_surface, opt->out_alpha_from);
#endif
    code = tde_osi_single_src_to_get_opt_code(single_src->src_surface->color_fmt,
                                              single_src->dst_surface->color_fmt);

    tde_osi_get_conv_by_code(code, &conv);

    if (tde_hal_node_set_color_convert(hw_node, &conv) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_free((hi_void *)drv_surface);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = tde_osi_set_clut_opt(single_src->src_surface, single_src->dst_surface, &clut_usage,
                               opt->clut_reload, hw_node);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_free((hi_void *)drv_surface);
        return ret;
    }

    ret = tde_osi_set_base_opt_para_for_blit(opt, HI_NULL, single_src->src_surface,
        TDE_OPERATION_SINGLE_SRC2, hw_node);
    if (ret != HI_SUCCESS) {
        tde_error("Set base opt para for blit failed, ret = 0x%x\n", ret);
    }
    tde_osi_set_ext_alpha(HI_NULL, single_src->src_surface, hw_node);
    return HI_SUCCESS;
}

static hi_s32 tde_osi_single_node(hi_s32 handle, hi_tde_opt *opt, hi_tde_single_src *single_src,
    tde_surface_msg *drv_surface, tde_hw_node *hw_node)
{
    hi_bool set_filter_node = HI_FALSE;
    hi_tde_double_src double_src;
    hi_s32 ret;

    set_filter_node = ((opt->resize) || (opt->deflicker_mode != HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ||
                       (single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422));
    if (set_filter_node) {
#if (TDE_CAPABILITY & SLICE)
        if ((single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422) && (!(opt->resize))) {
            single_src->src_rect->height = single_src->dst_rect->height;
            single_src->src_rect->width = single_src->dst_rect->width;
        }
#endif
        double_src.bg_surface = HI_NULL;
        double_src.bg_rect = HI_NULL;
        double_src.fg_surface = single_src->src_surface;
        double_src.fg_rect = single_src->src_rect;
        double_src.dst_surface = single_src->dst_surface;
        double_src.dst_rect = single_src->dst_rect;
        ret = tde_osi_set_filter_node(handle, hw_node, &double_src, opt->deflicker_mode,
                                      opt->filter_mode);
        if (ret != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            tde_free((hi_void *)drv_surface);
            return ret;
        }
#if (TDE_CAPABILITY & SLICE)
        tde_hal_free_node_buf(hw_node);
#endif
        tde_free((hi_void *)drv_surface);
        return HI_SUCCESS;
    }
    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_free((hi_void *)drv_surface);
        return ret;
    }
    tde_free((hi_void *)drv_surface);
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_single_src_2_blit
 * Description:   source2 operate realization
 * Input:         dst_surface: target bitmap info struct
 *                dst_rect: target bitmap operate zone
 * Return:        success/fail
 */
static hi_s32 tde_osi_single_src_2_blit(hi_s32 handle, hi_tde_single_src *single_src,
                                        hi_tde_opt *opt, hi_bool mmz_for_src, hi_bool mmz_for_dst)
{
    tde_hw_node *hw_node = HI_NULL;
    tde_surface_msg *drv_surface = HI_NULL;
    hi_s32 ret;
    tde_scandirection_mode src_scan_info = { 0 };
    tde_scandirection_mode dst_scan_info = { 0 };

    ret = tde_osi_single_para(single_src, opt, &hw_node, &src_scan_info, &dst_scan_info);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    drv_surface = (tde_surface_msg *)tde_malloc(sizeof(tde_surface_msg));
    if (drv_surface == HI_NULL) {
        tde_error ("malloc pstDrvSurface failed, size=%ld!\n", (unsigned long)(sizeof(tde_surface_msg)));
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_NO_MEM;
    }
    tde_osi_convert_surface(single_src->src_surface, single_src->src_rect, &src_scan_info, drv_surface);
    drv_surface->cma = mmz_for_src;

#if (TDE_CAPABILITY & COMPRESS)
    (opt->is_decompress) ? tde_hal_node_set_src_to_decompress(hw_node, drv_surface) :
    tde_hal_node_set_src2(hw_node, drv_surface);
#else
    tde_hal_node_set_src2(hw_node, drv_surface);
#endif

    tde_osi_convert_surface(single_src->dst_surface, single_src->dst_rect, &dst_scan_info, drv_surface);
    if (opt->out_alpha_from >= HI_TDE_OUT_ALPHA_FROM_MAX) {
        tde_error("out_alpha_from error!\n");
        tde_hal_free_node_buf(hw_node);
        tde_free((hi_void *)drv_surface);
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->out_alpha_from == HI_TDE_OUT_ALPHA_FROM_BACKGROUND) {
        tde_hal_free_node_buf(hw_node);
        tde_free((hi_void *)drv_surface);
        tde_error("out_alpha_from error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    drv_surface->cma = mmz_for_dst;
    ret = tde_osi_single_opt(opt, single_src, drv_surface, hw_node);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = tde_osi_single_node(handle, opt, single_src, drv_surface, hw_node);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_single_src_to_para(hi_tde_surface *fore_ground, hi_tde_rect *fore_ground_rect,
                                               hi_tde_surface *dst_surface, hi_tde_rect *dst_rect,
                                               hi_tde_opt *opt)
{
    hi_bool null_ptr = HI_FALSE;

    if (opt == HI_NULL) {
        tde_error("pstOpt is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    null_ptr = ((dst_surface == HI_NULL) || (dst_rect == HI_NULL) || ((opt == HI_NULL)) ||
                (fore_ground == HI_NULL) || (fore_ground_rect == HI_NULL));

    if (null_ptr) {
        return HI_ERR_TDE_NULL_PTR;
    }

    if (tde_osi_check_single_src_to_opt(fore_ground->color_fmt, dst_surface->color_fmt, opt) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->resize) {
        if (tde_osi_check_resize_para(fore_ground_rect->width, fore_ground_rect->height,
                                      dst_rect->width, dst_rect->height) != HI_SUCCESS) {
            return HI_ERR_TDE_MINIFICATION;
        }
    }
#if TDE_CAPABILITY & COMPRESS
    if (opt->is_compress) {
        if (tde_osi_check_compress_para(fore_ground, fore_ground_rect, dst_surface, dst_rect, opt) !=
            HI_SUCCESS) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }

    if (opt->is_decompress) {
        if (tde_osi_check_decompress_para(fore_ground, fore_ground_rect, dst_surface, dst_rect, opt) !=
            HI_SUCCESS) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
#endif

    return HI_SUCCESS;
}

static hi_s32 tde_osi_double_para(hi_tde_double_src *double_src, hi_tde_opt *opt, tde_hw_node **hw_node)
{
    hi_bool null_ptr = ((double_src->bg_surface == HI_NULL) || ((double_src->bg_rect == HI_NULL)) ||
                        (double_src->fg_surface == HI_NULL) || ((double_src->fg_rect == HI_NULL)) ||
                        (double_src->dst_surface == HI_NULL) || ((double_src->dst_rect == HI_NULL)) ||
                        (opt == HI_NULL));

    if (null_ptr) {
        return HI_ERR_TDE_NULL_PTR;
    }

    if (tde_osi_check_double_src_opt(double_src->bg_surface->color_fmt, double_src->fg_surface->color_fmt,
                                     double_src->dst_surface->color_fmt, opt) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }
    if (opt->resize) {
        if (tde_osi_check_resize_para(double_src->fg_rect->width, double_src->fg_rect->height,
                                      double_src->dst_rect->width, double_src->dst_rect->height) != HI_SUCCESS) {
            return HI_ERR_TDE_MINIFICATION;
        }
    }

#if (TDE_CAPABILITY & COMPRESS)
    if (opt->is_compress) {
        if (tde_osi_check_compress_para(double_src->fg_surface, double_src->fg_rect,
                                        double_src->dst_surface, double_src->dst_rect, opt) < 0) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }

    if (opt->is_decompress) {
        if (tde_osi_check_decompress_para(double_src->fg_surface, double_src->fg_rect,
                                          double_src->dst_surface, double_src->dst_rect, opt) < 0) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
#endif

    if (tde_hal_node_init_nd(hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    if (tde_osi_set_clip_para(double_src, opt, *hw_node) != HI_SUCCESS) {
        tde_hal_free_node_buf(*hw_node);
        return HI_ERR_TDE_CLIP_AREA;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_double_opt(hi_tde_double_src *double_src, hi_tde_opt *opt, tde_hw_node *hw_node,
    tde_surface_msg *drv_surface)
{
    hi_s32 ret;
    hi_u16 code;
    tde_conv_mode_cmd conv = { 0 };
    tde_clut_usage clut_usage = TDE_CLUT_USAGE_BUTT;

    if (opt->out_alpha_from >= HI_TDE_OUT_ALPHA_FROM_MAX) {
        tde_error("out_alpha_from error!\n");
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }

#if (TDE_CAPABILITY & COMPRESS)
    if (opt->is_compress) {
        tde_hal_node_set_compress_tqt(hw_node, drv_surface, opt->out_alpha_from);
        tde_hal_node_set_compress(hw_node);
    } else {
        tde_hal_node_set_tqt(hw_node, drv_surface, opt->out_alpha_from);
    }
#else
    tde_hal_node_set_tqt(hw_node, drv_surface, opt->out_alpha_from);
#endif

    code = tde_osi_double_src_get_opt_code(double_src->bg_surface->color_fmt,
        double_src->fg_surface->color_fmt, double_src->dst_surface->color_fmt);

    tde_osi_get_conv_by_code(code, &conv);

    if (tde_hal_node_set_color_convert(hw_node, &conv) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = tde_osi_set_clut_opt(double_src->fg_surface, double_src->dst_surface,
                               &clut_usage, opt->clut_reload, hw_node);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    ret = tde_osi_set_color_key(double_src, hw_node,
        opt->color_key_value, opt->color_key_mode, clut_usage);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    ret = tde_osi_set_base_opt_para_for_blit(opt, double_src->bg_surface, double_src->fg_surface,
        TDE_OPERATION_DOUBLE_SRC, hw_node);
    if (ret != HI_SUCCESS) {
        tde_error("Set base opt para for blit failed, ret = 0x%x\n", ret);
    }
    tde_osi_set_ext_alpha(double_src->bg_surface, double_src->fg_surface, hw_node);
    return HI_SUCCESS;
}

static hi_s32 tde_osi_double_node(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_opt *opt,
    tde_hw_node *hw_node)
{
    hi_s32 ret;
    hi_bool set_filter_node = HI_FALSE;

#if (TDE_CAPABILITY & SLICE)
    set_filter_node = ((opt->resize) || ((opt->deflicker_mode != HI_TDE_DEFLICKER_LEVEL_MODE_NONE)) ||
                       (double_src->fg_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422) ||
                       (double_src->bg_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422));
#else
    set_filter_node = ((opt->resize) || (opt->deflicker_mode != HI_TDE_DEFLICKER_LEVEL_MODE_NONE) ||
                       (double_src->fg_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422));
#endif
    if (set_filter_node) {
#if (TDE_CAPABILITY & SLICE)
        double_src->bg_rect->height = double_src->dst_rect->height;
        double_src->bg_rect->width = double_src->dst_rect->width;

        if ((double_src->fg_surface->color_fmt >= HI_TDE_COLOR_FMT_YCBCR422) && (!(opt->resize))) {
            double_src->fg_rect->height = double_src->dst_rect->height;
            double_src->fg_rect->width = double_src->dst_rect->width;
        }
        ret = tde_osi_set_filter_node(handle, hw_node, double_src,
                                      opt->deflicker_mode, opt->filter_mode);
        if (ret != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            return ret;
        }
        tde_hal_free_node_buf(hw_node);
#else
        ret = tde_osi_set_filter_node(handle, hw_node, double_src, opt->deflicker_mode, opt->filter_mode);
        if (ret != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            return ret;
        }
#endif
        return HI_SUCCESS;
    }
    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_double_src_2_blit
 * Description:   dual source operate realization
 * Input:         double_src:  bitmap info struct
 *                opt: config parameter
 * Return:        success/fail
 * Others:        add support for YCbCr422
 */
static hi_s32 tde_osi_double_src_2_blit(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_opt *opt)
{
    tde_hw_node *hw_node = HI_NULL;
    tde_scandirection_mode src_scan_info = { 0 };
    tde_scandirection_mode dst_scan_info = { 0 };
    tde_surface_msg drv_surface = { 0 };
    hi_s32 ret;
    hi_tde_single_src single_src;

    ret = tde_osi_double_para(double_src, opt, &hw_node);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    single_src.src_surface = double_src->fg_surface;
    single_src.src_rect = double_src->fg_rect;
    single_src.dst_surface = double_src->dst_surface;
    single_src.dst_rect = double_src->dst_rect;
    if (tde_osi_get_scan_info_ex(&single_src, opt, &src_scan_info, &dst_scan_info) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_INVALID_PARA;
    }

#if (TDE_CAPABILITY & COMPRESS)
    if ((opt->is_compress) || (opt->is_decompress)) {
        src_scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
        src_scan_info.v_scan = TDE_SCAN_UP_DOWN;
        dst_scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
        dst_scan_info.v_scan = TDE_SCAN_UP_DOWN;
    }
#endif

    tde_osi_convert_surface(double_src->bg_surface, double_src->bg_rect, &src_scan_info, &drv_surface);

    tde_hal_node_set_src1(hw_node, &drv_surface);

    tde_osi_convert_surface(double_src->fg_surface, double_src->fg_rect, &src_scan_info, &drv_surface);

    tde_hal_node_set_src2(hw_node, &drv_surface);

    tde_osi_convert_surface(double_src->dst_surface, double_src->dst_rect, &dst_scan_info, &drv_surface);
    ret = tde_osi_double_opt(double_src, opt, hw_node, &drv_surface);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = tde_osi_double_node(handle, double_src, opt, hw_node);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_set_color_key(hi_tde_double_src *double_src,
                                    tde_hw_node *hw_node, hi_tde_color_key color_key_value,
                                    hi_tde_color_key_mode color_key_mode,
                                    tde_clut_usage clut_usage)
{
    tde_color_key_cmd color_key;
    tde_colorfmt_category fmt_category;
    hi_bool unknown_fmt_category = HI_FALSE;
    hi_bool color_key_foreground_before_clut_mode = (clut_usage != TDE_CLUT_COLOREXPENDING) &&
                                                     (clut_usage != TDE_CLUT_CLUT_BYPASS);
    hi_bool set_color_key = (color_key_mode != HI_TDE_COLOR_KEY_MODE_NONE);

    color_key.color_key_value = color_key_value;

    if (!set_color_key) {
        return HI_SUCCESS;
    }

    switch (color_key_mode) {
        case HI_TDE_COLOR_KEY_MODE_BACKGROUND:
            color_key.color_key_mode = TDE_DRV_COLORKEY_BACKGROUND;
            fmt_category = tde_osi_get_fmt_category(double_src->bg_surface->color_fmt);
            break;
        case HI_TDE_COLOR_KEY_MODE_FOREGROUND:
            color_key.color_key_mode = (color_key_foreground_before_clut_mode) ?
                TDE_DRV_COLORKEY_FOREGROUND_AFTER_CLUT : TDE_DRV_COLORKEY_FOREGROUND_BEFORE_CLUT;
            fmt_category = tde_osi_get_fmt_category(double_src->fg_surface->color_fmt);
            break;
        default:
            tde_error("invalid ColorKeyMode!\n");
            return HI_ERR_TDE_INVALID_PARA;
    }

    unknown_fmt_category = fmt_category >= TDE_COLORFMT_CATEGORY_BUTT;

    if (unknown_fmt_category) {
        tde_error("Unknown fmt category!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_hal_node_set_colorkey(hw_node, fmt_category, &color_key) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return HI_SUCCESS;
}
#endif

static hi_s32 tde_osi_mb_val(hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt, hi_u32 *cb_cr_height,
    hi_u32 *byte_per_pixel, hi_u32 *cb_cr_byte_per_pixel)
{
    hi_s32 ret, bpp;

    ret = tde_osi_check_mb_blit_para(mb_src, mb_opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if ((mb_src->dst_rect->height == mb_src->src_rect->height) &&
        (mb_src->dst_rect->width == mb_src->src_rect->width)) {
        mb_opt->resize_en = HI_TDE_MB_RESIZE_NONE;
    }

    if (mb_opt->resize_en == HI_TDE_MB_RESIZE_NONE) {
        tde_unify_rect(mb_src->src_rect, mb_src->dst_rect);
    }

    if (tde_osi_check_resize_para(mb_src->src_rect->width, mb_src->src_rect->height,
                                  mb_src->dst_rect->width, mb_src->dst_rect->height) != HI_SUCCESS) {
        tde_error("The Scale is too large!\n");
        return HI_ERR_TDE_MINIFICATION;
    }

    *cb_cr_height = TDE_MAX_SLICE_RECT_HEIGHT;
    if ((mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP1_YCBCR420MBP) ||
        (mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBP) ||
        (mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBI) ||
        (mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR420MBP) ||
        (mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR422MBVP)) {
        *cb_cr_height = TDE_MAX_SLICE_RECT_HEIGHT / 2; /* 2 half height */
    }

    *cb_cr_byte_per_pixel = ((mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR444MBP) ||
        (mb_src->mb_surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR422MBVP)) ? 2 : 1; /* 2 1 byte per pixel */

    bpp = tde_osi_get_bpp_by_fmt(mb_src->dst_surface->color_fmt);
    *byte_per_pixel = ((hi_u32)bpp >> 3); /* 3 bpp data */
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_mb_blit
 * Description:   Mb blit
 * Return:        >0: return task id of current operate; <0: fail
 */
hi_s32 tde_osi_mb_blit(hi_s32 handle, hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt)
{
    hi_u32 height, width, i, j, m, n, byte_per_pixel, cbcr_pixel;
    hi_u32 cb_cr_height = 0;
    static hi_u64 u64phy, yphy_addr, cb_crphy_addr;
    hi_s32 ret = tde_osi_mb_val(mb_src, mb_opt, &cb_cr_height, &byte_per_pixel, &cbcr_pixel);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    drv_tde_mb_src_val(mb_src, height, width);
    drv_tde_mb_src_addr(mb_src, yphy_addr, cb_crphy_addr, u64phy);
    ret = tde_osi_get_double_cycle_data(mb_src->src_rect, &i, &j);
    if (ret != HI_SUCCESS) {
        tde_error("Tde osi Get double cycle data failed, ret = 0x%x\n", ret);
    }
    for (n = 0; n < j; n++) {
        for (m = 0; m < i; m++) {
            if ((i - 1) == m) {
                mb_src->src_rect->width = width - m * TDE_MAX_SLICE_RECT_WIDTH;
            } else {
                mb_src->src_rect->width = TDE_MAX_SLICE_RECT_WIDTH;
            }

            if (height > TDE_MAX_SLICE_RECT_HEIGHT) {
                mb_src->src_rect->height = TDE_MAX_SLICE_RECT_HEIGHT;
            }

            if ((j - 1) == n) {
                mb_src->src_rect->height = height - n * TDE_MAX_SLICE_RECT_HEIGHT;
            } else {
                mb_src->src_rect->height = TDE_MAX_SLICE_RECT_HEIGHT;
            }
            if (!((i == 1) && (j == 1))) {
                mb_src->dst_rect->width = mb_src->src_rect->width;
                mb_src->dst_rect->height = mb_src->src_rect->height;
            }
            mb_src->mb_surface->y_addr = yphy_addr + (hi_u64)m * TDE_MAX_SLICE_RECT_WIDTH +
                (hi_u64)n * (hi_u64)(mb_src->mb_surface->y_stride) * TDE_MAX_SLICE_RECT_HEIGHT;
            mb_src->dst_surface->phy_addr = u64phy + (hi_u64)m * (hi_u64)byte_per_pixel * TDE_MAX_SLICE_RECT_WIDTH +
                (hi_u64)n * (hi_u64)(mb_src->dst_surface->stride) * TDE_MAX_SLICE_RECT_HEIGHT;
            mb_src->mb_surface->cbcr_phy_addr = cb_crphy_addr + (hi_u64)m *
                (hi_u64)cbcr_pixel * TDE_MAX_SLICE_RECT_WIDTH +
                (hi_u64)n * (hi_u64)(mb_src->mb_surface->cbcr_stride) * (hi_u64)(cb_cr_height);
            ret = tde_osi_set_mb_para(handle, mb_src, mb_opt);
            if (ret != HI_SUCCESS) {
                return ret;
            }
        }
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_surface_ex(hi_tde_surface *surface, hi_tde_rect *rect)
{
    hi_bool invalid_operation_area = HI_FALSE;
    hi_s32 ret = tde_osi_pre_check_surface_ex(surface, rect);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    invalid_operation_area = ((rect->width > TDE_MAX_RECT_WIDTH_EX) ||
                              (rect->height > TDE_MAX_RECT_HEIGHT_EX));

    if (invalid_operation_area) {
        tde_error("invalid operation SurfaceEX area!width=%d,height=%d\n", rect->width, rect->height);
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_osi_check_mb_src_opt(hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt)
{
    if (mb_src->dst_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (mb_src->mb_surface->mb_color_fmt >= HI_TDE_MB_COLOR_FMT_MAX) {
        tde_error("mb color format error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (((mb_opt->is_deflicker != HI_TRUE) && (mb_opt->is_deflicker != HI_FALSE)) ||
        ((mb_opt->is_set_out_alpha != HI_TRUE) && (mb_opt->is_set_out_alpha != HI_FALSE))) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (mb_opt->resize_en >= HI_TDE_MB_RESIZE_MAX) {
        tde_error("enMBResize error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}
#endif

#ifdef  CONFIG_DRM_HISI_HISMART
static hi_s32 tde_osi_pre_check_mb_drm_dumb(hi_tde_mb_surface *surface, hi_u32 size)
{
    hi_s32 ret;

    ret = drm_check_dumb_phy_addr(surface->y_addr, size);
    if (ret != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (surface->cbcr_phy_addr != 0) {
        size = surface->cbcr_stride * surface->y_height;
        if ((surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR422MBVP)  ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP1_YCBCR420MBP) ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBP)   ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBI) ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR420MBP)) {
            size = surface->cbcr_stride * (surface->y_height / 2); /* 2 half height */
        }
        ret = drm_check_dumb_phy_addr(surface->cbcr_phy_addr, size);
        if (ret != HI_SUCCESS) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    return HI_SUCCESS;
}
#endif

static hi_s32 tde_osi_pre_check_mb_mmz(hi_tde_mb_surface *surface, hi_u32 size)
{
    hi_s32 ret;

#ifdef  CONFIG_DRM_HISI_HISMART
    ret = tde_osi_pre_check_mb_drm_dumb(surface, size);
    if (ret == HI_SUCCESS) {
        return HI_SUCCESS;
    }
#endif
    ret = cmpi_check_mmz_phy_addr(surface->y_addr, size);
    if (ret != HI_SUCCESS) {
        tde_error("Check MB surface phy_addr addr failed!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (surface->cbcr_phy_addr != 0) {
        size = surface->cbcr_stride * surface->y_height;
        if ((surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR422MBVP)  ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP1_YCBCR420MBP) ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBP)   ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_MP2_YCBCR420MBI) ||
            (surface->mb_color_fmt == HI_TDE_MB_COLOR_FMT_JPG_YCBCR420MBP)) {
            size = surface->cbcr_stride * (surface->y_height / 2); /* 2 half height */
        }
        ret = cmpi_check_mmz_phy_addr(surface->cbcr_phy_addr, size);
        if (ret != HI_SUCCESS) {
            tde_error("Check MB surface cbcr_phy_addr addr failed!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_mb_blit_para(hi_tde_mb_src *mb_src, hi_tde_mb_opt *mb_opt)
{
    hi_u32 size;
    hi_s32 ret;
    hi_bool null_ptr = ((mb_src->mb_surface == HI_NULL) || ((mb_src->src_rect == HI_NULL)) ||
                        (mb_src->dst_surface == HI_NULL) || (mb_src->dst_rect == HI_NULL) || (mb_opt == HI_NULL));

    if (null_ptr) {
        tde_error("Contains HI_NULL ptr!\n");
        return HI_ERR_TDE_NULL_PTR;
    }

#ifndef HI_BUILD_IN_BOOT
    ret = tde_osi_check_mb_src_opt(mb_src, mb_opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    if ((tde_osi_check_surface_ex(mb_src->dst_surface, mb_src->dst_rect) != HI_SUCCESS) ||
        (tde_osi_check_mb_surface_ex(mb_src->mb_surface, mb_src->src_rect) != HI_SUCCESS)) {
        return HI_ERR_TDE_INVALID_PARA;
    }
    /* CMPI_CheckMmzphy_addr Call a semaphore, Cannot be used in the interrupt */
    if (!osal_in_interrupt()) {
        size = mb_src->mb_surface->y_stride * mb_src->mb_surface->y_height;
        ret = tde_osi_pre_check_mb_mmz(mb_src->mb_surface, size);
        if (ret != HI_SUCCESS) {
            return HI_ERR_TDE_INVALID_PARA;
        }
    }
#endif

    return HI_SUCCESS;
}

static hi_s32 tde_osi_get_double_cycle_data(hi_tde_rect *mb_rect, hi_u32 *i, hi_u32 *j)
{
    if ((mb_rect->height > TDE_MAX_RECT_HEIGHT) && (mb_rect->width > TDE_MAX_RECT_WIDTH)) {
        *i = 2; /* 2 double cycle data */
        *j = 2; /* 2 double cycle data */
    } else if ((mb_rect->height > TDE_MAX_RECT_HEIGHT) && (mb_rect->width <= TDE_MAX_RECT_WIDTH)) {
        *i = 1;
        *j = 2; /* 2 double cycle data */
    } else if ((mb_rect->height <= TDE_MAX_RECT_HEIGHT) && (mb_rect->width > TDE_MAX_RECT_WIDTH)) {
        *i = 2; /* 2 double cycle data */
        *j = 1;
    } else {
        *i = 1;
        *j = 1;
    }
    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT

/*
 * Function:      tde_osi_bitmap_mask_check_para
 * Description:   check for trinal source operate parameter
 * Return:        =0: success; <0: fail
 */
#if (TDE_CAPABILITY & MASKROP || TDE_CAPABILITY & MASKBLEND)
static hi_s32 tde_osi_bitmap_mask_check_triple_src(hi_tde_triple_src *triple_src)
{
    if (tde_osi_check_surface(triple_src->bg_surface, triple_src->bg_rect) != HI_SUCCESS) {
        tde_error("pstBackGroundRect does not correct!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_surface(triple_src->fg_surface, triple_src->fg_rect) != HI_SUCCESS) {
        tde_error("pstForeGroundRect does not correct!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_surface(triple_src->mask_surface, triple_src->mask_rect) != HI_SUCCESS) {
        tde_error("pstMaskRect does not correct!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_surface(triple_src->dst_surface, triple_src->dst_rect) != HI_SUCCESS) {
        tde_error("dst_rect does not correct!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_bitmap_mask_check_para(hi_tde_triple_src *triple_src)
{
    hi_bool is_unsupport_scale = HI_FALSE;
    hi_s32 ret;
    is_unsupport_scale = ((triple_src->bg_surface == HI_NULL) || (triple_src->bg_rect == HI_NULL) ||
        (triple_src->fg_surface == HI_NULL) || (triple_src->fg_rect == HI_NULL) ||
        (triple_src->mask_surface == HI_NULL) || (triple_src->mask_rect == HI_NULL) ||
        (triple_src->dst_surface == HI_NULL) || (triple_src->dst_rect == HI_NULL));

    if (is_unsupport_scale) {
        tde_error("Contains HI_NULL ptr!\n");
        return HI_ERR_TDE_NULL_PTR;
    }

    if ((triple_src->fg_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
        (triple_src->bg_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
        (triple_src->mask_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
        (triple_src->dst_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP)) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    if ((triple_src->bg_surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) ||
        (triple_src->bg_surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("This operation doesn't support PKG!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#endif

    ret = tde_osi_bitmap_mask_check_triple_src(triple_src);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    is_unsupport_scale = ((triple_src->bg_rect->width != triple_src->fg_rect->width) ||
                          (triple_src->fg_rect->width != triple_src->mask_rect->width) ||
                          (triple_src->mask_rect->width != triple_src->dst_rect->width) ||
                          (triple_src->bg_rect->height != triple_src->fg_rect->height) ||
                          (triple_src->fg_rect->height != triple_src->mask_rect->height) ||
                          (triple_src->mask_rect->height != triple_src->dst_rect->height));

    if (is_unsupport_scale) {
        tde_error("Don't support scale!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_bitmap_mask_rop
 * Description:   Ropmask foreground and mask at firs, and then ropmask background and middle bitmap
 *                output result to target bitmap
 */
#if (TDE_CAPABILITY & MASKROP)
static hi_s32 tde_osi_bitmap_mask_rop_set_hw_node(tde_hw_node *hw_node, hi_tde_triple_src *triple_src,
                                                  hi_tde_surface *mid_surface, hi_tde_rect *mid_rect)
{
    tde_scandirection_mode scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};

    scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    scan_info.v_scan = TDE_SCAN_UP_DOWN;

    tde_osi_convert_surface(triple_src->fg_surface, triple_src->fg_rect, &scan_info, &src_drv_surface);

    tde_hal_node_set_src1(hw_node, &src_drv_surface);

    tde_osi_convert_surface(triple_src->mask_surface, triple_src->mask_rect, &scan_info, &src_drv_surface);

    tde_hal_node_set_src2(hw_node, &src_drv_surface);

    if (memcpy_s(mid_surface, sizeof(hi_tde_surface), triple_src->fg_surface, sizeof(hi_tde_surface)) != EOK) {
        tde_error("secure function failure\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    mid_surface->phy_addr = tde_osi_list_get_phy_buff(0);
    if (mid_surface->phy_addr == 0) {
        tde_error("There is no temp buffer in tde_osi_bitmap_mask_rop!\n");
        return HI_ERR_TDE_NO_MEM;
    }
    mid_surface->clut_phy_addr = 0;
    mid_rect->pos_x = 0;
    mid_rect->pos_y = 0;
    mid_rect->height = triple_src->fg_rect->height;
    mid_rect->width = triple_src->fg_rect->width;

    tde_osi_convert_surface(mid_surface, mid_rect, &scan_info, &dst_drv_surface);

    tde_hal_node_set_tqt(hw_node, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_NORM);

    tde_osi_set_ext_alpha(triple_src->fg_surface, mid_surface, hw_node);

    if (tde_hal_node_set_base_operate(hw_node, TDE_NORM_BLIT_2OPT, TDE_ALU_MASK_ROP1, 0) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_bitmap_mask_rop_set_hw_node_pass2(tde_hw_node *hw_node_pass2,
                                                        hi_tde_triple_src *triple_src,
                                                        hi_tde_none_src *none_src,
                                                        hi_tde_rop_mode rop_color, hi_tde_rop_mode rop_alpha)
{
    tde_scandirection_mode scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};
    hi_u16 code;
    tde_conv_mode_cmd conv = {0};

    scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    scan_info.v_scan = TDE_SCAN_UP_DOWN;

    tde_osi_convert_surface(triple_src->bg_surface, triple_src->bg_rect, &scan_info, &src_drv_surface);

    tde_hal_node_set_src1(hw_node_pass2, &src_drv_surface);

    tde_osi_convert_surface(none_src->dst_surface, none_src->dst_rect, &scan_info, &dst_drv_surface);

    tde_hal_node_set_src2(hw_node_pass2, &dst_drv_surface);

    tde_osi_convert_surface(triple_src->dst_surface, triple_src->dst_rect, &scan_info, &dst_drv_surface);

    tde_hal_node_set_tqt(hw_node_pass2, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_NORM);

    code = tde_osi_single_src_to_get_opt_code(triple_src->fg_surface->color_fmt,
                                              triple_src->dst_surface->color_fmt);

    tde_osi_get_conv_by_code(code, &conv);

    if (tde_hal_node_set_color_convert(hw_node_pass2, &conv) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    if (tde_hal_node_set_rop(hw_node_pass2, rop_color, rop_alpha) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    tde_osi_set_ext_alpha(triple_src->bg_surface, none_src->dst_surface, hw_node_pass2);

    /* logical operation second passs */
    if (tde_hal_node_set_base_operate(hw_node_pass2, TDE_NORM_BLIT_2OPT, TDE_ALU_MASK_ROP2, 0) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_bitmap_mask_rop_check_color(hi_tde_triple_src *triple_src)
{
    if (!tde_osi_whether_contain_alpha(triple_src->fg_surface->color_fmt)) {
        tde_error("ForeGround bitmap must contains alpha component!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (triple_src->mask_surface->color_fmt != HI_TDE_COLOR_FMT_A1) {
        tde_error("Maskbitmap's colorformat can only be A1 in tde_osi_bitmap_mask_rop!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

hi_s32 tde_osi_bitmap_mask_rop(hi_s32 handle, hi_tde_triple_src *triple_src, hi_tde_rop_mode rop_color,
                               hi_tde_rop_mode rop_alpha)
{
    tde_hw_node *hw_node = HI_NULL;
    tde_hw_node *hw_node_pass2 = HI_NULL;
    hi_tde_surface mid_surface = {0};
    hi_tde_rect mid_rect = {0};
    hi_tde_none_src none_src = {&mid_surface, &mid_rect};
    hi_s32 ret;

    ret = tde_osi_bitmap_mask_check_para(triple_src);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (tde_osi_bitmap_mask_rop_check_color(triple_src) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    /* init hw_node */
    if (tde_hal_node_init_nd(&hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }
    /*
     * set hw_node:
     * set src1
     * set src2
     * set tqt
     * set ext alpha
     * set opt
     * set global alpha
     */
    ret = tde_osi_bitmap_mask_rop_set_hw_node(hw_node, triple_src, &mid_surface, &mid_rect);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_osi_list_put_phy_buff(1);
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    /* init hw_node_pass2 */
    if (tde_hal_node_init_nd(&hw_node_pass2) != HI_SUCCESS) {
        tde_osi_list_put_phy_buff(1);
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_NO_MEM;
    }

    /*
     * set hw_node:
     * set src1
     * set src2
     * set tqt
     * set color convert
     * set rop
     * set ext alpha
     * set opt
     */
    ret = tde_osi_bitmap_mask_rop_set_hw_node_pass2(hw_node_pass2, triple_src, &none_src,
                                                    rop_color, rop_alpha);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_hal_free_node_buf(hw_node_pass2);
        return ret;
    }

    ret = tde_osi_set_node_finish(handle, hw_node_pass2, 1, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_hal_free_node_buf(hw_node_pass2);
        tde_osi_list_put_phy_buff(1);
        return ret;
    }
    return HI_SUCCESS;
}

#endif

#if (TDE_CAPABILITY & MASKBLEND)
static hi_s32 tde_osi_bitmap_mask_blend_set_hw_node(tde_hw_node *hw_node, hi_tde_triple_src *triple_src,
                                                    hi_tde_surface *mid_surface, hi_tde_rect *mid_rect)
{
    tde_scandirection_mode scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};

    scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    scan_info.v_scan = TDE_SCAN_UP_DOWN;

    tde_osi_convert_surface(triple_src->fg_surface, triple_src->fg_rect, &scan_info, &src_drv_surface);

    tde_hal_node_set_src1(hw_node, &src_drv_surface);

    tde_osi_convert_surface(triple_src->mask_surface, triple_src->mask_rect, &scan_info, &src_drv_surface);

    tde_hal_node_set_src2(hw_node, &src_drv_surface);

    if (memcpy_s(mid_surface, sizeof(hi_tde_surface), triple_src->fg_surface, sizeof(hi_tde_surface)) != EOK) {
        tde_error("secure function failure\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    mid_surface->phy_addr = tde_osi_list_get_phy_buff(0);
    if (mid_surface->phy_addr == 0) {
        tde_error("There is no temp buffer in tde_osi_bitmap_mask_blend!\n");
        return HI_ERR_TDE_NO_MEM;
    }
    mid_rect->pos_x = 0;
    mid_rect->pos_y = 0;
    mid_rect->height = triple_src->fg_rect->height;
    mid_rect->width = triple_src->fg_rect->width;

    tde_osi_convert_surface(mid_surface, mid_rect, &scan_info, &dst_drv_surface);

    tde_hal_node_set_tqt(hw_node, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_NORM);

    tde_osi_set_ext_alpha(triple_src->fg_surface, mid_surface, hw_node);

    if (tde_hal_node_set_base_operate(hw_node, TDE_NORM_BLIT_2OPT, TDE_ALU_MASK_BLEND, 0) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    tde_hal_node_set_global_alpha(hw_node, 0xff, HI_TRUE);
    return HI_SUCCESS;
}

static hi_s32 tde_osi_bitmap_mask_blend_set_hw_node_pass2(tde_hw_node *hw_node_pass2,
                                                          hi_tde_triple_src *triple_src,
                                                          hi_tde_surface *mid_surface, hi_tde_rect *mid_rect,
                                                          hi_u8 alpha)
{
    tde_scandirection_mode scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};
    hi_u16 code;
    tde_conv_mode_cmd conv = {0};
    tde_alu_mode drv_alu_mode = TDE_ALU_BLEND;
    hi_tde_blend_opt blend_opt = {0};

    scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    scan_info.v_scan = TDE_SCAN_UP_DOWN;
    tde_osi_convert_surface(triple_src->bg_surface, triple_src->bg_rect, &scan_info, &src_drv_surface);

    tde_hal_node_set_src1(hw_node_pass2, &src_drv_surface);

    tde_osi_convert_surface(mid_surface, mid_rect, &scan_info, &dst_drv_surface);

    tde_hal_node_set_src2(hw_node_pass2, &dst_drv_surface);

    tde_osi_convert_surface(triple_src->dst_surface, triple_src->dst_rect, &scan_info, &dst_drv_surface);

    tde_hal_node_set_tqt(hw_node_pass2, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_NORM);

    code = tde_osi_single_src_to_get_opt_code(triple_src->fg_surface->color_fmt, triple_src->dst_surface->color_fmt);

    tde_osi_get_conv_by_code(code, &conv);

    if (tde_hal_node_set_color_convert(hw_node_pass2, &conv) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    tde_hal_node_set_global_alpha(hw_node_pass2, alpha, HI_TRUE);

    if (tde_hal_node_set_base_operate(hw_node_pass2, TDE_NORM_BLIT_2OPT, drv_alu_mode, 0) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    blend_opt.global_alpha_en = HI_TRUE;
    blend_opt.pixel_alpha_en = HI_TRUE;
    blend_opt.blend_cmd = HI_TDE_BLEND_CMD_NONE;
    if (tde_hal_node_set_blend(hw_node_pass2, &blend_opt) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    tde_osi_set_ext_alpha(triple_src->bg_surface, mid_surface, hw_node_pass2);
    return HI_SUCCESS;
}

static hi_s32 tde_osi_bitmap_mask_blend_check_color(hi_tde_triple_src *triple_src,
                                                    hi_tde_alpha_blending blend_mode)
{
    if ((triple_src->mask_surface->color_fmt != HI_TDE_COLOR_FMT_A1) &&
        (triple_src->mask_surface->color_fmt != HI_TDE_COLOR_FMT_A8)) {
        tde_error("Maskbitmap's colorformat can only be An in tde_osi_bitmap_mask_blend!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (blend_mode != HI_TDE_ALPHA_BLENDING_BLEND) {
        tde_error("Alum mode can only be blending in tde_osi_bitmap_mask_blend!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

hi_s32 tde_osi_bitmap_mask_blend(hi_s32 handle, hi_tde_triple_src *triple_src, hi_u8 alpha,
                                 hi_tde_alpha_blending blend_mode)
{
    tde_hw_node *hw_node = HI_NULL;
    tde_hw_node *hw_node_pass2 = HI_NULL;
    hi_tde_surface mid_surface = {0};
    hi_tde_rect mid_rect = {0};
    hi_s32 ret;

    ret = tde_osi_bitmap_mask_check_para(triple_src);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    /* check color format and blend mode */
    if (tde_osi_bitmap_mask_blend_check_color(triple_src, blend_mode) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    /* init hw_node */
    if (tde_hal_node_init_nd(&hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    /*
     * set hw_node:
     * set src1
     * set src2
     * set tqt
     * set ext alpha
     * set opt
     * set global alpha
     */
    ret = tde_osi_bitmap_mask_blend_set_hw_node(hw_node, triple_src, &mid_surface, &mid_rect);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_osi_list_put_phy_buff(1);
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    /* init hw_node_pass2 */
    if (tde_hal_node_init_nd(&hw_node_pass2) != HI_SUCCESS) {
        tde_osi_list_put_phy_buff(1);
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_NO_MEM;
    }

    /*
     * set hw_node_pass2:
     * set src1
     * set src2
     * set tqt
     * set color convert
     * set global alpha
     * set opt
     * set blend
     * set ext alpha
     */
    ret = tde_osi_bitmap_mask_blend_set_hw_node_pass2(hw_node_pass2, triple_src, &mid_surface, &mid_rect,
                                                      alpha);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_hal_free_node_buf(hw_node_pass2);
        return ret;
    }

    ret = tde_osi_set_node_finish(handle, hw_node_pass2, 1, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        tde_hal_free_node_buf(hw_node_pass2);
        tde_osi_list_put_phy_buff(1);
        return ret;
    }

    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_solid_draw
 * Description:   operate src1 with src2, which result to dst_surface,operate setting is in pstOpt
 *                if src is MB, only support single source operate,
                  just to say to only support pstBackGround or pstForeGround
 * Input:         handle: task handle
 *                fill_color:  fill  color
 *                opt: operate parameter setting struct
 * Return:        HI_SUCCESS/HI_FAILURE
 */
hi_s32 tde_osi_solid_draw(hi_s32 handle, hi_tde_single_src *single_src, hi_tde_fill_color *fill_color,
                          hi_tde_opt *opt)
{
    if (single_src->src_surface == HI_NULL) {
        return tde_osi_1_source_fill(handle, single_src->dst_surface, single_src->dst_rect, fill_color, opt);
    } else {
        if ((single_src->dst_surface == HI_NULL) || (fill_color == HI_NULL)) {
            return HI_ERR_TDE_NULL_PTR;
        }

        if ((single_src->src_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
            (single_src->dst_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
            (fill_color->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP)) {
            tde_error("This operation doesn't support Semi-plannar!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
        return tde_osi_2_source_fill(handle, single_src, fill_color, opt);
    }
}

#if (TDE_CAPABILITY & DEFLICKER)
hi_s32 tde_osi_set_deflicker_level(hi_tde_deflicker_level deflicker_level)
{
    return tde_hal_set_deflicer_level(deflicker_level);
}

hi_s32 tde_osi_get_deflicker_level(hi_tde_deflicker_level *deflicker_level)
{
    if (deflicker_level == HI_NULL) {
        return HI_FAILURE;
    }
    return tde_hal_get_deflicer_level(deflicker_level);
}
#endif

#endif

hi_s32 tde_osi_set_alpha_threshold_value(hi_u8 alpha_threshold_value)
{
    return tde_hal_set_alpha_threshold(alpha_threshold_value);
}
#ifndef HI_BUILD_IN_BOOT
#endif

hi_s32 tde_osi_set_alpha_threshold_state(hi_bool alpha_threshold_en)
{
    return tde_hal_set_alpha_threshold_state(alpha_threshold_en);
}

#ifndef HI_BUILD_IN_BOOT

hi_s32 tde_osi_get_alpha_threshold_value(hi_u8 *alpha_threshold_value)
{
    if (alpha_threshold_value == HI_NULL) {
        return HI_FAILURE;
    }
    return tde_hal_get_alpha_threshold(alpha_threshold_value);
}

hi_s32 tde_osi_get_alpha_threshold_state(hi_bool *alpha_threshold_en)
{
    if (alpha_threshold_en == HI_NULL) {
        return HI_FAILURE;
    }

    return tde_hal_get_alpha_threshold_state(alpha_threshold_en);
}

static hi_s32 tde_osi_check_src_pattern_fill_opt(hi_tde_pattern_fill_opt *opt)
{
    if (opt == HI_NULL) {
        return HI_FAILURE;
    }

    if ((opt->color_key_mode >= HI_TDE_COLOR_KEY_MODE_MAX) ||
        (opt->color_key_mode < HI_TDE_COLOR_KEY_MODE_NONE)) {
        tde_error("color_key_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (((opt->clut_reload != HI_TRUE) && (opt->clut_reload != HI_FALSE)) ||
        ((opt->blend_opt.global_alpha_en != HI_TRUE) && (opt->blend_opt.global_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.pixel_alpha_en != HI_TRUE) && (opt->blend_opt.pixel_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.src1_alpha_premulti != HI_TRUE) && (opt->blend_opt.src1_alpha_premulti != HI_FALSE)) ||
        ((opt->blend_opt.src2_alpha_premulti != HI_TRUE) && (opt->blend_opt.src2_alpha_premulti != HI_FALSE))) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->color_key_mode != HI_TDE_COLOR_KEY_MODE_NONE) {
        tde_error("It doesn't support colorkey in single source pattern mode!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if (opt->alpha_blending_cmd >= HI_TDE_ALPHA_BLENDING_MAX) {
        tde_error("alpha_blending_cmd error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_check_single_src_pattern_opt
 * Description:   check if single source mode fill operate is valid
 * Input:         src_fmt foreground pixel format
                  dst_fmt  target pixel format
                  opt    operate attribute pointer
 * Return:        0  valid parameter
                  -1 invalid parameter
 */
hi_s32 tde_osi_check_single_src_pattern_opt(hi_tde_color_fmt src_fmt,
                                            hi_tde_color_fmt dst_fmt, hi_tde_pattern_fill_opt *opt)
{
    hi_s32 ret;
    tde_colorfmt_transform color_trans_type = tde_osi_get_fmt_trans_type(src_fmt, dst_fmt);
    if (color_trans_type == TDE_COLORFMT_TRANSFORM_BUTT) {
        tde_error("Unknown color transport type!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    ret = tde_osi_check_src_pattern_fill_opt(opt);
    if (ret != HI_SUCCESS) {
        if (ret == HI_FAILURE) {
            return HI_SUCCESS;
        }
        return ret;
    }
#if (TDE_CAPABILITY & ROP)
    if ((hi_u32)opt->alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) {
        if ((!tde_osi_is_single_src_to_rop(opt->rop_alpha)) ||
            (!tde_osi_is_single_src_to_rop(opt->rop_color))) {
            tde_error("Only support single s2 rop!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }
    } else
#endif
        if (opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE) {
            tde_error("Alu mode error!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }

    if (color_trans_type != TDE_COLORFMT_TRANSFORM_CLUT_CLUT) {
        return HI_SUCCESS;
    }

    if ((opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE)) {
        tde_error("It doesn't ROP/Blend/Colorize!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if (src_fmt != dst_fmt) {
        tde_error("If src fmt and dst fmt are clut, they shoulod be the same fmt!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_bool_opt(hi_tde_color_fmt back_ground_fmt, hi_tde_pattern_fill_opt *opt)
{
    hi_bool real = (((opt->clut_reload != HI_TRUE) && (opt->clut_reload != HI_FALSE)) ||
        ((opt->blend_opt.global_alpha_en != HI_TRUE) && (opt->blend_opt.global_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.pixel_alpha_en != HI_TRUE) && (opt->blend_opt.pixel_alpha_en != HI_FALSE)) ||
        ((opt->blend_opt.src1_alpha_premulti != HI_TRUE) && (opt->blend_opt.src1_alpha_premulti != HI_FALSE)) ||
        ((opt->blend_opt.src2_alpha_premulti != HI_TRUE) && (opt->blend_opt.src2_alpha_premulti != HI_FALSE)));

    if (opt->alpha_blending_cmd >= HI_TDE_ALPHA_BLENDING_MAX) {
        tde_error("alpha_blending_cmd error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    if ((back_ground_fmt == HI_TDE_COLOR_FMT_YCBCR422) || (back_ground_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("This operation doesn't support PKG!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
#else
    hi_unused(back_ground_fmt);
#endif

    if (real) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((opt->color_key_mode >= HI_TDE_COLOR_KEY_MODE_MAX) ||
        (opt->color_key_mode < HI_TDE_COLOR_KEY_MODE_NONE)) {
        tde_error("color_key_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (opt->out_alpha_from >= HI_TDE_OUT_ALPHA_FROM_MAX) {
        tde_error("out_alpha_from error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_check_double_src_pattern_opt
 * Description:   check if doul source mode fill operate parameter is valid
 * Input:         back_ground_fmt background pixel format
                  fore_ground_fmt foreground pixel format
                  dst_fmt  target pixel format
                  opt     operate attribute pointer
 * Return:        0  valid parameter
                  -1 invalid parameter
 */
hi_s32 tde_osi_check_double_src_pattern_opt(hi_tde_color_fmt back_ground_fmt, hi_tde_color_fmt fore_ground_fmt,
                                            hi_tde_color_fmt dst_fmt, hi_tde_pattern_fill_opt *opt)
{
    tde_colorfmt_category bg_category, fg_category, dst_category;
    hi_bool temp_fmt = HI_FALSE;

    hi_s32 ret = tde_osi_check_bool_opt(back_ground_fmt, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    bg_category = tde_osi_get_fmt_category(back_ground_fmt);

    fg_category = tde_osi_get_fmt_category(fore_ground_fmt);

    dst_category = tde_osi_get_fmt_category(dst_fmt);
    if ((bg_category >= TDE_COLORFMT_CATEGORY_BYTE) ||
        (fg_category >= TDE_COLORFMT_CATEGORY_BYTE) || (dst_category >= TDE_COLORFMT_CATEGORY_BYTE)) {
        tde_error("unknown format!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }
    temp_fmt = tde_osi_whether_contain_alpha(dst_fmt);
    if ((bg_category == TDE_COLORFMT_CATEGORY_ARGB) &&
        (fg_category == TDE_COLORFMT_CATEGORY_AN) && (!temp_fmt)) {
        tde_error("Target must have alpha component!\n");
        return HI_ERR_TDE_INVALID_PARA;
    } else if ((fg_category == TDE_COLORFMT_CATEGORY_AN) && ((back_ground_fmt == HI_TDE_COLOR_FMT_YCBCR888) ||
        (fore_ground_fmt == HI_TDE_COLOR_FMT_AYCBCR8888))) {
        tde_error("Target must have alpha component!\n");
        return HI_ERR_TDE_INVALID_PARA;
    } else if ((bg_category == TDE_COLORFMT_CATEGORY_CLUT) && (
        opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE)) {
        tde_error("It doesn't support ROP!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    } else if ((bg_category == TDE_COLORFMT_CATEGORY_CLUT) &&
        ((back_ground_fmt != fore_ground_fmt) || (back_ground_fmt != dst_fmt))) {
        tde_error("If background, foreground , dst are clut, they should be the same fmt!\n");
        return HI_ERR_TDE_INVALID_PARA;
    } else if ((bg_category == TDE_COLORFMT_CATEGORY_CLUT) &&
        ((fg_category != TDE_COLORFMT_CATEGORY_CLUT) || (dst_category != TDE_COLORFMT_CATEGORY_CLUT))) {
        tde_error("Unsupported operation!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    } else if ((bg_category == TDE_COLORFMT_CATEGORY_AN) && (fg_category == TDE_COLORFMT_CATEGORY_AN) &&
        (dst_category == TDE_COLORFMT_CATEGORY_AN) && (opt->alpha_blending_cmd != HI_TDE_ALPHA_BLENDING_NONE)) {
        tde_error("It doesn't support ROP or mirror!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    } else if ((dst_category == TDE_COLORFMT_CATEGORY_CLUT) && ((fg_category != TDE_COLORFMT_CATEGORY_CLUT) ||
        (bg_category != TDE_COLORFMT_CATEGORY_CLUT))) {
        tde_error("Unsupported operation!\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_get_pattern_opt_category
 * Description:   analyze pattern fill operate type
 * Input:         dst_surface: target bitmap info
 *                dst_rect: target bitmap operate zone
 *                opt: operate option
 * Return:        TDE operate type
 */
tde_pattern_operation_category tde_osi_get_pattern_opt_category(hi_tde_double_src *double_src,
                                                                hi_tde_pattern_fill_opt *opt)
{
    if ((double_src == HI_NULL) || (double_src->dst_surface == HI_NULL) || (double_src->dst_rect == HI_NULL)) {
        tde_error("Dst/Dst Rect should not be null!\n");
        return TDE_PATTERN_OPERATION_BUTT;
    }
    if ((HI_TDE_COLOR_FMT_YCBCR422 == double_src->dst_surface->color_fmt) ||
        (double_src->dst_surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("It doesn't support YCbCr422 in pattern fill!\n");
        return -1;
    }

    if (double_src->dst_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_surface(double_src->dst_surface, double_src->dst_rect) != HI_SUCCESS) {
        return TDE_PATTERN_OPERATION_BUTT;
    }

    if ((double_src->bg_surface == HI_NULL) && (double_src->fg_surface == HI_NULL)) {
        tde_error("No src!\n");
        return TDE_PATTERN_OPERATION_BUTT;
    } else if ((double_src->bg_surface != HI_NULL) && (double_src->fg_surface != HI_NULL)) {
        return tde_osi_double_src_pattern_operation(double_src, opt);
    } else {
        return tde_osi_check_single_src_pattern_operation(double_src, opt);
    }
}

static tde_pattern_operation_category tde_osi_check_single_src_pattern_operation(hi_tde_double_src *double_src,
                                                                                 hi_tde_pattern_fill_opt *opt)
{
    hi_tde_surface *tmp_src2 = HI_NULL;
    hi_tde_rect *tmp_src2_rect = HI_NULL;
    hi_unused(opt);
    if (double_src->bg_surface != HI_NULL) {
        if (double_src->bg_rect == HI_NULL) {
            tde_error("Background rect shouldn't be HI_NULL!\n");
            return TDE_PATTERN_OPERATION_BUTT;
        }
        tmp_src2 = double_src->bg_surface;
        tmp_src2_rect = double_src->bg_rect;
    } else if (double_src->fg_surface != HI_NULL) {
        if (double_src->fg_rect == HI_NULL) {
            tde_error("Foreground rect shouldn't be HI_NULL!\n");
            return TDE_PATTERN_OPERATION_BUTT;
        }
        if (double_src->fg_surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) {
            tde_error("It doesn't support YCbCr422 in pattern fill!\n");
            return TDE_PATTERN_OPERATION_BUTT;
        }
        tmp_src2 = double_src->fg_surface;
        tmp_src2_rect = double_src->fg_rect;
    }

    if ((tmp_src2 != HI_NULL) && (tmp_src2_rect != HI_NULL)) {
        if (tmp_src2->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) {
            tde_error("This operation doesn't support Semi-plannar!\n");
            return HI_ERR_TDE_INVALID_PARA;
        }

        if (tmp_src2_rect->width > TDE_MAX_PATTERNWIDTH) {
            tde_error("Max pattern width is 256!\n");
            return TDE_PATTERN_OPERATION_BUTT;
        }
        if (tde_osi_check_surface(tmp_src2, tmp_src2_rect) != HI_SUCCESS) {
            return TDE_PATTERN_OPERATION_BUTT;
        }
    }

    return TDE_PATTERN_OPERATION_SINGLE_SRC;
}

static tde_pattern_operation_category tde_osi_double_src_pattern_operation(hi_tde_double_src *double_src,
                                                                           hi_tde_pattern_fill_opt *opt)
{
    if ((double_src->bg_rect == HI_NULL) || (double_src->fg_rect == HI_NULL) || (opt == HI_NULL)) {
        tde_error("Background Rect/Foreground Rect/Opt should not be null in two src pattern fill!\n");
        return TDE_PATTERN_OPERATION_BUTT;
    }

    if ((double_src->bg_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP) ||
        (double_src->fg_surface->color_fmt >= HI_TDE_COLOR_FMT_JPG_YCBCR400MBP)) {
        tde_error("This operation doesn't support Semi-plannar!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    if ((double_src->fg_surface->color_fmt == HI_TDE_COLOR_FMT_YCBCR422) ||
        (double_src->fg_surface->color_fmt == HI_TDE_COLOR_FMT_PKGVYUY)) {
        tde_error("It doesn't support YCbCr422 in pattern fill!\n");
        return TDE_PATTERN_OPERATION_BUTT;
    }

    if ((tde_osi_check_surface(double_src->bg_surface, double_src->bg_rect) != HI_SUCCESS) ||
        (tde_osi_check_surface(double_src->fg_surface, double_src->fg_rect) != HI_SUCCESS)) {
        return TDE_PATTERN_OPERATION_BUTT;
    }

    if ((double_src->bg_rect->height != double_src->dst_rect->height) ||
        (double_src->bg_rect->width != double_src->dst_rect->width)) {
        tde_error("Size of background rect and Dst rect should be the same in two src pattern fill.\n");
        tde_error("background x:%d, y:%d, w:%d, h:%d; dst x:%d, y:%d, w:%d, h:%d\n", double_src->bg_rect->pos_x,
            double_src->bg_rect->pos_y, double_src->bg_rect->width, double_src->bg_rect->height,
            double_src->dst_rect->pos_x, double_src->dst_rect->pos_y, double_src->dst_rect->width,
            double_src->dst_rect->height);
        return TDE_PATTERN_OPERATION_BUTT;
    }

    if (double_src->fg_rect->width > TDE_MAX_PATTERNWIDTH) {
        tde_error("Max pattern width is 256!\n");
        return TDE_PATTERN_OPERATION_BUTT;
    }

    return TDE_PATTERN_OPERATION_DOUBLE_SRC;
}

hi_bool tde_osi_check_overlap(hi_tde_surface *sur1, hi_tde_rect *rect1, hi_tde_surface *sur2,
                              hi_tde_rect *rect2)
{
    hi_u32 rect1_start_phy;
    hi_u32 rect1_end_phy;
    hi_u32 rect2_start_phy;
    hi_u32 rect2_end_phy;
    hi_u32 bpp1;
    hi_u32 bpp2;

    if (rect2->height < rect1->height) {
        rect1->height = rect2->height;
    }

    if (rect2->width < rect1->width) {
        rect1->width = rect2->width;
    }

    bpp1 = tde_osi_get_bpp_by_fmt(sur1->color_fmt) / 8; /* 8 matrixing bpp */
    bpp2 = tde_osi_get_bpp_by_fmt(sur2->color_fmt) / 8; /* 8 matrixing bpp */

    rect1_start_phy = sur1->phy_addr + (rect1->pos_y * sur1->stride)  +
        rect1->pos_x * bpp1;
    rect1_end_phy = rect1_start_phy + (rect1->height - 1) * sur1->stride  +
        (rect1->width - 1) * bpp1;

    rect2_start_phy = sur2->phy_addr + (rect2->pos_y * sur2->stride)  +
        rect2->pos_x * bpp2;
    rect2_end_phy = rect2_start_phy + (rect2->height - 1) * sur2->stride  +
        (rect2->width - 1) * bpp2;

    tde_info("u32Rect1StartPhy:%x, u32Rect1EndPhy:%x, u32Rect2StartPhy:%x, u32Rect2EndPhy:%x\n", rect1_start_phy,
        rect1_end_phy, rect2_start_phy, rect2_end_phy);
    return HI_FALSE;
}

static hi_s32 tde_osi_pattern_fill_opt(tde_hw_node *hw_node, hi_tde_pattern_fill_opt *opt,
                                       tde_alu_mode *alu_mode, hi_bool is_check_single_src2rop,
                                       hi_bool is_check_blend)
{
    hi_s32 ret;
#if (TDE_CAPABILITY & ROP)
    ret = tde_osi_set_rop(hw_node, opt->alpha_blending_cmd, opt->rop_color, opt->rop_alpha,
                          alu_mode, is_check_single_src2rop);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
#endif
    ret = tde_osi_set_blend(hw_node, opt->alpha_blending_cmd, opt->blend_opt, alu_mode,
                            is_check_blend);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
#if (TDE_CAPABILITY & COLORIZE)
    ret = tde_osi_set_colorize(hw_node, opt->alpha_blending_cmd, opt->color_resize);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
#endif
    tde_hal_node_set_global_alpha(hw_node, opt->global_alpha, opt->blend_opt.global_alpha_en);
    return HI_SUCCESS;
}

static hi_s32 tde_osi_single_pattern_fill_opt(tde_hw_node *hw_node, hi_tde_single_src *single_src,
                                              hi_tde_pattern_fill_opt *opt)
{
    hi_bool is_check_single_src2rop = HI_TRUE;
    hi_bool is_check_blend = HI_FALSE;
    hi_s32 ret;
    tde_alu_mode alu_mode = TDE_ALU_NONE;
    tde_base_opt_mode base_mode = TDE_SINGLE_SRC_PATTERN_FILL_OPT;
    hi_tde_double_src double_src = {0};

    if (opt != HI_NULL) {
        if (opt->out_alpha_from >= HI_TDE_OUT_ALPHA_FROM_MAX) {
            tde_error("out_alpha_from error!\n");
            tde_hal_free_node_buf(hw_node);
            return HI_ERR_TDE_INVALID_PARA;
        }

        if (opt->out_alpha_from == HI_TDE_OUT_ALPHA_FROM_BACKGROUND) {
            tde_error("Single src pattern fill doesn't support out alpha form background!\n");
            tde_hal_free_node_buf(hw_node);
            return HI_ERR_TDE_INVALID_PARA;
        }

        ret = tde_osi_pattern_fill_opt(hw_node, opt, &alu_mode, is_check_single_src2rop, is_check_blend);
        if (ret != HI_SUCCESS) {
            return ret;
        }

        double_src.bg_surface = HI_NULL;
        double_src.bg_rect = HI_NULL;
        double_src.fg_surface = single_src->src_surface;
        double_src.fg_rect = single_src->src_rect;
        double_src.dst_surface = single_src->dst_surface;
        double_src.dst_rect = single_src->dst_rect;
        if (tde_osi_set_pattern_clip_para(&double_src, opt, hw_node) != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            return HI_ERR_TDE_CLIP_AREA;
        }
    }

    if (tde_hal_node_set_base_operate(hw_node, base_mode, alu_mode, HI_NULL) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_single_pattern_fill_set_up(tde_hw_node *hw_node, hi_tde_single_src *single_src,
                                                 hi_tde_pattern_fill_opt *opt)
{
    hi_s32 ret;
    tde_scandirection_mode src_scan_info = {0};
    tde_scandirection_mode dst_scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};
    hi_u16 code;
    tde_conv_mode_cmd conv = { 0 };
    tde_clut_usage clut_usage = TDE_CLUT_USAGE_BUTT;

    /* set ext alpha */
    tde_osi_set_ext_alpha(HI_NULL, single_src->src_surface, hw_node);
    /* set src2 and tqt */
    src_scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    src_scan_info.v_scan = TDE_SCAN_UP_DOWN;
    dst_scan_info.h_scan = TDE_SCAN_LEFT_RIGHT;
    dst_scan_info.v_scan = TDE_SCAN_UP_DOWN;

    tde_osi_convert_surface(single_src->src_surface, single_src->src_rect, &src_scan_info, &src_drv_surface);

    tde_hal_node_set_src2(hw_node, &src_drv_surface);

    tde_osi_convert_surface(single_src->dst_surface, single_src->dst_rect, &dst_scan_info, &dst_drv_surface);

    (opt != HI_NULL) ? tde_hal_node_set_tqt(hw_node, &dst_drv_surface, opt->out_alpha_from) :
        tde_hal_node_set_tqt(hw_node, &dst_drv_surface, HI_TDE_OUT_ALPHA_FROM_FOREGROUND);
    /* set color */
    code = tde_osi_single_src_to_get_opt_code(single_src->src_surface->color_fmt,
        single_src->dst_surface->color_fmt);

    tde_osi_get_conv_by_code(code, &conv);

    if (tde_hal_node_set_color_convert(hw_node, &conv) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    /* set clut opt */
    ret = (opt != HI_NULL) ?
           tde_osi_set_clut_opt(single_src->src_surface, single_src->dst_surface, &clut_usage, opt->clut_reload,
                                hw_node) :
           tde_osi_set_clut_opt(single_src->src_surface, single_src->dst_surface, &clut_usage, HI_TRUE, hw_node);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    /* set csc */
    if (opt != HI_NULL) {
        ret = tde_set_node_csc(hw_node, opt->csc_opt);
        if (ret != HI_SUCCESS) {
            tde_hal_free_node_buf(hw_node);
            return ret;
        }
    }
    return HI_SUCCESS;
}

hi_s32 tde_osi_single_src_pattern_fill(hi_s32 handle, hi_tde_single_src *single_src,
                                       hi_tde_pattern_fill_opt *opt)
{
    tde_hw_node *hw_node = HI_NULL;
    hi_s32 ret;
    ret = tde_osi_check_single_src_pattern_fill_para(single_src, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (tde_hal_node_init_nd(&hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    /* deal with opt */
    ret = tde_osi_single_pattern_fill_opt(hw_node, single_src, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    /* set:
     * set ext alpha
     * set src2
     * set tqt
     * set color format
     * set clut opt
     * set csc
     */
    ret = tde_osi_single_pattern_fill_set_up(hw_node, single_src, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    /* set node finish */
    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_single_src_pattern_fill_para(hi_tde_single_src *single_src,
                                                         hi_tde_pattern_fill_opt *opt)
{
    hi_bool is_null_ptr = ((single_src == HI_NULL) || (single_src->src_surface == HI_NULL) ||
                           ((single_src->src_rect == HI_NULL)) || (single_src->dst_surface == HI_NULL) ||
                           (single_src->dst_rect == HI_NULL));
    if (is_null_ptr) {
        return HI_ERR_TDE_NULL_PTR;
    }

    if (tde_osi_check_single_src_pattern_opt(single_src->src_surface->color_fmt,
        single_src->dst_surface->color_fmt, opt) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_overlap(single_src->src_surface, single_src->src_rect,
        single_src->dst_surface, single_src->dst_rect)) {
        tde_error("Surface overlap!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_double_src_pattern_fill
 * Description:   dual resource pattern fill
 * Input:         opt: operate option
 * Return:        success/fail
 */
static hi_s32 tde_osi_double_pattern_fill_opt(tde_hw_node *hw_node, hi_tde_double_src *double_src,
                                              hi_tde_pattern_fill_opt *opt)
{
    hi_bool is_check_single_src2rop = HI_FALSE;
    hi_bool is_check_blend = HI_TRUE;
    tde_alu_mode alu_mode = TDE_ALU_NONE;
    hi_s32 ret;
    tde_base_opt_mode base_mode = TDE_DOUBLE_SRC_PATTERN_FILL_OPT;

    ret = tde_osi_pattern_fill_opt(hw_node, opt, &alu_mode, is_check_single_src2rop, is_check_blend);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (tde_osi_set_pattern_clip_para(double_src, opt, hw_node) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_CLIP_AREA;
    }

    if (tde_hal_node_set_base_operate(hw_node, base_mode, alu_mode, HI_NULL) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    return HI_SUCCESS;
}

static hi_void tde_osi_get_scan_info(tde_scandirection_mode *src_scan_info, tde_scandirection_mode *dst_scan_info)
{
    src_scan_info->h_scan = TDE_SCAN_LEFT_RIGHT;
    src_scan_info->v_scan = TDE_SCAN_UP_DOWN;
    dst_scan_info->h_scan = TDE_SCAN_LEFT_RIGHT;
    dst_scan_info->v_scan = TDE_SCAN_UP_DOWN;
}

static hi_s32 tde_osi_double_pattern_fill_set_up(tde_hw_node *hw_node, hi_tde_double_src *double_src,
                                                 hi_tde_pattern_fill_opt *opt)
{
    tde_scandirection_mode src_scan_info = {0};
    tde_scandirection_mode dst_scan_info = {0};
    tde_surface_msg src_drv_surface = {0};
    tde_surface_msg dst_drv_surface = {0};
    hi_u16 code;
    tde_conv_mode_cmd conv = { 0 };
    tde_clut_usage clut_usage = TDE_CLUT_USAGE_BUTT;
    hi_s32 ret;

    tde_osi_set_ext_alpha(double_src->bg_surface, double_src->fg_surface, hw_node);

    tde_osi_get_scan_info(&src_scan_info, &dst_scan_info);

    tde_osi_convert_surface(double_src->bg_surface, double_src->bg_rect, &src_scan_info, &src_drv_surface);

    tde_hal_node_set_src1(hw_node, &src_drv_surface);

    tde_osi_convert_surface(double_src->fg_surface, double_src->fg_rect, &src_scan_info, &src_drv_surface);

    tde_hal_node_set_src2(hw_node, &src_drv_surface);

    tde_osi_convert_surface(double_src->dst_surface, double_src->dst_rect, &dst_scan_info, &dst_drv_surface);

    tde_hal_node_set_tqt(hw_node, &dst_drv_surface, opt->out_alpha_from);

    code = tde_osi_double_src_get_opt_code(double_src->bg_surface->color_fmt,
        double_src->fg_surface->color_fmt, double_src->dst_surface->color_fmt);

    tde_osi_get_conv_by_code(code, &conv);

    if (tde_hal_node_set_color_convert(hw_node, &conv) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    ret = tde_osi_set_clut_opt(double_src->fg_surface, double_src->bg_surface, &clut_usage, opt->clut_reload, hw_node);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    if ((opt->color_key_mode >= HI_TDE_COLOR_KEY_MODE_MAX) || (opt->color_key_mode < HI_TDE_COLOR_KEY_MODE_NONE)) {
        tde_error("color_key_mode error!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    ret = tde_osi_set_color_key(double_src, hw_node, opt->color_key_value, opt->color_key_mode, clut_usage);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    ret = tde_set_node_csc(hw_node, opt->csc_opt);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }
    return HI_SUCCESS;
}

hi_s32 tde_osi_double_src_pattern_fill(hi_s32 handle, hi_tde_double_src *double_src,
                                       hi_tde_pattern_fill_opt *opt)
{
    tde_hw_node *hw_node = HI_NULL;
    hi_s32 ret;

    ret = tde_osi_check_double_src_pattern_fill_para(double_src, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if (tde_hal_node_init_nd(&hw_node) != HI_SUCCESS) {
        return HI_ERR_TDE_NO_MEM;
    }

    /* deal with opt */
    ret = tde_osi_double_pattern_fill_opt(hw_node, double_src, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = tde_osi_double_pattern_fill_set_up(hw_node, double_src, opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = tde_osi_set_node_finish(handle, hw_node, 0, TDE_NODE_SUBM_ALONE);
    if (ret != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return ret;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_check_double_src_pattern_fill_para(hi_tde_double_src *double_src,
                                                         hi_tde_pattern_fill_opt *opt)
{
    hi_bool is_contain_null_ptr = ((double_src == HI_NULL) || (opt == HI_NULL) ||
                                   (double_src->bg_surface == HI_NULL) || (double_src->bg_rect == HI_NULL) ||
                                   (double_src->fg_surface == HI_NULL) || (double_src->fg_rect == HI_NULL) ||
                                   (double_src->dst_surface == HI_NULL) || (double_src->dst_rect == HI_NULL));

    if (is_contain_null_ptr) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_double_src_pattern_opt(double_src->bg_surface->color_fmt,
        double_src->fg_surface->color_fmt, double_src->dst_surface->color_fmt, opt) != HI_SUCCESS) {
        return HI_ERR_TDE_INVALID_PARA;
    }

    if (tde_osi_check_overlap(double_src->fg_surface, double_src->fg_rect, double_src->bg_surface,
                              double_src->bg_rect) ||
        tde_osi_check_overlap(double_src->fg_surface, double_src->fg_rect, double_src->dst_surface,
                              double_src->dst_rect)) {
        tde_error("Surface overlap!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osi_set_blend(tde_hw_node *hw_node, hi_tde_alpha_blending alpha_blending_cmd,
                                hi_tde_blend_opt blend_opt,
                                tde_alu_mode *alu_mode, hi_bool check_blend)
{
    hi_bool set_blend = ((hi_u32)alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_BLEND) ? HI_TRUE : HI_FALSE;
    hi_bool unknown_blend_mode = (blend_opt.blend_cmd == HI_TDE_BLEND_CMD_CONFIG) &&
                                 ((blend_opt.src1_blend_mode >= HI_TDE_BLEND_MAX) ||
                                 (blend_opt.src2_blend_mode >= HI_TDE_BLEND_MAX));
#if (TDE_CAPABILITY & ROP)
    hi_bool enable_alpha_rop = ((hi_u32)alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) ? HI_TRUE : HI_FALSE;
#endif
    hi_bool b_unknown_blend_cmd = (blend_opt.blend_cmd >= HI_TDE_BLEND_CMD_MAX);

    if (!set_blend) {
        return HI_SUCCESS;
    }

    *alu_mode = TDE_ALU_BLEND;

    if (check_blend) {
        if (b_unknown_blend_cmd) {
            tde_error("Unknown blend cmd!\n");

            return HI_ERR_TDE_INVALID_PARA;
        }

        if (unknown_blend_mode) {
            tde_error("Unknown blend mode!\n");

            return HI_ERR_TDE_INVALID_PARA;
        }
    }

    if (tde_hal_node_set_blend(hw_node, &blend_opt) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
#if (TDE_CAPABILITY & ROP)

    if (enable_alpha_rop) {
        tde_hal_node_enable_alpha_rop(hw_node);
    }
#endif
    return HI_SUCCESS;
}

#if (TDE_CAPABILITY & COLORIZE)
static hi_s32 tde_osi_set_colorize(tde_hw_node *hw_node, hi_tde_alpha_blending alpha_blending_cmd,
    hi_s32 color_resize)
{
    hi_bool set_colorize = ((hi_u32)alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_COLORIZE) ? HI_TRUE : HI_FALSE;

    if (!set_colorize) {
        return HI_SUCCESS;
    }

    if (tde_hal_node_set_colorize(hw_node, color_resize) != HI_SUCCESS) {
        tde_hal_free_node_buf(hw_node);
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return HI_SUCCESS;
}
#endif

#if (TDE_CAPABILITY & ROP)
static hi_s32 tde_osi_set_rop(tde_hw_node *hw_node, hi_tde_alpha_blending alpha_blending_cmd,
                              hi_tde_rop_mode rop_code_color,
                              hi_tde_rop_mode rop_code_alpha, tde_alu_mode *alu_mode, hi_bool single_sr2_rop)
{
    hi_bool set_rop = ((hi_u32)alpha_blending_cmd & HI_TDE_ALPHA_BLENDING_ROP) ? HI_TRUE : HI_FALSE;
    hi_bool error_rop_code = (rop_code_color >= HI_TDE_ROP_MAX) || (rop_code_alpha >= HI_TDE_ROP_MAX);
    hi_bool only_support_single_sr2_rop = ((!tde_osi_is_single_src_to_rop(rop_code_alpha)) ||
                                           (!tde_osi_is_single_src_to_rop(rop_code_color)));

    if (!set_rop) {
        return HI_SUCCESS;
    }

    *alu_mode = TDE_ALU_ROP;

    if (error_rop_code) {
        tde_error("enRopCode error!\n");

        return HI_ERR_TDE_INVALID_PARA;
    }

    if (single_sr2_rop) {
        if (only_support_single_sr2_rop) {
            tde_error("Only support single s2 rop!\n");

            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    }

    if (tde_hal_node_set_rop(hw_node, rop_code_color, rop_code_alpha) != HI_SUCCESS) {
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    return HI_SUCCESS;
}
#endif

/*
 * Function:      tde_osi_pattern_fill
 * Description:   pattern fill
 * Input:         handle:task handle
                  opt: operate option
 * Return:        success/fail
 */
hi_s32 tde_osi_pattern_fill(hi_s32 handle, hi_tde_double_src *double_src, hi_tde_pattern_fill_opt *opt)
{
    tde_pattern_operation_category opt_category_en;
    hi_tde_single_src single_src = {0};

    opt_category_en = tde_osi_get_pattern_opt_category(double_src, opt);
    switch (opt_category_en) {
        case TDE_PATTERN_OPERATION_SINGLE_SRC:
            if (double_src->bg_surface != HI_NULL) {
                single_src.src_surface = double_src->bg_surface;
                single_src.src_rect = double_src->bg_rect;
                single_src.dst_surface = double_src->dst_surface;
                single_src.dst_rect = double_src->dst_rect;
            } else {
                single_src.src_surface = double_src->fg_surface;
                single_src.src_rect = double_src->fg_rect;
                single_src.dst_surface = double_src->dst_surface;
                single_src.dst_rect = double_src->dst_rect;
            }
            return tde_osi_single_src_pattern_fill(handle, &single_src, opt);
        case TDE_PATTERN_OPERATION_DOUBLE_SRC:
            return tde_osi_double_src_pattern_fill(handle, double_src, opt);
        default:
            return HI_ERR_TDE_INVALID_PARA;
    }
}

/*
 * Function:      tde_cal_scale_rect
 * Description:   update zoom rect information
 * Input:         src_rect:source bitmap operate zone
                  dst_rect: target bitmap operate zone
 *                rect_in_src: source bitmap scale zone
 *                rect_in_dst: target bitmap info atfer calculating
 * Return:        success/fail
 */
hi_s32 tde_cal_scale_rect(const hi_tde_rect *src_rect, const hi_tde_rect *dst_rect,
                          hi_tde_rect *rect_in_src, hi_tde_rect *rect_in_dst)
{
#if (TDE_CAPABILITY & RESIZE)
    tde_update_config reg = { 0 };
    tde_update_info info = { 0 };

    reg.ori_in_height = src_rect->height;
    reg.ori_in_width = src_rect->width;
    reg.zme_out_height = dst_rect->height;
    reg.zme_out_width = dst_rect->width;

    reg.update_instart_w = rect_in_src->pos_x;
    reg.update_instart_h = rect_in_src->pos_y;
    reg.update_in_width = rect_in_src->width;
    reg.update_in_height = rect_in_src->height;

    tde_osi_get_hupdate_info(&reg, &info, HI_TRUE);
    tde_osi_get_vupdate_info(&reg, &info, HI_TRUE, HI_TRUE);

    rect_in_src->pos_x = info.zme_instart_w;
    rect_in_src->pos_y = info.zme_instart_h;
    rect_in_src->width = info.zme_in_width;
    rect_in_src->height = info.zme_in_height;

    rect_in_dst->pos_x = info.zme_outstart_w;
    rect_in_dst->pos_y = info.zme_outstart_h;
    rect_in_dst->width = info.zme_out_width;
    rect_in_dst->height = info.zme_out_height;
#endif
    return HI_SUCCESS;
}

hi_s32 tde_osi_enable_region_deflicker(hi_bool is_region_deflicker)
{
#if (TDE_CAPABILITY & DEFLICKER)
    if ((is_region_deflicker != HI_TRUE) && (is_region_deflicker != HI_FALSE)) {
        tde_error("hi_bool should be HI_TRUE or HI_FALSE!!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    g_region_deflicker = is_region_deflicker;
#else
    hi_unused(is_region_deflicker);
#endif
    return HI_SUCCESS;
}

#ifdef CONFIG_TDE_BLIT_EX
hi_s32 tde_osi_single_blit_ex(hi_s32 handle, hi_tde_surface *src_surface, hi_tde_rect *src_rect,
                              hi_tde_surface *dst_surface, hi_tde_rect *dst_rect,
                              hi_tde_opt *opt, hi_bool mmz_for_src, hi_bool mmz_for_dst)
{
    hi_s32 ret;
    hi_tde_single_src single_src;

    ret = tde_osi_check_surface(src_surface, src_rect);
    if (ret < 0) {
        return ret;
    }
    ret = tde_osi_check_surface(dst_surface, dst_rect);
    if (ret < 0) {
        return ret;
    }

    single_src.src_surface = src_surface;
    single_src.src_rect    = src_rect;
    single_src.dst_surface = dst_surface;
    single_src.dst_rect    = dst_rect;
    return tde_osi_single_src_2_blit(handle, &single_src, opt, mmz_for_src, mmz_for_dst);
}
#endif

#endif

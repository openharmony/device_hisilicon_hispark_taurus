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

#ifndef __VOU_DRV_H__
#define __VOU_DRV_H__

#include "hi_osal.h"
#include "hi_common_adapt.h"
#include "valg_plat.h"
#include "sys_ext.h"
#include "hi_comm_vo_adapt.h"
#include "mkp_vo_dev.h"
#include "vou_hal.h"
#include "vou_coef_org.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VOU_INTCLEAR_ALL      0xffffffff

#define VO_MAX_VGS_JOB_CNT    1

typedef enum {
    VO_INT_MODE_FRAME = 0x0,
    VO_INT_MODE_FIELD = 0x1,
    VO_INT_MODE_BUTT
} vo_int_mode;

typedef enum {
    VO_LAYER_DDR0 = 0,
    VO_LAYER_DDR1 = 1,
    VO_LAYER_DDR_BUTT
} vo_layer_ddr;

/* vou mixer prio id */
typedef enum {
    VOU_MIX_PRIO0 = 0,
    VOU_MIX_PRIO1,
    VOU_MIX_PRIO2,
    VOU_MIX_PRIO3,
    VOU_MIX_PRIO4,
    VOU_MIX_PRIO5,
    VOU_MIX_PRIO_CLR,
    VOU_MIX_BUTT
} vou_mix_prio;

typedef enum {
    /* for video surface interface */
    VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_400 = 0x1,
    VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_420 = 0x2,
    VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_422 = 0x3,
    VOU_LAYER_PIXERL_FORMAT_BUTT
} vou_layer_pixerl_format;

typedef enum {
    /* for device interface */
    VOU_INTF_PIXERL_FORMAT_YCBCR422 = 0x0,
    VOU_INTF_PIXERL_FORMAT_YCBCR444 = 0xC,
    VOU_INTF_PIXERL_FORMAT_RGB888 = 0xC,
    VOU_INTF_PIXERL_FORMAT_BUTT
} vou_intf_pixerl_format;

typedef enum {
    VO_CSC_DEF_VAL = 50,
    VO_CSC_LUMA_MAX = 100,
    VO_CSC_LUMA_MIN = 0,

    VO_CSC_CONT_MAX = 100,
    VO_CSC_CONT_MIN = 0,

    VO_CSC_HUE_MAX = 100,
    VO_CSC_HUE_MIN = 0,

    VO_CSC_SAT_MAX = 100,
    VO_CSC_SAT_MIN = 0,
} vo_csc_range;

/* vou zoom mode */
typedef enum {
    VOU_DISP_ZMEMODE_HORL = 0,
    VOU_DISP_ZMEMODE_HORC,
    VOU_DISP_ZMEMODE_VERL,
    VOU_DISP_ZMEMODE_VERC,

    VOU_DISP_ZMEMODE_HOR,
    VOU_DISP_ZMEMODE_VER,
    VOU_DISP_ZMEMODE_ALPHA,
    VOU_DISP_ZMEMODE_ALPHAV,
    VOU_DISP_ZMEMODE_VERT,
    VOU_DISP_ZMEMODE_VERB,

    VOU_DISP_ZMEMODE_ALL,
    VOU_DISP_ZMEMODE_NONL,
    VOU_DISP_ZMEMODE_BUTT
} vou_disp_zmemode;

typedef enum {
    VO_HD_HW_DEV = 0,  /* HD dev */
    VO_SD_HW_DEV,  /* SD dev */
    VO_UHD_HW_DEV,  /* UHD dev */
    VO_CAS_DEV,  /* cascade dev */
    VO_VIRT_DEV,  /* virtual dev */
    VO_DEV_TYPE_BUTT,
} vo_dev_type;

typedef struct {
    vo_dev_type dev_type; /* dev type */
    hi_bool hd_vpss_rciv; /* */
    hi_bool sd_tde_disp; /* */
    hi_bool support_wbc; /* WBC support or not */
    hi_bool spt_get_screen_frm; /* */
} vo_dev_capability;

typedef enum {
    ICE_BIT_DEPTH_8 = 0,
    ICE_BIT_DEPTH_10 = 1,
    ICE_BIT_DEPTH_12 = 2,
    ICE_BIT_DEPTH_14 = 3,
    ICE_BIT_DEPTH_BUTT
} ice_bit_depth;

typedef enum {
    ICE_REG_CFG_MODE_TYP = 0,
    ICE_REG_CFG_MODE_MIN = 1,
    ICE_REG_CFG_MODE_MAX = 2,
    ICE_REG_CFG_MODE_RAND = 3,
    ICE_REG_CFG_MODE_MINMAX = 4,
    ICE_REG_CFG_MODE_MANUAL = 5,
    ICE_REG_CFG_MODE_BUTT
} ice_reg_cfg_mode;

typedef enum {
    ICE_DATA_FMT_YUV420 = 1,
    ICE_DATA_FMT_YUV422 = 2,
    ICE_DATA_FMT_BAYER = 3,
    ICE_DATA_FMT_BUTT
} ice_data_fmt;

typedef enum {
    ICE_BAYER_FMT_GRBG = 0,
    ICE_BAYER_FMT_GBRG = 1,
    ICE_BAYER_FMT_RGGB = 2,
    ICE_BAYER_FMT_BGGR = 3,
    ICE_BAYER_FMT_BUTT
} ice_bayer_fmt;

typedef struct {
    /* input */
    hi_u32 layer;
    hi_u32 frame_wth;
    hi_u32 frame_hgt;
    hi_u32 tile_hgt;
    hi_u32 cmp_ratio;
    hi_bool is_lossy;
    hi_bool chm_proc_en;
    hi_bool esl_en;
    ice_bit_depth bit_depth;
    ice_data_fmt data_fmt;
    ice_bayer_fmt bayer_fmt;
    ice_reg_cfg_mode cmp_cfg_mode;
    /* file name */
    char **cmp_file_name;
    /* output */
    hi_u32 *cmp_size;
    /* debug */
    hi_u32 sti_err_type;
} cmp_seg_info;

typedef enum {
    OSD_CMP_MODE_TYP = 0,
    OSD_CMP_MODE_RAND,
    OSD_CMP_MODE_BUTT
} osd_cmp_mode;

typedef struct {
    /* input */
    hi_u32 layer;
    osd_cmp_mode cmp_cfg_mode;
    hi_u32 cmp_ratio;
    /* file name */
    char **cmp_file_name;
    /* output */
    hi_u32 *cmp_size;
    /* debug */
    hi_u32 sti_err_type;
} cmp_frm_info;

typedef struct {
    hi_u64 lum_addr;
    hi_u64 chm_addr;
    hi_u32 lum_str;
    hi_u32 chm_str;
} vdp_cap_addr_info;

typedef enum {
    VDP_CAP_ADDR_DATA = 0,  /* 2d left eye data */
    VDP_CAP_ADDR_HEAD,  /* dcmp head data */
    VDP_CAP_ADDR_3D,  /* 3d right eye data */
    VDP_CAP_ADDR_BUTT
} vdp_cap_addr;

typedef enum {
    VDP_CAP_INT_MODE_FRAME = 0,
    VDP_CAP_INT_MODE_FIELD,
    VDP_CAP_INT_BUTT
} vdp_cap_int_mode;

typedef struct {
    hi_bool cmp_en;
    hi_u32 cmp_ratio;
    hi_bool is_lossy;
    hi_bool esl_en;
    ice_bit_depth bit_depth;
    ice_data_fmt data_fmt;
    ice_reg_cfg_mode cmp_cfg_mode;

    hi_u32 lum_cmp_ratio;
    hi_u32 chm_cmp_ratio;
    hi_bool is_lum_lossy;
    hi_bool is_chm_lossy;
    hi_bool is_esl_mode;
    hi_bool is_raw_mode;
} cmp_seg_cfg;

typedef struct {
    /* src info */
    vdp_cap_addr_info addr[VDP_CAP_ADDR_BUTT];
    vdp_data_wth data_width;
    vdp_proc_fmt data_fmt;
    vdp_rect src_rect;
    hi_u32 finfo;
    /* bus */
    vdp_req_length req_len;
    hi_u32 interval;
    /* operation */
    cmp_seg_cfg seg_cfg;
    /* operation */
    hi_bool flip_en;  /* reserved */
    hi_bool uv_order;  /* reserved */
    /* mmu */
    hi_bool smmu_en;  /* reserved */
} vdp_cap_cfg;

typedef struct {
    /* basic cfg parameter */
    hi_s32 frm_hgt;
    hi_s32 frm_wth;
    hi_s32 bit_depth;  /* 8,10,12,14 */
    hi_s32 lossy;
    hi_s32 format;  /* 1:420 2:422 3:bayer */
    hi_s32 bayer_format;  /* RANGE:0-2    TYPICAL 0:GRBG     1:GBRG    2:RGGB   3:BGGR */
    hi_s32 esl_en;
    hi_s32 esl_mode_qp;
    hi_s32 cmp_ratio_pct_y;
    hi_s32 type;  /* */
    hi_s32 til_hgt;
    hi_s32 c_en;
    /* when type == 5, modifiable cfg parameter */
    hi_s32 line_buf_ref_sel;
    hi_s32 dither_qp;
    hi_s32 tol_num_y;
    hi_s32 grad_ave_thr0_y;
    hi_s32 grad_ave_thr1_y;
    hi_s32 grad_ave_thr2_y;
    hi_s32 simi_fct_thr0_y;
    hi_s32 simi_fct_thr1_y;
    hi_s32 simi_fct_thr2_y;
    hi_s32 mb_offset_thr_y;
    hi_s32 mb_offset_enable_y;
    hi_s32 rc_qp_offset_y;
    hi_s32 bgn_k_y;
    hi_s32 max_k_y;
    hi_s32 adp_k_y;
    hi_s32 escape_intreim_y;
    hi_s32 intreim_table_y;
    hi_s32 round_y;
    hi_s32 max_qp_offset_y;
    hi_s32 dither_segots_y;
    hi_s32 dither_ots_y;
    hi_s32 dither_glb_y;
    hi_s32 dither_dts_y;
    hi_s32 dither_msq_y;

    hi_s32 is_enc;

    hi_s32 frm_num;
    hi_s32 start_frm_num;
    hi_s32 seg_wth;
    hi_s32 mb_wth;
    hi_s32 seg_ratio_maxpct;
    hi_s32 tile_input;
    hi_s32 mb_mode_sel_grad_thr;
    hi_s32 seg_bst_align;

    hi_s32 factor;
    hi_s32 mb_independent;
    hi_s32 raw_wdr_test;
    hi_s32 pred_1ppc;

    hi_s32 cmp_ratio_pct;

    hi_s32 err_en;
} para_input;

typedef struct file vo_file;

typedef struct {
    int frm_start;
    int frm_len;

    int frame_width;
    int frame_height;
    int bitdepth;
    int cmp_mode;  /* 0 --compress   1-- bypass raw */
    int is_lossless;
    int comp_ratio_int;  /* *1000 */
    int pixel_format;  /* 1:420 2:422 3:bayer */
    int chroma_en;
    int tile_en;
    int tile_height;
    int mirror_en;
    int err_en;
    int type;
    /* when type == 5, modifiable cfg parameter */
    int grad_ave_thr0;
    int grad_ave_thr1;
    int grad_ave_thr2;
    int simi_fct_thr0;
    int simi_fct_thr1;
    int simi_fct_thr2;
    int rc_qp_offset;
    int fix_k;
    int max_qp_offset;
    int seg_ratio_maxpct;
    int mb_mode_sel_grad_thr;
    int dither_qp;
    int dither_segots;
    int dither_ots;
    int dither_glb;
    int dither_dts;
    int dither_msq;
} ice_v2r1_seg_256x1_para_input;

typedef struct {
    hi_u32 fbdiv;
    hi_u32 frac;
    hi_u32 refdiv;
    hi_u32 postdiv1;
    hi_u32 postdiv2;
    hi_u32 hdmiclk_div;
} vo_intfsync_pll;

typedef struct {
    hi_u32 mirror_en;
    hi_u16 cmp_mode;
    hi_u16 frame_width;
    hi_u16 frame_height;
    hi_u16 chroma_en;
    hi_u16 bitdepth;
    hi_u16 tile_en;
    hi_u16 tile_height;
    hi_u16 is_lossless;
    hi_u32 slc_cfg_tar_bit;
    hi_u32 seg_cfg_max_bit;
    hi_u32 seg_cfg_tar_bit;
    hi_u32 max_qp;

    hi_u16 dither_qp;
    hi_u16 dither_segots;
    hi_u16 dither_ots;
    hi_u16 dither_glb;
    hi_u16 dither_dts;
    hi_u16 dither_msq;

    /* tmu */
    hi_u16 grad_ave_thr0;
    hi_u16 grad_ave_thr1;
    hi_u16 grad_ave_thr2;
    hi_u16 simi_fct_thr0;
    hi_u16 simi_fct_thr1;
    hi_u16 simi_fct_thr2;
    hi_u16 rc_qp_offset;
    hi_u16 fix_k;
    hi_u16 mb_mode_sel_grad_thr;

    vo_file *error;
    hi_u32 err_en;
    hi_u32 cmp_ratio;
} ice_v2r1_seg_256x1_rm_cfg;

/* ICE definition */
typedef enum {
    ICE_ERROR_TYPE_NORMAL = 0,
    ICE_ERROR_TYPE_HEAD_ZERO,
    ICE_ERROR_TYPE_HEAD_RAND,
    ICE_ERROR_TYPE_HEAD_MAX,
    ICE_ERROR_TYPE_DATA_ZERO,
    ICE_ERROR_TYPE_DATA_RAND,
    ICE_ERROR_TYPE_DATA_MAX,
    ICE_ERROR_TYPE_FRAMESIZE_MAX,
    ICE_ERROR_TYPE_FRAMESIZE_RAND,
    ICE_ERROR_TYPE_FRAMESIZE_ZERO,
    ICE_ERROR_TYPE_BUTT
} ice_error_type;

typedef struct {
    hi_u32 layer;
    hi_u32 frame_wth;
    hi_u32 frame_hgt;
    hi_u32 tile_hgt;
    hi_u32 cmp_ratio_y;
    hi_u32 cmp_ratio_c;
    hi_bool cmp_en;
    hi_bool is_raw_en;
    hi_bool is_lossy_y;
    hi_bool is_lossy_c;
    hi_bool chm_proc_en;
    hi_bool esl_en;
    hi_bool csc_en;
    ice_bit_depth bit_depth;
    ice_data_fmt data_fmt;
    ice_reg_cfg_mode cmp_cfg_mode;
    /* file name */
    char *cmp_info;
    char **cmp_file_name;
    /* output */
    hi_u32 *cmp_size;
    /* debug */
    ice_error_type err_type;
} ice_seg_cfg;

typedef struct {
    /* vid layer */
    hi_u64 vid_lum_data_addr[VID_MAX];
    hi_u64 vid_chm_data_addr[VID_MAX];
    hi_u64 vid_lum_head_addr[VID_MAX];
    hi_u64 vid_chm_head_addr[VID_MAX];
    hi_u64 vid_lum2_bit_addr[VID_MAX];
    hi_u64 vid_chm2_bit_addr[VID_MAX];
    hi_u64 vid_lum_tile_addr[VID_MAX];
    hi_u64 vid_chm_tile_addr[VID_MAX];
    hi_u32 vid_lum_data_str[VID_MAX];
    hi_u32 vid_chm_data_str[VID_MAX];
    hi_u32 vid_lum_head_str[VID_MAX];
    hi_u32 vid_chm_head_str[VID_MAX];
    hi_u32 vid_lum2_bit_str[VID_MAX];
    hi_u32 vid_chm2_bit_str[VID_MAX];
    hi_u32 vid_lum_tile_str[VID_MAX];
    hi_u32 vid_chm_tile_str[VID_MAX];
    /* gfx layer */
    hi_u64 gfx_data_addr[GFX_MAX];
    hi_u64 gfx_dcmp_addr[GFX_MAX];
    hi_u32 gfx_data_str[GFX_MAX];
    hi_u32 gfx_dcmp_str[GFX_MAX];
    /* wbc layer */
    hi_u64 wbc_lum_data_addr[WBC_MAX];
    hi_u64 wbc_chm_data_addr[WBC_MAX];
    hi_u64 wbc_lum_head_addr[WBC_MAX];
    hi_u64 wbc_chm_head_addr[WBC_MAX];
    hi_u32 wbc_lum_data_str[WBC_MAX];
    hi_u32 wbc_chm_data_str[WBC_MAX];
    hi_u32 wbc_lum_head_str[WBC_MAX];
    hi_u32 wbc_chm_head_str[WBC_MAX];
    /* para */
    hi_u64 para_addr[PARA_MAX];
} vo_addr_cfg;

typedef enum {
    VDP_FDR_CFG_MODE_TYP,
    VDP_FDR_CFG_MODE_TYP1,
    VDP_FDR_CFG_MODE_RAND,

    VDP_FDR_CFG_MODE_BUTT
} vdp_fdr_cfg_mode;

typedef struct {
    hi_u64 lum_addr;
    hi_u64 chm_addr;

    hi_u32 lum_str;
    hi_u32 chm_str;
} vdp_vid_addr_info;

typedef enum {
    VDP_VID_ADDR_DATA = 0,  /* 2d left eye data */
    VDP_VID_ADDR_2B,  /* 2bit */
    VDP_VID_ADDR_HEAD,  /* dcmp head data */
    VDP_VID_ADDR_3D,  /* 3d right eye data */
    VDP_VID_ADDR_TILE_8B,  /* tile 8bit */
    VDP_VID_ADDR_BUTT
} vdp_vid_addr;

typedef struct {
    hi_u64 addr;
    hi_u32 str;
} vdp_gfx_addr_info;

typedef enum {
    VDP_GFX_ADDR_DATA = 0,  /* 2d left eye data */
    VDP_GFX_ADDR_3D,  /* 3d right eye data */
    VDP_GFX_ADDR_DCMP,  /* dcmp head data */
    VDP_GFX_ADDR_DCMP_3D,  /* dcmp head data */
    VDP_GFX_ADDR_BUTT
} vdp_gfx_addr;

typedef enum {
    VDP_TESTPTTN_CLR_MODE_FIX = 0,
    VDP_TESTPTTN_CLR_MODE_NORM,
    VDP_TESTPTTN_CLR_MODE_BUTT
} vdp_testpttn_clr_mode;

typedef enum {
    VDP_DCMP_SRC_MODE_ARGB8888 = 0,
    VDP_DCMP_SRC_MODE_ARGB1555,
    VDP_DCMP_SRC_MODE_ARGB4444,
    VDP_DCMP_SRC_MODE_RGB565,
    VDP_DCMP_SRC_MODE_RGB888,
    VDP_DCMP_SRC_MODE_YUV444,
    VDP_DCMP_SRC_MODE_BUTT
} vdp_dcmp_src_mode;

typedef enum {
    VDP_CMP_MODE_CMP = 0,
    VDP_CMP_MODE_RAW,
    VDP_CMP_MODE_BUTT
} vdp_cmp_mode;

typedef enum {
    VDP_TESTPTTN_LINEW_1P = 0,
    VDP_TESTPTTN_LINEW_2P,
    VDP_TESTPTTN_LINEW_BUTT
} vdp_testpttn_linew;

typedef struct {
    hi_bool dcmp_en;
    hi_bool is_esl_mode;
    hi_bool is_raw_mode;
    hi_bool is_lum_lossy;
    hi_bool is_chm_lossy;
    ice_reg_cfg_mode lum_cmp_mode;
    ice_reg_cfg_mode chm_cmp_mode;
    hi_u32 lum_cmp_ratio;  /* reserved */
    hi_u32 chm_cmp_ratio;  /* reserved */
} vdp_dcmp_seg_cfg;

typedef struct {
    hi_u64 addr;
    hi_u32 str;
} vdp_addr;

#define MAX_REGION_NUM        64
#define V0_REGION_NUM         64
#define V1_REGION_NUM         16
#define V2_REGION_NUM         0
/* one region has 0x30 bytes(0x12000 ~ 0x12028), one region has 16( = 0x30 / 4) registers */
#define REGION_OFFSET         (0x30 / 4)

typedef enum {
    VDP_REGION_ADDR_DATA_LUM = 0,
    VDP_REGION_ADDR_DATA_CHM,
    VDP_REGION_ADDR_HEAD_LUM,
    VDP_REGION_ADDR_HEAD_CHM,
    VDP_REGION_ADDR_BUTT
} vdp_region_addr;

typedef enum {
    VDP_REGION_COLOR_MODE_FORE = 0,
    VDP_REGION_COLOR_MODE_BACK,
    VDP_REGION_COLOR_MODE_BUTT
} vdp_region_edge_color_mode;

typedef enum {
    VDP_REGION_MUTE_MODE_SHOW = 0,
    VDP_REGION_MUTE_MODE_NOSHOW,
    VDP_REGION_MUTE_MODE_BUTT
} vdp_region_mute_mode;

typedef struct {
    hi_bool region_en;
    hi_bool mute_en;
    hi_bool mmu_bypass;
    hi_bool crop_en;
    hi_bool edge_en;
    hi_bool dcmp_en;
    vdp_region_edge_color_mode edge_color_type;
    vdp_rect disp_reso;
    vdp_rect src_reso;
    vdp_rect in_reso;
    vdp_addr addr[VDP_REGION_ADDR_BUTT];
} vdp_region_info;

typedef struct {
    hi_u32 bkg_y;
    hi_u32 bkg_u;
    hi_u32 bkg_v;

    hi_u32 bkg_a;

    hi_u32 bk_type;
} vo_drv_bkg;

typedef struct {
    hi_bool multi_region_en;
    vdp_region_mute_mode multi_region_mute_mode;
    hi_u32 region_num;
    vdp_region_info region_info[MAX_REGION_NUM];
} vdp_vid_fdr_mrg_cfg;

typedef struct {
    /* src info */
    vdp_vid_addr_info addr[VDP_VID_ADDR_BUTT];

    vdp_vid_ifmt in_fmt;
    vdp_data_wth data_width;

    vdp_rect src_rect;
    vdp_rect in_rect;

    hi_bool smmu_en;  /* reserved */

    vdp_data_rmode read_mode;  /* lm & chm */
    hi_bool chm_copy_en;  /* reserved */

    /* dcmp */
    vdp_dcmp_seg_cfg dcmp_cfg;

    /* operation */
    hi_bool crop_en;
    hi_bool flip_en;
    hi_bool uv_order;

    /* fdr mute */
    hi_bool fdr_mute_en;
    hi_bool fdr_mute_req_en;

    vo_drv_bkg fdr_mute_bkg;  /* R,G,B */

    /* mac */
    hi_bool pre_rd_en;
    hi_bool ofl_master;  /* reserved */
    vdp_req_length req_len;

    vdp_vid_fdr_mrg_cfg *mrg_cfg;

    /* memory */
    hi_bool is_single_port;
    hi_bool mem_mode;
} vdp_vid_fdr_cfg;

typedef struct {
    hi_bool dcmp_en;
    vdp_dcmp_src_mode dcmp_src_mode;
    vdp_cmp_mode argb_cmp_mode;
    hi_bool cmp_lostless;
    hi_bool cmp_lostless_a;

    hi_u32 dcmp_frame_size0;
    hi_u32 dcmp_frame_size1;
} vdp_osd_dcmp_cfg;

typedef struct {
    /* src info */
    vdp_gfx_addr_info addr[VDP_GFX_ADDR_BUTT];

    vdp_gfx_ifmt in_fmt;

    vdp_rect src_rect;

    hi_bool pre_mult_en;

    /* palpha */
    hi_bool palpha_en;
    hi_bool palpha_range_is_full;  /* 0: 0~128, 1: 0~255 */

    /* for 1555 */
    hi_u32 alpha0;
    hi_u32 alpha1;

    /* read way */

    hi_bool flip_en;

    /* pixel value calc */
    hi_bool ckey_en;

    /* fdr mute */
    hi_bool fdr_mute_en;
    hi_bool fdr_mute_req_en;

    /* dcmp */
    vdp_osd_dcmp_cfg dcmp_cfg;

    /* mac */
    hi_bool ofl_master;
    vdp_req_length req_len;

    /* mmu */
    hi_bool smmu_bypass_en;

    /* memory */
    hi_bool port_mode;

    /* axi */
    hi_bool latency_clr;
} vdp_gfx_fdr_cfg;

#define VO_ALL_COEF_SIZE      2000000
#define VO_COEF_OVER_SIZE     128
#define COEF_SIZE_V0_HZME     (4096 * 128 / 8)
#define COEF_SIZE_V0_VZME     (4096 * 128 / 8)
#define COEF_SIZE_V0_HDRD     (4096 * 128 / 8)
#define COEF_SIZE_V0_HDRS     (4096 * 128 / 8)
#define COEF_SIZE_G0_HDR_TMAP (4096 * 128 / 8)
#define COEF_SIZE_G0_HDR_GMM  (4096 * 128 / 8)
#define COEF_SIZE_G0ZME       (4096 * 128 / 8)
#define COEF_SIZE_WD_HZME     (4096 * 128 / 8)
#define COEF_SIZE_WD_VZME     (4096 * 128 / 8)
#define COEF_SIZE_WD_HDRD     (4096 * 128 / 8)
#define COEF_SIZE_WD_HDRS     (4096 * 128 / 8)
#define COEF_SIZE_REGION_V0   (192 * 128 / 8)
#define COEF_SIZE_REGION_V1   (48 * 128 / 8)

#define COEF_SIZE_SHARPEN     (4096 * 128 / 8)  /* (30  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_DCI         (4096 * 128 / 8)  /* (35  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_ACM         (4096 * 128 / 8)  /* (884 * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_VZME        (4096 * 128 / 8)  /* (33  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_HZME        (4096 * 128 / 8)  /* (33  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_GPZME       (4096 * 128 / 8)  /* (67  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_DIM         (4096 * 128 / 8)  /* (127 * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_GMM         (4096 * 128 / 8)  /* (98  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_OD          (4096 * 128 / 8)  /* (206 * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_CLUT        (4096 * 128 / 8)  /* (63  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_PCID        (4096 * 128 / 8)  /* (37  * 128/8  + VO_COEF_OVER_SIZE) */
#define COEF_SIZE_V0_ZME      (4096 * 128 / 8)  /* (37  * 128/8  + VO_COEF_OVER_SIZE) */

/* video buf size: stride*h*2(flip)*2(3d) = (4096*10/8)*2160*2*2 ~= 0x3000000 */
#define VO_BUF_SIZE_V0        0x3000000
#define VO_BUF_SIZE_V1        0x3000000
#define VO_BUF_SIZE_V2        0x3000000
#define VO_BUF_SIZE_V3        0x3000000
#define VO_BUF_SIZE_V4        0x3000000

typedef struct file vo_file;

typedef enum {
    STI_FILE_DATA_VID0 = 0,
    STI_FILE_DATA_VID1,
    STI_FILE_DATA_VID2,
    STI_FILE_DATA_VID3,
    STI_FILE_DATA_VID4,
    STI_FILE_DATA_GFX0,
    STI_FILE_DATA_GFX1,
    STI_FILE_DATA_GFX2,
    STI_FILE_DATA_GFX3,
    STI_FILE_DATA_GFX4,

    STI_FILE_COEF_SR,
    STI_FILE_COEF_ZME_V0,
    STI_FILE_COEF_SHARPEN,
    STI_FILE_COEF_ACM,
    STI_FILE_COEF_DCI,
    STI_FILE_COEF_DIM,
    STI_FILE_COEF_GMM,
    STI_FILE_COEF_CLUT,
    STI_FILE_COEF_HZME,
    STI_FILE_COEF_VZME,
    STI_FILE_COEF_GPZME,
    STI_FILE_COEF_OD,
    STI_FILE_COEF_PCID,
    STI_FILE_COEF_RGBW,
    STI_FILE_COEF_DEMURA,
    STI_FILE_TYPE_NODEF,
    STI_FILE_V0_Y,
    STI_FILE_V0_C,
    STI_FILE_TILE_64_CMP_Y,
    STI_FILE_TILE_64_CMP_C,
    STI_FILE_TILE_64_CMP_Y_2BIT,
    STI_FILE_TILE_64_CMP_C_2BIT,
    STI_FILE_TILE_64_CMP_Y_HEAD,
    STI_FILE_TILE_64_CMP_C_HEAD,
    STI_FILE_OSD_CMP,
    STI_FILE_COEF_DM,
    STI_FILE_COEF_GDM,
    STI_FILE_COEF_METADATA,
    STI_FILE_DATA_TNR_REF,
    STI_FILE_DATA_TNR_MAD,
    STI_FILE_DATA_TNR_RGMV,
    STI_FILE_TNR_REF_CMP_Y,
    STI_FILE_COEF_TONEMAP,

    STI_FILE_DATA_VMX,
    STI_FILE_COEF_HIHDR_V_D,
    STI_FILE_COEF_HIHDR_V,
    STI_FILE_COEF_HIHDR_G_TMAP,
    STI_FILE_COEF_HIHDR_G_GMM,
    STI_FILE_COEF_G0ZME,
    STI_FILE_COEF_WD_HZME,
    STI_FILE_COEF_WD_VZME,
    STI_FILE_COEF_HIHDR_WD_D,
    STI_FILE_COEF_HIHDR_WD,

    STI_FILE_TYPE_BUTT
} sti_file_type;

typedef struct {
    hi_u32 data3;
    hi_u32 data2;
    hi_u32 data1;
    hi_u32 data0;
    hi_u32 depth;
} vo_drv_u128;

typedef enum {
    DRV_COEF_DATA_TYPE_U8 = 0,
    DRV_COEF_DATA_TYPE_S8,
    DRV_COEF_DATA_TYPE_U16,
    DRV_COEF_DATA_TYPE_S16,
    DRV_COEF_DATA_TYPE_U32,
    DRV_COEF_DATA_TYPE_S32,
    DRV_COEF_DATA_TYPE_BUTT
} vo_drv_coef_data_type;

typedef struct {
    hi_u8 *coef_addr;
    sti_file_type sti_type;
    vo_file *fp_coef;
    hi_u32 lut_num;
    hi_u32 burst_num;
    hi_u32 cycle_num;
    void **coef_array;
    hi_u32 *lut_length;
    hi_u32 *coef_bit_length;
    vo_drv_coef_data_type data_type;
} vo_drv_coef_send_cfg;

typedef enum {
    VO_RM_COEF_MODE_TYP = 0x0,
    VO_RM_COEF_MODE_RAN = 0x1,
    VO_RM_COEF_MODE_MIN = 0x2,
    VO_RM_COEF_MODE_MAX = 0x3,
    VO_RM_COEF_MODE_ZRO = 0x4,
    VO_RM_COEF_MODE_CUS = 0x5,
    VO_RM_COEF_MODE_UP = 0x6,
    VO_RM_COEF_MODE_BUTT
} vo_rm_coef_mode;

typedef struct {
    hi_u64 start_phy_addr;
    hi_void *start_vir_addr;
    hi_u32 size;
} vo_mmz_buffer;

typedef enum {
    VO_BUF_V0 = 0,
    VO_BUF_V1,
    VO_BUF_V2,
    VO_BUF_V3,
    VO_BUF_V4,
    VO_BUF_G0,
    VO_BUF_G1,
    VO_BUF_G2,
    VO_BUF_G3,
    VO_BUF_G4,
    VO_BUF_G5,
    VO_BUF_BUTT
} vo_layer_buf;

typedef enum {
    VO_COEF_BUF_V0_HZME = 0,
    VO_COEF_BUF_V0_VZME = 1,
    VO_COEF_BUF_V0_HDRD = 2,
    VO_COEF_BUF_V0_HDRS = 3,
    VO_COEF_BUF_G0_HDR_TMAP = 4,
    VO_COEF_BUF_G0_HDR_GMM = 5,
    VO_COEF_BUF_G0ZME = 6,
    VO_COEF_BUF_WD_HZME = 7,
    VO_COEF_BUF_WD_VZME = 8,
    VO_COEF_BUF_WD_HDRD = 9,
    VO_COEF_BUF_WD_HDRS = 10,

    VO_COEF_BUF_REGION_V0 = 12,
    VO_COEF_BUF_REGION_V1 = 13,
    VO_COEF_BUF_SHARPEN,
    VO_COEF_BUF_DCI,
    VO_COEF_BUF_ACM,
    VO_COEF_BUF_VZME,
    VO_COEF_BUF_HZME,
    VO_COEF_BUF_GPZME,
    VO_COEF_BUF_DIM,
    VO_COEF_BUF_GMM,
    VO_COEF_BUF_OD,
    VO_COEF_BUF_CLUT,
    VO_COEF_BUF_PCID,

    VO_COEF_BUF_ALL = 32,
    VO_COEF_BUF_BUTT = 33
} vo_coef_buf;

typedef struct {
    vo_mmz_buffer buf_base_addr;
    hi_u32 u32size;

    hi_u8 *coef_vir_addr[VO_COEF_BUF_BUTT];
    hi_u64 coef_phy_addr[VO_COEF_BUF_BUTT];
} vo_coef_addr;

typedef struct {
    vo_mmz_buffer buf_base_addr;
    hi_u32 u32size;

    hi_u32 buf_addr[VO_BUF_BUTT];
} vo_layer_buf_addr;

typedef enum {
    VO_HIHDR_V_TYP = 0,
    VO_HIHDR_V_TYP1,
    VO_HIHDR_V_RAND,
    VO_HIHDR_V_MAX,
    VO_HIHDR_V_MIN,
    VO_HIHDR_V_ZERO,
    VO_HIHDR_V_BUTT
} vo_hihdr_v_mode;

typedef enum {
    VO_VHDR_GAMUT_TO_TONEMAP = 0,
    VO_VHDR_TONEMAP_TO_GAMUT = 1
} vo_vhdr_gamut_bind;

typedef enum {
    VO_VHDR_Y2R_BT709 = 0,
    VO_VHDR_Y2R_BT2020 = 1,
    VO_VHDR_Y2R_XVYCC = 2,
    VO_VHDR_Y2R_BUTT
} vo_vhdr_y2r_cfg_mode;

typedef enum {
    VO_CSC_V0_TYP = 0,
    VO_CSC_V0_TYP1,
    VO_CSC_V0_RAND,
    VO_CSC_V0_MAX,
    VO_CSC_V0_MIN,
    VO_CSC_V0_ZERO,
    VO_CSC_V0_BUTT
} vo_csc_v0_mode;

typedef enum {
    VHDR_HDR10_IN_SDR_OUT = 0,
    VHDR_HDR10_IN_HLG_OUT,
    VHDR_HLG_IN_SDR_OUT,
    VHDR_HLG_IN_HDR10_OUT,
    VHDR_SDR10_IN_SDR8_OUT,
    VHDR_XVYCC,
    VHDR_SDR2020CL_IN_709_OUT,
    VHDR_BUTT
} vhdr_scene_mode;

typedef struct {
    /* module ctrl */
    hi_u32 vhdr_en;
    hi_u32 vhdr_ck_gt_en;
    hi_u32 vhdr_degmm_en;
    hi_u32 vhdr_gamut_en;
    hi_u32 vhdr_tmap_en;
    hi_u32 vhdr_gmm_en;
    hi_u32 vhdr_dither_en;
    hi_u32 vhdr_r2y_en;
    hi_u32 vhdr_cadj_en;
    hi_u32 vhdr_gamut_bind;

    /* dither ctrl */
    hi_u32 vhdr_dither_round_unlim;
    hi_u32 vhdr_dither_round;
    hi_u32 vhdr_dither_domain_mode;
    hi_u32 vhdr_dither_tap_mode;

    /* hi_hdr scene */
    vhdr_scene_mode scene_mode;
} vo_hihdr_v_cfg;

typedef struct {
    /* module ctrl */
    hi_u32 hihdr_y2r_en;
    hi_u32 hihdr_y2r_ck_gt_en;

    /* PIP ctrl */
    hi_u32 hihdr_v0_y2r_en;
    hi_u32 hihdr_v1_y2r_en;
    hi_u32 hihdr_v0_vhdr_en;
    hi_u32 hihdr_v1_vhdr_en;
    hi_u32 hihdr_v0_cl_en;
    hi_u32 hihdr_v1_cl_en;

    /* demo ctrl */
    hi_u32 hihdr_v_demo_en;
    hi_u32 hihdr_v_demo_mode;
    hi_u32 hihdr_v_demo_pos;

    /* hi_hdr scene */
    vhdr_scene_mode scene_mode;
} vo_csc_v0_cfg;

typedef struct {
    hi_u32 ck_gt_en;
    hi_u32 hfir_order;
    hi_u32 lhfir_en;
    hi_u32 chfir_en;
    hi_u32 lhmid_en;
    hi_u32 chmid_en;
    hi_u32 non_lnr_en;
    hi_u32 lhfir_mode;
    hi_u32 chfir_mode;
    hi_u32 hl_shootctrl_en;
    hi_u32 hl_shootctrl_mode;
    hi_u32 hc_shootctrl_en;
    hi_u32 hc_shootctrl_mode;

    hi_u32 in_pro;
    hi_u32 out_pro;
    hi_u32 out_fmt;
    hi_u64 in_height;
    hi_u64 out_height;
    hi_u64 in_width;
    hi_u64 out_width;
    hi_u32 graphdet_en;

    hi_u32 lvfir_en;
    hi_u32 cvfir_en;
    hi_u32 lvmid_en;
    hi_u32 cvmid_en;
    hi_u32 vfir_1tap_en;
    hi_u32 lvfir_mode;
    hi_u32 cvfir_mode;
    hi_u32 vl_shootctrl_en;
    hi_u32 vl_shootctrl_mode;
    hi_u32 vc_shootctrl_en;
    hi_u32 vc_shootctrl_mode;
} vo_v0_zme_cfg;

typedef struct {
    hi_u32 vluma_offset;
    hi_u32 vchroma_offset;
    hi_u32 vbluma_offset;
    hi_u32 vbchroma_offset;
    hi_u32 lhfir_offset;
    hi_u32 chfir_offset;
    hi_u32 vl_flatdect_mode;
    hi_u32 vl_coringadj_en;
    hi_u32 vl_gain;
    hi_u32 vl_coring;
    hi_u32 vc_flatdect_mode;
    hi_u32 vc_coringadj_en;
    hi_u32 vc_gain;
    hi_u32 vc_coring;
    hi_u32 hl_flatdect_mode;
    hi_u32 hl_coringadj_en;
    hi_u32 hl_gain;
    hi_u32 hl_coring;
    hi_u32 hc_flatdect_mode;
    hi_u32 hc_coringadj_en;
    hi_u32 hc_gain;
    hi_u32 hc_coring;
} vo_zme_comm_pq_cfg;

typedef struct {
    hi_u32 zme_vprec;
    hi_u32 zme_hprec;
} vo_zme_ds_info;

typedef enum {
    VO_ZME_TYP = 0,
    VO_ZME_TYP1,
    VO_ZME_RAND,
    VO_ZME_MAX,
    VO_ZME_MIN,
    VO_ZME_ZERO,
    VO_ZME_BUTT
} vo_zme_mode;

typedef enum {
    XDP_ZME_HFIR_ORDER_HV = 0,
    XDP_ZME_HFIR_ORDER_VH,
    XDP_ZME_HFIR_ORDER_BUTT
} xdp_zme_hfir_order;

typedef enum {
    VDP_V0_HFIR_TYP = 0,
    VDP_V0_HFIR_TYP1,
    VDP_V0_HFIR_RAND,
    VDP_V0_HFIR_MAX,
    VDP_V0_HFIR_MIN,
    VDP_V0_HFIR_ZERO,
    VDP_V0_HFIR_BUTT
} vo_v0_hfir_mode;

typedef enum {
    XDP_V0_HFIR_BYPASS = 0,
    XDP_V0_HFIR_COPY,
    XDP_V0_HFIR_BILT,
    XDP_V0_HFIR_FILT,
    XDP_V0_HFIR_BUTT
} xdp_v0_hfir_mode;

typedef struct {
    hi_u32 ck_gt_en;  /* use for cfg ck_gt_en */
    hi_u32 mid_en;
    xdp_v0_hfir_mode hfir_mode;  /* use for cfg mode */
} vo_v0_hfir_cfg;

typedef struct {
    hi_u32 vid_iw;
    hi_u32 vid_ih;
    hi_u32 vid_ow;
    hi_u32 vid_oh;

    hi_u32 zme_en;
    hi_u32 hfir_en;
    hi_u32 csc_en;
    hi_u32 hdr_en;

    vo_v0_zme_cfg zme_cfg;
    vo_v0_hfir_cfg hfir_cfg;
    vo_csc_v0_cfg yuv2rgb_cfg;
    vo_hihdr_v_cfg v_hihdr_cfg;
} vdp_vid_ip_cfg;

/* for V1 */
typedef struct {
    hi_u32 ck_gt_en;
    hi_u32 out_pro;
    hi_u32 out_fmt;
    hi_u64 in_height;
    hi_u64 out_height;
    hi_u64 in_width;
    hi_u64 out_width;
    hi_u32 cvfir_en;
    hi_u32 cvmid_en;
    hi_u32 cvfir_mode;

    hi_u32 hfir_order;
    hi_u32 lhfir_en;
    hi_u32 chfir_en;
    hi_u32 lhmid_en;
    hi_u32 chmid_en;
    hi_u32 non_lnr_en;
    hi_u32 lhfir_mode;
    hi_u32 chfir_mode;
    hi_u32 hl_shootctrl_en;
    hi_u32 hl_shootctrl_mode;
    hi_u32 hc_shootctrl_en;
    hi_u32 hc_shootctrl_mode;

    hi_u32 in_pro;
    hi_u32 graphdet_en;

    hi_u32 lvfir_en;
    hi_u32 lvmid_en;
    hi_u32 vfir_1tap_en;
    hi_u32 lvfir_mode;
    hi_u32 vl_shootctrl_en;
    hi_u32 vl_shootctrl_mode;
    hi_u32 vc_shootctrl_en;
    hi_u32 vc_shootctrl_mode;
} vdp_v1_cvfir_cfg;
typedef enum {
    VDP_V1_HFIR_TYP = 0,
    VDP_V1_HFIR_TYP1,
    VDP_V1_HFIR_RAND,
    VDP_V1_HFIR_MAX,
    VDP_V1_HFIR_MIN,
    VDP_V1_HFIR_ZERO,
    VDP_V1_HFIR_BUTT
} vdp_v1_hfir_mode;

typedef enum {
    XDP_HFIR_BYPASS = 0,
    XDP_HFIR_COPY,
    XDP_HFIR_BILT,
    XDP_HFIR_FILT,
    XDP_HFIR_BUTT
} xdp_hfir_mode;

typedef struct {
    hi_u32 ck_gt_en;  /* use for ck_gt_en */
    hi_u32 mid_en;
    xdp_hfir_mode hfir_mode;  /* use for mode */
} vdp_v1_hfir_cfg;

typedef struct {
    hi_u32 csc_en;
    hi_u32 csc_ck_gt_en;
} vdp_csc_v1_cfg;

typedef enum {
    DITHER_IO_MODE_12_10 = 1,
    DITHER_IO_MODE_12_8 = 2,
    DITHER_IO_MODE_10_8 = 3,
    DITHER_IO_MODE_10_6 = 4,
    DITHER_IO_MODE_9_6 = 5,
    DITHER_IO_MODE_8_6 = 6,
    DITHER_IO_MODE_10_9_6 = 5,
    DITHER_IO_MODE_10_8_6 = 6,
    DITHER_IO_MODE_BUTT
} dither_io_mode;

typedef struct {
    hi_u32 dither_en;
    hi_u32 dither_mode;
    hi_u32 dither_round;
    hi_u32 dither_round_unlim;
    hi_u32 i_data_width_dither;
    hi_u32 o_data_width_dither;
    hi_u32 dither_domain_mode;
    hi_u32 dither_tap_mode;
    hi_u32 dither_sed_y0;
    hi_u32 dither_sed_u0;
    hi_u32 dither_sed_v0;
    hi_u32 dither_sed_w0;
    hi_u32 dither_sed_y1;
    hi_u32 dither_sed_u1;
    hi_u32 dither_sed_v1;
    hi_u32 dither_sed_w1;
    hi_u32 dither_sed_y2;
    hi_u32 dither_sed_u2;
    hi_u32 dither_sed_v2;
    hi_u32 dither_sed_w2;
    hi_u32 dither_sed_y3;
    hi_u32 dither_sed_u3;
    hi_u32 dither_sed_v3;
    hi_u32 dither_sed_w3;
    hi_u32 dither_thr_max;
    hi_u32 dither_thr_min;
    dither_io_mode io_mode;
} vdp_dither_cfg;

typedef enum {
    VDP_DITHER_MODE_TYP = 0,
    VDP_DITHER_MODE_TYP1,
    VDP_DITHER_MODE_RAND,
    VDP_DITHER_MODE_MAX,
    VDP_DITHER_MODE_MIN,
    VDP_DITHER_MODE_ZERO,
    VDP_DITHER_MODE_BUTT
} vdp_dither_mode;

typedef enum {
    DITHER_MODE_10BIT = 0,
    DITHER_MODE_8BIT = 1,
    DITHER_MODE_BUTT
} dither_mode;

typedef enum {
    DITHER_OWIDTH_MODE_5BIT = 0,
    DITHER_OWIDTH_MODE_6BIT = 1,
    DITHER_OWIDTH_MODE_7BIT = 2,
    DITHER_OWIDTH_MODE_8BIT = 3,
    DITHER_OWIDTH_MODE_9BIT = 4,
    DITHER_OWIDTH_MODE_10BIT = 5,
    DITHER_OWIDTH_MODE_BUTT
} dither_owidth_mode;

typedef enum {
    DITHER_IWIDTH_MODE_8BIT = 0,
    DITHER_IWIDTH_MODE_9BIT = 1,
    DITHER_IWIDTH_MODE_10BIT = 2,
    DITHER_IWIDTH_MODE_11BIT = 3,
    DITHER_IWIDTH_MODE_12BIT = 4,
    DITHER_IWIDTH_MODE_BUTT
} dither_iwidth_mode;

typedef enum {
    DITHER_DOMAIN_MODE_SPACE = 0,
    DITHER_DOMAIN_MODE_TIME = 1,
    DITHER_DOMAIN_MODE_BUTT
} dither_domain_mode;

typedef enum {
    DITHER_TAP_MODE_7 = 0,
    DITHER_TAP_MODE_15 = 1,
    DITHER_TAP_MODE_23 = 2,
    DITHER_TAP_MODE_31 = 3,
    DITHER_TAP_MODE_BUTT
} dither_tap_mode;

typedef struct {
    hi_u32 dither_in_bit_width;
    hi_u32 dither_out_bit_width;
} vo_dither_info;

vo_coef_addr *vo_drv_get_coef_buf_addr(hi_void);
hi_void vo_drv_v_set_v0_zme_coef(vo_rm_coef_mode coef_mode);

#define SEG_WIDTH             256

hi_bool vou_drv_is_virt_dev(hi_vo_dev dev);

hi_void vou_drv_set_dev_dither_mode(hi_vo_dev dev, vdp_dither_mode dither_mode_set, vdp_dither_cfg *cfg);

hi_void vou_drv_board_init(hi_void);
hi_void vou_drv_board_de_init(hi_void);
hi_void vo_drv_int_reg_up_mode(hi_vo_layer layer, vo_int_mode int_mode);
hi_void vou_drv_set_dev_intf_type(hi_vo_dev dev, hi_vo_intf_type intf_type);
hi_void vou_drv_set_dev_bk_grd(hi_vo_dev dev, hi_u32 bg_color);
hi_void vou_drv_set_dev_out_sync(hi_vo_dev dev, hi_vo_intf_sync vo_out_mode);
hi_void vou_drv_set_dev_default_vtth(hi_vo_dev dev);
hi_s32 vou_drv_set_dev_vtth(hi_vo_dev dev, hi_u32 vtth);
hi_void vou_drv_get_dev_vtth(hi_vo_dev dev, hi_u32 *vtth);
hi_bool vou_drv_is_support_save_buf_mode(hi_void);
hi_void vou_drv_set_dev_default_vtth2(hi_vo_dev dev);
hi_s32 vou_drv_set_dev_vtth2(hi_vo_dev dev, hi_u32 vtth);
hi_void vou_drv_get_dev_vtth2(hi_vo_dev dev, hi_u32 *vtth);
hi_void vou_drv_set_dev_dither_out(hi_vo_dev dev, hi_u32 out_bit_width);
hi_void vou_drv_set_dev_dither(hi_vo_dev dev, hi_u32 in);
hi_bool vou_drv_get_dev_enable(hi_vo_dev dev);
hi_s32 vou_drv_check_usr_sync_timing(hi_vo_dev dev, hi_vo_sync_info *cfg);
hi_void vou_drv_set_usr_sync_timing(hi_vo_sync_info *cfg);
hi_s32 vo_drv_check_dev_user_sync_info(hi_vo_dev dev, hi_vo_user_intfsync_info *user_info);
hi_void vo_drv_set_dev_user_intf_sync_attr(hi_vo_dev dev, hi_vo_user_intfsync_attr *user_attr);
hi_void vo_drv_set_dev_div(hi_vo_dev dev, hi_u32 dev_div);
hi_void vo_drv_set_hdmi_div(hi_vo_dev dev, hi_u32 pre_div);
hi_void vo_drv_set_clk_reverse(hi_bool reverse);

/* interrupt relative */
hi_void vo_drv_dev_int_enable(hi_vo_dev dev, hi_bool enable);
hi_void vou_drv_int_clear(hi_u32 int_clear);
hi_void vou_drv_int_disable_all(hi_void);
hi_void vo_drv_int_set_mode(hi_vo_dev dev, vo_int_mode int_mode);

/* surface relative */
hi_void vou_drv_layer_enable(hi_vo_layer layer, hi_bool enable);
hi_void vou_drv_set_layer_bk_grd(hi_vo_layer layer, hi_u32 bk_grd);
hi_void vou_drv_set_layer_data_fmt(hi_vo_layer layer, vou_layer_pixerl_format data_fmt);
hi_void vou_drv_get_layer_data_fmt(hi_vo_layer layer, vou_layer_pixerl_format *data_fmt);
hi_void vou_drv_set_multi_area_enable(hi_vo_layer layer,
                                      hi_u32 u32area_num,
                                      hi_bool enable);
hi_void vou_drv_set_multi_addr_stride(hi_vo_layer layer,
                                      hi_u32 area_num,
                                      hi_u32 layer_luma_addr,
                                      hi_u32 layer_chroma_addr,
                                      hi_u32 layer_luma_stride,
                                      hi_u32 layer_chroma_stride);


hi_void vou_drv_layer_csc_enable(hi_vo_layer layer, hi_bool csc_en);

hi_void vou_drv_def_layer_bind_dev(hi_void);

hi_void vo_drv_set_dev_bg_color(hi_vo_dev dev, hi_u32 u32bgcolor, hi_bool rgb);
hi_void vo_drv_open(hi_vo_dev dev);
hi_void vo_drv_close(hi_vo_dev dev);
hi_void vo_drv_enable(hi_vo_dev dev);
hi_void vo_drv_disable(hi_vo_dev dev);

hi_void vo_drv_get_dev_hal_id(hi_vo_dev dev, vo_hal_dev *hi_vo_dev);
hi_void vo_drv_get_channel_hal_id(hi_vo_dev dev, hal_disp_outputchannel *vo_channel);
hi_void vou_drv_default_setting(hi_void);
hi_void vo_drv_transparent_transmit_setting(hi_bool transparent_transmit);
hi_s32 vou_drv_check_dev_id(hi_vo_dev dev);
hi_void vo_drv_dev_info_init(hi_void);
hi_void vo_drv_dev_info_part_init(hi_void);
hi_void vo_drv_dev_info_de_init(hi_void);

hi_bool vo_drv_is_phy_dev_hd(hi_vo_dev dev);
hi_bool vo_drv_is_phy_dev_sd(hi_vo_dev dev);
hi_bool vou_drv_special_work_flow(hi_vo_dev dev);

hi_s32 vou_drv_set_dev_full_frame_rate(hi_vo_dev dev, hi_u32 frame_rate);
hi_s32 vou_drv_check_dev_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr);
hi_s32 vou_drv_set_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *vo_pub_attr);
hi_s32 vou_drv_set_user_sync_info(hi_vo_dev dev, hi_vo_user_intfsync_info *info);
hi_void vou_drv_set_disp_max_size(hi_vo_dev dev, hi_u32 max_width, hi_u32 max_height);
hi_s32 vou_drv_set_dev_clk(hi_vo_dev dev);


hi_void vo_drv_set_all_crg_clk(hi_bool clk_en);
hi_void vo_drv_set_hd_clk_sel(hi_u32 hd_clk_sel);
hi_void vo_lpw_bus_reset(hi_bool reset);
hi_void vo_set_low_power_ctrl_clk_en(hi_bool clk_en);
hi_void vou_drv_init_default_csc(hi_vo_layer layer);

hi_void vou_drv_lbw_show_proc(osal_proc_entry_t *s);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of __VOU_DRV_H__ */

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

#ifndef _TDE_ADP_H_
#define _TDE_ADP_H_

#ifndef HI_BUILD_IN_BOOT
#if defined(CONFIG_GFX_STB_SDK)
#include "hi_drv_mmz.h"
#include "hi_drv_dev.h"
#endif
#endif
#include "hi_defines.h"
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define SIZE_256BYTE_ALIGN

#define TDE_NODE_HEAD_BYTE 16

#define TDE_NODE_TAIL_BYTE 12

#ifndef SIZE_256BYTE_ALIGN
#define CMD_SIZE 64
#define JOB_SIZE 96
#define NODE_SIZE 208
#define FILTER_SIZE 960
#else
#define CMD_SIZE 64
#define JOB_SIZE 96
#define NODE_SIZE ((sizeof(tde_hw_node) + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE + 0x0F) & (~0x0F))
#define FILTER_SIZE 1792
#endif

#define HI_TDE_FILTER_NUM 3

#if (HICHIP == HI3519A_V100)
#define TDE_INTNUM 85
#define TDE_REG_BASEADDR 0x04ad0000
#define TDE_REG_CLOCK 0x04510140

#define TDE_CTRL 0x0500
#define TDE_INT 0x0504
#define TDE_INTCLR 0x0508
#define TDE_AQ_NADDR_LOW 0x04fc
#define TDE_AQ_NADDR_HI 0x04f8

#define TDE_STA 0x4000

#define TDE_AQ_ADDR_LOW 0x4098
#define TDE_AQ_ADDR_HI 0x4094

#define TDE_MISCELLANEOUS 0x0514
#define MAX_LINE_BUFFER 1920

#elif (HICHIP == HI3516C_V500)
#define TDE_INTNUM 35 + 32
#define TDE_REG_BASEADDR 0x11210000
#define TDE_REG_CLOCK 0x120100b8

#define TDE_CTRL 0x0500
#define TDE_INT 0x0514
#define TDE_INTCLR 0x0510
#define TDE_AQ_NADDR_LOW 0x00f8
#define TDE_AQ_NADDR_HI 0x00fc

#define TDE_STA 0x4000

#define TDE_AQ_ADDR_LOW 0x4010
#define TDE_AQ_ADDR_HI 0x4014

#define TDE_MISCELLANEOUS 0x0530

#define MAX_LINE_BUFFER 1024
#else
#define TDE_INTNUM 85
#define TDE_REG_BASEADDR 0x11230000
#define TDE_REG_CLOCK 0x12010140

#define TDE_CTRL 0x0500
#define TDE_INT 0x0504
#define TDE_INTCLR 0x0508
#define TDE_AQ_NADDR_LOW 0x04fc
#define TDE_AQ_NADDR_HI 0x04f8

#define TDE_STA 0x4000

#define TDE_AQ_ADDR_LOW 0x4098
#define TDE_AQ_ADDR_HI 0x4094
#define MAX_LINE_BUFFER 4096

#define TDE_MISCELLANEOUS 0x0514
#endif

#ifndef __HuaweiLite__
#define TDE_IRQ_NAME "tde_osr_isr"
#else
#define TDE_IRQ_NAME "tde"
#endif

#ifndef __RTOS__
#define DESCRIPTION "Hisilicon TDE Device driver"
#define AUTHOR "Digital Media Team, Hisilicon crop."
#define TDE_VERSION "V1.0.0.0"
#endif

#if (HICHIP == HI3519A_V100)
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_fmt : 6;         /* [5..0]  */
        hi_u32 src1_argb_order : 5;  /* [10..6]  */
        hi_u32 src1_cbcr_order : 1;  /* [11]  */
        hi_u32 src1_rgb_exp : 2;     /* [13..12]  */
        hi_u32 reserved_0 : 1;       /* [14]  */
        hi_u32 src1_rgb_mode : 1;    /* [15]  */
        hi_u32 reserved_1 : 2;       /* [17..16]  */
        hi_u32 src1_alpha_range : 1; /* [18]  */
        hi_u32 src1_v_scan_ord : 1;  /* [19]  */
        hi_u32 src1_h_scan_ord : 1;  /* [20]  */
        hi_u32 src1_422v_pro : 1;    /* [21]  */
        hi_u32 reserved_2 : 7;       /* [28..22]  */
        hi_u32 src1_dma : 1;         /* [29]  */
        hi_u32 src1_mode : 1;        /* [30]  */
        hi_u32 src1_en : 1;          /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ctrl;
#else
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_fmt : 6;         /* [5..0]  */
        hi_u32 src1_argb_order : 5;  /* [10..6]  */
        hi_u32 src1_cbcr_order : 1;  /* [11]  */
        hi_u32 src1_rgb_exp : 2;     /* [13..12]  */
        hi_u32 reserved_0 : 4;       /* [17..14]  */
        hi_u32 src1_alpha_range : 1; /* [18]  */
        hi_u32 src1_v_scan_ord : 1;  /* [19]  */
        hi_u32 src1_h_scan_ord : 1;  /* [20]  */
        hi_u32 src1_422v_pro : 1;    /* [21]  */
        hi_u32 reserved_1 : 7;       /* [28..22]  */
        hi_u32 src1_dma : 1;         /* [29]  */
        hi_u32 src1_mode : 1;        /* [30]  */
        hi_u32 src1_en : 1;          /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ctrl;
#endif

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_ch0_addr_high : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ch0_addr_high;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_ch0_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ch0_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_ch1_addr_high : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ch1_addr_high;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_ch1_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ch1_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_ch0_stride : 20; /* [19..0]  */
        hi_u32 reserved : 12;        /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ch0_stride;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_ch1_stride : 20; /* [19..0]  */
        hi_u32 reserved : 12;        /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_ch1_stride;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_width : 16;  /* [15..0]  */
        hi_u32 src1_height : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_imgsize;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_color_fill : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_fill;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_alpha0 : 8; /* [7..0]  */
        hi_u32 src1_alpha1 : 8; /* [15..8]  */
        hi_u32 reserved_0 : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_alpha;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_hoffset_pix : 16; /* [15..0]  */
        hi_u32 reserved_0 : 16;       /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_pix_offset;

#if (HICHIP == HI3519A_V100)
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_fmt : 6;           /* [5..0]  */
        hi_u32 src2_argb_order : 5;    /* [10..6]  */
        hi_u32 src2_cbcr_order : 1;    /* [11]  */
        hi_u32 src2_rgb_exp : 2;       /* [13..12]  */
        hi_u32 src2_clut_mode : 1;     /* [14]  */
        hi_u32 src2_rgb_mode : 1;      /* [15]  */
        hi_u32 reserved_0 : 2;         /* [17..16]  */
        hi_u32 src2_alpha_range : 1;   /* [18]  */
        hi_u32 src2_v_scan_ord : 1;    /* [19]  */
        hi_u32 src2_h_scan_ord : 1;    /* [20]  */
        hi_u32 src2_422v_pro : 1;      /* [21]  */
        hi_u32 src2_dcmp_en : 1;       /* [22]  */
        hi_u32 src2_is_lossless : 1;   /* [23]  */
        hi_u32 src2_is_lossless_a : 1; /* [24]  */
        hi_u32 src2_cmp_mode : 1;      /* [25]  */
        hi_u32 src2_top_pred_en : 1;   /* [26]  */
        hi_u32 reserved_1 : 2;         /* [28..27]  */
        hi_u32 src2_mode : 2;          /* [30..29]  */
        hi_u32 src2_en : 1;            /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ctrl;
#else
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_fmt : 6;           /* [5..0]  */
        hi_u32 src2_argb_order : 5;    /* [10..6]  */
        hi_u32 src2_cbcr_order : 1;    /* [11]  */
        hi_u32 src2_rgb_exp : 2;       /* [13..12]  */
        hi_u32 src2_clut_mode : 1;     /* [14]  */
        hi_u32 reserved_0 : 3;         /* [17..15]  */
        hi_u32 src2_alpha_range : 1;   /* [18]  */
        hi_u32 src2_v_scan_ord : 1;    /* [19]  */
        hi_u32 src2_h_scan_ord : 1;    /* [20]  */
        hi_u32 src2_422v_pro : 1;      /* [21]  */
        hi_u32 src2_dcmp_en : 1;       /* [22]  */
        hi_u32 src2_is_lossless : 1;   /* [23]  */
        hi_u32 src2_is_lossless_a : 1; /* [24]  */
        hi_u32 src2_cmp_mode : 1;      /* [25]  */
        hi_u32 src2_top_pred_en : 1;   /* [26]  */
        hi_u32 reserved_1 : 2;         /* [28..27]  */
        hi_u32 src2_mode : 2;          /* [30..29]  */
        hi_u32 src2_en : 1;            /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ctrl;
#endif

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_start_x : 16; /* [15..0]  */
        hi_u32 des_start_y : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_sur_xy;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_ch0_addr_high : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ch0_addr_high;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_ch0_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ch0_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_ch1_addr_high : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ch1_addr_high;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_ch1_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ch1_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_ch0_stride : 20; /* [19..0]  */
        hi_u32 reserved : 12;        /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ch0_stride;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_ch1_stride : 20; /* [19..0]  */
        hi_u32 reserved : 12;        /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_ch1_stride;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_width : 16;  /* [15..0]  */
        hi_u32 src2_height : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_imgsize;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_color_fill : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_fill;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_alpha0 : 8; /* [7..0]  */
        hi_u32 src2_alpha1 : 8; /* [15..8]  */
        hi_u32 reserved_0 : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_alpha;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_hoffset_pix : 16; /* [15..0]  */
        hi_u32 reserved_0 : 16;       /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_pix_offset;

typedef union {
    struct {
        hi_u32 x : 12;       /* First X coordinate */
        hi_u32 reserve1 : 4; /* Reserve */
        hi_u32 y : 12;       /* First Y coordinate */
        hi_u32 reserve2 : 4; /* Reserve */
    } bits;
    hi_u32 all;
} u_sur_xy;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int des_fmt : 6;         /* [5..0]  */
        unsigned int des_argb_order : 5;  /* [10..6]  */
        unsigned int des_cbcr_order : 1;  /* [11]  */
        unsigned int des_bind_en : 1;     /* [12]  */
        unsigned int des_bind_mode : 1;   /* [13]  */
        unsigned int reserved_0 : 3;      /* [16..14]  */
        unsigned int des_rgb_round : 1;   /* [17]  */
        unsigned int des_alpha_range : 1; /* [18]  */
        unsigned int des_v_scan_ord : 1;  /* [19]  */
        unsigned int des_h_scan_ord : 1;  /* [20]  */
        unsigned int reserved_1 : 2;      /* [22..21]  */
        unsigned int cmp_en : 1;          /* [23]  */
        unsigned int cmp_addr_chg : 1;    /* [24]  */
        unsigned int reserved_2 : 6;      /* [30..25]  */
        unsigned int des_en : 1;          /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ctrl;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 head_ar_addr_hi : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_head_ar_addr_hi;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 head_ar_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_head_ar_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 head_gb_addr_hi : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_head_gb_addr_hi;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 head_gb_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_head_gb_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_ch0_addr_high : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ch0_addr_high;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_ch0_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ch0_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_ch1_addr_hi : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ch1_addr_high;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_ch1_addr_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ch1_addr_low;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_ch0_stride : 20; /* [19..0]  */
        hi_u32 reserved : 12;       /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ch0_stride;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_ch1_stride : 20; /* [19..0]  */
        hi_u32 reserved : 12;       /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_ch1_stride;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_width : 16;  /* [15..0]  */
        hi_u32 des_height : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_imgsize;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_alpha_thd : 8; /* [7..0]  */
        hi_u32 des_crop_mode : 1; /* [8]  */
        hi_u32 des_crop_en : 1;   /* [9]  */
        hi_u32 reserved_0 : 22;   /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_alpha;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_crop_start_x : 16; /* [15..0]  */
        hi_u32 des_crop_start_y : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_crop_pos_st;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_crop_end_x : 16; /* [15..0]  */
        hi_u32 des_crop_end_y : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_crop_pos_ed;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_ch0_mmu_bypass : 1; /* [0]  */
        hi_u32 src1_ch1_mmu_bypass : 1; /* [1]  */
        hi_u32 src2_ch0_mmu_bypass : 1; /* [2]  */
        hi_u32 src2_ch1_mmu_bypass : 1; /* [3]  */
        hi_u32 gdc3_mmu_bypass : 1;     /* [4]  */
        hi_u32 gdc4_mmu_bypass : 1;     /* [5]  */
        hi_u32 gdc5_mmu_bypass : 1;     /* [6]  */
        hi_u32 gdc6_mmu_bypass : 1;     /* [7]  */
        hi_u32 gdc7_mmu_bypass : 1;     /* [8]  */
        hi_u32 clut_mmu_bypass : 1;     /* [9]  */
        hi_u32 reserved_0 : 6;          /* [15..10]  */
        hi_u32 src1_ch0_prot : 1;       /* [16]  */
        hi_u32 src1_ch1_prot : 1;       /* [17]  */
        hi_u32 src2_ch0_prot : 1;       /* [18]  */
        hi_u32 src2_ch1_prot : 1;       /* [19]  */
        hi_u32 gdc3_prot : 1;           /* [20]  */
        hi_u32 gdc4_prot : 1;           /* [21]  */
        hi_u32 gdc5_prot : 1;           /* [22]  */
        hi_u32 gdc6_prot : 1;           /* [23]  */
        hi_u32 gdc7_prot : 1;           /* [24]  */
        hi_u32 reserved_1 : 7;          /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_mmu_prot_ctrl;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_hoffset_pix : 16; /* [15..0]  */
        hi_u32 reserved_0 : 16;      /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_pix_offset;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_safe_dist : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_safe_dist;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 des_safe_dist_inverse : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_safe_dist_inverse;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 hratio : 24;    /* [23..0]  */
        hi_u32 hfir_order : 1; /* [24]  */
        hi_u32 hchfir_en : 1;  /* [25]  */
        hi_u32 hlfir_en : 1;   /* [26]  */
        hi_u32 hafir_en : 1;   /* [27]  */
        hi_u32 hchmid_en : 1;  /* [28]  */
        hi_u32 hlmid_en : 1;   /* [29]  */
        hi_u32 hchmsc_en : 1;  /* [30]  */
        hi_u32 hlmsc_en : 1;   /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_hsp;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 hor_loffset : 28; /* [27..0]  */
        hi_u32 reserved_0 : 4;   /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_hloffset;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 hor_coffset : 28; /* [27..0]  */
        hi_u32 reserved_0 : 4;   /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_hcoffset;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 19;    /* [18..0]  */
        hi_u32 zme_in_fmt : 2;     /* [20..19]  */
        hi_u32 zme_out_fmt : 2;    /* [22..21]  */
        hi_u32 vchfir_en : 1;      /* [23]  */
        hi_u32 vlfir_en : 1;       /* [24]  */
        hi_u32 vafir_en : 1;       /* [25]  */
        hi_u32 vsc_chroma_tap : 1; /* [26]  */
        hi_u32 reserved_1 : 1;     /* [27]  */
        hi_u32 vchmid_en : 1;      /* [28]  */
        hi_u32 vlmid_en : 1;       /* [29]  */
        hi_u32 vchmsc_en : 1;      /* [30]  */
        hi_u32 vlmsc_en : 1;       /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_vsp;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 vratio : 16;     /* [15..0]  */
        hi_u32 reserved_0 : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_vsr;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 vchroma_offset : 16; /* [15..0]  */
        hi_u32 vluma_offset : 16;   /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_voffset;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 ow : 16; /* [15..0]  */
        hi_u32 oh : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_zmeoreso;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 iw : 16; /* [15..0]  */
        hi_u32 ih : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_zmeireso;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_crop_x : 13; /* [12..0]  */
        hi_u32 reserved_0 : 3;   /* [15..13]  */
        hi_u32 src1_crop_y : 13; /* [28..16]  */
        hi_u32 reserved_1 : 2;   /* [30..29]  */
        hi_u32 src1_crop_en : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_crop_pos;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_crop_width : 13;  /* [12..0]  */
        hi_u32 reserved_0 : 3;        /* [15..13]  */
        hi_u32 src1_crop_height : 13; /* [28..16]  */
        hi_u32 reserved_1 : 3;        /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_crop_size;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_hpzme_en : 1;   /* [0]  */
        hi_u32 reserved_0 : 3;      /* [3..1]  */
        hi_u32 src1_hpzme_mode : 4; /* [7..4]  */
        hi_u32 reserved_1 : 24;     /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_hpzme;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src1_hpzme_width : 16; /* [15..0]  */
        hi_u32 reserved_0 : 16;       /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_hpzme_size;

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscidc0 : 11;   /* [10..0]  */
        hi_u32 reserved_0 : 5; /* [15..11]  */
        hi_u32 cscidc1 : 11;   /* [26..16]  */
        hi_u32 reserved_1 : 4; /* [30..27]  */
        hi_u32 csc_en : 1;     /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_idc0;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscidc2 : 11;    /* [10..0]  */
        hi_u32 reserved_0 : 20; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_idc1;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscodc0 : 11;   /* [10..0]  */
        hi_u32 reserved_0 : 5; /* [15..11]  */
        hi_u32 cscodc1 : 11;   /* [26..16]  */
        hi_u32 reserved_1 : 5; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_odc0;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscodc2 : 11;    /* [10..0]  */
        hi_u32 reserved_0 : 20; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_odc1;
#else
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscidc0 : 9;    /* [8..0]  */
        hi_u32 cscidc1 : 9;    /* [17..9]  */
        hi_u32 cscidc2 : 9;    /* [26..18]  */
        hi_u32 csc_en : 1;     /* [27]  */
        hi_u32 reserved_0 : 4; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_idc;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscodc0 : 9;    /* [8..0]  */
        hi_u32 cscodc1 : 9;    /* [17..9]  */
        hi_u32 cscodc2 : 9;    /* [26..18]  */
        hi_u32 reserved_0 : 5; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_odc;
#endif

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp00 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp01 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_p0;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp02 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp10 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_p1;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp11 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp12 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_p2;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp20 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp21 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_p3;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp22 : 15;     /* [14..0]  */
        hi_u32 reserved_0 : 17; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_csc_p4;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 29;  /* [28..0]  */
        hi_u32 dither_round : 1; /* [29]  */
        hi_u32 reserved_1 : 1;   /* [30]  */
        hi_u32 dither_en : 1;    /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_dither_ctrl;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 hratio : 24;    /* [23..0]  */
        hi_u32 hfir_order : 1; /* [24]  */
        hi_u32 hchfir_en : 1;  /* [25]  */
        hi_u32 hlfir_en : 1;   /* [26]  */
        hi_u32 hafir_en : 1;   /* [27]  */
        hi_u32 hchmid_en : 1;  /* [28]  */
        hi_u32 hlmid_en : 1;   /* [29]  */
        hi_u32 hchmsc_en : 1;  /* [30]  */
        hi_u32 hlmsc_en : 1;   /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_hsp;

typedef union {
    /* Define the struct bits */
    struct {
        hi_s32 hor_loffset : 28; /* [27..0]  */
        hi_u32 reserved_0 : 4;   /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_hloffset;

typedef union {
    /* Define the struct bits */
    struct {
        hi_s32 hor_coffset : 28; /* [27..0]  */
        hi_u32 reserved_0 : 4;   /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_hcoffset;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 19;    /* [18..0]  */
        hi_u32 zme_in_fmt : 2;     /* [20..19]  */
        hi_u32 zme_out_fmt : 2;    /* [22..21]  */
        hi_u32 vchfir_en : 1;      /* [23]  */
        hi_u32 vlfir_en : 1;       /* [24]  */
        hi_u32 vafir_en : 1;       /* [25]  */
        hi_u32 vsc_chroma_tap : 1; /* [26]  */
        hi_u32 reserved_1 : 1;     /* [27]  */
        hi_u32 vchmid_en : 1;      /* [28]  */
        hi_u32 vlmid_en : 1;       /* [29]  */
        hi_u32 vchmsc_en : 1;      /* [30]  */
        hi_u32 vlmsc_en : 1;       /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_vsp;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 vratio : 16;     /* [15..0]  */
        hi_u32 reserved_0 : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_vsr;

typedef union {
    /* Define the struct bits */
    struct {
        hi_s32 vchroma_offset : 16; /* [15..0]  */
        hi_s32 vluma_offset : 16;   /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_voffset;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 ow : 16; /* [15..0]  */
        hi_u32 oh : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_zmeoreso;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 iw : 16; /* [15..0]  */
        hi_u32 ih : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_zmeireso;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_crop_x : 13; /* [12..0]  */
        hi_u32 reserved_0 : 3;   /* [15..13]  */
        hi_u32 src2_crop_y : 13; /* [28..16]  */
        hi_u32 reserved_1 : 2;   /* [30..29]  */
        hi_u32 src2_crop_en : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_crop_pos;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_crop_width : 13;  /* [12..0]  */
        hi_u32 reserved_0 : 3;        /* [15..13]  */
        hi_u32 src2_crop_height : 13; /* [28..16]  */
        hi_u32 reserved_1 : 3;        /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_crop_size;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_hpzme_en : 1;   /* [0]  */
        hi_u32 reserved_0 : 3;      /* [3..1]  */
        hi_u32 src2_hpzme_mode : 4; /* [7..4]  */
        hi_u32 reserved_1 : 24;     /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_hpzme;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_hpzme_width : 16; /* [15..0]  */
        hi_u32 reserved_0 : 16;       /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_hpzme_size;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 src2_csc_mode : 1; /* [0]  */
        hi_u32 des_premulten : 1; /* [1]  */
        hi_u32 src_premulten : 1; /* [2]  */
        hi_u32 reserved_0 : 29;   /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_csc_mux;

#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscidc0 : 11;   /* [10..0]  */
        hi_u32 reserved_0 : 5; /* [15..11]  */
        hi_u32 cscidc1 : 11;   /* [26..16]  */
        hi_u32 reserved_1 : 4; /* [30..27]  */
        hi_u32 csc_en : 1;     /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_idc0;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscidc2 : 11;    /* [10..0]  */
        hi_u32 reserved_0 : 20; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_idc1;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscodc0 : 11;   /* [10..0]  */
        hi_u32 reserved_0 : 5; /* [15..11]  */
        hi_u32 cscodc1 : 11;   /* [26..16]  */
        hi_u32 reserved_1 : 5; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_odc0;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscodc2 : 11;    /* [10..0]  */
        hi_u32 reserved_0 : 20; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_odc1;
#else
typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscidc0 : 9;    /* [8..0]  */
        hi_u32 cscidc1 : 9;    /* [17..9]  */
        hi_u32 cscidc2 : 9;    /* [26..18]  */
        hi_u32 csc_en : 1;     /* [27]  */
        hi_u32 reserved_0 : 4; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_idc;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscodc0 : 9;    /* [8..0]  */
        hi_u32 cscodc1 : 9;    /* [17..9]  */
        hi_u32 cscodc2 : 9;    /* [26..18]  */
        hi_u32 reserved_0 : 5; /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_odc;
#endif

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp00 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp01 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_p0;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp02 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp10 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_p1;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp11 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp12 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_p2;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp20 : 15;    /* [14..0]  */
        hi_u32 reserved_0 : 1; /* [15]  */
        hi_u32 cscp21 : 15;    /* [30..16]  */
        hi_u32 reserved_1 : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_p3;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 cscp22 : 15;     /* [14..0]  */
        hi_u32 reserved_0 : 17; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_csc_p4;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 29;  /* [28..0]  */
        hi_u32 dither_round : 1; /* [29]  */
        hi_u32 reserved_1 : 1;   /* [30]  */
        hi_u32 dither_en : 1;    /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_dither_ctrl;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 des_h_dswm_mode : 2;        /* [1..0]  */
        hi_u32 reserved_0 : 2;             /* [3..2]  */
        hi_u32 des_v_dswm_mode : 1;        /* [4]  */
        hi_u32 des_alpha_detect_clear : 1; /* [5]  */
        hi_u32 reserved_1 : 26;            /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_des_dswm;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 8;           /* [7..0]  */
        hi_u32 src1_rcopy_pixel_num : 8; /* [15..8]  */
        hi_u32 reserved_1 : 14;          /* [29..16]  */
        hi_u32 src1_rcopy_en : 1;        /* [30]  */
        hi_u32 src1_copy_cfg_from : 1;   /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_befor_zme_copy;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 8;           /* [7..0]  */
        hi_u32 src1_rcrop_pixel_num : 8; /* [15..8]  */
        hi_u32 reserved_1 : 14;          /* [29..16]  */
        hi_u32 src1_rcrop_en : 1;        /* [30]  */
        hi_u32 src1_crop_cfg_from : 1;   /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_after_zme_crop;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 8;           /* [7..0]  */
        hi_u32 src2_rcopy_pixel_num : 8; /* [15..8]  */
        hi_u32 reserved_1 : 14;          /* [29..16]  */
        hi_u32 src2_rcopy_en : 1;        /* [30]  */
        hi_u32 src2_copy_cfg_from : 1;   /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_befor_zme_copy;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 8;           /* [7..0]  */
        hi_u32 src2_rcrop_pixel_num : 8; /* [15..8]  */
        hi_u32 reserved_1 : 14;          /* [29..16]  */
        hi_u32 src2_rcrop_en : 1;        /* [30]  */
        hi_u32 src2_crop_cfg_from : 1;   /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_after_zme_crop;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 mix_prio0 : 3;  /* [2..0]  */
        hi_u32 mix_prio1 : 3;  /* [5..3]  */
        hi_u32 mix_prio2 : 3;  /* [8..6]  */
        hi_u32 mix_prio3 : 3;  /* [11..9]  */
        hi_u32 mix_prio4 : 3;  /* [14..12]  */
        hi_u32 mix_prio5 : 3;  /* [17..15]  */
        hi_u32 mix_prio6 : 3;  /* [20..18]  */
        hi_u32 alu_mode : 4;   /* [24..21]  */
        hi_u32 cbm_mode : 1;   /* [25]  */
        hi_u32 reserved_0 : 5; /* [30..26]  */
        hi_u32 cbm_en : 1;     /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmctrl;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 bkgb : 8; /* [7..0]  */
        hi_u32 bkgg : 8; /* [15..8]  */
        hi_u32 bkgr : 8; /* [23..16]  */
        hi_u32 bkga : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmbkg;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 colorizeb : 8;  /* [7..0]  */
        hi_u32 colorizeg : 8;  /* [15..8]  */
        hi_u32 colorizer : 8;  /* [23..16]  */
        hi_u32 reserved_0 : 7; /* [30..24]  */
        hi_u32 colorizeen : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmcolorize;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 rgb_rop : 4;         /* [3..0]  */
        hi_u32 rop : 4;           /* [7..4]  */
        hi_u32 reserved_0 : 8;      /* [15..8]  */
        hi_u32 alpha_from : 2;      /* [17..16]  */
        hi_u32 alpha_border_en : 2; /* [19..18]  */
        hi_u32 reserved_1 : 11;     /* [30..20]  */
        hi_u32 blendropen : 1;      /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmalupara;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 keybmode : 2;    /* [1..0]  */
        hi_u32 keygmode : 2;    /* [3..2]  */
        hi_u32 keyrmode : 2;    /* [5..4]  */
        hi_u32 keyamode : 2;    /* [7..6]  */
        hi_u32 keysel : 2;      /* [9..8]  */
        hi_u32 reserved_0 : 21; /* [30..10]  */
        hi_u32 keyen : 1;       /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmkeypara;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u8 keybmin : 8; /* [7..0]  */
        hi_u8 keygmin : 8; /* [15..8]  */
        hi_u8 keyrmin : 8; /* [23..16]  */
        hi_u8 keyamin : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmkeymin;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u8 keybmax : 8; /* [7..0]  */
        hi_u8 keygmax : 8; /* [15..8]  */
        hi_u8 keyrmax : 8; /* [23..16]  */
        hi_u8 keyamax : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmkeymax;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u8 keybmask : 8; /* [7..0]  */
        hi_u8 keygmask : 8; /* [15..8]  */
        hi_u8 keyrmask : 8; /* [23..16]  */
        hi_u8 keyamask : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_cbmkeymask;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 s1_galphaen : 1;      /* [0]  */
        hi_u32 s1_palphaen : 1;      /* [1]  */
        hi_u32 s1_premulten : 1;     /* [2]  */
        hi_u32 s1_multiglobalen : 1; /* [3]  */
        hi_u32 s1_blendmode : 4;     /* [7..4]  */
        hi_u32 s1_galpha : 8;        /* [15..8]  */
        hi_u32 reserved_0 : 15;      /* [30..16]  */
        hi_u32 s1_coverblenden : 1;  /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_cbmpara;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 s1_xfpos : 16; /* [15..0]  */
        hi_u32 s1_yfpos : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src1_cbmstpos;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 s2_galphaen : 1;      /* [0]  */
        hi_u32 s2_palphaen : 1;      /* [1]  */
        hi_u32 s2_premulten : 1;     /* [2]  */
        hi_u32 s2_multiglobalen : 1; /* [3]  */
        hi_u32 s2_blendmode : 4;     /* [7..4]  */
        hi_u32 s2_galpha : 8;        /* [15..8]  */
        hi_u32 reserved_0 : 15;      /* [30..16]  */
        hi_u32 s2_coverblenden : 1;  /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_cbmpara;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 s2_rtt_en : 1;   /* [0]  */
        hi_u32 s2_rtt_dir : 1;  /* [1]  */
        hi_u32 s2_rtt_fmt : 2;  /* [2]  */
        hi_u32 reserved_0 : 28; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_rtt_ctrl;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 s2_xfpos : 16; /* [15..0]  */
        hi_u32 s2_yfpos : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_src2_cbmstpos;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_scl_lh : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src1_zme_lhaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_scl_lv : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src1_zme_lvaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_scl_ch : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src1_zme_chaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src1_scl_cv : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src1_zme_cvaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_scl_lh : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src2_zme_lhaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_scl_lv : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src2_zme_lvaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_scl_ch : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src2_zme_chaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 src2_scl_cv : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_src2_zme_cvaddr;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 clut_addr : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_clut_addr;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 reserved_0 : 20; /* [19..0]  */
        hi_u32 awid_cfg0 : 4;   /* [23..20]  */
        hi_u32 reserved_1 : 4;  /* [27..24]  */
        hi_u32 arid_cfg0 : 4;   /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_axiid;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 nodeid : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_nodeid;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 eof_mask : 1;     /* [0]  */
        hi_u32 timeout_mask : 1; /* [1]  */
        hi_u32 bus_err_mask : 1; /* [2]  */
        hi_u32 eof_end_mask : 1; /* [3]  */
        hi_u32 reserved_0 : 28;  /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_intmask;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 next_hi : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_pnext_hi;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 next_low : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_pnext_low;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 start : 1;       /* [0]  */
        hi_u32 reserved_0 : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_start;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 eof_state : 1;     /* [0]  */
        hi_u32 timeout_state : 1; /* [1]  */
        hi_u32 bus_err : 1;       /* [2]  */
        hi_u32 eof_end_state : 1; /* [3]  */
        hi_u32 reserved_0 : 28;   /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_intstate;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 eof_clr : 1;     /* [0]  */
        hi_u32 timeout_clr : 1; /* [1]  */
        hi_u32 bus_err_clr : 1; /* [2]  */
        hi_u32 eof_end_clr : 1; /* [3]  */
        hi_u32 reserved_0 : 28; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_intclr;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 raw_eof : 1;     /* [0]  */
        hi_u32 raw_timeout : 1; /* [1]  */
        hi_u32 raw_bus_err : 1; /* [2]  */
        hi_u32 raw_eof_end : 1; /* [3]  */
        hi_u32 reserved_0 : 28; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 all;
} u_tde_rawint;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 pfcnt : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_pfcnt;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 routstanding : 4;  /* [3..0]  */
        hi_u32 woutstanding : 4;  /* [7..4]  */
        hi_u32 init_timer : 16;   /* [23..8]  */
        hi_u32 ck_gt_en : 1;      /* [24]  */
        hi_u32 ck_gt_en_calc : 1; /* [25]  */
        hi_u32 split_128b_en : 1; /* [26]  */
        hi_u32 split_256b_en : 1; /* [27]  */
        hi_u32 split_1k_en : 1;   /* [28]  */
        hi_u32 split_2k_en : 1;   /* [29]  */
        hi_u32 reserved_0 : 2;    /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_miscellaneous;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 mac_ch_prio : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_maccfg;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 timeout : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_timeout;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 eof_cnt : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_eofcnt;

typedef union {
    /* Define the struct bits */
    struct {
        hi_u32 emab : 3;         /* [2..0]  */
        hi_u32 emaa : 3;         /* [5..3]  */
        hi_u32 emasa : 1;        /* [6]  */
        hi_u32 emaw : 2;         /* [8..7]  */
        hi_u32 ema : 3;          /* [11..9]  */
        hi_u32 rfsuhd_wtsel : 2; /* [13..12]  */
        hi_u32 rfsuhd_rtsel : 2; /* [15..14]  */
        hi_u32 rfs_wtsel : 2;    /* [17..16]  */
        hi_u32 rfs_rtsel : 2;    /* [19..18]  */
        hi_u32 rfts_wct : 2;     /* [21..20]  */
        hi_u32 rfts_rct : 2;     /* [23..22]  */
        hi_u32 rfts_kp : 3;      /* [26..24]  */
        hi_u32 rftf_wct : 2;     /* [28..27]  */
        hi_u32 rftf_rct : 2;     /* [30..29]  */
        hi_u32 reserved : 1;     /* [31]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_memctrl;

#if (HICHIP == HI3519A_V100)
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int is_lossless : 1;       /* [0]  */
        unsigned int is_lossless_alpha : 1; /* [1]  */
        unsigned int cmp_mode : 1;          /* [2]  */
        unsigned int osd_mode : 2;          /* [4..3]  */
        unsigned int partition_en : 1;      /* [5]  */
        unsigned int part_num : 3;          /* [8..6]  */
        unsigned int reserved_0 : 23;       /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_glb_info;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int frame_width : 13;  /* [12..0]  */
        unsigned int reserved_0 : 3;    /* [15..13]  */
        unsigned int frame_height : 13; /* [28..16]  */
        unsigned int reserved_1 : 3;    /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_frame_size;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int budget_bits_mb : 10; /* [9..0]  */
        unsigned int reserved_0 : 6;      /* [15..10]  */
        unsigned int min_mb_bits : 10;    /* [25..16]  */
        unsigned int reserved_1 : 6;      /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg0;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int budget_bits_mb_cap : 10; /* [9..0]  */
        unsigned int reserved_0 : 22;         /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_qp : 4;            /* [3..0]  */
        unsigned int smth_qp : 4;           /* [7..4]  */
        unsigned int sad_bits_ngain : 4;    /* [11..8]  */
        unsigned int reserved_0 : 4;        /* [15..12]  */
        unsigned int rc_smth_ngain : 3;     /* [18..16]  */
        unsigned int reserved_1 : 5;        /* [23..19]  */
        unsigned int special_bits_gain : 4; /* [27..24]  */
        unsigned int reserved_2 : 4;        /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg2;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_sad_thr : 7; /* [6..0]  */
        unsigned int reserved_0 : 9;  /* [15..7]  */
        unsigned int min_sad_thr : 7; /* [22..16]  */
        unsigned int reserved_1 : 9;  /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg3;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int smth_thr : 7;      /* [6..0]  */
        unsigned int reserved_0 : 1;    /* [7]  */
        unsigned int still_thr : 7;     /* [14..8]  */
        unsigned int reserved_1 : 1;    /* [15]  */
        unsigned int big_grad_thr : 10; /* [25..16]  */
        unsigned int reserved_2 : 6;    /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg4;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int smth_pix_num_thr : 6;       /* [5..0]  */
        unsigned int reserved_0 : 2;             /* [7..6]  */
        unsigned int still_pix_num_thr : 6;      /* [13..8]  */
        unsigned int reserved_1 : 2;             /* [15..14]  */
        unsigned int noise_pix_num_thr : 6;      /* [21..16]  */
        unsigned int reserved_2 : 2;             /* [23..22]  */
        unsigned int large_smth_pix_num_thr : 6; /* [29..24]  */
        unsigned int reserved_3 : 2;             /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg5;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int noise_sad : 7;    /* [6..0]  */
        unsigned int reserved_0 : 9;   /* [15..7]  */
        unsigned int pix_diff_thr : 9; /* [24..16]  */
        unsigned int reserved_1 : 7;   /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg6;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int adj_sad_bits_thr : 7; /* [6..0]  */
        unsigned int reserved_0 : 1;       /* [7]  */
        unsigned int max_trow_bits : 8;    /* [15..8]  */
        unsigned int reserved_1 : 16;      /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg7;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int qp_inc1_bits_thr : 8; /* [7..0]  */
        unsigned int qp_dec1_bits_thr : 8; /* [15..8]  */
        unsigned int qp_dec2_bits_thr : 8; /* [23..16]  */
        unsigned int qp_dec3_bits_thr : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg8;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int force_qp_thr : 10;     /* [9..0]  */
        unsigned int reserved_0 : 6;        /* [15..10]  */
        unsigned int force_qp_thr_cap : 10; /* [25..16]  */
        unsigned int reserved_1 : 6;        /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg9;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_row_len : 10; /* [9..0]  */
        unsigned int reserved_0 : 22;  /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg10;
#else
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int is_lossless : 1;   /* [0]  */
        unsigned int is_lossless_a : 1; /* [1]  */
        unsigned int cmp_mode : 1;      /* [2]  */
        unsigned int source_mode : 3;   /* [5..3]  */
        unsigned int part_cmp_en : 1;   /* [6]  */
        unsigned int top_pred_en : 1;   /* [7]  */
        unsigned int graphic_en : 1;    /* [8]  */
        unsigned int hq_cmp_en : 1;     /* [9]  */
        unsigned int reserved_0 : 22;   /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_glb_info;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int frame_width : 13;  /* [12..0]  */
        unsigned int reserved_0 : 3;    /* [15..13]  */
        unsigned int frame_height : 13; /* [28..16]  */
        unsigned int reserved_1 : 3;    /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_frame_size;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int mb_bits : 10;     /* [9..0]  */
        unsigned int reserved_0 : 6;   /* [15..10]  */
        unsigned int min_mb_bits : 10; /* [25..16]  */
        unsigned int reserved_1 : 6;   /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg0;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_qp : 4;        /* [3..0]  */
        unsigned int reserved_0 : 4;    /* [7..4]  */
        unsigned int sad_bits_gain : 4; /* [11..8]  */
        unsigned int reserved_1 : 4;    /* [15..12]  */
        unsigned int rc_smth_ngain : 3; /* [18..16]  */
        unsigned int reserved_2 : 5;    /* [23..19]  */
        unsigned int max_trow_bits : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_sad_thr : 7; /* [6..0]  */
        unsigned int reserved_0 : 9;  /* [15..7]  */
        unsigned int min_sad_thr : 7; /* [22..16]  */
        unsigned int reserved_1 : 9;  /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg2;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int smth_thr : 7;      /* [6..0]  */
        unsigned int reserved_0 : 1;    /* [7]  */
        unsigned int still_thr : 7;     /* [14..8]  */
        unsigned int reserved_1 : 1;    /* [15]  */
        unsigned int big_grad_thr : 10; /* [25..16]  */
        unsigned int reserved_2 : 6;    /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg3;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int smth_pix_num_thr : 6;  /* [5..0]  */
        unsigned int reserved_0 : 2;        /* [7..6]  */
        unsigned int still_pix_num_thr : 6; /* [13..8]  */
        unsigned int reserved_1 : 2;        /* [15..14]  */
        unsigned int noise_pix_num_thr : 6; /* [21..16]  */
        unsigned int reserved_2 : 10;       /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg4;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int noise_sad : 7;     /* [6..0]  */
        unsigned int reserved_0 : 9;    /* [15..7]  */
        unsigned int pix_diff_thr : 10; /* [25..16]  */
        unsigned int reserved_1 : 6;    /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg5;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int adj_sad_bits_thr : 7; /* [6..0]  */
        unsigned int reserved_0 : 25;      /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg6;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int qp_inc1_bits_thr : 8; /* [7..0]  */
        unsigned int qp_inc2_bits_thr : 8; /* [15..8]  */
        unsigned int qp_dec1_bits_thr : 8; /* [23..16]  */
        unsigned int qp_dec2_bits_thr : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg7;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int est_err_gain : 5;      /* [4..0]  */
        unsigned int reserved_0 : 11;       /* [15..5]  */
        unsigned int max_est_err_level : 9; /* [24..16]  */
        unsigned int reserved_1 : 7;        /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg8;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reserved_0 : 16;       /* [15..0]  */
        unsigned int vbv_buf_loss1_thr : 7; /* [22..16]  */
        unsigned int reserved_1 : 1;        /* [23]  */
        unsigned int vbv_buf_loss2_thr : 7; /* [30..24]  */
        unsigned int reserved_2 : 1;        /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg9;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int qp_thr0 : 3;     /* [2..0]  */
        unsigned int reserved_0 : 5;  /* [7..3]  */
        unsigned int qp_thr1 : 3;     /* [10..8]  */
        unsigned int reserved_1 : 5;  /* [15..11]  */
        unsigned int qp_thr2 : 3;     /* [19..16]  */
        unsigned int reserved_2 : 13; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg10;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int grph_bias_bit_thr0 : 8;  /* [7..0]  */
        unsigned int grph_bias_bit_thr1 : 8;  /* [15..8]  */
        unsigned int grph_ideal_bit_thr : 10; /* [25..16]  */
        unsigned int reserved_0 : 6;          /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg11;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int force_rc_en : 1;           /* [0]  */
        unsigned int reserved_0 : 7;            /* [7..1]  */
        unsigned int forcerc_bits_diff_thr : 8; /* [15..8]  */
        unsigned int reserved_1 : 16;           /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg12;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int maxdiff_ctrl_en : 1; /* [0]  */
        unsigned int reserved_0 : 31;     /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg13;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int mb_bits_cap : 10;       /* [9..0]  */
        unsigned int reserved_0 : 6;         /* [15..10]  */
        unsigned int init_buf_bits_cap : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg14;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int lfw_mb_len : 7;    /* [6..0]  */
        unsigned int reserved_0 : 1;    /* [7]  */
        unsigned int cmplx_sad_thr : 4; /* [11..8]  */
        unsigned int reserved_1 : 4;    /* [15..12]  */
        unsigned int err_thr0 : 4;      /* [19..16]  */
        unsigned int reserved_2 : 4;    /* [23..20]  */
        unsigned int err_thr1 : 4;      /* [27..24]  */
        unsigned int reserved_3 : 4;    /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg15;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int sim_num_thr : 3;   /* [2..0]  */
        unsigned int reserved_0 : 5;    /* [7..3]  */
        unsigned int sum_y_err_thr : 7; /* [14..8]  */
        unsigned int reserved_1 : 1;    /* [15]  */
        unsigned int sum_c_err_thr : 7; /* [22..16]  */
        unsigned int reserved_2 : 9;    /* [31..23]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg16;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int cpmlx_sad_thr_y : 4;   /* [3..0]  */
        unsigned int reserved_0 : 4;        /* [7..4]  */
        unsigned int smpl_sad_thr_c : 4;    /* [11..8]  */
        unsigned int reserved_1 : 4;        /* [15..12]  */
        unsigned int smpl_sumsad_thr_y : 8; /* [23..16]  */
        unsigned int smpl_sumsad_thr_c : 8; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg17;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int future_sad_y_thr0 : 4; /* [3..0]  */
        unsigned int reserved_0 : 4;        /* [7..4]  */
        unsigned int future_sad_c_thr0 : 4; /* [11..8]  */
        unsigned int reserved_1 : 4;        /* [15..12]  */
        unsigned int future_sad_y_thr1 : 4; /* [19..16]  */
        unsigned int reserved_2 : 4;        /* [23..20]  */
        unsigned int future_sad_c_thr1 : 4; /* [27..24]  */
        unsigned int reserved_3 : 4;        /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg18;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int cmplx_sumsad_thr_y : 8; /* [7..0]  */
        unsigned int cmplx_sumsad_thr_c : 8; /* [15..8]  */
        unsigned int reserved_0 : 16;        /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg19;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int buffer_up_lmt_thr : 12;  /* [11..0]  */
        unsigned int reserved0 : 4;           /* [15..12]  */
        unsigned int buffer_low_lmt_thr : 12; /* [27..16]  */
        unsigned int reserved1 : 4;           /* [31..27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg20;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int buffer_adj_qp_thr0 : 4; /* [3..0]  */
        unsigned int reserved0 : 4;          /* [7..4]  */
        unsigned int buffer_adj_qp_thr1 : 4; /* [11..8]  */
        unsigned int reserved1 : 4;          /* [15..12]  */
        unsigned int buffer_adj_qp_thr2 : 4; /* [19..16]  */
        unsigned int reserved2 : 12;         /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg21;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int buffer_surplus_thr0 : 8; /* [7..0]  */
        unsigned int buffer_surplus_thr1 : 8; /* [15..8]  */
        unsigned int buffer_surplus_thr2 : 8; /* [23..16]  */
        unsigned int reserved0 : 8;           /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg22;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int offset_bits_thr : 10; /* [9..0]  */
        unsigned int reserved0 : 22;       /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_rc_cfg23;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_gap_bw_row_len_thr : 7; /* [6..0]  */
        unsigned int reserved_0 : 25;            /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_stat_thr;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int pcmp_start_hpos : 13; /* [12..0]  */
        unsigned int reserved_0 : 3;       /* [15..13]  */
        unsigned int pcmp_end_hpos : 13;   /* [28..16]  */
        unsigned int reserved_1 : 3;       /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_pcmp;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int frame_size_reg : 22; /* [21..0]  */
        unsigned int reserved_0 : 10;     /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_des_osd_bs_size;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_frm_row_len : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;      /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_des_osd_worst_row;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int min_frm_row_len : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;      /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_des_osd_best_row;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_gap_bw_row_len_cnt : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;             /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_des_osd_stat_info;

typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int glb_st : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_des_osd_debug0;

typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int bitsmux_st : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_des_osd_debug1;
#endif

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug0 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug0;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug1 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug1;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug2 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug2;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug3 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug3;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug4 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug4;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug5 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug5;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug6 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug6;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug7 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug7;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug8 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug8;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug9 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug9;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug10 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug10;

typedef union {
    /* Define the struct bits  */
    struct {
        hi_u32 debug11 : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    hi_u32 u32;
} u_tde_debug11;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int cmp1_frame_size_reg : 22; /* [21..0]  */
        unsigned int reserved_0 : 10;          /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_bs_size_cmp1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int cmp1_max_frm_row_len : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;           /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_worst_row_cmp1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int cmp1_min_frm_row_len : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;           /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_best_row_cmp1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int cmp1_max_gap_bw_row_len_cnt : 16; /* [15..0]  */
        unsigned int reserved_0 : 16;                  /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_stat_info_cmp1;

typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int cmp1_glb_st : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_debug0_cmp1;

typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int cmp1_bitsmux_st : 32; /* [31..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_debug1_cmp1;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int special_max_row_len : 16;      /* [15..0]  */
        unsigned int cmp1_special_max_row_len : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_tde_od_pic_osd_special_worst_row_cmp1;

#if (HICHIP == HI3516C_V500)
typedef struct {
    hi_u32 tde_src1_zme_lhaddr_high;         /* 0x0 */
    hi_u32 tde_src1_zme_lhaddr_low;          /* 0x4 */
    hi_u32 tde_src1_zme_lvaddr_high;         /* 0x8 */
    hi_u32 tde_src1_zme_lvaddr_low;          /* 0xc */
    hi_u32 tde_src1_zme_chaddr_high;         /* 0x10 */
    hi_u32 tde_src1_zme_chaddr_low;          /* 0x14 */
    hi_u32 tde_src1_zme_cvaddr_high;         /* 0x18 */
    hi_u32 tde_src1_zme_cvaddr_low;          /* 0x1c */
    hi_u32 tde_src2_zme_lhaddr_high;         /* 0x20 */
    hi_u32 tde_src2_zme_lhaddr_low;          /* 0x24 */
    hi_u32 tde_src2_zme_lvaddr_high;         /* 0x28 */
    hi_u32 tde_src2_zme_lvaddr_low;          /* 0x2c */
    hi_u32 tde_src2_zme_chaddr_high;         /* 0x30 */
    hi_u32 tde_src2_zme_chaddr_low;          /* 0x34 */
    hi_u32 tde_src2_zme_cvaddr_high;         /* 0x38 */
    hi_u32 tde_src2_zme_cvaddr_low;          /* 0x3c */
    hi_u32 tde_clut_addr_high;               /* 0x40 */
    hi_u32 tde_clut_addr_low;                /* 0x44 */
    u_tde_axiid tde_axiid;                   /* 0x48 */
    hi_u32 reserved_0[41];                   /* 0x4c~0xec 41 is the length of the resources */
    u_tde_intmask tde_intmask;               /* 0xf0 */
    u_tde_nodeid tde_nodeid;                 /* 0xf4 */
    u_tde_pnext_low tde_pnext_low;           /* 0xf8 */
    u_tde_pnext_hi tde_pnext_hi;             /* 0xfc */
    u_src1_ctrl src1_ctrl;                   /* 0x100 */
    u_src1_ch0_addr_high src1_ch0_addr_high; /* 0x104 */
    u_src1_ch0_addr_low src1_ch0_addr_low;   /* 0x108 */
    u_src1_ch1_addr_high src1_ch1_addr_high; /* 0x10c */
    u_src1_ch1_addr_low src1_ch1_addr_low;   /* 0x110 */
    u_src1_ch0_stride src1_ch0_stride;       /* 0x114 */
    u_src1_ch1_stride src1_ch1_stride;       /* 0x118 */
    u_src1_imgsize src1_imgsize;             /* 0x11c */
    u_src1_fill src1_fill;                   /* 0x120 */
    u_src1_alpha src1_alpha;                 /* 0x124 */
    u_src1_pix_offset src1_pix_offset;       /* 0x128 */
    hi_u32 reserved_1[9];                    /* 0x12c~0x14c 9 is the length of the resources */
    u_src2_ctrl src2_ctrl;                   /* 0x150 */
    u_src2_ch0_addr_high src2_ch0_addr_high; /* 0x154 */
    u_src2_ch0_addr_low src2_ch0_addr_low;   /* 0x158 */
    u_src2_ch1_addr_high src2_ch1_addr_high; /* 0x15c */
    u_src2_ch1_addr_low src2_ch1_addr_low;   /* 0x160 */
    u_src2_ch0_stride src2_ch0_stride;       /* 0x164 */
    u_src2_ch1_stride src2_ch1_stride;       /* 0x168 */
    u_src2_imgsize src2_imgsize;             /* 0x16c */
    u_src2_fill src2_fill;                   /* 0x170 */
    u_src2_alpha src2_alpha;                 /* 0x174 */
    u_src2_pix_offset src2_pix_offset;       /* 0x178 */
    hi_u32 reserved_2[33];                   /* 0x17c~0x1fc 33 is the length of the resources */
    hi_u32 gdc3_ctrl;                        /* 0x200 */
    hi_u32 gdc3_addr_high;                   /* 0x204 */
    hi_u32 gdc3_addr_low;                    /* 0x208 */
    hi_u32 gdc3_stride;                      /* 0x20c */
    hi_u32 gdc3_imgsize;                     /* 0x210 */
    hi_u32 gdc3_fill;                        /* 0x214 */
    hi_u32 gdc3_alpha;                       /* 0x218 */
    hi_u32 gdc3_pix_offset;                  /* 0x21c */
    hi_u32 gdc4_ctrl;                        /* 0x220 */
    hi_u32 gdc4_addr_high;                   /* 0x224 */
    hi_u32 gdc4_addr_low;                    /* 0x228 */
    hi_u32 gdc4_stride;                      /* 0x22c */
    hi_u32 gdc4_imgsize;                     /* 0x230 */
    hi_u32 gdc4_fill;                        /* 0x234 */
    hi_u32 gdc4_alpha;                       /* 0x238 */
    hi_u32 gdc4_pix_offset;                  /* 0x23c */
    hi_u32 gdc5_ctrl;                        /* 0x240 */
    hi_u32 gdc5_addr_high;                   /* 0x244 */
    hi_u32 gdc5_addr_low;                    /* 0x248 */
    hi_u32 gdc5_stride;                      /* 0x24c */
    hi_u32 gdc5_imgsize;                     /* 0x250 */
    hi_u32 gdc5_fill;                        /* 0x254 */
    hi_u32 gdc5_alpha;                       /* 0x258 */
    hi_u32 gdc5_pix_offset;                  /* 0x25c */
    hi_u32 gdc6_ctrl;                        /* 0x260 */
    hi_u32 gdc6_addr_high;                   /* 0x264 */
    hi_u32 gdc6_addr_low;                    /* 0x268 */
    hi_u32 gdc6_stride;                      /* 0x26c */
    hi_u32 gdc6_imgsize;                     /* 0x270 */
    hi_u32 gdc6_fill;                        /* 0x274 */
    hi_u32 gdc6_alpha;                       /* 0x278 */
    hi_u32 gdc6_pix_offset;                  /* 0x27c */
    hi_u32 gdc7_ctrl;                        /* 0x280 */
    hi_u32 gdc7_addr_high;                   /* 0x284 */
    hi_u32 gdc7_addr_low;                    /* 0x288 */
    hi_u32 gdc7_stride;                      /* 0x28c */
    hi_u32 gdc7_imgsize;                     /* 0x290 */
    hi_u32 gdc7_fill;                        /* 0x294 */
    hi_u32 gdc7_alpha;                       /* 0x298 */
    hi_u32 gdc7_pix_offset;                  /* 0x29c */
    u_des_ctrl des_ctrl;                     /* 0x2a0 */
    u_des_ch0_addr_high des_ch0_addr_high;   /* 0x2a4 */
    u_des_ch0_addr_low des_ch0_addr_low;     /* 0x2a8 */
    u_des_ch1_addr_high des_ch1_addr_high;   /* 0x2ac */
    u_des_ch1_addr_low des_ch1_addr_low;     /* 0x2b0 */
    u_des_ch0_stride des_ch0_stride;         /* 0x2b4 */
    u_des_ch1_stride des_ch1_stride;         /* 0x2b8 */
    u_des_imgsize des_imgsize;               /* 0x2bc */
    u_des_alpha des_alpha;                   /* 0x2c0 */
    u_des_crop_pos_st des_crop_pos_st;       /* 0x2c4 */
    u_des_crop_pos_ed des_crop_pos_ed;       /* 0x2c8 */
    u_des_pix_offset des_pix_offset;         /* 0x2cc */
    hi_u32 reserved_3[12];                   /* 0x2d0~0x2fc 12 is the length of the resources */
    u_src1_hsp src1_hsp;                     /* 0x300 */
    u_src1_hloffset src1_hloffset;           /* 0x304 */
    u_src1_hcoffset src1_hcoffset;           /* 0x308 */
    u_src1_vsp src1_vsp;                     /* 0x30c */
    u_src1_vsr src1_vsr;                     /* 0x310 */
    u_src1_voffset src1_voffset;             /* 0x314 */
    u_src1_zmeoreso src1_zmeoreso;           /* 0x318 */
    u_src1_zmeireso src1_zmeireso;           /* 0x31c */
    u_src1_hpzme src1_hpzme;                 /* 0x320 */
    u_src1_hpzme_size src1_hpzme_size;       /* 0x324 */
    u_src1_csc_idc0 src1_csc_idc0;           /* 0x328 */
    u_src1_csc_idc1 src1_csc_idc1;           /* 0x32c */
    u_src1_csc_odc0 src1_csc_odc0;           /* 0x330 */
    u_src1_csc_odc1 src1_csc_odc1;           /* 0x334 */
    u_src1_csc_p0 src1_csc_p0;               /* 0x338 */
    u_src1_csc_p1 src1_csc_p1;               /* 0x33c */
    u_src1_csc_p2 src1_csc_p2;               /* 0x340 */
    u_src1_csc_p3 src1_csc_p3;               /* 0x344 */
    u_src1_csc_p4 src1_csc_p4;               /* 0x348 */
    u_src1_dither_ctrl src1_dither_ctrl;     /* 0x34c */
    u_src2_hsp src2_hsp;                     /* 0x350 */
    hi_s32 src2_hloffset;                    /* 0x354 */
    hi_s32 src2_hcoffset;                    /* 0x358 */
    u_src2_vsp src2_vsp;                     /* 0x35c */
    u_src2_vsr src2_vsr;                     /* 0x360 */
    u_src2_voffset src2_voffset;             /* 0x364 */
    u_src2_zmeoreso src2_zmeoreso;           /* 0x368 */
    u_src2_zmeireso src2_zmeireso;           /* 0x36c */
    u_src2_hpzme src2_hpzme;                 /* 0x370 */
    u_src2_hpzme_size src2_hpzme_size;       /* 0x374 */
    u_src2_csc_mux src2_csc_mux;             /* 0x378 */
    u_des_csc_idc0 des_csc_idc0;             /* 0x37c */
    u_des_csc_idc1 des_csc_idc1;             /* 0x380 */
    u_des_csc_odc0 des_csc_odc0;             /* 0x384 */
    u_des_csc_odc1 des_csc_odc1;             /* 0x388 */
    u_des_csc_p0 des_csc_p0;                 /* 0x38c */
    u_des_csc_p1 des_csc_p1;                 /* 0x390 */
    u_des_csc_p2 des_csc_p2;                 /* 0x394 */
    u_des_csc_p3 des_csc_p3;                 /* 0x398 */
    u_des_csc_p4 des_csc_p4;                 /* 0x39c */
    u_des_dither_ctrl dst_dither_ctrl;       /* 0x3a0 */
    u_des_dswm des_dswm;                     /* 0x3a4 */
    hi_u32 reserved_4[18];                   /* 0x3a8~0x3ec 18 is the length of the resources */
    u_src2_rtt_ctrl src2_rtt_ctrl;           /* 0x3f0 */
    hi_u32 reserved_5[3];                    /* 0x3f4~0x3fc 3 is the length of the resources */
    u_cbmctrl cbmctrl;                       /* 0x400 */
    u_cbmbkg cbmbkg;                         /* 0x404 */
    u_cbmcolorize cbmcolorize;               /* 0x408 */
    u_cbmalupara cbmalupara;                 /* 0x40c */
    u_cbmkeypara cbmkeypara;                 /* 0x410 */
    u_cbmkeymin cbmkeymin;                   /* 0x414 */
    u_cbmkeymax cbmkeymax;                   /* 0x418 */
    u_cbmkeymask cbmkeymask;                 /* 0x41c */
    u_src1_cbmpara src1_cbmpara;             /* 0x420 */
    u_src1_cbmstpos src1_cbmstpos;           /* 0x424 */
    u_src2_cbmpara src2_cbmpara;             /* 0x428 */
} tde_hw_node;

#else
typedef struct {
    u_src1_ctrl src1_ctrl;                   /* 0x0 */
    u_src1_ch0_addr_high src1_ch0_addr_high; /* 0x4 */
    u_src1_ch0_addr_low src1_ch0_addr_low;   /* 0x8 */
    u_src1_ch1_addr_high src1_ch1_addr_high; /* 0xc */
    u_src1_ch1_addr_low src1_ch1_addr_low;   /* 0x10 */
    u_src1_ch0_stride src1_ch0_stride;       /* 0x14 */
    u_src1_ch1_stride src1_ch1_stride;       /* 0x18 */
    u_src1_imgsize src1_imgsize;             /* 0x1c */
    u_src1_fill src1_fill;                   /* 0x20 */
    u_src1_alpha src1_alpha;                 /* 0x24 */
    u_src1_pix_offset src1_pix_offset;       /* 0x28 */
    u_src2_ctrl src2_ctrl;                   /* 0x2c */
    u_src2_ch0_addr_high src2_ch0_addr_high; /* 0x30 */
    u_src2_ch0_addr_low src2_ch0_addr_low;   /* 0x34 */
    u_src2_ch1_addr_high src2_ch1_addr_high; /* 0x38 */
    u_src2_ch1_addr_low src2_ch1_addr_low;   /* 0x3c */
    u_src2_ch0_stride src2_ch0_stride;       /* 0x40 */
    u_src2_ch1_stride src2_ch1_stride;       /* 0x44 */
    u_src2_imgsize src2_imgsize;             /* 0x48 */
    u_src2_fill src2_fill;                   /* 0x4c */
    u_src2_alpha src2_alpha;                 /* 0x50 */
    u_src2_pix_offset src2_pix_offset;       /* 0x54 */
    hi_u32 gdc3_ctrl;                        /* 0x58 */
    hi_u32 gdc3_addr_high;                   /* 0x5c */
    hi_u32 gdc3_addr_low;                    /* 0x60 */
    hi_u32 gdc3_stride;                      /* 0x64 */
    hi_u32 gdc3_imgsize;                     /* 0x68 */
    hi_u32 gdc3_fill;                        /* 0x6c */
    hi_u32 gdc3_alpha;                       /* 0x70 */
    hi_u32 gdc3_pix_offset;                  /* 0x74 */
    hi_u32 gdc4_ctrl;                        /* 0x78 */
    hi_u32 gdc4_addr_high;                   /* 0x7c */
    hi_u32 gdc4_addr_low;                    /* 0x80 */
    hi_u32 gdc4_stride;                      /* 0x84 */
    hi_u32 gdc4_imgsize;                     /* 0x88 */
    hi_u32 gdc4_fill;                        /* 0x8c */
    hi_u32 gdc4_alpha;                       /* 0x90 */
    hi_u32 gdc4_pix_offset;                  /* 0x94 */
    hi_u32 gdc5_ctrl;                        /* 0x98 */
    hi_u32 gdc5_addr_high;                   /* 0x9C */
    hi_u32 gdc5_addr_low;                    /* 0xA0 */
    hi_u32 gdc5_stride;                      /* 0xA4 */
    hi_u32 gdc5_imgsize;                     /* 0xA8 */
    hi_u32 gdc5_fill;                        /* 0xAC */
    hi_u32 gdc5_alpha;                       /* 0xB0 */
    hi_u32 gdc5_pix_offset;                  /* 0xB4 */
    hi_u32 gdc6_ctrl;                        /* 0xB8 */
    hi_u32 gdc6_addr_high;                   /* 0xBC */
    hi_u32 gdc6_addr_low;                    /* 0xC0 */
    hi_u32 gdc6_stride;                      /* 0xC4 */
    hi_u32 gdc6_imgsize;                     /* 0xC8 */
    hi_u32 gdc6_fill;                        /* 0xCc */
    hi_u32 gdc6_alpha;                       /* 0xD0 */
    hi_u32 gdc6_pix_offset;                  /* 0xD4 */
    hi_u32 gdc7_ctrl;                        /* 0xD8 */
    hi_u32 gdc7_addr_high;                   /* 0xDC */
    hi_u32 gdc7_addr_low;                    /* 0xE0 */
    hi_u32 gdc7_stride;                      /* 0xE4 */
    hi_u32 gdc7_imgsize;                     /* 0xE8 */
    hi_u32 gdc7_fill;                        /* 0xEC */
    hi_u32 gdc7_alpha;                       /* 0xF0 */
    hi_u32 gdc7_pix_offset;                  /* 0xF4 */
    u_des_ctrl des_ctrl;                     /* 0xF8 */
    u_des_ch0_addr_high des_ch0_addr_high;   /* 0xFC */
    u_des_ch0_addr_low des_ch0_addr_low;     /* 0x100 */
    u_des_ch1_addr_high des_ch1_addr_high;   /* 0x104 */
    u_des_ch1_addr_low des_ch1_addr_low;     /* 0x108 */
    u_des_ch0_stride des_ch0_stride;         /* 0x10C */
    u_des_ch1_stride des_ch1_stride;         /* 0x110 */
    u_des_imgsize des_imgsize;               /* 0x114 */
    u_des_alpha des_alpha;                   /* 0x118 */
    u_des_crop_pos_st des_crop_pos_st;       /* 0x11c */
    u_des_crop_pos_ed des_crop_pos_ed;       /* 0x120 */
    u_des_pix_offset des_pix_offset;         /* 0x124 */
    hi_u32 reserved_1[54];                   /* 0x128~0x1fc 54 is the length of the resources */
    u_src1_hsp src1_hsp;                     /* 0x200 */
    hi_s32 src1_hloffset;                    /* 0x204 */
    hi_s32 src1_hcoffset;                    /* 0x208 */
    u_src1_vsp src1_vsp;                     /* 0x20c */
    u_src1_vsr src1_vsr;                     /* 0x210 */
    u_src1_voffset src1_voffset;             /* 0x214 */
    u_src1_zmeoreso src1_zmeoreso;           /* 0x218 */
    u_src1_zmeireso src1_zmeireso;           /* 0x21c */
    hi_u32 reserved_2[2];                    /* 0x220~0x224 2 is the length of the resources */
    u_src1_hpzme src1_hpzme;                 /* 0x228 */
    u_src1_hpzme_size src1_hpzme_size;       /* 0x22c */

#if (HICHIP == HI3519A_V100)
    u_src1_csc_idc0 src1_csc_idc0;           /* 0x230 */
    u_src1_csc_idc1 src1_csc_idc1;           /* 0x234 */
    u_src1_csc_odc0 src1_csc_odc0;           /* 0x238 */
    u_src1_csc_odc1 src1_csc_odc1;           /* 0x23c */
    u_src1_csc_p0 src1_csc_p0;               /* 0x240 */
    u_src1_csc_p1 src1_csc_p1;               /* 0x244 */
    u_src1_csc_p2 src1_csc_p2;               /* 0x248 */
    u_src1_csc_p3 src1_csc_p3;               /* 0x24c */
    u_src1_csc_p4 src1_csc_p4;               /* 0x250 */
    u_src1_dither_ctrl src1_dither_ctrl;     /* 0x254 */
    hi_u32 reserved_3[10];                   /* 0x258~0x27c 10 is the length of the resources */
#else
    u_src1_csc_idc src1_csc_idc; /* 0x230 */
    u_src1_csc_odc src1_csc_odc; /* 0x234 */
    u_src1_csc_p0 src1_csc_p0;   /* 0x238 */
    u_src1_csc_p1 src1_csc_p1;   /* 0x23c */
    u_src1_csc_p2 src1_csc_p2;   /* 0x240 */
    u_src1_csc_p3 src1_csc_p3;   /* 0x244 */
    u_src1_csc_p4 src1_csc_p4;   /* 0x248 */
    hi_u32 reserved_3[13];       /* 0x24c~0x27c 13 is the length of the resources */
#endif
    u_src2_hsp src2_hsp;                     /* 0x280 */
    hi_s32 src2_hloffset;                    /* 0x284 */
    hi_s32 src2_hcoffset;                    /* 0x288 */
    u_src2_vsp src2_vsp;                     /* 0x28c */
    u_src2_vsr src2_vsr;                     /* 0x290 */
    u_src2_voffset src2_voffset;             /* 0x294 */
    u_src2_zmeoreso src2_zmeoreso;           /* 0x298 */
    u_src2_zmeireso src2_zmeireso;           /* 0x29c */
    hi_u32 reserved_4[2];                    /* 0x2a0~0x2a4 2 is the length of the resources */
    u_src2_hpzme src2_hpzme;                 /* 0x2a8 */
    u_src2_hpzme_size src2_hpzme_size;       /* 0x2ac */
    u_src2_csc_mux src2_csc_mux;             /* 0x2b0 */

#if (HICHIP == HI3519A_V100)
    u_des_csc_idc0 des_csc_idc0;             /* 0x2b4 */
    u_des_csc_idc1 des_csc_idc1;             /* 0x2b8 */
    u_des_csc_odc0 des_csc_odc0;             /* 0x2bc */
    u_des_csc_odc1 des_csc_odc1;             /* 0x2c0 */
    u_des_csc_p0 des_csc_p0;                 /* 0x2c4 */
    u_des_csc_p1 des_csc_p1;                 /* 0x2c8 */
    u_des_csc_p2 des_csc_p2;                 /* 0x2cc */
    u_des_csc_p3 des_csc_p3;                 /* 0x2d0 */
    u_des_csc_p4 des_csc_p4;                 /* 0x2d4 */
    u_des_dither_ctrl dst_dither_ctrl;       /* 0x2d8 */
#else
    hi_u32 reserved_5[3];        /* 0x2b4~0x2bc 3 is the length of the resources */
    u_des_csc_idc des_csc_idc;   /* 0x2c0 */
    u_des_csc_odc des_csc_odc;   /* 0x2c4 */
    u_des_csc_p0 des_csc_p0;     /* 0x2c8 */
    u_des_csc_p1 des_csc_p1;     /* 0x2cc */
    u_des_csc_p2 des_csc_p2;     /* 0x2d0 */
    u_des_csc_p3 des_csc_p3;     /* 0x2d4 */
    u_des_csc_p4 des_csc_p4;     /* 0x2d8 */
#endif
    u_des_dswm des_dswm;                     /* 0x2dc */
    hi_u32 reserved_6[8];                    /* 0x2e0~0x2fc 8 is the length of the resources */
    u_cbmctrl cbmctrl;                       /* 0x300 */
    u_cbmbkg cbmbkg;                         /* 0x304 */
    u_cbmcolorize cbmcolorize;               /* 0x308 */
    u_cbmalupara cbmalupara;                 /* 0x30c */
    u_cbmkeypara cbmkeypara;                 /* 0x310 */
    u_cbmkeymin cbmkeymin;                   /* 0x314 */
    u_cbmkeymax cbmkeymax;                   /* 0x318 */
    u_cbmkeymask cbmkeymask;                 /* 0x31c */
    u_src1_cbmpara src1_cbmpara;             /* 0x320 */
    hi_u32 src1_cbmstpos;                    /* 0x324 */
    u_src2_cbmpara src2_cbmpara;             /* 0x328 */
    hi_u32 src2_cbmstpos;                    /* 0x32c */
    hi_u32 gdc3_cbmpara;                     /* 0x330 */
    hi_u32 gdc3_cbmstpos;                    /* 0x334 */
    hi_u32 gdc4_cbmpara;                     /* 0x338 */
    hi_u32 gdc4_cbmstpos;                    /* 0x33c */
    hi_u32 gdc5_cbmpara;                     /* 0x340 */
    hi_u32 gdc5_cbmstpos;                    /* 0x344 */
    hi_u32 gdc6_cbmpara;                     /* 0x348 */
    hi_u32 gdc6_cbmstpos;                    /* 0x34c */
    hi_u32 gdc7_cbmpara;                     /* 0x350 */
    hi_u32 gdc7_cbmstpos;                    /* 0x354 */
    hi_u32 reserved_7[38];                   /* 0x358~0x3ec 38 is the length of the resources */
    u_src2_rtt_ctrl src2_rtt_ctrl;           /* 0x3f0 */
    hi_u32 reserved_8[35];                   /* 0x3f4~0x47c 35 is the length of the resources */
    hi_u32 tde_src1_zme_lhaddr_high;         /* 0x480 */
    hi_u32 tde_src1_zme_lhaddr_low;          /* 0x484 */
    hi_u32 tde_src1_zme_lvaddr_high;         /* 0x488 */
    hi_u32 tde_src1_zme_lvaddr_low;          /* 0x48C */
    hi_u32 tde_src1_zme_chaddr_high;         /* 0x490 */
    hi_u32 tde_src1_zme_chaddr_low;          /* 0x494 */
    hi_u32 tde_src1_zme_cvaddr_high;         /* 0x498 */
    hi_u32 tde_src1_zme_cvaddr_low;          /* 0x49c */
    hi_u32 tde_src2_zme_lhaddr_high;         /* 0x4A0 */
    hi_u32 tde_src2_zme_lhaddr_low;          /* 0x4A4 */
    hi_u32 tde_src2_zme_lvaddr_high;         /* 0x4A8 */
    hi_u32 tde_src2_zme_lvaddr_low;          /* 0x4AC */
    hi_u32 tde_src2_zme_chaddr_high;         /* 0x4B0 */
    hi_u32 tde_src2_zme_chaddr_low;          /* 0x4B4 */
    hi_u32 tde_src2_zme_cvaddr_high;         /* 0x4B8 */
    hi_u32 tde_src2_zme_cvaddr_low;          /* 0x4Bc */
    hi_u32 tde_clut_addr_high;               /* 0x4C0 */
    hi_u32 tde_clut_addr_low;                /* 0x4C4 */
    hi_u32 tde_axiid;                        /* 0x4C8 */
    hi_u32 tde_nodeid;                       /* 0x4CC */
    u_tde_intmask tde_intmask;               /* 0x4D0 */

    hi_u32 reserved_9[9];                    /* 0x3D4~0x4F4 9 is the length of the resources */
    u_tde_pnext_hi tde_pnext_hi;             /* 0x4f8 */
    u_tde_pnext_low tde_pnext_low;           /* 0x4fc */
    u_tde_start tde_start;                   /* 0x500 */
    u_tde_intstate tde_intstate;             /* 0x504 */
    u_tde_intclr tde_intclr_;               /* 0x508 */
    u_tde_rawint tde_rawint;                 /* 0x50c */
    u_tde_pfcnt tde_pfcnt;                   /* 0x510 */
    u_tde_miscellaneous tde_miscellaneous_; /* 0x514 */
    u_tde_maccfg tde_maccfg;                 /* 0x518 */
    u_tde_timeout tde_timeout;               /* 0x51c */
    u_tde_eofcnt tde_eofcnt;                 /* 0x520 */
    u_tde_memctrl tde_memctrl;               /* 0x524 */
    hi_u32 tde_memctrl1;                     /* 0x528 */

    hi_u32 reserved_10[53]; /* 0x528~0x3ffc 53 is the length of the resources */

    u_tde_od_pic_osd_glb_info tde_od_pic_osd_glb_info;     /* 0x600 */
    u_tde_od_pic_osd_frame_size tde_od_pic_osd_frame_size; /* 0x604 */
    u_tde_od_pic_osd_rc_cfg0 tde_od_pic_osd_rc_cfg0;       /* 0x608 */
    u_tde_od_pic_osd_rc_cfg1 tde_od_pic_osd_rc_cfg1;       /* 0x60c */
    u_tde_od_pic_osd_rc_cfg2 tde_od_pic_osd_rc_cfg2;       /* 0x610 */
    u_tde_od_pic_osd_rc_cfg3 tde_od_pic_osd_rc_cfg3;       /* 0x614 */
    u_tde_od_pic_osd_rc_cfg4 tde_od_pic_osd_rc_cfg4;       /* 0x618 */
    u_tde_od_pic_osd_rc_cfg5 tde_od_pic_osd_rc_cfg5;       /* 0x61c */
    u_tde_od_pic_osd_rc_cfg6 tde_od_pic_osd_rc_cfg6;       /* 0x620 */
    u_tde_od_pic_osd_rc_cfg7 tde_od_pic_osd_rc_cfg7;       /* 0x624 */
    u_tde_od_pic_osd_rc_cfg8 tde_od_pic_osd_rc_cfg8;       /* 0x628 */
    u_tde_od_pic_osd_rc_cfg9 tde_od_pic_osd_rc_cfg9;       /* 0x62c */
    u_tde_od_pic_osd_rc_cfg10 tde_od_pic_osd_rc_cfg10;     /* 0x630 */
#if (HICHIP != HI3519A_V100)
    u_tde_od_pic_osd_rc_cfg11 tde_od_pic_osd_rc_cfg11;     /* 0x634 */
    u_tde_od_pic_osd_rc_cfg12 tde_od_pic_osd_rc_cfg12;     /* 0x638 */
    u_tde_od_pic_osd_rc_cfg13 tde_od_pic_osd_rc_cfg13;     /* 0x63c */
    u_tde_od_pic_osd_rc_cfg14 tde_od_pic_osd_rc_cfg14;     /* 0x640 */
    u_tde_od_pic_osd_rc_cfg15 tde_od_pic_osd_rc_cfg15;     /* 0x644 */
    u_tde_od_pic_osd_rc_cfg16 tde_od_pic_osd_rc_cfg16;     /* 0x648 */
    u_tde_od_pic_osd_rc_cfg17 tde_od_pic_osd_rc_cfg17;     /* 0x64c */
    u_tde_od_pic_osd_rc_cfg18 tde_od_pic_osd_rc_cfg18;     /* 0x650 */
    u_tde_od_pic_osd_rc_cfg19 tde_od_pic_osd_rc_cfg19;     /* 0x654 */
    hi_u32 reserved_11[2];                                 /* 0x658~0x65c 2 is the length of the resources */
    u_tde_od_pic_osd_stat_thr tde_od_pic_osd_stat_thr;     /* 0x660 */
    u_tde_od_pic_osd_pcmp tde_od_pic_osd_pcmp;             /* 0x664 */
    u_tde_od_pic_osd_rc_cfg20 tde_od_pic_osd_rc_cfg20;     /* 0x668 */
    u_tde_od_pic_osd_rc_cfg21 tde_od_pic_osd_rc_cfg21;     /* 0x66c */
    u_tde_od_pic_osd_rc_cfg22 tde_od_pic_osd_rc_cfg22;     /* 0x670 */
    u_tde_od_pic_osd_rc_cfg23 tde_od_pic_osd_rc_cfg23;     /* 0x674 */
#endif
} tde_hw_node;
#endif

typedef enum {
    TDE_DRV_INT_NODE = 0x1,
    TDE_DRV_INT_TIMEOUT = 0x2,
    TDE_DRV_INT_ERROR = 0x4,
    TDE_DRV_INT_NODE_COMP_AQ = 0x8,
} tde_drv_int;

typedef enum {
    HIGFX_TDE_ID = 0, /* < TDE ID         */
    HIGFX_JPGDEC_ID,  /* < JPEG DECODE ID */
    HIGFX_JPGENC_ID,  /* < JPEG_ENCODE ID */
    HIGFX_FB_ID,      /* <  FRAMEBUFFER ID */
    HIGFX_PNG_ID,     /* < PNG ID          */
    HIGFX_HIGO_ID,
    HIGFX_GFX2D_ID,
    HIGFX_BUTT_ID,
} higfx_mode_id;

#define conver_id(module_id) ((module_id) + HI_ID_TDE - HIGFX_TDE_ID)

#define CONFIG_TDE_TDE_EXPORT_FUNC

#define TDE_NO_SCALE_VSTEP 0x1000
#define TDE_NO_SCALE_HSTEP 0x100000
#define TDE_FLOAT_BITLEN 12
#define TDE_HAL_HSTEP_FLOATLEN 20
#define TDE_HAL_VSTEP_FLOATLEN 12
#define TDE_MAX_SLICE_WIDTH 256
#define TDE_MAX_SLICE_NUM 20
#define TDE_MAX_SURFACE_PITCH 0xffff
#define TDE_MAX_ZOOM_OUT_STEP 8
#define TDE_MAX_RECT_WIDTH_EX 0x2000
#define TDE_MAX_RECT_HEIGHT_EX 0x2000

#define TDE_MAX_RECT_WIDTH 0x1000
#define TDE_MAX_RECT_HEIGHT 0x1000

#define TDE_MAX_SLICE_RECT_WIDTH 0xfff
#define TDE_MAX_SLICE_RECT_HEIGHT 0xfff

#define TDE_MAX_MINIFICATION_H 255
#define TDE_MAX_MINIFICATION_V 255

#define ROP 0x1                /* Rop */
#define ALPHABLEND (0x1 << 1)  /* AlphaBlend */
#define COLORIZE (0x1 << 2)    /* Colorize */
#define CLUT (0x1 << 3)        /* Clut */
#define COLORKEY (0x1 << 4)    /* ColorKey */
#define CLIP (0x1 << 5)        /* Clip */
#define DEFLICKER (0x1 << 6)   /* Deflicker */
#define RESIZE (0x1 << 7)      /* Resize */
#define MIRROR (0x1 << 8)      /* Mirror */
#define CSCCOVERT (0x1 << 9)   /* CSC */
#define QUICKCOPY (0x1 << 10)  /* Quick copy */
#define QUICKFILL (0x1 << 11)  /* Rapid filling */
#define PATTERFILL (0x1 << 12) /* Pattern fill */
#define MASKROP (0x1 << 13)    /* MaskRop */
#define MASKBLEND (0x1 << 14)  /* MaskBlend */
#define ROTATE (0x1 << 15)     /* Rotate */
#define COMPRESS (0x1 << 16)   /* Compress */
#define SYNC (0x1 << 17)       /* Sync */
#define SLICE (0x1 << 18)      /* Slice */

hi_s32 tde_hal_get_capability(hi_u32 *capability);

#if (HICHIP == HI3516C_V500)
#define TDE_CAPABILITY                                                                                      \
    (ALPHABLEND | CLUT | COLORKEY | COLORIZE | RESIZE | CLIP | MIRROR | CSCCOVERT | QUICKFILL | QUICKCOPY | \
     PATTERFILL | ROTATE | ROP)
#else
#define TDE_CAPABILITY                                                                                      \
    (ALPHABLEND | CLUT | COLORKEY | DEFLICKER | COLORIZE | RESIZE | CLIP | MIRROR | CSCCOVERT | QUICKFILL | \
     QUICKCOPY | PATTERFILL | ROTATE | ROP | MASKROP | MASKBLEND | COMPRESS)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TDE_ADP_H_ */

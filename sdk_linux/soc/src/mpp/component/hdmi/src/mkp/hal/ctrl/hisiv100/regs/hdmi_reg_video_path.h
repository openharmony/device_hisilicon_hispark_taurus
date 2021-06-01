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
#ifndef __HDMI_REG_VIDEO_PATH_H__
#define __HDMI_REG_VIDEO_PATH_H__

#include "hi_type.h"

typedef union {
    struct {
        unsigned int reg_timing_gen_en : 1;  /* [0] */
        unsigned int reg_extmode       : 1;  /* [1] */
        unsigned int reg_timing_sel    : 6;  /* [7:2] */
        unsigned int reg_sync_polarity : 2;  /* [9:8] */
        unsigned int rsv_0             : 22; /* [31:10] */
    } bits;
    unsigned int u32;
} timing_gen_ctrl;

typedef union {
    struct {
        unsigned int hsync_total_count       : 13; /* [12:0] */
        unsigned int hsync_high_to_low_count : 13; /* [25:13] */
        unsigned int rsv_1                   : 6;  /* [31:26] */
    } bits;
    unsigned int u32;
} hsync_timing_config0;

typedef union {
    struct {
        unsigned int hsync_de_start_count : 13; /* [12:0] */
        unsigned int hsync_de_end_count   : 13; /* [25:13] */
        unsigned int rsv_2                : 6;  /* [31:26] */
    } bits;
    unsigned int u32;
} hsync_timing_config1;

typedef union {
    struct {
        unsigned int hsync_low_to_high_count : 13; /* [12:0] */
        unsigned int rsv_3                   : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} hsync_timing_config2;

typedef union {
    struct {
        unsigned int vsync_total_count       : 12; /* [11:0] */
        unsigned int vsync_high_to_low_count : 12; /* [23:12] */
        unsigned int rsv_4                   : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} vsync_timing_config0;

typedef union {
    struct {
        unsigned int vsync_de_start_count : 12; /* [11:0] */
        unsigned int vsync_de_end_count   : 12; /* [23:12] */
        unsigned int rsv_5                : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} vsync_timing_config1;

typedef union {
    struct {
        unsigned int vsync_low_to_high_count : 12; /* [11:0] */
        unsigned int rsv_6                   : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} vsync_timing_config2;

typedef union {
    struct {
        unsigned int reg_video_blank_en   : 1;  /* [0] */
        unsigned int reg_video_lp_disable : 1;  /* [1] */
        unsigned int rsv_6                : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} video_path_ctrl;

typedef union {
    struct {
        unsigned int tpg_enable          : 1;  /* [0] */
        unsigned int video_format        : 2;  /* [2:1] */
        unsigned int solid_pattern_en    : 1;  /* [3] */
        unsigned int colorbar_en         : 1;  /* [4] */
        unsigned int square_pattern_en   : 1;  /* [5] */
        unsigned int mask_pattern_en     : 3;  /* [8:6] */
        unsigned int replace_pattern_en  : 3;  /* [11:9] */
        unsigned int bar_pattern_extmode : 1;  /* [12] */
        unsigned int cbar_pattern_sel    : 2;  /* [14:13] */
        unsigned int mix_color_en        : 1;  /* [15] */
        unsigned int increase_en         : 1;  /* [16] */
        unsigned int rsv_7               : 15; /* [31:17] */
    } bits;
    unsigned int u32;
} pattern_gen_ctrll;

typedef union {
    struct {
        unsigned int solid_pattern_cb : 10; /* [9:0] */
        unsigned int solid_pattern_y  : 10; /* [19:10] */
        unsigned int solid_pattern_cr : 10; /* [29:20] */
        unsigned int rsv_8            : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} solid_pattern_config;

typedef union {
    struct {
        unsigned int mask_pattern_cb : 10; /* [9:0] */
        unsigned int mask_pattern_y  : 10; /* [19:10] */
        unsigned int mask_pattern_cr : 10; /* [29:20] */
        unsigned int rsv_9           : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} mask_pattern_config;

typedef union {
    struct {
        unsigned int colorbar_width : 12; /* [11:0] */
        unsigned int square_height  : 12; /* [23:12] */
        unsigned int rsv_10         : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} bar_ext_config;

typedef union {
    struct {
        unsigned int bar_pattern_a0 : 30; /* [29:0] */
        unsigned int rsv_11         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a0;

typedef union {
    struct {
        unsigned int bar_pattern_a1 : 30; /* [29:0] */
        unsigned int rsv_12         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a1;

typedef union {
    struct {
        unsigned int bar_pattern_a2 : 30; /* [29:0] */
        unsigned int rsv_13         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a2;

typedef union {
    struct {
        unsigned int bar_pattern_a3 : 30; /* [29:0] */
        unsigned int rsv_14         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a3;

typedef union {
    struct {
        unsigned int bar_pattern_a4 : 30; /* [29:0] */
        unsigned int rsv_15         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a4;

typedef union {
    struct {
        unsigned int bar_pattern_a5 : 30; /* [29:0] */
        unsigned int rsv_16         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a5;

typedef union {
    struct {
        unsigned int bar_pattern_a6 : 30; /* [29:0] */
        unsigned int rsv_17         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a6;

typedef union {
    struct {
        unsigned int bar_pattern_a7 : 30; /* [29:0] */
        unsigned int rsv_18         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_a7;

typedef union {
    struct {
        unsigned int bar_pattern_b0 : 30; /* [29:0] */
        unsigned int rsv_19         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b0;

typedef union {
    struct {
        unsigned int bar_pattern_b1 : 30; /* [29:0] */
        unsigned int rsv_20         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b1;

typedef union {
    struct {
        unsigned int bar_pattern_b2 : 30; /* [29:0] */
        unsigned int rsv_21         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b2;

typedef union {
    struct {
        unsigned int bar_pattern_b3 : 30; /* [29:0] */
        unsigned int rsv_22         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b3;

typedef union {
    struct {
        unsigned int bar_pattern_b4 : 30; /* [29:0] */
        unsigned int rsv_23         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b4;

typedef union {
    struct {
        unsigned int bar_pattern_b5 : 30; /* [29:0] */
        unsigned int rsv_24         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b5;

typedef union {
    struct {
        unsigned int bar_pattern_b6 : 30; /* [29:0] */
        unsigned int rsv_25         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b6;

typedef union {
    struct {
        unsigned int bar_pattern_b7 : 30; /* [29:0] */
        unsigned int rsv_26         : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} cbar_pattern_b7;

typedef union {
    struct {
        unsigned int sync_polarity_force  : 1;  /* [0] */
        unsigned int hsync_polarity_value : 1;  /* [1] */
        unsigned int vsync_polarity_value : 1;  /* [2] */
        unsigned int fdt_status_clear     : 1;  /* [3] */
        unsigned int pixel_cnt_threhold   : 4;  /* [7:4] */
        unsigned int rsv_27               : 24; /* [31:8] */
    } bits;
    unsigned int u32;
} format_det_config;

typedef union {
    struct {
        unsigned int interlaced     : 1;  /* [0] */
        unsigned int hsync_polarity : 1;  /* [1] */
        unsigned int vsync_polarity : 1;  /* [2] */
        unsigned int rsv_28         : 29; /* [31:3] */
    } bits;
    unsigned int u32;
} fdet_status;

typedef union {
    struct {
        unsigned int hsync_active_cnt : 13; /* [12:0] */
        unsigned int hsync_total_cnt  : 13; /* [25:13] */
        unsigned int rsv_29           : 6;  /* [31:26] */
    } bits;
    unsigned int u32;
} fdet_hori_res;

typedef union {
    struct {
        unsigned int vsync_active_cnt : 13; /* [12:0] */
        unsigned int vsync_total_cnt  : 13; /* [25:13] */
        unsigned int rsv_30           : 6;  /* [31:26] */
    } bits;
    unsigned int u32;
} fdet_hori_vert_res;

typedef union {
    struct {
        unsigned int dither_rnd_byp      : 1;  /* [0] */
        unsigned int dither_mode         : 2;  /* [2:1] */
        unsigned int dither_rnd_en       : 1;  /* [3] */
        unsigned int dither_spatial_en   : 1;  /* [4] */
        unsigned int dither_spatial_dual : 1;  /* [5] */
        unsigned int rsv_31              : 26; /* [31:6] */
    } bits;
    unsigned int u32;
} dither_config;

typedef union {
    struct {
        unsigned int range_clip_byp : 1;  /* [0] */
        unsigned int clip_rgb_mode  : 1;  /* [1] */
        unsigned int rsv_32         : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} clip_ctrl;

typedef union {
    struct {
        unsigned int clip_y_min : 12; /* [11:0] */
        unsigned int clip_y_max : 12; /* [23:12] */
        unsigned int rsv_33     : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} clip_y_config;

typedef union {
    struct {
        unsigned int clip_c_min : 12; /* [11:0] */
        unsigned int clip_c_max : 12; /* [23:12] */
        unsigned int rsv_34     : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} clip_c_config;

typedef union {
    struct {
        unsigned int auto_trigger_en : 1;  /* [0] */
        unsigned int soft_trigger_en : 1;  /* [1] */
        unsigned int show_point_en   : 1;  /* [2] */
        unsigned int rsv_35          : 1;  /* [3] */
        unsigned int cap_stat_done   : 1;  /* [4] */
        unsigned int cap_stat_busy   : 1;  /* [5] */
        unsigned int cap_stat_error  : 1;  /* [6] */
        unsigned int rsv_36          : 25; /* [31:7] */
    } bits;
    unsigned int u32;
} pxl_cap_ctrl;

typedef union {
    struct {
        unsigned int cap_pixel_position : 13; /* [12:0] */
        unsigned int cap_line_position  : 13; /* [25:13] */
        unsigned int rsv_37             : 6;  /* [31:26] */
    } bits;
    unsigned int u32;
} pxl_cap_position;

typedef union {
    struct {
        unsigned int capture_y_value : 12; /* [11:0] */
        unsigned int rsv_38          : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} cap_y_value;

typedef union {
    struct {
        unsigned int capture_cb_value : 12; /* [11:0] */
        unsigned int rsv_39           : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} cap_cb_value;

typedef union {
    struct {
        unsigned int capture_cr_value : 12; /* [11:0] */
        unsigned int rsv_40           : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} cap_cr_value;

typedef union {
    struct {
        unsigned int reg_csc_en          : 1;  /* [0] */
        unsigned int reg_csc_coef_ext    : 1;  /* [1] */
        unsigned int reg_csc_dither_en   : 1;  /* [2] */
        unsigned int reg_csc_saturate_en : 1;  /* [3] */
        unsigned int reg_csc_mode        : 8;  /* [11:4] */
        unsigned int rsv_41              : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} multi_csc_ctrl;

typedef union {
    struct {
        unsigned int reg_csc_r1c1 : 16; /* [15:0] */
        unsigned int rsv_42       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff11;

typedef union {
    struct {
        unsigned int reg_csc_r1c2 : 16; /* [15:0] */
        unsigned int rsv_43       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff12;

typedef union {
    struct {
        unsigned int reg_csc_r1c3 : 16; /* [15:0] */
        unsigned int rsv_44       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff13;

typedef union {
    struct {
        unsigned int reg_csc_r2c1 : 16; /* [15:0] */
        unsigned int rsv_45       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff21;

typedef union {
    struct {
        unsigned int reg_csc_r2c2 : 16; /* [15:0] */
        unsigned int rsv_46       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff22;

typedef union {
    struct {
        unsigned int reg_csc_r2c3 : 16; /* [15:0] */
        unsigned int rsv_47       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff23;

typedef union {
    struct {
        unsigned int reg_csc_r3c1 : 16; /* [15:0] */
        unsigned int rsv_48       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff31;

typedef union {
    struct {
        unsigned int reg_csc_r3c2 : 16; /* [15:0] */
        unsigned int rsv_49       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff32;

typedef union {
    struct {
        unsigned int reg_csc_r3c3 : 16; /* [15:0] */
        unsigned int rsv_50       : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} multi_csc_coeff33;

typedef union {
    struct {
        unsigned int reg_csc_inoffset_y : 13; /* [12:0] */
        unsigned int rsv_51             : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} multi_csc_inoffset_y;

typedef union {
    struct {
        unsigned int reg_csc_inoffset_cb : 13; /* [12:0] */
        unsigned int rsv_52              : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} multi_csc_inoffset_cb;

typedef union {
    struct {
        unsigned int reg_csc_inoffset_cr : 13; /* [12:0] */
        unsigned int rsv_53              : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} multi_csc_inoffset_cr;

typedef union {
    struct {
        unsigned int reg_csc_outoffset_y : 13; /* [12:0] */
        unsigned int rsv_54              : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} multi_csc_outoffset_y;

typedef union {
    struct {
        unsigned int reg_csc_outoffset_cb : 13; /* [12:0] */
        unsigned int rsv_55               : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} multi_csc_outoffset_cb;

typedef union {
    struct {
        unsigned int reg_csc_outoffset_cr : 13; /* [12:0] */
        unsigned int rsv_56               : 19; /* [31:13] */
    } bits;
    unsigned int u32;
} multi_csc_outoffset_cr;

typedef union {
    struct {
        unsigned int reg_dwsm_hori_en   : 1;  /* [0] */
        unsigned int reg_hori_filter_en : 1;  /* [1] */
        unsigned int reg_dwsm_vert_en   : 1;  /* [2] */
        unsigned int reg_dwsm_vert_byp  : 1;  /* [3] */
        unsigned int reg_vert_cbcr_sel  : 1;  /* [4] */
        unsigned int rsv_57             : 27; /* [31:5] */
    } bits;
    unsigned int u32;
} video_dwsm_ctrl;

typedef union {
    struct {
        unsigned int reg_demux_420_en     : 1;  /* [0] */
        unsigned int reg_ddr_en           : 1;  /* [1] */
        unsigned int reg_yc_mux_en        : 1;  /* [2] */
        unsigned int reg_blank_replace_en : 1;  /* [3] */
        unsigned int reg_pixel_rate       : 2;  /* [5:4] */
        unsigned int reg_ddr_polarity     : 1;  /* [6] */
        unsigned int reg_yc_mux_polarity  : 1;  /* [7] */
        unsigned int reg_cbcr_order       : 1;  /* [8] */
        unsigned int reg_demux_cb_or_cr   : 1;  /* [9] */
        unsigned int reg_pxl_div_en       : 1;  /* [10] */
        unsigned int rsv_58               : 21; /* [31:11] */
    } bits;
    unsigned int u32;
} data_align_ctrl;

typedef union {
    struct {
        unsigned int reg_blank_y : 12; /* [11:0] */
        unsigned int rsv_59      : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} blank_data_y;

typedef union {
    struct {
        unsigned int reg_blank_cb : 12; /* [11:0] */
        unsigned int rsv_60       : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} blank_data_cb;

typedef union {
    struct {
        unsigned int reg_blank_cr : 12; /* [11:0] */
        unsigned int rsv_61       : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} blank_data_cr;

typedef union {
    struct {
        unsigned int reg_vmux_y_sel  : 3; /* [2:0] */
        unsigned int reg_vmux_cb_sel : 3; /* [5:3] */
        unsigned int reg_vmux_cr_sel : 3; /* [8:6] */
        unsigned int reg_bitmask_y   : 2; /* [10:9] */
        unsigned int reg_bitmask_cb  : 2; /* [12:11] */
        unsigned int reg_bitmask_cr  : 2; /* [14:13] */
        unsigned int reg_bitrev_en   : 3; /* [17:15] */
        unsigned int reg_datamask_en : 3; /* [20:18] */
        unsigned int reg_syncmask_en : 4; /* [24:21] */
        unsigned int reg_inver_sync  : 4; /* [28:25] */
        unsigned int rsv_62          : 3; /* [31:29] */
    } bits;
    unsigned int u32;
} video_dmux_ctrl;

typedef struct {
    volatile timing_gen_ctrl        tim_gen_ctrl;           /* 800 */
    volatile hsync_timing_config0   hsync_timing_cfg0;      /* 804 */
    volatile hsync_timing_config1   hsync_timing_cfg1;      /* 808 */
    volatile hsync_timing_config2   hsync_timing_cfg2;      /* 80C */
    volatile vsync_timing_config0   vsync_timing_cfg0;      /* 810 */
    volatile vsync_timing_config1   vsync_timing_cfg1;      /* 814 */
    volatile vsync_timing_config2   vsync_timing_cfg2;      /* 818 */
    unsigned int                    reserved_0[5];          /* 81C-82C */
    volatile video_path_ctrl        vid_path_ctrl;          /* 830 */
    unsigned int                    reserved_1[3];          /* 834-83c */
    volatile pattern_gen_ctrll      ptn_gen_ctrl;           /* 840 */
    volatile solid_pattern_config   solid_ptn_ctrl;         /* 844 */
    volatile mask_pattern_config    mask_ptn_ctrl;          /* 848 */
    volatile bar_ext_config         bar_ext_cfg;            /* 84C */
    volatile cbar_pattern_a0        cbar_a0;                /* 850 */
    volatile cbar_pattern_a1        cbar_a1;                /* 854 */
    volatile cbar_pattern_a2        cbar_a2;                /* 858 */
    volatile cbar_pattern_a3        cbar_a3;                /* 85C */
    volatile cbar_pattern_a4        cbar_a4;                /* 860 */
    volatile cbar_pattern_a5        cbar_a5;                /* 864 */
    volatile cbar_pattern_a6        cbar_a6;                /* 868 */
    volatile cbar_pattern_a7        cbar_a7;                /* 86C */
    volatile cbar_pattern_b0        cbar_b0;                /* 870 */
    volatile cbar_pattern_b1        cbar_b1;                /* 874 */
    volatile cbar_pattern_b2        cbar_b2;                /* 878 */
    volatile cbar_pattern_b3        cbar_b3;                /* 87C */
    volatile cbar_pattern_b4        cbar_b4;                /* 880 */
    volatile cbar_pattern_b5        cbar_b5;                /* 884 */
    volatile cbar_pattern_b6        cbar_b6;                /* 888 */
    volatile cbar_pattern_b7        cbar_b7;                /* 88C */
    unsigned int                    reserved_2[10];         /* 890-8B4 */
    volatile format_det_config      fmt_det_cfg;            /* 8B8 */
    volatile fdet_status            fmt_dect_status;        /* 8BC */
    volatile fdet_hori_res          fmt_dect_hori_res;      /* 8C0 */
    volatile fdet_hori_vert_res     fmt_dect_hori_vert_res; /* 8C4 */
    unsigned int                    reserved_3[9];          /* 8C8-8E8 */
    volatile dither_config          dither_cfg;             /* 8EC */
    unsigned int                    reserved_4[2];          /* 8F0-8F4 */
    volatile clip_ctrl              clip_ctrl;              /* 8F8 */
    volatile clip_y_config          clip_y_cfg;             /* 8FC */
    volatile clip_c_config          clip_c_cfg;             /* 900 */
    unsigned int                    reserved_5[2];          /* 904-908 */
    volatile pxl_cap_ctrl           capture_ctrl;           /* 90C */
    volatile pxl_cap_position       capture_cfg;            /* 910 */
    volatile cap_y_value            pxl_y_capture;          /* 914 */
    volatile cap_cb_value           pxl_cb_capture;         /* 918 */
    volatile cap_cr_value           pxl_cr_capture;         /* 91C */
    unsigned int                    reserved_6[5];          /* 920-930 */
    volatile multi_csc_ctrl         csc_ctrl;               /* 934 */
    volatile multi_csc_coeff11      csc_coeff11;            /* 938 */
    volatile multi_csc_coeff12      csc_coeff12;            /* 93C */
    volatile multi_csc_coeff13      csc_coeff13;            /* 940 */
    volatile multi_csc_coeff21      csc_coeff21;            /* 944 */
    volatile multi_csc_coeff22      csc_coeff22;            /* 948 */
    volatile multi_csc_coeff23      csc_coeff23;            /* 94C */
    volatile multi_csc_coeff31      csc_coeff31;            /* 950 */
    volatile multi_csc_coeff32      csc_coeff32;            /* 954 */
    volatile multi_csc_coeff33      csc_coeff33;            /* 958 */
    volatile multi_csc_inoffset_y   y_in_offset;            /* 95C */
    volatile multi_csc_inoffset_cb  cb_in_offset;           /* 960 */
    volatile multi_csc_inoffset_cr  cr_in_offset;           /* 964 */
    volatile multi_csc_outoffset_y  y_out_offset;           /* 968 */
    volatile multi_csc_outoffset_cb cb_out_offset;          /* 96C */
    volatile multi_csc_outoffset_cr cr_out_offset;          /* 970 */
    unsigned int                    reserved_7[3];          /* 974-97c */
    volatile video_dwsm_ctrl        dwsm_ctrl;              /* 980 */
    unsigned int                    reserved_8[2];          /* 984-988 */
    volatile data_align_ctrl        align_ctrl;             /* 98C */
    volatile blank_data_y           blk_data_y;             /* 990 */
    volatile blank_data_cb          blk_data_cb;            /* 994 */
    volatile blank_data_cr          blk_data_cr;            /* 998 */
    unsigned int                    reserved_9[3];         /* 99C-9A4 */
    volatile video_dmux_ctrl        dmux_ctrl;              /* 9A8 */
} video_path_reg_regs_type;

int hdmi_reg_video_path_regs_init(hi_char *addr);
int hdmi_reg_video_path_regs_deinit(void);
int hdmi_reg_timing_gen_en_set(unsigned int reg_timing_gen_en);
hi_u32 hdmi_reg_timing_gen_en_get(hi_void);
int hdmi_reg_extmode_set(unsigned int reg_extmode);
hi_u32 hdmi_reg_extmode_get(hi_void);
int hdmi_reg_timing_sel_set(unsigned int reg_timing_sel);
hi_u32 hdmi_reg_timing_sel_get(hi_void);
int hdmi_reg_sync_polarity_set(unsigned int reg_sync_polarity);
hi_u32 hdmi_reg_sync_polarity_get(hi_void);
int hdmi_reg_video_blank_en_set(unsigned int reg_video_blank_en);
hi_u32 hdmi_reg_video_blank_en_get(hi_void);
int hdmi_reg_video_lp_disable_set(unsigned int reg_video_lp_disable);
int hdmi_reg_video_format_set(unsigned int video_format);
hi_u32 hdmi_reg_video_format_get(hi_void);
int hdmi_reg_solid_pattern_en_set(unsigned int solid_pattern_en);
hi_u32 hdmi_reg_solid_pattern_en_get(hi_void);
int hdmi_reg_colorbar_en_set(unsigned int colorbar_en);
hi_u32 hdmi_reg_colorbar_en_get(hi_void);
int hdmi_reg_square_pattern_en_set(unsigned int square_pattern_en);
hi_u32 hdmi_reg_square_pattern_en_get(hi_void);
int hdmi_reg_mask_pattern_en_set(unsigned int mask_pattern_en);
hi_u32 hdmi_reg_mask_pattern_en_get(hi_void);
int hdmi_reg_cbar_pattern_sel_set(unsigned int cbar_pattern_sel);
int hdmi_reg_solid_pattern_cb_set(unsigned int solid_pattern_cb);
int hdmi_reg_solid_pattern_y_set(unsigned int solid_pattern_y);
int hdmi_reg_solid_pattern_cr_set(unsigned int solid_pattern_cr);
int hdmi_reg_sync_polarity_force_set(unsigned int sync_polarity_force);
int hdmi_reg_fdt_status_clear_set(unsigned int fdt_status_clear);
hi_u32 hdmi_reg_interlaced_get(hi_void);
hi_u32 hdmi_reg_hsync_polarity_get(hi_void);
hi_u32 hdmi_reg_vsync_polarity_get(hi_void);
hi_u32 hdmi_reg_hsync_active_cnt_get(hi_void);
hi_u32 hdmi_reg_hsync_total_cnt_get(hi_void);
hi_u32 hdmi_reg_vsync_active_cnt_get(hi_void);
hi_u32 hdmi_reg_vsync_total_cnt_get(hi_void);
int hdmi_reg_dither_rnd_bypass_set(unsigned int dither_rnd_byp);
hi_u32 hdmi_reg_dither_rnd_bypass_get(hi_void);
int hdmi_reg_dither_mode_set(unsigned int dither_mode);
hi_u32 hdmi_reg_dither_mode_get(hi_void);
int hdmi_reg_csc_en_set(unsigned int reg_csc_en);
hi_u32 hdmi_reg_csc_en_get(hi_void);
int hdmi_reg_csc_saturate_en_set(unsigned int reg_csc_saturate_en);
int hdmi_reg_csc_mode_set(unsigned int reg_csc_mode);
hi_u32 hdmi_reg_csc_mode_get(hi_void);
int hdmi_reg_dwsm_hori_en_set(unsigned int reg_dwsm_hori_en);
hi_u32 hdmi_reg_dwsm_hori_en_get(hi_void);
int hdmi_reg_hori_filter_en_set(unsigned int reg_hori_filter_en);
hi_u32 hdmi_reg_hori_filter_en_get(hi_void);
int hdmi_reg_dwsm_vert_en_set(unsigned int reg_dwsm_vert_en);
hi_u32 hdmi_reg_dwsm_vert_en_get(hi_void);
int hdmi_reg_dwsm_vert_bypass_set(unsigned int reg_dwsm_vert_byp);
hi_u32 hdmi_reg_dwsm_vert_bypass_get(hi_void);
int hdmi_reg_demux_420_en_set(unsigned int reg_demux_420_en);
hi_u32 hdmi_reg_demux_420_en_get(hi_void);
int hdmi_reg_pxl_div_en_set(unsigned int reg_pxl_div_en);
hi_u32 hdmi_reg_pxl_div_en_get(hi_void);
int hdmi_reg_vmux_y_sel_set(unsigned int reg_vmux_y_sel);
hi_u32 hdmi_reg_vmux_y_sel_get(hi_void);
int hdmi_reg_vmux_cb_sel_set(unsigned int reg_vmux_cb_sel);
hi_u32 hdmi_reg_vmux_cb_sel_get(hi_void);
int hdmi_reg_vmux_cr_sel_set(unsigned int reg_vmux_cr_sel);
hi_u32 hdmi_reg_vmux_cr_sel_get(hi_void);
int hdmi_reg_syncmask_en_set(unsigned int reg_syncmask_en);
int hdmi_reg_inver_sync_set(unsigned int reg_inver_sync);
hi_u32 hdmi_reg_inver_sync_get(hi_void);

#endif  /* __HDMI_REG_VIDEO_PATH_H__ */


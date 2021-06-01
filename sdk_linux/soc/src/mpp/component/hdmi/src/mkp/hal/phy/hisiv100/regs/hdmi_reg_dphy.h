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
#ifndef __HDMI_REG_DPHY_H__
#define __HDMI_REG_DPHY_H__
#include "hi_type.h"

typedef union {
    struct {
        unsigned int reg_set               : 24; /* [23:0] */
        unsigned int reg_set_load          : 1;  /* [24] */
        unsigned int reg_sscfifo_tune_en   : 1;  /* [25] */
        unsigned int reg_sscfifo_depth_clr : 1;  /* [26] */
        unsigned int reg_deep_color        : 2;  /* [28:27] */
        unsigned int reg_ssc_mode          : 1;  /* [29] */
        unsigned int reg_pllfbmash111_en   : 1;  /* [30] */
        unsigned int reg_sscin_bypass_en   : 1;  /* [31] */
    } bits;
    unsigned int u32;
} hdmitx_inssc_set;

typedef union {
    struct {
        unsigned int reg_tune_step            : 8;  /* [7:0] */
        unsigned int reg_sscfifo_lower_thresh : 10; /* [17:8] */
        unsigned int reg_sscfifo_upper_thresh : 10; /* [27:18] */
        unsigned int reg_tune_reverse         : 1;  /* [28] */
        unsigned int reg_ssc_sync             : 1;  /* [29] */
        unsigned int rsv_0                    : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} hdmitx_inssc_fifo;

typedef union {
    struct {
        unsigned int reg_step  : 10; /* [9:0] */
        unsigned int reg_span  : 14; /* [23:10] */
        unsigned int reg_testd : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_inssc_step;

typedef union {
    struct {
        unsigned int reg_tune_interval : 32; /* [31:0] */
    } bits;
    unsigned int u32;
} hdmitx_inssc_tune;

typedef union {
    struct {
        unsigned int reg_dphy_srst_req : 1;  /* [0] */
        unsigned int rsv_0             : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} hdmitx_dphy_rst;

typedef union {
    struct {
        unsigned int reg_dnum           : 5;  /* [4:0] */
        unsigned int reg_hkmash_en      : 1;  /* [5] */
        unsigned int reg_mash111_hk_sel : 1;  /* [6] */
        unsigned int rsv_1              : 25; /* [31:7] */
    } bits;
    unsigned int u32;
} hdmitx_ssc_hk;

typedef union {
    struct {
        unsigned int sscin_span_cnt     : 15; /* [14:0] */
        unsigned int sscin_segment      : 2;  /* [16:15] */
        unsigned int sscin_stop_flag    : 1;  /* [17] */
        unsigned int sscin_reverse_flag : 1;  /* [18] */
        unsigned int sscin_div          : 5;  /* [23:19] */
        unsigned int rsv_2              : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_sscin_status0;

typedef union {
    struct {
        unsigned int sscin_ssc_offset : 24; /* [23:0] */
        unsigned int rsv_3            : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_sscin_status1;

typedef union {
    struct {
        unsigned int sscin_sscfifo_depth_max : 10; /* [9:0] */
        unsigned int sscin_sscfifo_depth_min : 10; /* [19:10] */
        unsigned int rsv_4                   : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} hdmitx_sscin_status2;

typedef union {
    struct {
        unsigned int sscfb_span_cnt     : 15; /* [14:0] */
        unsigned int sscfb_segment      : 2;  /* [16:15] */
        unsigned int sscfb_stop_flag    : 1;  /* [17] */
        unsigned int sscfb_reverse_flag : 1;  /* [18] */
        unsigned int sscfb_div          : 5;  /* [23:19] */
        unsigned int rsv_5              : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_sscfb_status0;

typedef union {
    struct {
        unsigned int sscfb_ssc_offset : 24; /* [23:0] */
        unsigned int rsv_6            : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_sscfb_status1;

typedef union {
    struct {
        unsigned int sscfb_sscfifo_depth_max : 10; /* [9:0] */
        unsigned int sscfb_sscfifo_depth_min : 10; /* [19:10] */
        unsigned int rsv_7                   : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} hdmitx_sscfb_status2;

typedef union {
    struct {
        unsigned int fbmash_span_cnt     : 15; /* [14:0] */
        unsigned int fbmash_segment      : 2;  /* [16:15] */
        unsigned int fbmash_stop_flag    : 1;  /* [17] */
        unsigned int fbmash_reverse_flag : 1;  /* [18] */
        unsigned int fbmash_div          : 5;  /* [23:19] */
        unsigned int rsv_8               : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_sscfbmash111_status0;

typedef union {
    struct {
        unsigned int fbmash_ssc_offset : 24; /* [23:0] */
        unsigned int rsv_9             : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_sscfbmash111_status1;

typedef union {
    struct {
        unsigned int fbmash_sscfifo_depth_max : 10; /* [9:0] */
        unsigned int fbmash_sscfifo_depth_min : 10; /* [19:10] */
        unsigned int rsv_10                   : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} hdmitx_sscfbmash111_status2;

typedef union {
    struct {
        unsigned int reg_aphy_data_ch0_h : 10; /* [9:0] */
        unsigned int reg_aphy_data_ch1_h : 10; /* [19:10] */
        unsigned int reg_aphy_data_ch2_h : 10; /* [29:20] */
        unsigned int rsv_11              : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} hdmitx_afifo_data_high;

typedef union {
    struct {
        unsigned int reg_txafifo_status_rst : 1;  /* [0] */
        unsigned int reg_txafifo_rst        : 1;  /* [1] */
        unsigned int reg_aphy_data_ch0_sel  : 1;  /* [2] */
        unsigned int reg_aphy_data_ch1_sel  : 1;  /* [3] */
        unsigned int reg_aphy_data_ch2_sel  : 1;  /* [4] */
        unsigned int txafifo_aful_status    : 1;  /* [5] */
        unsigned int txafifo_full_status    : 1;  /* [6] */
        unsigned int txafifo_aempt_status   : 1;  /* [7] */
        unsigned int txafifo_empt_status    : 1;  /* [8] */
        unsigned int reg_aphy_data_clk_h    : 10; /* [18:9] */
        unsigned int rsv_12                 : 13; /* [31:19] */
    } bits;
    unsigned int u32;
} hdmitx_afifo_data_sel;

typedef union {
    struct {
        unsigned int reg_aphy_data_clk_l : 30; /* [29:0] */
        unsigned int rsv_13              : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} hdmitx_afifo_clk;

typedef union {
    struct {
        unsigned int reg_aphy_data_ch0 : 10; /* [9:0] */
        unsigned int reg_aphy_data_ch1 : 10; /* [19:10] */
        unsigned int reg_aphy_data_ch2 : 10; /* [29:20] */
        unsigned int rsv_14            : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} hdmitx_afifo_data_low;

typedef union {
    struct {
        unsigned int reg_prbs7_en        : 1;  /* [0] */
        unsigned int reg_prbs7_clr       : 1;  /* [1] */
        unsigned int reg_prbs7_errinject : 1;  /* [2] */
        unsigned int reg_ck_dataout_sel  : 3;  /* [5:3] */
        unsigned int reg_d0_dataout_sel  : 3;  /* [8:6] */
        unsigned int reg_d1_dataout_sel  : 3;  /* [11:9] */
        unsigned int reg_d2_dataout_sel  : 3;  /* [14:12] */
        unsigned int rsv_15              : 17; /* [31:15] */
    } bits;
    unsigned int u32;
} hdmitx_afifo_channel;

typedef union {
    struct {
        unsigned int ad_rxsense      : 1;  /* [0] */
        unsigned int ad_txpll_hvflag : 1;  /* [1] */
        unsigned int ad_txpll_lock   : 1;  /* [2] */
        unsigned int rsv_16          : 29; /* [31:3] */
    } bits;
    unsigned int u32;
} aphy_status0;

typedef union {
    struct {
        unsigned int reg_gc_pd         : 4; /* [3:0] */
        unsigned int reg_gc_pd_bist    : 1; /* [4] */
        unsigned int reg_gc_pd_de      : 4; /* [8:5] */
        unsigned int reg_gc_pd_ldo     : 2; /* [10:9] */
        unsigned int reg_gc_pd_rterm   : 8; /* [18:11] */
        unsigned int reg_gc_pd_rxsense : 1; /* [19] */
        unsigned int reg_gc_txpll_pd   : 1; /* [20] */
        unsigned int reg_divsel        : 2; /* [22:21] */
        unsigned int rsv_17            : 9; /* [31:23] */
    } bits;
    unsigned int u32;
} aphy_top_pd;

typedef union {
    struct {
        unsigned int reg_isel_main_clk    : 6; /* [5:0] */
        unsigned int reg_isel_main_d0     : 6; /* [11:6] */
        unsigned int reg_isel_main_d1     : 6; /* [17:12] */
        unsigned int reg_isel_main_d2     : 6; /* [23:18] */
        unsigned int reg_isel_main_de_clk : 6; /* [29:24] */
        unsigned int rsv_18               : 2; /* [31:30] */
    } bits;
    unsigned int u32;
} aphy_driver_imain;

typedef union {
    struct {
        unsigned int reg_isel_main_de_d0 : 6; /* [5:0] */
        unsigned int reg_isel_main_de_d1 : 6; /* [11:6] */
        unsigned int reg_isel_main_de_d2 : 6; /* [17:12] */
        unsigned int reg_isel_pre_clk    : 6; /* [23:18] */
        unsigned int reg_isel_pre_d0     : 6; /* [29:24] */
        unsigned int rsv_19              : 2; /* [31:30] */
    } bits;
    unsigned int u32;
} aphy_driver_ipre;

typedef union {
    struct {
        unsigned int reg_isel_pre_d1     : 6; /* [5:0] */
        unsigned int reg_isel_pre_d2     : 6; /* [11:6] */
        unsigned int reg_isel_pre_de_clk : 6; /* [17:12] */
        unsigned int reg_isel_pre_de_d0  : 6; /* [23:18] */
        unsigned int reg_isel_pre_de_d1  : 6; /* [29:24] */
        unsigned int rsv_20              : 2; /* [31:30] */
    } bits;
    unsigned int u32;
} aphy_driver_iprede;

typedef union {
    struct {
        unsigned int reg_isel_pre_de_d2  : 6; /* [5:0] */
        unsigned int reg_rsel_pre_clk    : 3; /* [8:6] */
        unsigned int reg_rsel_pre_d0     : 3; /* [11:9] */
        unsigned int reg_rsel_pre_d1     : 3; /* [14:12] */
        unsigned int reg_rsel_pre_d2     : 3; /* [17:15] */
        unsigned int reg_rsel_pre_de_clk : 3; /* [20:18] */
        unsigned int reg_rsel_pre_de_d0  : 3; /* [23:21] */
        unsigned int reg_rsel_pre_de_d1  : 3; /* [26:24] */
        unsigned int reg_rsel_pre_de_d2  : 3; /* [29:27] */
        unsigned int rsv_21              : 2; /* [31:30] */
    } bits;
    unsigned int u32;
} aphy_driver_rpre;

typedef union {
    struct {
        unsigned int reg_rt_clk : 8; /* [7:0] */
        unsigned int reg_rt_d0  : 8; /* [15:8] */
        unsigned int reg_rt_d1  : 8; /* [23:16] */
        unsigned int reg_rt_d2  : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aphy_rterm_ctrl;

typedef union {
    struct {
        unsigned int reg_sel_ldo     : 6;  /* [5:0] */
        unsigned int reg_sel_rxsense : 3;  /* [8:6] */
        unsigned int reg_sr_clk      : 2;  /* [10:9] */
        unsigned int rsv_22          : 21; /* [31:11] */
    } bits;
    unsigned int u32;
} aphy_misc_ctrl;

typedef union {
    struct {
        unsigned int reg_test    : 16; /* [15:0] */
        unsigned int reg_test_dc : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} aphy_test_ctrl;

typedef union {
    struct {
        unsigned int reg_txpll_clkin_sel         : 2;  /* [1:0] */
        unsigned int reg_txpll_divsel_in         : 3;  /* [4:2] */
        unsigned int reg_txpll_divsel_loop       : 7;  /* [11:5] */
        unsigned int reg_txpll_divsel_post       : 2;  /* [13:12] */
        unsigned int reg_txpll_icp_ictrl         : 4;  /* [17:14] */
        unsigned int reg_gc_txpll_en_ref_fb_div2 : 1;  /* [18] */
        unsigned int reg_gc_txpll_en_sscdiv      : 1;  /* [19] */
        unsigned int rsv_23                      : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} aphy_txpll_ctrl;

typedef union {
    struct {
        unsigned int reg_txpll_test : 20; /* [19:0] */
        unsigned int rsv_24         : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} aphy_txpll_test;

typedef union {
    struct {
        unsigned int reg_fb_div_sel       : 2; /* [1:0] */
        unsigned int reg_fb_prd_sel       : 2; /* [3:2] */
        unsigned int reg_ref_lock_timer   : 8; /* [11:4] */
        unsigned int reg_ref_unlock_timer : 8; /* [19:12] */
        unsigned int reg_pll_lock_sel     : 2; /* [21:20] */
        unsigned int reg_pll_unlock_sel   : 1; /* [22] */
        unsigned int reg_pll_lockext_sel  : 3; /* [25:23] */
        unsigned int reg_pll_lock_times   : 1; /* [26] */
        unsigned int rsv_25               : 5; /* [31:27] */
    } bits;
    unsigned int u32;
} pll_lock_ctrl;

typedef union {
    struct {
        unsigned int clk_fb_exist     : 1;  /* [0] */
        unsigned int pll_ena_edge     : 1;  /* [1] */
        unsigned int test_pll_cnt     : 17; /* [18:2] */
        unsigned int pll_lock_oneshot : 1;  /* [19] */
        unsigned int hdmi_clk_lock    : 1;  /* [20] */
        unsigned int rsv_26           : 11; /* [31:21] */
    } bits;
    unsigned int u32;
} pll_lock_status;

typedef union {
    struct {
        unsigned int reg_txdphy_dummy_ctrl : 32; /* [31:0] */
    } bits;
    unsigned int u32;
} reg_txdphy_dummy_ctrl;

typedef union {
    struct {
        unsigned int reg_set_fb               : 24; /* [23:0] */
        unsigned int reg_set_load_fb          : 1;  /* [24] */
        unsigned int reg_sscfifo_tune_en_fb   : 1;  /* [25] */
        unsigned int reg_sscfifo_depth_clr_fb : 1;  /* [26] */
        unsigned int reg_ssc_mode_fb          : 1;  /* [27] */
        unsigned int rsv_27                   : 4;  /* [31:28] */
    } bits;
    unsigned int u32;
} hdmitx_fbssc_set;

typedef union {
    struct {
        unsigned int reg_tune_step_fb            : 8;  /* [7:0] */
        unsigned int reg_sscfifo_lower_thresh_fb : 10; /* [17:8] */
        unsigned int reg_sscfifo_upper_thresh_fb : 10; /* [27:18] */
        unsigned int reg_tune_reverse_fb         : 1;  /* [28] */
        unsigned int reg_ssc_sync_fb             : 1;  /* [29] */
        unsigned int rsv_28                      : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} hdmitx_fbssc_fifo;

typedef union {
    struct {
        unsigned int reg_step_fb  : 10; /* [9:0] */
        unsigned int reg_span_fb  : 14; /* [23:10] */
        unsigned int reg_testd_fb : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} hdmitx_fbssc_step;

typedef union {
    struct {
        unsigned int reg_tune_interval_fb : 32; /* [31:0] */
    } bits;
    unsigned int u32;
} hdmitx_fbssc_tune;

typedef struct {
    volatile hdmitx_inssc_set            ssc_in_set;         /* 0 */
    volatile hdmitx_inssc_fifo           ssc_in_fifo;        /* 4 */
    volatile hdmitx_inssc_step           ssc_in_step;        /* 8 */
    volatile hdmitx_inssc_tune           ssc_in_tune;        /* C */
    volatile hdmitx_dphy_rst             dphy_rst;           /* 0x10 */
    volatile hdmitx_ssc_hk               ssc_hk;             /* 14 */
    volatile unsigned int                reserved_1[2];      /* 0x18~0x1c */
    volatile hdmitx_sscin_status0        ssc_in_st0;         /* 20 */
    volatile hdmitx_sscin_status1        ssc_in_st1;         /* 24 */
    volatile hdmitx_sscin_status2        ssc_in_st2;         /* 28 */
    volatile hdmitx_sscfb_status0        ssc_fb_st0;         /* 2C */
    volatile hdmitx_sscfb_status1        ssc_fb_st1;         /* 30 */
    volatile hdmitx_sscfb_status2        ssc_fb_st2;         /* 34 */
    volatile hdmitx_sscfbmash111_status0 ssc_fb_mash111_st0; /* 38 */
    volatile hdmitx_sscfbmash111_status1 ssc_fb_mash111_st1; /* 3C */
    volatile hdmitx_sscfbmash111_status2 ssc_fb_mash111_st2; /* 40 */
    volatile unsigned int                reserved_2[2];      /* 0x44~0x48 */
    volatile hdmitx_afifo_data_high      afifo_data_high;    /* 4C */
    volatile hdmitx_afifo_data_sel       afifo_data_sel;     /* 50 */
    volatile hdmitx_afifo_clk            afifo_data_clk;     /* 54 */
    volatile hdmitx_afifo_data_low       afifo_data_low;     /* 58 */
    volatile hdmitx_afifo_channel        afifo_chn;          /* 5C */
    volatile aphy_status0                aphy_st0;           /* 60 */
    volatile aphy_top_pd                 aphy_top_misc_ctrl; /* 64 */
    volatile aphy_driver_imain           aphy_imain;         /* 68 */
    volatile aphy_driver_ipre            aphy_ipre;          /* 6C */
    volatile aphy_driver_iprede          aphy_iprede;        /* 70 */
    volatile aphy_driver_rpre            aphy_rpre;          /* 74 */
    volatile aphy_rterm_ctrl             rterm_ctrl;         /* 78 */
    volatile aphy_misc_ctrl              misc_ctrl;          /* 7C */
    volatile aphy_test_ctrl              test_ctrl_aphy;     /* 80 */
    volatile unsigned int                reserved_3[2];      /* 0x84~0x88 */
    volatile aphy_txpll_ctrl             txpll_ctrl;         /* 8C */
    volatile aphy_txpll_test             txpll_test;         /* 90 */
    volatile pll_lock_ctrl               lock_ctrl;          /* 94 */
    volatile pll_lock_status             lock_st;            /* 98 */
    volatile reg_txdphy_dummy_ctrl       dphy_dummy_ctrl;    /* 9C */
    volatile unsigned int                reserved_4[4];      /* 0xa0~0xac */
    volatile hdmitx_fbssc_set            fb_ssc_set;         /* B0 */
    volatile hdmitx_fbssc_fifo           fb_ssc_fifo;        /* B4 */
    volatile hdmitx_fbssc_step           fb_ssc_step;        /* B8 */
    volatile hdmitx_fbssc_tune           fb_ssc_tune;        /* 0xbc */
} hdmi2tx_dphy_reg_type;

hi_s32 hdmi_reg_tx_phy_init(hi_char *addr);
hi_s32 hdmi_reg_tx_phy_deinit(hi_void);
hi_s32 hdmi_reg_pllfbmash111_en_set(hi_u32 reg_pllfbmash111_en);
hi_s32 hdmi_reg_sscin_bypass_en_set(hi_u32 reg_sscin_bypass_en);
hi_s32 hdmi_reg_dphy_rst_set(hi_u32 reg_rst);
hi_s32 hdmi_reg_aphy_data_clk_height_set(hi_u32 reg_aphy_data_clk_h);
hi_s32 hdmi_reg_aphy_data_clk_low_set(hi_u32 reg_aphy_data_clk_l);
hi_s32 hdmi_reg_gc_pd_set(hi_u32 reg_gc_pd);
hi_u32 hdmi_reg_gc_pd_get(hi_void);
hi_s32 hdmi_reg_gc_pd_bist_set(hi_u32 reg_gc_pd_bist);
hi_u32 hdmi_reg_gc_pd_bist_get(hi_void);
hi_s32 hdmi_reg_gc_pd_de_set(hi_u32 reg_gc_pd_de);
hi_u32 hdmi_reg_gc_pd_de_get(hi_void);
hi_s32 hdmi_reg_gc_pd_ldo_set(hi_u32 reg_gc_pd_ldo);
hi_u32 hdmi_reg_gc_pd_ldo_get(hi_void);
hi_u32 hdmi_reg_gc_pd_rterm_get(hi_void);
hi_s32 hdmi_reg_gc_pd_rterm_set(hi_u32 reg_gc_pd_rterm);
hi_s32 hdmi_reg_gc_pd_rxsense_set(hi_u32 reg_gc_pd_rxsense);
hi_u32 hdmi_reg_gc_pd_rxsense_get(hi_void);
hi_s32 hdmi_reg_gc_txpll_pd_set(hi_u32 reg_gc_txpll_pd);
hi_u32 hdmi_reg_gc_txpll_pd_get(hi_void);
hi_s32 hdmi_reg_divsel_set(hi_u32 reg_divsel);
hi_s32 hdmi_reg_isel_main_clk_set(hi_u32 reg_isel_main_clk);
hi_s32 hdmi_reg_isel_main_d0_set(hi_u32 reg_isel_main_d0);
hi_s32 hdmi_reg_isel_main_d1_set(hi_u32 reg_isel_main_d1);
hi_s32 hdmi_reg_isel_main_d2_set(hi_u32 reg_isel_main_d2);
hi_s32 hdmi_reg_isel_main_de_clk_set(hi_u32 reg_isel_main_de_clk);
hi_u32 hdmi_reg_isel_main_de_clk_get(hi_void);
hi_s32 hdmi_reg_isel_main_de_d0_set(hi_u32 reg_isel_main_de_d0);
hi_u32 hdmi_reg_isel_main_de_d0_get(hi_void);
hi_s32 hdmi_reg_isel_main_de_d1_set(hi_u32 reg_isel_main_de_d1);
hi_u32 hdmi_reg_isel_main_de_d1_get(hi_void);
hi_s32 hdmi_reg_isel_main_de_d2_set(hi_u32 reg_isel_main_de_d2);
hi_u32 hdmi_reg_isel_main_de_d2_get(hi_void);
hi_s32 hdmi_reg_isel_pre_clk_set(hi_u32 reg_isel_pre_clk);
hi_s32 hdmi_reg_isel_pre_d0_set(hi_u32 reg_isel_pre_d0);
hi_s32 hdmi_reg_isel_pre_d1_set(hi_u32 reg_isel_pre_d1);
hi_s32 hdmi_reg_isel_pre_d2_set(hi_u32 reg_isel_pre_d2);
hi_s32 hdmi_reg_isel_pre_de_clk_set(hi_u32 reg_isel_pre_de_clk);
hi_s32 hdmi_reg_isel_pre_de_d0_set(hi_u32 reg_isel_pre_de_d0);
hi_s32 hdmi_reg_isel_pre_de_d1_set(hi_u32 reg_isel_pre_de_d1);
hi_s32 hdmi_reg_isel_pre_de_d2_set(hi_u32 reg_isel_pre_de_d2);
hi_s32 hdmi_reg_rsel_pre_clk_set(hi_u32 reg_rsel_pre_clk);
hi_s32 hdmi_reg_rsel_pre_d0_set(hi_u32 reg_rsel_pre_d0);
hi_s32 hdmi_reg_rsel_pre_d1_set(hi_u32 reg_rsel_pre_d1);
hi_s32 hdmi_reg_rsel_pre_d2_set(hi_u32 reg_rsel_pre_d2);
hi_s32 hdmi_reg_rsel_pre_de_clk_set(hi_u32 reg_rsel_pre_de_clk);
hi_s32 hdmi_reg_rsel_pre_de_d0_set(hi_u32 reg_rsel_pre_de_d0);
hi_s32 hdmi_reg_rsel_pre_de_d1_set(hi_u32 reg_rsel_pre_de_d1);
hi_s32 hdmi_reg_rsel_pre_de_d2_set(hi_u32 reg_rsel_pre_de_d2);
hi_s32 hdmi_reg_rt_clk_set(hi_u32 reg_rt_clk);
hi_s32 hdmi_reg_rt_d0_set(hi_u32 reg_rt_d0);
hi_s32 hdmi_reg_rt_d1_set(hi_u32 reg_rt_d1);
hi_s32 hdmi_reg_rt_d2_set(hi_u32 reg_rt_d2);
hi_s32 hdmi_reg_txpll_divsel_loop_set(hi_u32 reg_txpll_divsel_loop);
hi_s32 hdmi_reg_txpll_icp_ictrl_set(hi_u32 reg_txpll_icp_ictrl);
hi_s32 hdmi_reg_gc_txpll_en_sscdiv_set(hi_u32 reg_gc_txpll_en_sscdiv);
hi_s32 hdmi_reg_gc_txpll_test_set(hi_u32 reg_txpll_test);
hi_s32 hdmi_reg_fb_set(hi_u32 reg_set_fb);
hi_s32 hdmi_reg_load_fb_set(hi_u32 reg_set_load_fb);
hi_s32 hdmi_reg_ssc_mode_fb_set(hi_u32 reg_ssc_mode_fb);
hi_s32 hdmi_reg_step_fb_set(hi_u32 reg_step_fb);
hi_u32 hdmi_reg_step_fb_get(hi_void);
hi_s32 hdmi_reg_span_fb_set(hi_u32 reg_span_fb);
hi_u32 hdmi_reg_span_fb_get(hi_void);
hi_u32 hdmi_reg_isel_main_clk_get(hi_void);
hi_u32 hdmi_reg_isel_main_d0_get(hi_void);
hi_s32 hdmi_reg_test_set(hi_u32 reg_test);
hi_u32 hdmi_reg_rsel_pre_d0_get(hi_void);
hi_u32 hdmi_reg_rsel_pre_clk_get(hi_void);

#endif /* __HDMI_REG_DPHY_H__ */


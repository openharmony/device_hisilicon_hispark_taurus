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
#ifndef __HDMI_REG_AUDIO_PATH_H__
#define __HDMI_REG_AUDIO_PATH_H__

#include "hi_type.h"

typedef union {
    struct {
        unsigned int aud_in_en     : 1;  /* [0] */
        unsigned int aud_mute_en   : 1;  /* [1] */
        unsigned int aud_layout    : 1;  /* [2] */
        unsigned int rsv_0         : 1;  /* [3] */
        unsigned int aud_i2s_en    : 4;  /* [7:4] */
        unsigned int aud_spdif_en  : 1;  /* [8] */
        unsigned int aud_src_en    : 1;  /* [9] */
        unsigned int aud_src_ctrl  : 1;  /* [10] */
        unsigned int rsv_1         : 1;  /* [11] */
        unsigned int aud_fifo0_map : 2;  /* [13:12] */
        unsigned int aud_fifo1_map : 2;  /* [15:14] */
        unsigned int aud_fifo2_map : 2;  /* [17:16] */
        unsigned int aud_fifo3_map : 2;  /* [19:18] */
        unsigned int rsv_2         : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} tx_audio_ctrl;

typedef union {
    struct {
        unsigned int i2s_hbra_on     : 1;  /* [0] */
        unsigned int i2s_1st_shift   : 1;  /* [1] */
        unsigned int i2s_ws_polarity : 1;  /* [2] */
        unsigned int i2s_justify     : 1;  /* [3] */
        unsigned int i2s_data_dir    : 1;  /* [4] */
        unsigned int i2s_vbit        : 1;  /* [5] */
        unsigned int rsv_3           : 2;  /* [7:6] */
        unsigned int i2s_length      : 4;  /* [11:8] */
        unsigned int i2s_ch_swap     : 4;  /* [15:12] */
        unsigned int rsv_4           : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} audio_i2s_ctrl;

typedef union {
    struct {
        unsigned int spdif_1ui_lock   : 1; /* [0] */
        unsigned int spdif_2ui_lock   : 1; /* [1] */
        unsigned int i2s_cbit_order   : 1; /* [2] */
        unsigned int spdif_fs_ovr_en  : 1; /* [3] */
        unsigned int spdif_err_thresh : 6; /* [9:4] */
        unsigned int spdif_size_sw    : 2; /* [11:10] */
        unsigned int spdif_1ui_max    : 8; /* [19:12] */
        unsigned int spdif_2ui_max    : 8; /* [27:20] */
        unsigned int rsv_5            : 4; /* [31:28] */
    } bits;
    unsigned int u32;
} audio_spdif_ctrl;

typedef union {
    struct {
        unsigned int chst_byte0_a              : 1; /* [0] */
        unsigned int chst_byte0_b              : 1; /* [1] */
        unsigned int chst_byte0_other          : 6; /* [7:2] */
        unsigned int chst_byte1                : 8; /* [15:8] */
        unsigned int chst_byte2                : 8; /* [23:16] */
        unsigned int chst_byte3_fs             : 4; /* [27:24] */
        unsigned int chst_byte3_clock_accuracy : 4; /* [31:28] */
    } bits;
    unsigned int u32;
} audio_chst_cfg0;

typedef union {
    struct {
        unsigned int chst_byte4_length : 4;  /* [3:0] */
        unsigned int chst_byte4_org_fs : 4;  /* [7:4] */
        unsigned int chst_byte5_6      : 16; /* [23:8] */
        unsigned int rsv_6             : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} audio_chst_cfg1;

typedef union {
    struct {
        unsigned int aud_inavailable  : 1; /* [0] */
        unsigned int aud_spdif_new_fs : 1; /* [1] */
        unsigned int rsv_7            : 2; /* [3:2] */
        unsigned int aud_length       : 4; /* [7:4] */
        unsigned int aud_spdif_fs     : 6; /* [13:8] */
        unsigned int rsv_8            : 2; /* [15:14] */
        unsigned int spdif_max_1ui_st : 8; /* [23:16] */
        unsigned int spdif_max_2ui_st : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} tx_audio_state;

typedef union {
    struct {
        unsigned int aud_fifo_test     : 5;  /* [4:0] */
        unsigned int rsv_9             : 3;  /* [7:5] */
        unsigned int aud_fifo_hbr_mask : 4;  /* [11:8] */
        unsigned int rsv_10            : 4;  /* [15:12] */
        unsigned int aud_fifo_ptr_diff : 6;  /* [21:16] */
        unsigned int rsv_11            : 10; /* [31:22] */
    } bits;
    unsigned int u32;
} audio_fifo_ctrl;

typedef union {
    struct {
        unsigned int acr_cts_req_en    : 1;  /* [0] */
        unsigned int acr_cts_hw_sw_sel : 1;  /* [1] */
        unsigned int acr_cts_gen_sel   : 1;  /* [2] */
        unsigned int acr_cts_flt_en    : 1;  /* [3] */
        unsigned int acr_use_sw_cts    : 1;  /* [4] */
        unsigned int acr_cts_ave_en    : 1;  /* [5] */
        unsigned int rsv_12            : 26; /* [31:6] */
    } bits;
    unsigned int u32;
} audio_acr_ctrl;

typedef union {
    struct {
        unsigned int acr_fm_val_sw    : 3; /* [2:0] */
        unsigned int acr_ave_max      : 5; /* [7:3] */
        unsigned int acr_cts_thre     : 8; /* [15:8] */
        unsigned int acr_cts_chg_thre : 8; /* [23:16] */
        unsigned int rsv_13           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} audio_acr_cfg;

typedef union {
    struct {
        unsigned int acr_n_val_sw : 20; /* [19:0] */
        unsigned int rsv_14       : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} acr_n_val_sw;

typedef union {
    struct {
        unsigned int acr_cts_val_sw : 20; /* [19:0] */
        unsigned int rsv_15         : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} acr_cts_val_sw;

typedef union {
    struct {
        unsigned int acr_cts_val_hw : 20; /* [19:0] */
        unsigned int rsv_16         : 12; /* [31:20] */
    } bits;
    unsigned int u32;
} acr_cts_val_hw;

typedef struct {
    volatile tx_audio_ctrl    audio_ctl;    /* 1000 */
    volatile audio_i2s_ctrl   i2s_ctl;      /* 1004 */
    volatile audio_spdif_ctrl spdif_ctl;    /* 1008 */
    volatile audio_chst_cfg0  chst_cfg0;    /* 100C */
    volatile audio_chst_cfg1  chst_cfg1;    /* 1010 */
    volatile tx_audio_state   aud_state;    /* 1014 */
    volatile audio_fifo_ctrl  fifo_ctl;     /* 1018 */
    unsigned int              reserved_0[9];
    volatile audio_acr_ctrl   acr_ctl;      /* 1040 */
    volatile audio_acr_cfg    acr_cfg;      /* 1044 */
    volatile acr_n_val_sw     acr_sw_n;     /* 1048 */
    volatile acr_cts_val_sw   acr_sw_cts;   /* 104C */
    volatile acr_cts_val_hw   acr_hw_cts;   /* 1050 */
} hdmi_reg_audio_path;

int hdmi_reg_audio_path_regs_init(hi_char *addr);
int hdmi_reg_audio_path_regs_deinit(hi_void);
int hdmi_reg_aud_in_en_set(unsigned int aud_in_en);
int hdmi_reg_aud_mute_en_set(unsigned int aud_mute_en);
int hdmi_reg_aud_layout_set(unsigned int aud_layout);
int hdmi_reg_aud_i2s_en_set(unsigned int aud_i2s_en);
int hdmi_reg_aud_spdif_en_set(unsigned int aud_spdif_en);
int hdmi_reg_i2s_hbra_on_set(unsigned int i2s_hbra_on);
int hdmi_reg_i2s_1st_shift_set(unsigned int i2s_1st_shift);
int hdmi_reg_i2s_ws_polarity_set(unsigned int i2s_ws_polarity);
int hdmi_reg_i2s_justify_set(unsigned int i2s_justify);
int hdmi_reg_i2s_data_dir_set(unsigned int i2s_data_dir);
int hdmi_reg_i2s_vbit_set(unsigned int i2s_vbit);
int hdmi_reg_i2s_length_set(unsigned int i2s_length);
int hdmi_reg_i2s_ch_swap_set(unsigned int i2s_ch_swap);
int hdmi_reg_chst_byte0_aset(unsigned int chst_byte0_a);
int hdmi_reg_chst_byte0_bset(unsigned int chst_byte0_b);
int hdmi_reg_chst_byte3_fs_set(unsigned int chst_byte3_fs);
int hdmi_reg_chst_byte3_clock_accuracy_set(unsigned int chst_byte3_clock_accuracy);
int hdmi_reg_chst_byte4_length_set(unsigned int chst_byte4_length);
int hdmi_reg_chst_byte4_org_fs_set(unsigned int chst_byte4_org_fs);
int hdmi_reg_aud_fifo_test_set(unsigned int aud_fifo_test);
int hdmi_reg_aud_fifo_hbr_mask_set(unsigned int aud_fifo_hbr_mask);
int hdmi_reg_acr_cts_hw_sw_sel_set(unsigned int acr_cts_hw_sw_sel);
int hdmi_reg_acr_n_val_sw_set(unsigned int acr_n_value);
hi_u32 hdmi_reg_aud_in_en_get(hi_void);
hi_u32 hdmi_reg_aud_mute_en_get(hi_void);
hi_u32 hdmi_reg_aud_layout_get(hi_void);
hi_u32 hdmi_reg_aud_i2s_en_get(hi_void);
hi_u32 hdmi_reg_aud_spdif_en_get(hi_void);
hi_u32 hdmi_reg_i2s_hbra_on_get(hi_void);
hi_u32 hdmi_reg_chst_byte3_fs_get(hi_void);
hi_u32 hdmi_reg_chst_byte4_length_get(hi_void);
hi_u32 hdmi_reg_chst_byte4_org_fs_get(hi_void);
hi_u32 hdmi_reg_aud_length_get(hi_void);
hi_u32 hdmi_reg_aud_spdif_fs_get(hi_void);
hi_u32 hdmi_reg_acr_cts_hw_sw_sel_get(hi_void);
hi_u32 hdmi_reg_acr_n_val_sw_get(hi_void);
hi_u32 hdmi_reg_acr_cts_val_sw_get(hi_void);
hi_u32 hdmi_reg_acr_cts_val_hw_get(hi_void);

#endif /* __HDMI_REG_AUDIO_PATH_H__ */


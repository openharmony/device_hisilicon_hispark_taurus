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
#ifndef __HHDMI_REG_TX_H__
#define __HHDMI_REG_TX_H__

#include "hi_type.h"

typedef union {
    struct {
        unsigned int tmds_pack_mode       : 2;  /* [1:0] */
        unsigned int reg_fifo_auto_rst_en : 1;  /* [2] */
        unsigned int reg_fifo_manu_rst    : 1;  /* [3] */
        unsigned int reg_clock_det_en     : 1;  /* [4] */
        unsigned int reg_ext_tmds_para    : 1;  /* [5] */
        unsigned int rsv_0                : 2;  /* [7:6] */
        unsigned int reg_fifo_delay_cnt   : 8;  /* [15:8] */
        unsigned int rsv_1                : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} tx_pack_fifo_ctrl;

typedef union {
    struct {
        unsigned int pclk2tclk_stable : 1;  /* [0] */
        unsigned int rsv_2            : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} tx_pack_fifo_st;

typedef union {
    struct {
        unsigned int reg_pclk_refer_cnt : 18; /* [17:0] */
        unsigned int rsv_3              : 14; /* [31:18] */
    } bits;
    unsigned int u32;
} pclk_refer_cnt;

typedef union {
    struct {
        unsigned int reg_tcnt_lower_threshold : 18; /* [17:0] */
        unsigned int rsv_4                    : 14; /* [31:18] */
    } bits;
    unsigned int u32;
} tclk_lower_threshold;

typedef union {
    struct {
        unsigned int reg_tcnt_upper_threshold : 18; /* [17:0] */
        unsigned int rsv_5                    : 14; /* [31:18] */
    } bits;
    unsigned int u32;
} tclk_upper_threshold;

typedef union {
    struct {
        unsigned int avi_pkt_hb0 : 8; /* [7:0] */
        unsigned int avi_pkt_hb1 : 8; /* [15:8] */
        unsigned int avi_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_6       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_pkt_header;

typedef union {
    struct {
        unsigned int avi_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int avi_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int avi_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int avi_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt0_low;

typedef union {
    struct {
        unsigned int avi_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int avi_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int avi_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_7            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt0_high;

typedef union {
    struct {
        unsigned int avi_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int avi_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int avi_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int avi_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt1_low;

typedef union {
    struct {
        unsigned int avi_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int avi_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int avi_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_8            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt1_high;

typedef union {
    struct {
        unsigned int avi_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int avi_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int avi_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int avi_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt2_low;

typedef union {
    struct {
        unsigned int avi_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int avi_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int avi_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_9            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt2_high;

typedef union {
    struct {
        unsigned int avi_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int avi_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int avi_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int avi_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt3_low;

typedef union {
    struct {
        unsigned int avi_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int avi_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int avi_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_10           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} avi_sub_pkt3_high;

typedef union {
    struct {
        unsigned int aif_pkt_hb0 : 8; /* [7:0] */
        unsigned int aif_pkt_hb1 : 8; /* [15:8] */
        unsigned int aif_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_11      : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_pkt_header;

typedef union {
    struct {
        unsigned int aif_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int aif_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int aif_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int aif_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt0_low;

typedef union {
    struct {
        unsigned int aif_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int aif_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int aif_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_12           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt0_high;

typedef union {
    struct {
        unsigned int aif_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int aif_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int aif_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int aif_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt1_low;

typedef union {
    struct {
        unsigned int aif_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int aif_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int aif_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_13           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt1_high;

typedef union {
    struct {
        unsigned int aif_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int aif_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int aif_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int aif_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt2_low;

typedef union {
    struct {
        unsigned int aif_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int aif_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int aif_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_14           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt2_high;

typedef union {
    struct {
        unsigned int aif_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int aif_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int aif_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int aif_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt3_low;

typedef union {
    struct {
        unsigned int aif_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int aif_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int aif_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_15           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} aif_sub_pkt3_high;

typedef union {
    struct {
        unsigned int spd_pkt_hb0 : 8; /* [7:0] */
        unsigned int spd_pkt_hb1 : 8; /* [15:8] */
        unsigned int spd_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_16      : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_pkt_header;

typedef union {
    struct {
        unsigned int spd_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int spd_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int spd_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int spd_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt0_low;

typedef union {
    struct {
        unsigned int spd_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int spd_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int spd_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_17           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt0_high;

typedef union {
    struct {
        unsigned int spd_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int spd_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int spd_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int spd_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt1_low;

typedef union {
    struct {
        unsigned int spd_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int spd_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int spd_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_18           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt1_high;

typedef union {
    struct {
        unsigned int spd_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int spd_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int spd_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int spd_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt2_low;

typedef union {
    struct {
        unsigned int spd_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int spd_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int spd_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_19           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt2_high;

typedef union {
    struct {
        unsigned int spd_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int spd_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int spd_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int spd_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt3_low;

typedef union {
    struct {
        unsigned int spd_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int spd_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int spd_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_20           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} spif_sub_pkt3_high;

typedef union {
    struct {
        unsigned int mpeg_pkt_hb0 : 8; /* [7:0] */
        unsigned int mpeg_pkt_hb1 : 8; /* [15:8] */
        unsigned int mpeg_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_21       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} speg_pkt_header;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int mpeg_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} speg_sub_pkt0_low;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_22            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} speg_sub_pkt0_high;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int mpeg_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} speg_sub_pkt1_low;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_23            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} mpeg_sub_pkt1_high;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int mpeg_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} mpeg_sub_pkt2_low;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_24            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} mpeg_sub_pkt2_high;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int mpeg_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} mpeg_sub_pkt3_low;

typedef union {
    struct {
        unsigned int mpeg_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int mpeg_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int mpeg_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_25            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} mpeg_sub_pkt3_high;

typedef union {
    struct {
        unsigned int gen_pkt_hb0 : 8; /* [7:0] */
        unsigned int gen_pkt_hb1 : 8; /* [15:8] */
        unsigned int gen_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_26      : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_pkt_header;

typedef union {
    struct {
        unsigned int gen_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int gen_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int gen_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int gen_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt0_low;

typedef union {
    struct {
        unsigned int gen_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int gen_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int gen_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_27           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt0_high;

typedef union {
    struct {
        unsigned int gen_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int gen_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int gen_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int gen_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt1_low;

typedef union {
    struct {
        unsigned int gen_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int gen_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int gen_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_28           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt1_high;

typedef union {
    struct {
        unsigned int gen_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int gen_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int gen_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int gen_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt2_low;

typedef union {
    struct {
        unsigned int gen_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int gen_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int gen_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_29           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt2_high;

typedef union {
    struct {
        unsigned int gen_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int gen_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int gen_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int gen_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt3_low;

typedef union {
    struct {
        unsigned int gen_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int gen_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int gen_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_30           : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen_sub_pkt3_high;

typedef union {
    struct {
        unsigned int gen2_pkt_hb0 : 8; /* [7:0] */
        unsigned int gen2_pkt_hb1 : 8; /* [15:8] */
        unsigned int gen2_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_31       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_pkt_header;

typedef union {
    struct {
        unsigned int gen2_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int gen2_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt0_low;

typedef union {
    struct {
        unsigned int gen2_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_32            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt0_high;

typedef union {
    struct {
        unsigned int gen2_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int gen2_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt1_low;

typedef union {
    struct {
        unsigned int gen2_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_33            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt1_high;

typedef union {
    struct {
        unsigned int gen2_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int gen2_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt2_low;

typedef union {
    struct {
        unsigned int gen2_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_34            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt2_high;

typedef union {
    struct {
        unsigned int gen2_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int gen2_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt3_low;

typedef union {
    struct {
        unsigned int gen2_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int gen2_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int gen2_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_35            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen2_sub_pkt3_high;

typedef union {
    struct {
        unsigned int gen3_pkt_hb0 : 8; /* [7:0] */
        unsigned int gen3_pkt_hb1 : 8; /* [15:8] */
        unsigned int gen3_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_36       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_pkt_header;

typedef union {
    struct {
        unsigned int gen3_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int gen3_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt0_low;

typedef union {
    struct {
        unsigned int gen3_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_37            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt0_high;

typedef union {
    struct {
        unsigned int gen3_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int gen3_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt1_low;

typedef union {
    struct {
        unsigned int gen3_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_38            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt1_high;

typedef union {
    struct {
        unsigned int gen3_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int gen3_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt2_low;

typedef union {
    struct {
        unsigned int gen3_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_39            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt2_high;

typedef union {
    struct {
        unsigned int gen3_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int gen3_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt3_low;

typedef union {
    struct {
        unsigned int gen3_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int gen3_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int gen3_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_40            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen3_sub_pkt3_high;

typedef union {
    struct {
        unsigned int gen4_pkt_hb0 : 8; /* [7:0] */
        unsigned int gen4_pkt_hb1 : 8; /* [15:8] */
        unsigned int gen4_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_41       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_pkt_header;

typedef union {
    struct {
        unsigned int gen4_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int gen4_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt0_low;

typedef union {
    struct {
        unsigned int gen4_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_42            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt0_high;

typedef union {
    struct {
        unsigned int gen4_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int gen4_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt1_low;

typedef union {
    struct {
        unsigned int gen4_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_43            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt1_high;

typedef union {
    struct {
        unsigned int gen4_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int gen4_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt2_low;

typedef union {
    struct {
        unsigned int gen4_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_44            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt2_high;

typedef union {
    struct {
        unsigned int gen4_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int gen4_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt3_low;

typedef union {
    struct {
        unsigned int gen4_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int gen4_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int gen4_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_45            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen4_sub_pkt3_high;

typedef union {
    struct {
        unsigned int gen5_pkt_hb0 : 8; /* [7:0] */
        unsigned int gen5_pkt_hb1 : 8; /* [15:8] */
        unsigned int gen5_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_46       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_pkt_header;

typedef union {
    struct {
        unsigned int gen5_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int gen5_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt0_low;

typedef union {
    struct {
        unsigned int gen5_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_47            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt0_high;

typedef union {
    struct {
        unsigned int gen5_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int gen5_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt1_low;

typedef union {
    struct {
        unsigned int gen5_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_48            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt1_high;

typedef union {
    struct {
        unsigned int gen5_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int gen5_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt2_low;

typedef union {
    struct {
        unsigned int gen5_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_49            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt2_high;

typedef union {
    struct {
        unsigned int gen5_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int gen5_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt3_low;

typedef union {
    struct {
        unsigned int gen5_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int gen5_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int gen5_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_50            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gen5_sub_pkt3_high;

typedef union {
    struct {
        unsigned int gamut_pkt_hb0 : 8; /* [7:0] */
        unsigned int gamut_pkt_hb1 : 8; /* [15:8] */
        unsigned int gamut_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_51        : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_pkt_header;

typedef union {
    struct {
        unsigned int gamut_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int gamut_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt0_low;

typedef union {
    struct {
        unsigned int gamut_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_52             : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt0_high;

typedef union {
    struct {
        unsigned int gamut_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int gamut_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt1_low;

typedef union {
    struct {
        unsigned int gamut_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_53             : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt1_high;

typedef union {
    struct {
        unsigned int gamut_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int gamut_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt2_low;

typedef union {
    struct {
        unsigned int gamut_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_54             : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt2_high;

typedef union {
    struct {
        unsigned int gamut_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int gamut_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt3_low;

typedef union {
    struct {
        unsigned int gamut_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int gamut_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int gamut_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_55             : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} gamut_sub_pkt3_high;

typedef union {
    struct {
        unsigned int vsif_pkt_hb0 : 8; /* [7:0] */
        unsigned int vsif_pkt_hb1 : 8; /* [15:8] */
        unsigned int vsif_pkt_hb2 : 8; /* [23:16] */
        unsigned int rsv_56       : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_pkt_header;

typedef union {
    struct {
        unsigned int vsif_sub_pkt0_pb0 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt0_pb1 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt0_pb2 : 8; /* [23:16] */
        unsigned int vsif_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt0_low;

typedef union {
    struct {
        unsigned int vsif_sub_pkt0_pb4 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt0_pb5 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt0_pb6 : 8; /* [23:16] */
        unsigned int rsv_57            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt0_high;

typedef union {
    struct {
        unsigned int vsif_sub_pkt1_pb0 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt1_pb1 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt1_pb2 : 8; /* [23:16] */
        unsigned int vsif_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt1_low;

typedef union {
    struct {
        unsigned int vsif_sub_pkt1_pb4 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt1_pb5 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt1_pb6 : 8; /* [23:16] */
        unsigned int rsv_58            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt1_high;

typedef union {
    struct {
        unsigned int vsif_sub_pkt2_pb0 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt2_pb1 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt2_pb2 : 8; /* [23:16] */
        unsigned int vsif_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt2_low;

typedef union {
    struct {
        unsigned int vsif_sub_pkt2_pb4 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt2_pb5 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt2_pb6 : 8; /* [23:16] */
        unsigned int rsv_59            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt2_high;

typedef union {
    struct {
        unsigned int vsif_sub_pkt3_pb0 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt3_pb1 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt3_pb2 : 8; /* [23:16] */
        unsigned int vsif_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt3_low;

typedef union {
    struct {
        unsigned int vsif_sub_pkt3_pb4 : 8; /* [7:0] */
        unsigned int vsif_sub_pkt3_pb5 : 8; /* [15:8] */
        unsigned int vsif_sub_pkt3_pb6 : 8; /* [23:16] */
        unsigned int rsv_60            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} vsif_sub_pkt3_high;

typedef union {
    struct {
        unsigned int cea_avi_en     : 1;  /* [0] */
        unsigned int cea_avi_rpt_en : 1;  /* [1] */
        unsigned int rsv_61         : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_avi_cfg;

typedef union {
    struct {
        unsigned int cea_spf_en     : 1;  /* [0] */
        unsigned int cea_spf_rpt_en : 1;  /* [1] */
        unsigned int rsv_62         : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_spf_cfg;

typedef union {
    struct {
        unsigned int cea_aud_en     : 1;  /* [0] */
        unsigned int cea_aud_rpt_en : 1;  /* [1] */
        unsigned int rsv_63         : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_aud_cfg;

typedef union {
    struct {
        unsigned int cea_mpeg_en     : 1;  /* [0] */
        unsigned int cea_mpeg_rpt_en : 1;  /* [1] */
        unsigned int rsv_64          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_mpeg_cfg;

typedef union {
    struct {
        unsigned int cea_gen_en     : 1;  /* [0] */
        unsigned int cea_gen_rpt_en : 1;  /* [1] */
        unsigned int rsv_65         : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_gen_cfg;

typedef union {
    struct {
        unsigned int cea_cp_en      : 1;  /* [0] */
        unsigned int cea_cp_rpt_en  : 1;  /* [1] */
        unsigned int cea_cp_rpt_cnt : 8;  /* [2:7] */
        unsigned int rsv_66         : 22; /* [31:8] */
    } bits;
    unsigned int u32;
} cea_cp_cfg;

typedef union {
    struct {
        unsigned int cea_gen2_en     : 1;  /* [0] */
        unsigned int cea_gen2_rpt_en : 1;  /* [1] */
        unsigned int rsv_67          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_gen2_cfg;

typedef union {
    struct {
        unsigned int cea_gen3_en     : 1;  /* [0] */
        unsigned int cea_gen3_rpt_en : 1;  /* [1] */
        unsigned int rsv_68          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_gen3_cfg;

typedef union {
    struct {
        unsigned int cea_gen4_en     : 1;  /* [0] */
        unsigned int cea_gen4_rpt_en : 1;  /* [1] */
        unsigned int rsv_69          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_gen4_cfg;

typedef union {
    struct {
        unsigned int cea_gen5_en     : 1;  /* [0] */
        unsigned int cea_gen5_rpt_en : 1;  /* [1] */
        unsigned int rsv_70          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_gen5_cfg;

typedef union {
    struct {
        unsigned int cea_gamut_en     : 1;  /* [0] */
        unsigned int cea_gamut_rpt_en : 1;  /* [1] */
        unsigned int rsv_71           : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_gamut_cfg;

typedef union {
    struct {
        unsigned int cea_vsif_en     : 1;  /* [0] */
        unsigned int cea_vsif_rpt_en : 1;  /* [1] */
        unsigned int rsv_72          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cea_vsif_cfg;

typedef union {
    struct {
        unsigned int cea_avi_state   : 1;  /* [0] */
        unsigned int cea_aud_state   : 1;  /* [1] */
        unsigned int cea_cp_state    : 1;  /* [2] */
        unsigned int cea_gen_state   : 1;  /* [3] */
        unsigned int cea_gen2_state  : 1;  /* [4] */
        unsigned int cea_gen3_state  : 1;  /* [5] */
        unsigned int cea_gen4_state  : 1;  /* [6] */
        unsigned int cea_gen5_state  : 1;  /* [7] */
        unsigned int cea_spd_state   : 1;  /* [8] */
        unsigned int cea_mpeg_state  : 1;  /* [9] */
        unsigned int cea_gamut_state : 1;  /* [10] */
        unsigned int cea_vsif_state  : 1;  /* [11] */
        unsigned int rsv_73          : 20; /* [31:12] */
    } bits;
    unsigned int u32;
} cea_pktf_state;

typedef union {
    struct {
        unsigned int hdmi_mode           : 1;  /* [0] */
        unsigned int dc_pkt_en           : 1;  /* [1] */
        unsigned int null_pkt_en         : 1;  /* [2] */
        unsigned int null_pkt_en_vs_high : 1;  /* [3] */
        unsigned int intr_encryption     : 1;  /* [4] */
        unsigned int ovr_dc_pkt_en       : 1;  /* [5] */
        unsigned int priotity_ctl        : 1;  /* [6] */
        unsigned int pkt_bypass_mode     : 1;  /* [7] */
        unsigned int avmute_in_phase     : 1;  /* [8] */
        unsigned int hdmi_dvi_sel        : 1;  /* [9] */
        unsigned int eess_mode_en        : 1;  /* [10] */
        unsigned int rsv_74              : 21; /* [31:11] */
    } bits;
    unsigned int u32;
} cea_avmixer_config;

typedef union {
    struct {
        unsigned int cp_set_avmute : 1;  /* [0] */
        unsigned int cp_clr_avmute : 1;  /* [1] */
        unsigned int rsv_75        : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} cp_pkt_avmute;

typedef union {
    struct {
        unsigned int video_blank : 24; /* [23:0] */
        unsigned int rsv_76      : 8;  /* [31:24] */
    } bits;
    unsigned int u32;
} video_blank_cfg;

typedef union {
    struct {
        unsigned int reg_tbist_en         : 1;  /* [0] */
        unsigned int reg_tbist_syn_pol    : 2;  /* [2:1] */
        unsigned int reg_tbist_timing_sel : 6;  /* [8:3] */
        unsigned int reg_tbist_patt_sel   : 5;  /* [13:9] */
        unsigned int rsv_77               : 18; /* [31:14] */
    } bits;
    unsigned int u32;
} tmds_bist_ctrl;

typedef union {
    struct {
        unsigned int enc_hdmi2_on   : 1;  /* [0] */
        unsigned int enc_scr_on     : 1;  /* [1] */
        unsigned int enc_scr_md     : 1;  /* [2] */
        unsigned int enc_hdmi_val   : 1;  /* [3] */
        unsigned int enc_hdmi_ovr   : 1;  /* [4] */
        unsigned int enc_bypass     : 1;  /* [5] */
        unsigned int enc_ck_div_sel : 2;  /* [7:6] */
        unsigned int rsv_78         : 24; /* [31:8] */
    } bits;
    unsigned int u32;
} hdmi_enc_ctrl;

typedef union {
    struct {
        unsigned int enc_ck_sharp0 : 10; /* [9:0] */
        unsigned int enc_ck_sharp1 : 10; /* [19:10] */
        unsigned int enc_ck_sharp2 : 10; /* [29:20] */
        unsigned int rsv_79        : 2;  /* [31:30] */
    } bits;
    unsigned int u32;
} enc_ck_sharp;

typedef struct {
    volatile tx_pack_fifo_ctrl    pack_fifo_ctrl;   /* 1800 */
    volatile tx_pack_fifo_st      pack_fifo_status; /* 1804 */
    volatile pclk_refer_cnt       pclk_ref_cnt;     /* 1808 */
    volatile tclk_lower_threshold lower_threshold;  /* 180C */
    volatile tclk_upper_threshold upper_threshold;  /* 1810 */
    unsigned int                  reserved_0[1];    /* 1814 */
    volatile avi_pkt_header       avi_head;         /* 1818 */
    volatile avi_sub_pkt0_low     avi_pkt0l;        /* 181C */
    volatile avi_sub_pkt0_high    avi_pkt0h;        /* 1820 */
    volatile avi_sub_pkt1_low     avi_pkt1l;        /* 1824 */
    volatile avi_sub_pkt1_high    avi_pkt1h;        /* 1828 */
    volatile avi_sub_pkt1_low     avi_pkt2l;        /* 182C */
    volatile avi_sub_pkt2_high    avi_pkt2h;        /* 1830 */
    volatile avi_sub_pkt3_low     avi_pkt3l;        /* 1834 */
    volatile avi_sub_pkt3_high    avi_pkt3h;        /* 1838 */
    volatile aif_pkt_header       aif_pkt_head;     /* 183C */
    volatile aif_sub_pkt0_low     aif_pkt0l;        /* 1840 */
    volatile aif_sub_pkt0_high    aif_pkt0h;        /* 1844 */
    volatile aif_sub_pkt1_low     aif_pkt1l;        /* 1848 */
    volatile aif_sub_pkt1_high    aif_pkt1h;        /* 184C */
    volatile aif_sub_pkt2_low     aif_pkt2l;        /* 1850 */
    volatile aif_sub_pkt2_high    aif_pkt2h;        /* 1854 */
    volatile aif_sub_pkt3_low     aif_pkt3l;        /* 1858 */
    volatile aif_sub_pkt3_high    aif_pkt3h;        /* 185C */
    volatile spif_pkt_header      spif_pkt_head;    /* 1860 */
    volatile spif_sub_pkt0_low    spif_pkt0l;       /* 1864 */
    volatile spif_sub_pkt0_high   spif_pkt0h;       /* 1868 */
    volatile spif_sub_pkt1_low    spif_pkt1l;       /* 186C */
    volatile spif_sub_pkt1_high   spif_pkt1h;       /* 1870 */
    volatile spif_sub_pkt2_low    spif_pkt2l;       /* 1874 */
    volatile spif_sub_pkt2_high   spif_pkt2h;       /* 1878 */
    volatile spif_sub_pkt3_low    spif_pkt3l;       /* 187C */
    volatile spif_sub_pkt3_high   spif_pkt3h;       /* 1880 */
    volatile speg_pkt_header      mpeg_pkt_head;    /* 1884 */
    volatile speg_sub_pkt0_low    mpeg_pkt0l;       /* 1888 */
    volatile speg_sub_pkt0_high   mpeg_pkt0h;       /* 188C */
    volatile speg_sub_pkt1_low    mpeg_pkt1l;       /* 1890 */
    volatile mpeg_sub_pkt1_high   mpeg_pkt1h;       /* 1894 */
    volatile mpeg_sub_pkt2_low    mpeg_pkt2l;       /* 1898 */
    volatile mpeg_sub_pkt2_high   mpeg_pkt2h;       /* 189C */
    volatile mpeg_sub_pkt3_low    mpeg_pkt3l;       /* 18A0 */
    volatile mpeg_sub_pkt3_high   mpeg_pkt3h;       /* 18A4 */
    volatile gen_pkt_header       gen_pkt_head;     /* 18A8 */
    volatile gen_sub_pkt0_low     gen_pkt0l;        /* 18AC */
    volatile gen_sub_pkt0_high    gen_pkt0h;        /* 18B0 */
    volatile gen_sub_pkt1_low     gen_pkt1l;        /* 18B4 */
    volatile gen_sub_pkt1_high    gen_pkt1h;        /* 18B8 */
    volatile gen_sub_pkt2_low     gen_pkt2l;        /* 18BC */
    volatile gen_sub_pkt2_high    gen_pkt2h;        /* 18C0 */
    volatile gen_sub_pkt3_low     gen_pkt3l;        /* 18C4 */
    volatile gen_sub_pkt3_high    gen_pkt3h;        /* 18C8 */
    volatile gen2_pkt_header      gen2_pkt_head;    /* 18CC */
    volatile gen2_sub_pkt0_low    gen2_pkt0l;       /* 18D0 */
    volatile gen2_sub_pkt0_high   gen2_pkt0h;       /* 18D4 */
    volatile gen2_sub_pkt1_low    gen2_pkt1l;       /* 18D8 */
    volatile gen2_sub_pkt1_high   gen2_pkt1h;       /* 18DC */
    volatile gen2_sub_pkt2_low    gen2_pkt2l;       /* 18E0 */
    volatile gen2_sub_pkt2_high   gen2_pkt2h;       /* 18E4 */
    volatile gen2_sub_pkt3_low    gen2_pkt3l;       /* 18E8 */
    volatile gen2_sub_pkt3_high   gen2_pkt3h;       /* 18EC */
    volatile gen3_pkt_header      gen3_pkt_head;    /* 18F0 */
    volatile gen3_sub_pkt0_low    gen3_pkt0l;       /* 18F4 */
    volatile gen3_sub_pkt0_high   gen3_pkt0h;       /* 18F8 */
    volatile gen3_sub_pkt1_low    gen3_pkt1l;       /* 18FC */
    volatile gen3_sub_pkt1_high   gen3_pkt1h;       /* 1900 */
    volatile gen3_sub_pkt2_low    gen3_pkt2l;       /* 1904 */
    volatile gen3_sub_pkt2_high   gen3_pkt2h;       /* 1908 */
    volatile gen3_sub_pkt3_low    gen3_pkt3l;       /* 190C */
    volatile gen3_sub_pkt3_high   gen3_pkt3h;       /* 1910 */
    volatile gen4_pkt_header      gen4_pkt_head;    /* 1914 */
    volatile gen4_sub_pkt0_low    gen4_pkt0l;       /* 1918 */
    volatile gen4_sub_pkt0_high   gen4_pkt0h;       /* 191C */
    volatile gen4_sub_pkt1_low    gen4_pkt1l;       /* 1920 */
    volatile gen4_sub_pkt1_high   gen4_pkt1h;       /* 1924 */
    volatile gen4_sub_pkt2_low    gen4_pkt2l;       /* 1928 */
    volatile gen4_sub_pkt2_high   gen4_pkt2h;       /* 192C */
    volatile gen4_sub_pkt3_low    gen4_pkt3l;       /* 1930 */
    volatile gen4_sub_pkt3_high   gen4_pkt3h;       /* 1934 */
    volatile gen5_pkt_header      gen5_pkt_head;    /* 1938 */
    volatile gen5_sub_pkt0_low    gen5_pkt0l;       /* 193C */
    volatile gen5_sub_pkt0_high   gen5_pkt0h;       /* 1940 */
    volatile gen5_sub_pkt1_low    gen5_pkt1l;       /* 1944 */
    volatile gen5_sub_pkt1_high   gen5_pkt1h;       /* 1948 */
    volatile gen5_sub_pkt2_low    gen5_pkt2l;       /* 194C */
    volatile gen5_sub_pkt2_high   gen5_pkt2h;       /* 1950 */
    volatile gen5_sub_pkt3_low    gen5_pkt3l;       /* 1954 */
    volatile gen5_sub_pkt3_high   gen5_pkt3h;       /* 1958 */
    volatile gamut_pkt_header     gamut_pkt_head;   /* 195C */
    volatile gamut_sub_pkt0_low   gamut_pkt0l;      /* 1960 */
    volatile gamut_sub_pkt0_high  gamut_pkt0h;      /* 1964 */
    volatile gamut_sub_pkt1_low   gamut_pkt1l;      /* 1968 */
    volatile gamut_sub_pkt1_high  gamut_pkt1h;      /* 196C */
    volatile gamut_sub_pkt2_low   gamut_pkt2l;      /* 1970 */
    volatile gamut_sub_pkt2_high  gamut_pkt2h;      /* 1974 */
    volatile gamut_sub_pkt3_low   gamut_pkt3l;      /* 1978 */
    volatile gamut_sub_pkt3_high  gamut_pkt3h;      /* 197C */
    volatile vsif_pkt_header      vsif_pkt_head;    /* 1980 */
    volatile vsif_sub_pkt0_low    vsif_pkt0l;       /* 1984 */
    volatile vsif_sub_pkt0_high   vsif_pkt0h;       /* 1988 */
    volatile vsif_sub_pkt1_low    vsif_pkt1l;       /* 198C */
    volatile vsif_sub_pkt1_high   vsif_pkt1h;       /* 1990 */
    volatile vsif_sub_pkt2_low    vsif_pkt2l;       /* 1994 */
    volatile vsif_sub_pkt2_high   vsif_pkt2h;       /* 1998 */
    volatile vsif_sub_pkt3_low    vsif_pkt3l;       /* 199C */
    volatile vsif_sub_pkt3_high   vsif_pkt3h;       /* 19A0 */
    volatile cea_avi_cfg          avi_cfg;          /* 19A4 */
    volatile cea_spf_cfg          spf_cfg;          /* 19A8 */
    volatile cea_aud_cfg          aud_cfg;          /* 19AC */
    volatile cea_mpeg_cfg         mpeg_cfg;         /* 19B0 */
    volatile cea_gen_cfg          gen_cfg;          /* 19B4 */
    volatile cea_cp_cfg           cp_cfg;           /* 19B8 */
    volatile cea_gen2_cfg         gen2_cfg;         /* 19BC */
    volatile cea_gen3_cfg         gen3_cfg;         /* 19C0 */
    volatile cea_gen4_cfg         gen4_cfg;         /* 19C4 */
    volatile cea_gen5_cfg         gen5_cfg;         /* 19C8 */
    volatile cea_gamut_cfg        gamut_cfg;        /* 19CC */
    volatile cea_vsif_cfg         vsif_cfg;         /* 19D0 */
    unsigned int                  reserved_1[3];    /* 19D4-19DC */
    volatile cea_pktf_state       pkt_stats;        /* 19E0 */
    unsigned int                  reserved_2[9];    /* 19E4-1A04 */
    volatile cea_avmixer_config   avmixer_cfg;      /* 1A08 */
    volatile cp_pkt_avmute        pkt_avmute;       /* 1A0C */
    volatile video_blank_cfg      vblank_cfg;       /* 1A10 */
    unsigned int                  reserved_3[16];   /* 1A14-1A50 */
    volatile tmds_bist_ctrl       bist_ctrl;        /* 1A54 */
    unsigned int                  reserved_4[2];    /* 1A58-1A5C */
    volatile hdmi_enc_ctrl        enc_ctrl;         /* 1A60 */
    volatile enc_ck_sharp         enc_sharp;        /* 1A64 */
} tx_hdmi_reg_regs_type;

int hdmi_reg_tx_hdmi_regs_init(hi_char *addr);
int hdmi_reg_tx_hdmi_regs_deinit(void);
int hdmi_reg_tmds_pack_mode_set(unsigned int tmds_pack_mode);
int hdmi_reg_avi_pkt_header_hb_set(unsigned int hb0, unsigned int hb1, unsigned int hb2);
int hdmi_reg_avi_pkt0_low_set(unsigned int avi_pkt0_pb0, unsigned int avi_pkt0_pb1,
    unsigned int avi_pkt0_pb2, unsigned int avi_pkt0_pb3);
int hdmi_reg_avi_pkt0_high_set(unsigned int avi_pkt0_pb4, unsigned int avi_pkt0_pb5, unsigned int avi_pkt0_pb6);
int hdmi_reg_avi_pkt1_low_set(unsigned int avi_pkt1_pb0, unsigned int avi_pkt1_pb1,
    unsigned int avi_pkt1_pb2, unsigned int avi_pkt1_pb3);
int hdmi_reg_avi_pkt1_high_set(unsigned int avi_pkt1_pb4, unsigned int avi_pkt1_pb5, unsigned int avi_pkt1_pb6);
int hdmi_reg_avi_pkt2_low_set(unsigned int avi_pkt2_pb0, unsigned int avi_pkt2_pb1,
    unsigned int avi_pkt2_pb2, unsigned int avi_pkt2_pb3);
int hdmi_reg_avi_pkt2_high_set(unsigned int avi_pkt2_pb4, unsigned int avi_pkt2_pb5, unsigned int avi_pkt2_pb6);
int hdmi_reg_avi_pkt3_low_set(unsigned int avi_pkt3_pb0, unsigned int avi_pkt3_pb1,
    unsigned int avi_pkt3_pb2, unsigned int avi_pkt3_pb3);
int hdmi_reg_avi_pkt3_high_set(unsigned int avi_pkt3_pb4, unsigned int avi_pkt3_pb5, unsigned int avi_pkt3_pb6);
int hdmi_reg_avi_pkt_header_hb_get(avi_pkt_header *pkt_header);
int hdmi_reg_avi_pkt0_low_get(avi_sub_pkt0_low *sub_pkt0_low);
int hdmi_reg_avi_pkt0_high_get(avi_sub_pkt0_high *sub_pkt0_high);
int hdmi_reg_avi_pkt1_low_get(avi_sub_pkt1_low *sub_pkt1_low);
int hdmi_reg_avi_pkt1_high_get(avi_sub_pkt1_high *sub_pkt1_high);
int hdmi_reg_avi_pkt2_low_get(avi_sub_pkt2_low *sub_pkt2_low);
void hdmi_reg_avi_pkt2_high_get(avi_sub_pkt2_high *sub_pkt2_high);
void hdmi_reg_avi_pkt3_low_get(avi_sub_pkt3_low *sub_pkt3_low);
void hdmi_reg_avi_pkt3_high_get(avi_sub_pkt3_high *sub_pkt3_high);
int hdmi_reg_audio_pkt_header_set(unsigned int hb0, unsigned int hb1, unsigned int hb2);
int hdmi_reg_audio_pkt0_low_set(unsigned int avi_pkt0_pb0, unsigned int avi_pkt0_pb1,
    unsigned int avi_pkt0_pb2, unsigned int avi_pkt0_pb3);
int hdmi_reg_audio_pkt0_high_set(unsigned int avi_pkt0_pb4, unsigned int avi_pkt0_pb5,
    unsigned int avi_pkt0_pb6);
int hdmi_reg_audio_pkt1_low_set(unsigned int avi_pkt1_pb0, unsigned int avi_pkt1_pb1,
    unsigned int avi_pkt1_pb2, unsigned int avi_pkt1_pb3);
int hdmi_reg_audio_pkt1_high_set(unsigned int avi_pkt1_pb4, unsigned int avi_pkt1_pb5, unsigned int avi_pkt1_pb6);
int hdmi_reg_audio_pkt2_low_set(unsigned int avi_pkt2_pb0, unsigned int avi_pkt2_pb1,
    unsigned int avi_pkt2_pb2, unsigned int avi_pkt2_pb3);
int hdmi_reg_audio_pkt2_high_set(unsigned int avi_pkt2_pb4, unsigned int avi_pkt2_pb5,
    unsigned int avi_pkt2_pb6);
int hdmi_reg_audio_pkt3_low_set(unsigned int avi_pkt3_pb0, unsigned int avi_pkt3_pb1,
    unsigned int avi_pkt3_pb2, unsigned int avi_pkt3_pb3);
int hdmi_reg_audio_pkt3_high_set(unsigned int avi_pkt3_pb4, unsigned int avi_pkt3_pb5, unsigned int avi_pkt3_pb6);
int hdmi_reg_aif_pkt_header_get(aif_pkt_header *pkt_header);
int hdmi_reg_aif_pkt0_low_get(aif_sub_pkt0_low *sub_pkt0_low);
int hdmi_reg_aif_pkt0_high_get(aif_sub_pkt0_high *sub_pkt0_high);
int hdmi_reg_aif_pkt1_low_get(aif_sub_pkt1_low *sub_pkt1_low);
int hdmi_reg_aif_pkt1_high_get(aif_sub_pkt1_high *sub_pkt1_high);
int hdmi_reg_aif_pkt2_low_get(aif_sub_pkt2_low *sub_pkt2_low);
int hdmi_reg_aif_pkt2_high_get(aif_sub_pkt2_high *sub_pkt2_high);
int hdmi_reg_aif_pkt3_low_get(aif_sub_pkt3_low *sub_pkt3_low);
int hdmi_reg_aif_pkt3_high_get(aif_sub_pkt3_high *sub_pkt3_high);
int hdmi_reg_gen_pkt_header_set(unsigned int hb0, unsigned int hb1, unsigned int hb2);
int hdmi_reg_gen_pkt0_low_set(unsigned int gen_pkt0_pb0, unsigned int gen_pkt0_pb1,
    unsigned int gen_pkt0_pb2, unsigned int gen_pkt0_pb3);
int hdmi_reg_gen_pkt0_high_set(unsigned int gen_pkt0_pb4, unsigned int gen_pkt0_pb5, unsigned int gen_pkt0_pb6);
int hdmi_reg_gen_pkt1_low_set(unsigned int gen_pkt1_pb0, unsigned int gen_pkt1_pb1,
    unsigned int gen_pkt1_pb2, unsigned int gen_pkt1_pb3);
int hdmi_reg_gen_pkt1_high_set(unsigned int gen_pkt1_pb4, unsigned int gen_pkt1_pb5, unsigned int gen_pkt1_pb6);
int hdmi_reg_gen_pkt2_low_set(unsigned int gen_pkt2_pb0, unsigned int gen_pkt2_pb1,
    unsigned int gen_pkt2_pb2, unsigned int gen_pkt2_pb3);
int hdmi_reg_gen_pkt2_high_set(unsigned int gen_pkt2_pb4, unsigned int gen_pkt2_pb5, unsigned int gen_pkt2_pb6);
int hdmi_reg_gen_pkt3_low_set(unsigned int gen_pkt3_pb0, unsigned int gen_pkt3_pb1,
    unsigned int gen_pkt3_pb2, unsigned int gen_pkt3_pb3);
int hdmi_reg_gen_pkt3_high_set(unsigned int gen_pkt3_pb4, unsigned int gen_pkt3_pb5, unsigned int gen_pkt3_pb6);
int hdmi_reg_gen_pkt_header_get(gen_pkt_header *pkt_header);
int hdmi_reg_gen_pkt0_low_get(gen_sub_pkt0_low *sub_pkt0_low);
int hdmi_reg_gen_pkt0_high_get(gen_sub_pkt0_high *sub_pkt0_high);
int hdmi_reg_gen_pkt1_low_get(gen_sub_pkt1_low *sub_pkt1_low);
int hdmi_reg_gen_pkt1_high_get(gen_sub_pkt1_high *sub_pkt1_high);
int hdmi_reg_gen_pkt2_low_get(gen_sub_pkt2_low *sub_pkt2_low);
int hdmi_reg_gen_pkt2_high_get(gen_sub_pkt2_high *sub_pkt2_high);
int hdmi_reg_gen_pkt3_low_get(gen_sub_pkt3_low *sub_pkt3_low);
int hdmi_reg_gen_pkt3_high_get(gen_sub_pkt3_high *sub_pkt3_high);
int hdmi_reg_gamut_pkt_header_get(gamut_pkt_header *pkt_header);
int hdmi_reg_gamut_pkt0_low_get(gamut_sub_pkt0_low *sub_pkt0_low);
int hdmi_reg_gamut_pkt0_high_get(gamut_sub_pkt0_high *sub_pkt0_high);
int hdmi_reg_gamut_pkt1_low_get(gamut_sub_pkt1_low *sub_pkt1_low);
int hdmi_reg_gamut_pkt1_high_get(gamut_sub_pkt1_high *sub_pkt1_high);
int hdmi_reg_gamut_pkt2_low_get(gamut_sub_pkt2_low *sub_pkt2_low);
int hdmi_reg_gamut_pkt2_high_get(gamut_sub_pkt2_high *sub_pkt2_high);
int hdmi_reg_gamut_pkt3_low_get(gamut_sub_pkt3_low *sub_pkt3_low);
int hdmi_reg_gamut_pkt3_high_get(gamut_sub_pkt3_high *sub_pkt3_high);
int hdmi_reg_vsif_header_set(unsigned int hb0, unsigned int hb1, unsigned int hb2);
int hdmi_reg_vsif_pkt0_low_set(unsigned int vsif_pkt0_pb0, unsigned int vsif_pkt0_pb1,
    unsigned int vsif_pkt0_pb2, unsigned int vsif_pkt0_pb3);
int hdmi_reg_vsif_pkt0_high_set(unsigned int vsif_pkt0_pb4, unsigned int vsif_pkt0_pb5,
    unsigned int vsif_pkt0_pb6);
int hdmi_reg_vsif_pkt1_low_set(unsigned int vsif_pkt1_pb0, unsigned int vsif_pkt1_pb1,
    unsigned int vsif_pkt1_pb2, unsigned int vsif_pkt1_pb3);
int hdmi_reg_vsif_pkt1_high_set(unsigned int vsif_pkt1_pb4, unsigned int vsif_pkt1_pb5,
    unsigned int vsif_pkt1_pb6);
int hdmi_reg_vsif_pkt2_low_set(unsigned int vsif_pkt2_pb0, unsigned int vsif_pkt2_pb1,
    unsigned int vsif_pkt2_pb2, unsigned int vsif_pkt2_pb3);
int hdmi_reg_vsif_pkt2_high_set(unsigned int vsif_pkt2_pb4, unsigned int vsif_pkt2_pb5,
    unsigned int vsif_pkt2_pb6);
int hdmi_reg_vsif_pkt3_low_set(unsigned int vsif_pkt3_pb0, unsigned int vsif_pkt3_pb1,
    unsigned int vsif_pkt3_pb2, unsigned int vsif_pkt3_pb3);
int hdmi_reg_vsif_pkt3_high_set(unsigned int vsif_pkt3_pb4, unsigned int vsif_pkt3_pb5,
    unsigned int vsif_pkt3_pb6);
int hdmi_reg_vsif_pkt_header_get(vsif_pkt_header *pkt_header);
int hdmi_reg_vsif_pkt0_low_get(vsif_sub_pkt0_low *sub_pkt0_low);
int hdmi_reg_vsif_pkt0_high_get(vsif_sub_pkt0_high *sub_pkt0_high);
int hdmi_reg_vsif_pkt1_low_get(vsif_sub_pkt1_low *sub_pkt1_low);
int hdmi_reg_vsif_pkt1_high_get(vsif_sub_pkt1_high *sub_pkt1_high);
int hdmi_reg_vsif_pkt2_low_get(vsif_sub_pkt2_low *sub_pkt2_low);
int hdmi_reg_vsif_pkt2_high_get(vsif_sub_pkt2_high *sub_pkt2_high);
int hdmi_reg_vsif_pkt3_low_get(vsif_sub_pkt3_low *sub_pkt3_low);
int hdmi_reg_vsif_pkt3_high_get(vsif_sub_pkt3_high *sub_pkt3_high);
int hdmi_reg_cea_avi_en_set(unsigned int cea_avi_en);
int hdmi_reg_cea_avi_rpt_en_set(unsigned int cea_avi_rpt_en);
int hdmi_reg_cea_aud_en_set(unsigned int cea_aud_en);
int hdmi_reg_cea_aud_rpt_en_set(unsigned int cea_aud_rpt_en);
int hdmi_reg_cea_gen_en_set(unsigned int cea_gen_en);
int hdmi_reg_cea_gen_rpt_en_set(unsigned int cea_gen_rpt_en);
int hdmi_reg_cea_cp_en_set(unsigned int cea_cp_en);
int hdmi_reg_cea_cp_rpt_cnt_set(unsigned int cea_cp_rpt_cnt);
int hdmi_reg_cea_cp_rpt_en_set(unsigned int cea_cp_rpt_en);
int hdmi_reg_cea_gamut_en_set(unsigned int cea_gamut_en);
int hdmi_reg_cea_gamut_rpt_en_set(unsigned int cea_gamut_rpt_en);
int hdmi_reg_cea_vsif_en_set(unsigned int cea_vsif_en);
int hdmi_reg_cea_vsif_rpt_en_set(unsigned int cea_vsif_rpt_en);
int hdmi_reg_hdmi_mode_set(unsigned int hdmi_mode);
int hdmi_reg_dc_pkt_en_set(unsigned int dc_pkt_en);
int hdmi_reg_null_pkt_en_set(unsigned int null_pkt_en);
int hdmi_reg_hdmi_dvi_sel_set(unsigned int hdmi_dvi_sel);
int hdmi_reg_eess_mode_en_set(unsigned int eess_mode_en);
int hdmi_reg_cp_set_avmute_set(unsigned int cp_set_avmute);
int hdmi_reg_cp_clr_avmute_set(unsigned int cp_clr_avmute);
int hdmi_reg_enc_hdmi2_on_set(unsigned int enc_hdmi2_on);
int hdmi_reg_enc_scr_on_set(unsigned int enc_scr_on);
int hdmi_reg_enc_bypass_set(unsigned int enc_bypass);
hi_u32 hdmi_reg_tmds_pack_mode_get(hi_void);
hi_u32 hdmi_reg_pclk2tclk_stable_get(hi_void);
hi_u32 hdmi_reg_cea_avi_en_get(hi_void);
hi_u32 hdmi_reg_cea_aud_en_get(hi_void);
hi_u32 hdmi_reg_cea_gen_en_get(hi_void);
hi_u32 hdmi_reg_cea_cp_rpt_en_get(hi_void);
hi_u32 hdmi_reg_cea_gamut_en_get(hi_void);
hi_u32 hdmi_reg_cea_vsif_rpt_en_get(hi_void);
hi_u32 hdmi_reg_hdmi_mode_get(hi_void);
hi_u32 hdmi_reg_dc_pkt_en_get(hi_void);
hi_u32 hdmi_reg_cp_set_avmute_get(hi_void);
hi_u32 hdmi_reg_enc_hdmi2_on_get(hi_void);
hi_u32 hdmi_reg_enc_scr_on_get(hi_void);
hi_u32 hdmi_reg_null_pkt_en_get(hi_void);

#endif  /* __HHDMI_REG_TX_H__ */


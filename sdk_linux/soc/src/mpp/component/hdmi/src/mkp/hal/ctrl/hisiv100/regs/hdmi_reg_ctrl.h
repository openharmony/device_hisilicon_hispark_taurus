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
#ifndef __HDMI_REG_CTRL_H__
#define __HDMI_REG_CTRL_H__

#include "hi_type.h"

typedef union {
    struct {
        unsigned int tx_pwd_srst_req    : 1;  /* [0] */
        unsigned int tx_sys_srst_req    : 1;  /* [1] */
        unsigned int tx_vid_srst_req    : 1;  /* [2] */
        unsigned int tx_hdmi_srst_req   : 1;  /* [3] */
        unsigned int tx_hdcp1x_srst_req : 1;  /* [4] */
        unsigned int tx_phy_srst_req    : 1;  /* [5] */
        unsigned int tx_aud_srst_req    : 1;  /* [6] */
        unsigned int tx_acr_srst_req    : 1;  /* [7] */
        unsigned int tx_afifo_srst_req  : 1;  /* [8] */
        unsigned int tx_hdcp2x_srst_req : 1;  /* [9] */
        unsigned int tx_mcu_srst_req    : 1;  /* [10] */
        unsigned int rsv_0              : 21; /* [31:11] */
    } bits;
    unsigned int u32;
} tx_pwd_rst_ctrl;

typedef union {
    struct {
        unsigned int scdc_ddcm_abort      : 1;  /* [0] */
        unsigned int scdc_access_en       : 1;  /* [1] */
        unsigned int scdc_auto_reply      : 1;  /* [2] */
        unsigned int scdc_auto_poll       : 1;  /* [3] */
        unsigned int scdc_auto_reply_stop : 1;  /* [4] */
        unsigned int scdc_poll_sel        : 1;  /* [5] */
        unsigned int scdc_hdcp_det_en     : 1;  /* [6] */
        unsigned int scdc_stall_req       : 1;  /* [7] */
        unsigned int rsv_1                : 24; /* [31:8] */
    } bits;
    unsigned int u32;
} scdc_fsm_ctrl;

typedef union {
    struct {
        unsigned int scdc_poll_timer : 22; /* [21:0] */
        unsigned int rsv_2           : 10; /* [31:22] */
    } bits;
    unsigned int u32;
} scdc_poll_timerl;

typedef union {
    struct {
        unsigned int scdc_fsm_state    : 4;  /* [3:0] */
        unsigned int scdc_rreq_state   : 4;  /* [7:4] */
        unsigned int scdc_active       : 1;  /* [8] */
        unsigned int scdc_in_prog      : 1;  /* [9] */
        unsigned int scdc_rreq_in_prog : 1;  /* [10] */
        unsigned int rsv_3             : 21; /* [31:11] */
    } bits;
    unsigned int u32;
} scdc_fsm_state;

typedef union {
    struct {
        unsigned int scdc_flag_byte0 : 8;  /* [7:0] */
        unsigned int scdc_flag_byte1 : 8;  /* [15:8] */
        unsigned int rsv_4           : 16; /* [31:16] */
    } bits;
    unsigned int u32;
} scdc_flag_byte;

typedef union {
    struct {
        unsigned int pwd_fifo_data_out : 8;  /* [7:0] */
        unsigned int rsv_5             : 24; /* [31:8] */
    } bits;
    unsigned int u32;
} pwd_fifo_rdata;

typedef union {
    struct {
        unsigned int pwd_fifo_data_in : 8;  /* [7:0] */
        unsigned int rsv_6            : 24; /* [31:8] */
    } bits;
    unsigned int u32;
} pwd_fifo_wdata;

typedef union {
    struct {
        unsigned int pwd_fifo_data_cnt : 5;  /* [4:0] */
        unsigned int rsv_7             : 3;  /* [7:5] */
        unsigned int pwd_data_out_cnt  : 10; /* [17:8] */
        unsigned int rsv_8             : 14; /* [31:18] */
    } bits;
    unsigned int u32;
} pwd_data_cnt;

typedef union {
    struct {
        unsigned int pwd_slave_addr   : 8; /* [7:0] */
        unsigned int pwd_slave_offset : 8; /* [15:8] */
        unsigned int pwd_slave_seg    : 8; /* [23:16] */
        unsigned int rsv_9            : 8; /* [31:24] */
    } bits;
    unsigned int u32;
} pwd_slave_cfg;

typedef union {
    struct {
        unsigned int pwd_i2c_no_ack     : 1;  /* [0] */
        unsigned int pwd_i2c_bus_low    : 1;  /* [1] */
        unsigned int pwd_i2c_in_prog    : 1;  /* [2] */
        unsigned int pwd_fifo_wr_in_use : 1;  /* [3] */
        unsigned int pwd_fifo_rd_in_use : 1;  /* [4] */
        unsigned int pwd_fifo_empty     : 1;  /* [5] */
        unsigned int pwd_fifo_half_full : 1;  /* [6] */
        unsigned int pwd_fifo_full      : 1;  /* [7] */
        unsigned int rsv_10             : 24; /* [31:8] */
    } bits;
    unsigned int u32;
} pwd_mst_state;

typedef union {
    struct {
        unsigned int pwd_mst_cmd : 4;  /* [3:0] */
        unsigned int rsv_11      : 28; /* [31:4] */
    } bits;
    unsigned int u32;
} pwd_mst_cmd;

typedef union {
    struct {
        unsigned int pwd_clr_bus_low : 1;  /* [0] */
        unsigned int pwd_clr_no_ack  : 1;  /* [1] */
        unsigned int rsv_12          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} pwd_mst_clr;

typedef union {
    struct {
        unsigned int cpu_ddc_force_req : 1;  /* [0] */
        unsigned int reg_auto_abort_en : 1;  /* [1] */
        unsigned int rsv_13            : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} ddc_mst_arb_ctrl;

typedef union {
    struct {
        unsigned int cpu_ddc_req : 1;  /* [0] */
        unsigned int rsv_14      : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} ddc_mst_arb_reql;

typedef union {
    struct {
        unsigned int cpu_ddc_req_ack : 1;  /* [0] */
        unsigned int rsv_15          : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} ddc_mst_arb_ack;

typedef union {
    struct {
        unsigned int ddc_arb_state : 9;  /* [8:0] */
        unsigned int rsv_16        : 23; /* [31:9] */
    } bits;
    unsigned int u32;
} ddc_mst_arb_state;

typedef union {
    struct {
        unsigned int tx_pwd_intr_state : 1;  /* [0] */
        unsigned int rsv_17            : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} tx_pwd_intr_state;

typedef union {
    struct {
        unsigned int tx_sys_intr_state  : 1;  /* [0] */
        unsigned int vidpath_intr_state : 1;  /* [1] */
        unsigned int audpath_intr_state : 1;  /* [2] */
        unsigned int txhdmi_intr_state  : 1;  /* [3] */
        unsigned int txhdcp_intr_state  : 1;  /* [4] */
        unsigned int hdcp2x_intr_state  : 1;  /* [5] */
        unsigned int rsv_18             : 26; /* [31:6] */
    } bits;
    unsigned int u32;
} pwd_sub_intr_state;

typedef union {
    struct {
        unsigned int tx_sys_intr_mask  : 1;  /* [0] */
        unsigned int vidpath_intr_mask : 1;  /* [1] */
        unsigned int audpath_intr_mask : 1;  /* [2] */
        unsigned int txhdmi_intr_mask  : 1;  /* [3] */
        unsigned int txhdcp_intr_mask  : 1;  /* [4] */
        unsigned int hdcp2x_intr_mask  : 1;  /* [5] */
        unsigned int rsv_19            : 26; /* [31:6] */
    } bits;
    unsigned int u32;
} pwd_sub_intr_mask;

typedef union {
    struct {
        unsigned int tx_sys_intr_state0 : 1;  /* [0] */
        unsigned int tx_sys_intr_state1 : 1;  /* [1] */
        unsigned int tx_sys_intr_state2 : 1;  /* [2] */
        unsigned int tx_sys_intr_state3 : 1;  /* [3] */
        unsigned int tx_sys_intr_state4 : 1;  /* [4] */
        unsigned int tx_sys_intr_state5 : 1;  /* [5] */
        unsigned int rsv_20             : 26; /* [31:6] */
    } bits;
    unsigned int u32;
} txsys_intr_state;

typedef union {
    struct {
        unsigned int tx_sys_intr_mask0 : 1;  /* [0] */
        unsigned int tx_sys_intr_mask1 : 1;  /* [1] */
        unsigned int tx_sys_intr_mask2 : 1;  /* [2] */
        unsigned int tx_sys_intr_mask3 : 1;  /* [3] */
        unsigned int tx_sys_intr_mask4 : 1;  /* [4] */
        unsigned int tx_sys_intr_mask5 : 1;  /* [5] */
        unsigned int rsv_21            : 26; /* [31:6] */
    } bits;
    unsigned int u32;
} txsys_intr_mask;

typedef union {
    struct {
        unsigned int vidpath_intr_state0 : 1;  /* [0] */
        unsigned int vidpath_intr_state1 : 1;  /* [1] */
        unsigned int vidpath_intr_state2 : 1;  /* [2] */
        unsigned int vidpath_intr_state3 : 1;  /* [3] */
        unsigned int vidpath_intr_state4 : 1;  /* [4] */
        unsigned int vidpath_intr_state5 : 1;  /* [5] */
        unsigned int vidpath_intr_state6 : 1;  /* [6] */
        unsigned int rsv_22              : 25; /* [31:7] */
    } bits;
    unsigned int u32;
} video_path_intr_state;

typedef union {
    struct {
        unsigned int vidpath_intr_mask0 : 1;  /* [0] */
        unsigned int vidpath_intr_mask1 : 1;  /* [1] */
        unsigned int vidpath_intr_mask2 : 1;  /* [2] */
        unsigned int vidpath_intr_mask3 : 1;  /* [3] */
        unsigned int vidpath_intr_mask4 : 1;  /* [4] */
        unsigned int vidpath_intr_mask5 : 1;  /* [5] */
        unsigned int vidpath_intr_mask6 : 1;  /* [6] */
        unsigned int rsv_23             : 25; /* [31:7] */
    } bits;
    unsigned int u32;
} video_path_intr_mask;

typedef union {
    struct {
        unsigned int audpath_intr_state0 : 1;  /* [0] */
        unsigned int audpath_intr_state1 : 1;  /* [1] */
        unsigned int audpath_intr_state2 : 1;  /* [2] */
        unsigned int audpath_intr_state3 : 1;  /* [3] */
        unsigned int audpath_intr_state4 : 1;  /* [4] */
        unsigned int audpath_intr_state5 : 1;  /* [5] */
        unsigned int audpath_intr_state6 : 1;  /* [6] */
        unsigned int audpath_intr_state7 : 1;  /* [7] */
        unsigned int audpath_intr_state8 : 1;  /* [8] */
        unsigned int rsv_24              : 23; /* [31:9] */
    } bits;
    unsigned int u32;
} audio_path_intr_state;

typedef union {
    struct {
        unsigned int audpath_intr_mask0 : 1;  /* [0] */
        unsigned int audpath_intr_mask1 : 1;  /* [1] */
        unsigned int audpath_intr_mask2 : 1;  /* [2] */
        unsigned int audpath_intr_mask3 : 1;  /* [3] */
        unsigned int audpath_intr_mask4 : 1;  /* [4] */
        unsigned int audpath_intr_mask5 : 1;  /* [5] */
        unsigned int audpath_intr_mask6 : 1;  /* [6] */
        unsigned int audpath_intr_mask7 : 1;  /* [7] */
        unsigned int audpath_intr_mask8 : 1;  /* [8] */
        unsigned int rsv_25             : 23; /* [31:9] */
    } bits;
    unsigned int u32;
} audio_path_intr_mask;

typedef union {
    struct {
        unsigned int txhdmi_intr_state0 : 1;  /* [0] */
        unsigned int rsv_26             : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} txhdmi_intr_state;

typedef union {
    struct {
        unsigned int txhdmi_intr_mask0 : 1;  /* [0] */
        unsigned int rsv_27            : 31; /* [31:1] */
    } bits;
    unsigned int u32;
} txhdmi_intr_mask;

typedef union {
    struct {
        unsigned int hdcp_intr_state0 : 1;  /* [0] */
        unsigned int hdcp_intr_state1 : 1;  /* [1] */
        unsigned int rsv_28           : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} hdcp_intr_state;

typedef union {
    struct {
        unsigned int hdcp_intr_mask0 : 1;  /* [0] */
        unsigned int hdcp_intr_mask1 : 1;  /* [1] */
        unsigned int rsv_29          : 30; /* [31:2] */
    } bits;
    unsigned int u32;
} hdcp_intr_mask;

typedef struct {
    unsigned int                   reserved_0[4];   /* 0-C */
    volatile tx_pwd_rst_ctrl       pwd_rst_ctrl;    /* 10 */
    volatile scdc_fsm_ctrl         fsm_ctrl;        /* 14 */
    volatile scdc_poll_timerl      poll_timer;      /* 18 */
    volatile scdc_fsm_state        fsm_state;       /* 1C */
    volatile scdc_flag_byte        flag;            /* 20 */
    unsigned int                   reserved_1[5];   /* 0-c */
    volatile pwd_fifo_rdata        rdata;           /* 38 */
    volatile pwd_fifo_wdata        wdata;           /* 3C */
    volatile pwd_data_cnt          data_cnt;        /* 40 */
    volatile pwd_slave_cfg         slave_cfg;       /* 44 */
    volatile pwd_mst_state         mst_state;       /* 48 */
    volatile pwd_mst_cmd           mst_cmd;         /* 4C */
    volatile pwd_mst_clr           mst_clr;         /* 50 */
    unsigned int                   reserved_2[4];   /* 54-60 */
    volatile ddc_mst_arb_ctrl      ddc_arb_ctrl;    /* 64 */
    volatile ddc_mst_arb_reql      ddc_arb_req;     /* 68 */
    volatile ddc_mst_arb_ack       ddc_arb_ack;     /* 6C */
    volatile ddc_mst_arb_state     ddc_arb_state;   /* 70 */
    unsigned int                   reserved_3[35];  /* 74-FC */
    volatile tx_pwd_intr_state     pwd_irq_state;   /* 100 */
    volatile pwd_sub_intr_state    sub_irq_state;   /* 104 */
    volatile pwd_sub_intr_mask     sub_irq_mask;    /* 108 */
    volatile txsys_intr_state      sys_irq_state;   /* 10C */
    volatile txsys_intr_mask       sys_irq_mask;    /* 110 */
    volatile video_path_intr_state video_irq_state; /* 114 */
    volatile video_path_intr_mask  video_irq_mask;  /* 118 */
    volatile audio_path_intr_state audio_irq_state; /* 11C */
    volatile audio_path_intr_mask  audio_irq_mask;  /* 120 */
    volatile txhdmi_intr_state     hdmi_irq_state;  /* 124 */
    volatile txhdmi_intr_mask      hdmi_irq_mask;   /* 128 */
    volatile hdcp_intr_state       hdcp_irq_state;  /* 12C */
    volatile hdcp_intr_mask        hdcp_irq_mask;   /* 130 */
} hdmi_reg_tx_ctrl;

int hdmi_reg_tx_ctrl_regs_init(char *addr);
int hdmi_reg_tx_ctrl_regs_deinit(void);
int hdmi_reg_tx_afifo_srst_req_set(unsigned int tx_afifo_srst_req);
int hdmi_reg_tx_acr_srst_req_set(unsigned int tx_acr_srst_req);
int hdmi_reg_tx_aud_srst_req_set(unsigned int tx_aud_srst_req);
int hdmi_reg_tx_hdmi_srst_req_set(unsigned int tx_hdmi_srst_req);
int hdmi_reg_tx_pwd_srst_req_set(unsigned int tx_pwd_srst_req);
int hdmi_reg_pwd_fifo_data_in_set(unsigned int pwd_fifo_data_in);
int hdmi_reg_pwd_data_out_cnt_set(unsigned int pwd_data_out_cnt);
int hdmi_reg_pwd_slave_addr_set(unsigned int pwd_slave_addr);
int hdmi_reg_pwd_slave_offset_set(unsigned int pwd_slave_offset);
int hdmi_reg_pwd_slave_seg_set(unsigned int pwd_slave_seg);
int hdmi_reg_pwd_mst_cmd_set(unsigned int pwd_mst_cmd);
int hdmi_reg_cpu_ddc_req_set(unsigned int cpu_ddc_req);
int hdmi_reg_tx_mcu_srst_req_set(unsigned int tx_mcu_srst_req);
hi_u8 hdmi_reg_rdata_pwd_fifo_data_out_get(void);
hi_u32 hdmi_reg_pwd_fifo_data_out_get(void);
hi_u32 hdmi_reg_pwd_i2c_in_prog_get(void);
hi_u32 hdmi_reg_pwd_fifo_empty_get(void);
hi_u32 hdmi_reg_cpu_ddc_req_ack_get(void);

#endif  /* __HDMI_REG_CTRL_H__ */


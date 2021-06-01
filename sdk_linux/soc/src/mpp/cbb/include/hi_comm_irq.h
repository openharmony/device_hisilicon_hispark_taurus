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
#ifndef __HI_COMM_IRQ_H__
#define __HI_COMM_IRQ_H__
#include "hi_defines.h"

/* For IRQ */
#define HI_MAX_STATE_NUM       1

typedef enum {
    HI_INT_NORMAL = 0,
    HI_INT_VICAP  = 1,
    HI_INT_SCD    = 2,
    HI_INT_VDH    = 3,
    HI_INT_NNIE   = 4,
    HI_INT_AIO    = 5,
    HI_INT_BUTT,
} hi_int_type;

typedef struct {
    volatile hi_u32 state_num;                    /* Number of interrupt states */
    volatile hi_u32 raw_state[HI_MAX_STATE_NUM];  /* raw interrupt states */
    volatile hi_u32 mask_state[HI_MAX_STATE_NUM]; /* interrupt states after mask */
}hi_normal_int_state;

typedef struct {
    volatile unsigned int cap_state;
    volatile unsigned int pt_state[VI_MAX_DEV_NUM];
    volatile unsigned int ch_state[VI_MAX_PHY_PIPE_NUM];
    volatile unsigned int isp_fe_state[VI_MAX_ISP_FE_NUM];
}hi_vicap_int_state;

typedef struct {
    unsigned int aio_int_status_state;
    unsigned int aio_int_raw_state;
    unsigned int tx_raw_state[AO_DEV_MAX_NUM];
    unsigned int tx_stat_state[AO_DEV_MAX_NUM];
    unsigned int rx_raw_state[AI_DEV_MAX_NUM];
    unsigned int rx_stat_state[AI_DEV_MAX_NUM];
} hi_aio_int_state;

typedef struct {
    hi_int_type type;
    union {
        hi_normal_int_state normal;
        hi_vicap_int_state  vicap;
        hi_aio_int_state    aio;
    };
}hi_int_state_info;

typedef struct {
    hi_void *dev;
    hi_int_state_info int_info;
}hi_irq_arg;


typedef struct {
    unsigned int cap_reg;
    unsigned int pt_reg[VI_MAX_DEV_NUM];
    unsigned int ch_reg[VI_MAX_PHY_PIPE_NUM];
    unsigned int isp_fe_reg[VI_MAX_ISP_FE_NUM];
} hi_vicap_int_reg;

typedef struct {
    unsigned int aio_int_status_reg;
    unsigned int aio_int_raw_reg;
    unsigned int tx_clr_reg[AO_DEV_MAX_NUM];
    unsigned int tx_raw_reg[AO_DEV_MAX_NUM];
    unsigned int tx_stat_reg[AO_DEV_MAX_NUM];
    unsigned int rx_clr_reg[AI_DEV_MAX_NUM];
    unsigned int rx_raw_reg[AI_DEV_MAX_NUM];
    unsigned int rx_stat_reg[AI_DEV_MAX_NUM];
} hi_aio_int_reg;


typedef struct {
    unsigned int reg_num;
    unsigned int raw_int_reg[HI_MAX_STATE_NUM];   /* raw int state */
    unsigned int mask_int_reg[HI_MAX_STATE_NUM];  /* int state after mask */
    unsigned int clr_int_reg[HI_MAX_STATE_NUM];
} hi_normal_int_reg;

typedef struct {
    hi_int_type type;
    union {
        hi_normal_int_reg normal;
        hi_vicap_int_reg  vicap;
        hi_aio_int_reg    aio;
    };
} hi_int_reg_info;

#endif /* end of #ifndef __HI_COMM_IRQ_H__ */


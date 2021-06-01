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

#ifndef __DDRC_REG_H__
#define __DDRC_REG_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MDDRC_BASE_ADDR 0x20115000

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int mem_mode : 1;  /* [0] */
        unsigned int mem_comb : 2;  /* [2..1] */
        unsigned int reserved : 29; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_ddr_mode;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int apg_gt_en : 1;     /* [0] */
        unsigned int muxcmd_gt_en : 1;  /* [1] */
        unsigned int detaddr_gt_en : 1; /* [2] */
        unsigned int reserved : 29;     /* [31..3] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_clk_cfg;

/* Define the union u_awaddr_srvlnc_start */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int awaddr_srvlnc_start : 32; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_awaddr_srvlnc_start;

/* Define the union u_awaddr_srvlnc_end */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int awaddr_srvlnc_end : 32; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} u_awaddr_srvlnc_end;

#define HIFB_RESERVED_RANGE 62
typedef struct {
    u_ddr_mode ddr_mode;
    u_clk_cfg clk_cfg;
    unsigned int reserved_1[HIFB_RESERVED_RANGE];
    u_awaddr_srvlnc_start awaddr_srvlnc_start;
    u_awaddr_srvlnc_end awaddr_srvlnc_end;
    unsigned int reserved_2[HIFB_RESERVED_RANGE];
    unsigned int awaddr_srvlnc_status;
} mddrc_regs;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DDRC_REG_H__ */

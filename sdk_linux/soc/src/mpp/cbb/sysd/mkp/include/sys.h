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

#ifndef __SYS_H__
#define __SYS_H__

#include "hi_type.h"
#include "hi_common.h"
#include "valg_plat.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    hi_s32 *ddr_data_buff;
    hi_s32 buff_len;
    hi_s32 value_count;
} ddr_occupy;

typedef struct {
    osal_semaphore_t sem;
    osal_atomic_t user_ref;
    hi_u32 state;
    hi_mpp_sys_config sys_cfg;
    hi_s32 time_zone;
} sys_context;

typedef struct {
    struct osal_list_head list;
    hi_char mmz_name[MAX_MMZ_NAME_LEN];
} sys_mem_node;

typedef struct {
    hi_char *mmz_name;
} sys_mod_chn_mmz;

typedef struct {
    hi_u32 max_dev_cnt;
    hi_u32 max_chn_cnt;

    sys_mod_chn_mmz *mmz_tbl;
} sys_mem_ctx;

typedef struct {
    hi_s32 set_value; /* customer value meaning: whether check is ok. */
    hi_u32 set_count; /* customer set count */

    hi_s32 connect_status; /* 0: not ok, 1: ok, other: reserve */
} sys_customer_check;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
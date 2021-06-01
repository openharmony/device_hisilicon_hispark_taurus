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

#ifndef __HI_MOD_PARAM__
#define __HI_MOD_PARAM__

#include "hi_type.h"
#include "hi_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hi_vgs_module_params_s {
    HI_U32 u32_max_vgs_job;
    HI_U32 u32_max_vgs_task;
    HI_U32 u32_max_vgs_node;
    HI_U32 au32_vgs_en[VGS_IP_NUM];
    HI_BOOL b_vgs_hdr_support;
    HI_BOOL b_vgs_exit_in_sys;
} vgs_module_params_s;

typedef struct hi_gdc_module_params_s {
    HI_U32 u32_max_gdc_job;
    HI_U32 u32_max_gdc_task;
    HI_U32 u32_max_gdc_node;
    HI_U32 au32_gdc_en[GDC_IP_NUM];
} gdc_module_params_s;

typedef struct hivdec_module_params_s {
    HI_U32 u32_vdec_max_chn_num;
    HI_BOOL b_vdec_hfr;
} vdec_module_params_s;

typedef struct hi_ive_module_params_s {
    HI_BOOL b_save_power_en;
    HI_U16  u16_ive_node_num;
    HI_U16  u16_rsv;
} ive_module_params_s;

typedef struct hi_svp_nnie_module_params_s {
    HI_BOOL b_save_power_en;
    HI_U16 u16_nnie_tsk_buf_num;
} svp_nnie_module_params_s;


typedef struct hi_h265e_module_params_s {
    HI_U32 u32_feature_enable;
} h265e_module_params_s;

typedef struct hi_venc_module_params_s {
    HI_U32 u32_venc_max_chn_num;
} venc_module_params_s;

typedef struct hi_vfmw_module_params_s {
    HI_S32 s32_vfmw_max_chn_num;
} vfmw_module_params_s;

typedef struct hi_acodec_module_params_s {
    HI_U32  u32_init_delay_timems;
}acodec_module_params_s;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HI_MOD_PARAM__ */

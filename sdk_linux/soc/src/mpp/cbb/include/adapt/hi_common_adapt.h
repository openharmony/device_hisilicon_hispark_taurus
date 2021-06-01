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

#ifndef __HI_COMMON_ADAPT_H__
#define __HI_COMMON_ADAPT_H__

#include "autoconf.h"
#include "hi_type.h"
#include "hi_defines.h"
#include "hi_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef struct {
    hi_char version[VERSION_NAME_MAXLEN];
} hi_mpp_version;

typedef hi_s32 hi_ai_chn;
typedef hi_s32 hi_ao_chn;
typedef hi_s32 hi_aenc_chn;
typedef hi_s32 hi_adec_chn;
typedef hi_s32 hi_audio_dev;
typedef hi_s32 hi_vi_dev;
typedef hi_s32 hi_vi_pipe;
typedef hi_s32 hi_vi_chn;
typedef hi_s32 hi_vi_stitch_grp;
typedef hi_s32 hi_vi_mcf_grp;
typedef hi_s32 hi_vo_dev;
typedef hi_s32 hi_vo_layer;
typedef hi_s32 hi_vo_chn;
typedef hi_s32 hi_vo_wbc;
typedef hi_s32 hi_graphic_layer;
typedef hi_s32 hi_venc_chn;
typedef hi_s32 hi_vdec_chn;
typedef hi_s32 hi_ive_handle;
typedef hi_s32 hi_cls_handle;
typedef hi_s32 hi_fd_chn;
typedef hi_s32 hi_md_chn;
typedef hi_s32 hi_isp_dev;
typedef hi_s32 hi_blk_dev;
typedef hi_s32 hi_sensor_id;
typedef hi_s32 hi_mipi_dev;
typedef hi_s32 hi_slave_dev;
typedef hi_s32 hi_svp_nnie_handle;
typedef hi_s32 hi_svp_dsp_handle;
typedef hi_s32 hi_svp_alg_chn;
typedef hi_s32 hi_vpss_grp;
typedef hi_s32 hi_vpss_grp_pipe;
typedef hi_s32 hi_vpss_chn;
typedef hi_s32 hi_avs_grp;
typedef hi_s32 hi_avs_pipe;
typedef hi_s32 hi_avs_chn;
typedef hi_s32 hi_mcf_grp;
typedef hi_s32 hi_mcf_pipe;
typedef hi_s32 hi_mcf_chn;

typedef MOD_ID_E hi_mod_id;

typedef struct {
    hi_mod_id    mod_id;
    hi_s32      dev_id;
    hi_s32      chn_id;
} hi_mpp_chn;

typedef PAYLOAD_TYPE_E hi_payload_type;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* __HI_COMMON_ADAPT_H__ */


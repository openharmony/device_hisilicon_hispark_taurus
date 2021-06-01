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
#ifndef __HDMI_HAL_MACHINE_H__
#define __HDMI_HAL_MACHINE_H__

#include "hdmi_product_define.h"

#define HDMI_MACH_MAX_STAMPE_NUM   6
#define HDMI_MACH_MAX_NAME_SIZE    15
#define HDMI_MACH_DEFUALT_INTERVAL 10

typedef hi_void (*mach_callback)(hi_void *data);

typedef struct {
    hi_char *name;
    hi_u64   interval;
    hi_void *data;
    mach_callback callback;
} hdmi_mach_ctrl;

typedef struct {
    hi_bool valid_id;
    hi_bool enable;
    hi_u64  enable_time;
    hi_u64  disable_time;
    hi_u64  last_time;
    hi_u32  run_cnt;
    hi_u32  stamp_idx;
    hi_u64  timestamp[HDMI_MACH_MAX_STAMPE_NUM];
    hi_char name[HDMI_MACH_MAX_NAME_SIZE];
} hdmi_mach_run;

typedef struct {
    hdmi_mach_run  mach_run;
    hdmi_mach_ctrl mach_ctrl;
} hdmi_mach_elem;

typedef struct {
    hdmi_mach_run  mach_run;
    hdmi_mach_ctrl mach_ctrl;
} hdmi_mach_elem_status;

typedef struct {
    hi_bool init;
    hi_u32  total;
} hdmi_mach_status;

hi_s32 hal_hdmi_mach_init(hi_void);

hi_s32 hal_hdmi_mach_deinit(hi_void);

hi_s32 hal_hdmi_mach_invoke(hi_void);

hi_s32 hal_hdmi_mach_register(const hdmi_mach_ctrl *mach_ctrl, hi_u32 *mach_id);

hi_s32 hal_hdmi_mach_unregister(hi_u32 mach_id);

hi_s32 hal_hdmi_mach_cfg_set(hi_u32 mach_id, const hdmi_mach_ctrl *mach_ctrl);

hi_s32 hal_hdmi_mach_cfg_get(hi_u32 mach_id, hdmi_mach_ctrl *mach_ctrl);

hi_s32 hal_hdmi_mach_start(hi_u32 mach_id);

hi_s32 hal_hdmi_mach_stop(hi_u32 mach_id);

hi_s32 hal_hdmi_mach_status_get(hdmi_mach_status *status);

hi_s32 hal_hdmi_mach_elem_status_get(hi_u32 mach_id, hdmi_mach_elem_status *status);

hi_u64 hal_hdmi_mach_ms_get(hi_void);

#endif /* __HDMI_HAL_MACHINE_H__ */


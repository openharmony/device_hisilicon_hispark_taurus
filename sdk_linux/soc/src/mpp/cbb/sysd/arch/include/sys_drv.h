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

#ifndef __SYS_DRV_H__
#define __SYS_DRV_H__

#include "hi_comm_sys_adapt.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/* end : segment compress parameter data definition. */
hi_s32 sys_drv_init(hi_void);
hi_s32 sys_drv_exit(hi_void);

hi_s32 sys_drv_drv_ioctrl(hi_mpp_chn *mpp_chn, hi_u32 func_id, hi_void *io_args);

hi_void sys_drv_vi_vpss_mode_init(hi_void);

hi_void sys_drv_set_priority(hi_vi_vpss_mode *vi_vpss_mode);

hi_void sysconfig_drv_set_vi_vpss_mode(const hi_s32 vi_vpss_mode);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __SYS_DRV_H__ */

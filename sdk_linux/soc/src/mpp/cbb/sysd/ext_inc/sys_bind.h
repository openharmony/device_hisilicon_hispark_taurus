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

#ifndef __SYS_BIND_H__
#define __SYS_BIND_H__

#include "hi_common.h"
#include "hi_osal.h"
#include "sys_ext.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

hi_void sys_bind_proc_show(struct osal_proc_dir_entry *entry);
hi_s32 sys_get_bind_by_src_with_lock(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_dest);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __SYS_BIND_H__ */
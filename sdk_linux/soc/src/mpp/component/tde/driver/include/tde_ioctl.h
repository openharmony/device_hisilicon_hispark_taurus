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

#ifndef __TDE_IOCTL_H__
#define __TDE_IOCTL_H__

#include "hi_type.h"
#include "tde_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

hi_s32 tde_init_module_k(hi_void);
hi_void tde_cleanup_module_k(hi_void);
long tde_ioctl(unsigned int cmd, unsigned long arg, hi_void *private_data);
int tde_open(hi_void *private_data);
int tde_release(hi_void *private_data);

#ifdef CONFIG_TDE_PM_ENABLE
int tde_pm_suspend(pm_basedev *pdev, pm_message_t state);
int tde_pm_resume(pm_basedev *pdev);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif


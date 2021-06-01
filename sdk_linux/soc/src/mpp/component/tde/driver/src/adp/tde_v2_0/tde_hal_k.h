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

#ifndef __TDE_HAL_K_H__
#define __TDE_HAL_K_H__

#include "hi_type.h"
#include "tde_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

volatile hi_u32 *tde_hal_get_base_vir_addr(hi_void);
tde_color_fmt tde_get_common_drv_color_fmt(hi_u32 count);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __SOURCE_MSP_DRV_TDE_HAL_K__ */

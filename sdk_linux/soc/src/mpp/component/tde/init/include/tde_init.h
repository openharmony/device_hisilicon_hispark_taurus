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

#ifndef __SOURCE_MSP_DRV_TDE_INIT__
#define __SOURCE_MSP_DRV_TDE_INIT__

#include "hi_type.h"
#include <linux/module.h>
#include <linux/interrupt.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

hi_void set_tde_irq(unsigned int temp_tde_irq);
hi_s32 tde_drv_mod_init(hi_void);
hi_void tde_drv_mod_exit(hi_void);
hi_void tde_hal_set_base_vir_addr(hi_u32 *temp_base_vir_addr);
hi_void tde_init_set_buf(hi_u32 tde_buf);
hi_void tde_set_resize_filter(hi_bool is_resize_filter);
hi_void tde_set_tde_tmp_buffer(hi_u32 tde_tmp_buf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __SOURCE_MSP_DRV_TDE_INIT__ */

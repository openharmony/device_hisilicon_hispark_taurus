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

#ifndef _HI_DRV_CPMPAT_H_
#define _HI_DRV_CPMPAT_H_

hi_s32 hi_drv_compat_init(hi_void);
hi_s32 hi_drv_compat_deinit(hi_void);
hi_s32 klad_load_hard_key(hi_u32 handle, hi_u32 ca_type, const hi_u8 *key, hi_u32 key_len);
hi_s32 klad_encrypt_key(hi_u32 keysel, hi_u32 target, hi_u8 *clear, hi_u8 *encrypt, hi_u32 key_len);

#endif


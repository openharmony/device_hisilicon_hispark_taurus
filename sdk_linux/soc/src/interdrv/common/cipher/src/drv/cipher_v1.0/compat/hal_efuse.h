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

#ifndef _HAL_EFUSE_H_
#define _HAL_EFUSE_H_

#include "hi_types.h"

#define hal_cipher_read_reg(addr,  result)    (*(result) = *(volatile unsigned int *)(hi_uintptr_t)(addr))
#define hal_cipher_write_reg(addr, result)    (*(volatile unsigned int *)(hi_uintptr_t)(addr) = (result))

#define hal_set_bit(src, bit)               (src |= (1 << (bit)))
#define hal_clear_bit(src, bit)             (src &= ~(1 << (bit)))

hi_s32 hal_efuse_write_key(hi_u32 *p_key, hi_u32 opt_id);
hi_s32 hal_efuse_otp_load_cipher_key(hi_u32 chn_id, hi_u32 opt_id);
hi_s32 hal_efuse_load_hash_key(hi_u32 opt_id);
hi_s32 hal_efuse_otp_init(hi_void);
hi_void hal_efuse_otp_set_reg_base(hi_u8 *reg_base);
hi_u8 *hal_efuse_otp_get_reg_base(hi_void);

#endif


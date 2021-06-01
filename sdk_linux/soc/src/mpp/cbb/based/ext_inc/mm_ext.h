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

#ifndef __MM_EXT_H__
#define __MM_EXT_H__

#include "hi_osal.h"
#include "hi_math.h"
#include "hi_common.h"
#include "osal_mmz.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static inline hi_bool read_user_linear_space_valid(hi_u8 *addr_start, hi_u32 len)
{
    hi_u8 check;
    hi_u8 *addr_end = HI_NULL;

    if (len == 0) {
        return HI_FALSE;
    }

    if (!osal_access_ok(OSAL_VERIFY_READ, addr_start, len)) {
        return HI_FALSE;
    }

    addr_end = addr_start + len - 1;
    if (osal_copy_from_user(&check, addr_end, 1)) {
        return HI_FALSE;
    }

    return HI_TRUE;
}

static inline hi_bool write_user_linear_space_valid(hi_u8 *addr_start, hi_u32 len)
{
    hi_u8 check = 0;
    hi_u8 *addr_end = HI_NULL;

    if (len == 0) {
        return HI_FALSE;
    }

    if (!osal_access_ok(OSAL_VERIFY_WRITE, addr_start, len)) {
        return HI_FALSE;
    }

    addr_end = addr_start + len - 1;
    if (osal_copy_to_user(addr_end, &check, 1)) {
        return HI_FALSE;
    }

    return HI_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __MM_EXT_H__ */

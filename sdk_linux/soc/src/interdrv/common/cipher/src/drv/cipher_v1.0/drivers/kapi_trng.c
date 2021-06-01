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

#include "drv_osal_lib.h"
#include "cryp_trng.h"

/* ****************************** API Code **************************** */
hi_s32 kapi_trng_get_random(hi_u32 *randnum, hi_u32 timeout)
{
    hi_s32 ret;

    hi_log_func_enter();

    hi_log_chk_param_return(randnum == HI_NULL);

    ret = cryp_trng_get_random(randnum, timeout);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    hi_log_func_exit();
    return HI_SUCCESS;
}

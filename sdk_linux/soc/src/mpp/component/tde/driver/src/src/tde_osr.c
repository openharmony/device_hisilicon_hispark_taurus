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

#include "tde_osr.h"
#ifndef HI_BUILD_IN_BOOT
#include "tde_osilist.h"
#include "hi_type.h"
#include "tde_ioctl.h"
#include "tde_handle.h"
#include "tde_hal.h"
#include "tde_init.h"

#define mkstr(exp)        #exp
#define mkmarcotostr(exp) mkstr(exp)

void tde_osr_deinit(void)
{
    tde_osi_list_term();
    tde_hal_release();
    return;
}

hi_s32 tde_drv_mod_init(hi_void)
{
#ifndef HI_MCE_SUPPORT
    hi_s32 ret = tde_init_module_k();
    if (ret != HI_SUCCESS) {
        return HI_FAILURE;
    }
#endif

    return HI_SUCCESS;
}

hi_void tde_drv_mod_exit(hi_void)
{
#ifndef HI_MCE_SUPPORT
    tde_cleanup_module_k();
#endif
}

#ifndef __RTOS__
#ifndef HI_ADVCA_FUNCTION_RELEASE
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION(TDE_VERSION);
MODULE_LICENSE("GPL");
#else
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");
MODULE_VERSION("");
MODULE_LICENSE("");
#endif
#endif /* __RTOS__ */

#endif

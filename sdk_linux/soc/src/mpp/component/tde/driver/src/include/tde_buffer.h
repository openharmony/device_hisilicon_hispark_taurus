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

#ifndef __TDE_BUFFER_H__
#define __TDE_BUFFER_H__

#include "tde_define.h"
#include "hi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static hi_u32 g_tde_phy_buff = 0;
static hi_u32 g_tde_buff_ref = 0;
static hi_u32 g_tde_tmp_buf = 0;

hi_void tde_set_tde_tmp_buffer(hi_u32 tde_tmp_buf)
{
    g_tde_tmp_buf = tde_tmp_buf;
}

#ifndef HI_BUILD_IN_BOOT
static osal_spinlock_t g_tde_buff_lock;
static unsigned long g_tde_buff_lock_flags;
#endif
static hi_u32 tde_alloc_physic_buff(hi_u32 cb_cr_offset)
{
    hi_u32 phy_addr;
    hi_u32 csc_buffer_size;

#ifdef CFG_HI_TDE_CSCTMPBUFFER_SIZE
    csc_buffer_size = CFG_HI_TDE_CSCTMPBUFFER_SIZE;
#elif defined(HI_BUILD_IN_BOOT)
    csc_buffer_size = 0;
#else
    csc_buffer_size = g_tde_tmp_buf;
#endif
    tde_spin_lock(&g_tde_buff_lock, g_tde_buff_lock_flags);
    if (g_tde_phy_buff == 0) {
        if ((cb_cr_offset * 3) > csc_buffer_size) { /* 3 * size */
            tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);
            return 0;
        }

        tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);

        tde_get_phyaddr_mmb("TDE_TEMP_BUFFER", csc_buffer_size, phy_addr);
        osal_printk("\n u32CscBufferSize is %x 0x%x\n", csc_buffer_size, phy_addr);

        if (phy_addr == 0) {
            return 0;
        }

        tde_spin_lock(&g_tde_buff_lock, g_tde_buff_lock_flags);
        if (g_tde_phy_buff == 0) {
            g_tde_phy_buff = phy_addr;
            g_tde_buff_ref = 0;
        } else {
            g_tde_buff_ref++;
            tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);
#ifndef HI_BUILD_IN_BOOT
            tde_free_mmb(phy_addr);
#endif
            return g_tde_phy_buff + cb_cr_offset;
        }
    }

    g_tde_buff_ref++;
    tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);

    return g_tde_phy_buff + cb_cr_offset;
}

static hi_void tde_free_physic_buff(hi_void)
{
    tde_spin_lock(&g_tde_buff_lock, g_tde_buff_lock_flags);
    if (g_tde_buff_ref == 0) {
        tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);
        return;
    }

    g_tde_buff_ref--;
    if (g_tde_buff_ref == 0) {
#ifndef HI_BUILD_IN_BOOT
        hi_u32 phy_buff = g_tde_phy_buff;
#endif
        g_tde_phy_buff = 0;
        tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);
#ifndef HI_BUILD_IN_BOOT
        osal_printk("FREE MMB 0x%x\n", phy_buff);
        tde_free_mmb(phy_buff);
#endif
        return;
    }
    tde_spin_unlock(&g_tde_buff_lock, g_tde_buff_lock_flags);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HI_HANDLE_MGR_H__ */

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

#include "tde_handle.h"
#ifdef HI_BUILD_IN_BOOT
#include "hi_go_common.h"
#endif
#include "tde_define.h"
#include "tde_handle.h"
#include "osal_list.h"

hi_handle_mgr *g_tde_handle_list = HI_NULL; /* Manager list of global handle */

static hi_s32 g_handle = 1;
#ifndef HI_BUILD_IN_BOOT
static osal_spinlock_t g_handle_lock;
#endif

hi_handle_mgr *tde_get_handle_list(hi_void)
{
    return g_tde_handle_list;
}

hi_bool tde_initial_handle(hi_void)
{
    if (g_tde_handle_list == HI_NULL) {
#ifndef HI_BUILD_IN_BOOT

        g_tde_handle_list = (hi_handle_mgr *)osal_kmalloc(sizeof(hi_handle_mgr), osal_gfp_kernel);

#else
        g_tde_handle_list = (hi_handle_mgr *)HI_GFX_LOGO_Malloc(sizeof(hi_handle_mgr), "tde handle");
#endif
        if (g_tde_handle_list == HI_NULL) {
            return HI_FALSE;
        }

        /* Initialize list head */
        OSAL_INIT_LIST_HEAD(&g_tde_handle_list->header);
#ifndef HI_BUILD_IN_BOOT
        osal_spin_lock_init(&g_tde_handle_list->lock);
        osal_spin_lock_init(&g_handle_lock);
#endif
    }

    g_handle = 1;

    return HI_TRUE;
}

hi_void tde_get_handle(hi_handle_mgr *res, hi_s32 *handle)
{
#ifndef HI_BUILD_IN_BOOT
    unsigned long lockflags;
    unsigned long handlockflags;
#endif
    if ((res == HI_NULL) || (handle == HI_NULL) || (g_tde_handle_list == HI_NULL)) {
        return;
    }
    tde_spin_lock(&g_handle_lock, handlockflags);
    /* Jump over unlawful handle */
    if (g_handle == TDE_MAX_HANDLE_VALUE) {
        g_handle = 1;
    }
    res->handle = g_handle;
    *handle = g_handle++;
    tde_spin_unlock(&g_handle_lock, handlockflags);
    tde_spin_lock(&g_tde_handle_list->lock, lockflags);
    osal_list_add_tail(&res->header, &g_tde_handle_list->header);
    tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
}

/* To accelerate the speed of find, find form back to front */
hi_bool tde_query_handle(hi_s32 handle, hi_handle_mgr **res)
{
#ifndef HI_BUILD_IN_BOOT
    unsigned long lockflags;
#endif
    hi_handle_mgr *hdl = HI_NULL;
    if (res == HI_NULL) {
        return HI_FAILURE;
    }
    if (g_tde_handle_list == HI_NULL) {
        return HI_FAILURE;
    }
    tde_spin_lock(&g_tde_handle_list->lock, lockflags);
    hdl = osal_list_entry(g_tde_handle_list->header.prev, hi_handle_mgr, header);
    if (hdl == HI_NULL) {
        tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
        return HI_FALSE;
    }
    while (hdl != g_tde_handle_list) {
        if (hdl->handle == handle) {
            *res = hdl;
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return HI_TRUE;
        }
        hdl = osal_list_entry(hdl->header.prev, hi_handle_mgr, header);
        if (hdl == HI_NULL) {
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return HI_FALSE;
        }
    }
    tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
    return HI_FALSE;
}

hi_bool tde_release_handle(hi_s32 handle)
{
#ifndef HI_BUILD_IN_BOOT
    unsigned long lockflags;
#endif
    hi_handle_mgr *hdl = HI_NULL;
    if (g_tde_handle_list == HI_NULL) {
        return HI_FAILURE;
    }
    tde_spin_lock(&g_tde_handle_list->lock, lockflags);
    hdl = osal_list_entry(g_tde_handle_list->header.next, hi_handle_mgr, header);
    if (hdl == HI_NULL) {
        tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
        return HI_FALSE;
    }
    while (hdl != g_tde_handle_list) {
        if (hdl->handle == handle) {
            osal_list_del_init(&hdl->header);
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return HI_TRUE;
        }
        hdl = osal_list_entry(hdl->header.next, hi_handle_mgr, header);
        if (hdl == HI_NULL) {
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return HI_FALSE;
        }
    }
    tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
    return HI_FALSE;
}

hi_void tde_destroy_handle(hi_void)
{
    /* Free head node, note: other nodes are all loaded, their resource  are responsibilited by its own module */
    if (g_tde_handle_list != HI_NULL) {
        osal_spin_lock_destroy(&g_tde_handle_list->lock);
        osal_spin_lock_destroy(&g_handle_lock);
#ifndef HI_BUILD_IN_BOOT
        osal_kfree(g_tde_handle_list);
#else
        osal_kfree((HI_CHAR *)g_tde_handle_list);
#endif

        g_tde_handle_list = HI_NULL;
    }

    return;
}

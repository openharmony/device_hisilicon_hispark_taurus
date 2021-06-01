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

#ifndef HI_BUILD_IN_BOOT
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
#include "tde_proc.h"
#include "tde_hal.h"
#endif
#include "securec.h"

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
typedef struct {
    hi_u32 cur_node;
    tde_hw_node tde_hw_node[TDE_MAX_PROC_NUM];
    hi_bool proc_enable;
} tde_procinfo;

tde_procinfo g_tde_proc_info = {
    .cur_node = 0,
    .proc_enable = HI_TRUE,
};

hi_void tdeproc_record_node(tde_hw_node *hw_node)
{
    if ((!g_tde_proc_info.proc_enable) || (hw_node == HI_NULL)) {
        return;
    }

    if (g_tde_proc_info.cur_node >= TDE_MAX_PROC_NUM) {
        return;
    }
    if (memcpy_s(&g_tde_proc_info.tde_hw_node[g_tde_proc_info.cur_node], sizeof(tde_hw_node), hw_node,
        sizeof(tde_hw_node)) != EOK) {
        tde_error("secure function failure\n");
        return;
    }
    g_tde_proc_info.cur_node++;
    g_tde_proc_info.cur_node = (g_tde_proc_info.cur_node) % TDE_MAX_PROC_NUM;
}

hi_void tdeproc_clear_node(hi_void)
{
    (hi_void)memset_s(&g_tde_proc_info.tde_hw_node[0], sizeof(g_tde_proc_info.tde_hw_node), 0,
        sizeof(g_tde_proc_info.tde_hw_node));
    g_tde_proc_info.cur_node = 0;
}

int tde_read_proc(osal_proc_entry_t *p)
{
    hi_u32 j;
    hi_u32 *cur = HI_NULL;
    tde_hw_node *hw_node = HI_NULL;

    if (p == HI_NULL) {
        return HI_FAILURE;
    }

    hw_node = g_tde_proc_info.tde_hw_node;
    if (hw_node == NULL) {
        return HI_FAILURE;
    }
    p = wprintinfo(p);

    for (j = 0; j < g_tde_proc_info.cur_node; j++) {
        cur = (hi_u32 *)&hw_node[j];
        tde_hal_node_print_info(p, cur);
    }

    return HI_SUCCESS;
}
#endif
#endif

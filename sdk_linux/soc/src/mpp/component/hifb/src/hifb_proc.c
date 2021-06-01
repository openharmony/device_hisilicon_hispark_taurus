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

#include "hifb_proc.h"
#include "proc_ext.h"
#include "securec.h"
#include "hifb_main.h"
#include "hifb_comm.h"

#define MAX_PROC_ENTRIES 16

static osal_proc_entry_t g_proc_items[MAX_PROC_ENTRIES];

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
hi_void hifb_proc_add_module(const hi_char *entry_name, hi_u32 size, hifb_proc_show show,
                             hifb_proc_write w, hi_void *data)
{
    hi_s32 i, ret;
    osal_proc_entry_t* proc_item = HI_NULL;
    hi_unused(size);

    for (i = 0; i < MAX_PROC_ENTRIES; i++) {
        if (!g_proc_items[i].proc_dir_entry) {
            break;
        }
    }
    if (i == MAX_PROC_ENTRIES) {
        osal_printk("hifb proc num full. \n");
        return;
    }

    proc_item = osal_create_proc_entry(entry_name, HI_NULL);
    if (proc_item == HI_NULL) {
        osal_printk("create proc err. \n");
        return;
    }

    proc_item->write = w;
    proc_item->read = show,
    proc_item->private = data;

    ret = memcpy_s(&g_proc_items[i], sizeof(osal_proc_entry_t), proc_item, sizeof(osal_proc_entry_t));
    hifb_unequal_eok_return_void(ret);
}

hi_void hifb_proc_remove_module(const char *entry_name)
{
    hi_s32 i;

    for (i = 0; i < MAX_PROC_ENTRIES; i++) {
        if (!strcmp(g_proc_items[i].name, entry_name)) {
            break;
        }
    }

    if (i == MAX_PROC_ENTRIES) {
        return;
    }

    osal_remove_proc_entry(entry_name, HI_NULL);
    (hi_void)memset_s(&g_proc_items[i], sizeof(osal_proc_entry_t), 0, sizeof(osal_proc_entry_t));

    return;
}

hi_void hifb_proc_remove_all_module(hi_void)
{
    hi_s32 i;

    for (i = 0; i < MAX_PROC_ENTRIES; i++) {
        if (g_proc_items[i].proc_dir_entry == HI_NULL) {
            continue;
        }

        osal_remove_proc_entry(g_proc_items[i].name, HI_NULL);
        (hi_void)memset_s(&g_proc_items[i], sizeof(osal_proc_entry_t), 0, sizeof(osal_proc_entry_t));
    }
}

hi_void hifb_proc_init(hi_void)
{
    /* hifb proc:Uniformly hang under umap, not separately under graphic */
    (hi_void)memset_s(g_proc_items, sizeof(g_proc_items), 0, sizeof(g_proc_items));
}

#endif

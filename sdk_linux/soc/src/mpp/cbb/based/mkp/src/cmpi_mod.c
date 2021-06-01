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


#include "hi_osal.h"
#include "hi_type.h"
#include "hi_common.h"
#include "hi_debug.h"
#include "mod_ext.h"
#include "proc_ext.h"
#include "dev_ext.h"

static hi_u32 g_log_buf_len = 0;

struct osal_list_head g_mod_list;

hi_s32 g_proc_enable = 1;

hi_char *cmpi_get_module_name(hi_mod_id mod_id)
{
    umap_module *tmp = HI_NULL;

    osal_list_for_each_entry(tmp, &g_mod_list, list) {
        if (tmp->mod_id == mod_id) {
            return tmp->mod_name;
        }
    }

    return NULL;
}

umap_module *cmpi_get_module_by_id(hi_mod_id mod_id)
{
    umap_module *tmp = HI_NULL;

    osal_list_for_each_entry(tmp, &g_mod_list, list) {
        if (tmp->mod_id == mod_id) {
            return tmp;
        }
    }

    return NULL;
}

hi_void *cmpi_get_module_func_by_id(hi_mod_id mod_id)
{
    umap_module *tmp = NULL;
    hi_bool find = HI_FALSE;

    osal_list_for_each_entry(tmp, &g_mod_list, list) {
        if (tmp->mod_id == mod_id) {
            find = HI_TRUE;
            break;
        }
    }

    return (find == HI_TRUE) ? tmp->export_funcs : NULL;
}

/* just notify all modules to stop working.
 * after received notice "MOD_NOTICE_STOP", modules will
 * finish the task which is running and then stand by.
 * NOTE! returning from this call do __NOT__ mean
 * the module already stopped! you should invoke query_state
 * to check if the module is really stopped!
 */
hi_void cmpi_stop_modules(hi_void)
{
    umap_module *tmp = HI_NULL;

    osal_list_for_each_entry_reverse(tmp, &g_mod_list, list) {
        /* is the module registered */
        if (!tmp->pfn_notify) {
            continue;
        }

        tmp->pfn_notify(MOD_NOTICE_STOP);
    }
}

/* if all registered modules are free, return 0, otherwise return -1. */
hi_s32 cmpi_query_modules(hi_void)
{
    umap_module *tmp = HI_NULL;

    mod_state state;

    osal_list_for_each_entry_reverse(tmp, &g_mod_list, list) {
        /* is the module registered */
        if (!tmp->pfn_query_state) {
            continue;
        }

        state = MOD_STATE_BUSY;

        tmp->pfn_query_state(&state);

        if ((state == MOD_STATE_FREE) || (state == MOD_STATE_BYPASS)) {
            continue;
        }

        HI_TRACE(HI_DBG_INFO, HI_ID_CMPI,
                 "MOD [%s] is busy!\n", tmp->mod_name);
        return -1;
    }

    return 0;
}

/* caller must guarantee that this function can _not_
 * be invoked consecutively more than one time without calling
 * cmpi_init_modules.
 */
hi_void cmpi_exit_modules(hi_void)
{
    umap_module *tmp = HI_NULL;

    osal_list_for_each_entry_reverse(tmp, &g_mod_list, list) {
        /* is the module registered */
        if (!tmp->inited) {
            HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "MOD[%s] already exited!\n",
                     tmp->mod_name);
            continue;
        }

        tmp->pfn_exit();
        tmp->inited = HI_FALSE;

        HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI,
                 "MOD[%s] exit OK!\n", tmp->mod_name);
    }
}

/* this function has similar requirement than cmpi_exit_modules.  */
hi_s32 cmpi_init_modules(hi_void)
{
    umap_module *tmp = HI_NULL;

    osal_list_for_each_entry(tmp, &g_mod_list, list) {
        /* already been inited */
        if (tmp->inited) {
            continue;
        }

        if (tmp->pfn_init(tmp->data) < 0) {
            HI_TRACE(HI_DBG_ERR, HI_ID_CMPI,
                     "MOD[%s] pfn_init failed!\n", tmp->mod_name);

            goto fail;
        }

        tmp->inited = HI_TRUE;
    }

    return 0;

fail:

    /* release modules inited before. */
    osal_list_for_each_entry_reverse(tmp, &tmp->list, list) {
        if (!tmp->inited) {
            continue;
        }

        tmp->pfn_exit();

        tmp->inited = HI_FALSE;
    }

    return -1;
}

hi_s32 cmpi_register_module(umap_module *module)
{
    umap_module *tmp = HI_NULL;
    hi_s32 ret;

    if (module == HI_NULL ||
        module->pfn_init == HI_NULL ||
        module->pfn_exit == HI_NULL) {
        HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "null ptr!\n");
        return -1;
    }

    HI_ASSERT(module->mod_id < MAX_MPP_MODULES);

    /* check if module already registered */
    osal_list_for_each_entry(tmp, &g_mod_list, list) {
        if (tmp->mod_id == module->mod_id) {
            HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "MOD[%s] already registered!\n",
                     module->mod_name);
            return -1;
        }
    }

    /* warn: pfn_init can't get mod_name */
    ret = module->pfn_init(module->data);
    if (ret < 0) {
        HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "MOD[%s] pfn_init failed!\n",
                 module->mod_name);
        return -1;
    }

    module->inited = HI_TRUE;

    osal_list_add_tail(&module->list, &g_mod_list);

    return ret;
}

hi_void cmpi_unregister_module(hi_mod_id mod_id)
{
    umap_module *tmp = HI_NULL;
    umap_module *_tmp = HI_NULL;

    HI_ASSERT(mod_id < MAX_MPP_MODULES);

    osal_list_for_each_entry_safe(tmp, _tmp, &g_mod_list, list) {
        if (tmp->mod_id == mod_id) {
            if (tmp->inited) {
                tmp->pfn_exit();
            }

            osal_list_del(&tmp->list);

            HI_TRACE(HI_DBG_DEBUG,
                     HI_ID_CMPI, "MOD[%s] unregister OK!\n", tmp->mod_name);
            return;
        }
    }

    HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "MOD[%d] already unregister!\n", mod_id);
}

int comm_init(void)
{
    OSAL_INIT_LIST_HEAD(&g_mod_list);

    if (cmpi_log_init(g_log_buf_len)) {
        HI_TRACE(HI_DBG_ERR, HI_ID_CMPI, "log_init failed!\n");
        goto OUT;
    }

    HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI, "load hi35xx_base ... OK!\n");
    return HI_SUCCESS;

OUT:
    return HI_FAILURE;
}

void comm_exit(void)
{
    cmpi_log_exit();
    HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI, "Unload hi35xx_base ... OK!\n");
    return;
}


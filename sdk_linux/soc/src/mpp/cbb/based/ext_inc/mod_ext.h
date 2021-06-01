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
#ifndef __MOD_EXT_H__
#define __MOD_EXT_H__

#include "hi_osal.h"
#include "hi_type.h"
#include "hi_common_adapt.h"

#define MAX_MPP_MODULES HI_ID_BUTT

#define VERSION_MAGIC   20191120

#define MAX_MOD_NAME    16

typedef enum {
    MOD_NOTICE_STOP = 0x11,
} mod_notice_id;

typedef enum {
    MOD_STATE_FREE = 0x11,
    MOD_STATE_BUSY = 0X22,
    MOD_STATE_BYPASS = 0x33,
} mod_state;

typedef hi_s32 fn_mod_init(hi_void *);
typedef hi_void fn_mod_exit(hi_void);
typedef hi_void fn_mod_notify(mod_notice_id notice_id);
typedef hi_void fn_mod_query_state(mod_state *state);
typedef hi_u32 fn_mod_ver_checker(hi_void);

typedef struct {
    struct osal_list_head list;

    hi_char mod_name[MAX_MOD_NAME];
    hi_mod_id mod_id;

    fn_mod_init *pfn_init;
    fn_mod_exit *pfn_exit;
    fn_mod_query_state *pfn_query_state;
    fn_mod_notify *pfn_notify;
    fn_mod_ver_checker *pfn_ver_checker;

    hi_bool inited;

    hi_void *export_funcs;
    hi_void *data;

    hi_char *version;
} umap_module;

extern hi_char *cmpi_get_module_name(hi_mod_id mod_id);
extern umap_module *cmpi_get_module_by_id(hi_mod_id mod_id);
extern hi_void *cmpi_get_module_func_by_id(hi_mod_id mod_id);

extern hi_void cmpi_stop_modules(hi_void);
extern hi_s32 cmpi_query_modules(hi_void);
extern hi_s32 cmpi_init_modules(hi_void);
extern hi_void cmpi_exit_modules(hi_void);
extern hi_s32 cmpi_register_module(umap_module *modules);
extern hi_void cmpi_unregister_module(hi_mod_id mod_id);

#define func_entry(type, id) ((type *)cmpi_get_module_func_by_id(id))
#define check_func_entry(id) (cmpi_get_module_func_by_id(id) != NULL)
#define func_entry_null(id) (!check_func_entry(id))

#endif /*  __MOD_EXT_H__ */


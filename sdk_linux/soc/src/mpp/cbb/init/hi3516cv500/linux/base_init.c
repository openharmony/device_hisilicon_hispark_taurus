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
#include <linux/module.h>

#include "mod_ext.h"

extern int HI_LOG(hi_s32 level, hi_mod_id mod_id, const char *fmt, ...);
EXPORT_SYMBOL(HI_LOG);

/*************************MOD********************/
EXPORT_SYMBOL(cmpi_get_module_name);
EXPORT_SYMBOL(cmpi_get_module_by_id);
EXPORT_SYMBOL(cmpi_get_module_func_by_id);
EXPORT_SYMBOL(cmpi_stop_modules);
EXPORT_SYMBOL(cmpi_query_modules);
EXPORT_SYMBOL(cmpi_exit_modules);
EXPORT_SYMBOL(cmpi_init_modules);
EXPORT_SYMBOL(cmpi_register_module);
EXPORT_SYMBOL(cmpi_unregister_module);

/******* create node  /proc/sys/dev/debug/proc_message_enable ************/
extern hi_s32 g_proc_enable;
EXPORT_SYMBOL(g_proc_enable);

static struct ctl_table comm_eproc_table[] = {
    {
        .procname       = "proc_message_enable",
        .data           = &g_proc_enable,
        .maxlen         = sizeof(g_proc_enable),
        .mode           = 0644, /* 0644 Node permission */
        .proc_handler   = proc_dointvec
    },
    {}
};

static struct ctl_table comm_dir_table[] = {
    {
        .procname       = "debug",
        .mode           = 0555, /* 0555 Node permission */
        .child          = comm_eproc_table
    },
    {}
};

static struct ctl_table comm_parent_tbl[] = {
    {
        .procname       = "dev",
        .mode           = 0555, /* 0555 Node permission */
        .child          = comm_dir_table
    },
    {}
};

static struct ctl_table_header *comm_eproc_tbl_head;

int __init comm_init_proc_ctrl(void)
{
    comm_eproc_tbl_head = register_sysctl_table(comm_parent_tbl);
    if (comm_eproc_tbl_head == HI_NULL)
        return -ENOMEM;
    return 0;
}

void comm_exit_proc_ctrl(void)
{
    unregister_sysctl_table(comm_eproc_tbl_head);
}

extern int comm_init(void);
extern void comm_exit(void);

static int __init base_mod_init(void)
{
    comm_init();

    /* init proc switch */
    if (comm_init_proc_ctrl() != HI_SUCCESS) {
        comm_exit();
        return -1;
    }

    return 0;
}
static void __exit base_mod_exit(void)
{
    comm_exit_proc_ctrl();
    comm_exit();
}

module_init(base_mod_init);
module_exit(base_mod_exit);

MODULE_LICENSE("GPL");


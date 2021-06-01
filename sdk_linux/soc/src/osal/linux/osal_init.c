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
#include <linux/kernel.h>
#include "hi_osal.h"
#include "osal.h"

static int __init osal_init(void)
{
    if (mem_check_module_param() == -1) {
        return -1;
    }

    osal_device_init();
    osal_proc_init();
    himedia_init();
    media_mem_init();
    osal_trace("hi_osal %s init success!\n", HI_OSAL_VERSION);
    return 0;
}

static void __exit osal_exit(void)
{
    himedia_exit();
    media_mem_exit();
    osal_proc_exit();
    osal_trace("hi_osal v1.0 exit!\n");
}

module_init(osal_init);
module_exit(osal_exit);

MODULE_AUTHOR("Hisilicon");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

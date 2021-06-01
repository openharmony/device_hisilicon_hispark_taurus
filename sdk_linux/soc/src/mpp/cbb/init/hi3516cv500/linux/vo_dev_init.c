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

extern int vou_dev_module_init(void);
extern void vou_dev_module_exit(void);


static int __init vou_dev_mod_init(void)
{
    vou_dev_module_init();
    return 0;
}
static void __exit vou_dev_mod_exit(void)
{
    vou_dev_module_exit();
}

module_init(vou_dev_mod_init);
module_exit(vou_dev_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon");
MODULE_DESCRIPTION("Hisilicon VO Driver Init");
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
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>

#include "hi_common.h"
#include "hi_osal.h"


extern int sys_do_mod_init(void);
extern void sys_do_mod_exit(void);

extern void  *g_reg_crg_base_va;
EXPORT_SYMBOL(g_reg_crg_base_va);
#ifndef CONFIG_USER_SPACE
extern void  *g_reg_sys_base_va;
EXPORT_SYMBOL(g_reg_sys_base_va);
extern void  *g_reg_ddr0_base_va;
EXPORT_SYMBOL(g_reg_ddr0_base_va);
extern void  *g_reg_misc_base_va;
EXPORT_SYMBOL(g_reg_misc_base_va);
extern void  *g_reg_otp_base_va;
EXPORT_SYMBOL(g_reg_otp_base_va);
#endif

static int hi35xx_sys_probe(struct platform_device *pdev)
{
    hi_unused(pdev);

    if (sys_do_mod_init() != HI_SUCCESS) {
        return HI_FAILURE;
    }
    return 0;
}

static int hi35xx_sys_remove(struct platform_device *pdev)
{
    hi_unused(pdev);

    sys_do_mod_exit();
    return 0;
}

static const struct of_device_id hi35xx_sys_match[] = {
    { .compatible = "hisilicon,hisi-sys" },
    {},
};
MODULE_DEVICE_TABLE(of, hi35xx_sys_match);

static struct platform_driver hi35xx_sys_driver = {
    .probe = hi35xx_sys_probe,
    .remove = hi35xx_sys_remove,
    .driver = {
        .name = "hi35xx_sys",
        .of_match_table = hi35xx_sys_match,
    },
};

osal_module_platform_driver(hi35xx_sys_driver);

MODULE_LICENSE("GPL");

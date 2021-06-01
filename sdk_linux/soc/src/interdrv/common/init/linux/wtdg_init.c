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
#include "hi_type.h"
#include "watchdog.h"

#define OSDRV_MODULE_VERSION_STRING "HISI_wtdg @HiMPP"

extern int default_margin;
extern int nodeamon;
extern volatile void *g_wtdg_reg_base;

module_param(default_margin, int, 0);
MODULE_PARM_DESC(default_margin,
    "Watchdog default_margin in seconds. (0<default_margin<80, default=" __MODULE_STRING(HIDOG_TIMER_MARGIN) ")");

module_param(nodeamon, int, 0);
MODULE_PARM_DESC(nodeamon,
    "By default, a kernel daemon feed watchdog when idle, set 'nodeamon=1' to disable this. (default=0)");

static int hi_wdg_probe(struct platform_device *pdev)
{
    struct resource *mem = NULL;

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    g_wtdg_reg_base = devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR((void*)g_wtdg_reg_base)) {
        return PTR_ERR((void*)g_wtdg_reg_base);
    }

    return watchdog_init();
}

static int hi_wdg_remove(struct platform_device *pdev)
{
    hi_wtdg_unused(pdev);

    watchdog_exit();
    return 0;
}

static const struct of_device_id g_hi_wdg_match[] = {
    { .compatible = "hisilicon,hi_wdg" },
    { .compatible = "hisilicon,hi-wdg" },
    {},
};

static struct platform_driver g_hi_wdg_driver = {
    .probe  = hi_wdg_probe,
    .remove = hi_wdg_remove,
    .driver =  { .name = "hi_wdg",
                 .of_match_table = g_hi_wdg_match,
               },
};

module_platform_driver(g_hi_wdg_driver);
MODULE_LICENSE("GPL");

/*
 * Export symbol
 */
MODULE_AUTHOR("Digital Media Team ,Hisilicon crop ");
MODULE_DESCRIPTION("hi_wdg Driver");
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);


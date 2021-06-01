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
#include "hiirq.h"
#include "hi_osal.h"

#define OSDRV_MODULE_VERSION_STRING "HISI_irq @HiMPP"

static int hi_irq_probe(struct platform_device *pdev)
{
    printk("enter hi_irq_probe \n");
    return hiirq_init(pdev);
}

static int hi_irq_remove(struct platform_device *pdev)
{
    hiirq_exit();
    return 0;
}

static const struct of_device_id g_hi_irq_match[] = {
    { .compatible = "hisilicon,hi_irq" },
    { .compatible = "hisilicon,hi-irq" },
    {},
};

static struct platform_driver g_hi_irq_driver = {
    .probe  = hi_irq_probe,
    .remove = hi_irq_remove,
    .driver =  {
        .name = HIIRQ_DEVICE_NAME, /* hi_irq */
        .of_match_table = g_hi_irq_match,
    },
};

osal_module_platform_driver(g_hi_irq_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon");
MODULE_DESCRIPTION("Hisilicon Infrared remoter(HIIR11) Device Driver");
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);

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
#include "hi_adc.h"

#define OSDRV_MODULE_VERSION_STRING "HISI_adc @HiMPP"

extern unsigned int g_lsadc_irq;
extern volatile void *g_lsadc_reg_base;

static int hi_adc_probe(struct platform_device *pdev)
{
    struct resource *mem = NULL;

    g_lsadc_irq = platform_get_irq(pdev, 0);
    if (g_lsadc_irq <= 0) {
            dev_err(&pdev->dev, "cannot find lsadc IRQ%d. \n", g_lsadc_irq);
    }

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    g_lsadc_reg_base = (volatile void *)devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR((void*)g_lsadc_reg_base)) {
        dev_err(&pdev->dev, "lsadc reg map failed. \n");
    }

    return lsadc_init();
}

static int hi_adc_remove(struct platform_device *pdev)
{
    hi_adc_unused(pdev);

    lsadc_exit();
    return 0;
}

static const struct of_device_id g_hi_adc_match[] = {
    { .compatible = "hisilicon,hisi-lsadc" },
    {},
};

static struct platform_driver g_hi_lsadc_driver = {
    .probe  = hi_adc_probe,
    .remove = hi_adc_remove,
    .driver =  { .name = "hi_lsadc",
                 .of_match_table = g_hi_adc_match,
               },
};

module_platform_driver(g_hi_lsadc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Digital Media Team ,Hisilicon crop ");
MODULE_DESCRIPTION("HISI ADC Driver");
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);


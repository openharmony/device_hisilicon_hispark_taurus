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
#include "tde_init.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>
#include "hi_type.h"
#include "hi_common.h"
#include "hi_osal.h"

hi_u32 g_u32TdeTmpBuf = 1658880; /* 1658880 buffer size */
bool g_bResizeFilter = true;
hi_u32 g_u32TdeBuf = 0x20000; /* 0x20000 buffer size */

module_param(g_u32TdeTmpBuf, uint, S_IRUGO);
module_param(g_bResizeFilter, bool, S_IRUGO);
module_param(g_u32TdeBuf, uint, S_IRUGO);

static int hi35xx_tde_probe(struct platform_device *pdev)
{
    struct resource *mem = HI_NULL;
    int tde_irq;
    hi_u32 *base_vir_addr = HI_NULL;
    mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, "tde");
    base_vir_addr = devm_ioremap_resource(&pdev->dev, mem);
    tde_hal_set_base_vir_addr(base_vir_addr);
    if (IS_ERR(base_vir_addr)) {
        return PTR_ERR(base_vir_addr);
    }

    tde_irq = osal_platform_get_irq_byname(pdev, "tde_osr_isr");
    if (tde_irq <= 0) {
        dev_err(&pdev->dev, "cannot find tde IRQ\n");
        return -1;
    }
    set_tde_irq((hi_u32)tde_irq);

    if (g_u32TdeBuf > 0) {
        tde_init_set_buf(g_u32TdeBuf);
    }
    tde_set_resize_filter(g_bResizeFilter);
    if (g_u32TdeTmpBuf > 0) {
        tde_set_tde_tmp_buffer(g_u32TdeTmpBuf);
    }

    if (tde_drv_mod_init() != HI_SUCCESS) {
        osal_printk("load tde.ko for %s...FAILED!\n", CHIP_NAME);
        return -1;
    }
    osal_printk("load tde.ko for %s...OK!\n", CHIP_NAME);
    return 0;
}

static int hi35xx_tde_remove(struct platform_device *pdev)
{
    hi_unused(pdev);
    tde_drv_mod_exit();
    osal_printk("unload tde.ko for %s...OK!\n", CHIP_NAME);
    return 0;
}

static const struct of_device_id g_hi35xx_tde_match[] = {
    { .compatible = "hisilicon,hisi-tde" },
    {},
};
MODULE_DEVICE_TABLE(of, g_hi35xx_tde_match);

static struct platform_driver g_hi35xx_tde_driver = {
    .probe = hi35xx_tde_probe,
    .remove = hi35xx_tde_remove,
    .driver = {
        .name = "hi35xx_tde",
        .of_match_table = g_hi35xx_tde_match,
    },
};

osal_module_platform_driver(g_hi35xx_tde_driver);

MODULE_LICENSE("GPL");


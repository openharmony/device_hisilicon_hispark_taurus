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

extern unsigned int g_mipi_tx_irq_num;
extern void *g_mipi_tx_regs_va;

extern int mipi_tx_module_init(int smooth);
extern void mipi_tx_module_exit(void);
static int smooth = 0;

static int hi35xx_mipi_tx_probe(struct platform_device *pdev)
{
    struct  resource *mem;

    mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mipi_tx");
    g_mipi_tx_regs_va = devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR(g_mipi_tx_regs_va)) {
        dev_err(&pdev->dev, "mipi_tx remap mem error.\n");
        return PTR_ERR(g_mipi_tx_regs_va);
    }

    g_mipi_tx_irq_num = platform_get_irq_byname(pdev, "mipi_tx");
    if (g_mipi_tx_irq_num <= 0) {
        dev_err(&pdev->dev, "can not find mipi_tx IRQ\n");
    }
    return mipi_tx_module_init(smooth);
}

static int hi35xx_mipi_tx_remove(struct platform_device *pdev)
{
    (void)pdev;
    mipi_tx_module_exit();
    g_mipi_tx_regs_va = NULL;

    return 0;
}

static const struct of_device_id hi35xx_mipi_tx_match[] = {
    { .compatible = "hisilicon,hisi-mipi_tx" },
    {},
};
MODULE_DEVICE_TABLE(of, hi35xx_mipi_tx_match);

static struct platform_driver hi35xx_mipi_tx_driver = {
    .probe          = hi35xx_mipi_tx_probe,
    .remove         = hi35xx_mipi_tx_remove,
    .driver         = {
        .name   = "hi35xx_mipi_tx",
        .of_match_table = hi35xx_mipi_tx_match,
    },
};

module_param(smooth, int, S_IRUGO);
module_platform_driver(hi35xx_mipi_tx_driver);

MODULE_AUTHOR("Digital Media Team ,Hisilicon crop.");
MODULE_DESCRIPTION("Mipi_Tx Driver");
MODULE_VERSION("HI_VERSION = HISI_mipi_tx @HiMPP");
MODULE_LICENSE("GPL");

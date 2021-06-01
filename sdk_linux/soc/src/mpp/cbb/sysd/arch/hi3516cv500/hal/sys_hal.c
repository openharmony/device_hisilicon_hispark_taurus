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

#include "sys_hal.h"

#include "hi_osal.h"
#include "hi_type.h"
#include "hi_board.h"

void *g_reg_crg_base_va = HI_NULL;
void *g_reg_sys_base_va = HI_NULL;
void *g_reg_ddr0_base_va = HI_NULL;
void *g_reg_misc_base_va = HI_NULL;
void *g_reg_otp_base_va = HI_NULL;

#define io_crg_address(x) ((hi_uintptr_t)g_reg_crg_base_va + (x) - CRG_REGS_ADDR)
#define io_sys_address(x) ((hi_uintptr_t)g_reg_sys_base_va + (x) - SYS_REGS_ADDR)
#define io_ddr0_address(x) ((hi_uintptr_t)g_reg_ddr0_base_va + (x) - DDRC0_REG_ADDR)
#define io_misc_address(x) ((hi_uintptr_t)g_reg_misc_base_va + (x) - MISC_REGS_ADDR)

static osal_spinlock_t g_crg_spin_lock = {0};
#define sys_crg_spin_lock(flags) osal_spin_lock_irqsave(&g_crg_spin_lock, &(flags))
#define sys_crg_spin_unlock(flags) osal_spin_unlock_irqrestore(&g_crg_spin_lock, &(flags))

static void hi_reg_set_bit(unsigned long value, unsigned long offset, unsigned long addr)
{
    unsigned long t, mask;
    unsigned long flags;

    sys_crg_spin_lock(flags);
    mask = 1 << offset;
    t = osal_readl((const volatile void *)(hi_uintptr_t)addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    osal_writel(t, (volatile void *)(hi_uintptr_t)addr);
    sys_crg_spin_unlock(flags);
}

static void hi_reg_write32(unsigned long value, unsigned long mask, unsigned long addr)
{
    unsigned long t;
    unsigned long flags;

    sys_crg_spin_lock(flags);
    t = osal_readl((const volatile void *)(hi_uintptr_t)addr);
    t &= ~mask;
    t |= value & mask;
    osal_writel(t, (volatile void *)(hi_uintptr_t)addr);
    sys_crg_spin_unlock(flags);
}

static inline void hi_reg_read(unsigned int *pvalue, unsigned long addr)
{
    *pvalue = osal_readl((const volatile void *)(hi_uintptr_t)addr);
}

/* VO control */
#define LCD_CRG_PERCTL_ADDR CRG_PERCTL65_ADDR
#define VOU_CRG_PERCTL_ADDR CRG_PERCTL66_ADDR

hi_s32 sys_hal_vou_bus_reset_sel(hi_bool reset)
{
    hi_u32 tmp = (reset == HI_TRUE) ? 1 : 0;

    hi_reg_set_bit(tmp, 0, io_crg_address(VOU_CRG_PERCTL_ADDR));

    return 0;
}

hi_s32 sys_hal_vou_apb_clk_en(hi_s32 dev, hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_set_bit(tmp, 1, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 1:vo software reset clock */

    return 0;
}

/* VO AXI BUS CLK */
hi_s32 sys_hal_vou_bus_clk_en(hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;

    hi_reg_set_bit(tmp, 2, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 2:vo axi clock */

    return 0;
}

hi_s32 sys_hal_vou_cfg_clk_en(hi_s32 dev, hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_set_bit(tmp, 3, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 3:vo cfg clock */

    return 0;
}

hi_s32 sys_hal_vou_core_clk_en(hi_s32 dev, hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_set_bit(tmp, 5, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 5:vo core clock */

    return 0;
}

hi_s32 sys_hal_vou_dev_clk_en(hi_s32 vo_dev, hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(vo_dev);

    hi_reg_set_bit(tmp, 6, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 6:vo dev clock */

    return 0;
}

hi_s32 sys_hal_vou_out_clk_en(hi_s32 dev, hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_set_bit(tmp, 8, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 8:VO_CLKOUT */

    return 0;
}

hi_s32 sys_hal_vou_bt_clk_en(hi_s32 vo_dev, hi_bool bt_clk_en)
{
    hi_u32 tmp = (bt_clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(vo_dev);

    hi_reg_set_bit(tmp, 8, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 8:vdp bt_bp clock en */
    hi_reg_set_bit(tmp, 9, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 9:vdp BT1120/BT656 clock en */

    return 0;
}

hi_s32 sys_hal_vou_hdmi_clk_en(hi_s32 vo_dev, hi_bool hdmi_clk_en)
{
    hi_u32 tmp = (hdmi_clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(vo_dev);

    hi_reg_set_bit(tmp, 10, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 10:vdp hdmi clock en */

    return 0;
}

hi_s32 sys_hal_vou_mipi_clk_en(hi_s32 vo_dev, hi_bool mipi_clk_en)
{
    hi_u32 tmp = (mipi_clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(vo_dev);

    hi_reg_set_bit(tmp, 11, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 11:vdp mipi clock en */

    return 0;
}

hi_s32 sys_hal_vou_hd0_div_mode(hi_s32 dev, hi_u32 hd0_div_mod)
{
    hi_unused(dev);

    hi_reg_write32(hd0_div_mod << 12, 0x3 << 12, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 12:clock div; 0x3:mask */

    return 0;
}

hi_s32 sys_hal_vou_hd_clk_sel(hi_s32 dev, hi_u32 clk_sel)
{
    hi_unused(dev);

    hi_reg_write32(clk_sel << 14, 0x1 << 14, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 14:vdp hd clock; 0x1:mask */

    return 0;
}

hi_s32 sys_hal_vou_hdmi_tx_clk_en(hi_s32 dev, hi_bool hdmi_tx_clk_en)
{
    hi_u32 tmp = (hdmi_tx_clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_write32(tmp << 18, 0x1 << 18, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 18:vdp hdmi clock; 0x1:mask */

    return 0;
}

hi_s32 sys_hal_vou_mipi_tx_clk_en(hi_s32 dev, hi_bool mi_pi_tx_clk_en)
{
    hi_u32 tmp = (mi_pi_tx_clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_write32(tmp << 19, 0x1 << 19, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 19:mipi_tx clock; 0x1:mask */

    return 0;
}

hi_s32 sys_hal_vou_hd_out_pctrl(hi_s32 dev, hi_bool clk_reverse)
{
    hi_u32 tmp = (clk_reverse == HI_TRUE) ? 1 : 0;
    hi_unused(dev);

    hi_reg_set_bit(tmp, 20, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 20:VDP_MIPI phase */

    return 0;
}

hi_s32 sys_hal_vou_out_clk_sel(hi_s32 dev, hi_u32 clk_sel)
{
    hi_unused(dev);

    hi_reg_write32(clk_sel << 21, 0xf << 21, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 25:clock sel; 0xf:mask */

    return 0;
}

hi_s32 sys_hal_vou_hd_hdmi_clk_div(hi_s32 dev, hi_u32 hdmi_clk_div)
{
    hi_unused(dev);

    hi_reg_write32(hdmi_clk_div << 25, 0x1f << 25, io_crg_address(VOU_CRG_PERCTL_ADDR)); /* 25:clock div; 0x1f:mask */

    return 0;
}
#ifdef HI_DEBUG
hi_s32 sys_hal_vou_bt_clk_sel(hi_s32 vo_dev, hi_u32 bt_clk_ch_sel)
{
    if (vo_dev == 0) {
    } else {
        return -1;
    }

    return 0;
}
#endif

hi_s32 sys_hal_vou_lcd_clk_en(hi_s32 vo_dev, hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;
    hi_unused(vo_dev);

    hi_reg_set_bit(tmp, 27, io_crg_address(LCD_CRG_PERCTL_ADDR)); /* 27:lcd clock en */

    return 0;
}

hi_s32 sys_hal_lcd_mclk_div(hi_u32 mclk_div)
{
    hi_reg_write32(mclk_div, 0x7ffffff, io_crg_address(LCD_CRG_PERCTL_ADDR)); /* 0x7ffffff:clock div coef */

    return 0;
}

hi_s32 sys_hal_lcd_data_mode(hi_u32 data_mode)
{
    hi_reg_write32(data_mode << 8, 0x7 << 8, io_misc_address(MISC_CTL18_ADDR)); /* bit[8~10]:out mode; 0x7:mask */

    return 0;
}

hi_s32 sys_hal_set_vo_pll_frac(hi_s32 pll, hi_u32 bits_set)
{
    if ((pll == 0) || (pll == 1)) {
        hi_reg_write32(bits_set << 0, 0xffffff << 0, io_crg_address(CRG_PERCTL6_ADDR));
    } else {
        return -1;
    }
    return 0;
}

hi_s32 sys_hal_set_vo_pll_postdiv1(hi_s32 pll, hi_u32 bits_set)
{
    if ((pll == 0) || (pll == 1)) {
        hi_reg_write32(bits_set << 24, 0x7 << 24, io_crg_address(CRG_PERCTL6_ADDR)); /* 24:vpll 1st;0x7:coef */
    } else {
        return -1;
    }
    return 0;
}
hi_s32 sys_hal_set_vo_pll_postdiv2(hi_s32 pll, hi_u32 bits_set)
{
    if ((pll == 0) || (pll == 1)) {
        hi_reg_write32(bits_set << 28, 0x7 << 28, io_crg_address(CRG_PERCTL6_ADDR)); /* 28:vpll 2nd;0x7:coef */
    } else {
        return -1;
    }
    return 0;
}
hi_s32 sys_hal_set_vo_pll_refdiv(hi_s32 pll, hi_u32 bits_set)
{
    if ((pll == 0) || (pll == 1)) {
        hi_reg_write32(bits_set << 12, 0x3f << 12, io_crg_address(CRG_PERCTL7_ADDR)); /* 12:vpll div;0x3f:coef */
    } else {
        return -1;
    }
    return 0;
}
hi_s32 sys_hal_set_vo_pll_fbdiv(hi_s32 pll, hi_u32 bits_set)
{
    if ((pll == 0) || (pll == 1)) {
        hi_reg_write32(bits_set << 0, 0xfff << 0, io_crg_address(CRG_PERCTL7_ADDR)); /* 0xfff:coef */
    } else {
        return -1;
    }
    return 0;
}

hi_s32 sys_hal_aio_reset_sel(hi_bool reset)
{
    hi_u32 tmp = (reset == HI_TRUE) ? 1 : 0;

    hi_reg_set_bit(tmp, 0, io_crg_address(CRG_PERCTL103_ADDR)); /* 0:software reset */
    hi_reg_set_bit(tmp, 2, io_crg_address(CRG_PERCTL103_ADDR)); /* 2:pop reset */

    return 0;
}

hi_s32 sys_hal_aio_clk_en(hi_bool clk_en)
{
    hi_u32 tmp = (clk_en == HI_TRUE) ? 1 : 0;

    hi_reg_set_bit(tmp, 1, io_crg_address(CRG_PERCTL103_ADDR)); /* 1:clock */
    hi_reg_set_bit(tmp, 3, io_crg_address(CRG_PERCTL103_ADDR)); /* 3:pll clock */

    return 0;
}


static hi_void sys_hal_low_power(hi_void)
{
    hi_reg_write32(0x00, 0x01, io_crg_address(CRG_PERCTL111_ADDR));
}

hi_s32 sys_hal_init(hi_void)
{
    if (osal_spin_lock_init(&g_crg_spin_lock) < 0) {
        osal_printk("spinlock init fail, line: %d. \n", __LINE__);
        return -1;
    }

    if (g_reg_crg_base_va == HI_NULL) {
        g_reg_crg_base_va = (void *)osal_ioremap(CRG_REGS_ADDR, (hi_u32)CRG_REGS_SIZE);
        if (g_reg_crg_base_va == HI_NULL) {
            osal_printk("remap crg reg fail, line: %d. \n", __LINE__);
            goto sys_hal_fail;
        }
    }

    if (g_reg_sys_base_va == HI_NULL) {
        g_reg_sys_base_va = (void *)osal_ioremap(SYS_REGS_ADDR, (hi_u32)SYS_REGS_SIZE);
        if (g_reg_sys_base_va == HI_NULL) {
            osal_printk("remap sys reg fail, line: %d. \n", __LINE__);
            goto sys_hal_fail;
        }
    }

    if (g_reg_ddr0_base_va == HI_NULL) {
        g_reg_ddr0_base_va = (void *)osal_ioremap(DDRC0_REG_ADDR, (hi_u32)DDRC_REGS_SIZE);
        if (g_reg_ddr0_base_va == HI_NULL) {
            osal_printk("remap ddr0 reg fail, line: %d. \n", __LINE__);
            goto sys_hal_fail;
        }
    }

    if (g_reg_misc_base_va == HI_NULL) {
        g_reg_misc_base_va = (void *)osal_ioremap(MISC_REGS_ADDR, (hi_u32)MISC_REGS_SIZE);
        if (g_reg_misc_base_va == HI_NULL) {
            osal_printk("remap MISC reg fail, line: %d. \n", __LINE__);
            goto sys_hal_fail;
        }
    }

    if (g_reg_otp_base_va == HI_NULL) {
        g_reg_otp_base_va = (void *)osal_ioremap(OTP_REGS_ADDR, (hi_u32)OTP_REGS_SIZE);
        if (g_reg_otp_base_va == HI_NULL) {
            osal_printk("remap OTP reg fail, line: %d. \n", __LINE__);
            goto sys_hal_fail;
        }
    }

    sys_hal_low_power();

    return 0;

sys_hal_fail:
    sys_hal_exit();
    return -1;
}

hi_void sys_hal_exit(hi_void)
{
    if (g_reg_crg_base_va != HI_NULL) {
        osal_iounmap(g_reg_crg_base_va, (hi_u32)CRG_REGS_SIZE);
        g_reg_crg_base_va = HI_NULL;
    }
    if (g_reg_sys_base_va != HI_NULL) {
        osal_iounmap(g_reg_sys_base_va, (hi_u32)SYS_REGS_SIZE);
        g_reg_sys_base_va = HI_NULL;
    }
    if (g_reg_ddr0_base_va != HI_NULL) {
        osal_iounmap(g_reg_ddr0_base_va, (hi_u32)DDRC_REGS_SIZE);
        g_reg_ddr0_base_va = HI_NULL;
    }

    if (g_reg_misc_base_va != HI_NULL) {
        osal_iounmap(g_reg_misc_base_va, (hi_u32)MISC_REGS_SIZE);
        g_reg_misc_base_va = HI_NULL;
    }

    if (g_reg_otp_base_va != HI_NULL) {
        osal_iounmap(g_reg_otp_base_va, (hi_u32)OTP_REGS_SIZE);
        g_reg_otp_base_va = HI_NULL;
    }

    osal_spin_lock_destroy(&g_crg_spin_lock);

    return;
}

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
#include "hdmi_product_define.h"
#include "hdmi_reg_crg.h"
#include "hdmi_reg_dphy.h"

#define HDMI_IO_CFG_FUNCTION1_SEL 0x1

hi_void hdmi_tx_reg_write(hi_u32 *reg_addr, hi_u32 value)
{
    *(volatile hi_u32 *)reg_addr = value;
    return;
}

hi_u32 hdmi_tx_reg_read(const hi_u32 *reg_addr)
{
    return *(volatile hi_u32 *)(reg_addr);
}

hi_void hdmi_reg_write_u32(hi_u32 reg_addr, hi_u32 value)
{
    volatile hi_u32 *addr = HI_NULL;

    addr = (volatile hi_u32 *)osal_ioremap_nocache((hi_u64)reg_addr, HDMI_REGISTER_SIZE);
    if (addr != HI_NULL) {
        *addr = value;
        osal_iounmap((void *)addr, HDMI_REGISTER_SIZE);
    } else {
        hdmi_err("osal_ioremap_nocache addr=0x%x err!\n", reg_addr);
    }

    return;
}

hi_u32 hdmi_reg_read_u32(hi_u32 reg_addr)
{
    volatile hi_u32 *addr = HI_NULL;
    hi_u32 value = 0;

    addr = (volatile hi_u32 *)osal_ioremap_nocache((hi_u64)reg_addr, HDMI_REGISTER_SIZE);
    if (addr != HI_NULL) {
        value = *addr;
        osal_iounmap((void *)addr, HDMI_REGISTER_SIZE);
    } else {
        hdmi_err("osal_ioremap_nocache addr=0x%x\n err!\n", reg_addr);
    }

    return value;
}

hi_void drv_hdmi_prod_io_cfg_set(hi_void)
{
    /* no IO multiplex */
    return;
}

hi_void drv_hdmi_prod_crg_gate_set(hi_bool enable)
{
#ifndef HDMI_FPGA_SUPPORT
    /* gate */
    /* crg68 HDMITX_CTRL CRG 0x00110 0x0001043F */
    hdmi_reg_ssc_in_cken_set(enable);
    hdmi_reg_ssc_bypass_cken_set(enable);
    hdmi_reg_ctrl_osc_24m_cken_set(enable);
    hdmi_reg_ctrl_cec_cken_set(enable);
    hdmi_reg_ctrl_os_cken_set(enable);
    hdmi_reg_ctrl_as_cken_set(enable);
    hdmi_reg_pxl_cken_set(enable);
    /* crg70 HDMITX_PHY 0x00114 0x00000001 */
    hdmi_reg_hdmirx_phy_tmds_cken_set(enable);
#endif

    return;
}

hi_void drv_hdmi_prod_crg_all_reset_set(hi_bool enable)
{
#ifndef HDMI_FPGA_SUPPORT
    /* crg68 HDMITX_CTRL CRG 0x00110 0x0001043F */
    hdmi_reg_ctrl_bus_srst_req_set(enable);
    hdmi_reg_ctrl_srst_req_set(enable);
    hdmi_reg_ctrl_cec_srst_req_set(enable);
    hdmi_reg_ssc_srst_req_set(enable);
    /* crg69 HDMITX_PHY 0x00114 0x00000001 */
    hdmi_reg_phy_srst_req_set(enable);
    hdmi_reg_phy_tmds_srst_req_set(enable);
    enable = !enable;
    /* delay 1us, to insure ctrl reset success. */
    osal_udelay(1);
    /* crg68  HDMITX_CTRL CRG  0x00110   0x0001043F */
    hdmi_reg_ctrl_bus_srst_req_set(enable);
    hdmi_reg_ctrl_srst_req_set(enable);
    hdmi_reg_ctrl_cec_srst_req_set(enable);
    hdmi_reg_ssc_srst_req_set(enable);
    /* crg69 HDMITX_PHY 0x00114 0x00000001 */
    hdmi_reg_phy_srst_req_set(enable);
    hdmi_reg_phy_tmds_srst_req_set(enable);
    /* delay 2us, avoid system crash when start HDMI(no boot) */
    /* delay 100ms, HPD can not read after reset when enable HPD filtering if not delay. */
    osal_msleep(100);
#endif

    return;
}

hi_void drv_hdmi_hardware_reset(hi_void)
{
#ifndef HDMI_FPGA_SUPPORT
    if (hdmi_reg_crg_init() != HI_SUCCESS) {
        hdmi_err("hdmi crg reg init failed!!!!!!!\n");
        return;
    }
    /* crg68 HDMITX_CTRL CRG 0x00110 0x0001043F */
    hdmi_reg_ctrl_bus_srst_req_set(HI_TRUE);
    hdmi_reg_ctrl_srst_req_set(HI_TRUE);
    hdmi_reg_ctrl_cec_srst_req_set(HI_TRUE);
    hdmi_reg_ssc_srst_req_set(HI_TRUE);
    /* crg69 HDMITX_PHY 0x00114 0x00000001 */
    hdmi_reg_phy_srst_req_set(HI_TRUE);
    hdmi_reg_phy_tmds_srst_req_set(HI_TRUE);
    /* crg68 HDMITX_CTRL CRG 0x00110 gate */
    drv_hdmi_prod_crg_gate_set(HI_FALSE);
    hdmi_reg_crg_deinit();
#endif

    return;
}

hi_void drv_hdmi_low_power_set(hi_bool enable)
{
    hi_unused(enable);
    return;
}

hi_void drv_hdmi_prod_crg_phy_reset_set(hi_bool enable)
{
#ifndef HDMI_FPGA_SUPPORT
    /* crg69 HDMITX_PHY 0x00114 0x00000001 */
    hdmi_reg_phy_srst_req_set(enable);
    hdmi_reg_phy_tmds_srst_req_set(enable);
    enable = !enable;
    /* crg69 HDMITX_PHY 0x00114 0x00000001 */
    hdmi_reg_phy_srst_req_set(enable);
    hdmi_reg_phy_tmds_srst_req_set(enable);
#endif
    return;
}

hi_void drv_hdmi_prod_crg_phy_reset_get(hi_bool *enable)
{
#ifndef HDMI_FPGA_SUPPORT
    hdmi_if_null_return_void(enable);

    /* crg69 HDMITX_PHY 0x00114 0x00000001 */
    if (hdmi_reg_phy_srst_req_get() || hdmi_reg_phy_tmds_srst_req_get()) {
        *enable = HI_TRUE;
    } else {
        *enable = HI_FALSE;
    }
#endif

    return;
}

hi_void drv_hdmi_prod_crg_div_set(hdmi_crg_cfg *crg_cfg)
{
#ifndef HDMI_FPGA_SUPPORT
    hi_u32 tmp;

    if (crg_cfg != HI_NULL) {
        tmp = crg_cfg->ssc_bypass_div - 1;
        hdmi_reg_ssc_clk_div_set(tmp);
        tmp = crg_cfg->tmds_clk_div - 1;
        hdmi_reg_tmds_clk_div_set(tmp);
        /* use phy reg to rest dphy, don't use CRG phy rst reg */
        hdmi_reg_dphy_rst_set(HI_TRUE);
        hdmi_reg_dphy_rst_set(HI_FALSE);
    }
#endif

    return;
}

hi_void drv_hdmi_prod_crg_init(hi_void)
{
#ifndef HDMI_FPGA_SUPPORT
    drv_hdmi_prod_io_cfg_set();
    drv_hdmi_prod_crg_gate_set(HI_TRUE);
    drv_hdmi_prod_crg_all_reset_set(HI_TRUE);
#endif

    return;
}

hdmi_chip_type drv_hdmi_prod_chip_get(hi_void)
{
    return CHIP_HI3516CV500;
}


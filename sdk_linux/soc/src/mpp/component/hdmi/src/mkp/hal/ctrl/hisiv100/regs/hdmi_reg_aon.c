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
#include "hdmi_reg_aon.h"
#include "hdmi_product_define.h"

volatile tx_aon_regs *g_tx_aon_regs = NULL;

int hdmi_reg_aon_regs_init(hi_char *addr)
{
    g_tx_aon_regs = (volatile tx_aon_regs *)(addr + (HDMI_TX_BASE_ADDR_AON));
    return HI_SUCCESS;
}

int hdmi_reg_aon_regs_deinit(void)
{
    if (g_tx_aon_regs != HI_NULL) {
        g_tx_aon_regs = HI_NULL;
    }
    return HI_SUCCESS;
}

int hdmi_reg_aon_intr_mask0_set(unsigned int aon_intr_mask0)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_intr_mask mask;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_irq_mask.u32);
    mask.u32 = hdmi_tx_reg_read(reg_addr);
    mask.bits.aon_intr_mask0 = aon_intr_mask0;
    hdmi_tx_reg_write(reg_addr, mask.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aon_intr_stat1_set(unsigned int aon_intr_stat1)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_intr_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_irq_state.u32);
    state.u32 = 0;
    state.bits.aon_intr_stat1 = aon_intr_stat1;
    hdmi_tx_reg_write(reg_addr, state.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aon_intr_stat0_set(unsigned int aon_intr_stat0)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_intr_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_irq_state.u32);
    state.u32 = 0;
    state.bits.aon_intr_stat0 = aon_intr_stat0;
    hdmi_tx_reg_write(reg_addr, state.u32);

    return HI_SUCCESS;
}

int hdmi_reg_dcc_man_en_set(unsigned int dcc_man_en)
{
    hi_u32 *reg_addr = NULL;
    ddc_mst_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->ddc_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.dcc_man_en = dcc_man_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_ddc_sda_oen_set(unsigned int ddc_sda_oen)
{
    hi_u32 *reg_addr = NULL;
    ddc_man_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->ddc_sw_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.ddc_sda_oen = ddc_sda_oen;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_ddc_scl_oen_set(unsigned int ddc_scl_oen)
{
    hi_u32 *reg_addr = NULL;
    ddc_man_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->ddc_sw_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.ddc_scl_oen = ddc_scl_oen;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_ddc_i2c_no_ack_get(void)
{
    hi_u32 *reg_addr = NULL;
    ddc_mst_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->mst_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.ddc_i2c_no_ack;
}

int hdmi_reg_ddc_i2c_bus_low_get(void)
{
    hi_u32 *reg_addr = NULL;
    ddc_mst_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->mst_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.ddc_i2c_bus_low;
}

int hdmi_reg_hpd_polarity_ctl_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    hotplug_st_cfg cfg;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->hpd_cfg.u32);
    cfg.u32 = hdmi_tx_reg_read(reg_addr);
    return cfg.bits.hpd_polarity_ctl;
}

int hdmi_reg_phy_rx_sense_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.phy_rx_sense;
}

int hdmi_reg_hotplug_state_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.hotplug_state;
}

int hdmi_reg_aon_intr_stat1_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_intr_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_irq_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.aon_intr_stat1;
}

int hdmi_reg_aon_intr_stat0_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_aon_intr_state state;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->aon_irq_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.aon_intr_stat0;
}

int hdmi_reg_ddc_sda_st_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    ddc_man_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->ddc_sw_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.ddc_sda_st;
}

int hdmi_reg_ddc_scl_st_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    ddc_man_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_tx_aon_regs->ddc_sw_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.ddc_scl_st;
}


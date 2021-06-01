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
#include "hdmi_reg_audio_path.h"
#include "hdmi_product_define.h"

volatile hdmi_reg_audio_path *g_audio_path_regs = NULL;

int hdmi_reg_audio_path_regs_init(hi_char *addr)
{
    g_audio_path_regs = (volatile hdmi_reg_audio_path *)(addr + (HDMI_TX_BASE_ADDR_AUDIO));
    return HI_SUCCESS;
}

int hdmi_reg_audio_path_regs_deinit(void)
{
    if (g_audio_path_regs != HI_NULL) {
        g_audio_path_regs = HI_NULL;
    }
    return HI_SUCCESS;
}

int hdmi_reg_aud_spdif_en_set(unsigned int aud_spdif_en)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.aud_spdif_en = aud_spdif_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aud_i2s_en_set(unsigned int aud_i2s_en)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.aud_i2s_en = aud_i2s_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aud_layout_set(unsigned int aud_layout)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.aud_layout = aud_layout;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aud_mute_en_set(unsigned int aud_mute_en)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.aud_mute_en = aud_mute_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aud_in_en_set(unsigned int aud_in_en)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.aud_in_en = aud_in_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_ch_swap_set(unsigned int i2s_ch_swap)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_ch_swap = i2s_ch_swap;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_length_set(unsigned int i2s_length)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_length = i2s_length;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_vbit_set(unsigned int i2s_vbit)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_vbit = i2s_vbit;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_data_dir_set(unsigned int i2s_data_dir)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_data_dir = i2s_data_dir;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_justify_set(unsigned int i2s_justify)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_justify = i2s_justify;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_ws_polarity_set(unsigned int i2s_ws_polarity)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_ws_polarity = i2s_ws_polarity;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_1st_shift_set(unsigned int i2s_1st_shift)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_1st_shift = i2s_1st_shift;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_i2s_hbra_on_set(unsigned int i2s_hbra_on)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_i2s_ctrl.bits.i2s_hbra_on = i2s_hbra_on;
    hdmi_tx_reg_write(reg_addr, aud_i2s_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_chst_byte3_clock_accuracy_set(unsigned int chst_byte3_clock_accuracy)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg0 aud_chst_cfg0;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg0.u32);
    aud_chst_cfg0.u32 = hdmi_tx_reg_read(reg_addr);
    aud_chst_cfg0.bits.chst_byte3_clock_accuracy = chst_byte3_clock_accuracy;
    hdmi_tx_reg_write(reg_addr, aud_chst_cfg0.u32);

    return HI_SUCCESS;
}

int hdmi_reg_chst_byte3_fs_set(unsigned int chst_byte3_fs)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg0 aud_chst_cfg0;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg0.u32);
    aud_chst_cfg0.u32 = hdmi_tx_reg_read(reg_addr);
    aud_chst_cfg0.bits.chst_byte3_fs = chst_byte3_fs;
    hdmi_tx_reg_write(reg_addr, aud_chst_cfg0.u32);

    return HI_SUCCESS;
}

int hdmi_reg_chst_byte0_bset(unsigned int chst_byte0_b)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg0 aud_chst_cfg0;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg0.u32);
    aud_chst_cfg0.u32 = hdmi_tx_reg_read(reg_addr);
    aud_chst_cfg0.bits.chst_byte0_b = chst_byte0_b;
    hdmi_tx_reg_write(reg_addr, aud_chst_cfg0.u32);

    return HI_SUCCESS;
}

int hdmi_reg_chst_byte0_aset(unsigned int chst_byte0_a)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg0 aud_chst_cfg0;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg0.u32);
    aud_chst_cfg0.u32 = hdmi_tx_reg_read(reg_addr);
    aud_chst_cfg0.bits.chst_byte0_a = chst_byte0_a;
    hdmi_tx_reg_write(reg_addr, aud_chst_cfg0.u32);

    return HI_SUCCESS;
}

int hdmi_reg_chst_byte4_org_fs_set(unsigned int chst_byte4_org_fs)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg1 aud_chst_cfg1;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg1.u32);
    aud_chst_cfg1.u32 = hdmi_tx_reg_read(reg_addr);
    aud_chst_cfg1.bits.chst_byte4_org_fs = chst_byte4_org_fs;
    hdmi_tx_reg_write(reg_addr, aud_chst_cfg1.u32);

    return HI_SUCCESS;
}

int hdmi_reg_chst_byte4_length_set(unsigned int chst_byte4_length)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg1 aud_chst_cfg1;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg1.u32);
    aud_chst_cfg1.u32 = hdmi_tx_reg_read(reg_addr);
    aud_chst_cfg1.bits.chst_byte4_length = chst_byte4_length;
    hdmi_tx_reg_write(reg_addr, aud_chst_cfg1.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aud_fifo_hbr_mask_set(unsigned int aud_fifo_hbr_mask)
{
    hi_u32 *reg_addr = NULL;
    audio_fifo_ctrl aud_fifo_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->fifo_ctl.u32);
    aud_fifo_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_fifo_ctrl.bits.aud_fifo_hbr_mask = aud_fifo_hbr_mask;
    hdmi_tx_reg_write(reg_addr, aud_fifo_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aud_fifo_test_set(unsigned int aud_fifo_test)
{
    hi_u32 *reg_addr = NULL;
    audio_fifo_ctrl aud_fifo_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->fifo_ctl.u32);
    aud_fifo_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_fifo_ctrl.bits.aud_fifo_test = aud_fifo_test;
    hdmi_tx_reg_write(reg_addr, aud_fifo_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_acr_cts_hw_sw_sel_set(unsigned int acr_cts_hw_sw_sel)
{
    hi_u32 *reg_addr = NULL;
    audio_acr_ctrl aud_acr_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->acr_ctl.u32);
    aud_acr_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    aud_acr_ctrl.bits.acr_cts_hw_sw_sel = acr_cts_hw_sw_sel;
    hdmi_tx_reg_write(reg_addr, aud_acr_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_acr_n_val_sw_set(unsigned int acr_n_value)
{
    hi_u32 *reg_addr = NULL;
    acr_n_val_sw acr_n_val;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->acr_sw_n.u32);
    acr_n_val.u32 = hdmi_tx_reg_read(reg_addr);
    acr_n_val.bits.acr_n_val_sw = acr_n_value;
    hdmi_tx_reg_write(reg_addr, acr_n_val.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_aud_spdif_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.aud_spdif_en;
}

hi_u32 hdmi_reg_aud_i2s_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.aud_i2s_en;
}

hi_u32 hdmi_reg_aud_layout_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.aud_layout;
}

hi_u32 hdmi_reg_aud_mute_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.aud_mute_en;
}

hi_u32 hdmi_reg_aud_in_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->audio_ctl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.aud_in_en;
}

hi_u32 hdmi_reg_i2s_hbra_on_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    audio_i2s_ctrl aud_i2s_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->i2s_ctl.u32);
    aud_i2s_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return aud_i2s_ctrl.bits.i2s_hbra_on;
}

hi_u32 hdmi_reg_chst_byte3_fs_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg0 aud_chst_cfg0;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg0.u32);
    aud_chst_cfg0.u32 = hdmi_tx_reg_read(reg_addr);
    return aud_chst_cfg0.bits.chst_byte3_fs;
}

hi_u32 hdmi_reg_chst_byte4_org_fs_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg1 aud_chst_cfg1;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg1.u32);
    aud_chst_cfg1.u32 = hdmi_tx_reg_read(reg_addr);
    return aud_chst_cfg1.bits.chst_byte4_org_fs;
}

hi_u32 hdmi_reg_chst_byte4_length_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    audio_chst_cfg1 aud_chst_cfg1;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->chst_cfg1.u32);
    aud_chst_cfg1.u32 = hdmi_tx_reg_read(reg_addr);
    return aud_chst_cfg1.bits.chst_byte4_length;
}

hi_u32 hdmi_reg_aud_spdif_fs_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_state state;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->aud_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.aud_spdif_fs;
}

hi_u32 hdmi_reg_aud_length_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_audio_state state;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->aud_state.u32);
    state.u32 = hdmi_tx_reg_read(reg_addr);
    return state.bits.aud_length;
}

hi_u32 hdmi_reg_acr_cts_hw_sw_sel_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    audio_acr_ctrl aud_acr_ctrl;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->acr_ctl.u32);
    aud_acr_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return aud_acr_ctrl.bits.acr_cts_hw_sw_sel;
}

hi_u32 hdmi_reg_acr_n_val_sw_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    acr_n_val_sw acr_n_val;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->acr_sw_n.u32);
    acr_n_val.u32 = hdmi_tx_reg_read(reg_addr);
    return acr_n_val.bits.acr_n_val_sw;
}

hi_u32 hdmi_reg_acr_cts_val_sw_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    acr_cts_val_sw acr_cts_val;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->acr_sw_cts.u32);
    acr_cts_val.u32 = hdmi_tx_reg_read(reg_addr);
    return acr_cts_val.bits.acr_cts_val_sw;
}

hi_u32 hdmi_reg_acr_cts_val_hw_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    acr_cts_val_hw acr_cts_val;

    reg_addr = (hi_u32 *)&(g_audio_path_regs->acr_hw_cts.u32);
    acr_cts_val.u32 = hdmi_tx_reg_read(reg_addr);
    return acr_cts_val.bits.acr_cts_val_hw;
}


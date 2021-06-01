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
#include "vou_hal.h"
#include "hi_osal.h"
#include "hi_board.h"
#include "hi_comm_vo_adapt.h"
#include "vou_coef.h"
#include "hi_math_adapt.h"
#include "vou_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define HAL_PRINT       HI_PRINT

/* for CMP and DCMP */
#define CMP_SEG_OFFSET  (0x80 / 4)
#define DCMP_SEG_OFFSET (0x20 / 4)

#define REG_BYTES 4

volatile S_VDP_REGS_TYPE *g_vo_reg = HI_NULL;

hi_void hal_vou_init(hi_void)
{
    if (g_vo_reg == HI_NULL) {
        g_vo_reg = (volatile S_VDP_REGS_TYPE *)osal_ioremap(VOU_REGS_ADDR, (hi_u32)VOU_REGS_SIZE);
    }

    if (g_vo_reg == HI_NULL) {
        osal_printk("ioremap_nocache failed\n");
    }
}

hi_void hal_vou_exit(hi_void)
{
}

hi_void hal_write_reg(hi_u32 *address, hi_u32 value)
{
    vo_coef_addr *coef_addr = vo_drv_get_coef_buf_addr();
    hi_u32 addr = VOU_REGS_ADDR + (hi_ulong)(hi_uintptr_t)address - (hi_ulong)(hi_uintptr_t)(&(g_vo_reg->VOCTRL.u32));
    hi_u32 *tmp = HI_NULL;
    hi_u32 v0_mrg_addr = VOU_REGS_ADDR + MRG_OFFSET_ADDR;
    hi_u32 v1_mrg_addr = v0_mrg_addr + MRG_REGS_LEN;

    if ((addr >= v0_mrg_addr) && (addr < v0_mrg_addr + MRG_REGS_LEN)) {
        tmp = (addr - v0_mrg_addr) / REG_BYTES + (hi_u32 *)coef_addr->coef_vir_addr[VO_COEF_BUF_REGION_V0];
        *(volatile hi_u32 *)tmp = value;
    } else if ((addr >= v1_mrg_addr) && (addr < v1_mrg_addr + MRG_REGS_LEN)) {
        tmp = (addr - v1_mrg_addr) / REG_BYTES + (hi_u32 *)coef_addr->coef_vir_addr[VO_COEF_BUF_REGION_V1];
        *(volatile hi_u32 *)tmp = value;
    } else {
        *(volatile hi_u32 *)address = value;
    }

    return;
}

hi_u32 hal_read_reg(const hi_u32 *address)
{
    vo_coef_addr *coef_addr = vo_drv_get_coef_buf_addr();
    hi_u32 addr = VOU_REGS_ADDR + (hi_ulong)(hi_uintptr_t)address - (hi_ulong)(hi_uintptr_t)(&(g_vo_reg->VOCTRL.u32));
    hi_u32 *tmp = HI_NULL;

    hi_u32 v0_mrg_addr = VOU_REGS_ADDR + MRG_OFFSET_ADDR;
    hi_u32 v1_mrg_addr = v0_mrg_addr + MRG_REGS_LEN;

    if ((addr >= v0_mrg_addr) && (addr < v0_mrg_addr + MRG_REGS_LEN)) {
        tmp = (addr - v0_mrg_addr) / REG_BYTES + (hi_u32 *)coef_addr->coef_vir_addr[VO_COEF_BUF_REGION_V0];
        return *(volatile hi_u32 *)tmp;
    } else if ((addr >= v1_mrg_addr) && (addr < v1_mrg_addr + MRG_REGS_LEN)) {
        tmp = (addr - v1_mrg_addr) / REG_BYTES + (hi_u32 *)coef_addr->coef_vir_addr[VO_COEF_BUF_REGION_V1];
        return *(volatile hi_u32 *)tmp;
    } else {
        return *(volatile hi_u32 *)(address);
    }
}

hi_ulong vou_get_abs_addr(hal_disp_layer layer, hi_ulong reg)
{
    hi_ulong reg_abs_addr;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            reg_abs_addr = (reg) + (layer - HAL_DISP_LAYER_VHD0) * VHD_REGS_LEN;
            break;
        }

        case HAL_DISP_LAYER_GFX0: {
            reg_abs_addr = (reg) + (layer - HAL_DISP_LAYER_GFX0) * GFX_REGS_LEN;
            break;
        }

        default: {
            return 0;
        }
    }

    return reg_abs_addr;
}

hi_ulong vou_get_chn_abs_addr(hal_disp_outputchannel chan, hi_ulong reg)
{
    volatile hi_ulong reg_abs_addr;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            reg_abs_addr = reg + (chan - HAL_DISP_CHANNEL_DHD0) * DHD_REGS_LEN;
            break;
        }

        default: {
            return 0;
        }
    }

    return reg_abs_addr;
}

hi_ulong vou_get_hdmi_or_mipi_intf_abs_addr(hal_disp_intf intf, hi_ulong reg)
{
    volatile hi_ulong reg_abs_addr;

    switch (intf) {
        case HAL_DISP_INTF_HDMI: {
            reg_abs_addr = reg;
            break;
        }

        case HAL_DISP_INTF_MIPI:
        case HAL_DISP_INTF_MIPI_SLAVE: {
            reg_abs_addr = reg + 0x80;  /* 0x80: the offset of reg INTF_MIPI_CTRL */
            break;
        }
        default: {
            return 0;
        }
    }

    return reg_abs_addr;
}

hi_ulong vou_get_intf_abs_addr(hal_disp_intf intf, hi_ulong reg)
{
    volatile hi_ulong reg_abs_addr;
    hi_u32 bt_offset = 1;
    hi_u32 lcd_offset = 2;

    switch (intf) {
        case HAL_DISP_INTF_HDMI: {
            reg_abs_addr = reg;
            break;
        }

        case HAL_DISP_INTF_MIPI:
        case HAL_DISP_INTF_MIPI_SLAVE: {
            reg_abs_addr = reg + 0x80;  /* 0x80: the offset of reg INTF_MIPI_CTRL */
            break;
        }

        case HAL_DISP_INTF_BT656:
        case HAL_DISP_INTF_BT1120: {
            reg_abs_addr = reg + bt_offset * INTF_REGS_LEN;
            break;
        }

        case HAL_DISP_INTF_LCD:
        case HAL_DISP_INTF_LCD_6BIT:
        case HAL_DISP_INTF_LCD_8BIT:
        case HAL_DISP_INTF_LCD_16BIT:
        case HAL_DISP_INTF_LCD_18BIT:
        case HAL_DISP_INTF_LCD_24BIT: {
            reg_abs_addr = reg + lcd_offset * INTF_REGS_LEN;
            break;
        }

        default: {
            return 0;
        }
    }

    return reg_abs_addr;
}

hi_ulong vou_get_vid_abs_addr(hal_disp_layer layer, hi_ulong reg)
{
    volatile hi_ulong reg_abs_addr;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            reg_abs_addr = reg + (layer - HAL_DISP_LAYER_VHD0) * VID_REGS_LEN;
            break;
        }
        default: {
            return 0;
        }
    }

    return reg_abs_addr;
}

hi_ulong vou_get_gfx_abs_addr(hal_disp_layer layer, hi_ulong reg)
{
    volatile hi_ulong reg_abs_addr;

    switch (layer) {
        case HAL_DISP_LAYER_GFX0:
        case HAL_DISP_LAYER_GFX1:
        case HAL_DISP_LAYER_GFX3: {
            reg_abs_addr = reg + (layer - HAL_DISP_LAYER_GFX0) * GRF_REGS_LEN;
            break;
        }
        default: {
            return 0;
        }
    }

    return reg_abs_addr;
}

/* outstanding */
hi_void hal_sys_set_outstanding(hi_void)
{
    volatile U_MAC_OUTSTANDING mac_outstanding;

    mac_outstanding.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->MAC_OUTSTANDING.u32));
    mac_outstanding.bits.mstr0_routstanding = 0xf;  /* keep the default 0xf for master0 read. */
    mac_outstanding.bits.mstr0_woutstanding = 0x4;

    mac_outstanding.bits.mstr1_routstanding = 0x7;  /* keep the default 0x7 for master1 read. */
    mac_outstanding.bits.mstr1_woutstanding = 0x7;

    mac_outstanding.bits.mstr2_routstanding = 0x3;
    mac_outstanding.bits.mstr2_woutstanding = 0x3;
    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->MAC_OUTSTANDING.u32), mac_outstanding.u32);

    return;
}

hi_bool hal_disp_set_intf_enable(hal_disp_outputchannel chan, hi_bool intf)
{
    volatile U_DHD0_CTRL dhd0_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_CTRL.u32));
            dhd0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_ctrl.bits.intf_en = intf;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_ctrl.u32);

            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_get_intf_enable(hal_disp_outputchannel chan, hi_bool *intf_en)
{
    volatile U_DHD0_CTRL dhd0_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_CTRL.u32));
            dhd0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            *intf_en = dhd0_ctrl.bits.intf_en;
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_get_int_state(hal_disp_outputchannel chan, hi_bool *bottom)
{
    volatile U_DHD0_STATE dhd0_state;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_STATE.u32));
            dhd0_state.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            *bottom = dhd0_state.bits.bottom_field;
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_get_int_state_vcnt(hal_disp_outputchannel chan, hi_u32 *vcnt)
{
    volatile U_DHD0_STATE DHD0_STATE;
    volatile hi_ulong addr_reg;

    if (chan <= HAL_DISP_CHANNEL_DHD1) {
        addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_STATE.u32));
        DHD0_STATE.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        *vcnt = DHD0_STATE.bits.vcnt;
    } else {
        HAL_PRINT("error channel id found in %s: L%d\n", __FUNCTION__, __LINE__);
        return HI_FALSE;
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_intf_sync(hal_disp_outputchannel chan,
                               hal_disp_syncinfo *sync_info, hal_disp_syncinv *inv)
{
    volatile U_DHD0_CTRL dhd0_ctrl;
    volatile U_DHD0_VSYNC1 dhd0_vsync1;
    volatile U_DHD0_VSYNC2 dhd0_vsync2;
    volatile U_DHD0_HSYNC1 dhd0_hsync1;
    volatile U_DHD0_HSYNC2 dhd0_hync2;
    volatile U_DHD0_VPLUS1 dhd0_vplus1;
    volatile U_DHD0_VPLUS2 dhd0_vplus2;
    volatile U_DHD0_PWR dhd0_pwr;

    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_CTRL.u32));
            dhd0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_ctrl.bits.iop = sync_info->iop;
            dhd0_ctrl.bits.intf_ihs = inv->hs_inv;
            dhd0_ctrl.bits.intf_ivs = inv->vs_inv;
            dhd0_ctrl.bits.intf_idv = inv->dv_inv;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_ctrl.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_HSYNC1.u32));
            dhd0_hsync1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_hsync1.bits.hact = sync_info->hact - 1;
            dhd0_hsync1.bits.hbb = (sync_info->hbb) - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_hsync1.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_HSYNC2.u32));
            dhd0_hync2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_hync2.bits.hmid = (sync_info->hmid == 0) ? 0 : sync_info->hmid - 1;
            dhd0_hync2.bits.hfb = (sync_info->hfb) - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_hync2.u32);

            /* config VHD interface vertical timing */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VSYNC1.u32));
            dhd0_vsync1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_vsync1.bits.vact = sync_info->vact - 1;
            dhd0_vsync1.bits.vbb = sync_info->vbb - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_vsync1.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VSYNC2.u32));
            dhd0_vsync2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_vsync2.bits.vfb = sync_info->vfb - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_vsync2.u32);

            /* config VHD interface vertical bottom timing,no use in progressive mode */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VPLUS1.u32));
            dhd0_vplus1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_vplus1.bits.bvact = sync_info->bvact - 1;
            dhd0_vplus1.bits.bvbb = sync_info->bvbb - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_vplus1.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VPLUS2.u32));
            dhd0_vplus2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_vplus2.bits.bvfb = sync_info->bvfb - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_vplus2.u32);

            /* config VHD interface vertical bottom timing, */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_PWR.u32));
            dhd0_pwr.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_pwr.bits.hpw = sync_info->hpw - 1;
            dhd0_pwr.bits.vpw = sync_info->vpw - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_pwr.u32);

            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dev_multi_chn_en(hal_disp_outputchannel chn, hal_multichn_en multi_chn_en)
{
    volatile U_DHD0_PWR dhd0_pwr;
    volatile hi_ulong addr_reg;
    addr_reg = vou_get_chn_abs_addr(chn, (hi_uintptr_t) & (g_vo_reg->DHD0_PWR.u32));
    dhd0_pwr.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
    dhd0_pwr.bits.multichn_en = multi_chn_en;
    hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_pwr.u32);

    return HI_TRUE;
}

hi_bool hal_disp_set_lcd_serial_perd(hi_u32 serial_perd)
{
    U_INTF_LCD_CTRL intf_lcd_ctrl;
    volatile hi_ulong addr_reg;

    addr_reg = vou_get_intf_abs_addr(HAL_DISP_INTF_LCD_8BIT, (hi_uintptr_t) & (g_vo_reg->INTF_HDMI_CTRL.u32));
    intf_lcd_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);

    intf_lcd_ctrl.bits.lcd_serial_perd = serial_perd;

    hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf_lcd_ctrl.u32);
    return HI_TRUE;
}

hi_bool hal_disp_set_intf_ctrl(hal_disp_intf intf, hi_u32 *ctrl_info)
{
    U_INTF_HDMI_CTRL intf_hdmi_ctrl;
    U_INTF_BT_CTRL intf_bt_ctrl;
    U_INTF_LCD_CTRL intf_lcd_ctrl;
    volatile hi_ulong addr_reg;
    U_INTF_HDMI_CTRL *hdmi_ctrl = HI_NULL;
    U_INTF_BT_CTRL *bt_ctrl = HI_NULL;
    U_INTF_LCD_CTRL *lcd_ctrl = HI_NULL;

    if ((intf == VO_INTF_BT1120) ||
        (intf == VO_INTF_BT656)) {
        bt_ctrl = (U_INTF_BT_CTRL *)ctrl_info;
        addr_reg = vou_get_intf_abs_addr(intf, (hi_uintptr_t) & (g_vo_reg->INTF_HDMI_CTRL.u32));
        intf_bt_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        intf_bt_ctrl.bits.hdmi_mode = bt_ctrl->bits.hdmi_mode;
        intf_bt_ctrl.bits.lcd_serial_mode = bt_ctrl->bits.lcd_serial_mode;
        intf_bt_ctrl.bits.lcd_parallel_order = bt_ctrl->bits.lcd_parallel_order;
        intf_bt_ctrl.bits.lcd_data_inv = bt_ctrl->bits.lcd_data_inv;
        intf_bt_ctrl.bits.lcd_parallel_mode = bt_ctrl->bits.lcd_parallel_mode;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf_bt_ctrl.u32);
    } else if ((intf == VO_INTF_LCD) ||
               (intf == VO_INTF_LCD_6BIT) ||
               (intf == VO_INTF_LCD_8BIT) ||
               (intf == VO_INTF_LCD_16BIT) ||
               (intf == VO_INTF_LCD_18BIT) ||
               (intf == VO_INTF_LCD_24BIT)) {
        lcd_ctrl = (U_INTF_LCD_CTRL *)ctrl_info;
        addr_reg = vou_get_intf_abs_addr(intf, (hi_uintptr_t) & (g_vo_reg->INTF_HDMI_CTRL.u32));
        intf_lcd_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        intf_lcd_ctrl.bits.hdmi_mode = lcd_ctrl->bits.hdmi_mode;
        intf_lcd_ctrl.bits.lcd_serial_mode = lcd_ctrl->bits.lcd_serial_mode;
        intf_lcd_ctrl.bits.lcd_parallel_order = lcd_ctrl->bits.lcd_parallel_order;
        intf_lcd_ctrl.bits.lcd_data_inv = lcd_ctrl->bits.lcd_data_inv;
        intf_lcd_ctrl.bits.lcd_parallel_mode = lcd_ctrl->bits.lcd_parallel_mode;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf_lcd_ctrl.u32);
    } else if ((intf == VO_INTF_HDMI) ||
               (intf == VO_INTF_MIPI) ||
               (intf == VO_INTF_MIPI_SLAVE)) {
        hdmi_ctrl = (U_INTF_HDMI_CTRL *)ctrl_info;
        addr_reg = vou_get_hdmi_or_mipi_intf_abs_addr(intf, (hi_uintptr_t) & (g_vo_reg->INTF_HDMI_CTRL.u32));
        intf_hdmi_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        intf_hdmi_ctrl.bits.hdmi_mode = hdmi_ctrl->bits.hdmi_mode;
        intf_hdmi_ctrl.bits.intf_420_mode = hdmi_ctrl->bits.intf_420_mode;
        intf_hdmi_ctrl.bits.intf_420_en = hdmi_ctrl->bits.intf_420_en;
        intf_hdmi_ctrl.bits.intf_422_en = hdmi_ctrl->bits.intf_422_en;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf_hdmi_ctrl.u32);
    } else {
        return HI_FALSE;
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_intf_mux_sel(hal_disp_outputchannel chan, hal_disp_intf intf)
{
    volatile U_VO_MUX vo_mux;
    volatile hi_ulong addr_reg;

    hi_unused(chan);

    addr_reg = (hi_ulong)(hi_uintptr_t) & (g_vo_reg->VO_MUX.u32);
    vo_mux.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
    switch (intf) {
        case HAL_DISP_INTF_BT1120: {
            vo_mux.bits.digital_sel = 0;
            vo_mux.bits.bt_sel = 0;
            break;
        }

        case HAL_DISP_INTF_BT656: {
            vo_mux.bits.digital_sel = 1;
            vo_mux.bits.bt_sel = 0;
            break;
        }

        case HAL_DISP_INTF_MIPI:
        case HAL_DISP_INTF_MIPI_SLAVE: {
            vo_mux.bits.digital_sel = 0;
            vo_mux.bits.mipi_sel = 0;
            break;
        }
        case HAL_DISP_INTF_LCD:
        case HAL_DISP_INTF_LCD_6BIT:
        case HAL_DISP_INTF_LCD_8BIT:
        case HAL_DISP_INTF_LCD_16BIT:
        case HAL_DISP_INTF_LCD_18BIT:
        case HAL_DISP_INTF_LCD_24BIT: {
            vo_mux.bits.digital_sel = 2;
            vo_mux.bits.lcd_sel = 0;
            break;
        }

        default: {
            vo_mux.bits.digital_sel = 15;
            break;
        }
    }

    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VO_MUX.u32), vo_mux.u32);

    return HI_TRUE;
}

hi_bool hal_disp_set_intf_clip(hal_disp_intf intf, hi_bool clip, hal_disp_clip *clip_data)
{
    volatile U_BT_CLIP0_L bt_clip0_l;
    volatile U_BT_CLIP0_H bt_clip0_h;
    volatile hi_ulong addr_reg;

    hi_unused(intf);

    addr_reg = (hi_ulong)(hi_uintptr_t) & (g_vo_reg->BT_CLIP0_L.u32);
    bt_clip0_l.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
    bt_clip0_l.bits.clip_en = clip;
    bt_clip0_l.bits.clip_cl2 = clip_data->clip_low_y;
    bt_clip0_l.bits.clip_cl1 = clip_data->clip_low_cb;
    bt_clip0_l.bits.clip_cl0 = clip_data->clip_low_cr;
    hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, bt_clip0_l.u32);

    addr_reg = (hi_ulong)(hi_uintptr_t) & (g_vo_reg->BT_CLIP0_H.u32);
    bt_clip0_h.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
    bt_clip0_h.bits.clip_ch2 = clip_data->clip_high_y;
    bt_clip0_h.bits.clip_ch1 = clip_data->clip_high_cb;
    bt_clip0_h.bits.clip_ch0 = clip_data->clip_high_cr;
    hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, bt_clip0_h.u32);

    return HI_TRUE;
}

hi_bool hal_disp_set_vt_thd_mode(hal_disp_outputchannel chan, hi_u32 field_mode)
{
    volatile U_DHD0_VTTHD dhd0_vtthd;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VTTHD.u32));
            dhd0_vtthd.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_vtthd.bits.thd1_mode = field_mode;
            dhd0_vtthd.bits.thd2_mode = field_mode;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_vtthd.u32);
            break;
        }
        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_get_vtthd_mode(hal_disp_outputchannel chan, hi_bool *field_mode)
{
    volatile U_DHD0_VTTHD dhd0_vtthd;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VTTHD.u32));
            dhd0_vtthd.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            *field_mode = dhd0_vtthd.bits.thd1_mode;
            break;
        }
        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_vt_thd(hal_disp_outputchannel chan, hi_u32 vtthd, hi_u32 vtthd2)
{
    volatile U_DHD0_VTTHD dhd0_vtthd;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0: {
            /* for vtmgthd1 and vtmgthd2 */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_VTTHD.u32));
            dhd0_vtthd.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            dhd0_vtthd.bits.vtmgthd1 = vtthd;
            dhd0_vtthd.bits.vtmgthd2 = dhd0_vtthd.bits.vtmgthd1 - 1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_vtthd.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}
hi_bool hal_disp_set_int_mask(hi_u32 mask_en)
{
    volatile U_VOINTMSK vo_intmsk;
    /* display interrupt mask enable */
    vo_intmsk.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK.u32));
    vo_intmsk.u32 = vo_intmsk.u32 | mask_en;
    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK.u32), vo_intmsk.u32);

    return HI_TRUE;
}

hi_bool hal_disp_clr_int_mask(hi_u32 mask_en)
{
    volatile U_VOINTMSK vo_intmsk;

    /* display interrupt mask enable */
    vo_intmsk.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK.u32));
    vo_intmsk.u32 = vo_intmsk.u32 & (~mask_en);
    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK.u32), vo_intmsk.u32);

    return HI_TRUE;
}

hi_bool hal_disp_set_int_mask1(hi_u32 mask_en)
{
    volatile U_VOINTMSK1 vo_intmsk1;
    /* display interrupt mask enable */
    vo_intmsk1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK1.u32));
    vo_intmsk1.u32 = vo_intmsk1.u32 | mask_en;

    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK1.u32), vo_intmsk1.u32);
    vo_intmsk1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK1.u32));

    return HI_TRUE;
}

hi_bool hal_disp_clr_int_mask1(hi_u32 mask_en)
{
    volatile U_VOINTMSK1 vo_intmsk1;

    /* display interrupt mask enable */
    vo_intmsk1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK1.u32));
    vo_intmsk1.u32 = vo_intmsk1.u32 & (~mask_en);
    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTMSK1.u32), vo_intmsk1.u32);

    return HI_TRUE;
}

hi_u32 hal_disp_get_int_status(hi_u32 int_msk)
{
    volatile U_VOMSKINTSTA vo_msk_int_sta;

    /* read interrupt status */
    vo_msk_int_sta.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOMSKINTSTA.u32));

    return (vo_msk_int_sta.u32 & int_msk);
}

hi_u32 hal_disp_get_ori_int_status(hi_u32 int_status)
{
    volatile U_VOINTSTA vo_int_sta;

    /* read original interrupt status */
    vo_int_sta.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOINTSTA.u32));

    return (vo_int_sta.u32 & int_status);
}

hi_bool hal_disp_clear_int_status(hi_u32 int_msk)
{
    /* read interrupt status */
    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->VOMSKINTSTA.u32), int_msk);
    return HI_TRUE;
}

hi_bool hal_intf_bt_set_dfir_en(hi_u32 dfir_en)
{
    U_INTF_BT_CTRL intf_bt_ctrl;

    intf_bt_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->INTF_BT_CTRL.u32));
    intf_bt_ctrl.bits.dfir_en = dfir_en;
    hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->INTF_BT_CTRL.u32), intf_bt_ctrl.u32);

    return HI_TRUE;
}

hi_void hal_disp_set_reg_up(hal_disp_outputchannel chan)
{
    volatile U_DHD0_CTRL dhd0_ctrl;
    volatile hi_ulong addr_reg;

    if (chan > HAL_DISP_CHANNEL_DHD1) {
        HI_PRINT("error,hal_disp_set_reg_up select wrong CHANNEL ID\n");
        return;
    }

    addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_CTRL.u32));
    dhd0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
    dhd0_ctrl.bits.regup = 0x1;
    hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, dhd0_ctrl.u32);
    return;
}

hi_u32 hal_disp_get_reg_up(hal_disp_outputchannel chan)
{
    hi_u32 data;
    volatile U_DHD0_CTRL dhd0_ctrl;
    volatile hi_ulong addr_reg;

    addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->DHD0_CTRL.u32));
    dhd0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
    data = dhd0_ctrl.bits.regup;
    return data & 0x1;
}

hi_bool hal_disp_set_dither_round_unlim(hal_disp_outputchannel chan, hi_u32 dither_round_unlim)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.dither_round_unlim = dither_round_unlim;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_data_in_out(hal_disp_outputchannel chan, hi_u32 i_data_width_dither,
                                        hi_u32 o_data_width_dither)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.i_data_width_dither = i_data_width_dither;
            intf0_dither_ctrl.bits.o_data_width_dither = o_data_width_dither;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_en(hal_disp_outputchannel chan, hi_u32 dither_en)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.dither_en = dither_en;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_mode(hal_disp_outputchannel chan, hi_u32 mode)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.dither_mode = mode;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_round(hal_disp_outputchannel chan, hi_u32 dither_round)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.dither_round = dither_round;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_domain_mode(hal_disp_outputchannel chan, hi_u32 domain_mode)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.dither_domain_mode = domain_mode;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_tap_mode(hal_disp_outputchannel chan, hi_u32 tap_mode)
{
    volatile U_INTF0_DITHER_CTRL intf0_dither_ctrl;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_CTRL.u32));
            intf0_dither_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_ctrl.bits.dither_tap_mode = tap_mode;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_sed(hal_disp_outputchannel chan, hal_disp_dihter_sed *dither_sed)
{
    volatile U_INTF0_DITHER_SED_Y0 intf0_dither_sed_y0;
    volatile U_INTF0_DITHER_SED_U0 intf0_dither_sed_u0;
    volatile U_INTF0_DITHER_SED_V0 intf0_dither_sed_v0;
    volatile U_INTF0_DITHER_SED_W0 intf0_dither_sed_w0;

    volatile U_INTF0_DITHER_SED_Y1 intf0_dither_sed_y1;
    volatile U_INTF0_DITHER_SED_U1 intf0_dither_sed_u1;
    volatile U_INTF0_DITHER_SED_V1 intf0_dither_sed_v1;
    volatile U_INTF0_DITHER_SED_W1 intf0_dither_sed_w1;

    volatile U_INTF0_DITHER_SED_Y2 intf0_dither_sed_y2;
    volatile U_INTF0_DITHER_SED_U2 intf0_dither_sed_u2;
    volatile U_INTF0_DITHER_SED_V2 intf0_dither_sed_v2;
    volatile U_INTF0_DITHER_SED_W2 intf0_dither_sed_w2;

    volatile U_INTF0_DITHER_SED_Y3 intf0_dither_sed_y3;
    volatile U_INTF0_DITHER_SED_U3 intf0_dither_sed_u3;
    volatile U_INTF0_DITHER_SED_V3 intf0_dither_sed_v3;
    volatile U_INTF0_DITHER_SED_W3 intf0_dither_sed_w3;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            /* set sed_y0,sed_u0,sed_v0,sed_w0 */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_Y0.u32));
            intf0_dither_sed_y0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_y0.bits.dither_sed_y0 = dither_sed->dither_sed_y0;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_y0.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_U0.u32));
            intf0_dither_sed_u0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_u0.bits.dither_sed_u0 = dither_sed->dither_sed_u0;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_u0.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_V0.u32));
            intf0_dither_sed_v0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_v0.bits.dither_sed_v0 = dither_sed->dither_sed_v0;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_v0.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_W0.u32));
            intf0_dither_sed_w0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_w0.bits.dither_sed_w0 = dither_sed->dither_sed_w0;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_w0.u32);

            /* set sed_y1,sed_u1,sed_v1,sed_w1 */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_Y1.u32));
            intf0_dither_sed_y1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_y1.bits.dither_sed_y1 = dither_sed->dither_sed_y1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_y1.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_U1.u32));
            intf0_dither_sed_u1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_u1.bits.dither_sed_u1 = dither_sed->dither_sed_u1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_u1.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_V1.u32));
            intf0_dither_sed_v1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_v1.bits.dither_sed_v1 = dither_sed->dither_sed_v1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_v1.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_W1.u32));
            intf0_dither_sed_w1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_w1.bits.dither_sed_w1 = dither_sed->dither_sed_w1;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_w1.u32);

            /* set sed_y2,sed_u2,sed_v2,sed_w2 */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_Y2.u32));
            intf0_dither_sed_y2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_y2.bits.dither_sed_y2 = dither_sed->dither_sed_y2;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_y2.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_U2.u32));
            intf0_dither_sed_u2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_u2.bits.dither_sed_u2 = dither_sed->dither_sed_u2;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_u2.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_V2.u32));
            intf0_dither_sed_v2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_v2.bits.dither_sed_v2 = dither_sed->dither_sed_v2;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_v2.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_W2.u32));
            intf0_dither_sed_w2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_w2.bits.dither_sed_w2 = dither_sed->dither_sed_w2;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_w2.u32);

            /* set sed_y3,sed_u3,sed_v3,sed_w3 */
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_Y3.u32));
            intf0_dither_sed_y3.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_y3.bits.dither_sed_y3 = dither_sed->dither_sed_y3;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_y3.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_U3.u32));
            intf0_dither_sed_u3.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_u3.bits.dither_sed_u3 = dither_sed->dither_sed_u3;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_u3.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_V3.u32));
            intf0_dither_sed_v3.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_v3.bits.dither_sed_v3 = dither_sed->dither_sed_v3;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_v3.u32);

            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_SED_W3.u32));
            intf0_dither_sed_w3.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_sed_w3.bits.dither_sed_w3 = dither_sed->dither_sed_w3;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_sed_w3.u32);

            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_disp_set_dither_thr_min_max(hal_disp_outputchannel chan, hi_u32 thr_min, hi_u32 thr_max)
{
    volatile U_INTF0_DITHER_THR intf0_dither_thr;
    volatile hi_ulong addr_reg;

    switch (chan) {
        case HAL_DISP_CHANNEL_DHD0:
        case HAL_DISP_CHANNEL_DHD1: {
            addr_reg = vou_get_chn_abs_addr(chan, (hi_uintptr_t) & (g_vo_reg->INTF0_DITHER_THR.u32));
            intf0_dither_thr.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            intf0_dither_thr.bits.dither_thr_min = thr_min;
            intf0_dither_thr.bits.dither_thr_max = thr_max;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, intf0_dither_thr.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_video_set_layer_up_mode(hal_disp_layer layer, hi_u32 up_mode)
{
    U_V0_CTRL v0_ctrl;
    volatile hi_ulong addr_reg;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CTRL.u32));
            v0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            v0_ctrl.bits.rgup_mode = up_mode;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

/* v0_alpha */
hi_bool hal_video_set_layer_alpha(hal_disp_layer layer, hi_u32 arange)
{
    volatile U_V0_ALPHA v0_alpha;
    volatile hi_ulong addr_reg;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_ALPHA.u32));
            v0_alpha.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            v0_alpha.bits.vbk_alpha = arange;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_alpha.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_layer_enable_layer(hal_disp_layer layer, hi_u32 enable)
{
    volatile U_V0_CTRL v0_ctrl;
    volatile U_G0_CTRL G0_CTRL;
    volatile hi_ulong addr_reg;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CTRL.u32));
            v0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            v0_ctrl.bits.surface_en = enable;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_ctrl.u32);
            break;
        }

        case HAL_DISP_LAYER_GFX0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_CTRL.u32));
            G0_CTRL.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            G0_CTRL.bits.surface_en = enable;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, G0_CTRL.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_layer_get_layer_enable(hal_disp_layer layer, hi_bool *enable)
{
    volatile U_V0_CTRL v0_ctrl;
    volatile U_G0_CTRL G0_CTRL;

    volatile hi_ulong addr_reg;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CTRL.u32));
            v0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            *enable = v0_ctrl.bits.surface_en;
            break;
        }

        case HAL_DISP_LAYER_GFX0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_CTRL.u32));
            G0_CTRL.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            *enable = G0_CTRL.bits.surface_en;
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_layer_set_layer_data_fmt(hal_disp_layer layer,
                                     hal_disp_pixel_format data_fmt)
{
    volatile U_VID_SRC_INFO VID_SRC_INFO;
    volatile U_GFX_SRC_INFO GFX_SRC_INFO;
    volatile hi_ulong addr_reg;

    /* only for v0 and v1 */
    if (layer == HAL_DISP_LAYER_VHD0) {
        addr_reg = vou_get_vid_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->VID_SRC_INFO.u32));
        VID_SRC_INFO.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        VID_SRC_INFO.bits.data_type = data_fmt;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, VID_SRC_INFO.u32);
    } else if (layer == HAL_DISP_LAYER_GFX0) {
        addr_reg = vou_get_gfx_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->GFX_SRC_INFO.u32));
        GFX_SRC_INFO.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        GFX_SRC_INFO.bits.ifmt = data_fmt;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, GFX_SRC_INFO.u32);
    } else {
        return HI_FALSE;
    }

    return HI_TRUE;
}

hi_bool hal_layer_get_layer_data_fmt(hal_disp_layer layer, hi_u32 *fmt)
{
    volatile U_VID_SRC_INFO VID_SRC_INFO;
    volatile hi_ulong addr_reg;

    if (layer == HAL_DISP_LAYER_VHD0) {
        addr_reg = vou_get_vid_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->VID_SRC_INFO.u32));
        VID_SRC_INFO.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        *fmt = VID_SRC_INFO.bits.data_type;
    } else {
        return HI_FALSE;
    }

    return HI_TRUE;
}

hi_void hal_video_cvfir_set_coef(hal_disp_layer layer, cvfir_coef *coef)
{
    U_V0_CVFIR_VCOEF0 v0_cvfir_vcoef0;
    U_V0_CVFIR_VCOEF1 v0_cvfir_vcoef1;
    U_V0_CVFIR_VCOEF2 v0_cvfir_vcoef2;
    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CVFIR_VCOEF0.u32));
        v0_cvfir_vcoef0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_cvfir_vcoef0.bits.vccoef00 = coef->vccoef00;
        v0_cvfir_vcoef0.bits.vccoef01 = coef->vccoef01;
        v0_cvfir_vcoef0.bits.vccoef02 = coef->vccoef02;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_cvfir_vcoef0.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CVFIR_VCOEF1.u32));
        v0_cvfir_vcoef1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_cvfir_vcoef1.bits.vccoef03 = coef->vccoef03;
        v0_cvfir_vcoef1.bits.vccoef10 = coef->vccoef10;
        v0_cvfir_vcoef1.bits.vccoef11 = coef->vccoef11;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_cvfir_vcoef1.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CVFIR_VCOEF2.u32));
        v0_cvfir_vcoef2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_cvfir_vcoef2.bits.vccoef12 = coef->vccoef12;
        v0_cvfir_vcoef2.bits.vccoef13 = coef->vccoef13;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_cvfir_vcoef2.u32);
    }

    return;
}

hi_void hal_video_hfir_set_ck_gt_en(hal_disp_layer layer, hi_u32 ck_gt_en)
{
    U_V0_HFIR_CTRL v0_hfir_ctrl;
    volatile hi_ulong addr_reg;

    hi_unused(ck_gt_en);

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIR_CTRL.u32));
        v0_hfir_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_ctrl.bits.ck_gt_en = 1;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_ctrl.u32);
    }

    return;
}

hi_void hal_video_hfir_set_mid_en(hal_disp_layer layer, hi_u32 mid_en)
{
    U_V0_HFIR_CTRL v0_hfir_ctrl;
    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIR_CTRL.u32));
        v0_hfir_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_ctrl.bits.mid_en = mid_en;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_ctrl.u32);
    }

    return;
}

hi_void hal_video_hfir_set_hfir_mode(hal_disp_layer layer, hi_u32 hfir_mode)
{
    U_V0_HFIR_CTRL v0_hfir_ctrl;
    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIR_CTRL.u32));
        v0_hfir_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_ctrl.bits.hfir_mode = hfir_mode;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_ctrl.u32);
    }

    return;
}

hi_void hal_video_hfir_set_coef(hal_disp_layer layer, hfir_coef *coef)
{
    U_V0_HFIRCOEF01 v0_hfir_coef01;
    U_V0_HFIRCOEF23 v0_hfir_coef23;
    U_V0_HFIRCOEF45 v0_hfir_coef45;
    U_V0_HFIRCOEF67 v0_hfir_coef67;
    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIRCOEF01.u32));
        v0_hfir_coef01.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_coef01.bits.coef0 = coef->coef0;
        v0_hfir_coef01.bits.coef1 = coef->coef1;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_coef01.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIRCOEF23.u32));
        v0_hfir_coef23.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_coef23.bits.coef2 = coef->coef2;
        v0_hfir_coef23.bits.coef3 = coef->coef3;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_coef23.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIRCOEF45.u32));
        v0_hfir_coef45.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_coef45.bits.coef4 = coef->coef4;
        v0_hfir_coef45.bits.coef5 = coef->coef5;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_coef45.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HFIRCOEF67.u32));
        v0_hfir_coef67.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hfir_coef67.bits.coef6 = coef->coef6;
        v0_hfir_coef67.bits.coef7 = coef->coef7;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hfir_coef67.u32);
    }

    return;
}

hi_void hal_layer_csc_set_enable(hal_disp_layer layer, hi_bool csc_en)
{
    U_V0_HIPP_CSC_CTRL v0_hipp_csc_ctrl;

    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_CTRL.u32));
        v0_hipp_csc_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hipp_csc_ctrl.bits.hipp_csc_en = csc_en;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hipp_csc_ctrl.u32);
    } else if (layer >= HAL_DISP_LAYER_GFX0 && layer <= HAL_DISP_LAYER_GFX1) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_CTRL.u32));
        v0_hipp_csc_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hipp_csc_ctrl.bits.hipp_csc_en = csc_en;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hipp_csc_ctrl.u32);
    }
}

hi_void hal_layer_csc_set_ck_gt_en(hal_disp_layer layer, hi_bool ck_gt_en)
{
    U_V0_HIPP_CSC_CTRL v0_hipp_csc_ctrl;

    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_CTRL.u32));
        v0_hipp_csc_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hipp_csc_ctrl.bits.hipp_csc_ck_gt_en = ck_gt_en;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hipp_csc_ctrl.u32);
    } else if (layer >= HAL_DISP_LAYER_GFX0 && layer <= HAL_DISP_LAYER_GFX1) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_CTRL.u32));
        v0_hipp_csc_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        v0_hipp_csc_ctrl.bits.hipp_csc_ck_gt_en = ck_gt_en;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_hipp_csc_ctrl.u32);
    }
}

hi_void hal_layer_csc_set_coef(hal_disp_layer layer, vdp_csc_coef *coef)
{
    U_V0_HIPP_CSC_COEF00 V0_HIPP_CSC_COEF00;
    U_V0_HIPP_CSC_COEF01 V0_HIPP_CSC_COEF01;
    U_V0_HIPP_CSC_COEF02 V0_HIPP_CSC_COEF02;
    U_V0_HIPP_CSC_COEF10 V0_HIPP_CSC_COEF10;
    U_V0_HIPP_CSC_COEF11 V0_HIPP_CSC_COEF11;
    U_V0_HIPP_CSC_COEF12 V0_HIPP_CSC_COEF12;
    U_V0_HIPP_CSC_COEF20 V0_HIPP_CSC_COEF20;
    U_V0_HIPP_CSC_COEF21 V0_HIPP_CSC_COEF21;
    U_V0_HIPP_CSC_COEF22 V0_HIPP_CSC_COEF22;

    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF00.u32));
        V0_HIPP_CSC_COEF00.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF00.bits.hipp_csc_coef00 = coef->csc_coef00;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF00.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF01.u32));
        V0_HIPP_CSC_COEF01.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF01.bits.hipp_csc_coef01 = coef->csc_coef01;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF01.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF02.u32));
        V0_HIPP_CSC_COEF02.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF02.bits.hipp_csc_coef02 = coef->csc_coef02;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF02.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF10.u32));
        V0_HIPP_CSC_COEF10.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF10.bits.hipp_csc_coef10 = coef->csc_coef10;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF10.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF11.u32));
        V0_HIPP_CSC_COEF11.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF11.bits.hipp_csc_coef11 = coef->csc_coef11;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF11.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF12.u32));
        V0_HIPP_CSC_COEF12.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF12.bits.hipp_csc_coef12 = coef->csc_coef12;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF12.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF20.u32));
        V0_HIPP_CSC_COEF20.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF20.bits.hipp_csc_coef20 = coef->csc_coef20;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF20.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF21.u32));
        V0_HIPP_CSC_COEF21.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF21.bits.hipp_csc_coef21 = coef->csc_coef21;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF21.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_COEF22.u32));
        V0_HIPP_CSC_COEF22.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF22.bits.hipp_csc_coef22 = coef->csc_coef22;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF22.u32);
    } else if (layer >= HAL_DISP_LAYER_GFX0 && layer <= HAL_DISP_LAYER_GFX1) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF00.u32));
        V0_HIPP_CSC_COEF00.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF00.bits.hipp_csc_coef00 = coef->csc_coef00;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF00.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF01.u32));
        V0_HIPP_CSC_COEF01.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF01.bits.hipp_csc_coef01 = coef->csc_coef01;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF01.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF02.u32));
        V0_HIPP_CSC_COEF02.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF02.bits.hipp_csc_coef02 = coef->csc_coef02;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF02.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF10.u32));
        V0_HIPP_CSC_COEF10.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF10.bits.hipp_csc_coef10 = coef->csc_coef10;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF10.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF11.u32));
        V0_HIPP_CSC_COEF11.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF11.bits.hipp_csc_coef11 = coef->csc_coef11;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF11.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF12.u32));
        V0_HIPP_CSC_COEF12.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF12.bits.hipp_csc_coef12 = coef->csc_coef12;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF12.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF20.u32));
        V0_HIPP_CSC_COEF20.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF20.bits.hipp_csc_coef20 = coef->csc_coef20;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF20.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF21.u32));
        V0_HIPP_CSC_COEF21.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF21.bits.hipp_csc_coef21 = coef->csc_coef21;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF21.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_COEF22.u32));
        V0_HIPP_CSC_COEF22.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_COEF22.bits.hipp_csc_coef22 = coef->csc_coef22;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_COEF22.u32);
    } else {
    }

    return;
}

hi_void hal_layer_csc_set_dc_coef(hal_disp_layer layer, vdp_csc_dc_coef *csc_dc_coef)
{
    U_V0_HIPP_CSC_IDC0 V0_HIPP_CSC_IDC0;
    U_V0_HIPP_CSC_IDC1 V0_HIPP_CSC_IDC1;
    U_V0_HIPP_CSC_IDC2 V0_HIPP_CSC_IDC2;
    U_V0_HIPP_CSC_ODC0 V0_HIPP_CSC_ODC0;
    U_V0_HIPP_CSC_ODC1 V0_HIPP_CSC_ODC1;
    U_V0_HIPP_CSC_ODC2 V0_HIPP_CSC_ODC2;

    volatile hi_ulong addr_reg;

    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_IDC0.u32));
        V0_HIPP_CSC_IDC0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_IDC0.bits.hipp_csc_idc0 = csc_dc_coef->csc_in_dc0;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_IDC0.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_IDC1.u32));
        V0_HIPP_CSC_IDC1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_IDC1.bits.hipp_csc_idc1 = csc_dc_coef->csc_in_dc1;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_IDC1.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_IDC2.u32));
        V0_HIPP_CSC_IDC2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_IDC2.bits.hipp_csc_idc2 = csc_dc_coef->csc_in_dc2;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_IDC2.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_ODC0.u32));
        V0_HIPP_CSC_ODC0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_ODC0.bits.hipp_csc_odc0 = csc_dc_coef->csc_out_dc0;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_ODC0.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_ODC1.u32));
        V0_HIPP_CSC_ODC1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_ODC1.bits.hipp_csc_odc1 = csc_dc_coef->csc_out_dc1;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_ODC1.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_ODC2.u32));
        V0_HIPP_CSC_ODC2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_ODC2.bits.hipp_csc_odc2 = csc_dc_coef->csc_out_dc2;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_ODC2.u32);
    } else if (layer >= HAL_DISP_LAYER_GFX0 && layer <= HAL_DISP_LAYER_GFX1) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_IDC0.u32));
        V0_HIPP_CSC_IDC0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_IDC0.bits.hipp_csc_idc0 = csc_dc_coef->csc_in_dc0;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_IDC0.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_IDC1.u32));
        V0_HIPP_CSC_IDC1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_IDC1.bits.hipp_csc_idc1 = csc_dc_coef->csc_in_dc1;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_IDC1.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_IDC2.u32));
        V0_HIPP_CSC_IDC2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_IDC2.bits.hipp_csc_idc2 = csc_dc_coef->csc_in_dc2;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_IDC2.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_ODC0.u32));
        V0_HIPP_CSC_ODC0.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_ODC0.bits.hipp_csc_odc0 = csc_dc_coef->csc_out_dc0;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_ODC0.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_ODC1.u32));
        V0_HIPP_CSC_ODC1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_ODC1.bits.hipp_csc_odc1 = csc_dc_coef->csc_out_dc1;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_ODC1.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_ODC2.u32));
        V0_HIPP_CSC_ODC2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_ODC2.bits.hipp_csc_odc2 = csc_dc_coef->csc_out_dc2;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_ODC2.u32);
    } else {
    }

    return;
}

hi_void hal_layer_csc_set_param(hal_disp_layer layer, csc_coef_param *csc_param)
{
    U_V0_HIPP_CSC_SCALE V0_HIPP_CSC_SCALE;
    U_V0_HIPP_CSC_MIN_Y V0_HIPP_CSC_MIN_Y;
    U_V0_HIPP_CSC_MIN_C V0_HIPP_CSC_MIN_C;
    U_V0_HIPP_CSC_MAX_Y V0_HIPP_CSC_MAX_Y;
    U_V0_HIPP_CSC_MAX_C V0_HIPP_CSC_MAX_C;

    volatile hi_ulong addr_reg;
    if (layer >= LAYER_VHD_START && layer <= LAYER_VHD_END) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_SCALE.u32));
        V0_HIPP_CSC_SCALE.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_SCALE.bits.hipp_csc_scale = csc_param->csc_scale2p;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_SCALE.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_MIN_Y.u32));
        V0_HIPP_CSC_MIN_Y.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MIN_Y.bits.hipp_csc_min_y = csc_param->csc_clip_min;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MIN_Y.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_MIN_C.u32));
        V0_HIPP_CSC_MIN_C.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MIN_C.bits.hipp_csc_min_c = csc_param->csc_clip_min;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MIN_C.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_MAX_Y.u32));
        V0_HIPP_CSC_MAX_Y.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MAX_Y.bits.hipp_csc_max_y = csc_param->csc_clip_max;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MAX_Y.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_HIPP_CSC_MAX_C.u32));
        V0_HIPP_CSC_MAX_C.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MAX_C.bits.hipp_csc_max_c = csc_param->csc_clip_max;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MAX_C.u32);
    } else if (layer >= HAL_DISP_LAYER_GFX0 && layer <= HAL_DISP_LAYER_GFX1) {
        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_SCALE.u32));
        V0_HIPP_CSC_SCALE.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_SCALE.bits.hipp_csc_scale = csc_param->csc_scale2p;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_SCALE.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_MIN_Y.u32));
        V0_HIPP_CSC_MIN_Y.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MIN_Y.bits.hipp_csc_min_y = csc_param->csc_clip_min;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MIN_Y.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_MIN_C.u32));
        V0_HIPP_CSC_MIN_C.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MIN_C.bits.hipp_csc_min_c = csc_param->csc_clip_min;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MIN_C.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_MAX_Y.u32));
        V0_HIPP_CSC_MAX_Y.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MAX_Y.bits.hipp_csc_max_y = csc_param->csc_clip_max;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MAX_Y.u32);

        addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->G0_HIPP_CSC_MAX_C.u32));
        V0_HIPP_CSC_MAX_C.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
        V0_HIPP_CSC_MAX_C.bits.hipp_csc_max_c = csc_param->csc_clip_max;
        hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, V0_HIPP_CSC_MAX_C.u32);
    }
}

hi_bool hal_layer_set_csc_coef(hal_disp_layer layer, csc_coef *coef)
{
    if ((layer < HAL_DISP_LAYER_VHD0) || (layer > HAL_DISP_LAYER_GFX1)) {
        return HI_FALSE;
    }

    hal_layer_csc_set_dc_coef(layer, (vdp_csc_dc_coef *)(&coef->csc_in_dc0));
    hal_layer_csc_set_coef(layer, (vdp_csc_coef *)(&coef->csc_coef00));
    hal_layer_csc_set_param(layer, (csc_coef_param *)(&coef->new_csc_scale2p));

    return HI_TRUE;
}

hi_bool hal_layer_set_csc_en(hal_disp_layer layer, hi_bool csc_en)
{
    if ((layer < HAL_DISP_LAYER_VHD0) || (layer > HAL_DISP_LAYER_GFX1)) {
        return HI_FALSE;
    }

    hal_layer_csc_set_ck_gt_en(layer, csc_en);
    hal_layer_csc_set_enable(layer, csc_en);

    return HI_TRUE;
}

hi_bool hal_layer_set_layer_g_alpha(hal_disp_layer layer,
                                    hi_u8 alpha0)
{
    volatile U_V0_CTRL v0_ctrl;
    volatile hi_ulong addr_reg;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0: {
            addr_reg = vou_get_abs_addr(layer, (hi_uintptr_t) & (g_vo_reg->V0_CTRL.u32));
            v0_ctrl.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)addr_reg);
            v0_ctrl.bits.galpha = alpha0;
            hal_write_reg((hi_u32 *)(hi_uintptr_t)addr_reg, v0_ctrl.u32);
            break;
        }

        default: {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

hi_bool hal_cbm_set_cbm_bkg(hi_u32 mixer_id, hal_disp_bkcolor *bkg)
{
    U_CBM_BKG1 CBM_BKG1;
    U_CBM_BKG2 CBM_BKG2;

    if (mixer_id == HAL_CBMMIX1) {
        CBM_BKG1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->CBM_BKG1.u32));
        CBM_BKG1.bits.cbm_bkgy1 = (bkg->bkg_y);
        CBM_BKG1.bits.cbm_bkgcb1 = (bkg->bkg_cb);
        CBM_BKG1.bits.cbm_bkgcr1 = (bkg->bkg_cr);
        hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->CBM_BKG1.u32), CBM_BKG1.u32);
    } else if (mixer_id == HAL_CBMMIX2) {
        CBM_BKG2.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->CBM_BKG2.u32));
        CBM_BKG2.bits.cbm_bkgy2 = (bkg->bkg_y);
        CBM_BKG2.bits.cbm_bkgcb2 = (bkg->bkg_cb);
        CBM_BKG2.bits.cbm_bkgcr2 = (bkg->bkg_cr);
        hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->CBM_BKG2.u32), CBM_BKG2.u32);
    } else {
        return HI_FALSE;
    }

    return HI_TRUE;
}

hi_bool hal_cbm_set_cbm_mixer_prio(hal_disp_layer layer, hi_u8 prio, hi_u8 mixer_id)
{
    U_CBM_MIX1 CBM_MIX1;
    hi_u8 layer_id = 0;

    /* settings for v0 and g0 */
    if (mixer_id == HAL_CBMMIX1) {
        /* check layer availability */
        switch (layer) {
            case HAL_DISP_LAYER_VHD0: {
                layer_id = 0x1;
                break;
            }
            case HAL_DISP_LAYER_GFX0: {
                layer_id = 0x2;
                break;
            }
            case HAL_DISP_INVALID_LAYER: {
                layer_id = 0x0;
                break;
            }
            default: {
                return HI_FALSE;
            }
        }

        /* set mixer prio */
        CBM_MIX1.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->CBM_MIX1.u32));

        switch (prio) {
            case VOU_MIX_PRIO0: {
                CBM_MIX1.bits.mixer_prio0 = layer_id;
                break;
            }

            case VOU_MIX_PRIO1: {
                CBM_MIX1.bits.mixer_prio1 = layer_id;
                break;
            }

            case VOU_MIX_PRIO2: {
                CBM_MIX1.bits.mixer_prio2 = layer_id;
                break;
            }

            case VOU_MIX_PRIO3: {
                CBM_MIX1.bits.mixer_prio3 = layer_id;
                break;
            }

            default: {
                return HI_FALSE;
            }
        }

        hal_write_reg((hi_u32 *)(hi_uintptr_t) & (g_vo_reg->CBM_MIX1.u32), CBM_MIX1.u32);
    } else {
        return HI_FALSE;
    }

    return HI_TRUE;
}

hi_void hal_fdr_vid_set_vid_out_uv_order(hi_u32 layer, hi_u32 uv_order_en)
{
    U_VID_OUT_CTRL VID_OUT_CTRL;

    VID_OUT_CTRL.u32 = hal_read_reg((hi_u32 *)(hi_uintptr_t)(&(g_vo_reg->VID_OUT_CTRL.u32) + layer * FDR_VID_OFFSET));
    VID_OUT_CTRL.bits.uv_order_en = uv_order_en;
    hal_write_reg((hi_u32 *)(hi_uintptr_t)(&(g_vo_reg->VID_OUT_CTRL.u32) + layer * FDR_VID_OFFSET), VID_OUT_CTRL.u32);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

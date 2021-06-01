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

#include "sys_drv.h"
#include "hi_osal.h"
#include "mkp_sys.h"
#include "sysconfig/sys_config.h"
#include "sys_hal.h"

#define PHASE                        32
#define COEFF_BUTT                   (PHASE / 2 + 1)
#define SYS_MULTI_V                  1048576
#define SYS_MULTI_H                  1048576

static hi_vi_vpss_mode g_vi_vpss_mode;

#ifndef arry_size
#define arry_size(x) (sizeof(x) / sizeof((x)[0]))
#endif

hi_void sys_drv_set_priority(hi_vi_vpss_mode *vi_vpss_mode)
{
#if (defined(CONFIG_AMP) && defined(CONFIG_HUAWEI_LITEOS_OS)) || defined(CONFIG_SMP)
    hi_u32 i;
    hi_u32 flag;

    for (i = 0; i < VIPROC_IP_NUM; i++) {
        if (vi_vpss_mode->mode[i] == VI_ONLINE_VPSS_ONLINE) {
            flag = 3; /* 3:VI_ONLINE_VPSS_ONLINE */
        } else if (vi_vpss_mode->mode[i] == VI_ONLINE_VPSS_OFFLINE) {
            flag = 2; /* 2:VI_ONLINE_VPSS_OFFLINE */
        } else if (vi_vpss_mode->mode[i] == VI_OFFLINE_VPSS_ONLINE) {
            flag = 1; /* 1:VI_OFFLINE_VPSS_ONLINE */
        } else {
            flag = 0;
        }
        set_vi_vpss_mode(flag);
    }
#endif
    return;
}

hi_void sysconfig_drv_set_vi_vpss_mode(const hi_s32 vi_vpss_mode)
{
    set_vi_vpss_mode(vi_vpss_mode);
}

hi_void sys_drv_vi_vpss_mode_init(hi_void)
{
    hi_s32 i;

    for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
        g_vi_vpss_mode.mode[i] = VI_OFFLINE_VPSS_OFFLINE;
    }

    sys_drv_set_priority(&g_vi_vpss_mode);

    return;
}

hi_s32 sys_drv_init(hi_void)
{
    hi_s32 ret;

    ret = sys_hal_init();
    if (ret != HI_SUCCESS) {
        return ret;
    }

    sys_drv_vi_vpss_mode_init();

    return ret;
}

hi_s32 sys_drv_exit(hi_void)
{
    sys_hal_exit();
    return HI_SUCCESS;
}

hi_s32 sys_drv_drv_ioctrl(hi_mpp_chn *mpp_chn, sys_func func_id, hi_void *io_args)
{
    hi_s32 dev;

    if ((mpp_chn == HI_NULL) || (io_args == HI_NULL)) {
        sys_emerg_trace("null point!\n");
        return HI_ERR_SYS_NULL_PTR;
    }

    dev = mpp_chn->dev_id;

    switch (func_id) {
        case SYS_VOU_BUS_RESET_SEL: {
            hi_bool *reset = HI_NULL;
            reset = (hi_bool *)io_args;
            sys_hal_vou_bus_reset_sel(*reset);
            break;
        }
        case SYS_VOU_APB_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_vou_apb_clk_en(dev, *clk_en);
            break;
        }
        case SYS_VOU_BUS_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_vou_bus_clk_en(*clk_en);
            break;
        }
        case SYS_VOU_CFG_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_vou_cfg_clk_en(dev, *clk_en);
            break;
        }
        case SYS_VOU_CORE_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_vou_core_clk_en(dev, *clk_en);
            break;
        }
        case SYS_VOU_DEV_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_vou_dev_clk_en(dev, *clk_en);
            break;
        }
        case SYS_VOU_OUT_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_vou_out_clk_en(dev, *clk_en);
            break;
        }
        case SYS_VO_BT1120_CLK_EN: {
            hi_bool *bt1120_clk_en = HI_NULL;
            bt1120_clk_en = (hi_bool *)io_args;
            sys_hal_vou_bt_clk_en(dev, *bt1120_clk_en);
            break;
        }
        case SYS_VO_HDMI_CLK_EN: {
            hi_bool *hdmi_clk_en = HI_NULL;
            hdmi_clk_en = (hi_bool *)io_args;
            sys_hal_vou_hdmi_clk_en(dev, *hdmi_clk_en);
            break;
        }
        case SYS_VO_MIPI_CLK_EN: {
            hi_bool *mipi_clk_en = HI_NULL;
            mipi_clk_en = (hi_bool *)io_args;
            sys_hal_vou_mipi_clk_en(dev, *mipi_clk_en);
            break;
        }
        case SYS_VOU_HD0_DIV_MOD: {
            hi_u32 *clk_sel = HI_NULL;
            clk_sel = (hi_u32 *)io_args;
            sys_hal_vou_hd0_div_mode(dev, *clk_sel);
            break;
        }
        case SYS_VOU_HD_CLK_SEL: {
            hi_u32 *clk_sel = HI_NULL;
            clk_sel = (hi_u32 *)io_args;
            sys_hal_vou_hd_clk_sel(dev, *clk_sel);
            break;
        }
        case SYS_VO_HDMI_TX_CLK_EN: {
            hi_bool *hdmi_tx_clk_en = HI_NULL;
            hdmi_tx_clk_en = (hi_bool *)io_args;
            sys_hal_vou_hdmi_tx_clk_en(dev, *hdmi_tx_clk_en);
            break;
        }
        case SYS_VO_MIPI_TX_CLK_EN: {
            hi_bool *mipi_tx_clk_en = HI_NULL;
            mipi_tx_clk_en = (hi_bool *)io_args;
            sys_hal_vou_mipi_tx_clk_en(dev, *mipi_tx_clk_en);
            break;
        }
        case SYS_VO_HDMI_CLK_SEL: {
            hi_u32 *hdmi_clkdiv = HI_NULL;
            hdmi_clkdiv = (hi_u32 *)io_args;
            sys_hal_vou_hd_hdmi_clk_div(dev, *hdmi_clkdiv);
            break;
        }
        case SYS_VOU_DAC_CLK_PHASIC_REVERSE_EN: {
            break;
        }
        case SYS_VOU_HD_CLKOUT_PHASIC_REVERSE_EN: {
            hi_bool *clk_reverse_en = HI_NULL;
            clk_reverse_en = (hi_bool *)io_args;
            sys_hal_vou_hd_out_pctrl(dev, *clk_reverse_en);
            break;
        }
        case SYS_VOU_OUT_CLK_SEL: {
            hi_u32 *clk_sel = HI_NULL;
            clk_sel = (hi_u32 *)io_args;
            sys_hal_vou_out_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_CLK_SEL: {
            hi_u32 *clk_sel = HI_NULL;
            clk_sel = (hi_u32 *)io_args;
            sys_hal_vou_out_clk_sel(dev, *clk_sel);
            break;
        }
        case SYS_VOU_LCD_CLK_EN: {
            hi_bool *lcd_clk_en = HI_NULL;
            lcd_clk_en = (hi_bool *)io_args;
            sys_hal_vou_lcd_clk_en(dev, *lcd_clk_en);
            break;
        }
        case SYS_LCD_MCLK_DIV: {
            hi_u32 *lcd_mclk_div = HI_NULL;
            lcd_mclk_div = (hi_u32 *)io_args;
            sys_hal_lcd_mclk_div(*lcd_mclk_div);
            break;
        }
        case SYS_LCD_DATA_MODE: {
            hi_u32 *data_mode = HI_NULL;
            data_mode = (hi_u32 *)io_args;
            sys_hal_lcd_data_mode(*data_mode);
            break;
        }

        case SYS_VO_PLL_FRAC_SET: {
            hi_s32 pll;
            hi_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(hi_u32 *)io_args;
            sys_hal_set_vo_pll_frac(pll, bits_set);
            break;
        }
        case SYS_VO_PLL_POSTDIV1_SET: {
            hi_s32 pll;
            hi_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(hi_u32 *)io_args;
            sys_hal_set_vo_pll_postdiv1(pll, bits_set);
            break;
        }
        case SYS_VO_PLL_POSTDIV2_SET: {
            hi_s32 pll;
            hi_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(hi_u32 *)io_args;
            sys_hal_set_vo_pll_postdiv2(pll, bits_set);
            break;
        }
        case SYS_VO_PLL_REFDIV_SET: {
            hi_s32 pll;
            hi_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(hi_u32 *)io_args;
            sys_hal_set_vo_pll_refdiv(pll, bits_set);
            break;
        }
        case SYS_VO_PLL_FBDIV_SET: {
            hi_s32 pll;
            hi_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(hi_u32 *)io_args;
            sys_hal_set_vo_pll_fbdiv(pll, bits_set);
            break;
        }
        case SYS_AIO_RESET_SEL: {
            hi_bool *reset = HI_NULL;
            reset = (hi_bool *)io_args;
            sys_hal_aio_reset_sel(*reset);
            break;
        }
        case SYS_AIO_CLK_EN: {
            hi_bool *clk_en = HI_NULL;
            clk_en = (hi_bool *)io_args;
            sys_hal_aio_clk_en(*clk_en);
            break;
        }
        default:
        {
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}


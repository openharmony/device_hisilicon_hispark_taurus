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

#include "vou_proc.h"
#include "vou.h"
#include "vou_graphics.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

static hi_char *vo_proc_get_intf_mux_name(hi_u32 intf_type_index)
{
    hi_char *intf_type_name[VO_MAX_INTF_TYPE_NUM] = {
        "     CVBS", "    YPbPr", "      VGA", "    BT656", "   BT1120", "     HDMI", "      LCD", "   BT656H",
        "   BT656L", " LCD_6BIT", " LCD_8BIT", "LCD_16BIT", "LCD_18BIT", "LCD_24BIT", "     MIPI", "   MIPI_S"
    };

    if (intf_type_index >= VO_MAX_INTF_TYPE_NUM) {
        return "         ";
    }

    return intf_type_name[intf_type_index];
}

static hi_s32 vo_proc_get_intf_mux(hi_u32 intf_type_index, hi_char *mux_name, hi_u32 mux_name_len)
{
    hi_s32 ret;

    ret = strncpy_s(mux_name, mux_name_len, vo_proc_get_intf_mux_name(intf_type_index), VO_MAX_INTFNAME_LEN - 1);
    if (ret != EOK) {
        vo_err_trace("get intf type index %u mux name fail\n", intf_type_index);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void vou_print_mux(hi_vo_intf_type intf_type, hi_char (*asz)[VO_MAX_INTFNAME_LEN])
{
    hi_s32 i = 0;
    hi_u32 j;
    hi_s32 ret;

    for (j = 0; j < VO_MAX_MUX_NUM; j++) {
        ret = strncpy_s(asz[j], VO_MAX_INTFNAME_LEN, "         ", VO_MAX_INTFNAME_LEN - 1);
        if (ret != EOK) {
            vo_err_trace("init mux name fail\n");
            return;
        }
    }

    for (j = 0; j < VO_MAX_INTF_TYPE_NUM; j++) {
        if ((intf_type >> j) & 0x01L) {
            if (i >= VO_MAX_MUX_NUM) {
                break;
            }
            ret = vo_proc_get_intf_mux(j, asz[i], VO_MAX_INTFNAME_LEN);
            if (ret != HI_SUCCESS) {
                return;
            }
            i++;
        }
    }

    for (j = 0; j < VO_MAX_MUX_NUM; j++) {
        asz[j][VO_MAX_INTFNAME_LEN - 1] = '\0';
    }
}

static hi_char *vou_print_out_mode(hi_vo_intf_sync intf_sync)
{
    vo_sync_basic_info *sync_basic_info = vo_get_sync_basic_info();
    if (intf_sync >= VO_OUTPUT_PAL && intf_sync <= VO_OUTPUT_USER) {
        if (sync_basic_info[intf_sync].index == intf_sync) {
            return sync_basic_info[intf_sync].name;
        } else {
            return "-";
        }
    } else {
        return "-";
    }
}

static hi_char *vou_print_clk_source(hi_vo_clk_source clk_source)
{
    switch (clk_source) {
        case VO_CLK_SOURCE_PLL:
            return "PLL";
        case VO_CLK_SOURCE_LCDMCLK:
            return "LCDMCLK";
        default:
            return "-";
    }
}

static hi_char *vou_print_bool(hi_bool value)
{
    return (value) ? "Y" : "N";
}

static hi_void vou_proc_dev_cfg(osal_proc_entry_t *s)
{
    vo_dev_info *dev_ctx = HI_NULL;
    hi_u32 i;
    hi_char asz[VO_MAX_MUX_NUM][VO_MAX_INTFNAME_LEN] = {0};
    hi_u32 vtth = 0;
    hi_u32 vtth2 = 0;

    osal_seq_printf(s, "-----DEVICE CONFIG------------------------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%8s%11s%11s%11s%14s%13s%8s%8s%8s\n",
                    "DevId", "DevEn", "Mux1", "Mux2", "Mux3", "IntfSync", "BgClr", "DevFrt", "Vtth", "Vtth2");

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        dev_ctx = vo_get_dev_ctx(i);
        if (dev_ctx->vo_enable == HI_TRUE) {
            vou_print_mux(dev_ctx->vou_attr.intf_type, asz);
            vou_drv_get_dev_vtth2(i, &vtth2);
            vou_drv_get_dev_vtth(i, &vtth);
            osal_seq_printf(s, "%8u%8s%11s%11s%11s%14s%#13x%8u%8u%8u\n",
                            i, vou_print_bool(dev_ctx->vo_enable), asz[0], asz[1], asz[2],
                            vou_print_out_mode(dev_ctx->vou_attr.intf_sync),
                            dev_ctx->vou_attr.bg_color, dev_ctx->full_frame_rate,
                            vtth, vtth2);
        }
    }
    osal_seq_printf(s, "\r\n");
}

static hi_void vou_proc_dev_clk_info(osal_proc_entry_t *s)
{
    vo_dev_info *dev_ctx = HI_NULL;
    hi_u32 i;
    hi_vo_user_intfsync_attr *user_intf_sync_attr = HI_NULL;
    hi_vo_user_intfsync_pll *user_sync_pll = HI_NULL;

    osal_seq_printf(s, "-----DEVICE CLOCK INFO-------------------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%8s%11s%8s%8s%11s%11s%10s%10s%10s%10s%11s\n",
                    "DevId", "DevEn", "ClkSource", "FbDiv", "Frac", "RefDiv", "PostDiv1", "PostDiv2", "LCDMCLK",
                    "VoDevDiv", "VoPreDiv", "ClkReverse");

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        dev_ctx = vo_get_dev_ctx(i);

        user_intf_sync_attr = &dev_ctx->vo_user_sync_info.user_intf_sync_attr;
        user_sync_pll = &dev_ctx->vo_user_sync_info.user_intf_sync_attr.user_sync_pll;

        if ((dev_ctx->vo_enable == HI_TRUE) && (dev_ctx->vou_attr.intf_sync == VO_OUTPUT_USER)) {
            osal_seq_printf(s, "%8u%8s%11s",
                            i, vou_print_bool(dev_ctx->vo_enable),
                            vou_print_clk_source(user_intf_sync_attr->clk_source));

            if (user_intf_sync_attr->clk_source == VO_CLK_SOURCE_PLL) {
                osal_seq_printf(s, "%8u%8u%11u%11u%10u",
                                user_sync_pll->fbdiv, user_sync_pll->frac, user_sync_pll->refdiv,
                                user_sync_pll->postdiv1, user_sync_pll->postdiv2);

                osal_seq_printf(s, "%20u", dev_ctx->vo_user_sync_info.dev_div);
                osal_seq_printf(s, "%10u", dev_ctx->vo_user_sync_info.pre_div);
            } else {
                osal_seq_printf(s, "%58u", user_intf_sync_attr->lcd_m_clk_div);
                osal_seq_printf(s, "%10u", dev_ctx->vo_user_sync_info.dev_div);
                osal_seq_printf(s, "%10u", dev_ctx->vo_user_sync_info.pre_div);
            }

            osal_seq_printf(s, "%11s\n", vou_print_bool(dev_ctx->vo_user_sync_info.clk_reverse));
        }
    }
    osal_seq_printf(s, "\r\n");
}

static hi_void vou_proc_dev_show(osal_proc_entry_t *s)
{
    vou_proc_dev_cfg(s);
    vou_proc_dev_clk_info(s);
}

static hi_void vou_proc_module_param_show(osal_proc_entry_t *s)
{
    hi_u32 i;
    hi_s32 ret;
    hi_u32 len = 2;
    hi_vo_mod_param *vo_mod_param = HI_NULL;
    hi_char c_string[VO_MAX_PHY_DEV_NUM + VO_MAX_PHY_DEV_NUM + 1] = { 0 };

    vo_mod_param = vo_get_vo_mod_param();

    osal_seq_printf(s, "-----MODULE PARAM-----------------------------\n");
    osal_seq_printf(s, "%20s%11s%14s%12s%13s\n",
                    "transparentTransmit", "ExitDev", "WbcBgBlackEn", "DevClkExtEn", "SaveBufMode");

    for (i = 0; i < VO_MAX_PHY_DEV_NUM; i++) {
        ret = snprintf_s(&c_string[i * len], (sizeof(c_string) - i * len),
                         len, "%2s", vou_print_bool(vo_mod_param->save_buf_mode[i]));
        if (ret < 0) {
            return;
        }
    }

    osal_seq_printf(s, "%20s%11s%14s%12s%13s\n",
                    vou_print_bool(vo_mod_param->transparent_transmit), vou_print_bool(vo_mod_param->exit_dev),
                    vou_print_bool(vo_mod_param->wbc_bg_black_en), vou_print_bool(vo_mod_param->dev_clk_ext_en),
                    c_string);

    osal_seq_printf(s, "\r\n");
}

static hi_void vou_proc_dev_int_show(osal_proc_entry_t *s)
{
    vo_dev_info *dev_ctx = HI_NULL;
    hi_u32 i;

    osal_seq_printf(s, "-----DEV Int Status------------------------------------------------------------\n");
    osal_seq_printf(s, "%8s%8s%8s%8s%8s%9s%9s\n", "DevId", "IntRate", "IntTime", "MaxIntT", "TimePrM", "IntGapT",
                    "MaxGapT");

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        dev_ctx = vo_get_dev_ctx(i);
        if (dev_ctx->vo_enable == HI_TRUE) {
            osal_seq_printf(s, "%8u%8u%8u%8u%8u%9u%9u\n", i, dev_ctx->debug_info.int_rate,
                            dev_ctx->debug_info.int_time, dev_ctx->debug_info.max_int_time,
                            dev_ctx->debug_info.int_time_per_min, dev_ctx->debug_info.int_gap_time,
                            dev_ctx->debug_info.max_int_gap_time);
        }
    }
    osal_seq_printf(s, "\r\n");
}

static hi_s32 vou_proc_show(osal_proc_entry_t *s)
{
    osal_seq_printf(s, "\n[VO] Version: [" MPP_VERSION "], Build Time["__DATE__", "__TIME__"]\n");
    osal_seq_printf(s, "\n");

    vou_proc_dev_show(s);
    vou_proc_module_param_show(s);

#ifdef CONFIG_HI_VO_GRAPH
    vou_graphics_show_proc(s);
#endif

    vou_proc_dev_int_show(s);

#ifdef HI_DEBUG
    vou_drv_lbw_show_proc(s);
#endif

    return HI_SUCCESS;
}

hi_s32 vo_mod_init_proc(hi_void)
{
    osal_proc_entry_t *proc = HI_NULL;

    proc = osal_create_proc_entry(PROC_ENTRY_VO_DEV, HI_NULL);
    if (proc == HI_NULL) {
        vo_alert_trace("VO create proc fail!\n");
        return HI_FAILURE;
    }
    proc->read = vou_proc_show;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */


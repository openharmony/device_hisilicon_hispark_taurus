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

#include "hi_type.h"
#include "hi_common_adapt.h"
#include "hi_comm_sys_adapt.h"

#ifndef __SYS_HAL_H__
#define __SYS_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

typedef enum {
    SYS_DRVFUNCID_DDRRST,
    SYS_DRVFUNCID_VIURST,
    SYS_DRVFUNCID_VOURST,
    SYS_DRVFUNCID_BUTT,
} sys_drvfuncid;

hi_void sys_hal_soft_int_en(hi_bool soft_int_en);
hi_s32 sys_hal_vi_bus_reset(hi_bool reset);
hi_s32 sys_hal_vi_bus_clk_en(hi_bool clk_en);
hi_s32 sys_hal_vi_ppc_reset(hi_bool reset);
hi_s32 sys_hal_vi_ppc_clk_en(hi_bool clk_en);

hi_s32 sys_hal_vi_port_reset(hi_vi_dev dev, hi_bool reset);
hi_s32 sys_hal_vi_port_clk_en(hi_vi_dev dev, hi_bool clk_en);
hi_s32 sys_hal_vi_ppc_port_clk_en(hi_vi_dev dev, hi_bool clk_en);

hi_s32 sys_hal_vi_ppc_chn_clk_en(hi_vi_pipe pipe, hi_bool clk_en);
hi_s32 sys_hal_vi_isp_cfg_reset(hi_vi_pipe pipe, hi_bool reset);
hi_s32 sys_hal_vi_isp_core_reset(hi_vi_pipe pipe, hi_bool reset);
hi_s32 sys_hal_vi_isp_clk_en(hi_vi_pipe pipe, hi_bool clk_en);

hi_s32 sys_hal_vi_proc_reset(hi_s32 vi_proc, hi_bool reset);
hi_s32 sys_hal_vi_proc_clk_en(hi_s32 vi_proc, hi_bool clk_en);

hi_void sys_hal_vi_get_cap_online_flag(hi_u32 *value);
hi_s32 sys_hal_vi_cap_proc_online_sel(hi_s32 vi_proc, hi_bool online);
hi_s32 sys_hal_vpss_online_sel(hi_s32 vpss, hi_bool online);
hi_s32 sys_hal_vi_dev_inputi_sel(hi_s32 vi_dev, hi_s32 input);
hi_void sys_hal_vi_get_proc_clk(hi_u32 *read_crg63);
hi_void sys_hal_vi_get_port_clk(hi_u32 *value);
hi_void sys_hal_vi_set_port_clk(hi_s32 dev, hi_u32 clk_sel);
hi_void sys_hal_vi_get_ppc_clk(hi_u32 *value);
hi_void sys_hal_vi_set_ppc_clk(hi_u32 clk_sel);
hi_void sys_hal_vi_set_proc_clk(hi_u32 clk_sel);
hi_void sys_hal_get_isp_fe_clk(hi_u32 *read_crg64);
hi_void sys_hal_set_isp_fe_clk(hi_s32 pipe, hi_u32 clk_sel);
hi_void sys_hal_set_vi_cap_priority(hi_u32 id, hi_u32 priority_write);
#ifdef HI_DEBUG
hi_void sys_hal_set_vedu_priority(hi_u32 id, hi_u32 priority_write);
hi_void sys_hal_set_vgs_priority(hi_u32 id, hi_u32 priority_rrite);
#endif
hi_s32 sys_hal_vou_bus_reset_sel(hi_bool reset);
hi_s32 sys_hal_vou_hd_out_pctrl(hi_s32 dev, hi_bool clk_reverse);
hi_s32 sys_hal_vou_cfg_clk_en(hi_s32 dev, hi_bool clk_en);
hi_s32 sys_hal_vou_core_clk_en(hi_s32 dev, hi_bool clk_en);
hi_s32 sys_hal_vou_ppc_clk_en(hi_s32 dev, hi_bool clk_en);
hi_s32 sys_hal_vou_out_clk_en(hi_s32 dev, hi_bool clk_en);
hi_s32 sys_hal_vou_apb_clk_en(hi_s32 dev, hi_bool clk_en);
hi_s32 sys_hal_vou_bus_clk_en(hi_bool clk_en);
hi_s32 sys_hal_vou_dev_clk_en(hi_s32 vo_dev, hi_bool clk_en);
hi_s32 sys_hal_vou_hd_clk_sel(hi_s32 dev, hi_u32 clk_sel);
hi_s32 sys_hal_vou_hd0_div_mode(hi_s32 dev, hi_u32 hd1_div_mod);
hi_s32 sys_hal_vou_out_clk_sel(hi_s32 dev, hi_u32 clk_sel);
hi_s32 sys_hal_vou_hd_hdmi_clk_div(hi_s32 dev, hi_u32 hdmi_clk_div);
hi_s32 sys_hal_vou_hdmi_tx_clk_en(hi_s32 dev, hi_bool hdmi_tx_clk_en);
hi_s32 sys_hal_vou_mipi_tx_clk_en(hi_s32 dev, hi_bool mi_pi_tx_clk_en);
hi_s32 sys_hal_vou_hd_dac_clk_en(hi_bool clk_en);
hi_s32 sys_hal_vou_lcd_clk_en(hi_s32 vo_dev, hi_bool clk_en);
hi_s32 sys_hal_lcd_mclk_div(hi_u32 mclk_div);
hi_s32 sys_hal_lcd_data_mode(hi_u32 data_mode);
hi_s32 sys_hal_vou_bt_clk_en(hi_s32 vo_dev, hi_bool bt_clk_en);
#ifdef HI_DEBUG
hi_s32 sys_hal_vou_bt_clk_sel(hi_s32 vo_dev, hi_u32 bt_clk_ch_sel);
#endif
hi_s32 sys_hal_vou_mipi_clk_en(hi_s32 vo_dev, hi_bool mipi_clk_en);
hi_s32 sys_hal_vou_hdmi_clk_en(hi_s32 vo_dev, hi_bool hdmi_clk_en);

hi_s32 sys_hal_set_vo_pll_frac(hi_s32 pll, hi_u32 bits_set);
hi_s32 sys_hal_set_vo_pll_postdiv1(hi_s32 pll, hi_u32 bits_set);
hi_s32 sys_hal_set_vo_pll_postdiv2(hi_s32 pll, hi_u32 bits_set);
hi_s32 sys_hal_set_vo_pll_refdiv(hi_s32 pll, hi_u32 bits_set);
hi_s32 sys_hal_set_vo_pll_fbdiv(hi_s32 pll, hi_u32 bits_set);

hi_s32 sys_hal_vedu_reset_sel(hi_s32 vedu, hi_bool reset);
hi_s32 sys_hal_vedu_clk_en(hi_s32 vedu, hi_bool clk_en);
hi_s32 sys_hal_vedu_sed_clk_en(hi_s32 vedu, hi_bool clk_en);
#ifdef HI_DEBUG
hi_s32 sys_hal_vedu_clk_sel(hi_s32 vedu, hi_u32 clk_sel);
#endif
hi_s32 sys_hal_vpss_reset_sel(hi_s32 vpss, hi_bool reset);
hi_s32 sys_hal_vpss_clk_en(hi_s32 vpss, hi_bool clk_en);
#ifdef HI_DEBUG
hi_s32 sys_hal_vpss_get_frequency(hi_u32 *freq);
hi_s32 sys_hal_vpss_set_frequency(hi_u32 freq);
#endif
#ifdef CONFIG_HI_AVS_SUPPORT
hi_s32 sys_hal_avs_reset_sel(hi_bool reset);
hi_s32 sys_hal_avs_clk_en(hi_bool clk_en);
#endif
hi_s32 sys_hal_jpge_reset_sel(hi_bool reset);
hi_s32 sys_hal_jpge_clk_en(hi_bool clk_en);

hi_s32 sys_hal_jpgd_clk_en(hi_bool clk_en);
hi_s32 sys_hal_jpgd_reset_sel(hi_bool reset);

hi_s32 sys_hal_wk_cnn_reset_sel(hi_bool reset);
hi_s32 sys_hal_wk_cnn_clk_en(hi_bool clk_en);
hi_s32 sys_hal_wk_cnn_clk_set(hi_u32 clk_value);
hi_s32 sys_hal_wk_cnn_get_clk_state(hi_bool *clk_state);

hi_s32 sys_hal_md_clk_en(hi_bool clk_en);
hi_s32 sys_hal_ive_reset_sel(hi_bool reset);
hi_s32 sys_hal_ive_clk_en(hi_bool clk_en);
#ifdef HI_DEBUG
hi_s32 sys_hal_ive_set_frequency(hi_u32 clk_value);
#endif
hi_s32 sys_hal_vgs_reset_sel(hi_s32 vgs, hi_bool reset);
hi_s32 sys_hal_vgs_clk_en(hi_s32 vgs, hi_bool clk_en);
#ifdef HI_DEBUG
hi_s32 sys_hal_vgs_get_clk_sel(hi_s32 vgs, hi_u32 *clk_sel);
hi_s32 sys_hal_vgs_set_clk_sel(hi_s32 vgs, hi_u32 clk_sel);
#endif
hi_s32 sys_hal_vgs_bootroom_set_ram_using(hi_bool vgs_using);
hi_s32 sys_hal_gdc_reset_sel(hi_s32 gdc, hi_bool reset);
hi_s32 sys_hal_gdc_clk_en(hi_s32 gdc, hi_bool clk_en);
hi_s32 sys_hal_gdc_get_clk_state(hi_bool *clk_state);
hi_s32 sys_hal_gdc_nnie_set_ram_using(hi_bool gdc_using);
#ifdef HI_DEBUG
hi_s32 sys_hal_gdc_get_clk_sel(hi_u32 *clk_sel);
hi_s32 sys_hal_gdc_set_clk_sel(hi_ulong ul_clk_sel);
#endif

hi_s32 sys_hal_get_custom_code(hi_u32 *custom_code);

hi_s32 sys_hal_init(hi_void);
hi_void sys_hal_exit(hi_void);

hi_s32 sys_hal_aio_reset_sel(hi_bool reset);
hi_s32 sys_hal_aio_clk_en(hi_bool clk_en);

hi_u32 sys_hal_get_chip_id(hi_void);
hi_void sys_hal_get_unique_id(hi_unique_id *unique_id);

hi_void sys_hal_read_security_state(hi_bool *security);

/* nnie gdc mutual exclusion */
hi_s32 sys_hal_gdc_nnie_mutex_sel(hi_bool gdc_sel);
/* nnie and venc mutual exclusion, while venc resolution is greater than 2688*1944 */
hi_s32 sys_hal_venc_nnie_mutex_sel(hi_bool venc_sel);
hi_s32 sys_hal_nnie_get_mutex_state(hi_bool *mutex_state);
hi_s32 sys_hal_nnie_gdc_get_mutex_state(hi_bool *mutex_state);
hi_void sys_hal_get_sys(hi_u8 *read_crg_58);
hi_void sys_hal_get_sct(hi_u64 *sct);
hi_s32 sys_hal_udir_get_clk_state(hi_bool *clk_state);
hi_s32 sys_hal_udir_set_clk(hi_bool clk_state);
hi_s32 sys_hal_udir_reset_sel(hi_bool reset);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __SYS_HAL_H__ */

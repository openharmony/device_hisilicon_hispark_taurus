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

#ifndef __VOU_HAL_H__
#define __VOU_HAL_H__

#include "vou_reg.h"
#include "vou_def.h"
#include "vou_coef.h"
#include "mkp_vo_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

hi_void hal_vou_init(hi_void);
hi_void hal_vou_exit(hi_void);

hi_void hal_write_reg(hi_u32 *address, hi_u32 value);
hi_u32 hal_read_reg(const hi_u32 *address);

hi_void hal_sys_set_outstanding(hi_void);

hi_bool hal_disp_set_intf_enable(hal_disp_outputchannel chan, hi_bool intf);
hi_bool hal_disp_get_intf_enable(hal_disp_outputchannel chan, hi_bool *intf_en);

hi_bool hal_disp_get_int_state(hal_disp_outputchannel chan, hi_bool *bottom);
hi_bool hal_disp_get_int_state_vcnt(hal_disp_outputchannel chan, hi_u32 *vcnt);
hi_bool hal_disp_set_intf_sync(hal_disp_outputchannel chan,
                               hal_disp_syncinfo *sync_info, hal_disp_syncinv *inv);
hi_bool hal_disp_set_dev_multi_chn_en(hal_disp_outputchannel chn, hal_multichn_en multi_chn_en);
hi_bool hal_disp_set_lcd_serial_perd(hi_u32 serial_perd);
hi_bool hal_disp_set_intf_ctrl(hal_disp_intf intf, hi_u32 *ctrl_info);
hi_bool hal_disp_set_intf_mux_sel(hal_disp_outputchannel chan, hal_disp_intf intf);
hi_bool hal_disp_set_intf_clip(hal_disp_intf chan, hi_bool clip, hal_disp_clip *clip_data);
hi_bool hal_disp_set_vt_thd_mode(hal_disp_outputchannel chan, hi_u32 u_field_mode);
hi_bool hal_disp_get_vtthd_mode(hal_disp_outputchannel chan, hi_bool *field_mode);
hi_bool hal_disp_set_vt_thd(hal_disp_outputchannel chan, hi_u32 vtthd, hi_u32 vtthd2);
hi_bool hal_disp_set_int_mask(hi_u32 mask_en);
hi_bool hal_disp_clr_int_mask(hi_u32 mask_en);
hi_bool hal_disp_set_int_mask1(hi_u32 mask_en);
hi_bool hal_disp_clr_int_mask1(hi_u32 mask_en);
hi_u32 hal_disp_get_int_status(hi_u32 int_msk);
hi_u32 hal_disp_get_ori_int_status(hi_u32 int_status);
hi_bool hal_disp_clear_int_status(hi_u32 int_msk);

hi_bool hal_intf_bt_set_dfir_en(hi_u32 dfir_en);
hi_void hal_disp_set_reg_up(hal_disp_outputchannel chan);
hi_u32 hal_disp_get_reg_up(hal_disp_outputchannel chan);
hi_bool hal_disp_set_dither_round_unlim(hal_disp_outputchannel chan, hi_u32 dither_mode);
hi_bool hal_disp_set_dither_data_in_out(hal_disp_outputchannel chan, hi_u32 i_data_width_dither,
                                        hi_u32 o_data_width_dither);
hi_bool hal_disp_set_dither_en(hal_disp_outputchannel chan, hi_u32 dither_en);
hi_bool hal_disp_set_dither_mode(hal_disp_outputchannel chan, hi_u32 dither_mode);
hi_bool hal_disp_set_dither_round(hal_disp_outputchannel chan, hi_u32 dither_round);
hi_bool hal_disp_set_dither_domain_mode(hal_disp_outputchannel chan, hi_u32 dither_domain_mode);
hi_bool hal_disp_set_dither_tap_mode(hal_disp_outputchannel chan, hi_u32 dither_tap_mode);
hi_bool hal_disp_set_dither_sed(hal_disp_outputchannel chan, hal_disp_dihter_sed *dither_sed);
hi_bool hal_disp_set_dither_thr_min_max(hal_disp_outputchannel chan, hi_u32 thr_min, hi_u32 thr_max);

hi_bool hal_video_set_layer_up_mode(hal_disp_layer layer, hi_u32 up_mode);
hi_bool hal_video_set_layer_alpha(hal_disp_layer layer, hi_u32 arange);

/* video layer CVFIR relative hal functions */

hi_void hal_video_cvfir_set_coef(hal_disp_layer layer, cvfir_coef *coef);

/* video layer HFIR relative hal functions */
hi_void hal_video_hfir_set_ck_gt_en(hal_disp_layer layer, hi_u32 ck_gt_en);
hi_void hal_video_hfir_set_mid_en(hal_disp_layer layer, hi_u32 mid_en);
hi_void hal_video_hfir_set_hfir_mode(hal_disp_layer layer, hi_u32 hfir_mode);
hi_void hal_video_hfir_set_coef(hal_disp_layer layer, hfir_coef *coef);

/* video layer CSC relative hal functions. */
hi_void hal_layer_csc_set_enable(hal_disp_layer layer, hi_bool csc_en);
hi_void hal_layer_csc_set_ck_gt_en(hal_disp_layer layer, hi_bool ck_gt_en);
hi_void hal_layer_csc_set_coef(hal_disp_layer layer, vdp_csc_coef *coef);
hi_void hal_layer_csc_set_dc_coef(hal_disp_layer layer, vdp_csc_dc_coef *csc_dc_coef);

hi_bool hal_layer_enable_layer(hal_disp_layer layer, hi_u32 enable);
hi_bool hal_layer_get_layer_enable(hal_disp_layer layer, hi_bool *enable);
hi_bool hal_layer_set_layer_data_fmt(hal_disp_layer layer,
                                     hal_disp_pixel_format data_fmt);
hi_bool hal_layer_get_layer_data_fmt(hal_disp_layer layer, hi_u32 *fmt);
hi_bool hal_layer_set_csc_coef(hal_disp_layer layer, csc_coef *coef);
hi_bool hal_layer_set_csc_en(hal_disp_layer layer, hi_bool csc_en);
hi_bool hal_layer_set_layer_g_alpha(hal_disp_layer layer,
                                    hi_u8 alpha0);
hi_bool hal_cbm_set_cbm_bkg(hi_u32 mixer_id, hal_disp_bkcolor *bkg);
hi_bool hal_cbm_set_cbm_mixer_prio(hal_disp_layer layer, hi_u8 prio, hi_u8 mixer_id);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of __VOU_HAL_H__ */

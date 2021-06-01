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
#include "hdmi_reg_video_path.h"
#include "hdmi_product_define.h"

volatile video_path_reg_regs_type *g_video_path_regs = NULL;

int hdmi_reg_video_path_regs_init(hi_char *addr)
{
    g_video_path_regs = (volatile video_path_reg_regs_type *)(addr + HDMI_TX_BASE_ADDR_VIDEO);
    return HI_SUCCESS;
}

int hdmi_reg_video_path_regs_deinit(void)
{
    if (g_video_path_regs != HI_NULL) {
        g_video_path_regs = HI_NULL;
    }
    return HI_SUCCESS;
}

int hdmi_reg_sync_polarity_set(unsigned int reg_sync_polarity)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    gen_ctrl.bits.reg_sync_polarity = reg_sync_polarity;
    hdmi_tx_reg_write(reg_addr, gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_sync_polarity_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return gen_ctrl.bits.reg_sync_polarity;
}

int hdmi_reg_timing_sel_set(unsigned int reg_timing_sel)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    gen_ctrl.bits.reg_timing_sel = reg_timing_sel;
    hdmi_tx_reg_write(reg_addr, gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_timing_sel_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return gen_ctrl.bits.reg_timing_sel;
}

int hdmi_reg_extmode_set(unsigned int reg_extmode)
{
    hi_u32 *reg_addr = NULL;

    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    gen_ctrl.bits.reg_extmode = reg_extmode;
    hdmi_tx_reg_write(reg_addr, gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_extmode_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return gen_ctrl.bits.reg_extmode;
}

int hdmi_reg_timing_gen_en_set(unsigned int reg_timing_gen_en)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    gen_ctrl.bits.reg_timing_gen_en = reg_timing_gen_en;
    hdmi_tx_reg_write(reg_addr, gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_timing_gen_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    timing_gen_ctrl gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->tim_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return gen_ctrl.bits.reg_timing_gen_en;
}

int hdmi_reg_video_blank_en_set(unsigned int reg_video_blank_en)
{
    hi_u32 *reg_addr = NULL;
    video_path_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->vid_path_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_video_blank_en = reg_video_blank_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_video_blank_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_path_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->vid_path_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.reg_video_blank_en;
}

int hdmi_reg_video_lp_disable_set(unsigned int reg_video_lp_disable)
{
    hi_u32 *reg_addr = NULL;
    video_path_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->vid_path_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_video_lp_disable = reg_video_lp_disable;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_video_lp_disable_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_path_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->vid_path_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.reg_video_lp_disable;
}

int hdmi_reg_cbar_pattern_sel_set(unsigned int cbar_pattern_sel)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pattern_gen_ctrl.bits.cbar_pattern_sel = cbar_pattern_sel;
    hdmi_tx_reg_write(reg_addr, pattern_gen_ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_mask_pattern_en_set(unsigned int mask_pattern_en)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    gen_ctrl.bits.mask_pattern_en = mask_pattern_en;
    hdmi_tx_reg_write(reg_addr, gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_mask_pattern_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return pattern_gen_ctrl.bits.mask_pattern_en;
}

int hdmi_reg_square_pattern_en_set(unsigned int square_pattern_en)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pattern_gen_ctrl.bits.square_pattern_en = square_pattern_en;
    hdmi_tx_reg_write(reg_addr, pattern_gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_square_pattern_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return pattern_gen_ctrl.bits.square_pattern_en;
}

int hdmi_reg_colorbar_en_set(unsigned int colorbar_en)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pattern_gen_ctrl.bits.colorbar_en = colorbar_en;
    hdmi_tx_reg_write(reg_addr, pattern_gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_colorbar_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return pattern_gen_ctrl.bits.colorbar_en;
}

int hdmi_reg_solid_pattern_en_set(unsigned int solid_pattern_en)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pattern_gen_ctrl.bits.solid_pattern_en = solid_pattern_en;
    hdmi_tx_reg_write(reg_addr, pattern_gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_solid_pattern_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return pattern_gen_ctrl.bits.solid_pattern_en;
}

int hdmi_reg_video_format_set(unsigned int video_format)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pattern_gen_ctrl.bits.video_format = video_format;
    hdmi_tx_reg_write(reg_addr, pattern_gen_ctrl.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_video_format_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    pattern_gen_ctrll pattern_gen_ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->ptn_gen_ctrl.u32);
    pattern_gen_ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return pattern_gen_ctrl.bits.video_format;
}

int hdmi_reg_solid_pattern_cr_set(unsigned int solid_pattern_cr)
{
    hi_u32 *reg_addr = NULL;
    solid_pattern_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->solid_ptn_ctrl.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.solid_pattern_cr = solid_pattern_cr;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

int hdmi_reg_solid_pattern_y_set(unsigned int solid_pattern_y)
{
    hi_u32 *reg_addr = NULL;
    solid_pattern_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->solid_ptn_ctrl.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.solid_pattern_y = solid_pattern_y;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

int hdmi_reg_solid_pattern_cb_set(unsigned int solid_pattern_cb)
{
    hi_u32 *reg_addr = NULL;
    solid_pattern_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->solid_ptn_ctrl.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.solid_pattern_cb = solid_pattern_cb;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

int hdmi_reg_fdt_status_clear_set(unsigned int fdt_status_clear)
{
    hi_u32 *reg_addr = NULL;
    format_det_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_det_cfg.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.fdt_status_clear = fdt_status_clear;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

int hdmi_reg_sync_polarity_force_set(unsigned int sync_polarity_force)
{
    hi_u32 *reg_addr = NULL;
    format_det_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_det_cfg.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.sync_polarity_force = sync_polarity_force;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_vsync_polarity_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_status status;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_status.u32);
    status.u32 = hdmi_tx_reg_read(reg_addr);

    return status.bits.vsync_polarity;
}

hi_u32 hdmi_reg_hsync_polarity_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_status status;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_status.u32);
    status.u32 = hdmi_tx_reg_read(reg_addr);

    return status.bits.hsync_polarity;
}

hi_u32 hdmi_reg_interlaced_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_status status;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_status.u32);
    status.u32 = hdmi_tx_reg_read(reg_addr);

    return status.bits.interlaced;
}

hi_u32 hdmi_reg_hsync_total_cnt_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_hori_res tmp;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_hori_res.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);

    return tmp.bits.hsync_total_cnt;
}

hi_u32 hdmi_reg_hsync_active_cnt_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_hori_res tmp;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_hori_res.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.hsync_active_cnt;
}

hi_u32 hdmi_reg_vsync_total_cnt_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_hori_vert_res fdet_vert_res;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_hori_vert_res.u32);
    fdet_vert_res.u32 = hdmi_tx_reg_read(reg_addr);
    return fdet_vert_res.bits.vsync_total_cnt;
}

hi_u32 hdmi_reg_vsync_active_cnt_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    fdet_hori_vert_res fdet_vert_res;

    reg_addr = (hi_u32 *)&(g_video_path_regs->fmt_dect_hori_vert_res.u32);
    fdet_vert_res.u32 = hdmi_tx_reg_read(reg_addr);

    return fdet_vert_res.bits.vsync_active_cnt;
}

hi_u32 hdmi_reg_dwsm_vert_bypass_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_dwsm_vert_byp;
}

hi_u32 hdmi_reg_dwsm_vert_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_dwsm_vert_en;
}

hi_u32 hdmi_reg_hori_filter_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;

    video_dwsm_ctrl ctrl;
    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.reg_hori_filter_en;
}

hi_u32 hdmi_reg_dwsm_hori_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_dwsm_hori_en;
}

hi_u32 hdmi_reg_pxl_div_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    data_align_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->align_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_pxl_div_en;
}

hi_u32 hdmi_reg_demux_420_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    data_align_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->align_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_demux_420_en;
}

hi_u32 hdmi_reg_inver_sync_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_inver_sync;
}

hi_u32 hdmi_reg_vmux_cr_sel_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_vmux_cr_sel;
}

hi_u32 hdmi_reg_vmux_cb_sel_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_vmux_cb_sel;
}

hi_u32 hdmi_reg_vmux_y_sel_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_vmux_y_sel;
}

hi_u32 hdmi_reg_dither_mode_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    dither_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dither_cfg.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);

    return config.bits.dither_mode;
}

hi_u32 hdmi_reg_dither_rnd_bypass_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    dither_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dither_cfg.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);

    return config.bits.dither_rnd_byp;
}

hi_u32 hdmi_reg_csc_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    multi_csc_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->csc_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);

    return ctrl.bits.reg_csc_en;
}

hi_u32 hdmi_reg_csc_mode_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    multi_csc_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->csc_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return ctrl.bits.reg_csc_mode;
}

int hdmi_reg_dither_mode_set(unsigned int dither_mode)
{
    hi_u32 *reg_addr = NULL;
    dither_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dither_cfg.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.dither_mode = dither_mode;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

int hdmi_reg_dither_rnd_bypass_set(unsigned int dither_rnd_byp)
{
    hi_u32 *reg_addr = NULL;
    dither_config config;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dither_cfg.u32);
    config.u32 = hdmi_tx_reg_read(reg_addr);
    config.bits.dither_rnd_byp = dither_rnd_byp;
    hdmi_tx_reg_write(reg_addr, config.u32);

    return HI_SUCCESS;
}

int hdmi_reg_csc_mode_set(unsigned int reg_csc_mode)
{
    hi_u32 *reg_addr = NULL;
    multi_csc_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->csc_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_csc_mode = reg_csc_mode;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_csc_saturate_en_set(unsigned int reg_csc_saturate_en)
{
    hi_u32 *reg_addr = NULL;
    multi_csc_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->csc_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_csc_saturate_en = reg_csc_saturate_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_csc_en_set(unsigned int reg_csc_en)
{
    hi_u32 *reg_addr = NULL;
    multi_csc_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->csc_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_csc_en = reg_csc_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_dwsm_vert_bypass_set(unsigned int reg_dwsm_vert_byp)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_dwsm_vert_byp = reg_dwsm_vert_byp;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_dwsm_vert_en_set(unsigned int reg_dwsm_vert_en)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_dwsm_vert_en = reg_dwsm_vert_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_hori_filter_en_set(unsigned int reg_hori_filter_en)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_hori_filter_en = reg_hori_filter_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_dwsm_hori_en_set(unsigned int reg_dwsm_hori_en)
{
    hi_u32 *reg_addr = NULL;
    video_dwsm_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dwsm_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_dwsm_hori_en = reg_dwsm_hori_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_pxl_div_en_set(unsigned int reg_pxl_div_en)
{
    hi_u32 *reg_addr = NULL;
    data_align_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->align_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_pxl_div_en = reg_pxl_div_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_demux_420_en_set(unsigned int reg_demux_420_en)
{
    hi_u32 *reg_addr = NULL;
    data_align_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->align_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_demux_420_en = reg_demux_420_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_inver_sync_set(unsigned int reg_inver_sync)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_inver_sync = reg_inver_sync;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_syncmask_en_set(unsigned int reg_syncmask_en)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_syncmask_en = reg_syncmask_en;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vmux_cr_sel_set(unsigned int reg_vmux_cr_sel)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_vmux_cr_sel = reg_vmux_cr_sel;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vmux_cb_sel_set(unsigned int reg_vmux_cb_sel)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_vmux_cb_sel = reg_vmux_cb_sel;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vmux_y_sel_set(unsigned int reg_vmux_y_sel)
{
    hi_u32 *reg_addr = NULL;
    video_dmux_ctrl ctrl;

    reg_addr = (hi_u32 *)&(g_video_path_regs->dmux_ctrl.u32);
    ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    ctrl.bits.reg_vmux_y_sel = reg_vmux_y_sel;
    hdmi_tx_reg_write(reg_addr, ctrl.u32);

    return HI_SUCCESS;
}


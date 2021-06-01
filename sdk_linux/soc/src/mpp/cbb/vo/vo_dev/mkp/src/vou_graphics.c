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

#include "vou_graphics.h"
#include "hi_osal.h"

#include "hi_debug_adapt.h"
#include "hi_comm_vo_adapt.h"
#include "graphics_drv.h"
#include "vou.h"

vo_gfxlayer_context g_vo_gfx_layer_ctx[VO_MAX_GRAPHICS_LAYER_NUM];
hi_u32 g_dectect_zone = 0;

vo_gfxlayer_context *vou_graphics_get_layer_ctx(hi_graphic_layer gfx_layer)
{
    return &g_vo_gfx_layer_ctx[gfx_layer];
}

#ifndef CONFIG_HI_VO_FB_SEPARATE

hi_s32 vo_graphics_open_layer(hi_graphic_layer layer)
{
    hi_vo_dev dev;
    hi_ulong lock_flag = 0;
    hi_u32 layer_index;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;
    hal_disp_layer gfx_layer = HAL_DISP_LAYER_BUTT;
    vo_dev_info *dev_ctx = HI_NULL;

    graphic_drv_get_layer_id(layer, &gfx_layer);
    if (graphic_drv_get_layer_index(gfx_layer, &layer_index) != HI_SUCCESS) {
        vo_err_trace("gfx_layer(%u) is invalid!\n", (hi_u32)gfx_layer);
        return HI_ERR_VO_GFX_INVALID_ID;
    }

    gfx_layer_ctx = &g_vo_gfx_layer_ctx[layer_index];

    gfx_spin_lock_irqsave(&gfx_layer_ctx->spin_lock, &lock_flag);
    if (!gfx_layer_ctx->binded) {
        gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);
        vo_err_trace("graphics layer %u has not been binded!\n", layer_index);
        return HI_ERR_VO_GFX_NOT_BIND;
    }

    dev = gfx_layer_ctx->binded_dev;
    dev_ctx = vo_get_dev_ctx(dev);
    if (!dev_ctx->vo_enable) {
        gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);
        vo_err_trace("dev %d for graphics layer %u has not been enable!\n",
                     dev, layer_index);
        return HI_ERR_VO_DEV_NOT_ENABLE;
    }
    gfx_layer_ctx->opened = HI_TRUE;

    gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);

    return HI_SUCCESS;
}

hi_s32 vou_graphics_set_callback(hi_graphic_layer layer, vo_fb_int_type type,
                                 vo_fb_int_call_back call_back, hi_void *arg)
{
    hal_disp_layer gfx_layer = HAL_DISP_LAYER_BUTT;

    hi_ulong lock_flag;
    hi_u32 layer_index;
    hi_s32 ret = HI_SUCCESS;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;

    graphic_drv_get_layer_id(layer, &gfx_layer);
    if (graphic_drv_get_layer_index(gfx_layer, &layer_index) != HI_SUCCESS) {
        vo_err_trace("gfx_layer(%u) is invalid!\n", (hi_u32)gfx_layer);
        return HI_ERR_VO_GFX_INVALID_ID;
    }

    gfx_layer_ctx = &g_vo_gfx_layer_ctx[layer_index];

    gfx_spin_lock_irqsave(&gfx_layer_ctx->spin_lock, &lock_flag);
    switch (type) {
        case HIFB_INTTYPE_VO: {
            gfx_layer_ctx->vo_call_back = (fb_int_call_back)call_back;
            gfx_layer_ctx->vo_call_back_arg = arg;
            break;
        }
        case HIFB_INTTYPE_WBC: {
            gfx_layer_ctx->wbc_call_back = (fb_int_call_back)call_back;
            gfx_layer_ctx->wbc_call_back_arg = arg;
            break;
        }
        default: {
            ret = HI_FAILURE;
            break;
        }
    }
    gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);

    return ret;
}

hi_void vo_graphics_vtth_int_process(hi_vo_dev dev, hi_u32 int_status)
{
    hi_ulong lock_flag;
    hi_s32 i;
    hi_bool progressvie = HI_FALSE;
    hi_bool bottom_int = HI_FALSE;
    fb_int_call_back vo_call_back;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;

    graphic_drv_get_scan_mode(dev, &progressvie);
    graphic_drv_get_int_state(dev, &bottom_int);

    for (i = 0; i < VO_MAX_GRAPHICS_LAYER_NUM; ++i) {
        gfx_layer_ctx = &g_vo_gfx_layer_ctx[i];
        gfx_spin_lock_irqsave(&gfx_layer_ctx->spin_lock, &lock_flag);
        if (!gfx_layer_ctx->opened) {
            gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);
            continue;
        }

        if ((gfx_layer_ctx->binded) && (gfx_layer_ctx->binded_dev == dev)) {
            if ((progressvie == HI_FALSE) && (bottom_int == HI_FALSE)) {
                /* do nothing */
            } else if (gfx_layer_ctx->vo_call_back != HI_NULL) {
                vo_call_back = gfx_layer_ctx->vo_call_back;
                gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);

                vo_call_back(gfx_layer_ctx->vo_call_back_arg, &int_status);
                continue;
            }
        }
        gfx_spin_unlock_irqrestore(&gfx_layer_ctx->spin_lock, &lock_flag);
    }
}
#endif

hi_s32 vou_graphics_lock_init(hi_void)
{
    hi_s32 ret;
    ret = graphic_drv_lock_init();
    return ret;
}

hi_s32 vou_graphics_lock_de_init(hi_void)
{
    hi_s32 ret;
    ret = graphic_drv_lock_exit();
    return ret;
}

hi_s32 vo_graphics_init(hi_void)
{
    return graphic_drv_init();
}

hi_s32 vo_graphics_de_init(hi_void)
{
    return graphic_drv_exit();
}

hi_s32 vou_graphics_bind_dev(hi_graphic_layer gfx_layer, hi_vo_dev dev)
{
    hi_s32 ret;

    ret = graphic_drv_check_graphic_bind_support(gfx_layer, dev);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return graphic_drv_graphic_bind_dev(gfx_layer, dev);
}

hi_s32 vou_graphics_un_bind_dev(hi_graphic_layer gfx_layer, hi_vo_dev dev)
{
    hi_s32 ret;

    ret = graphic_drv_check_graphic_bind_support(gfx_layer, dev);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return graphic_drv_graphic_un_bind_dev(gfx_layer);
}

hi_s32 graphic_set_dev_bg_color(hi_graphic_layer layer, hi_vo_csc_matrix csc_matrix)
{
    hi_bool bkg_rgb = HI_FALSE;
    hi_vo_dev binded_vo_dev;
    vo_dev_info *vo_dev_ctx = HI_NULL;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;

    if (csc_matrix == VO_CSC_MATRIX_IDENTITY) {
        bkg_rgb = HI_TRUE;
    } else {
        bkg_rgb = HI_FALSE;
    }

    gfx_layer_ctx = vou_graphics_get_layer_ctx(layer);

    binded_vo_dev = gfx_layer_ctx->binded_dev;
    if (binded_vo_dev == HI_INVALID_VALUE) {
        vo_err_trace("graphics layer %d is not binded to any dev!\n", layer);
        return HI_ERR_VO_GFX_NOT_BIND;
    }

    vo_dev_ctx = vo_get_dev_ctx(binded_vo_dev);

    vo_drv_set_dev_bg_color(binded_vo_dev, vo_dev_ctx->vou_attr.bg_color, bkg_rgb);
    return HI_SUCCESS;
}

hi_s32 vou_graphics_set_csc(hi_u32 layer, hi_vo_csc *csc)
{
    hi_s32 ret;
    hal_disp_layer gfx_layer;
    hi_bool enable = HI_FALSE;

    if (!graphic_drv_is_chip_support_csc()) {
        vo_err_trace("graphics layer %u is not support to adjust csc!\n", layer);
        return HI_ERR_VO_NOT_SUPPORT;
    }

    vo_check_null_ptr_return(csc);

    ret = graphic_drv_get_hal_layer(layer, &gfx_layer);
    if (ret != HI_SUCCESS) {
        vo_err_trace("graphics layer %u is illegal!\n", layer);
        return HI_ERR_VO_GFX_INVALID_ID;
    }

    enable = graphic_drv_get_layer_enable(gfx_layer);
    if (enable == HI_FALSE) {
        vo_err_trace("graphic layer %u is not enable, can't set CSC!\n", layer);
        return HI_ERR_VO_NOT_PERMIT;
    }

    ret = graphic_drv_check_csc_coef(gfx_layer, csc);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    (hi_void)memcpy_s(&g_vo_gfx_layer_ctx[layer].gfx_csc, sizeof(hi_vo_csc), csc, sizeof(hi_vo_csc));

    ret = graphic_set_dev_bg_color(layer, csc->csc_matrix);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return graphic_drv_set_csc_coef(gfx_layer, csc, &g_vo_gfx_layer_ctx[layer].csc_param);
}

hi_s32 vou_graphics_get_csc(hi_u32 layer, hi_vo_csc *csc)
{
    hi_s32 ret;
    hal_disp_layer gfx_layer;
    hi_bool enable = HI_FALSE;

    if (!graphic_drv_is_chip_support_csc()) {
        vo_err_trace("graphics layer %u is not support to get csc!\n", layer);
        return HI_ERR_VO_NOT_SUPPORT;
    }

    vo_check_null_ptr_return(csc);

    ret = graphic_drv_get_hal_layer(layer, &gfx_layer);
    if (ret != HI_SUCCESS) {
        vo_err_trace("graphics layer %u is illegal!\n", layer);
        return HI_ERR_VO_GFX_INVALID_ID;
    }

    enable = graphic_drv_get_layer_enable(gfx_layer);
    if (enable == HI_FALSE) {
        vo_err_trace("graphic layer %u is not enable, can't get CSC!\n", layer);
        return HI_ERR_VO_NOT_PERMIT;
    }

    (hi_void)memcpy_s(csc, sizeof(hi_vo_csc), &g_vo_gfx_layer_ctx[layer].gfx_csc, sizeof(hi_vo_csc));

    return HI_SUCCESS;
}

hi_s32 vou_graphics_show_proc(osal_proc_entry_t *s)
{
    hi_vo_csc csc;
    hi_s32 dev;
    hi_s32 i;
    vo_dev_info *dev_ctx = HI_NULL;

    graphic_drv_show_proc(s);

    osal_seq_printf(s, "-----GRAPHIC LAYER CSC PARAM-----------------------------\n");
    osal_seq_printf(s, "%s%s%s%s%s%s\n", " LAYERID", "  Matrix", "    Luma", "    Cont", "     Hue", "    Satu");
    for (i = 0; i < VO_MAX_GRAPHICS_LAYER_NUM; i++) {
        dev = g_vo_gfx_layer_ctx[i].binded_dev;
        dev_ctx = vo_get_dev_ctx(dev);
        if (dev_ctx->vo_enable) {
            csc = g_vo_gfx_layer_ctx[i].gfx_csc;
            osal_seq_printf(s, "%8u%8u%8u%8u%8u%8u\n", i, csc.csc_matrix, csc.luma, csc.contrast,
                            csc.hue, csc.satuature);
        }
    }
    osal_seq_printf(s, "\r\n");

    return HI_SUCCESS;
}

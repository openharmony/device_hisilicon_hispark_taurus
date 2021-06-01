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
#include "graphics_drv.h"
#include "proc_ext.h"

#include "vou_graphics.h"

#include "vou.h"
#include "mm_ext.h"

#define GFX_CSC_SCALE    0xd
#define GFX_CSC_CLIP_MIN 0x0
#define GFX_CSC_CLIP_MAX 0x3ff

hi_s32 graphic_drv_get_hal_layer(hi_u32 layer, hal_disp_layer *hal_layer)
{
    switch (layer) {
        case GX_LAYER_INDEX0: {
            *hal_layer = HAL_DISP_LAYER_GFX0;
            break;
        }
        default: {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

hi_s32 graphic_drv_get_layer_index(hal_disp_layer hal_layer, hi_u32 *layer)
{
    switch (hal_layer) {
        case HAL_DISP_LAYER_GFX0: {
            *layer = GX_LAYER_INDEX0;
            break;
        }
        default: {
            return HI_ERR_VO_GFX_INVALID_ID;
        }
    }

    return HI_SUCCESS;
}

/* graphic bind */
hi_s32 graphic_drv_check_graphic_bind_support(hi_graphic_layer gfx_layer, hi_vo_dev dev)
{
    vo_err_trace("graphics layer %d not support bind dev %d!\n", gfx_layer, dev);
    return HI_ERR_VO_NOT_SUPPORT;
}

hi_s32 graphic_drv_graphic_bind_dev(hi_graphic_layer gfx_layer, hi_vo_dev dev)
{
    hi_unused(gfx_layer);
    hi_unused(dev);
    return HI_SUCCESS;
}

hi_s32 graphic_drv_graphic_un_bind_dev(hi_graphic_layer gfx_layer)
{
    hi_unused(gfx_layer);
    return HI_SUCCESS;
}

hi_s32 graphic_drv_lock_init(hi_void)
{
    hi_s32 i;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;

    for (i = 0; i < VO_MAX_GRAPHICS_LAYER_NUM; ++i) {
        gfx_layer_ctx = vou_graphics_get_layer_ctx(i);
        (hi_void)memset_s(gfx_layer_ctx, sizeof(vo_gfxlayer_context), 0, sizeof(vo_gfxlayer_context));
        gfx_spin_lock_init(&gfx_layer_ctx->spin_lock);
    }
    return HI_SUCCESS;
}

hi_s32 graphic_drv_lock_exit(hi_void)
{
    hi_s32 i;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;
    for (i = 0; i < VO_MAX_GRAPHICS_LAYER_NUM; ++i) {
        gfx_layer_ctx = vou_graphics_get_layer_ctx(i);
        gfx_spin_lock_deinit(&gfx_layer_ctx->spin_lock);
    }

    return HI_SUCCESS;
}

hi_s32 graphic_drv_init(hi_void)
{
    hi_s32 i;
    vo_gfxlayer_context *gfx_ctx = HI_NULL;

    for (i = 0; i < VO_MAX_GRAPHICS_LAYER_NUM; ++i) {
        gfx_ctx = vou_graphics_get_layer_ctx(i);

        gfx_ctx->layer_id = HAL_DISP_LAYER_GFX0 + i; /* HAL_DISP_LAYER_GFX0+1 */
        gfx_ctx->binded_dev = i; /* 0 */
        gfx_ctx->binded = HI_TRUE;
        gfx_ctx->vo_gfx_attr.dst_dynamic_range = DYNAMIC_RANGE_BUTT;
    }

    for (i = 0; i < VO_MAX_GRAPHICS_LAYER_NUM; ++i) {
        gfx_ctx = vou_graphics_get_layer_ctx(i);
        gfx_ctx->gfx_csc.csc_matrix = VO_CSC_MATRIX_RGB_TO_BT601_TV;
        gfx_ctx->gfx_csc.luma = VO_CSC_DEF_VAL;
        gfx_ctx->gfx_csc.contrast = VO_CSC_DEF_VAL;
        gfx_ctx->gfx_csc.hue = VO_CSC_DEF_VAL;
        gfx_ctx->gfx_csc.satuature = VO_CSC_DEF_VAL;

        gfx_ctx->csc_param.csc_scale2p = GFX_CSC_SCALE;
        gfx_ctx->csc_param.csc_clip_min = GFX_CSC_CLIP_MIN;
        gfx_ctx->csc_param.csc_clip_max = GFX_CSC_CLIP_MAX;
    }

    return HI_SUCCESS;
}

hi_s32 graphic_drv_exit(hi_void)
{
    return HI_SUCCESS;
}

hi_bool graphic_drv_is_chip_support_csc(hi_void)
{
    return HI_TRUE;
}

static hi_s32 graphic_drv_get_csc_matrix(hal_csc_mode csc_mode, const csc_coef **csc_tmp)
{
    switch (csc_mode) {
        case HAL_CSC_MODE_RGB_TO_RGB:
        case HAL_CSC_MODE_RGB_TO_BT601_PC:
        case HAL_CSC_MODE_RGB_TO_BT709_PC:
        case HAL_CSC_MODE_RGB_TO_BT601_TV:
        case HAL_CSC_MODE_RGB_TO_BT709_TV:
            *csc_tmp = vo_get_csc_coef(csc_mode);
            break;
        default:
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void graphic_drv_calculate_rgb2yuv(hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef)
{
    hi_s32 luma;
    hi_s32 contrast;
    hi_s32 hue;
    hi_s32 satu;
    hi_s32 csc_value_times = 100;
    hi_s32 table_times = 1000;
    hi_s32 square_cv_times = csc_value_times * csc_value_times;
    const int *cos_table = vo_get_cos_table();
    const int *sin_table = vo_get_sin_table();

    luma = csc_value->luma;
    contrast = csc_value->cont;
    hue = csc_value->hue;
    satu = csc_value->satu;

    coef->csc_coef00 = (contrast * csc_tmp->csc_coef00) / csc_value_times;
    coef->csc_coef01 = (contrast * csc_tmp->csc_coef01) / csc_value_times;
    coef->csc_coef02 = (contrast * csc_tmp->csc_coef02) / csc_value_times;
    coef->csc_coef10 = (contrast * satu * ((csc_tmp->csc_coef10 * cos_table[hue] + csc_tmp->csc_coef20 *
                                                sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef11 = (contrast * satu * ((csc_tmp->csc_coef11 * cos_table[hue] + csc_tmp->csc_coef21 *
                                                sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef12 = (contrast * satu * ((csc_tmp->csc_coef12 * cos_table[hue] + csc_tmp->csc_coef22 *
                                                sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef20 = (contrast * satu * ((csc_tmp->csc_coef20 * cos_table[hue] - csc_tmp->csc_coef10 *
                                                sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef21 = (contrast * satu * ((csc_tmp->csc_coef21 * cos_table[hue] - csc_tmp->csc_coef11 *
                                                sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef22 = (contrast * satu * ((csc_tmp->csc_coef22 * cos_table[hue] - csc_tmp->csc_coef12 *
                                                sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_out_dc0 += luma;
}

hi_void graphic_drv_calc_csc_matrix(hi_vo_csc *csc, hal_csc_mode csc_mode, csc_coef *coef)
{
    hi_s32 ret;
    hal_csc_value csc_value;
    const csc_coef *csc_tmp = HI_NULL;

    csc_value.luma = (hi_s32)csc->luma * 64 / 100 - 32;
    csc_value.cont = ((hi_s32)csc->contrast - 50) * 2 + 100;
    csc_value.hue = (hi_s32)csc->hue * 60 / 100;
    csc_value.satu = ((hi_s32)csc->satuature - 50) * 2 + 100;

    ret = graphic_drv_get_csc_matrix(csc_mode, &csc_tmp);
    if (ret != HI_SUCCESS) {
        return;
    }

    coef->csc_in_dc0 = csc_tmp->csc_in_dc0;
    coef->csc_in_dc1 = csc_tmp->csc_in_dc1;
    coef->csc_in_dc2 = csc_tmp->csc_in_dc2;
    coef->csc_out_dc0 = csc_tmp->csc_out_dc0;
    coef->csc_out_dc1 = csc_tmp->csc_out_dc1;
    coef->csc_out_dc2 = csc_tmp->csc_out_dc2;

    graphic_drv_calculate_rgb2yuv(&csc_value, csc_tmp, coef);

    return;
}

hi_bool graphic_drv_get_layer_enable(hal_disp_layer gfx_layer)
{
    hi_bool enable = HI_FALSE;
    hal_layer_get_layer_enable(gfx_layer, &enable);
    return enable;
}

hi_s32 graphic_drv_check_csc_coef(hal_disp_layer gfx_layer, hi_vo_csc *gfx_csc)
{
    hi_unused(gfx_layer);

    if ((gfx_csc->csc_matrix != VO_CSC_MATRIX_IDENTITY) &&
        (gfx_csc->csc_matrix != VO_CSC_MATRIX_RGB_TO_BT601_PC) &&
        (gfx_csc->csc_matrix != VO_CSC_MATRIX_RGB_TO_BT709_PC) &&
        (gfx_csc->csc_matrix != VO_CSC_MATRIX_RGB_TO_BT601_TV) &&
        (gfx_csc->csc_matrix != VO_CSC_MATRIX_RGB_TO_BT709_TV)) {
        vo_err_trace("vo CSC matrix type(%d) should be rgb to bt601/bt709 only!\n", gfx_csc->csc_matrix);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if (gfx_csc->contrast > VO_CSC_LUMA_MAX) {
        vo_err_trace("vo device CSC contrast value %d out of range!\n", gfx_csc->contrast);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if (gfx_csc->hue > VO_CSC_LUMA_MAX) {
        vo_err_trace("vo device CSC hue value %d out of range!\n", gfx_csc->hue);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if (gfx_csc->luma > VO_CSC_LUMA_MAX) {
        vo_err_trace("vo device CSC luma value %d out of range!\n", gfx_csc->luma);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if (gfx_csc->satuature > VO_CSC_LUMA_MAX) {
        vo_err_trace("vo device CSC satuature value %d out of range!\n", gfx_csc->satuature);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 graphic_drv_get_hal_csc_mode(hi_vo_csc_matrix csc_matrix, hal_csc_mode *csc_mode)
{
    switch (csc_matrix) {
        case VO_CSC_MATRIX_RGB_TO_BT601_PC:
            *csc_mode = HAL_CSC_MODE_RGB_TO_BT601_PC;
            break;
        case VO_CSC_MATRIX_RGB_TO_BT709_PC:
            *csc_mode = HAL_CSC_MODE_RGB_TO_BT709_PC;
            break;
        case VO_CSC_MATRIX_RGB_TO_BT601_TV:
            *csc_mode = HAL_CSC_MODE_RGB_TO_BT601_TV;
            break;
        case VO_CSC_MATRIX_RGB_TO_BT709_TV:
            *csc_mode = HAL_CSC_MODE_RGB_TO_BT709_TV;
            break;
        case VO_CSC_MATRIX_IDENTITY:
            *csc_mode = HAL_CSC_MODE_RGB_TO_RGB;
            break;
        default:
            vo_err_trace("csc_matrix %d err, should only be RGB to YUV matrix\n", csc_matrix);
            return HI_ERR_VO_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

hi_s32 graphic_drv_set_csc_coef(hal_disp_layer gfx_layer, hi_vo_csc *gfx_csc, csc_coef_param *csc_param)
{
    hi_s32 ret;
    csc_coef coef;
    hal_csc_mode csc_mode;

    hi_u32 pre = 8;
    hi_u32 dc_pre = 4;

    hi_u32 layer_index;
    vo_gfxlayer_context *gfx_layer_ctx = HI_NULL;

    if (graphic_drv_get_layer_index(gfx_layer, &layer_index) != HI_SUCCESS) {
        vo_err_trace("gfx_layer(%u) is invalid!\n", (hi_u32)gfx_layer);
        return HI_ERR_VO_GFX_INVALID_ID;
    }

    ret = graphic_drv_get_hal_csc_mode(gfx_csc->csc_matrix, &csc_mode);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    graphic_drv_calc_csc_matrix(gfx_csc, csc_mode, &coef);

    coef.new_csc_clip_max = GFX_CSC_CLIP_MAX;
    coef.new_csc_clip_min = GFX_CSC_CLIP_MIN;
    coef.new_csc_scale2p = GFX_CSC_SCALE;

    coef.csc_coef00 = (hi_s32)pre * coef.csc_coef00 * 1024 / 1000;
    coef.csc_coef01 = (hi_s32)pre * coef.csc_coef01 * 1024 / 1000;
    coef.csc_coef02 = (hi_s32)pre * coef.csc_coef02 * 1024 / 1000;
    coef.csc_coef10 = (hi_s32)pre * coef.csc_coef10 * 1024 / 1000;
    coef.csc_coef11 = (hi_s32)pre * coef.csc_coef11 * 1024 / 1000;
    coef.csc_coef12 = (hi_s32)pre * coef.csc_coef12 * 1024 / 1000;
    coef.csc_coef20 = (hi_s32)pre * coef.csc_coef20 * 1024 / 1000;
    coef.csc_coef21 = (hi_s32)pre * coef.csc_coef21 * 1024 / 1000;
    coef.csc_coef22 = (hi_s32)pre * coef.csc_coef22 * 1024 / 1000;

    coef.csc_in_dc0 = (hi_s32)dc_pre * coef.csc_in_dc0;
    coef.csc_in_dc1 = (hi_s32)dc_pre * coef.csc_in_dc1;
    coef.csc_in_dc2 = (hi_s32)dc_pre * coef.csc_in_dc2;

    coef.csc_out_dc0 = (hi_s32)dc_pre * coef.csc_out_dc0;
    coef.csc_out_dc1 = (hi_s32)dc_pre * coef.csc_out_dc1;
    coef.csc_out_dc2 = (hi_s32)dc_pre * coef.csc_out_dc2;

    hal_layer_set_csc_coef(gfx_layer, &coef);

    gfx_layer_ctx = vou_graphics_get_layer_ctx(layer_index);

    (hi_void)memcpy_s(&gfx_layer_ctx->gfx_csc, sizeof(hi_vo_csc), gfx_csc, sizeof(hi_vo_csc));
    (hi_void)memcpy_s(&gfx_layer_ctx->csc_param, sizeof(csc_coef_param), csc_param, sizeof(csc_coef_param));

    return HI_SUCCESS;
}

hi_s32 graphic_drv_show_proc(osal_proc_entry_t *s)
{
    hi_unused(s);
    return HI_SUCCESS;
}


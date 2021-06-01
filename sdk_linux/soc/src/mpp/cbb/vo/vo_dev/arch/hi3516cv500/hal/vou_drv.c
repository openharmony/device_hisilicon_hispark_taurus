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
#include "vou_drv.h"
#include "vou_dev_exp.h"
#include "hi_math_adapt.h"
#include "vou.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VO_DEV_MAX_NUM              1
#define VO_LAYER_MAX_NUM            1

#define VO_SD_VTTH_WATERLINE        100
#define VO_HD_VTTH_WATERLINE        240
#define VO_SD_VTTH_WATERLINE2       2
#define VO_HD_VTTH_WATERLINE2       2

#define VO_BACKGROUD_BLACK          0x8080
#define VO_BACKGROUD_GREEN          0x804D3A42
#define VO_BACKGROUD_BLUE           0x800404ee
#define VO_BACKGROUD_RED            0x00c8c8c8
#define VO_BACKGROUD_DEFAULT        VO_BACKGROUD_GREEN

#define VO_RESO_MIN_WDT             32 /* minimal width of display resolution */
#define VO_RESO_MIN_HGT             32 /* minimal height of display resolution */
#define VO_DISP_MAX_FRMRATE         240 /* max display frame rate */

/* default number of video layer buffer */
#define VO_HD_USE_BLK               4
#define VO_SD_USE_BLK               3

#define MULTI_REGION_NUM_VHD0       1
#define MULTI_REGION_NUM_VHD1       0

#define VO_MIN_DIV_MODE             1
#define VO_MAX_DIV_MODE             4
#define VO_MIN_HDMI_DIV             1
#define VO_MAX_HDMI_DIV             32

#define VO_LIMIT_USER_DISPWH_59V200 3840

#define VO_LIMIT_USER_DISPWH_OTHERS 1920
#define VO_SYNC_VACT_MIN 100

/* PLL FREF 24MHz */
#define VO_PLL_FREF 24

/* PLL frac precision  */
#define VO_PLL_FRAC_PREC (1 << 24)
#define VO_PLL_FOUTCVO_MIN 800
#define VO_PLL_FOUTCVO_MAX 3200
#define VO_PLL_FBDIV_MAX 0xfff
#define VO_PLL_FRAC_MAX 0xffffff
#define VO_PLL_REFDIV_MAX 0x3f
#define VO_PLL_POSTDIV1_MAX 0x7
#define VO_PLL_POSTDIV2_MAX 0x7

/* LCDMCLKDIV max ((75 / 1188) * 2^27 )(max 75Mhz)  */
#define VO_LCDMCLK_DIV_MAX 8473341

#define VO_DRV_DCMP_DATA_NUM 2
#define VO_DRV_DCMP_IP_NUM 2

#define VO_DRV_DCMP_EVEN_IP_OFFSET  0
#define VO_DRV_DCMP_ODD_IP_OFFSET  1
#define VO_DRV_DCMP_CHM_OFFSET_EVEN  2
#define VO_DRV_DCMP_CHM_OFFSET_ODD  3

#ifdef CONFIG_HI_VO_COVER_OSD_SUPPORT
#define VO_RGN_OVERLAY_LAYER_MIN 0
#define VO_RGN_OVERLAY_LAYER_MAX 0
#define VO_RGN_OVERLAY_PIXEL_FMT_NUM 3
#define VO_RGN_OVERLAY_FG_ALPHA_MAX 255
#define VO_RGN_OVERLAY_FG_ALPHA_MIN 0
#define VO_RGN_OVERLAY_BG_ALPHA_MAX 255
#define VO_RGN_OVERLAY_BG_ALPHA_MIN 0
#define VO_RGN_OVERLAY_GLOBAL_ALPHA_MAX 255
#define VO_RGN_OVERLAY_GLOBAL_ALPHA_MIN 0

#define VO_RGN_OVERLAY_QP_ABS_MAX 51
#define VO_RGN_OVERLAY_QP_ABS_MIN 0
#define VO_RGN_OVERLAY_QP_REL_MAX 51
#define VO_RGN_OVERLAY_QP_REL_MIN (-51)
#define VO_RGN_OVERLAY_STRIDE 64

#define VO_RGN_COVER_LAYER_MIN 0
#define VO_RGN_COVER_LAYER_MAX 0
#define VO_RGN_COVER_PIXEL_FMT_NUM 3
#define VO_RGN_COVER_FG_ALPHA_MAX 255
#define VO_RGN_COVER_FG_ALPHA_MIN 0
#define VO_RGN_COVER_BG_ALPHA_MAX 255
#define VO_RGN_COVER_BG_ALPHA_MIN 0
#define VO_RGN_COVER_GLOBAL_ALPHA_MAX 255
#define VO_RGN_COVER_GLOBAL_ALPHA_MIN 0

#define VO_RGN_COVER_QP_ABS_MAX 51
#define VO_RGN_COVER_QP_ABS_MIN 0
#define VO_RGN_COVER_QP_REL_MAX 51
#define VO_RGN_COVER_QP_REL_MIN (-51)
#define VO_RGN_COVER_STRIDE 64
#endif

#define VO_ASPECT_RATIO_RECT_XY_MIN 0

#define VO_ASPECT_RATIO_RECT_XYWH_ALIGN 2
#define VO_ASPECT_RATIO_BG_COLOR_MAX 0xffffff

#define VO_BORDER_WIDTH_MIN 2
#define VO_BORDER_WIDTH_MAX 8
#define VO_BORDER_WIDTH_ALIGN 2
#define VO_BORDER_COLOR_MASK 0xff000000

#define VO_LAYER_CSC_SCALE2P_DEF_VAL 0xa
#define VO_LAYER_CSC_CLIP_MIN 0x0
#define VO_LAYER_CSC_CLIP_MAX 0xfff

#define VO_MPP_CHN_DEV_DEF_VAL 0
#define VO_MPP_CHN_CHN_DEF_VAL 0

/* vou interrupt mask type */
typedef enum {
    VO_INTMSK_NONE = 0,
    VO_INTMSK_DHD0_VTTHD1 = 0x1,
    VO_INTMSK_DHD0_VTTHD2 = 0x2,
    VO_INTMSK_DHD0_VTTHD3 = 0x4,
    VO_INTMSK_DHD0_UFINT = 0x8,

    VO_INTMSK_DHD1_VTTHD1 = 0x10,
    VO_INTMSK_DHD1_VTTHD2 = 0x20,
    VO_INTMSK_DHD1_VTTHD3 = 0x40,
    VO_INTMSK_DHD1_UFINT = 0x80,

    VO_INTMSK_DSD_VTTHD1 = 0x100,
    VO_INTMSK_DSD_VTTHD2 = 0x200,
    VO_INTMSK_DSD_VTTHD3 = 0x400,
    VO_INTMSK_DSD_UFINT = 0x800,

    VO_INTMSK_B0_ERR = 0x1000,
    VO_INTMSK_B1_ERR = 0x2000,
    VO_INTMSK_B2_ERR = 0x4000,

    VO_INTMSK_WBC_DHDOVER = 0x8000,

    VO_INTREPORT_ALL = 0xffffffff
} vo_int_mask;

typedef struct {
    hi_bool int_ocurred;
    hi_bool detect_enabled;
} vo_drv_load_detect_info;

typedef struct {
    vo_dev_capability dev_cap;
    hi_bool vo_enable;
    hi_bool config;
    hi_vo_pub_attr vou_attr;

    hi_u32 layer_num;

    hi_u32 gfx_num;
    hi_graphic_layer gfx_layer[VO_MAX_GRAPHICS_LAYER_NUM];

    hi_u32 max_width;
    hi_u32 max_height;

    hi_bool dac_power_up;
    vo_drv_load_detect_info load_detect_info;
    hi_u32 low_bandwidth_cnt;
    hi_u64 bus_err;
    vo_dither_info dither_info;
} vo_drv_dev;

typedef struct {
    hi_bool video_enable;
    hi_bool video_config;

    hi_s32 bind_dev;

    hi_bool enlarge;

    hi_s32 priority;

    hi_u32 layer_size;
    hi_bool layer_limit;

    hi_u32 chn_num;
    hi_vo_video_layer_attr vo_video_attr;

    hi_rect zme_in_rect;
    hi_rect zme_out_rect;
    hi_vo_csc csc;
    hi_bool vhdr;
    hi_vo_csc vhdr_csc;
    csc_coef_param csc_param;
    hi_dynamic_range last_dynamic_range;
    hi_crop_info crop_info;
} vo_drv_layer;

typedef struct {
    hi_u32 vtth;
    hi_u32 vtth2;
    hi_u32 bk_grd;
    hi_vo_intf_type intf_type;
    hi_vo_intf_sync out_sync;
    hal_disp_pixel_format pixel_fmt;
} hal_dev_config;

typedef struct {
    hi_u32 bk_grd;
    vo_layer_ddr ddr_id;
} hal_layer_config;

vo_drv_dev g_vo_drv_dev[VO_MAX_DEV_NUM];
vo_drv_layer g_vo_drv_layer[VO_MAX_LAYER_NUM];

#define vo_drv_get_dev_ctx(dev) (&g_vo_drv_dev[(dev)])
#define vo_drv_get_layer_ctx(layer) (&g_vo_drv_layer[(layer)])

static hal_layer_config g_hal_layer_cfg[VO_LAYER_MAX_NUM] = {
    {
        .bk_grd = VO_BACKGROUD_DEFAULT,
        .ddr_id = VO_LAYER_DDR0, /* vhd0 */
    }
};

hal_disp_syncinfo g_sync_timing[VO_OUTPUT_BUTT] = {
    /*
     * |--INTFACE---||-----TOP-----||----HORIZON------||--BOTTOM---||-PULSE-||-INVERSE-|
     * synm, iop, itf,   vact, vbb,  vfb,  hact,  hbb,  hfb, hmid,bvact,bvbb, bvfb, hpw, vpw,idv, ihs, ivs
     */
    { 0, 0, 0, 288, 22, 2, 720, 132, 12, 1, 288, 23, 2, 126, 3, 0, 0, 0 }, /* 576I(PAL)  */
    { 0, 0, 0, 240, 18, 4, 720, 119, 19, 1, 240, 19, 4, 124, 3, 0, 0, 0 }, /* 480I(NTSC) */

    { 0, 1, 1, 1080, 41, 4, 1920, 192, 638, 1, 1, 1, 1, 44, 5, 0, 0, 0 }, /* 1080P@24_hz */
    { 0, 1, 1, 1080, 41, 4, 1920, 192, 528, 1, 1, 1, 1, 44, 5, 0, 0, 0 }, /* 1080P@25_hz */
    { 0, 1, 1, 1080, 41, 4, 1920, 192, 88,  1, 1, 1, 1, 44, 5, 0, 0, 0 }, /* 1080P@30_hz */

    { 0, 1, 1, 720,  25, 5, 1280, 260, 440, 1,    1,   1,  1, 40, 5, 0, 0, 0 }, /* 720P@50_hz */
    { 0, 1, 1, 720,  25, 5, 1280, 260, 110, 1,    1,   1,  1, 40, 5, 0, 0, 0 }, /* 720P@60_hz */
    { 0, 0, 1, 540,  20, 2, 1920, 192, 528, 1128, 540, 21, 2, 44, 5, 0, 0, 0 }, /* 1080I@50_hz */
    { 0, 0, 1, 540,  20, 2, 1920, 192, 88,  908,  540, 21, 2, 44, 5, 0, 0, 0 }, /* 1080I@60_hz */
    { 0, 1, 1, 1080, 41, 4, 1920, 192, 528, 1,    1,   1,  1, 44, 5, 0, 0, 0 }, /* 1080P@50_hz */
    { 0, 1, 1, 1080, 41, 4, 1920, 192, 88,  1,    1,   1,  1, 44, 5, 0, 0, 0 }, /* 1080P@60_hz */

    { 1, 1, 1, 576, 44, 5, 720, 132, 12, 1, 1, 1, 1, 64, 5, 0, 0, 0 }, /* 576P@50_hz */
    { 1, 1, 1, 480, 36, 9, 720, 122, 16, 1, 1, 1, 1, 62, 6, 0, 0, 0 }, /* 480P@60_hz */

    { 1, 1, 2, 600,  27, 1,  800,  216, 40,  1, 1, 1, 1, 128, 4, 0, 0, 0 }, /* 800*600@60_hz VGA@60_hz */
    { 1, 1, 2, 768,  35, 3,  1024, 296, 24,  1, 1, 1, 1, 136, 6, 0, 1, 1 }, /* 1024x768@60_hz */
    { 1, 1, 2, 1024, 41, 1,  1280, 360, 48,  1, 1, 1, 1, 112, 3, 0, 0, 0 }, /* 1280x1024@60_hz */
    { 1, 1, 2, 768,  27, 3,  1366, 356, 70,  1, 1, 1, 1, 143, 3, 0, 0, 0 }, /* 1366x768@60_hz */
    { 1, 1, 2, 900,  31, 3,  1440, 384, 80,  1, 1, 1, 1, 152, 6, 0, 1, 0 }, /* 1440x900@60_hz */
    { 1, 1, 2, 800,  28, 3,  1280, 328, 72,  1, 1, 1, 1, 128, 6, 0, 1, 0 }, /* 1280*800@60_hz VGA@60_hz */
    { 1, 1, 2, 1200, 49, 1,  1600, 496, 64,  1, 1, 1, 1, 192, 3, 0, 0, 0 }, /* 1600*1200@60_hz */
    { 1, 1, 2, 1050, 36, 3,  1680, 456, 104, 1, 1, 1, 1, 176, 6, 0, 1, 0 }, /* 1680*1050@60_hz */
    { 1, 1, 2, 1200, 32, 3,  1920, 112, 48,  1, 1, 1, 1, 32,  6, 0, 0, 1 }, /* 1920*1200@60_hz CVT (reduced blanking) */
    { 1, 1, 2, 480,  35, 10, 640,  144, 16,  1, 1, 1, 1, 96,  2, 0, 1, 1 }, /* 640*480@60_hz CVT */

    { 0, 0, 0, 288, 22, 2, 960, 176, 16, 1, 288, 23, 2, 168, 3, 0, 0, 0 }, /* 960H(PAL) */
    { 0, 0, 0, 240, 18, 4, 960, 163, 21, 1, 240, 19, 4, 168, 3, 0, 0, 0 }, /* 960H(NTSC) */

    { 0, 1, 1, 2160, 72, 8, 1920, 192, 88,   1, 1, 1, 1, 44, 5,  0, 0, 0 }, /* 1920*2160@30_hz */
    { 1, 1, 2, 1440, 39, 2, 2560, 112, 48,   1, 1, 1, 1, 32, 5,  0, 0, 0 }, /* 2560*1440@30_hz */
    { 1, 1, 2, 1440, 39, 2, 2560, 112, 48,   1, 1, 1, 1, 32, 5,  0, 0, 0 }, /* 2560*1440@60_hz */
    { 0, 1, 2, 1600, 43, 3, 2560, 112, 48,   1, 1, 1, 1, 32, 6,  0, 0, 1 }, /* 2560*1600@60_hz CVT (reduced blanking) */
    { 0, 1, 1, 2160, 82, 8, 3840, 384, 1276, 1, 1, 1, 1, 88, 10, 0, 0, 0 }, /* 3840*2160@24_hz */
    { 0, 1, 1, 2160, 82, 8, 3840, 384, 1056, 1, 1, 1, 1, 88, 10, 0, 0, 0 }, /* 3840*2160@25_hz */
    { 0, 1, 1, 2160, 82, 8, 3840, 384, 176,  1, 1, 1, 1, 88, 10, 0, 0, 0 }, /* 3840*2160@30_hz */
    { 0, 1, 1, 2160, 82, 8, 3840, 384, 1056, 1, 1, 1, 1, 88, 10, 0, 0, 0 }, /* 3840*2160@50_hz */
    { 0, 1, 1, 2160, 82, 8, 3840, 384, 176,  1, 1, 1, 1, 88, 10, 0, 0, 0 }, /* 3840*2160@60_hz */

    { 0,  1, 1, 2160, 82, 8,  4096, 384, 1020, 1, 1,   1,  1, 88, 10, 0, 0, 0 }, /* 4096x2160@24 */
    { 0,  1, 1, 2160, 82, 8,  4096, 216, 968,  1, 1,   1,  1, 88, 10, 0, 0, 0 }, /* 4096x2160@25 */
    { 0,  1, 1, 2160, 82, 8,  4096, 216, 88,   1, 1,   1,  1, 88, 10, 0, 0, 0 }, /* 4096x2160@30 */
    { 0,  1, 1, 2160, 82, 8,  4096, 216, 968,  1, 1,   1,  1, 88, 10, 0, 0, 0 }, /* 4096x2160@50 */
    { 0,  1, 1, 2160, 82, 8,  4096, 216, 88,   1, 1,   1,  1, 88, 10, 0, 0, 0 }, /* 4096x2160@60 */
    { 0,  1, 1, 240,  15, 9,  320,  65,  7,    1, 240, 14, 9, 1,  1,  0, 0, 0 }, /* 320X240@60  8bit LCD */
    { 0,  1, 1, 240,  2,  2,  320,  5,   10,   1, 1,   1,  1, 10, 1,  0, 0, 0 }, /* 320X240@50  6bit LCD */
    { 0,  1, 1, 320,  10, 4,  240,  30,  10,   1, 1,   1,  1, 10, 2,  0, 0, 0 }, /* 240X320@50  6bit LCD */
    { 0,  1, 1, 320,  2,  2,  240,  20,  10,   1, 1,   1,  1, 2,  1,  0, 0, 0 }, /* 240X320@60 16bit LCD */
    { 0,  1, 1, 600,  23, 12, 800,  210, 46,   1, 1,   1,  1, 2,  1,  0, 0, 0 }, /* 800X600@50 24bit LCD */

    { 0,  1, 1, 1280, 24,  8,  720,  123, 99,  1, 1, 1, 1, 24,  4,  0, 0, 0 }, /* for MIPI DSI tx 720 x1280 at 60 hz */
    { 0,  1, 1, 1920, 36,  16, 1080, 28,  130, 1, 1, 1, 1, 8,   10, 0, 0, 0 }, /* for MIPI DSI tx 1080 x1920 at 60 hz */
    { 0,  1, 1, 4320, 64,  16, 7680, 768, 552, 1, 1, 1, 1, 176, 20, 0, 0, 0 }, /* 7680x4320@30 */
    {} /* user */
};

static hal_dev_config g_hal_dev_cfg[VO_DEV_MAX_NUM] = {
    {
        .vtth = VO_HD_VTTH_WATERLINE,
        .vtth2 = VO_HD_VTTH_WATERLINE2,
        .bk_grd = VO_BACKGROUD_DEFAULT,
        .intf_type = VO_INTF_HDMI | VO_INTF_BT1120 | VO_INTF_BT656 | VO_INTF_MIPI | VO_INTF_MIPI_SLAVE | VO_INTF_LCD,
        .out_sync = VO_OUTPUT_1080P30,
        .pixel_fmt = HAL_INPUTFMT_YCBCR_SEMIPLANAR_422, /* VOU_PIXERL_FORMAT_RGB888, VOU_PIXERL_FORMAT_YCBCR422 */
    }
};

vo_coef_addr g_vo_coef_buf_addr = {0};

vo_coef_addr *vo_drv_get_coef_buf_addr(hi_void)
{
    return &g_vo_coef_buf_addr;
}

static hal_disp_layer vou_drv_convert_layer(hi_vo_layer layer)
{
    hal_disp_layer disp_layer = HAL_DISP_LAYER_BUTT;

    switch (layer) {
        case VO_HAL_LAYER_VHD0:
            disp_layer = HAL_DISP_LAYER_VHD0;
            break;

        default:
            break;
    }

    return disp_layer;
}

static hal_disp_pixel_format vou_drv_convert_data_format(vou_layer_pixerl_format data_fmt)
{
    hal_disp_pixel_format pixel_format = HAL_DISP_PIXELFORMAT_BUTT;

    switch (data_fmt) {
        case VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_400:
            pixel_format = HAL_INPUTFMT_YCBCR_SEMIPLANAR_400;
            break;
        case VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_420:
            pixel_format = HAL_INPUTFMT_YCBCR_SEMIPLANAR_420;
            break;
        case VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_422:
            pixel_format = HAL_INPUTFMT_YCBCR_SEMIPLANAR_422;
            break;
        default:
            break;
    }

    return pixel_format;
}

static hi_void vou_drv_set_layer_cvfir_coef(hi_u32 layer, vo_rm_coef_mode coef_mode)
{
    cvfir_coef coef = {0, 63, 0, 0, -6, 12, 51, 7};

    hi_unused(coef_mode);
    hal_video_cvfir_set_coef(layer, &coef);
}

hi_void vo_drv_v_set_v0_zme_coef(vo_rm_coef_mode coef_mode)
{
    vou_drv_set_layer_cvfir_coef(HAL_DISP_LAYER_VHD0, coef_mode);
}

hi_void vou_drv_layer_csc_enable(hi_vo_layer layer, hi_bool csc_en)
{
    hal_layer_set_csc_en(layer, csc_en);
    return;
}

hi_void vou_drv_board_init(hi_void)
{
    hal_vou_init();
    return;
}

hi_void vou_drv_board_de_init(hi_void)
{
    hal_vou_exit();
}

hi_void vo_drv_int_reg_up_mode(hi_vo_layer layer, vo_int_mode int_mode)
{
    hal_video_set_layer_up_mode(layer, int_mode);
    return;
}

hi_void vou_drv_set_dev_intf_type(hi_vo_dev dev, hi_vo_intf_type intf_type)
{
    g_hal_dev_cfg[dev].intf_type = intf_type;

    return;
}

hi_void vou_drv_set_dev_bk_grd(hi_vo_dev dev, hi_u32 bg_color)
{
    g_hal_dev_cfg[dev].bk_grd = bg_color;

    return;
}

hi_void vou_drv_set_dev_out_sync(hi_vo_dev dev, hi_vo_intf_sync intf_sync)
{
    g_hal_dev_cfg[dev].out_sync = intf_sync;

    return;
}

hi_void vou_drv_set_dev_default_vtth(hi_vo_dev dev)
{
    if (vo_drv_is_phy_dev_hd(dev) == HI_TRUE) {
        g_hal_dev_cfg[dev].vtth = VO_HD_VTTH_WATERLINE;
    } else {
        g_hal_dev_cfg[dev].vtth = VO_SD_VTTH_WATERLINE;
    }

    return;
}

hi_s32 vou_drv_set_dev_vtth(hi_vo_dev dev, hi_u32 vtth)
{
    hi_u32 dft_vtth;

    dft_vtth = (vo_drv_is_phy_dev_hd(dev)) ? VO_HD_VTTH_WATERLINE : VO_SD_VTTH_WATERLINE;
    g_hal_dev_cfg[dev].vtth = MAX2(vtth, dft_vtth);

    return HI_SUCCESS;
}

hi_void vou_drv_get_dev_vtth(hi_vo_dev dev, hi_u32 *vtth)
{
    *vtth = g_hal_dev_cfg[dev].vtth;
    return;
}

hi_bool vou_drv_is_support_save_buf_mode(hi_void)
{
    return HI_TRUE;
}

hi_void vou_drv_set_dev_default_vtth2(hi_vo_dev dev)
{
    hi_vo_mod_param *vo_mod_param = vo_get_vo_mod_param();

    if (dev == VO_DEV_DHD0) {
        if (vo_mod_param->save_buf_mode[dev]) {
            if (vo_drv_is_phy_dev_hd(dev) == HI_TRUE) {
                g_hal_dev_cfg[dev].vtth2 = VO_HD_VTTH_WATERLINE2;
            } else {
                g_hal_dev_cfg[dev].vtth2 = VO_SD_VTTH_WATERLINE2;
            }
        }
    }
    return;
}

hi_s32 vou_drv_set_dev_vtth2(hi_vo_dev dev, hi_u32 vtth)
{
    hi_u32 dft_vtth2;
    hi_u32 vtth1 = 0;
    hi_u32 tmp;

    dft_vtth2 = (vo_drv_is_phy_dev_hd(dev)) ? VO_HD_VTTH_WATERLINE2 : VO_SD_VTTH_WATERLINE2;
    vou_drv_get_dev_vtth(dev, &vtth1);
    tmp = MAX2(vtth, dft_vtth2);

    g_hal_dev_cfg[dev].vtth2 = MIN2(tmp, vtth1);

    return HI_SUCCESS;
}

hi_void vou_drv_get_dev_vtth2(hi_vo_dev dev, hi_u32 *vtth)
{
    if (dev <= VO_DEV_DHD0) {
        *vtth = g_hal_dev_cfg[dev].vtth2;
    }

    return;
}

hi_bool vou_drv_get_dev_enable(hi_vo_dev dev)
{
    hi_bool intf_en = HI_FALSE;

    hal_disp_get_intf_enable(dev, &intf_en);
    return intf_en;
}

hi_s32 vou_drv_check_usr_sync_timing(hi_vo_dev dev, hi_vo_sync_info *sync_info)
{
    if (((sync_info->synm != HI_TRUE) && (sync_info->synm != HI_FALSE)) ||
        ((sync_info->iop != HI_TRUE) && (sync_info->iop != HI_FALSE)) ||
        ((sync_info->idv != HI_TRUE) && (sync_info->idv != HI_FALSE)) ||
        ((sync_info->ihs != HI_TRUE) && (sync_info->ihs != HI_FALSE)) ||
        ((sync_info->ivs != HI_TRUE) && (sync_info->ivs != HI_FALSE))) {
        vo_err_trace("DHD%d: sync's synm(%d),iop(%d),idv(%d),ihs(%d),ivs(%d) should be %d or %d\n",
            dev, sync_info->synm, sync_info->iop, sync_info->idv, sync_info->ihs, sync_info->ivs, HI_TRUE, HI_FALSE);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->hact == 0) || (sync_info->hbb == 0) || (sync_info->hfb == 0)) {
        vo_err_trace("usr sync_time hact(%u) hbb(%u) hfb(%u) can't be 0!\n", sync_info->hact, sync_info->hbb,
                     sync_info->hfb);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->vact < VO_SYNC_VACT_MIN) || (sync_info->vbb == 0) || (sync_info->vfb == 0)) {
        vo_err_trace("usr sync_time vact(%u) can't be smaller than %d and vbb(%u) vfb(%u) can't be 0!\n",
                     sync_info->vact, VO_SYNC_VACT_MIN, sync_info->vbb, sync_info->vfb);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->hpw == 0) || (sync_info->vpw == 0)) {
        vo_err_trace("usr sync_time hpw(%u) vpw(%u) can't be 0!\n", sync_info->hpw, sync_info->vpw);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->iop == 0) && ((sync_info->bvact == 0) || (sync_info->bvbb == 0) || (sync_info->bvfb == 0))) {
        vo_err_trace("usr sync_time vact(%u) vbb(%u) vfb(%u) can't be 0 when interlaced timing !\n",
                     sync_info->vact, sync_info->vbb, sync_info->vfb);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

hi_void vou_drv_set_usr_sync_timing(hi_vo_sync_info *sync_info)
{
    g_sync_timing[VO_OUTPUT_USER].synm = sync_info->synm;
    g_sync_timing[VO_OUTPUT_USER].iop = sync_info->iop;
    g_sync_timing[VO_OUTPUT_USER].intfb = sync_info->intfb;
    g_sync_timing[VO_OUTPUT_USER].vact = sync_info->vact;
    g_sync_timing[VO_OUTPUT_USER].vbb = sync_info->vbb;
    g_sync_timing[VO_OUTPUT_USER].vfb = sync_info->vfb;
    g_sync_timing[VO_OUTPUT_USER].hact = sync_info->hact;
    g_sync_timing[VO_OUTPUT_USER].hbb = sync_info->hbb;
    g_sync_timing[VO_OUTPUT_USER].hfb = sync_info->hfb;
    g_sync_timing[VO_OUTPUT_USER].hmid = sync_info->hmid;
    g_sync_timing[VO_OUTPUT_USER].bvact = sync_info->bvact;
    g_sync_timing[VO_OUTPUT_USER].bvbb = sync_info->bvbb;
    g_sync_timing[VO_OUTPUT_USER].bvfb = sync_info->bvfb;
    g_sync_timing[VO_OUTPUT_USER].hpw = sync_info->hpw;
    g_sync_timing[VO_OUTPUT_USER].vpw = sync_info->vpw;
    g_sync_timing[VO_OUTPUT_USER].idv = sync_info->idv;
    g_sync_timing[VO_OUTPUT_USER].ihs = sync_info->ihs;
    g_sync_timing[VO_OUTPUT_USER].ivs = sync_info->ivs;
}

hi_s32 vo_drv_check_dev_pll_param(hi_vo_dev dev, hi_vo_user_intfsync_pll *pll)
{
    if ((pll->fbdiv > VO_PLL_FBDIV_MAX) ||
        (pll->frac > VO_PLL_FRAC_MAX) ||
        (pll->refdiv > VO_PLL_REFDIV_MAX) ||
        (pll->refdiv == 0) ||
        (pll->postdiv1 > VO_PLL_POSTDIV1_MAX) ||
        (pll->postdiv1 == 0) ||
        (pll->postdiv2 > VO_PLL_POSTDIV2_MAX) ||
        (pll->postdiv2 == 0)) {
        vo_err_trace("dev(%d) pll param (fbdiv,frac,refdiv,postdiv1,postdiv2)="
            "(0x%x,0x%x,0x%x,0x%x,0x%x) illegal .\n",
            dev, pll->fbdiv, pll->frac, pll->refdiv, pll->postdiv1, pll->postdiv2);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 vo_drv_check_dev_pll_postdiv(hi_vo_dev dev, hi_vo_user_intfsync_pll *pll)
{
    if (pll->postdiv1 < pll->postdiv2) {
        vo_err_trace("dev(%d) pll postdiv (postdiv1,postdiv2)="
            "(0x%x,0x%x) illegal.\n",
            dev, pll->postdiv1, pll->postdiv2);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 vo_drv_check_dev_pll_foutvco(hi_vo_dev dev, hi_vo_user_intfsync_pll *pll)
{
    hi_u32 foutcvo;

    /* 0x01000000: 2^24 */
    foutcvo = VO_PLL_FREF * pll->fbdiv / pll->refdiv + VO_PLL_FREF * pll->frac / VO_PLL_FRAC_PREC / pll->refdiv;
    if (foutcvo < VO_PLL_FOUTCVO_MIN || foutcvo > VO_PLL_FOUTCVO_MAX) {
        vo_err_trace("dev(%d) pll foutcvo (fbdiv,frac,refdiv)="
            "(0x%x,0x%x,0x%x) illegal .\n",
            dev, pll->fbdiv, pll->frac, pll->refdiv);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 vo_drv_check_dev_pll(hi_vo_dev dev, hi_vo_user_intfsync_pll *pll)
{
    hi_s32 pll_check;
    pll_check = vo_drv_check_dev_pll_param(dev, pll);
    if (pll_check != HI_SUCCESS) {
        return HI_FAILURE;
    }
    pll_check = vo_drv_check_dev_pll_postdiv(dev, pll);
    if (pll_check != HI_SUCCESS) {
        return HI_FAILURE;
    }
    pll_check = vo_drv_check_dev_pll_foutvco(dev, pll);
    if (pll_check != HI_SUCCESS) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 vo_drv_check_dev_lcdmclkdiv(hi_vo_dev dev, hi_u32 lcd_m_clk_div)
{
    if (lcd_m_clk_div > VO_LCDMCLK_DIV_MAX || lcd_m_clk_div == 0) {
        vo_err_trace("dev(%d) lcd_mclk value %d illegal, it must be in (0,%d].\n",
            dev, lcd_m_clk_div, VO_LCDMCLK_DIV_MAX);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 vo_drv_check_dev_clksource(hi_vo_dev dev, hi_vo_clk_source cs)
{
    if (cs == VO_CLK_SOURCE_PLL) {
        /* support pll. */
    } else if (cs == VO_CLK_SOURCE_LCDMCLK) {
        /* support lcdmclk. */
    } else {
        vo_err_trace("dev(%d) clk source %d illegal.\n", dev, cs);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 vo_drv_check_dev_clkvalue(hi_vo_dev dev, hi_vo_user_intfsync_info *user_info)
{
    hi_vo_user_intfsync_attr *user_intf_sync_attr = &user_info->user_intf_sync_attr;
    hi_vo_user_intfsync_pll *user_intf_sync_pll = &user_info->user_intf_sync_attr.user_sync_pll;
    hi_s32 value_check;

    if (user_intf_sync_attr->clk_source == VO_CLK_SOURCE_PLL) {
    value_check = vo_drv_check_dev_pll(dev, user_intf_sync_pll);
    } else if (user_intf_sync_attr->clk_source == VO_CLK_SOURCE_LCDMCLK) {
        value_check = vo_drv_check_dev_lcdmclkdiv(dev, user_intf_sync_attr->lcd_m_clk_div);
    } else {
        return HI_FAILURE;
    }

    return value_check;
}

hi_s32 vo_drv_check_dev_user_sync_info(hi_vo_dev dev, hi_vo_user_intfsync_info *user_info)
{
    hi_vo_user_intfsync_attr *user_intf_sync_attr = &user_info->user_intf_sync_attr;
    hi_s32 info_check;

    if (dev != VO_DEV_DHD0) {
        vo_err_trace("dev(%d) is illegal.\n", dev);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    info_check = vo_drv_check_dev_clksource(dev, user_intf_sync_attr->clk_source);
    if (info_check != HI_SUCCESS) {
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    info_check = vo_drv_check_dev_clkvalue(dev, user_info);
    if (info_check != HI_SUCCESS) {
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((user_info->dev_div > VO_MAX_DIV_MODE) || (user_info->dev_div < VO_MIN_DIV_MODE)) {
        vo_err_trace("VO(%d) div mode %d is illegal,it must be in [%d,%d].\n", dev, user_info->dev_div,
                     VO_MIN_DIV_MODE, VO_MAX_DIV_MODE);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((user_info->pre_div > VO_MAX_HDMI_DIV) || (user_info->pre_div < VO_MIN_HDMI_DIV)) {
        vo_err_trace("VO(%d) pre div %d is illegal,it must be in [%d,%d].\n", dev, user_info->pre_div,
                     VO_MIN_HDMI_DIV, VO_MAX_HDMI_DIV);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }

    if ((user_info->clk_reverse != HI_TRUE) && (user_info->clk_reverse != HI_FALSE)) {
        vo_err_trace("VO(%d) clk_reverse:%d is illegal, it must be HI_TRUE or HI_FALSE.\n", dev,
                     user_info->clk_reverse);
        return HI_ERR_VO_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

hi_void vo_drv_set_dev_user_intf_sync_attr(hi_vo_dev dev, hi_vo_user_intfsync_attr *user_attr)
{
    hi_mpp_chn mpp_chn = {0};
    hi_u32 fbdiv;
    hi_u32 frac;
    hi_u32 refdiv;
    hi_u32 postdiv1;
    hi_u32 postdiv2;
    hi_vo_user_intfsync_pll *user_sync_pll = HI_NULL;
    hi_u32 vdp_out_clk_sel = 0x0;
    hi_u32 lcd_mclk_div = 0x015E4C3;

    hi_bool clk_en = HI_TRUE;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.dev_id = dev;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    user_sync_pll = &user_attr->user_sync_pll;
    fbdiv = user_sync_pll->fbdiv;
    frac = user_sync_pll->frac;
    refdiv = user_sync_pll->refdiv;
    postdiv1 = user_sync_pll->postdiv1;
    postdiv2 = user_sync_pll->postdiv2;
    if (dev == VO_DEV_DHD0) {
        if (user_attr->clk_source == VO_CLK_SOURCE_PLL) {
            vdp_out_clk_sel = 0x0;
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);

            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_FBDIV_SET, &fbdiv);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_FRAC_SET, &frac);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_REFDIV_SET, &refdiv);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POSTDIV1_SET, &postdiv1);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POSTDIV2_SET, &postdiv2);
        } else if (user_attr->clk_source == VO_CLK_SOURCE_LCDMCLK) {
            vdp_out_clk_sel = 0x6;
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);
            lcd_mclk_div = user_attr->lcd_m_clk_div;
            call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_MCLK_DIV, &lcd_mclk_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);
        }
    } else {
        vo_err_trace("VO dev %d is illegal.\n", dev);
    }
    return;
}

hi_void vo_drv_set_dev_div(hi_vo_dev dev, hi_u32 dev_div)
{
    hi_mpp_chn mpp_chn = {0};
    hi_u32 div_mode = dev_div - 1;
    hi_vo_intf_type intf_type;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.dev_id = dev;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;

    if (dev == VO_DEV_DHD0) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_HD0_DIV_MOD, &div_mode);
    }

    intf_type = g_hal_dev_cfg[dev].intf_type;

    if ((VO_INTF_LCD_6BIT & intf_type) || (VO_INTF_LCD_8BIT & intf_type)) {
        if (dev_div == 3) {
            hal_disp_set_lcd_serial_perd(0);
        } else {
            hal_disp_set_lcd_serial_perd(1);
        }
    }
    return;
}

hi_void vo_drv_set_hdmi_div(hi_vo_dev dev, hi_u32 pre_div)
{
    hi_mpp_chn mpp_chn = {0};
    hi_u32 pre_divmode;

    pre_divmode = pre_div - 1;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.dev_id = dev;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HDMI_CLK_SEL, &pre_divmode);
}

hi_void vo_drv_set_clk_reverse(hi_bool reverse)
{
    hi_mpp_chn mpp_chn = {0};

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.dev_id = VO_MPP_CHN_DEV_DEF_VAL;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;

    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_HD_CLKOUT_PHASIC_REVERSE_EN, &reverse);
    return;
}

/* interrupt relative */
hi_void vo_drv_dev_int_enable(hi_vo_dev dev, hi_bool enable)
{
    hi_u32 int_type;
    hi_u32 hifb_int_type;

    if (dev != VO_DEV_DHD0) {
        return;
    }
    int_type = VO_INTMSK_DHD0_VTTHD1;
    hifb_int_type = VO_INTMSK_DHD0_VTTHD3 | VO_INTMSK_DHD0_VTTHD2;

    if (enable == HI_TRUE) {
        hal_disp_set_int_mask(int_type);
        hal_disp_set_int_mask1(hifb_int_type);
    } else {
        hal_disp_clr_int_mask(int_type);
        hal_disp_clr_int_mask1(hifb_int_type);
    }

    return;
}

hi_void vou_drv_int_clear(hi_u32 int_clear)
{
    hal_disp_clear_int_status(int_clear);
}

hi_void vou_drv_int_disable_all(hi_void)
{
    hal_disp_clr_int_mask(VO_INTREPORT_ALL);
}

hi_void vo_drv_int_set_mode(hi_vo_dev dev, vo_int_mode int_mode)
{
    hal_disp_set_vt_thd_mode(dev, int_mode);

    return;
}

static hi_s32 vou_drv_check_dev_intf(hi_vo_dev dev, hi_vo_intf_type intf_type)
{
    HI_ASSERT((dev == VO_DEV_DHD0));

    if ((intf_type & ~(VO_INTF_HDMI | VO_INTF_BT1120 | VO_INTF_BT656 | VO_INTF_MIPI | VO_INTF_LCD_6BIT |
                       VO_INTF_LCD_8BIT | VO_INTF_LCD_16BIT | VO_INTF_LCD_18BIT | VO_INTF_LCD_24BIT)) ||
        (intf_type == 0)) {
        vo_err_trace("device HD%d does not support intf(%d) !\n", dev, intf_type);
        return HI_ERR_VO_NOT_SUPPORT;
    }

    if ((intf_type & ~VO_INTF_HDMI) && (intf_type & ~VO_INTF_BT1120) && (intf_type & ~VO_INTF_BT656) &&
        (intf_type & ~VO_INTF_MIPI) && (intf_type & ~VO_INTF_LCD_6BIT) && (intf_type & ~VO_INTF_LCD_8BIT) &&
        (intf_type & ~VO_INTF_LCD_16BIT) && (intf_type & ~VO_INTF_LCD_18BIT) && (intf_type & ~VO_INTF_LCD_24BIT)) {
        vo_err_trace("this chip do not support more than one intfcace be used at the same time!\n");
        return HI_ERR_VO_NOT_SUPPORT;
    }

    return HI_SUCCESS;
}

static hi_s32 vou_drv_check_hdmi_sync(hi_vo_dev dev, hi_vo_intf_type intf_type, hi_vo_intf_sync intf_sync)
{
    if (intf_type & VO_INTF_HDMI) {
        if (!(((intf_sync >= VO_OUTPUT_1080P24) && (intf_sync <= VO_OUTPUT_640x480_60)) ||
              ((intf_sync >= VO_OUTPUT_1920x2160_30) && (intf_sync <= VO_OUTPUT_3840x2160_30)))) {
            vo_err_trace("for HDMI intface, vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }

        if ((intf_sync >= VO_OUTPUT_1920x2160_30) && (intf_sync <= VO_OUTPUT_3840x2160_30)) {
            vo_err_trace("the intf_sync is not support!");
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    return HI_SUCCESS;
}

static hi_s32 vou_drv_check_mipi_sync(hi_vo_dev dev, hi_vo_intf_type intf_type, hi_vo_intf_sync intf_sync)
{
    if ((intf_type & VO_INTF_MIPI)) {
        if (!((intf_sync >= VO_OUTPUT_720x1280_60) && (intf_sync <= VO_OUTPUT_1080x1920_60))) {
            vo_err_trace("for MIPI OR MIPI_SLAVE intface, vo%d's intfsync %d illegal!\n",
                         dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    return HI_SUCCESS;
}

static hi_s32 vou_drv_check_bt1120_sync(hi_vo_dev dev, hi_vo_intf_type intf_type, hi_vo_intf_sync intf_sync)
{
    if (intf_type & VO_INTF_BT1120) {
        if ((intf_sync < VO_OUTPUT_1080P24) || (intf_sync > VO_OUTPUT_640x480_60)) {
            vo_err_trace("for BT1120 intface, vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    return HI_SUCCESS;
}

static hi_s32 vou_drv_check_bt656_sync(hi_vo_dev dev, hi_vo_intf_type intf_type, hi_vo_intf_sync intf_sync)
{
    if ((intf_type & VO_INTF_BT656)) {
        if ((intf_sync != VO_OUTPUT_PAL) && (intf_sync != VO_OUTPUT_NTSC)) {
            vo_err_trace("for BT656 intface,vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    return HI_SUCCESS;
}

static hi_s32 vou_drv_check_lcd_sync(hi_vo_dev dev, hi_vo_intf_type intf_type, hi_vo_intf_sync intf_sync)
{
    if (intf_type & VO_INTF_LCD_8BIT) {
        if (intf_sync != VO_OUTPUT_320x240_60) {
            vo_err_trace("for LCD 8bit intface,vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    if (intf_type & VO_INTF_LCD_6BIT) {
        if ((intf_sync < VO_OUTPUT_320x240_50) || (intf_sync > VO_OUTPUT_240x320_50)) {
            vo_err_trace("for LCD 6bit intface,vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    if (intf_type & VO_INTF_LCD_16BIT) {
        if (intf_sync != VO_OUTPUT_240x320_60) {
            vo_err_trace("for LCD 16bit intface,vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    if (intf_type & VO_INTF_LCD_18BIT) {
        if (intf_sync != VO_OUTPUT_USER) {
            vo_err_trace("for LCD 18bit intface,vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }

    if (intf_type & VO_INTF_LCD_24BIT) {
        if (intf_sync != VO_OUTPUT_800x600_50) {
            vo_err_trace("for LCD 24bit intface,vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return HI_ERR_VO_NOT_SUPPORT;
        }
    }
    return HI_SUCCESS;
}

static hi_s32 vou_drv_check_dev_sync(hi_vo_dev dev, hi_vo_intf_type intf_type, hi_vo_intf_sync intf_sync)
{
    hi_s32 ret;
    if (intf_sync == VO_OUTPUT_USER) {
        /*
         * user defined sync will be welcome for all interface type
         * WARNING: user must make sure to config right sync, otherwise, screen doesn't't work!
         */
        return HI_SUCCESS;
    }

    if ((intf_sync == VO_OUTPUT_960H_PAL) || (intf_sync == VO_OUTPUT_960H_NTSC)) {
        vo_err_trace("vo%d's intfsync %d illegal, vo doesn't support interlace sync %d!\n", dev,
                     intf_sync, intf_sync);
        return HI_ERR_VO_NOT_SUPPORT;
    }

    ret = vou_drv_check_hdmi_sync(dev, intf_type, intf_sync);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = vou_drv_check_mipi_sync(dev, intf_type, intf_sync);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = vou_drv_check_bt1120_sync(dev, intf_type, intf_sync);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = vou_drv_check_bt656_sync(dev, intf_type, intf_sync);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = vou_drv_check_lcd_sync(dev, intf_type, intf_sync);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return HI_SUCCESS;
}

hi_s32 vou_drv_check_dev_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr)
{
    hi_s32 ret;

    ret = vou_drv_check_dev_intf(dev, pub_attr->intf_type);
    if (ret != HI_SUCCESS) {
        vo_err_trace("vo%d check device's interface %d illegal!\n", dev, pub_attr->intf_type);
        return ret;
    }

    ret = vou_drv_check_dev_sync(dev, pub_attr->intf_type, pub_attr->intf_sync);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return HI_SUCCESS;
}

hi_s32 vou_drv_set_dev_full_frame_rate(hi_vo_dev dev, hi_u32 frame_rate)
{
    hi_unused(dev);
    hi_unused(frame_rate);
    return HI_SUCCESS;
}

hi_s32 vou_drv_set_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr)
{
    vo_drv_dev *drv_dev_ctx = HI_NULL;
    drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    (hi_void)memcpy_s(&drv_dev_ctx->vou_attr, sizeof(hi_vo_pub_attr), pub_attr, sizeof(hi_vo_pub_attr));
    drv_dev_ctx->config = HI_TRUE;

    return HI_SUCCESS;
}

hi_s32 vou_drv_set_user_sync_info(hi_vo_dev dev, hi_vo_user_intfsync_info *info)
{
    hi_unused(dev);
    hi_unused(info);
    return HI_SUCCESS;
}

hi_void vou_drv_layer_enable(hi_vo_layer layer, hi_bool enable)
{
    hal_video_hfir_set_ck_gt_en(layer, enable);
    hal_layer_enable_layer(layer, enable);
    return;
}

hi_void vou_drv_set_layer_bk_grd(hi_vo_layer layer, hi_u32 bk_grd)
{
    g_hal_layer_cfg[layer].bk_grd = bk_grd;
    return;
}

hi_void vou_drv_set_layer_data_fmt(hi_vo_layer layer, vou_layer_pixerl_format data_fmt)
{
    hal_disp_layer hal_layer;
    hal_disp_pixel_format disp_data_fmt;

    hal_layer = vou_drv_convert_layer(layer);
    disp_data_fmt = vou_drv_convert_data_format(data_fmt);
    hal_layer_set_layer_data_fmt(hal_layer, disp_data_fmt);
    return;
}

hi_void vou_drv_get_layer_data_fmt(hi_vo_layer layer, vou_layer_pixerl_format *data_fmt)
{
    hal_disp_layer hal_layer;
    hal_disp_pixel_format disp_data_fmt;

    hal_layer = vou_drv_convert_layer(layer);

    hal_layer_get_layer_data_fmt(hal_layer, &disp_data_fmt);

    switch (disp_data_fmt) {
        case HAL_INPUTFMT_YCBCR_SEMIPLANAR_400:
            *data_fmt = VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_400;
            break;
        case HAL_INPUTFMT_YCBCR_SEMIPLANAR_420:
            *data_fmt = VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_420;
            break;
        case HAL_INPUTFMT_YCBCR_SEMIPLANAR_422:
            *data_fmt = VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_422;
            break;
        default:
            vo_warn_trace("layer(%d) get hal data format %d\n", layer, disp_data_fmt);
            break;
    }
}

hi_bool vou_drv_is_virt_dev(hi_vo_dev dev)
{
    hi_unused(dev);
    return HI_FALSE;
}

hi_void vo_drv_get_dev_hal_id(hi_vo_dev dev, vo_hal_dev *hal_dev)
{
    switch (dev) {
        case VO_DEV_INDEX0:
            *hal_dev = VO_DEV_DHD0;
            break;
        default:
            vo_err_trace("dev %d is illegal.\n", dev);
            break;
    }
}

hi_void vo_drv_get_channel_hal_id(hi_vo_dev dev, hal_disp_outputchannel *vo_channel)
{
    switch (dev) {
        case VO_CHANNEL_INDEX0:
            *vo_channel = HAL_DISP_CHANNEL_DHD0;
            break;
        default:
            vo_err_trace("dev %d is illegal.\n", dev);
            break;
    }
}

hi_void vou_drv_def_layer_bind_dev(hi_void)
{
    hal_cbm_set_cbm_mixer_prio(HAL_DISP_LAYER_VHD0, VOU_MIX_PRIO0, HAL_CBMMIX1);
    hal_cbm_set_cbm_mixer_prio(HAL_DISP_LAYER_GFX0, VOU_MIX_PRIO1, HAL_CBMMIX1);

    return;
}

hi_void vo_drv_set_dev_bg_color(hi_vo_dev dev, hi_u32 bg_color, hi_bool rgb)
{
    hal_disp_bkcolor bkg = {0};
    hi_u32 yuv_bk_grd = 0;
    if (rgb) {
        /* for 24bit value. */
        bkg.bkg_y = RGB_R(bg_color);
        bkg.bkg_cb = RGB_G(bg_color);
        bkg.bkg_cr = RGB_B(bg_color);
        bkg.bkg_y = bkg.bkg_y << 2;
        bkg.bkg_cb = bkg.bkg_cb << 2;
        bkg.bkg_cr = bkg.bkg_cr << 2;
    } else {
        yuv_bk_grd = rgb2_yuv_full(bg_color);
        bkg.bkg_y = YUV_Y(yuv_bk_grd);
        bkg.bkg_cb = YUV_U(yuv_bk_grd);
        bkg.bkg_cr = YUV_V(yuv_bk_grd);
    }

    if (dev == VO_DEV_DHD0) {
        hal_cbm_set_cbm_bkg(HAL_CBMMIX1, &bkg);
    } else if (dev == VO_DEV_DHD1) {
        hal_cbm_set_cbm_bkg(HAL_CBMMIX2, &bkg);
    }
}

static hi_void vo_drv_set_cbm_bkg(hi_vo_dev dev)
{
    hal_disp_bkcolor bkg;
    hi_u32 yuv_bk_grd = 0;
    hi_vo_intf_type intf_type;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    intf_type = hal_dev_cfg->intf_type;

    if ((VO_INTF_LCD & intf_type) || (VO_INTF_LCD_6BIT & intf_type) || (VO_INTF_LCD_8BIT & intf_type) ||
        (VO_INTF_LCD_16BIT & intf_type) || (VO_INTF_LCD_18BIT & intf_type) || (VO_INTF_LCD_24BIT & intf_type) ||
        (VO_INTF_MIPI & intf_type) || (VO_INTF_MIPI_SLAVE & intf_type)) {
        /* for 24bit value. */
        bkg.bkg_y = RGB_R(hal_dev_cfg->bk_grd);
        bkg.bkg_cb = RGB_G(hal_dev_cfg->bk_grd);
        bkg.bkg_cr = RGB_B(hal_dev_cfg->bk_grd);
        bkg.bkg_y = bkg.bkg_y << 2;
        bkg.bkg_cb = bkg.bkg_cb << 2;
        bkg.bkg_cr = bkg.bkg_cr << 2;
    } else {
        yuv_bk_grd = rgb2_yuv_full(hal_dev_cfg->bk_grd);
        bkg.bkg_y = YUV_Y(yuv_bk_grd);
        bkg.bkg_cb = YUV_U(yuv_bk_grd);
        bkg.bkg_cr = YUV_V(yuv_bk_grd);
    }

    hal_cbm_set_cbm_bkg(HAL_CBMMIX1, &bkg);
}

static hi_void vo_drv_get_intf_property(hi_vo_dev dev, hal_disp_syncinfo *sync_info, hal_disp_syncinv *inv)
{
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];

    (hi_void)memcpy_s(sync_info, sizeof(hal_disp_syncinfo),
        &g_sync_timing[hal_dev_cfg->out_sync], sizeof(hal_disp_syncinfo));
    inv->hs_inv = sync_info->ihs ? 1 : 0;
    inv->vs_inv = sync_info->ivs ? 1 : 0;
    inv->dv_inv = sync_info->idv ? 1 : 0;
}

static hi_void vo_drv_set_intf_hdmi_cfg(hi_vo_dev dev, hal_disp_syncinv *inv)
{
    hi_mpp_chn mpp_chn;
    hi_bool hdmi_clk_en = HI_TRUE;
    hi_bool hdmi_tx_clk_en = HI_TRUE;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HDMI_CLK_EN, &hdmi_clk_en);  /* [10] VDP hdmi_cken */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HDMI_TX_CLK_EN, &hdmi_tx_clk_en);  /* [18] VDP hdmi_tx_cken */
    if ((g_hal_dev_cfg[dev].out_sync == VO_OUTPUT_576P50) ||
        (g_hal_dev_cfg[dev].out_sync == VO_OUTPUT_480P60)) {
        inv->hs_inv = 1 - inv->hs_inv;
        inv->vs_inv = 1 - inv->vs_inv;
    }
}

static hi_void vo_drv_set_intf_mipi_cfg(hi_vo_dev dev, hal_disp_syncinv *inv)
{
    hi_mpp_chn mpp_chn;
    U_INTF_LCD_CTRL lcd_ctrl;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_bool mipi_clk_en = HI_TRUE;
    hi_bool mipi_tx_clk_en = HI_TRUE;
    hi_vo_intf_type intf_type;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    intf_type = hal_dev_cfg->intf_type;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    vo_drv_get_channel_hal_id(dev, &vo_channel);

    if (VO_INTF_MIPI & intf_type) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_CLK_EN, &mipi_clk_en);  /* [11] VDP mipi_cken */
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_TX_CLK_EN, &mipi_tx_clk_en);  /* [19] VDP mipi_tx_cken */

        hal_disp_set_intf_mux_sel(vo_channel, VO_INTF_MIPI);

        lcd_ctrl.bits.hdmi_mode = 1;
        hal_disp_set_intf_ctrl(intf_type, &(lcd_ctrl.u32));

        hal_intf_bt_set_dfir_en(0x1);

        g_vo_drv_layer[dev].csc.csc_matrix = VO_CSC_MATRIX_BT709_TO_RGB_PC;

        inv->hs_inv = 0;
        inv->vs_inv = 0;
        inv->dv_inv = 0;
    }
}

static hi_void vo_drv_set_intf_bt1120_cfg(hi_vo_dev dev)
{
    hi_mpp_chn mpp_chn;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_bool bt_clk_en = HI_TRUE;
    hi_u32 data_mode;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &bt_clk_en);   /*  [8][9] bt clk en ,bt bp en. */

    hal_disp_set_intf_mux_sel(vo_channel, VO_INTF_BT1120);

    /* for bt1120 interface image effect. */
    if (vo_get_transparent_transmit() == HI_TRUE)  {
        hal_intf_bt_set_dfir_en(0x0);
    } else {
        hal_intf_bt_set_dfir_en(0x1);
    }
    data_mode = 5; /* 5, bt1120 mode */
    call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_DATA_MODE, &data_mode);
}

static hi_void vo_drv_set_intf_bt656_cfg(hi_vo_dev dev, hi_u32 *div_mode)
{
    hi_mpp_chn mpp_chn;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_bool bt_clk_en = HI_TRUE;
    hi_u32 data_mode;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &bt_clk_en);

    hal_disp_set_intf_mux_sel(vo_channel, VO_INTF_BT656);
    hal_intf_bt_set_dfir_en(0x1);
    *div_mode = 1;
    data_mode = 6; /* 6, bt656 mode */
    call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_DATA_MODE, &data_mode);
}

static hi_void vo_drv_set_intf_lcd_ctrl(hi_vo_intf_type intf_type, hi_u32 *div_mode,
                                        hi_bool *clk_reverse, hi_u32 *data_mode, U_INTF_LCD_CTRL *lcd_ctrl)
{
    if (VO_INTF_LCD_8BIT & intf_type) {
        *div_mode = 3;
        *data_mode = 3;
        lcd_ctrl->bits.hdmi_mode = 1;
        lcd_ctrl->bits.lcd_serial_mode = 1;
        lcd_ctrl->bits.lcd_serial_perd = 1;
        lcd_ctrl->bits.lcd_parallel_order = 0;
        lcd_ctrl->bits.lcd_data_inv = 0;
        lcd_ctrl->bits.lcd_parallel_mode = 1;  /* 0:RGB565, 1:RGB888 */
    } else if (VO_INTF_LCD_6BIT & intf_type) {
        *div_mode = 2;
        *data_mode = 4;

        lcd_ctrl->bits.lcd_serial_mode = 1;
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.lcd_parallel_order = 0;
        lcd_ctrl->bits.lcd_data_inv = 0;
        lcd_ctrl->bits.lcd_parallel_mode = 0;
    } else if (VO_INTF_LCD_16BIT & intf_type) {
        *div_mode = 0;
        *data_mode = 2;

        lcd_ctrl->bits.lcd_serial_mode = 0;
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.lcd_parallel_order = 0;
        lcd_ctrl->bits.lcd_data_inv = 0;
        lcd_ctrl->bits.lcd_parallel_mode = 1;
    } else if (VO_INTF_LCD_18BIT & intf_type) {
        *div_mode = 0;
        *data_mode = 1;

        lcd_ctrl->bits.lcd_serial_mode = 0;
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.lcd_parallel_order = 0;
        lcd_ctrl->bits.lcd_data_inv = 0;
        lcd_ctrl->bits.lcd_parallel_mode = 1;
    } else if (VO_INTF_LCD_24BIT & intf_type) {
        *clk_reverse = HI_FALSE;
        *div_mode = 0;
        *data_mode = 0;

        lcd_ctrl->bits.lcd_serial_mode = 0;
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.lcd_parallel_order = 0;
        lcd_ctrl->bits.lcd_data_inv = 0;
        lcd_ctrl->bits.lcd_parallel_mode = 1;
    }
}

static hi_void vo_drv_set_intf_lcd_cfg(hi_vo_dev dev, hi_bool *clk_reverse, hi_u32 *div_mode)
{
    hi_u32 data_mode;
    hi_mpp_chn mpp_chn;
    U_INTF_LCD_CTRL lcd_ctrl;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_bool bt_clk_en = HI_TRUE;
    hi_bool clk_en = HI_TRUE;

    hi_vo_intf_type intf_type;
    hi_vo_intf_sync out_sync;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    intf_type = hal_dev_cfg->intf_type;
    out_sync = hal_dev_cfg->out_sync;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &bt_clk_en);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);

    hal_disp_set_intf_mux_sel(vo_channel, intf_type);

    vo_drv_set_intf_lcd_ctrl(intf_type, div_mode, clk_reverse, &data_mode, &lcd_ctrl);

    call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_DATA_MODE, &data_mode);

    hal_disp_set_intf_ctrl(intf_type, &(lcd_ctrl.u32));

    if (out_sync != VO_OUTPUT_USER) {
        hal_disp_set_lcd_serial_perd(lcd_ctrl.bits.lcd_serial_perd);
    }

    g_vo_drv_layer[dev].csc.csc_matrix = VO_CSC_MATRIX_BT709_TO_RGB_PC;
}

static hi_void vo_drv_set_intf_cfg(hi_vo_dev dev, hal_disp_syncinv *inv, hi_bool *clk_reverse, hi_u32 *div_mode)
{
    hi_vo_intf_type intf_type;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    intf_type = hal_dev_cfg->intf_type;

    if (VO_INTF_HDMI & intf_type) {
        vo_drv_set_intf_hdmi_cfg(dev, inv);
    }

    if ((VO_INTF_MIPI & intf_type) || (VO_INTF_MIPI_SLAVE & intf_type)) {
        vo_drv_set_intf_mipi_cfg(dev, inv);
    }

    if (VO_INTF_BT1120 & intf_type) {
        vo_drv_set_intf_bt1120_cfg(dev);
    }

    if (VO_INTF_BT656 & intf_type) {
        vo_drv_set_intf_bt656_cfg(dev, div_mode);
    }

    if ((VO_INTF_LCD & intf_type) || (VO_INTF_LCD_6BIT & intf_type) || (VO_INTF_LCD_8BIT & intf_type) ||
        (VO_INTF_LCD_16BIT & intf_type) || (VO_INTF_LCD_18BIT & intf_type) || (VO_INTF_LCD_24BIT & intf_type)) {
        vo_drv_set_intf_lcd_cfg(dev, clk_reverse, div_mode);
    }
}

static hi_void vo_drv_set_dev_out_reverse(hi_vo_dev dev, hi_bool clk_reverse)
{
    hi_mpp_chn mpp_chn;
    hi_vo_intf_sync out_sync;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    out_sync = hal_dev_cfg->out_sync;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    if (out_sync != VO_OUTPUT_USER) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_HD_CLKOUT_PHASIC_REVERSE_EN, &clk_reverse);
    }
}

static hi_void vo_drv_set_div_mod(hi_vo_dev dev, hi_u32 *div_mode)
{
    hi_mpp_chn mpp_chn;
    hi_vo_intf_sync out_sync;
    vo_hal_dev hal_dev = VO_DEV_BUTT;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    out_sync = hal_dev_cfg->out_sync;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    /* get the id of vo device in hal level */
    vo_drv_get_dev_hal_id(dev, &hal_dev);

    if (out_sync != VO_OUTPUT_USER) {
        if (hal_dev == VO_DEV_DHD0) {
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_HD0_DIV_MOD, div_mode);
        }
    }
}

static hi_void vo_drv_set_intf_clip(hi_vo_dev dev)
{
    hi_vo_intf_type intf_type;
    hal_dev_config *hal_dev_cfg = HI_NULL;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    intf_type = hal_dev_cfg->intf_type;

    if (VO_INTF_HDMI & intf_type) {
        hal_disp_clip clip_data = { 0x10, 0x10, 0x10, 0x3ff, 0x3ff, 0x3ff };
        hal_disp_set_intf_clip(VO_INTF_HDMI, HI_TRUE, &clip_data);
    }

    if (VO_INTF_BT1120 & intf_type) {
        hal_disp_clip clip_data = {0x40, 0x40, 0x40, 0x3ac, 0x3c0, 0x3c0};
        hal_disp_set_intf_clip(VO_INTF_BT1120, HI_TRUE, &clip_data);
    }

    if (VO_INTF_BT656 & intf_type) {
        hal_disp_clip clip_data = {0x40, 0x40, 0x40, 0x3ac, 0x3c0, 0x3c0};
        hal_disp_set_intf_clip(VO_INTF_BT656, HI_TRUE, &clip_data);
    }
}

static hi_void vo_drv_set_vtth(hi_vo_dev dev, hal_disp_syncinfo *sync_info)
{
    hi_u32 vtth_temp = 0;
    hi_u32 vtth_temp2 = 0;
    hi_u16 vert_all = 0;
    hi_u16 vtth_line2 = 0;
    hi_u16 vtth_line = 0;
    hi_u16 vtth_min = VO_HD_MIN_VTTH_WATERLINE;
    hal_dev_config *hal_dev_cfg = HI_NULL;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;

    hal_dev_cfg = &g_hal_dev_cfg[dev];

    vo_drv_get_channel_hal_id(dev, &vo_channel);

    if (vo_drv_is_phy_dev_hd(dev) || vo_drv_is_phy_dev_sd(dev)) {
        if (vo_drv_is_phy_dev_hd(dev)) {
            vtth_min = VO_HD_MIN_VTTH_WATERLINE;
        } else if (vo_drv_is_phy_dev_sd(dev)) {
            vtth_min = VO_SD_MIN_VTTH_WATERLINE;
        }

        if (sync_info->iop == 0) {
            vo_drv_int_set_mode(dev, VO_INT_MODE_FIELD);
            vo_drv_int_reg_up_mode(dev, VO_INT_MODE_FIELD);
        } else {
            vo_drv_int_set_mode(dev, VO_INT_MODE_FRAME);
            vo_drv_int_reg_up_mode(dev, VO_INT_MODE_FRAME);
        }

        vtth_temp = hal_dev_cfg->vtth;
        vtth_temp2 = hal_dev_cfg->vtth;
        vert_all = sync_info->vact + sync_info->vfb + sync_info->vbb;

        if (vtth_temp >= vert_all) {
            /* argu 0 means : set to default value. */
            vou_drv_set_dev_vtth(dev, 0);
            vo_err_trace("VO device %d vtthd : %d is not in logic [%d,%d], set to default minimum: %d\n",
                         dev, vtth_temp, vtth_min, vert_all - 1, hal_dev_cfg->vtth);
        }

        if (vtth_temp2 >= vert_all) {
            /* argu 0 means : set to default value. */
            vou_drv_set_dev_vtth2(dev, 0);
            vo_err_trace("VO device %d vtthd2 : %d is not in logic [%d,%d], set to default minimum: %d\n",
                         dev, vtth_temp2, vtth_min, vert_all - 1, hal_dev_cfg->vtth);
        }

        vtth_line = vert_all - 1 - (hi_u16)vtth_temp;
        vtth_line2 = vert_all - 1 - (hi_u16)vtth_temp2;
    }

    hal_disp_set_vt_thd(vo_channel, vtth_line, vtth_line2);
}

hi_void vo_drv_open(hi_vo_dev dev)
{
    hal_disp_syncinfo sync_info;
    hal_disp_syncinv inv = {0};
    hi_mpp_chn mpp_chn;
    hi_bool clk_en = HI_TRUE;
    hi_bool clk_reverse = HI_TRUE;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hal_dev_config *hal_dev_cfg = HI_NULL;
    hi_vo_intf_type intf_type;
    hi_u32 div_mode = 0;

    hal_dev_cfg = &g_hal_dev_cfg[dev];
    intf_type = hal_dev_cfg->intf_type;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    vo_drv_get_channel_hal_id(dev, &vo_channel);

    vo_drv_set_all_crg_clk(HI_TRUE);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_EN, &clk_en);

    vo_drv_set_cbm_bkg(dev);

    vou_drv_init_default_csc(dev);

    vo_drv_get_intf_property(dev, &sync_info, &inv);

    vo_drv_set_intf_cfg(dev, &inv, &clk_reverse, &div_mode);

    vo_drv_set_dev_out_reverse(dev, clk_reverse);
    vo_drv_set_div_mod(dev, &div_mode);

    hal_disp_set_intf_sync(vo_channel, &sync_info, &inv);

    hal_disp_set_dev_multi_chn_en(vo_channel, HAL_MULTICHN_EN_1P1C);

    vo_drv_set_intf_clip(dev);

    vo_drv_set_vtth(dev, &sync_info);

    if ((VO_INTF_LCD_6BIT & intf_type) || (VO_INTF_LCD_16BIT & intf_type) || (VO_INTF_LCD_18BIT & intf_type)) {
        vou_drv_set_dev_dither_out(dev, 6); /* 6, 6bit */
    }
    vou_drv_set_dev_dither(dev, 10); /* 10, 10bit */

    hal_disp_set_intf_enable(vo_channel, HI_TRUE);

    vo_drv_dev_int_enable(dev, HI_TRUE);

    hal_disp_set_reg_up(vo_channel);

    return;
}

hi_void vo_drv_close(hi_vo_dev dev)
{
    hi_mpp_chn mpp_chn;
    hi_bool clk_en = HI_FALSE;
    hi_u64 timeval = 0;
    hi_u64 time_begin;

    hi_u32 vdp_out_clk_sel = 0x0;
    hi_u32 vdp_hd_clk_sel = 0x0;
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_vo_intf_type intf_type = g_hal_dev_cfg[dev].intf_type;

    hal_disp_set_intf_enable(dev, HI_FALSE);
    vo_drv_dev_int_enable(dev, HI_FALSE);
    hal_disp_set_reg_up(dev);
    vo_drv_get_channel_hal_id(dev, &vo_channel);

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = dev;

    time_begin = get_sys_time_by_usec();

    do {
        timeval = get_sys_time_by_usec() - time_begin;
        if (timeval >= 50000) { /* 50000us */
            break;
        }
    } while (hal_disp_get_reg_up(dev) != 0);

    if (VO_INTF_HDMI & intf_type) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HDMI_CLK_EN, &clk_en);
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HDMI_TX_CLK_EN, &clk_en);
    }

    if ((VO_INTF_BT656 & g_hal_dev_cfg[dev].intf_type) || (VO_INTF_BT1120 & g_hal_dev_cfg[dev].intf_type)) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_EN, &clk_en);
        hal_disp_set_intf_mux_sel(vo_channel, HAL_DISP_INTF_BUTT);
    }

    if (VO_INTF_MIPI & intf_type) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_CLK_EN, &clk_en);
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_TX_CLK_EN, &clk_en); /* [19] VDP mipi_tx_cken */

        hal_disp_set_intf_mux_sel(vo_channel, HAL_DISP_INTF_BUTT);
    }

    if ((VO_INTF_LCD & intf_type) || (VO_INTF_LCD_6BIT & intf_type) || (VO_INTF_LCD_8BIT & intf_type) ||
        (VO_INTF_LCD_16BIT & intf_type) || (VO_INTF_LCD_18BIT & intf_type) || (VO_INTF_LCD_24BIT & intf_type)) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &clk_en);

        hal_disp_set_intf_mux_sel(vo_channel, HAL_DISP_INTF_BUTT);
    }
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_HD_CLK_SEL, &vdp_hd_clk_sel);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_DEV_CLK_EN, &clk_en);

    return;
}

hi_void vo_drv_enable(hi_vo_dev dev)
{
    g_vo_drv_dev[dev].vo_enable = HI_TRUE;
}

hi_void vo_drv_disable(hi_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = &g_vo_drv_dev[dev];

    drv_dev_ctx->config = HI_FALSE;
    drv_dev_ctx->vo_enable = HI_FALSE;
    (hi_void)memset_s(&drv_dev_ctx->vou_attr, sizeof(hi_vo_pub_attr), 0, sizeof(hi_vo_pub_attr));
}

hi_void vou_drv_default_setting(hi_void)
{
    hi_u32 i;
    hfir_coef h_coef = { 42, -10, -20, 28, -40, 61, -107, 330 };

    for (i = LAYER_VHD_START; i <= LAYER_VHD_END; i++) {
        /* set each graphic layer global alpha */
        hal_layer_set_layer_g_alpha(i, 255); /* global alpha max 255 */
        hal_video_set_layer_alpha(i, 255); /* alpha max 255 */

        /* set video layer hfir enable */
        hal_video_hfir_set_hfir_mode(i, HAL_HFIRMODE_COPY);

        hal_video_hfir_set_coef(i, &h_coef);
        hal_video_hfir_set_mid_en(i, HI_TRUE);
    }

    /* set each cross bar default PRI */
    vou_drv_def_layer_bind_dev();

    hal_sys_set_outstanding();

    hal_disp_clear_int_status(VO_INTREPORT_ALL);

    /* only set video layer */
    for (i = 0; i <= LAYER_VHD_END; i++) {
        vou_drv_layer_enable(i, HI_FALSE);
        vou_drv_set_layer_data_fmt(i, VOU_LAYER_PIXERL_FORMAT_SP_YCBCR_422);
        vou_drv_set_layer_bk_grd(i, VO_BACKGROUD_DEFAULT);
    }

    return;
}

hi_void vo_drv_transparent_transmit_setting(hi_bool transparent_transmit)
{
    hi_unused(transparent_transmit);
}

hi_s32 vou_drv_check_dev_id(hi_vo_dev dev)
{
    if ((dev < 0) || (dev >= VO_MAX_DEV_NUM)) {
        return HI_ERR_VO_INVALID_DEVID;
    }

    return HI_SUCCESS;
}

static hi_void vo_drv_dev_capability_init(hi_vo_dev dev)
{
    vo_dev_capability *dev_cap = NULL;
    dev_cap = &g_vo_drv_dev[dev].dev_cap;

    switch (dev) {
        case VO_DEV_DHD0: {
            dev_cap->dev_type = VO_UHD_HW_DEV;
            dev_cap->hd_vpss_rciv = HI_FALSE;
            dev_cap->sd_tde_disp = HI_TRUE;
            dev_cap->support_wbc = HI_TRUE;
            dev_cap->spt_get_screen_frm = HI_TRUE;
            break;
        }

        default:
            vo_err_trace("unknown dev %d!!\n", dev);
            break;
    }

    return;
}

hi_void vo_drv_dev_info_init(hi_void)
{
    hi_s32 i;
    (hi_void)memset_s(g_vo_drv_dev, sizeof(vo_drv_dev) * VO_MAX_DEV_NUM,
        0, sizeof(vo_drv_dev) * VO_MAX_DEV_NUM);

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        vo_drv_dev_capability_init(i);
        if (i == VO_DEV_DHD0) {
            /* 2 video layer and 1 graphic layer in DHD0 */
            g_vo_drv_dev[i].layer_num = 1;
            g_vo_drv_dev[i].gfx_num = 1;
            g_vo_drv_dev[i].dither_info.dither_out_bit_width = 8; /* 8, 8bit */
        }
    }

    return;
}

hi_void vo_drv_dev_info_part_init(hi_void)
{
    hi_s32 i;

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        vo_drv_dev_capability_init(i);
        if (i == VO_DEV_DHD0) {
            /* 2 video layer and 1 graphic layer in DHD0 */
            g_vo_drv_dev[i].layer_num = 1; /* 1 video layer */
            g_vo_drv_dev[i].gfx_num = 1; /* 1 gfx layer */
            g_vo_drv_dev[i].dither_info.dither_out_bit_width = 8; /* 8 bit */
        }
    }

    return;
}

hi_void vo_drv_dev_info_de_init(hi_void)
{
    return;
}

hi_bool vou_drv_special_work_flow(hi_vo_dev dev)
{
    hi_unused(dev);
    return HI_FALSE;
}

hi_bool vo_drv_is_phy_dev_hd(hi_vo_dev dev)
{
    vo_dev_capability *dev_cap = &g_vo_drv_dev[dev].dev_cap;
    if ((dev_cap->dev_type == VO_UHD_HW_DEV) ||
        (dev_cap->dev_type == VO_HD_HW_DEV)) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

hi_bool vo_drv_is_phy_dev_sd(hi_vo_dev dev)
{
    if (g_vo_drv_dev[dev].dev_cap.dev_type == VO_SD_HW_DEV) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

hi_void vou_drv_set_disp_max_size(hi_vo_dev dev, hi_u32 max_width, hi_u32 max_height)
{
    g_vo_drv_dev[dev].max_width = max_width;
    g_vo_drv_dev[dev].max_height = max_height;
}

static hi_void vou_drv_get_25_175_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 67;
    sync_pll->frac = 5035000;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0xf;
}

static hi_void vou_drv_get_27_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 72;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0xf;
}

static hi_void vou_drv_get_40_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 160;
    sync_pll->frac = 0;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0xf;
}

static hi_void vou_drv_get_65_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 130;
    sync_pll->frac = 0;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x7;
}

static hi_void vou_drv_get_74_25_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 99;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x7;
}

static hi_void vou_drv_get_83_5_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 114;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x7;
}

static hi_void vou_drv_get_85_5_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 167;
    sync_pll->frac = 0;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x7;
}

static hi_void vou_drv_get_106_5_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 71;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_108_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 72;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_120_8496_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 1;

    sync_pll->fbdiv = 119;
    sync_pll->frac = 6295000;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_146_25_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 146;
    sync_pll->frac = 4195000;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;

    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_148_5_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 99;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_154_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 154;
    sync_pll->frac = 0;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_162_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 162;
    sync_pll->frac = 0;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x3;
}

static hi_void vou_drv_get_241_6992_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 119;
    sync_pll->frac = 6295000;
    sync_pll->refdiv = 3;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x1;
}

static hi_void vou_drv_get_268_5_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 537;
    sync_pll->frac = 0;
    sync_pll->refdiv = 12;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x1;
}

static hi_void vou_drv_get_297_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 99;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x1;
}

static hi_void vou_drv_get_594_config(hi_u32 *vdp_hd_clk_sel, vo_intfsync_pll *sync_pll)
{
    *vdp_hd_clk_sel = 0;

    sync_pll->fbdiv = 99;
    sync_pll->frac = 0;
    sync_pll->refdiv = 2;
    sync_pll->postdiv1 = 2;
    sync_pll->postdiv2 = 1;
    sync_pll->hdmiclk_div = 0x0;
}

static hi_void vou_drv_set_pll_timing(hi_u32 vdp_hd_clk_sel, hi_mpp_chn *mpp_chn, vo_intfsync_pll *sync_pll)
{
    hi_u32 vdp_out_clk_sel = 0x0;

    call_sys_drv_ioctrl(mpp_chn, SYS_VOU_HD_CLK_SEL, &vdp_hd_clk_sel);
    call_sys_drv_ioctrl(mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);

    call_sys_drv_ioctrl(mpp_chn, SYS_VO_HDMI_CLK_SEL, &sync_pll->hdmiclk_div);
    call_sys_drv_ioctrl(mpp_chn, SYS_VO_PLL_FBDIV_SET, &sync_pll->fbdiv);
    call_sys_drv_ioctrl(mpp_chn, SYS_VO_PLL_FRAC_SET, &sync_pll->frac);
    call_sys_drv_ioctrl(mpp_chn, SYS_VO_PLL_REFDIV_SET, &sync_pll->refdiv);
    call_sys_drv_ioctrl(mpp_chn, SYS_VO_PLL_POSTDIV1_SET, &sync_pll->postdiv1);
    call_sys_drv_ioctrl(mpp_chn, SYS_VO_PLL_POSTDIV2_SET, &sync_pll->postdiv2);
}

hi_s32 vou_drv_set_dev_clk(hi_vo_dev dev)
{
    hi_bool clk_en = HI_TRUE;
    hi_mpp_chn mpp_chn = {0};
    vo_drv_dev *dev_ctx = HI_NULL;
    hi_u32 vdp_hd_clk_sel = 0x0;
    hi_u32 vdp_out_clk_sel = 0x0;
    hi_u32 lcd_mclk_div = 0x015E4C3;
    vo_intfsync_pll sync_pll;
    dev_ctx = vo_drv_get_dev_ctx(dev);

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.dev_id = dev;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;

    if (dev_ctx->vou_attr.intf_sync == VO_OUTPUT_USER) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_HD_CLK_SEL, &vdp_hd_clk_sel);

        return HI_SUCCESS;
    }

    switch (dev_ctx->vou_attr.intf_sync) {
        case VO_OUTPUT_PAL:
        case VO_OUTPUT_NTSC:
        case VO_OUTPUT_576P50:
        case VO_OUTPUT_480P60: {
            vou_drv_get_27_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1080P24:
        case VO_OUTPUT_1080P25:
        case VO_OUTPUT_1080P30:
        case VO_OUTPUT_720P50:
        case VO_OUTPUT_720P60:
        case VO_OUTPUT_1080I50:
        case VO_OUTPUT_1080I60:
        case VO_OUTPUT_720x1280_60: {  /* for MIPI tx interface. */
            vou_drv_get_74_25_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1080P50:
        case VO_OUTPUT_1080P60:
        case VO_OUTPUT_1920x2160_30:
        case VO_OUTPUT_1080x1920_60: {  /* for MIPI tx interface. */
            vou_drv_get_148_5_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_640x480_60: {
            vou_drv_get_25_175_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_800x600_60: {
            vou_drv_get_40_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1024x768_60: {
            vou_drv_get_65_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1280x1024_60: {
            vou_drv_get_108_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1366x768_60: {
            vou_drv_get_85_5_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1440x900_60: {
            vou_drv_get_106_5_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1280x800_60: {
            vou_drv_get_83_5_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1600x1200_60: {
            vou_drv_get_162_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1680x1050_60: {
            vou_drv_get_146_25_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_1920x1200_60: {
            vou_drv_get_154_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_320x240_60: {
            vdp_out_clk_sel = 0x6;
            lcd_mclk_div = 0x2aE4C3;
            call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_MCLK_DIV, &lcd_mclk_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);
            break;
        }
        case VO_OUTPUT_320x240_50: {
            vdp_out_clk_sel = 0x6;
            lcd_mclk_div = 0x152306;
            call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_MCLK_DIV, &lcd_mclk_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);
            break;
        }
        case VO_OUTPUT_240x320_50: {
            vdp_out_clk_sel = 0x6;
            lcd_mclk_div = 0x182ed6;
            call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_MCLK_DIV, &lcd_mclk_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);
            break;
        }
        case VO_OUTPUT_240x320_60: {
            vdp_out_clk_sel = 0x6;
            lcd_mclk_div = 0x90c54;
            call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_MCLK_DIV, &lcd_mclk_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);
            break;
        }
        case VO_OUTPUT_800x600_50: {
            vdp_out_clk_sel = 0x6;
            lcd_mclk_div = 0x3a9cc2;
            call_sys_drv_ioctrl(&mpp_chn, SYS_LCD_MCLK_DIV, &lcd_mclk_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_OUT_CLK_SEL, &vdp_out_clk_sel);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_LCD_CLK_EN, &clk_en);
            break;
        }
        case VO_OUTPUT_2560x1440_30: {
            vou_drv_get_120_8496_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_2560x1440_60: {
            vou_drv_get_241_6992_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_2560x1600_60: {
            vou_drv_get_268_5_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_3840x2160_24:
        case VO_OUTPUT_3840x2160_25:
        case VO_OUTPUT_3840x2160_30: {
            vou_drv_get_297_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        case VO_OUTPUT_7680x4320_30:
        case VO_OUTPUT_3840x2160_60: {
            vou_drv_get_594_config(&vdp_hd_clk_sel, &sync_pll);
            vou_drv_set_pll_timing(vdp_hd_clk_sel, &mpp_chn, &sync_pll);
            break;
        }
        default: {
            return HI_SUCCESS;
        }
    }

    return HI_SUCCESS;
}

hi_void vo_drv_set_all_crg_clk(hi_bool clk_en)
{
    hi_mpp_chn mpp_chn;
    hi_bool vo_clk_en = clk_en;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    mpp_chn.dev_id = VO_MPP_CHN_DEV_DEF_VAL;

    /* dev 0 :[4][5][6] sd ppc,hd ppc,hd0 clk; dev 1 :[4][5][7] sd ppc, hd ppc, hd1 clk */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_DEV_CLK_EN, &vo_clk_en);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_CORE_CLK_EN, &vo_clk_en);  /* [5] hd ppc clk */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_CFG_CLK_EN, &vo_clk_en);  /* [3] CFG clk */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_BUS_CLK_EN, &vo_clk_en);  /* [2] AXI bus */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_APB_CLK_EN, &vo_clk_en);  /* [1] APB clk */

    return;
}

hi_void vo_drv_set_hd_clk_sel(hi_u32 hd_clk_sel)
{
    hi_unused(hd_clk_sel);
}

hi_void vo_lpw_bus_reset(hi_bool reset)
{
    hi_mpp_chn mpp_chn;
    hi_bool vo_reset = reset;

    mpp_chn.mod_id = HI_ID_VO;
    mpp_chn.dev_id = VO_MPP_CHN_DEV_DEF_VAL;
    mpp_chn.chn_id = VO_MPP_CHN_CHN_DEF_VAL;
    /* see sys_hal_vou_bus_reset_sel */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VOU_BUS_RESET_SEL, &vo_reset);

    return;
}

hi_void vo_set_low_power_ctrl_clk_en(hi_bool clk_en)
{
    hi_unused(clk_en);
}

hi_void vou_drv_init_default_csc(hi_vo_layer layer)
{
    hi_s32 i;
    hi_vo_csc *csc = HI_NULL;

    hi_unused(layer);

    for (i = 0; i < VO_MAX_LAYER_NUM; i++) {
        csc = &g_vo_drv_layer[i].csc;
        csc->csc_matrix = VO_CSC_MATRIX_IDENTITY;
        csc->luma = VO_CSC_DEF_VAL;
        csc->contrast = VO_CSC_DEF_VAL;
        csc->hue = VO_CSC_DEF_VAL;
        csc->satuature = VO_CSC_DEF_VAL;

        csc = &g_vo_drv_layer[i].vhdr_csc;
        csc->csc_matrix = VO_CSC_MATRIX_BT709_TO_RGB_PC;
        csc->luma = VO_CSC_DEF_VAL;
        csc->contrast = VO_CSC_DEF_VAL;
        csc->hue = VO_CSC_DEF_VAL;
        csc->satuature = VO_CSC_DEF_VAL;
    }

    return;
}

static hi_void vo_drv_set_dither_sed(hi_vo_dev dev, vdp_dither_mode dither_mode_set)
{
    hal_disp_dihter_sed dither_sed = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_u32 dither_thr_max = 0;
    hi_u32 dither_thr_min = 0;

    vo_drv_get_channel_hal_id(dev, &vo_channel);

    if (dither_mode_set == VDP_DITHER_MODE_TYP) {
        dither_sed.dither_sed_y0 = 2147483647;
        dither_sed.dither_sed_u0 = 2147482647;
        dither_sed.dither_sed_v0 = 2147481647;
        dither_sed.dither_sed_w0 = 2147480647;
        dither_sed.dither_sed_y1 = 2147483647;
        dither_sed.dither_sed_u1 = 2147482647;
        dither_sed.dither_sed_v1 = 2147481647;
        dither_sed.dither_sed_w1 = 2147480647;
        dither_sed.dither_sed_y2 = 2147483647;
        dither_sed.dither_sed_u2 = 2147482647;
        dither_sed.dither_sed_v2 = 2147481647;
        dither_sed.dither_sed_w2 = 2147480647;
        dither_sed.dither_sed_y3 = 2147483647;
        dither_sed.dither_sed_u3 = 2147482647;
        dither_sed.dither_sed_v3 = 2147481647;
        dither_sed.dither_sed_w3 = 2147480647;
        dither_thr_max = 60000;
        dither_thr_min = 120;
    } else if (dither_mode_set == VDP_DITHER_MODE_MAX) {
        dither_sed.dither_sed_y0 = 2147483647;
        dither_sed.dither_sed_u0 = 2147483647;
        dither_sed.dither_sed_v0 = 2147483647;
        dither_sed.dither_sed_w0 = 2147483647;
        dither_sed.dither_sed_y1 = 2147483647;
        dither_sed.dither_sed_u1 = 2147483647;
        dither_sed.dither_sed_v1 = 2147483647;
        dither_sed.dither_sed_w1 = 2147483647;
        dither_sed.dither_sed_y2 = 2147483647;
        dither_sed.dither_sed_u2 = 2147483647;
        dither_sed.dither_sed_v2 = 2147483647;
        dither_sed.dither_sed_w2 = 2147483647;
        dither_sed.dither_sed_y3 = 2147483647;
        dither_sed.dither_sed_u3 = 2147483647;
        dither_sed.dither_sed_v3 = 2147483647;
        dither_sed.dither_sed_w3 = 2147483647;
        dither_thr_max = 0;
        dither_thr_min = 0;
    }

    hal_disp_set_dither_sed(vo_channel, &dither_sed);
    hal_disp_set_dither_thr_min_max(vo_channel, dither_thr_min, dither_thr_max);
}

hi_void vou_drv_set_dev_dither_mode(hi_vo_dev dev, vdp_dither_mode dither_mode_set, vdp_dither_cfg *cfg)
{
    hal_disp_outputchannel vo_channel = HAL_DISP_CHANNEL_BUTT;
    hi_u32 i_data_width_dither = 0;
    hi_u32 o_data_width_dither = 0;
    hi_u32 dither_io_md;
    hi_u32 dither_md = 0;

    dither_io_md = cfg->io_mode;

    if (dither_io_md == DITHER_IO_MODE_12_10) {
        dither_md = DITHER_MODE_10BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_12BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_10BIT;
    } else if (dither_io_md == DITHER_IO_MODE_12_8) {
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_12BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_8BIT;
    } else if (dither_io_md == DITHER_IO_MODE_10_8) {
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_10BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_8BIT;
    } else if (dither_io_md == DITHER_IO_MODE_10_6) {
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_10BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_6BIT;
    } else if (dither_io_md == DITHER_IO_MODE_9_6) {
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_9BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_6BIT;
    } else if (dither_io_md == DITHER_IO_MODE_8_6) {
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_8BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_6BIT;
    } else if (dither_io_md == 0) {
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_8BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_6BIT;
    } else {
        HI_ASSERT(0);
    }

    vo_drv_get_channel_hal_id(dev, &vo_channel);
    /* drv transfer */
    hal_disp_set_dither_round_unlim(vo_channel, cfg->dither_round_unlim);
    hal_disp_set_dither_data_in_out(vo_channel, i_data_width_dither, o_data_width_dither);
    hal_disp_set_dither_en(vo_channel, cfg->dither_en);
    hal_disp_set_dither_mode(vo_channel, dither_md);
    hal_disp_set_dither_round(vo_channel, cfg->dither_round);
    hal_disp_set_dither_domain_mode(vo_channel, cfg->dither_domain_mode);
    hal_disp_set_dither_tap_mode(vo_channel, cfg->dither_tap_mode);

    vo_drv_set_dither_sed(dev, dither_mode_set);
}

hi_void vou_drv_set_dev_dither_out(hi_vo_dev dev, hi_u32 out_bit_width)
{
    g_vo_drv_dev[dev].dither_info.dither_out_bit_width = out_bit_width;
    return;
}

hi_void vou_drv_set_dev_dither(hi_vo_dev dev, hi_u32 in_bit_widht)
{
    vo_drv_dev *drv_dev_ctx = HI_NULL;
    vdp_dither_cfg dither_cfg;

    drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    drv_dev_ctx->dither_info.dither_in_bit_width = in_bit_widht;

    if ((drv_dev_ctx->dither_info.dither_in_bit_width == 10) &&
        (drv_dev_ctx->dither_info.dither_out_bit_width == 8)) {
        dither_cfg.dither_en = 1;
        dither_cfg.dither_round = 1;
        dither_cfg.dither_round_unlim = 1;
        dither_cfg.dither_domain_mode = 1;
        dither_cfg.dither_tap_mode = 1;
        dither_cfg.io_mode = DITHER_IO_MODE_12_8;

        vou_drv_set_dev_dither_mode(dev, VDP_DITHER_MODE_TYP, &dither_cfg);
        hal_disp_set_dither_en(dev, HI_TRUE);
        /* 4:12bit 5:10bit; 3:8bit */
        hal_disp_set_dither_data_in_out(dev, DITHER_IWIDTH_MODE_12BIT, DITHER_OWIDTH_MODE_10BIT);
    } else if ((drv_dev_ctx->dither_info.dither_in_bit_width == 10) &&
               (drv_dev_ctx->dither_info.dither_out_bit_width == 6)) {
        dither_cfg.dither_en = 1;
        dither_cfg.dither_round = 0;
        dither_cfg.dither_round_unlim = 1;
        dither_cfg.dither_domain_mode = 1;
        dither_cfg.dither_tap_mode = 1;
        dither_cfg.io_mode = DITHER_IO_MODE_10_6;
        vou_drv_set_dev_dither_mode(dev, VDP_DITHER_MODE_TYP, &dither_cfg);
        hal_disp_set_dither_en(dev, HI_TRUE);
        /* 4:12bit 5:10bit; 3:8bit */
        hal_disp_set_dither_data_in_out(dev, DITHER_IWIDTH_MODE_10BIT, DITHER_OWIDTH_MODE_6BIT);
    } else if (((drv_dev_ctx->dither_info.dither_in_bit_width == 10) &&
                (drv_dev_ctx->dither_info.dither_out_bit_width == 10)) ||
               ((drv_dev_ctx->dither_info.dither_in_bit_width == 8) &&
                (drv_dev_ctx->dither_info.dither_out_bit_width == 8))) {
        hal_disp_set_dither_en(dev, HI_FALSE);
    }
}

#ifdef HI_DEBUG
hi_void vou_drv_lbw_show_proc(osal_proc_entry_t *s)
{
    hi_s32 i;
    vo_drv_dev *drv_dev_ctx = HI_NULL;

    osal_seq_printf(s,
                    "\n-----DEV LowBandwidth Info----------------------------------------------------------------\n");

    osal_seq_printf(s, "%8s" "%15s" "\n", "DevId", "LowBandwidth");

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        drv_dev_ctx = vo_drv_get_dev_ctx(i);

        if (drv_dev_ctx->vo_enable == HI_TRUE) {
            osal_seq_printf(s, "%8d" "%15d" "\n", i, drv_dev_ctx->low_bandwidth_cnt);
        }
    }

    osal_seq_printf(s, "\r\n");

    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

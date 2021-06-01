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
#include "vou_coef_org.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

/* RGB->YUV601 coefficient matrix */
const csc_coef g_csc_rgb2yuv601_tv = {
    /* csc multiplier coefficient */
    258, 504, 98, -148, -291, 439, 439, -368, -71,
    /* csc input DC component (IDC) */
    0, 0, 0,
    /* csc output DC component (ODC) */
    16, 128, 128,
    0, 0, 0
};

/* RGB->YUV601 coefficient matrix */
const csc_coef g_csc_rgb2yuv601_pc = {
    /* csc multiplier coefficient */
    299, 587, 114, -172, -339, 511, 511, -428, -83,
    /* csc input DC component (IDC) */
    0, 0, 0,
    /* csc output DC component (ODC) */
    0, 128, 128,
    0, 0, 0
};

/* RGB->YUV709 coefficient matrix */
const csc_coef g_csc_rgb2yuv709_tv = {
    /* csc multiplier coefficient */
    184, 614, 62, -101, -338, 439, 439, -399, -40,
    /* csc input DC component (IDC) */
    0, 0, 0,
    /* csc output DC component (ODC) */
    16, 128, 128,
    0, 0, 0
};

/* RGB->YUV709 coefficient matrix ，output full[0,255] */
const csc_coef g_csc_rgb2yuv709_pc = {
    /* csc multiplier coefficient */
    213, 715, 72, -117, -394, 511, 511, -464, -47,
    /* csc input DC component (IDC) */
    0, 0, 0,
    /* csc output DC component (ODC) */
    0, 128, 128,
    0, 0, 0
};

/* RGB -> YUV BT.2020 coefficient matrix */
const csc_coef g_csc_rgb2yuv2020_pc = {
    /* csc multiplier coefficient */
    263, 678, 59, -143, -368, 511, 511, -470, -41,
    /* csc input DC component (IDC) */
    0, 0, 0,
    /* csc output DC component (ODC) */
    0, 128, 128,
    0, 0, 0
};

/* YUV601->RGB coefficient matrix */
const csc_coef g_csc_yuv601_to_rgb_pc = {
    /* csc multiplier coefficient */
    1164, 0, 1596, 1164, -391, -813, 1164, 2018, 0,
    /* csc input DC component (IDC) */
    -16, -128, -128,
    /* csc output DC component (ODC) */
    0, 0, 0,
    0, 0, 0
};

/* YUV709->RGB coefficient matrix */
const csc_coef g_csc_yuv709_to_rgb_pc = {
    /* csc multiplier coefficient */
    1164, 0, 1793, 1164, -213, -534, 1164, 2115, 0,
    /* csc input DC component (IDC) */
    -16, -128, -128,
    /* csc output DC component (ODC) */
    0, 0, 0,
    0, 0, 0
};

/* BT.2020 YUV -> RGB coefficient matrix */
const csc_coef g_csc_yuv2020_to_rgb_pc = {
    /* csc multiplier coefficient */
    1000, 0, 1442, 1000, -160, -560, 1000, 1841, 0,
    /* csc input DC component (IDC) */
    0, -128, -128,
    /* csc output DC component (ODC) */
    0, 0, 0,
    0, 0, 0
};

/* YUV601->YUV709 coefficient matrix，output full[0,255] */
const csc_coef g_csc_yuv2yuv_601_709 = {
    /* csc multiplier coefficient */
    1000, -116, 208, 0, 1017, 114, 0, 75, 1025,
    /* csc input DC component (IDC) */
    -16, -128, -128,
    /* csc output DC component (ODC) */
    16, 128, 128,
    0, 0, 0
};

/* YUV709->YUV601 coefficient matrix，output full[0,255] */
const csc_coef g_csc_yuv2yuv_709_601 = {
    /* csc multiplier coefficient */
    1000, 99, 192, 0, 990, -111, 0, -72, 983,
    /* csc input DC component (IDC) */
    -16, -128, -128,
    /* csc output DC component (ODC) */
    16, 128, 128,
    0, 0, 0
};

/* YUV601->YUV709 coefficient matrix */
const csc_coef g_csc_init = {
    /* csc multiplier coefficient */
    1000, 0, 0, 0, 1000, 0, 0, 0, 1000,
    /* csc input DC component (IDC) */
    -16, -128, -128,
    /* csc output DC component (ODC) */
    16, 128, 128,
    0, 0, 0
};

const int g_sin_table[61] = {
    -500, -485, -469, -454, -438, -422, -407, -391, -374, -358,
    -342, -325, -309, -292, -276, -259, -242, -225, -208, -191,
    -174, -156, -139, -122, -104, -87, -70, -52, -35, -17,
    0, 17, 35, 52, 70, 87, 104, 122, 139, 156,
    174, 191, 208, 225, 242, 259, 276, 292, 309, 325,
    342, 358, 374, 391, 407, 422, 438, 454, 469, 485,
    500
};

const int g_cos_table[61] = {
    866, 875, 883, 891, 899, 906, 914, 921, 927, 934,
    940, 946, 951, 956, 961, 966, 970, 974, 978, 982,
    985, 988, 990, 993, 995, 996, 998, 999, 999, 1000,
    1000, 1000, 999, 999, 998, 996, 995, 993, 990, 988,
    985, 982, 978, 974, 970, 966, 961, 956, 951, 946,
    940, 934, 927, 921, 914, 906, 899, 891, 883, 875,
    866
};

const csc_coef *vo_get_csc_coef(hal_csc_mode csc_mode)
{
    switch (csc_mode) {
        case HAL_CSC_MODE_BT601_TO_BT601:
        case HAL_CSC_MODE_BT709_TO_BT709:
        case HAL_CSC_MODE_RGB_TO_RGB:
            return &g_csc_init;
        case HAL_CSC_MODE_BT709_TO_BT601:
            return &g_csc_yuv2yuv_709_601;
        case HAL_CSC_MODE_BT601_TO_BT709:
            return &g_csc_yuv2yuv_601_709;
        case HAL_CSC_MODE_BT601_TO_RGB_PC:
            return &g_csc_yuv601_to_rgb_pc;
        case HAL_CSC_MODE_BT709_TO_RGB_PC:
            return &g_csc_yuv709_to_rgb_pc;
        case HAL_CSC_MODE_RGB_TO_BT601_PC:
            return &g_csc_rgb2yuv601_pc;
        case HAL_CSC_MODE_RGB_TO_BT709_PC:
            return &g_csc_rgb2yuv709_pc;
        case HAL_CSC_MODE_RGB_TO_BT601_TV:
            return &g_csc_rgb2yuv601_tv;
        case HAL_CSC_MODE_RGB_TO_BT709_TV:
            return &g_csc_rgb2yuv709_tv;
        default:
            return HI_NULL;
    }
}

const int *vo_get_sin_table(hi_void)
{
    return g_sin_table;
}

const int *vo_get_cos_table(hi_void)
{
    return g_cos_table;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */


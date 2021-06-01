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

#ifndef _DRM_HAL_COMMON_H__
#define _DRM_HAL_COMMON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define drm_hal_print(level, fmt...) do {     \
    printk("[DRM-HAL][%s][%s][%u] ", level, __FUNCTION__, __LINE__); \
    printk(fmt);               \
} while (0)

/* DRM HAL log system */
#define drm_hal_err(fmt...) drm_hal_print("ERROR", fmt)
#define drm_hal_warn(fmt...) drm_hal_print("WARN", fmt)
#define drm_hal_info(fmt...) drm_hal_print("INFO", fmt)

/* define the display channel enum */
enum drm_hal_disp_chn {
    DRM_HAL_DISP_0,
    DRM_HAL_DISP_1,
    DRM_HAL_DISP_MAX
};

/* define the hdmitx channel enum */
enum drm_hal_hdmitx_id {
    DRM_HAL_HDMITX_0,
    DRM_HAL_HDMITX_1,
    DRM_HAL_HDMITX_MAX
};

/* define the graphics layer enum */
enum drm_hal_gfx_layer {
    DRM_HAL_GFX_G0,
    DRM_HAL_GFX_G1,
    DRM_HAL_GFX_G2,
    DRM_HAL_GFX_G3,
    DRM_HAL_GFX_MAX
};

/* define the color format enum */
enum drm_hal_color_fmt {
    DRM_HAL_FMT_RGB565 = 0, /* RGB565 16bpp */
    DRM_HAL_FMT_RGB888,     /* RGB888 24bpp */
    DRM_HAL_FMT_KRGB444,    /* RGB444 16bpp */
    DRM_HAL_FMT_KRGB555,    /* RGB555 16bpp */

    DRM_HAL_FMT_KRGB888,  /* RGB888 32bpp */
    DRM_HAL_FMT_ARGB4444, /* ARGB4444 */
    DRM_HAL_FMT_ARGB1555, /* ARGB1555 */
    DRM_HAL_FMT_ARGB8888, /* ARGB8888 */

    DRM_HAL_FMT_ARGB8565, /* ARGB8565 */
    DRM_HAL_FMT_RGBA4444, /* ARGB4444 */
    DRM_HAL_FMT_RGBA5551, /* RGBA5551 */
    DRM_HAL_FMT_RGBA5658, /* RGBA5658 */

    DRM_HAL_FMT_RGBA8888, /* RGBA8888 */
    DRM_HAL_FMT_BGR565,   /* BGR565 */
    DRM_HAL_FMT_BGR888,   /* BGR888 */
    DRM_HAL_FMT_ABGR4444, /* ABGR4444 */

    DRM_HAL_FMT_ABGR1555, /* ABGR1555 */
    DRM_HAL_FMT_ABGR8888, /* ABGR8888 */
    DRM_HAL_FMT_ABGR8565, /* ABGR8565 */
    DRM_HAL_FMT_KBGR444,  /* BGR444 16bpp */

    DRM_HAL_FMT_KBGR555, /* BGR555 16bpp */
    DRM_HAL_FMT_KBGR888, /* BGR888 32bpp */
    DRM_HAL_FMT_1BPP,    /* clut1 */
    DRM_HAL_FMT_2BPP,    /* clut2 */

    DRM_HAL_FMT_MAX
};

/* define the display rect struct */
struct drm_hal_rect {
    int x;
    int y;
    int w;
    int h;
};

/* define the display timing fmt */
enum drm_hal_timing_fmt {
    DRM_HAL_TIMING_FMT_1080P_60,
    DRM_HAL_TIMING_FMT_1080P_50,
    DRM_HAL_TIMING_FMT_1080P_59_94,
    DRM_HAL_TIMING_FMT_1080P_30,
    DRM_HAL_TIMING_FMT_1080P_25,
    DRM_HAL_TIMING_FMT_1080P_24,
    DRM_HAL_TIMING_FMT_1080I_60,
    DRM_HAL_TIMING_FMT_1080I_50,
    DRM_HAL_TIMING_FMT_720P_60,
    DRM_HAL_TIMING_FMT_720P_50,
    DRM_HAL_TIMING_FMT_720P_30,
    DRM_HAL_TIMING_FMT_720P_24,
    DRM_HAL_TIMING_FMT_576P_50,
    DRM_HAL_TIMING_FMT_480P_60,
    DRM_HAL_TIMING_FMT_PAL,
    DRM_HAL_TIMING_FMT_NTSC,
    DRM_HAL_TIMING_FMT_861D_640X480_60,
    DRM_HAL_TIMING_FMT_VESA_800X600_60,
    DRM_HAL_TIMING_FMT_VESA_1024X768_60,
    DRM_HAL_TIMING_FMT_VESA_1280X800_60,
    DRM_HAL_TIMING_FMT_VESA_1280X1024_60,
    DRM_HAL_TIMING_FMT_VESA_1366X768_60,
    DRM_HAL_TIMING_FMT_VESA_1440X900_60,
    DRM_HAL_TIMING_FMT_VESA_1600X1200_60,
    DRM_HAL_TIMING_FMT_VESA_1680X1050_60,
    DRM_HAL_TIMING_FMT_VESA_1920X1200_60,
    DRM_HAL_TIMING_FMT_2560X1440_30,
    DRM_HAL_TIMING_FMT_2560X1440_60,
    DRM_HAL_TIMING_FMT_2560X1600_60,
    DRM_HAL_TIMING_FMT_1920X2160_30,
    DRM_HAL_TIMING_FMT_3840X2160P_24,
    DRM_HAL_TIMING_FMT_3840X2160P_25,
    DRM_HAL_TIMING_FMT_3840X2160P_30,
    DRM_HAL_TIMING_FMT_3840X2160P_50,
    DRM_HAL_TIMING_FMT_3840X2160P_60,
    DRM_HAL_TIMING_FMT_4096X2160P_24,
    DRM_HAL_TIMING_FMT_4096X2160P_25,
    DRM_HAL_TIMING_FMT_4096X2160P_30,
    DRM_HAL_TIMING_FMT_4096X2160P_50,
    DRM_HAL_TIMING_FMT_4096X2160P_60,
    DRM_HAL_TIMING_FMT_USER,
    DRM_HAL_TIMING_FMT_MAX
};

/* define the display timing struct */
struct drm_hal_timing {
    int clock; /* in kHz */
    int hdisplay;
    int hsync_start;
    int hsync_end;
    int htotal;
    int hskew;
    int vdisplay;
    int vsync_start;
    int vsync_end;
    int vtotal;
    int vscan;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _DRM_HAL_COMMON_H__ */

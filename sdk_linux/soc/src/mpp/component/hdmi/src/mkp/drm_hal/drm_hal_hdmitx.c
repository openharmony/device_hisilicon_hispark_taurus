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

#include "drm_hal_hdmitx.h"
#include <linux/module.h>
#include "drv_hdmi_intf.h"
#include "drv_hdmi_edid.h"
#include "drm_hal_common.h"
#include "hi_drm_func_ext.h"

static hdmi_device_id hdmitx_id_translate(enum drm_hal_hdmitx_id id)
{
    switch (id) {
        case DRM_HAL_HDMITX_0:
            return HDMI_DEVICE_ID0;
        default:
            return HDMI_DEVICE_ID_BUTT;
    }

    return HDMI_DEVICE_ID_BUTT;
}

void drm_hal_hdmitx_get_capability(struct drm_hal_hdmitx_capability *cap)
{
    if (cap != NULL) {
        cap->hdmitx_num = 1;
    }
    return;
}

int drm_hal_hdmitx_init(void)
{
    return drv_hdmi_init();
}

void drm_hal_hdmitx_deinit(void)
{
    (void)drv_hdmi_deinit();
}

int drm_hal_hdmitx_open(enum drm_hal_hdmitx_id id)
{
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);
    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    return drv_hdmi_open(dev, HI_FALSE);
}

int drm_hal_hdmitx_close(enum drm_hal_hdmitx_id id)
{
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);
    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    return drv_hdmi_close(dev, HI_FALSE);
}

int drm_hal_hdmitx_enable(enum drm_hal_hdmitx_id id)
{
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);
    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    return drv_hdmi_start(dev);
}

int drm_hal_hdmitx_disable(enum drm_hal_hdmitx_id id)
{
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);
    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    return drv_hdmi_stop(dev);
}


int drm_hal_hdmitx_get_attr(enum drm_hal_hdmitx_id id, struct drm_hal_hdmitx_attr *attr)
{
    int ret;
    drv_hdmi_property param;
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);
    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    param.hdmi_id = dev_id;

    ret = drv_hdmi_cmd_process(CMD_HDMI_GET_ATTR, &param, HI_FALSE);
    if (ret != 0) {
        drm_hal_err("ret=%#x\n", ret);
        return ret;
    }

    return 0;
}

struct hdmitx_timing_map {
    enum drm_hal_timing_fmt hal_fmt;
    hdmi_disp_format hdmi_fmt;
    hdmi_video_timing hdmi_timing;
    unsigned int clock;
};

static struct hdmitx_timing_map g_hdmitx_timing_map[] = {
    {DRM_HAL_TIMING_FMT_1080P_60, HDMI_VIDEO_FORMAT_1080P_60, HDMI_VIDEO_TIMING_1920X1080P_60000, 148500},
    {DRM_HAL_TIMING_FMT_1080P_50, HDMI_VIDEO_FORMAT_1080P_50, HDMI_VIDEO_TIMING_1920X1080P_50000, 148500},
    {DRM_HAL_TIMING_FMT_1080P_30, HDMI_VIDEO_FORMAT_1080P_30, HDMI_VIDEO_TIMING_1920X1080P_30000, 74250},
    {DRM_HAL_TIMING_FMT_1080P_25, HDMI_VIDEO_FORMAT_1080P_25, HDMI_VIDEO_TIMING_1920X1080P_25000, 74250},
    {DRM_HAL_TIMING_FMT_1080P_24, HDMI_VIDEO_FORMAT_1080P_24, HDMI_VIDEO_TIMING_1920X1080P_24000, 74250},
    {DRM_HAL_TIMING_FMT_1080I_60, HDMI_VIDEO_FORMAT_1080I_60, HDMI_VIDEO_TIMING_1920X1080I_60000, 74250},
    {DRM_HAL_TIMING_FMT_1080I_50, HDMI_VIDEO_FORMAT_1080I_50, HDMI_VIDEO_TIMING_1920X1080I_50000, 74250},
    {DRM_HAL_TIMING_FMT_720P_60, HDMI_VIDEO_FORMAT_720P_60, HDMI_VIDEO_TIMING_1280X720P_60000, 74250},
    {DRM_HAL_TIMING_FMT_720P_50, HDMI_VIDEO_FORMAT_720P_50, HDMI_VIDEO_TIMING_1280X720P_50000, 74250},
    {DRM_HAL_TIMING_FMT_576P_50, HDMI_VIDEO_FORMAT_576P_50, HDMI_VIDEO_TIMING_720X576P_50000, 27000},
    {DRM_HAL_TIMING_FMT_480P_60, HDMI_VIDEO_FORMAT_480P_60, HDMI_VIDEO_TIMING_720X480P_60000, 27000},
    {DRM_HAL_TIMING_FMT_PAL, HDMI_VIDEO_FORMAT_PAL, HDMI_VIDEO_TIMING_1440X576I_50000, 13500},
    {DRM_HAL_TIMING_FMT_NTSC, HDMI_VIDEO_FORMAT_NTSC, HDMI_VIDEO_TIMING_1440X480I_60000, 13500},
    {DRM_HAL_TIMING_FMT_VESA_800X600_60, HDMI_VIDEO_FORMAT_VESA_800X600_60, HDMI_VIDEO_TIMING_VESA_800X600_60, 40000},
    {DRM_HAL_TIMING_FMT_VESA_1024X768_60, HDMI_VIDEO_FORMAT_VESA_1024X768_60, HDMI_VIDEO_TIMING_VESA_1024X768_60,
        48400},
    {DRM_HAL_TIMING_FMT_VESA_1280X800_60, HDMI_VIDEO_FORMAT_VESA_1280X800_60, HDMI_VIDEO_TIMING_VESA_1280X800_60,
        83500},
    {DRM_HAL_TIMING_FMT_VESA_1280X1024_60, HDMI_VIDEO_FORMAT_VESA_1280X1024_60, HDMI_VIDEO_TIMING_VESA_1280X1024_60,
        108000},
    {DRM_HAL_TIMING_FMT_VESA_1366X768_60, HDMI_VIDEO_FORMAT_VESA_1366X768_60, HDMI_VIDEO_TIMING_VESA_1366X768_60,
        85500},
    {DRM_HAL_TIMING_FMT_VESA_1440X900_60, HDMI_VIDEO_FORMAT_VESA_1440X900_60, HDMI_VIDEO_TIMING_VESA_1440X900_60,
        106500},
    {DRM_HAL_TIMING_FMT_VESA_1600X1200_60, HDMI_VIDEO_FORMAT_VESA_1600X1200_60, HDMI_VIDEO_TIMING_VESA_1600X1200_60,
        162000},
    {DRM_HAL_TIMING_FMT_VESA_1680X1050_60, HDMI_VIDEO_FORMAT_VESA_1680X1050_60, HDMI_VIDEO_TIMING_VESA_1680X1050_60,
        146250},
    {DRM_HAL_TIMING_FMT_VESA_1920X1200_60, HDMI_VIDEO_FORMAT_VESA_1920X1200_60, HDMI_VIDEO_TIMING_VESA_1920X1200_60,
        193250},
    {DRM_HAL_TIMING_FMT_2560X1440_60, HDMI_VIDEO_FORMAT_2560X1440_60, HDMI_VIDEO_TIMING_VESA_2560X1440_60_RB, 238750},
    {DRM_HAL_TIMING_FMT_2560X1600_60, HDMI_VIDEO_FORMAT_2560X1600_60, HDMI_VIDEO_TIMING_VESA_2560X1600_60, 268500},
    {DRM_HAL_TIMING_FMT_3840X2160P_24, HDMI_VIDEO_FORMAT_3840X2160P_24, HDMI_VIDEO_TIMING_3840X2160P_24000, 297000},
    {DRM_HAL_TIMING_FMT_3840X2160P_25, HDMI_VIDEO_FORMAT_3840X2160P_25, HDMI_VIDEO_TIMING_3840X2160P_25000, 297000},
    {DRM_HAL_TIMING_FMT_3840X2160P_30, HDMI_VIDEO_FORMAT_3840X2160P_30, HDMI_VIDEO_TIMING_3840X2160P_30000, 297000},
    {DRM_HAL_TIMING_FMT_3840X2160P_50, HDMI_VIDEO_FORMAT_3840X2160P_50, HDMI_VIDEO_TIMING_3840X2160P_50000, 594000},
    {DRM_HAL_TIMING_FMT_3840X2160P_60, HDMI_VIDEO_FORMAT_3840X2160P_60, HDMI_VIDEO_TIMING_3840X2160P_60000, 594000},
    {DRM_HAL_TIMING_FMT_4096X2160P_24, HDMI_VIDEO_FORMAT_4096X2160P_24, HDMI_VIDEO_TIMING_4096X2160P_24000, 297000},
    {DRM_HAL_TIMING_FMT_4096X2160P_25, HDMI_VIDEO_FORMAT_4096X2160P_25, HDMI_VIDEO_TIMING_4096X2160P_25000, 297000},
    {DRM_HAL_TIMING_FMT_4096X2160P_30, HDMI_VIDEO_FORMAT_4096X2160P_30, HDMI_VIDEO_TIMING_4096X2160P_30000, 297000},
    {DRM_HAL_TIMING_FMT_4096X2160P_50, HDMI_VIDEO_FORMAT_4096X2160P_50, HDMI_VIDEO_TIMING_4096X2160P_50000, 594000},
    {DRM_HAL_TIMING_FMT_4096X2160P_60, HDMI_VIDEO_FORMAT_4096X2160P_60, HDMI_VIDEO_TIMING_4096X2160P_60000, 594000}
};

static void hdmitx_translate_timing(enum drm_hal_timing_fmt hal_fmt, hdmi_disp_format *hdmi_fmt,
    hdmi_video_timing *hdmi_timing, unsigned int *pix_clk)
{
    int index;

    for (index = 0; index < sizeof(g_hdmitx_timing_map) / sizeof(struct hdmitx_timing_map); index++) {
        if (g_hdmitx_timing_map[index].hal_fmt == hal_fmt) {
            break;
        }
    }

    /* can not find, use the default */
    if (index == sizeof(g_hdmitx_timing_map) / sizeof(struct hdmitx_timing_map)) {
        drm_hal_err("can not find the matched fmt, hal_fmt = %u\n", hal_fmt);
        *hdmi_fmt = HDMI_VIDEO_FORMAT_1080P_60;
        *hdmi_timing = HDMI_VIDEO_TIMING_1920X1080P_60000;
        *pix_clk = 148500; /* 148500 times one second */
        return;
    }

    *hdmi_fmt = g_hdmitx_timing_map[index].hdmi_fmt;
    *hdmi_timing = g_hdmitx_timing_map[index].hdmi_timing;
    *pix_clk = g_hdmitx_timing_map[index].clock;
}

static void hdmitx_attr_translate(drv_hdmi_property *param, struct drm_hal_hdmitx_attr *attr, int flag)
{
    if (flag == 0) { /* drm_hal_hdmitx_attr translate to drv_hdmi_property */
        hdmitx_translate_timing(attr->fmt, &param->prop.disp_fmt, &param->prop.video_timing, &param->prop.pix_clk);
    } else { /* drv_hdmi_property translate to drm_hal_hdmitx_attr */
        drm_hal_err("not support yet!\n");
    }
}

int drm_hal_hdmitx_set_attr(enum drm_hal_hdmitx_id id, struct drm_hal_hdmitx_attr *attr)
{
    int ret;
    drv_hdmi_property param;
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);

    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    param.hdmi_id = dev_id;

    ret = drv_hdmi_cmd_process(CMD_HDMI_GET_ATTR, &param, HI_FALSE);
    if (ret != 0) {
        drm_hal_err("ret=%#x\n", ret);
        return ret;
    }

    hdmitx_attr_translate(&param, attr, 0);

    /* param need configured by DRM */
    param.prop.enable_hdmi           = HI_TRUE;
    param.prop.enable_video          = HI_TRUE;
    param.prop.deep_color_mode       = HDMI_DEEP_COLOR_24BIT;
    param.prop.enable_avi_infoframe  = HI_TRUE;
    param.prop.enable_deep_clr_adapt = HI_FALSE;
    param.prop.enable_vid_mode_adapt = HI_TRUE;
    param.prop.hdmi_action           = HDMI_DEFAULT_ACTION_NULL;
    param.prop.in_color_space        = HDMI_COLORSPACE_YCBCR444;
    param.prop.out_color_space       = HDMI_COLORSPACE_YCBCR444;
    param.prop.out_csc_quantization  = HDMI_QUANTIZATION_RANGE_LIMITED;

    ret = drv_hdmi_cmd_process(CMD_HDMI_SET_ATTR, &param, HI_FALSE);
    if (ret != 0) {
        drm_hal_err("ret=%#x\n", ret);
        return ret;
    }

    return 0;
}

int drm_hal_hdmitx_get_status(enum drm_hal_hdmitx_id id, enum drm_hal_hdmitx_status_type type,
    const enum drm_hal_hdmitx_connect_status *param)
{
    int ret;
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    drv_hdmi_status status = {0};

    status.hdmi_id = dev_id;
    ret = drv_hdmi_cmd_process(CMD_HDMI_GET_STATUS, &status, HI_FALSE);
    if (ret != 0) {
        drm_hal_err("ret=%#x\n", ret);
        return ret;
    }

    if (status.status.connected == HI_TRUE) {
        *(enum drm_hal_hdmitx_connect_status *)param = DRM_HAL_HDMITX_CONNECTED;
    } else {
        *(enum drm_hal_hdmitx_connect_status *)param = DRM_HAL_HDMITX_DISCONNECTED;
    }

    return 0;
}

int drm_hal_hdmitx_read_edid(enum drm_hal_hdmitx_id id, unsigned char *buffer, int buf_len)
{
    hdmi_device_id dev_id = hdmitx_id_translate(id);
    hdmi_device *dev = get_hdmi_device(dev_id);
    if (dev == NULL) {
        drm_hal_err("id = %u, dev is null!\n", id);
        return -1;
    }

    return drv_hdmi_edid_raw_get(&dev->edid_info, buffer, buf_len);
}

struct drm_hal_hdmitx_dev g_hdmitx_dev = {0};

hi_void drm_hal_hdmitx_dev_register(hi_void)
{
    hi_drm_export_func *drm_ext_func = HI_NULL;
    drm_ext_func = drm_export_func_register();
    if (drm_ext_func == HI_NULL) {
        drm_hal_err("err:drm_ext_func is NULL!\n");
        return;
    }

    drm_ext_func->hdmi_func = &g_hdmitx_dev;
    g_hdmitx_dev.get_capability = drm_hal_hdmitx_get_capability;
    g_hdmitx_dev.open = drm_hal_hdmitx_open;
    g_hdmitx_dev.close = drm_hal_hdmitx_close;
    g_hdmitx_dev.enable = drm_hal_hdmitx_enable;
    g_hdmitx_dev.disable = drm_hal_hdmitx_disable;
    g_hdmitx_dev.get_attr = drm_hal_hdmitx_get_attr;
    g_hdmitx_dev.set_attr = drm_hal_hdmitx_set_attr;
    g_hdmitx_dev.get_status = drm_hal_hdmitx_get_status;
    g_hdmitx_dev.read_edid = drm_hal_hdmitx_read_edid;
    return;
}

hi_void drm_hal_hdmitx_dev_unregister(hi_void)
{
    hi_drm_export_func *drm_ext_func = HI_NULL;
    drm_ext_func = drm_export_func_register();
    if (drm_ext_func == HI_NULL) {
        drm_hal_err("err:drm_ext_func is NULL!\n");
        return;
    }

    g_hdmitx_dev.get_capability = NULL;
    g_hdmitx_dev.open = NULL;
    g_hdmitx_dev.close = NULL;
    g_hdmitx_dev.enable = NULL;
    g_hdmitx_dev.disable = NULL;
    g_hdmitx_dev.get_attr = NULL;
    g_hdmitx_dev.set_attr = NULL;
    g_hdmitx_dev.get_status = NULL;
    g_hdmitx_dev.read_edid = NULL;
    drm_ext_func->hdmi_func = NULL;
    return;
}

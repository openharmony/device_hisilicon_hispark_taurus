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
#include "drv_hdmi_intf_k.h"
#include "drv_hdmi_intf.h"
#include "hdmi_hal.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_event.h"
#include "hdmi_product_define.h"
#include "drv_hdmi_ioctl.h"

static hi_s32 hdmi_ext_ioctl(unsigned int cmd, void *argp)
{
    hi_s32 ret;
    ret = drv_hdmi_cmd_process(cmd, argp, HI_FALSE);
    return ret;
}

static hi_void hdmi_hot_plug_process(hdmi_device_id hdmi_id)
{
    hi_s32 ret;
    drv_hdmi_status       status   = {0};
    drv_hdmi_property     property = {0};
    hdmi_device          *hdmi_dev = HI_NULL;
    hdmi_sink_capability *sink_cap = HI_NULL;

    hdmi_info("\n---HDMI kernel event(no user_call_back): HOTPLUG. --- \n");

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return_void(hdmi_dev);

    get_hdmi_default_action_set(hdmi_dev, HDMI_DEFAULT_ACTION_HDMI);
    ret = drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap);
    if (ret == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability err!\n");
    }

    status.hdmi_id = hdmi_id;
    ret = hdmi_ext_ioctl(CMD_HDMI_GET_STATUS, &status);
    if (ret != HI_SUCCESS) {
        hdmi_err("get HDMI status err!\n");
        return;
    }
    if (status.status.connected == HI_FALSE) {
        hdmi_err("no connect!\n");
        return;
    }
    hdmi_info("connected !\n");
    property.hdmi_id = hdmi_id;
    ret = hdmi_ext_ioctl(CMD_HDMI_GET_ATTR, &property);
    if (ret != HI_SUCCESS) {
        hdmi_err("get hdmi attr err!\n");
        return;
    }
    hdmi_info("CMD_HDMI_GET_ATTR ok! \n");
    ret = hdmi_ext_ioctl(CMD_HDMI_SET_ATTR, &property);
    if (ret != HI_SUCCESS) {
        hdmi_err("set attr err!:0x%x\n", ret);
    }
    hdmi_info("CMD_HDMI_SET_ATTR ok! \n");
    ret = hdmi_ext_ioctl(CMD_HDMI_START, &hdmi_id);
    if (ret != HI_SUCCESS) {
        hdmi_err("hdmi start err!:0x%x\n", ret);
        return;
    }
    hdmi_info("CMD_HDMI_START ok! \n");

    return;
}

static hi_void hdmi_hot_unplug_process(hdmi_device_id hdmi_id)
{
    hi_s32 ret;

    ret = hdmi_ext_ioctl(CMD_HDMI_STOP, &hdmi_id);
    if (ret != HI_SUCCESS) {
        hdmi_err("hdmi stop  err!:0x%x\n", ret);
        return;
    }
    hdmi_info("CMD_HDMI_STOP ok! \n");

    return;
}

hi_s32 hi_drv_hdmi_kernel_event_callback(hi_void *data, hdmi_event event)
{
    hdmi_device_id hdmi_id;
    hi_u32 need_hpd_process;
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_if_null_return(data, HI_FAILURE);

    hdmi_id = *(hdmi_device_id *)data;
    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    need_hpd_process = ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_START) ||
        ((hi_u32)hdmi_dev->run_state & HDMI_RUN_STATE_STOP);
    if (need_hpd_process) {
        if (event == HDMI_EVENT_HOTPLUG) {
            hdmi_hot_plug_process(hdmi_id);
        } else if (event == HDMI_EVENT_HOTUNPLUG) {
            hdmi_hot_unplug_process(hdmi_id);
        }
    }

    return HI_SUCCESS;
}

#if (defined(CONFIG_HI_PLATFORM_H8))
static hi_s32 csc_param_check(const hdmi_csc_param *csc_param)
{
    if (csc_param->colorimetry != HI_HDMI_COLORIMETRY_ITU601 &&
        csc_param->colorimetry != HI_HDMI_COLORIMETRY_ITU709 &&
        csc_param->colorimetry != HI_HDMI_COLORIMETRY_2020_CONST_LUMINOUS &&
        csc_param->colorimetry != HI_HDMI_COLORIMETRY_2020_NON_CONST_LUMINOUS) {
        hdmi_err("colorimetry err!\n");
        return HI_FAILURE;
    }

    if (csc_param->quantization != HI_HDMI_QUANT_RANGE_LIMITED &&
        csc_param->quantization != HI_HDMI_QUANT_RANGE_FULL) {
        hdmi_err("quantization err!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_s32 video_param_check(const hdmi_video_param *video_param)
{
    if (video_param->pixel_encoding != HI_HDMI_COLORSPACE_RGB &&
        video_param->pixel_encoding != HI_HDMI_COLORSPACE_YCBCR444) {
        hdmi_err("pixel_encoding err!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 hi_drv_hdmi_csc_param_set(hdmi_dev_id hdmi, const hdmi_csc_param *csc_param)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_vo_attr video_attr = {0};

    hdmi_info("in...\n");

    if (get_hdmi_device(hdmi) == HI_NULL) {
        hdmi_err("device id is wrong\n");
        return HI_FAILURE;
    }

    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(csc_param, HI_FAILURE);

    /* param check */
    ret = csc_param_check(csc_param);
    if (ret != HI_SUCCESS) {
        hdmi_err("csc_param_check fail... \n");
        return ret;
    }

    hdmi_dev->csc_param.colorimetry  = csc_param->colorimetry;
    hdmi_dev->csc_param.quantization = csc_param->quantization;
    /* HDMI not open */
    if (hdmi_dev->kernel_cnt == 0 && hdmi_dev->user_cnt == 0) {
        hdmi_warn("device not open! save param to dev. colorimetry=%u, quantization=%u\n",
            csc_param->colorimetry, csc_param->quantization);
        return HI_SUCCESS;
    } else {
        hdmi_dev->attr.app_attr.out_csc_quantization = csc_param->quantization;
        ret = hdmi_ext_ioctl(CMD_HDMI_GET_VO_ATTR, &video_attr);
        if (ret != HI_SUCCESS) {
            hdmi_err("get vo attr fail... \n");
            return ret;
        }
        video_attr.hdmi_id = hdmi;
        video_attr.vo_attr.colorimetry = csc_param->colorimetry;
        ret = hdmi_ext_ioctl(CMD_HDMI_SET_VO_ATTR, &video_attr);
        if (ret != HI_SUCCESS) {
            hdmi_err("set attr fail... \n");
            return ret;
        }
    }
    hdmi_info("out...\n");

    return ret;
}

hi_s32 hi_drv_hdmi_video_param_set(hdmi_dev_id hdmi, const hdmi_video_param *video_param)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_vo_attr vid_attr = {0};

    hdmi_info("in...\n");

    if (get_hdmi_device(hdmi) == HI_NULL) {
        hdmi_err("device id is wrong\n");
        return HI_FAILURE;
    }

    hdmi_dev = get_hdmi_device(hdmi);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    hdmi_if_null_return(video_param, HI_FAILURE);

    /* param check */
    ret = video_param_check(video_param);
    if (ret != HI_SUCCESS) {
        hdmi_err("video_param_check fail... \n");
        return ret;
    }

    hdmi_dev->csc_param.pixel_encoding = video_param->pixel_encoding;

    /* HDMI not open */
    if (hdmi_dev->kernel_cnt == 0 && hdmi_dev->user_cnt == 0) {
        hdmi_warn("device not open! save param to dev. pixel_encoding = %u\n", video_param->pixel_encoding);
        return HI_SUCCESS;
    } else {
        hdmi_dev->attr.app_attr.out_color_space = video_param->pixel_encoding;
        hdmi_dev->attr.vo_attr.in_color_space = video_param->pixel_encoding;
        ret = hdmi_ext_ioctl(CMD_HDMI_GET_VO_ATTR, &vid_attr);
        if (ret != HI_SUCCESS) {
            hdmi_err("get vo attr fail... \n");
            return ret;
        }
        vid_attr.hdmi_id = hdmi;
        ret = hdmi_ext_ioctl(CMD_HDMI_SET_VO_ATTR, &vid_attr);
        if (ret != HI_SUCCESS) {
            hdmi_err("set attr fail... \n");
            return ret;
        }
    }
    hdmi_info("out...\n");

    return ret;
}
#endif

hi_s32 hi_drv_hdmi_stop(hdmi_dev_id hdmi)
{
    hi_s32 ret;

    hdmi_info("in...\n");
    ret = drv_hdmi_cmd_process(CMD_HDMI_STOP, (hi_void *)(&hdmi), HI_FALSE);
    if (ret != HI_SUCCESS) {
        hdmi_err("stop hdmi err!:0x%x\n", ret);
        return ret;
    }
    hdmi_info("out...\n");

    return ret;
}

#ifdef HDMI_HDR_SUPPORT
static hi_void disp_to_hdmi_hdr_attr(const hdmi_device *hdmi_dev, drv_hdmi_hdr_attr *drv_hdr_attr,
    const hdmi_hdr_attr *hdr_attr)
{
    errno_t ret;
    hdmi_if_null_return_void(hdmi_dev);
    hdmi_if_null_return_void(drv_hdr_attr);

    drv_hdr_attr->hdr_mode      = hdr_attr->hdr_mode;
    drv_hdr_attr->user_hdr_mode = hdmi_dev->attr.hdr_attr.user_hdr_mode;
    drv_hdr_attr->colorimetry   = hdr_attr->colorimetry;
    drv_hdr_attr->eotf_type     = hdr_attr->eotf_type;
    drv_hdr_attr->metadata_id   = hdr_attr->metadata_id;
    ret = memcpy_s(&(drv_hdr_attr->un_descriptor), sizeof(drv_hdr_attr->un_descriptor),
        &(hdr_attr->descriptor), sizeof(hdmi_meta_descriptor));
    hdmi_unequal_eok_return_void(ret);

    return;
}

hi_s32 hi_drv_hdmi_set_hdr_attr(hdmi_dev_id hdmi, const hdmi_hdr_attr *hdr_attr)
{
    hi_s32             ret;
    drv_hdmi_hdr       drv_hdr      = {0};
    hdmi_device       *hdmi_dev     = HI_NULL;
    drv_hdmi_hdr_attr *drv_hdr_attr = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi);
    if (hdmi_dev == HI_NULL) {
        hdmi_warn("device id is wrong\n");
        return HI_FAILURE;
    }
    if (hdmi_dev->kernel_cnt == 0 && hdmi_dev->user_cnt == 0) {
        hdmi_warn("device not open\n");
        return HI_FAILURE;
    }

    hdmi_info("in...\n");
    hdmi_if_null_return(hdr_attr, HI_FAILURE);

    if (hdr_attr->hdr_mode == HI_HDMI_HDR_MODE_BUTT) {
        return HI_FAILURE;
    }
    drv_hdr_attr = &drv_hdr.hdr_attr;
    drv_hdr.hdmi_id = hdmi;
    disp_to_hdmi_hdr_attr(hdmi_dev, drv_hdr_attr, hdr_attr);
    ret = hdmi_ext_ioctl(CMD_HDMI_SET_HDR_ATTR, &drv_hdr);
    if (ret != HI_SUCCESS) {
        hdmi_info("set HDR attr fail... \n");
        return ret;
    }
    hdmi_info("out...\n");

    return ret;
}
#endif


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
#ifndef __DRV_HDMI_IOCTL_H__
#define __DRV_HDMI_IOCTL_H__

#include "hi_type.h"
#include "autoconf.h"
#include "drv_hdmi_infoframe.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_common.h"
#if defined(CONFIG_HI_PLATFORM_H8)
#include "hi_common_hdmi.h"
#else
#include "hi_comm_hdmi.h"
#include "hi_comm_hdmi_adapt.h"
#endif
#include "mkp_ioctl.h"

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_video_capability video_cap;
} drv_hdmi_video_capability;

#if defined(CONFIG_HI_PLATFORM_H8)
typedef hi_hdmi_colorimetry     drv_hdmi_colorimetry;
typedef hi_hdmi_rgb_quant_range drv_hdmi_rgb_quant_range;
typedef hi_hdmi_ycc_quant_range drv_hdmi_ycc_quant_range;
typedef hi_hdmi_pic_scaline     drv_hdmi_pic_scaline;
#else
typedef HI_HDMI_VIDEO_FMT_E       hi_hdmi_video_format;
typedef HI_HDMI_COLORIMETRY_E     drv_hdmi_colorimetry;
typedef HI_HDMI_RGB_QUAN_RAGE_E   drv_hdmi_rgb_quant_range;
typedef HI_HDMI_YCC_QUAN_RAGE_E   drv_hdmi_ycc_quant_range;
typedef HI_HDMI_PICTURE_SCALING_E drv_hdmi_pic_scaline;
#endif

typedef struct {
    hi_hdmi_video_format     video_fmt;
    hi_u32                   clk_fs;
    hi_u32                   pixel_repeat;
    hdmi_3d_mode             stereo_mode;
    hi_hdmi_video_mode       in_color_space;
    hdmi_hvsync_polarity     hv_sync_pol;
    drv_hdmi_colorimetry     colorimetry;
    drv_hdmi_rgb_quant_range rgb_quantization;
    drv_hdmi_ycc_quant_range ycc_quantization;
    hi_pic_aspect_ratio      picture_aspect;
    hdmi_active_aspect       active_aspect;
    drv_hdmi_pic_scaline     picture_scaling;
    hi_hdmi_deep_color       in_bit_depth;
} hdmi_video_attr;

typedef struct {
    hdmi_device_id  hdmi_id;
    hdmi_video_attr video_attr;
} drv_hdmi_disp_attr;

typedef struct {
    hdmi_device_id      hdmi_id;
    hdmi_default_action default_mode;
} hdmi_open;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_event     event;
} drv_hdmi_event;

typedef struct {
    hdmi_device_id      hdmi_id;
    hdmi_default_action default_action;
} drv_hdmi_default_action;

typedef struct {
    hdmi_device_id        hdmi_id;
    hdmi_audio_capability audio_cap;
} drv_hdmi_audio_capability;

typedef struct {
    hdmi_device_id       hdmi_id;
    hdmi_sink_capability cap;
} drv_hdmi_sink_capability;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_property  prop;
} drv_hdmi_property;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_vo_attr   vo_attr;
} drv_hdmi_vo_attr;

typedef struct {
    hdmi_device_id    hdmi_id;
    hdmi_infoframe_id infoframe_id;
    hdmi_infoframe    infoframe;
} drv_hdmi_infoframe;

typedef struct {
    hdmi_device_id hdmi_id;
    hi_bool        avmute;
} drv_hdmi_avmute;

typedef struct {
    hdmi_device_id  hdmi_id;
    hdmi_deep_color deep_color_mode;
} drv_hdmi_deep_color;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_cec_info  cec_info;
} drv_hdmi_cec_info;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_cec_cmd   cec_cmd;
    hi_u32 timeout;
} drv_hdmi_cec_cmd_code;

typedef struct {
    hdmi_device_id     hdmi_id;
    hdmi_edid_raw_data edid_raw;
} drv_hdmi_edid_raw_data;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_load_key  key;
} drv_hdmi_load_key;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_status    status;
} drv_hdmi_status;

typedef struct {
    hdmi_device_id hdmi_id;
    hi_bool        enable;
} drv_hdmi_xvycc_enable;

typedef struct {
    hdmi_device_id hdmi_id;
    hi_u32         status;
} drv_hdmi_play_status;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_delay     delay;
} drv_hdmi_delay;

typedef struct {
    hdmi_device_id    hdmi_id;
    drv_hdmi_hdr_attr hdr_attr;
} drv_hdmi_hdr;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_trace_len trace_len;
} drv_hdmi_mod_param;

typedef struct {
    hdmi_device_id hdmi_id;
    hdmi_hw_spec   hw_spec;
} drv_hdmi_hw_spec;

enum hi_ioctl_hdmi {
    IOCTL_HDMI_INIT = 0x01,
    IOCTL_HDMI_DEINIT,
    IOCTL_HDMI_OPEN,
    IOCTL_HDMI_CLOSE,
    IOCTL_HDMI_START,
    IOCTL_HDMI_STOP,
    IOCTL_HDMI_GET_AUDIO_CAPABILITY,
    IOCTL_HDMI_GET_SINK_CAPABILITY,
    IOCTL_HDMI_POLL_EVENT,
    IOCTL_HDMI_GET_ATTR,
    IOCTL_HDMI_SET_ATTR,
    IOCTL_HDMI_GET_VO_ATTR,
    IOCTL_HDMI_SET_VO_ATTR,
    IOCTL_HDMI_GET_INFOFRAME,
    IOCTL_HDMI_SET_INFOFRAME,
    IOCTL_HDMI_AVMUTE,
    IOCTL_HDMI_GET_DEEPCOLOR,
    IOCTL_HDMI_SET_DEEPCOLOR,
    IOCTL_HDMI_XVYCC,
    IOCTL_HDMI_SET_CEC,
    IOCTL_HDMI_GET_CEC,
    IOCTL_HDMI_GET_CECSTATUS,
    IOCTL_HDMI_UPDATE_EDID,
    IOCTL_HDMI_GET_HDMI_PLAYSTATUS,
    IOCTL_HDMI_CEC_ENABLE,
    IOCTL_HDMI_CEC_DISABLE,
    IOCTL_HDMI_REG_CALLBACK_FUNC,
    IOCTL_HDMI_LOAD_KEY,
    IOCTL_HDMI_LOAD_HDCP22_CODE,
    IOCTL_HDMI_GET_STATUS,
    IOCTL_HDMI_GET_DELAY,
    IOCTL_HDMI_SET_DELAY,
    IOCTL_HDMI_REGISTER_CALLBACK,
    IOCTL_HDMI_UNREGISTER_CALLBACK,
    IOCTL_HDMI_GET_VIDEO_CAPABILITY,
    IOCTL_HDMI_SET_DISP_ATTR,
    IOCTL_HDMI_SET_MOD_PARAM,
    IOCTL_HDMI_GET_MOD_PARAM,
    IOCTL_HDMI_SET_HW_SPEC,
    IOCTL_HDMI_GET_HW_SPEC,
#ifdef HDMI_HDR_SUPPORT
    IOCTL_HDMI_GET_HDR_ATTR,
    IOCTL_HDMI_SET_HDR_ATTR,
#endif
    IOCTL_HDMI_MAX
};

#define CMD_HDMI_INIT                 _IO(IOC_TYPE_HDMI, IOCTL_HDMI_INIT)
#define CMD_HDMI_DEINIT               _IO(IOC_TYPE_HDMI, IOCTL_HDMI_DEINIT)
#define CMD_HDMI_OPEN                 _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_OPEN, drv_hdmi_default_action)
#define CMD_HDMI_CLOSE                _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_CLOSE, hdmi_device_id)
#define CMD_HDMI_START                _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_START, hdmi_device_id)
#define CMD_HDMI_STOP                 _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_STOP, hdmi_device_id)
#define CMD_HDMI_GET_AUDIO_CAPABILITY _IOWR(HI_ID_HDMI, IOCTL_HDMI_GET_AUDIO_CAPABILITY, drv_hdmi_audio_capability)
#define CMD_HDMI_GET_SINK_CAPABILITY  _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_SINK_CAPABILITY, drv_hdmi_sink_capability)
#define CMD_HDMI_POLL_EVENT           _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_POLL_EVENT, drv_hdmi_event)
#define CMD_HDMI_GET_INFOFRAME        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_INFOFRAME, drv_hdmi_infoframe)
#define CMD_HDMI_SET_INFOFRAME        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_INFOFRAME, drv_hdmi_infoframe)
#define CMD_HDMI_SET_AVMUTE           _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_AVMUTE, drv_hdmi_avmute)
#define CMD_HDMI_GET_DEEPCOLOR        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_DEEPCOLOR, drv_hdmi_deep_color)
#define CMD_HDMI_SET_DEEPCOLOR        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_DEEPCOLOR, drv_hdmi_deep_color)
#define CMD_HDMI_SET_XVYCC            _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_XVYCC, drv_hdmi_xvycc_enable)
#define CMD_HDMI_GET_CEC              _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_CEC, drv_hdmi_cec_cmd_code)
#define CMD_HDMI_SET_CEC              _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_CEC, drv_hdmi_cec_cmd_code)
#define CMD_HDMI_GET_CECSTATUS        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_CECSTATUS, drv_hdmi_cec_info)
#define CMD_HDMI_UPDATE_EDID          _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_UPDATE_EDID, drv_hdmi_edid_raw_data)
#define CMD_HDMI_GET_HDMI_PLAYSTATUS  _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_HDMI_PLAYSTATUS, drv_hdmi_play_status)
#define CMD_HDMI_CEC_ENABLE           _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_CEC_ENABLE, hdmi_device_id)
#define CMD_HDMI_CEC_DISABLE          _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_CEC_DISABLE, hdmi_device_id)
#define CMD_HDMI_LOAD_KEY             _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_LOAD_KEY, drv_hdmi_load_key)
#define CMD_HDMI_GET_STATUS           _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_STATUS, drv_hdmi_status)
#define CMD_HDMI_GET_DELAY            _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_DELAY, hdmi_delay)
#define CMD_HDMI_SET_DELAY            _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_DELAY, hdmi_delay)
#define CMD_HDMI_GET_VIDEO_CAPABILITY _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_VIDEO_CAPABILITY, drv_hdmi_video_capability)
#define CMD_HDMI_SET_DISP_ATTR        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_DISP_ATTR, drv_hdmi_disp_attr)
#define CMD_HDMI_REGISTER_CALLBACK    _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_REGISTER_CALLBACK, hdmi_device_id)
#define CMD_HDMI_UNREGISTER_CALLBACK  _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_UNREGISTER_CALLBACK, hdmi_device_id)
#define CMD_HDMI_SET_MOD_PARAM        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_MOD_PARAM, drv_hdmi_mod_param)
#define CMD_HDMI_GET_MOD_PARAM        _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_MOD_PARAM, drv_hdmi_mod_param)
#define CMD_HDMI_SET_ATTR             _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_ATTR, drv_hdmi_property)
#define CMD_HDMI_GET_ATTR             _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_ATTR, drv_hdmi_property)
#define CMD_HDMI_SET_VO_ATTR          _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_VO_ATTR, drv_hdmi_vo_attr)
#define CMD_HDMI_GET_VO_ATTR          _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_VO_ATTR, drv_hdmi_vo_attr)
#define CMD_HDMI_GET_HW_SPEC          _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_GET_HW_SPEC, drv_hdmi_hw_spec)
#define CMD_HDMI_SET_HW_SPEC          _IOWR(IOC_TYPE_HDMI, IOCTL_HDMI_SET_HW_SPEC, drv_hdmi_hw_spec)
#ifdef HDMI_HDR_SUPPORT
#define CMD_HDMI_GET_HDR_ATTR         _IOWR(HI_ID_HDMI, IOCTL_HDMI_GET_HDR_ATTR, drv_hdmi_hdr)
#define CMD_HDMI_SET_HDR_ATTR         _IOWR(HI_ID_HDMI, IOCTL_HDMI_SET_HDR_ATTR, drv_hdmi_hdr)
#endif

#endif /* __DRV_HDMI_IOCTL_H__ */


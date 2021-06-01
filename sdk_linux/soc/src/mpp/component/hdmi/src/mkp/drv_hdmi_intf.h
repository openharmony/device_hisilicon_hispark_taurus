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
#ifndef __DRV_HDMI_INTF_H__
#define __DRV_HDMI_INTF_H__

#include "hi_type.h"
#include "hdmi_hal.h"
#include "drv_hdmi_common.h"
#include "drv_hdmi_ioctl.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_event.h"
#ifdef HDMI_FRL_SUPPORT
#include "drv_hdmi_frl.h"
#endif

typedef struct {
    hdmi_thread_state thread_timer_sate;
    osal_task_t      *thread_timer;
    hi_bool           thread_run;
    hi_bool           thread_exit;
    osal_wait_t       wait_destroy;
} hdmi_thread_info;

typedef struct {
    hi_bool         attr_busy;
    hi_bool         attr_set;
    osal_spinlock_t attr_spin;
} hdmi_hdr_mutex;

typedef struct {
    hi_u32                 hdmi_dev_id;
    hi_u32                 kernel_cnt;
    hi_u32                 user_cnt;
    hi_u32                 user_callback_cnt;
    hi_bool                emi_enable;
    hi_bool                hpd_detected;
    hi_bool                hpd_notified;
    hdmi_run_state         run_state;
    hdmi_transition_state  transition_state;
    hdmi_tmds_mode         tmds_mode;
    hdmi_callback          k_callback;
    hdmi_thread_info       thread_info;
    hdmi_edid_info         edid_info;
    hdmi_infoframe_type    info_frame;
    hdmi_attr              attr;
    hdmi_delay             delay;
    hdmi_hal              *hal;
    hdmi_debug             debug;
    osal_semaphore_t       mutex_thread;
    osal_semaphore_t       mutex_proc;
#ifdef HDMI_HDR_SUPPORT
    drv_hdmi_hdr_info      hdr_info;
    hdmi_hdr_mutex         hdr_mutex;
#endif
#ifdef HDMI_FRL_SUPPORT
    hdmi_frl_info          frl_info;
#endif
    hdmi_mode_param        mode_param;
    hdmi_csc_attr          csc_param;
    hdmi_user_vsif_content vendor_data;
} hdmi_device;

hi_s32 drv_hdmi_init(hi_void);

hi_s32 drv_hdmi_deinit(hi_void);

hi_s32 drv_hdmi_open(hdmi_device *hdmi_dev, hi_bool user);

hi_s32 drv_hdmi_close(hdmi_device *hdmi_dev, hi_bool user);

hi_s32 drv_hdmi_start(hdmi_device *hdmi_dev);

hi_s32 drv_hdmi_stop(hdmi_device *hdmi_dev);

hi_s32 drv_hdmi_deep_color_set(hdmi_device *hdmi_dev, hdmi_deep_color deep_color);

hi_s32 drv_hdmi_deep_color_get(hdmi_device *hdmi_dev, hdmi_deep_color *deep_color);

hi_s32 drv_hdmi_xvycc_mode_set(hdmi_device *hdmi_dev, hi_bool enable);

hi_void drv_hdmi_avmute_set(const hdmi_device *hdmi_dev, hi_bool av_mute);

hi_s32 drv_hdmi_vo_attr_set(hdmi_device *hdmi_dev, const hdmi_vo_attr *vo_attr);

hi_s32 drv_hdmi_play_status_get(const hdmi_device *hdmi_dev, hi_u32 *status);

hi_s32 drv_hdmi_ao_attr_set(hdmi_device *hdmi_dev, const hdmi_ao_attr *ao_attr);

hi_s32 drv_hdmi_ao_attr_get(const hdmi_device *hdmi_dev, hdmi_ao_attr *ao_attr);

hi_void drv_hdmi_audio_path_enable(const hdmi_device *hdmi_dev, hi_bool enable);

hi_s32 drv_hdmi_audio_output_enable_set(const hdmi_device *hdmi_dev, hi_bool mute);

hi_s32 drv_hdmi_attr_get(const hdmi_device *hdmi_dev, hdmi_attr *attr);

hi_s32 drv_hdmi_attr_set(hdmi_device *hdmi_dev, hdmi_attr *attr);

hi_s32 drv_hdmi_infoframe_get(const hdmi_device *hdmi_dev, hdmi_infoframe_id infoframe_id, hdmi_infoframe *info_frame);

hi_s32 drv_hdmi_infoframe_set(hdmi_device *hdmi_dev, hdmi_infoframe_id infoframe_id, hdmi_infoframe *info_frame);

hi_s32 drv_hdmi_cmd_process(unsigned int cmd, hi_void *arg, hi_bool user);

hdmi_video_timing disp_fmt_to_hdmi_timing(hdmi_device *hdmi_dev, hi_u32 disp_fmt);

hdmi_device *get_hdmi_device(hdmi_device_id hdmi_id);

hi_s32 get_hdmi_default_action_set(hdmi_device *hdmi_dev, hdmi_default_action action);

hi_s32 get_hdmi_thread_state_set(hdmi_device *hdmi_dev, hdmi_thread_state state);

hi_s32 get_hdmi_delay_get(const hdmi_device *hdmi_dev, hdmi_delay *delay);

hi_s32 get_hdmi_delay_set(hdmi_device *hdmi_dev, hdmi_delay *delay);

hi_s32 hdmi_mode_strategy(hdmi_device *hdmi_dev);

hi_s32 get_global_hdmi_irq_timer(hi_void);

#endif /* __DRV_HDMI_INTF_H__ */


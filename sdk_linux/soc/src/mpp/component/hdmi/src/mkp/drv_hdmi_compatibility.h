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
#ifndef __DRV_HDMI_COMPATIBILITY_H__
#define __DRV_HDMI_COMPATIBILITY_H__

#include "drv_hdmi_common.h"
#include "drv_hdmi_intf.h"

typedef struct {
    hi_bool avi_use_gen5;
    hi_bool avi_use_gen5_debug;
    hi_u8   ctl_type_config;
} compatibility_info;

hi_s32 drv_hdmi_compat_hdcp_ms_get(hdmi_device_id hdmi_id, hi_u64 *wait_ms);

hi_s32 drv_hdmi_compat_avmute_get(hdmi_device_id hdmi_id, hi_bool avmute, hdmi_avmute_cfg *avmute_cfg);

hi_bool drv_hdmi_compat_scdc_in_start_get(hdmi_device_id hdmi_id);

hi_s32 drv_hdmi_compat_delay_get(hdmi_device_id hdmi_id, hdmi_delay *delay);

hi_s32 drv_hdmi_compat_stop_delay(hdmi_device_id hdmi_id);

compatibility_info *compat_info_get(hdmi_device_id hdmi_id);

hi_s32 compatibility_info_update(hdmi_device_id hdmi_id);

hi_void compatibility_info_default_set(hdmi_device_id hdmi_id);

#endif  /* __DRV_HDMI_COMPATIBILITY_H__ */


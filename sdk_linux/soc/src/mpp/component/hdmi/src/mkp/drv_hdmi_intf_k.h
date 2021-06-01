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
#ifndef __DRV_HDMI_INTF_K_H__
#define __DRV_HDMI_INTF_K_H__

#include "drv_hdmi_common.h"

hi_s32 hi_drv_hdmi_kernel_event_callback(hi_void *data, hdmi_event event);

#if (defined(CONFIG_HI_PLATFORM_H8))
hi_s32 hi_drv_hdmi_csc_param_set(hdmi_dev_id hdmi, const hdmi_csc_param *csc_param);

hi_s32 hi_drv_hdmi_video_param_set(hdmi_dev_id hdmi, const hdmi_video_param *video_param);
#endif
hi_s32 hi_drv_hdmi_stop(hdmi_dev_id hdmi);

#endif /* __DRV_HDMI_INTF_K_H__ */


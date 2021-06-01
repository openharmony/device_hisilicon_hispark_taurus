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
#ifndef __HDMI_HAL_INTF_H__
#define __HDMI_HAL_INTF_H__

#include "drv_hdmi_common.h"
#include "hdmi_hal_ctrl.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_scdc.h"
#include "drv_hdmi_compatibility.h"

typedef struct {
    hi_bool init;
    hi_u32  tmds_clk;
    hdmi_tx_capability_data tx_capability;
} hdmi_hal_cfg;

typedef struct {
    hdmi_callback event_callback;
    hi_void      *event_handle;
} hdmi_hal_event;

#endif /* __HDMI_HAL_INTF_H__ */


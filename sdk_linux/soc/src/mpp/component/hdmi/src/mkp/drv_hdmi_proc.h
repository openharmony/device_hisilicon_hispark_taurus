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
#ifndef __DRV_HDMI_PROC_H__
#define __DRV_HDMI_PROC_H__

#include "hi_type.h"
#include "hdmi_product_define.h"

hi_s32 drv_hdmi_read_proc(hdmi_proc_entry *file);

hi_s32 drv_hdmi_vo_read_proc(hdmi_proc_entry *file);

hi_s32 drv_hdmi_ao_read_proc(hdmi_proc_entry *file);

hi_s32 drv_hdmi_sink_read_proc(hdmi_proc_entry *file);

#endif /* __DRV_HDMI_PROC_H__ */


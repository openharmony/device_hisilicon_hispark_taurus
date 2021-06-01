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
#ifndef __HDMI_HAL_NCTS_H__
#define __HDMI_HAL_NCTS_H__

#include "hi_type.h"

typedef struct {
    hi_u32 audio_smp_rate;
    hi_u32 tmds_clk;
    hi_u32 n_value;
    hi_u32 cts_value;
} hdmi_audio_cts_n;

hi_u32 hal_hdmi_n_value_get(hi_u32 sample_rate, hi_u32 tmds_clk);

hi_u32 hal_hdmi_cts_value_get(hi_u32 sample_rate, hi_u32 tmds_clk);

#endif /* __HDMI_HAL_NCTS_H__ */


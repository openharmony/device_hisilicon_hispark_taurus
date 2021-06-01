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

#ifndef __VOU_COEF_ORG_H__
#define __VOU_COEF_ORG_H__

#include "vou_coef.h"
#include "vou_def.h"

const csc_coef *vo_get_csc_coef(hal_csc_mode csc_mode);
const int *vo_get_sin_table(hi_void);
const int *vo_get_cos_table(hi_void);

#endif /* __VOU_COEF_ORG_H__ */

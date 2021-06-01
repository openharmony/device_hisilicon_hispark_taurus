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

#include "hi_common.h"

#ifndef __HI_VOU_EXP_H__
#define __HI_VOU_EXP_H__

typedef struct {
    void (*pfnVoNotify)(int module_id, int vodev);
} vou_export_callback_s;

typedef HI_S32 fn_vo_register_extern_callback(vou_export_callback_s *pstExpCallback);

typedef struct {
    fn_vo_register_extern_callback *pfn_vo_register_exp_callback;
} vou_export_symbol_s;

#endif /* __HI_VOU_EXP_H__ */

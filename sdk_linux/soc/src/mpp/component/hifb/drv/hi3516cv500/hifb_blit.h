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

#ifndef __HIFB_BLIT_H__
#define __HIFB_BLIT_H__

#include "hi_type.h"
#include "hifb_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

hi_bool hifb_tde_is_support_fmt(HIFB_COLOR_FMT_E fmt);
hi_s32 hifb_drv_fill(HIFB_BUFFER_S *dst_img, hi_u32 fill_data);
hi_s32 hifb_drv_blit(HIFB_BUFFER_S *src_img, HIFB_BUFFER_S *dst_img, hifb_blit_opt *opt, hi_bool refresh_screen,
                     hi_s32 *refresh_handle);
hi_s32 hifb_drv_rotate(HIFB_BUFFER_S *src_img, HIFB_BUFFER_S *dst_img, hifb_rotate_opt *rot_opt,
                       hi_bool refresh_screen);
hi_s32 hifb_drv_set_tde_callback(int_callback tde_callback);
hi_s32 hifb_drv_set_tde_rotate_callback(int_callback tde_callback);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __HIFB_BLIT_H__ */

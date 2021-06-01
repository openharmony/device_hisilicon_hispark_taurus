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

#ifndef __VOU_CHN_H__
#define __VOU_CHN_H__

#include "vou.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

vo_sync_basic_info *vo_get_sync_basic_info(hi_void);

hi_s32 vou_set_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr);
hi_s32 vou_get_pub_attr(hi_vo_dev dev, hi_vo_pub_attr *pub_attr);
hi_s32 vou_set_dev_frame_rate(hi_vo_dev dev, hi_u32 *frame_rate);
hi_s32 vou_get_dev_frame_rate(hi_vo_dev dev, hi_u32 *frame_rate);

hi_s32 vou_enable(hi_vo_dev dev);
hi_s32 vou_disable(hi_vo_dev dev);

hi_s32 vo_set_mod_param(const hi_vo_mod_param *mod_param);
hi_s32 vo_get_mod_param(hi_vo_mod_param *mod_param);

hi_s32 vou_set_vtth(hi_vo_dev dev, hi_u32 *vtth);
hi_s32 vou_get_vtth(hi_vo_dev dev, hi_u32 *vtth);
hi_s32 vou_set_vtth2(hi_vo_dev dev, hi_u32 *vtth);
hi_s32 vou_get_vtth2(hi_vo_dev dev, hi_u32 *vtth);

hi_s32 vou_set_user_intf_sync_info(hi_vo_dev dev, hi_vo_user_intfsync_info *user_info);

void vou_virt_dev_timer_func(unsigned long data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __VOU_CHN_H__ */

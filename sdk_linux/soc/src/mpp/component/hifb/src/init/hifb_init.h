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

#ifndef __HIFB_VSYNC_H__
#define __HIFB_VSYNC_H__

#include "hi_type.h"
#include "hifb_vou_graphics.h"
#include "hifb.h"
#ifndef __HuaweiLite__
#include <linux/dma-buf.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

hi_void set_hifb_irq(unsigned int temp_hifb_irq);
hi_void set_hifb_soft_irq(unsigned int temp_hifb_soft_irq);
hi_void set_update_rotate_rect(bool temp_update_rotate_rect);
hi_void set_video_name(char *temp_video);
char *hifb_get_layer_mmz_names(hi_u32 layer_id);
#ifndef __HuaweiLite__
hi_s32 hifb_init(hi_void);
struct dma_buf *hifb_memblock_export(phys_addr_t base, size_t size, int flags);
#endif
hi_void hifb_cleanup(hi_void);

int hifb_pts_init(void);

void hifb_pts_exit(void);

int hifb_vsync_init(void);

void hifb_vsync_exit(void);

int hifb_vsync_notify(void);

hi_u32 hifb_get_module_para(hi_void *args);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HIFB_VSYNC_H__ */


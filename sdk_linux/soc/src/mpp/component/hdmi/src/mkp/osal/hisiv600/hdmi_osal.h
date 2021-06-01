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
#ifndef __HDMI_OSAL_H__
#define __HDMI_OSAL_H__

#include "hi_type.h"

typedef enum {
    OSAL_RDONLY,
    OSAL_WRONLY,
    OSAL_RDWR,
    OSAL_BUTT
} osal_file_flag;

#define is_str_equal(p1, p2) ({ (((p1) != HI_NULL) && ((p2) != HI_NULL) && (osal_strlen(p1) == osal_strlen(p2)) && \
    (osal_strncmp(p1, p2, osal_strlen(p1)) == 0)) ? HI_TRUE : HI_FALSE; })

struct file *hdmi_osal_file_open(const hi_char *file_name, osal_file_flag e_flags);

hi_void hdmi_osal_file_close(struct file *file);

hi_s32 hdmi_osal_file_read(struct file *file, hi_char *buf, hi_u32 len);

hi_s32 hdmi_osal_file_write(struct file *file, hi_char *buf, hi_u32 len);

hi_u32 hdmi_osal_get_time_in_ms(hi_void);

hi_u64 hdmi_osal_get_time_in_us(hi_void);

#endif /* __HDMI_OSAL_H__ */


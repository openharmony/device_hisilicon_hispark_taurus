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
#include "hdmi_product_define.h" /* because of refined warning, hdmi_product_define.h go first. */
#include "hdmi_osal.h"
#include "hi_osal.h"

#define HDMI_MILLION 1000000

struct file *hdmi_osal_file_open(const hi_char *file_name, osal_file_flag e_flags)
{
    struct file *file = NULL;

    if (file_name == HI_NULL) {
        return HI_NULL;
    }

    e_flags = OSAL_O_RDWR | OSAL_O_CREAT;
    file = osal_klib_fopen(file_name, e_flags, HDMI_FILE_MODE);

    return file;
}

hi_void hdmi_osal_file_close(struct file *file)
{
    if (file != HI_NULL) {
        osal_klib_fclose(file);
    }
    return;
}

hi_s32 hdmi_osal_file_read(struct file *file, hi_char *buf, hi_u32 len)
{
    hi_s32 read_len;

    if (buf == HI_NULL) {
        return HI_SUCCESS;
    }
    read_len = osal_klib_fread(buf, len, file);

    return read_len;
}

hi_s32 hdmi_osal_file_write(struct file *file, hi_char *buf, hi_u32 len)
{
    hi_s32 write_len;

    if (buf == HI_NULL) {
        return HI_SUCCESS;
    }
    write_len = osal_klib_fwrite(buf, len, file);

    return write_len;
}

hi_u32 hdmi_osal_get_time_in_ms(hi_void)
{
    hi_u64 sys_time;
    sys_time = osal_div_u64(osal_sched_clock(), HDMI_MILLION);
    return (hi_u32)sys_time;
}

hi_u64 hdmi_osal_get_time_in_us(hi_void)
{
    hi_u64 curr_us;
    osal_timeval_t current_time = {0};

    osal_gettimeofday(&current_time);
    curr_us = ((hi_u64)current_time.tv_sec * HDMI_MILLION + current_time.tv_usec);

    return curr_us;
}


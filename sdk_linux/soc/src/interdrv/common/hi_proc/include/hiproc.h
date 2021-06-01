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

#ifndef __HI_PROC_H__
#define __HI_PROC_H__

#include "hi_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_PROC_CMD_LEN 50
#define MAX_PROC_NAME_LEN 50
#define HI_PROC_BUF_SIZE 65536
#define HI_PROC_WRITE_CMD_SIZE 32

#define HIPROC_DEVICE_NAME "hi_proc"
#define HIPROC_PFX "hiproc: "

#define HI_USER_PROC_READ_CMD "__read"
#define HI_USER_PROC_WRITE_CMD "__write"

typedef struct {
    hi_char *buf;
    hi_u32 size;
    hi_u32 offset;
} hi_proc_show_buf;

typedef struct {
    hi_char buf[HI_PROC_WRITE_CMD_SIZE];
    hi_u32 count;
    hi_s64 ppos;
} hi_proc_write_buf;

typedef struct {
    hi_char cmd[MAX_PROC_CMD_LEN];
    osal_proc_entry_t entry;
    hi_proc_write_buf write_buf;
} hi_proc_para;

typedef struct {
    hi_char name[MAX_PROC_NAME_LEN];
} hi_proc_name;

#define HI_ID_PROC 'Y'

#define USER_CREATE_PROC_ENTRY           _IOW(HI_ID_PROC, 1, hi_proc_name)
#define USER_REMOVE_PROC_ENTRY           _IOW(HI_ID_PROC, 2, hi_proc_name)
#define USER_PROC_GET_CMD                _IOWR(HI_ID_PROC, 5, hi_proc_para)
#define USER_PROC_WAKE_GET_CMD           _IOW(HI_ID_PROC, 4, hi_proc_para)
#define USER_PROC_WAKE_READ_TASK         _IOW(HI_ID_PROC, 6, hi_proc_show_buf)
#define USER_PROC_WAKE_WRITE_TASK        _IOW(HI_ID_PROC, 7, hi_proc_show_buf)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

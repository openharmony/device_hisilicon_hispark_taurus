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

#ifndef __ACODEC_H__
#define __ACODEC_H__
#include<stdio.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef enum {
    SDK_INIT_SUCC = 0,
    SDK_INIT_FAIL = 1,
    SDK_INIT_REENTRY = 2
} sdk_state_s;

extern void sdk_exit(void);
extern sdk_state_s sdk_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* End of #ifndef __ACODEC_H__ */

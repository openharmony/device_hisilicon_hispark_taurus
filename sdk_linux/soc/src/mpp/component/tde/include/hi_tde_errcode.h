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
#ifndef __TDE_ERRCODE_H__
#define __TDE_ERRCODE_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HI_ERR_TDE_BASE ((HI_S32)(((0x80UL + 0x20UL) << 24) | (100 << 16) | (4 << 13) | 1))

enum {
    HI_ERR_TDE_DEV_NOT_OPEN = HI_ERR_TDE_BASE, /* <  tde device not open yet */
    HI_ERR_TDE_DEV_OPEN_FAILED,                /* <  open tde device failed */
    HI_ERR_TDE_NULL_PTR,                       /* <  input parameters contain null ptr */
    HI_ERR_TDE_NO_MEM,                         /* <  malloc failed  */
    HI_ERR_TDE_INVALID_HANDLE,                 /* <  invalid job handle */
    HI_ERR_TDE_INVALID_PARA,                   /* <  invalid parameter */
    HI_ERR_TDE_NOT_ALIGNED,                    /* <  aligned error for position, stride, width */
    HI_ERR_TDE_MINIFICATION,                   /* <  invalid minification */
    HI_ERR_TDE_CLIP_AREA,                      /* <  clip area and operation area have no intersection */
    HI_ERR_TDE_JOB_TIMEOUT,                    /* <  blocked job wait timeout */
    HI_ERR_TDE_UNSUPPORTED_OPERATION,          /* <  unsupported operation */
    HI_ERR_TDE_QUERY_TIMEOUT,                  /* <  query time out */
    HI_ERR_TDE_INTERRUPT                       /* blocked job was interrupted */
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __TDE_ERRCODE_H__ */

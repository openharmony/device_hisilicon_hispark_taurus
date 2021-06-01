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

#ifndef __HI_ERRNO_H__
#define __HI_ERRNO_H__

#include "hi_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* 1010 0000b
 * VTOP use APPID from 0~31
 * so, hisilicon use APPID based on 32
 */
#define HI_ERR_APPID (0x80000000U + 0x20000000U)

typedef enum hiERR_LEVEL_E {
    EN_ERR_LEVEL_DEBUG   = 0,   /* debug-level                                  */
    EN_ERR_LEVEL_INFO    = 1,   /* informational                                */
    EN_ERR_LEVEL_NOTICE  = 2,   /* normal but significant condition             */
    EN_ERR_LEVEL_WARNING = 3,   /* warning conditions                           */
    EN_ERR_LEVEL_ERROR   = 4,   /* error conditions                             */
    EN_ERR_LEVEL_CRIT    = 5,   /* critical conditions                          */
    EN_ERR_LEVEL_ALERT   = 6,   /* action must be taken immediately             */
    EN_ERR_LEVEL_FATAL   = 7,   /* just for compatibility with previous version */
    EN_ERR_LEVEL_BUTT    = 30,
} ERR_LEVEL_E;

/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define HI_DEF_ERR(module, level, errid) \
    ((HI_S32)((HI_ERR_APPID) | ((HI_U32)(module) << 16U) | ((HI_U32)(level) << 13U) | ((HI_U32)errid)))

/* NOTE! the following defined all common error code,
 * all module must reserved 0~63 for their common error code
 */
typedef enum hiEN_ERR_CODE_E {
    EN_ERR_INVALID_DEVID = 1, /* invlalid device ID                           */
    EN_ERR_INVALID_CHNID = 2, /* invlalid channel ID                          */
    EN_ERR_ILLEGAL_PARAM = 3, /* at lease one parameter is illagal
                               * eg, an illegal enumeration value             */
    EN_ERR_EXIST         = 4, /* resource exists                              */
    EN_ERR_UNEXIST       = 5, /* resource unexists                            */

    EN_ERR_NULL_PTR      = 6, /* using a NULL point                           */

    EN_ERR_NOT_CONFIG    = 7, /* try to enable or initialize system, device
                               * or channel, before configing attribute       */

    EN_ERR_NOT_SUPPORT   = 8, /* operation or type is not supported by NOW    */
    EN_ERR_NOT_PERM      = 9, /* operation is not permitted
                              ** eg, try to change static attribute           */
    EN_ERR_INVALID_PIPEID = 10, /* invlalid pipe ID                           */
    EN_ERR_INVALID_STITCHGRPID  = 11, /* invlalid stitch group ID             */

    EN_ERR_NOMEM         = 12, /* failure caused by malloc memory              */
    EN_ERR_NOBUF         = 13, /* failure caused by malloc buffer              */

    EN_ERR_BUF_EMPTY     = 14, /* no data in buffer                            */
    EN_ERR_BUF_FULL      = 15, /* no buffer for new data                       */

    EN_ERR_SYS_NOTREADY  = 16, /* System is not ready, maybe not initialed or
                                * loaded. Returning the error code when opening
                                * a device file failed.                        */

    EN_ERR_BADADDR       = 17, /* bad address,
                                * eg. used for copy_from_user & copy_to_user   */

    EN_ERR_BUSY          = 18, /* resource is busy,
                                * eg. destroy a venc chn without unregister it */
    EN_ERR_SIZE_NOT_ENOUGH = 19, /* buffer size is smaller than the actual size required */

    EN_ERR_BUTT          = 63, /* maximum code, private error code of all modules
                                * must be greater than it                      */
} EN_ERR_CODE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_ERRNO_H__ */


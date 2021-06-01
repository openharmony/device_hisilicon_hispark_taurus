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

#ifndef __MKP_LOG_H__
#define __MKP_LOG_H__

#include "mkp_ioctl.h"
#include "hi_common_adapt.h"
#include "hi_debug_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    IOC_NR_LOG_SETLOGLEVEL = 0,
    IOC_NR_LOG_GETLOGLEVEL,
    IOC_NR_LOG_CLOSE,
    IOC_NR_LOG_SETWAITFLAG,
} ioc_nr_log;

#define LOG_SETLOGLEVEL_CTRL _IOW(IOC_TYPE_LOG, IOC_NR_LOG_SETLOGLEVEL, hi_log_level_conf)
#define LOG_GETLOGLEVEL_CTRL _IOWR(IOC_TYPE_LOG, IOC_NR_LOG_GETLOGLEVEL, hi_log_level_conf)
#define LOG_CLOSE_CTRL       _IO(IOC_TYPE_LOG, IOC_NR_LOG_CLOSE)
#define LOG_SETWAITFLAG_CTRL _IOW(IOC_TYPE_LOG, IOC_NR_LOG_SETWAITFLAG, hi_bool)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MKP_LOG_H__ */


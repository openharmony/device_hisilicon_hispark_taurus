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

#ifndef __HI_DEBUG_ADAPT_H__
#define __HI_DEBUG_ADAPT_H__

#ifndef __KERNEL__
#include <stdio.h>
#include <stdarg.h>
#endif

#ifdef __HuaweiLite__
#include<assert.h>
#else
#ifndef __KERNEL__
#include<assert.h>
#endif
#endif

#include "hi_type.h"
#include "hi_common_adapt.h"
#include "hi_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    hi_mod_id  mod_id;
    hi_s32  level;
    hi_char mod_name[16];
} hi_log_level_conf;

#define MPI_STATIC static inline

#ifdef __HuaweiLite__

#ifdef CONFIG_HI_LOG_TRACE_SUPPORT

    #define MPI_ASSERT(expr)              \
    do {                                   \
        if (!(expr)) {                    \
            (hi_void)printf("\nASSERT at:\n"       \
                   "  >Function : %s\n"   \
                   "  >Line No. : %d\n"   \
                   "  >Condition: %s\n",  __FUNCTION__, __LINE__, #expr); \
            assert(0 == 1); \
        } \
    } while (0)

#else

    #define MPI_ASSERT(expr)

#endif

#else

#ifndef __KERNEL__

#ifdef CONFIG_HI_LOG_TRACE_SUPPORT

    #define MPI_ASSERT(expr)              \
    do {                                   \
        if (!(expr)) {                    \
            (hi_void)printf("\nASSERT at:\n"       \
                   "  >Function : %s\n"   \
                   "  >Line No. : %d\n"   \
                   "  >Condition: %s\n",  __FUNCTION__, __LINE__, #expr); \
            assert(0 == 1); \
        } \
    } while (0)

#else

    #define MPI_ASSERT(expr)

#endif

#endif

#endif

#ifdef __KERNEL__

int hi_log(hi_s32 level, hi_mod_id mod_id, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#endif /* end of __KERNEL__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEBUG_ADAPT_H__ */


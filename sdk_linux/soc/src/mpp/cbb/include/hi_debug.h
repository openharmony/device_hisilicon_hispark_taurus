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

#ifndef __HI_DEBUG_H__
#define __HI_DEBUG_H__

#ifndef __KERNEL__
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#endif

#include "hi_type.h"
#include "hi_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define _EX__FILE_LINE(fxx, lxx) "[File]:" fxx"\n[Line]:"#lxx"\n[Info]:"
#define EX__FILE_LINE(fxx, lxx) _EX__FILE_LINE(fxx, lxx)
#define __FILE_LINE__ EX__FILE_LINE(__FILE__, __LINE__)

#define HI_DBG_EMERG      0   /* system is unusable                   */
#define HI_DBG_ALERT      1   /* action must be taken immediately     */
#define HI_DBG_CRIT       2   /* critical conditions                  */
#define HI_DBG_ERR        3   /* error conditions                     */
#define HI_DBG_WARN       4   /* warning conditions                   */
#define HI_DBG_NOTICE     5   /* normal but significant condition     */
#define HI_DBG_INFO       6   /* informational                        */
#define HI_DBG_DEBUG      7   /* debug-level messages                 */

typedef struct hiLOG_LEVEL_CONF_S {
    MOD_ID_E  enModId;
    HI_S32    s32Level;
    HI_CHAR   cModName[16];
} LOG_LEVEL_CONF_S;

#ifndef __KERNEL__

/* For User Mode : HI_PRINT, HI_ASSERT, HI_TRACE */
#define HI_PRINT      (hi_void)printf

/* #ifdef HI_DEBUG */
#ifdef CONFIG_HI_LOG_TRACE_SUPPORT
    /* Using samples:   HI_ASSERT(x>y); */
    #define HI_ASSERT(expr)               \
    do {                                   \
        if (!(expr)) {                    \
            (hi_void)printf("\nASSERT at:\n"       \
                   "  >Function : %s\n"   \
                   "  >Line No. : %d\n"   \
                   "  >Condition: %s\n",  __FUNCTION__, __LINE__, #expr); \
            assert(0); \
        } \
    } while (0)

    /* Using samples:
     * HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI, "Test %d, %s\n", 12, "Test");
     */
    #define HI_TRACE(level, enModId, fmt...) \
        do {                                 \
            if (level <= HI_DBG_ERR)         \
                (hi_void)fprintf(stderr, ##fmt);       \
        } while (0)

#else
    #define HI_ASSERT(expr)
    #define HI_TRACE(level, enModId, fmt...)
#endif

#else

/* For Linux Kernel : HI_PRINT, HI_ASSERT, HI_TRACE */
#define HI_PRINT      (hi_void)osal_printk

int HI_LOG(HI_S32 level, MOD_ID_E enModId, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

/* #ifdef HI_DEBUG */
#ifdef CONFIG_HI_LOG_TRACE_SUPPORT
    /* Using samples:   HI_ASSERT(x>y); */
    #define HI_ASSERT(expr)               \
    do {                                   \
        if (!(expr)) {                    \
            osal_panic("\nASSERT at:\n"   \
                  "  >Function : %s\n"    \
                  "  >Line No. : %d\n"    \
                  "  >Condition: %s\n",   __FUNCTION__, __LINE__, #expr); \
        } \
    } while (0)

    /* Using samples:
     * HI_TRACE(HI_DBG_DEBUG, HI_ID_CMPI, "Test %d, %s\n", 12, "Test");
     */
    #define HI_TRACE (hi_void)HI_LOG
#else
    #define HI_ASSERT(expr)
    #define HI_TRACE(level, enModId, fmt...)
#endif

#endif /* end of __KERNEL__ */

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_EMERG)
#define HI_EMERG_TRACE(mod, fmt...) HI_TRACE(HI_DBG_EMERG, mod, fmt)
#else
#define HI_EMERG_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_ALERT)
#define HI_ALERT_TRACE(mod, fmt...) HI_TRACE(HI_DBG_ALERT, mod, fmt)
#else
#define HI_ALERT_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_CRIT)
#define HI_CRIT_TRACE(mod, fmt...) HI_TRACE(HI_DBG_CRIT, mod, fmt)
#else
#define HI_CRIT_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_ERR)
#define HI_ERR_TRACE(mod, fmt...) HI_TRACE(HI_DBG_ERR, mod, fmt)
#else
#define HI_ERR_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_WARN)
#define HI_WARN_TRACE(mod, fmt...) HI_TRACE(HI_DBG_WARN, mod, fmt)
#else
#define HI_WARN_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_NOTICE)
#define HI_NOTICE_TRACE(mod, fmt...) HI_TRACE(HI_DBG_NOTICE, mod, fmt)
#else
#define HI_NOTICE_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_INFO)
#define HI_INFO_TRACE(mod, fmt...) HI_TRACE(HI_DBG_INFO, mod, fmt)
#else
#define HI_INFO_TRACE(mod, fmt...)
#endif

#if (CONFIG_HI_LOG_TRACE_LEVEL >= HI_DBG_DEBUG)
#define HI_DEBUG_TRACE(mod, fmt...) HI_TRACE(HI_DBG_DEBUG, mod, fmt)
#else
#define HI_DEBUG_TRACE(mod, fmt...)
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEBUG_H__ */


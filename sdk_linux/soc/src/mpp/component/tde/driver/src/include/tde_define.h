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

#ifndef __TDE_DEFINE_H__
#define __TDE_DEFINE_H__

#ifdef HI_BUILD_IN_BOOT
#include "exports.h"
#include "hi_common.h"
#include "hi_go_common.h"
#endif
#include "hi_type.h"
#include "hi_debug.h"
#include "wmalloc.h"
#include "tde_config.h"
#include "tde_adp.h"
#include "hi_osal.h"
#include "mm_ext.h"
#include "proc_ext.h"
#include "mod_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Pixel format classify */
typedef enum {
    TDE_COLORFMT_CATEGORY_ARGB,
    TDE_COLORFMT_CATEGORY_CLUT,
    TDE_COLORFMT_CATEGORY_AN,
    TDE_COLORFMT_CATEGORY_YCBCR,
    TDE_COLORFMT_CATEGORY_BYTE,
    TDE_COLORFMT_CATEGORY_HALFWORD,
    TDE_COLORFMT_CATEGORY_MB,
    TDE_COLORFMT_CATEGORY_BUTT
} tde_colorfmt_category;

/* Node submit type, Independent operation node, Operated head node, Operated middle node */
typedef enum {
    TDE_NODE_SUBM_ALONE = 0,  /* current node submit as independent operated node */
    TDE_NODE_SUBM_PARENT = 1, /* current node submit as operated parent node */
    TDE_NODE_SUBM_CHILD       /* current node submit as operated child node */
} tde_node_subm_type;

/* Operation setting information node */
typedef struct {
    hi_void *buf;    /* setting information node buffer */
    hi_u32 node_sz; /* current node occupied size, united in byte */
    hi_u64 update; /* current node update flag */
    hi_u64 phy_addr;  /* current node addr in hardware list */
} tde_node_buf;

/* Notified mode  after current node's job end */
typedef enum {
    TDE_JOB_NONE_NOTIFY = 0, /* none notify after job end */
    TDE_JOB_COMPL_NOTIFY,    /* notify after job end */
    TDE_JOB_WAKE_NOTIFY,     /* wake noytify after job end */
    TDE_JOB_NOTIFY_BUTT
} tde_notify_mode;

#define TDE_MAX_WAIT_TIMEOUT 2000

#define MMB_ADDR_INVALID 1

#define tde_trace(level, fmt...)                                                           \
    do {                                                                                   \
        HI_TRACE(level, HI_ID_TDE, "[Func]:%s [Line]:%d [Info]:", __FUNCTION__, __LINE__); \
        HI_TRACE(level, HI_ID_TDE, ##fmt);                                                 \
    } while (0)

#define tde_fatal(fmt...) tde_trace(HI_DBG_EMERG, fmt)
#define tde_error(fmt...) tde_trace(HI_DBG_ERR, fmt)
#define tde_warning(fmt...) tde_trace(HI_DBG_WARN, fmt)
#define tde_info(fmt...) tde_trace(HI_DBG_INFO, fmt)

hi_void *tde_malloc(hi_u32 size);
hi_void tde_free(hi_void *ptr);
#ifdef CONFIG_TDE_PM_ENABLE
typedef struct {
    hi_s32 id;
    const hi_s8 *name;
    struct device dev;
} pm_basedev;
#endif
#define tde_spin_lock(lock, flag) osal_spin_lock_irqsave((lock), &(flag))
#define tde_spin_unlock(lock, flag) osal_spin_unlock_irqrestore((lock), &(flag))

#define tde_free_mmb(phyaddr)              \
    do {                                   \
        if (phyaddr != MMB_ADDR_INVALID) { \
            cmpi_mmz_free(phyaddr, HI_NULL);  \
        }                                  \
    } while (0)

#define tde_get_phyaddr_mmb(bufname, size, phyaddr)     \
    do {                                                \
        phyaddr = cmpi_mmz_malloc(HI_NULL, bufname, size); \
        if (phyaddr == 0) {                             \
            tde_error("new_mmb failed!");               \
            phyaddr = 0;                                \
        }                                               \
    } while (0)

#define tde_unmap_mmb(virtaddr) cmpi_unmap((hi_void *)virtaddr)

#define tde_flush_cached_mmb(virtaddr, phyaddr, len)    \
    do {                                                \
        osal_flush_dcache_area(virtaddr, phyaddr, len); \
    } while (0)

#define tde_unequal_eok_return(ret)                       \
    do {                                                  \
        if ((ret) != EOK) {                               \
            tde_error("secure function failure\n");       \
            return HI_ERR_TDE_INVALID_PARA;              \
        }                                                 \
    } while (0)

#define tde_reg_map(base, size) osal_ioremap_nocache((base), (size))

#define tde_reg_unmap(base) osal_iounmap((hi_void *)(base), 0)

#define tde_init_waitqueue_head(pqueue) osal_wait_init(pqueue)

#define tde_wait_event_interruptible_timeout(queue, condition, timeout) osal_wait_timeout(queue, condition, timeout)

#define tde_wakeup_interruptible(pqueue) osal_wakeup(pqueue)

typedef struct timeval tde_timeval;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __TDE_DEFINE_H__ */

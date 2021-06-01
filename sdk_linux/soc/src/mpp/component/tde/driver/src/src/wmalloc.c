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

#include "wmalloc.h"
#include "securec.h"
#include "tde_define.h"
#include "tde_adp.h"

typedef struct {
    hi_u32 n_size;
    hi_u16 n_free;
    hi_u16 n_first;
    hi_u16 n_unit_size;
#if HI_TDE_MEMCOUNT_SUPPORT
    hi_u16 n_max_used; /* max used unit number */
    hi_u16 n_max_num;  /* max unit number */
#endif
    hi_void *start_addr;
    struct _memory_block *next;
} memory_block;

typedef enum {
    UNIT_SIZE_CMD = 0,
    UNIT_SIZE_JOB,
    UNIT_SIZE_NODE,
    UNIT_SIZE_FILTER,
    UNIT_SIZE_BUTT
} unit_size;

static memory_block g_stru_mem_block[UNIT_SIZE_BUTT];

#ifndef HI_BUILD_IN_BOOT
static osal_spinlock_t g_mem_lock;
#endif

#define print_mem_info()                                                                                              \
    do {                                                                                                            \
        tde_info("-----------------------------------------------------\n");                                        \
        tde_info("\tmem_block info\ttotal\tfree\n");                                                                 \
        tde_info("\t%d\t%lld\t%d\n", CMD_SIZE, HI_TDE_CMD_NUM, g_stru_mem_block[UNIT_SIZE_CMD].n_free);                \
        tde_info("\t%d\t%lld\t%d\n", JOB_SIZE, HI_TDE_JOB_NUM, g_stru_mem_block[UNIT_SIZE_JOB].n_free);                \
        tde_info("\t%ld\t%lld\t%d\n", (hi_ulong)NODE_SIZE, HI_TDE_NODE_NUM, g_stru_mem_block[UNIT_SIZE_NODE].n_free); \
        tde_info("\t%d\t%d\t%d\n", FILTER_SIZE, HI_TDE_FILTER_NUM, g_stru_mem_block[UNIT_SIZE_FILTER].n_free);         \
        tde_info("------------------------------------------------------\n");                                       \
    } while (0)

static hi_s32 memory_block_init(unit_size size, hi_u16 n_unit_num, hi_void *addr)
{
    hi_u16 i;

    hi_u8 *data = (hi_u8 *)addr;

    if ((size >= UNIT_SIZE_BUTT) || (addr == HI_NULL) || (n_unit_num == 0)) {
        return HI_FAILURE;
    }
    if (size == UNIT_SIZE_CMD) {
        g_stru_mem_block[size].n_unit_size = CMD_SIZE;
    } else if (size == UNIT_SIZE_JOB) {
        g_stru_mem_block[size].n_unit_size = JOB_SIZE;
    } else if (size == UNIT_SIZE_NODE) {
        g_stru_mem_block[size].n_unit_size = NODE_SIZE;
    } else if (size == UNIT_SIZE_FILTER) {
        g_stru_mem_block[size].n_unit_size = FILTER_SIZE;
    }
    for (i = 1; i < n_unit_num; i++) {
        /* Don't flag for last unit,for last unit is ready for assigned, which is say no next unit can be assigned */
        *(hi_u16 *)data = i;

        data += g_stru_mem_block[size].n_unit_size;
    }

    g_stru_mem_block[size].n_first = 0;
    g_stru_mem_block[size].n_free = n_unit_num;
    g_stru_mem_block[size].n_size = n_unit_num * g_stru_mem_block[size].n_unit_size;
    g_stru_mem_block[size].next = HI_NULL;
    g_stru_mem_block[size].start_addr = addr;

#if HI_TDE_MEMCOUNT_SUPPORT
    g_stru_mem_block[size].n_max_num = n_unit_num;
    g_stru_mem_block[size].n_max_used = 0;
#endif

    return HI_SUCCESS;
}

static hi_void *malloc_unit(unit_size size)
{
    unsigned long lockflags;

    memory_block *block = HI_NULL;
    hi_u8 *free = HI_NULL;

    tde_spin_lock(&g_mem_lock, lockflags);
    block = &g_stru_mem_block[size];
    tde_info("eUnitSize %d, malloc units:%d, first malloc unit:%d...\n", block->n_unit_size, block->n_free,
             block->n_first);
    if (!block->n_free) {
        tde_spin_unlock(&g_mem_lock, lockflags);
        return HI_NULL;
    }

    free = (hi_u8 *)block->start_addr + block->n_first * block->n_unit_size;

    block->n_first = *(hi_u16 *)free;
    block->n_free--;
#if HI_TDE_MEMCOUNT_SUPPORT
    if ((g_stru_mem_block[size].n_max_num - block->n_free) > g_stru_mem_block[size].n_max_used) {
        g_stru_mem_block[size].n_max_used = g_stru_mem_block[size].n_max_num - block->n_free;
    }
#endif
    tde_spin_unlock(&g_mem_lock, lockflags);
    (hi_void)memset_s(free, block->n_unit_size, 0, block->n_unit_size);
    return free;
}

static hi_void *wmalloc(unsigned long size)
{
    unit_size i;
    hi_void *malloc = HI_NULL;

    if ((size > FILTER_SIZE) || (size == 0)) {
        return HI_NULL;
    }
    if (size <= CMD_SIZE) {
        for (i = UNIT_SIZE_CMD; i < UNIT_SIZE_BUTT; i++) {
            malloc = malloc_unit(i);
            if (malloc != HI_NULL) {
                return malloc;
            }
        }
        return HI_NULL;
    } else if (size <= JOB_SIZE) {
        for (i = UNIT_SIZE_JOB; i < UNIT_SIZE_BUTT; i++) {
            malloc = malloc_unit(i);
            if (malloc != HI_NULL) {
                return malloc;
            }
        }
        return HI_NULL;
    } else if (size <= NODE_SIZE) {
        for (i = UNIT_SIZE_NODE; i < UNIT_SIZE_BUTT; i++) {
            malloc = malloc_unit(i);
            if (malloc != HI_NULL) {
                return malloc;
            }
        }
        return HI_NULL;
    } else {
        return malloc_unit(UNIT_SIZE_FILTER);
    }
}

static hi_s32 free_unit(unit_size size, hi_void *ptr)
{
    unsigned long lockflags;
    memory_block *block = HI_NULL;
    tde_spin_lock(&g_mem_lock, lockflags);
    block = &g_stru_mem_block[size];

    if (((hi_u8 *)ptr < (hi_u8 *)block->start_addr) ||
        ((hi_u8 *)ptr >= ((hi_u8 *)block->start_addr + block->n_size))) {
        tde_spin_unlock(&g_mem_lock, lockflags);
        return HI_FAILURE;
    }

    block->n_free++;
    *(hi_u16 *)ptr = block->n_first; /* point to next unit can be assigned */

    if (block->n_unit_size == 0) {
        tde_spin_unlock(&g_mem_lock, lockflags);
        return HI_FAILURE;
    }
    block->n_first = (hi_u16)(((hi_u8 *)ptr - (hi_u8 *)(block->start_addr)) / block->n_unit_size);

    tde_info("eUnitSize:%hu,first free unit:%hu, free units:%hu\n", block->n_unit_size, block->n_first, block->n_free);
    tde_spin_unlock(&g_mem_lock, lockflags);
    return HI_SUCCESS;
}

static hi_s32 wfree(hi_void *ptr)
{
    unit_size i;
    for (i = UNIT_SIZE_CMD; i < UNIT_SIZE_BUTT; i++) {
        if (free_unit(i, ptr) == HI_SUCCESS) {
            return HI_SUCCESS;
        }
    }
    tde_error("Free mem failed!vir:0x%pK, phy:%llx\n", ptr, wgetphy(ptr));
    return HI_FAILURE;
}

hi_void *tde_malloc(hi_u32 size)
{
    hi_void *ptr = HI_NULL;
    ptr = (hi_void *)wmalloc(size);
    return ptr;
}

hi_void tde_free(hi_void *ptr)
{
    if (ptr == HI_NULL) {
        return;
    }
    if (wfree(ptr) != HI_SUCCESS) {
        tde_error("Free mem failed\n");
    }
    return;
}

#define TDE_MIN_BUFFER ((FILTER_SIZE)*HI_TDE_FILTER_NUM + ((CMD_SIZE) + (NODE_SIZE) + (JOB_SIZE)) * 2)
#define TDE_MAX_BUFFER (1024 * 1024)
static hi_u64 g_mem_poolphy_addr;
static hi_void *g_mem_pool_vrt_addr = HI_NULL;
static hi_u32 g_tde_buf = 0;

#ifdef HI_BUILD_IN_BOOT
/* boot Memory optimization */
#define HI_TDE_CMD_NUM 20
#define HI_TDE_JOB_NUM 1
#define HI_TDE_NODE_NUM 20
#else
#define HI_TDE_CMD_NUM \
    (hi_u64)((g_tde_buf - (FILTER_SIZE)*HI_TDE_FILTER_NUM) / ((CMD_SIZE) + (NODE_SIZE) + (JOB_SIZE)))
#define HI_TDE_JOB_NUM HI_TDE_CMD_NUM
#define HI_TDE_NODE_NUM HI_TDE_CMD_NUM
#endif

#define TDE_CMD_OFFSET 0
#define TDE_JOB_OFFSET ((HI_TDE_CMD_NUM)*CMD_SIZE)
#define TDE_NODE_OFFSET (TDE_JOB_OFFSET + ((HI_TDE_JOB_NUM)*JOB_SIZE))
#define TDE_FILTER_OFFSET (TDE_NODE_OFFSET + ((HI_TDE_NODE_NUM)*NODE_SIZE))
#define TDE_MEMPOOL_SIZE                                                                   \
    ((HI_TDE_CMD_NUM)*CMD_SIZE + (HI_TDE_JOB_NUM)*JOB_SIZE + (HI_TDE_NODE_NUM)*NODE_SIZE + \
     (HI_TDE_FILTER_NUM)*FILTER_SIZE)
#ifdef HI_BUILD_IN_BOOT
#if !defined(HI_TDE_BUFFER)
#define HI_TDE_BUFFER 0x20000
#endif
#endif

hi_void tde_init_set_buf(hi_u32 temp_buf)
{
    g_tde_buf = temp_buf;
}

hi_s32 wmeminit(void)
{
    hi_s32 ret;
    if (g_tde_buf > TDE_MAX_BUFFER) {
        g_tde_buf = TDE_MAX_BUFFER;
    } else if (g_tde_buf < TDE_MIN_BUFFER) {
        g_tde_buf = TDE_MIN_BUFFER;
    }

    tde_get_phyaddr_mmb("TDE_MEMPOOL_MMB", TDE_MEMPOOL_SIZE, g_mem_poolphy_addr);
    if (g_mem_poolphy_addr == 0) {
        tde_error("malloc mempool buffer failed!\n");
        return HI_FAILURE;
    }
#ifdef TDE_CACH_STRATEGY
    g_mem_pool_vrt_addr = (hi_void *)CMPI_Remap_Cached(g_mem_poolphy_addr, TDE_MEMPOOL_SIZE);
    if (g_mem_pool_vrt_addr == 0) {
        tde_free_mmb(g_mem_poolphy_addr);
        return HI_FAILURE;
    }

#else
    g_mem_pool_vrt_addr = (hi_void *)cmpi_remap_nocache(g_mem_poolphy_addr, TDE_MEMPOOL_SIZE);
    if (g_mem_pool_vrt_addr == 0) {
        tde_free_mmb(g_mem_poolphy_addr);
        return HI_FAILURE;
    }

#endif

    if (g_mem_pool_vrt_addr == HI_NULL) {
        tde_free_mmb(g_mem_poolphy_addr);
        g_mem_poolphy_addr = 0;
        return HI_FAILURE;
    }
    ret = memory_block_init(UNIT_SIZE_CMD, HI_TDE_CMD_NUM, (g_mem_pool_vrt_addr));
    if (ret != HI_SUCCESS) {
    }
    ret = memory_block_init(UNIT_SIZE_JOB, HI_TDE_JOB_NUM, ((hi_u8 *)g_mem_pool_vrt_addr + TDE_JOB_OFFSET));
    if (ret != HI_SUCCESS) {
    }
    ret = memory_block_init(UNIT_SIZE_NODE, HI_TDE_NODE_NUM, ((hi_u8 *)g_mem_pool_vrt_addr + TDE_NODE_OFFSET));
    if (ret != HI_SUCCESS) {
    }
    ret = memory_block_init(UNIT_SIZE_FILTER, HI_TDE_FILTER_NUM, ((hi_u8 *)g_mem_pool_vrt_addr + TDE_FILTER_OFFSET));
    if (ret != HI_SUCCESS) {
    }
    osal_spin_lock_init(&g_mem_lock);
    print_mem_info();

    return HI_SUCCESS;
}

hi_void wmemterm(void)
{
    print_mem_info();
#ifndef HI_BUILD_IN_BOOT
    tde_unmap_mmb(g_mem_pool_vrt_addr);
    tde_free_mmb(g_mem_poolphy_addr);
#endif
    g_mem_poolphy_addr = 0;
    g_mem_pool_vrt_addr = HI_NULL;
#ifndef HI_BUILD_IN_BOOT
    osal_spin_lock_destroy(&g_mem_lock);
#endif
    return;
}

hi_u64 wgetphy(hi_void *ptr)
{
    if (ptr == HI_NULL) {
        return HI_SUCCESS;
    }

    if ((ptr < g_mem_pool_vrt_addr) || ((hi_u8 *)ptr >= ((hi_u8 *)g_mem_pool_vrt_addr + TDE_MEMPOOL_SIZE))) {
        return HI_SUCCESS;
    }

    return (g_mem_poolphy_addr + ((hi_u8 *)ptr - (hi_u8 *)g_mem_pool_vrt_addr));
}

hi_void *wgetvrt(hi_u64 phyaddr)
{
    if ((phyaddr < g_mem_poolphy_addr) || (phyaddr >= (g_mem_poolphy_addr + TDE_MEMPOOL_SIZE))) {
        return HI_NULL;
    }

    return (hi_void *)((hi_u8 *)g_mem_pool_vrt_addr + (phyaddr - g_mem_poolphy_addr));
}

hi_u32 wgetfreenum(hi_void)
{
    unit_size size = 0;
    hi_u32 free_unit_num = g_stru_mem_block[size].n_free;

    for (size = UNIT_SIZE_CMD; size < UNIT_SIZE_FILTER; size++) {
        free_unit_num = (free_unit_num > g_stru_mem_block[size].n_free) ? g_stru_mem_block[size].n_free :
                         free_unit_num;
    }

    return free_unit_num;
}

#ifndef HI_BUILD_IN_BOOT

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
osal_proc_entry_t *wprintinfo(osal_proc_entry_t *page)
{
#if HI_TDE_MEMCOUNT_SUPPORT
    hi_u32 max_used_cmd, max_used_job, max_used_node, max_used_filter;
#else
    hi_u32 free_cmd, free_job, free_node, free_filter;
#endif
    if (page == HI_NULL) {
        return HI_SUCCESS;
    }

#if HI_TDE_MEMCOUNT_SUPPORT
    max_used_cmd = g_stru_mem_block[UNIT_SIZE_CMD].n_max_used;
    max_used_job = g_stru_mem_block[UNIT_SIZE_JOB].n_max_used;
    max_used_node = g_stru_mem_block[UNIT_SIZE_NODE].n_max_used;
    max_used_filter = g_stru_mem_block[UNIT_SIZE_FILTER].n_max_used;
#else
    free_cmd = g_stru_mem_block[UNIT_SIZE_CMD].n_free;
    free_job = g_stru_mem_block[UNIT_SIZE_JOB].n_free;
    free_node = g_stru_mem_block[UNIT_SIZE_NODE].n_free;
    free_filter = g_stru_mem_block[UNIT_SIZE_FILTER].n_free;
#endif
    osal_seq_printf(page, "--------- Hisilicon TDE Memory Pool Info ---------\n");
#if HI_TDE_MEMCOUNT_SUPPORT
    osal_seq_printf(page, "     Type         Total       MaxUsed\n");
    osal_seq_printf(page, "[Unit %d ]   %8u  %8u\n", CMD_SIZE, HI_TDE_CMD_NUM, max_used_cmd);
    osal_seq_printf(page, "[Unit %d ]   %8u  %8u\n", JOB_SIZE, HI_TDE_JOB_NUM, max_used_job);
    osal_seq_printf(page, "[Unit %d]   %8u  %8u\n", NODE_SIZE, HI_TDE_NODE_NUM, max_used_job);
    osal_seq_printf(page, "[Unit%d]   %8u  %8u\n", FILTER_SIZE, HI_TDE_FILTER_NUM, max_used_filter);
    osal_seq_printf(page, "[Total   ]   %8uK %8uK\n", TDE_MEMPOOL_SIZE / 1024, /* 1024 Demotion byte */
                    (CMD_SIZE * max_used_cmd + JOB_SIZE * max_used_job + NODE_SIZE * max_used_node + FILTER_SIZE *
                     max_used_filter) / 1024); /* 1024 Demotion byte */
#else
    osal_seq_printf(page, "     Type         Total       Used\n");
    osal_seq_printf(page, "[Unit %d ]   %8lld  %8lld\n", CMD_SIZE, HI_TDE_CMD_NUM, HI_TDE_CMD_NUM - free_cmd);
    osal_seq_printf(page, "[Unit %d ]   %8lld  %8lld\n", JOB_SIZE, HI_TDE_JOB_NUM, HI_TDE_JOB_NUM - free_job);
    osal_seq_printf(page, "[Unit %ld]   %8lld  %8lld\n", (unsigned long)NODE_SIZE, HI_TDE_NODE_NUM,
                    HI_TDE_NODE_NUM - free_node);
    osal_seq_printf(page, "[Unit%d]   %8d  %8d\n", FILTER_SIZE, HI_TDE_FILTER_NUM, HI_TDE_FILTER_NUM - free_filter);
    osal_seq_printf(page, "[Total   ]   %8lldK %8lldK\n", TDE_MEMPOOL_SIZE / 1024, /* 1024 Demotion byte */
                    (TDE_MEMPOOL_SIZE - (CMD_SIZE * free_cmd + JOB_SIZE * free_job + NODE_SIZE * free_node +
                    FILTER_SIZE * free_filter)) / 1024); /* 1024 Demotion byte */

#endif
    return page;
}
#endif

#endif

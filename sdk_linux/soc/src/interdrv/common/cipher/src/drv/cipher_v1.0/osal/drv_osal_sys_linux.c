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

#include "drv_osal_lib.h"
#include <linux/dmapool.h>
#include <asm/cacheflush.h>
#include "securec.h"

/* under TEE, we only can malloc secure mmz at system setup,
 * then map the mmz to Smmu, but the smmu can't map to cpu address,
 * so we must save the cpu address in a static table when malloc and map mmz.
 * when call crypto_mem_map, we try to query the table to get cpu address firstly,
 * if can't get cpu address from the table, then call system api to map it.
 */
#define CRYPTO_MEM_MAP_TABLE_DEPTH      32
#define DMA_ALLOC_MAX_SIZE              (1024 * 256)

typedef struct {
    hi_u32         valid;
    compat_addr    dma;
    hi_void        *via;
} crypto_mem_map_table;

static crypto_mem_map_table g_local_map_table[CRYPTO_MEM_MAP_TABLE_DEPTH];

#ifdef CONFIG_64BIT
#define CRYPTO_FLUSH_DCACHE_AREA        __flush_dcache_area
#else
#define CRYPTO_FLUSH_DCACHE_AREA        __cpuc_flush_dcache_area
#endif

/* in 32-bit system. */
#define SYS_32BITS                      32

/* in 64-bit system. */
#define SYS_64BITS                      64

/* mmz/mmu api. */
static hi_s32 cipher_dma_set_mask(struct device *dev)
{
    hi_s32 ret;

    ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(SYS_64BITS));
    if (ret == HI_SUCCESS) {
        return ret;
    }

    ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(SYS_32BITS));
    if (ret != HI_SUCCESS) {
        hi_log_error("Failed to set DMA mask %d.\n", ret);
        return ret;
    }

    return HI_SUCCESS;
}

static hi_s32  cipher_dma_alloc_coherent(crypto_mem *mem, hi_u32 type, const hi_char *name, hi_u32 size)
{
    struct device *dev = HI_NULL;
    hi_s32 ret;
    hi_s32 i;

    crypto_unused(type);
    crypto_unused(name);

    if (mem == HI_NULL) {
        hi_log_error("mem is null.\n");
        return HI_ERR_CIPHER_INVALID_POINT;
    }

    if (size > DMA_ALLOC_MAX_SIZE) {
        hi_log_error("dma alloc coherent with invalid size(0x%x).\n", size);
        return HI_ERR_CIPHER_INVALID_PARAM;
    }

    dev = (struct device *)cipher_get_device();
    if (dev == HI_NULL) {
        hi_log_error("cipher_get_device error.\n");
        return HI_ERR_CIPHER_INVALID_POINT;
    }

    ret = cipher_dma_set_mask(dev);
    if (ret != HI_SUCCESS) {
        hi_log_error("cipher dma set mask failed.\n");
        return ret;
    }

    mem->dma_size = size;
    mem->dma_virt = dma_alloc_coherent(dev, mem->dma_size, (dma_addr_t *)(&addr_u64(mem->dma_addr)), GFP_ATOMIC);
    if (mem->dma_virt == HI_NULL) {
        hi_log_error("dma_alloc_coherent error.\n");
        return HI_ERR_CIPHER_FAILED_MEM;
    }
    addr_u64(mem->mmz_addr) = addr_u64(mem->dma_addr);

    /* save the map info. */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid == HI_FALSE) {
            addr_u64(g_local_map_table[i].dma) = addr_u64(mem->dma_addr);
            g_local_map_table[i].via = mem->dma_virt;
            g_local_map_table[i].valid = HI_TRUE;
            hi_log_debug("map local map %d, dam 0x%x, via 0x%pK\n",
                         i, addr_u64(mem->dma_addr), mem->dma_virt);
            break;
        }
    }

    return HI_SUCCESS;
}

static hi_s32 cipher_dma_free_coherent(crypto_mem *mem)
{
    struct device *dev = HI_NULL;
    hi_s32 ret;
    hi_s32 i;
    crypto_mem mem_temp;

    hi_log_chk_param_return(mem == HI_NULL);
    hi_log_chk_param_return(mem->dma_virt == HI_NULL);

    (hi_void)memset_s(&mem_temp, sizeof(mem_temp), 0, sizeof(mem_temp));
    if (memcpy_s(&mem_temp, sizeof(mem_temp), mem, sizeof(crypto_mem)) != EOK) {
        hi_log_print_func_err(memcpy_s, HI_ERR_CIPHER_MEMCPY_S_FAILED);
        return HI_ERR_CIPHER_MEMCPY_S_FAILED;
    }

    dev = (struct device *)cipher_get_device();
    if (dev == NULL) {
        hi_log_error("cipher_get_device error.\n");
        return HI_ERR_CIPHER_INVALID_POINT;
    }

    ret = cipher_dma_set_mask(dev);
    if (ret != HI_SUCCESS) {
        hi_log_error("cipher dma set mask failed.\n");
        return ret;
    }

    dma_free_coherent(dev, mem->dma_size, mem->dma_virt, (dma_addr_t)addr_u64(mem->dma_addr));

    /* remove the map info. */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid && addr_u64(g_local_map_table[i].dma) == addr_u64(mem_temp.dma_addr)) {
            addr_u64(g_local_map_table[i].dma) = 0x00;
            g_local_map_table[i].via = HI_NULL;
            g_local_map_table[i].valid = HI_FALSE;
            hi_log_debug("unmap local map %d, dam 0x%x, via 0x%pK\n",
                         i, addr_u64(mem_temp.dma_addr), mem_temp.dma_virt);
            break;
        }
    }
    (hi_void)memset_s(mem, sizeof(crypto_mem), 0, sizeof(crypto_mem));

    return HI_SUCCESS;
}

/*
 * brief allocate and map a mmz or smmu memory
 * we can't allocate smmu directly during TEE boot period.
 * in addition, the buffer of cipher node list must be mmz.
 * so here we have to allocate a mmz memory then map to smmu if necessary.
 */
static hi_s32 hash_mem_alloc_remap(crypto_mem *mem, hi_u32 type, const char *name, hi_u32 size)
{
    hi_u32 i;

    crypto_unused(type);
    crypto_unused(name);

    (hi_void)memset_s(mem, sizeof(crypto_mem), 0, sizeof(crypto_mem));

    hi_log_debug("mem_alloc_remap()- name %s, size 0x%x\n", name, size);

    mem->dma_size = size;
    mem->dma_virt = kzalloc(size, GFP_KERNEL);
    if (mem->dma_virt == NULL) {
        return HI_ERR_CIPHER_FAILED_MEM;
    }

    addr_u64(mem->mmz_addr) = (hi_phys_addr_t)virt_to_phys(mem->dma_virt);
    if (addr_u64(mem->mmz_addr) == 0) {
        if (mem->dma_virt != HI_NULL) {
            kfree(mem->dma_virt);
            mem->dma_virt = HI_NULL;
        }

        return HI_ERR_CIPHER_FAILED_MEM;
    }

    addr_u64(mem->dma_addr) = addr_u64(mem->mmz_addr);

    hi_log_debug("MMZ/MMU malloc, MMZ 0x%x, MMZ/MMU 0x%x, VIA 0x%pK, SIZE 0x%x\n",
                 addr_u64(mem->mmz_addr), addr_u64(mem->dma_addr), mem->dma_virt, size);

    mem->user_buf = HI_NULL;

    /* save the map info. */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid == HI_FALSE) {
            addr_u64(g_local_map_table[i].dma) = addr_u64(mem->dma_addr);
            g_local_map_table[i].via = mem->dma_virt;
            g_local_map_table[i].valid = HI_TRUE;
            hi_log_debug("map local map %u, dam 0x%x, via 0x%pK\n",
                         i, addr_u64(mem->dma_addr), mem->dma_virt);
            break;
        }
    }

    return HI_SUCCESS;
}

/* brief release and unmap a mmz or smmu memory. */
static hi_s32 hash_mem_release_unmap(crypto_mem *mem)
{
    hi_u32 i;

    kfree(mem->dma_virt);
    mem->dma_virt = HI_NULL;

    /* remove the map info. */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid &&
            addr_u64(g_local_map_table[i].dma) == addr_u64(mem->dma_addr)) {
            addr_u64(g_local_map_table[i].dma) = 0x00;
            g_local_map_table[i].via = HI_NULL;
            g_local_map_table[i].valid = HI_FALSE;
            hi_log_debug("unmap local map %u, dam 0x%x, via 0x%pK\n",
                         i, addr_u64(mem->dma_addr), mem->dma_virt);
            break;
        }
    }
    (hi_void)memset_s(mem, sizeof(crypto_mem), 0, sizeof(crypto_mem));

    return HI_SUCCESS;
}

static hi_s32 crypto_mem_alloc_remap(crypto_mem *mem, hi_u32 type, const hi_char *name, hi_u32 size)
{
    return cipher_dma_alloc_coherent(mem, type, name, size);
}

/* brief release and unmap a mmz or smmu memory. */
static hi_s32 crypto_mem_release_unmap(crypto_mem *mem)
{
    return cipher_dma_free_coherent(mem);
}

/* brief map a mmz or smmu memory. */
static hi_s32 crypto_mem_map(crypto_mem *mem)
{
    hi_u32 i;

    hi_log_debug("crypto_mem_map()- dma 0x%x, size 0x%x\n",
                 addr_u64(mem->dma_addr), mem->dma_size);

    /* try to query the table to get cpu address firstly,
     * if can't get cpu address from the table, then call system api to map it.
     */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid && addr_u64(g_local_map_table[i].dma) == addr_u64(mem->dma_addr)) {
            mem->dma_virt = g_local_map_table[i].via;
            hi_log_debug("local map %d, dam 0x%x, via 0x%pK\n", i, addr_u64(mem->dma_addr), mem->dma_virt);
            return HI_SUCCESS;
        }
    }

    mem->dma_virt = (hi_u8 *)phys_to_virt((phys_addr_t)addr_u64(mem->dma_addr));
    if (mem->dma_virt == HI_NULL) {
        return HI_ERR_CIPHER_FAILED_MEM;
    }

    hi_log_info("crypto_mem_map()- via 0x%pK\n", mem->dma_virt);

    return HI_SUCCESS;
}

/* brief unmap a mmz or smmu memory. */
static hi_s32 crypto_mem_unmap(crypto_mem *mem)
{
    hi_u32 i;

    hi_log_debug("crypto_mem_unmap()- dma 0x%x, size 0x%x\n",
                 addr_u64(mem->dma_addr), mem->dma_size);

    /* try to query the table to ummap cpu address firstly,
     * if can't get cpu address from the table, then call system api to unmap it.
     */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid && (addr_u64(g_local_map_table[i].dma) == addr_u64(mem->dma_addr))) {
            /* this api can't unmap the dma within the map table. */
            hi_log_debug("local unmap %d, dam 0x%x, via 0x%pK\n", i, addr_u64(mem->dma_addr), mem->dma_virt);
            return HI_SUCCESS;
        }
    }

    return HI_SUCCESS;
}

hi_void crypto_cpuc_flush_dcache_area(hi_void *kvir, hi_u32 length)
{
    CRYPTO_FLUSH_DCACHE_AREA(kvir, length);
}

hi_void crypto_mem_init(hi_void)
{
    (hi_void)memset_s(&g_local_map_table, sizeof(g_local_map_table), 0, sizeof(g_local_map_table));
}

hi_void crypto_mem_deinit(hi_void)
{
}

hi_s32 crypto_mem_create(crypto_mem *mem, hi_u32 type, const char *name, hi_u32 size)
{
    hi_log_chk_param_return(mem == HI_NULL);

    return crypto_mem_alloc_remap(mem, type, name, size);
}

hi_s32 crypto_mem_destroy(crypto_mem *mem)
{
    hi_log_chk_param_return(mem == HI_NULL);

    return crypto_mem_release_unmap(mem);
}

hi_s32 hash_mem_create(crypto_mem *mem, hi_u32 type, const char *name, hi_u32 size)
{
    hi_log_chk_param_return(mem == HI_NULL);

    return hash_mem_alloc_remap(mem, type, name, size);
}

hi_s32 hash_mem_destroy(crypto_mem *mem)
{
    hi_log_chk_param_return(mem == HI_NULL);

    return hash_mem_release_unmap(mem);
}

hi_s32 crypto_mem_open(crypto_mem *mem, compat_addr dma_addr, hi_u32 dma_size)
{
    hi_log_chk_param_return(mem == HI_NULL);

    mem->dma_addr = dma_addr;
    mem->dma_size = dma_size;

    return crypto_mem_map(mem);
}

hi_s32 crypto_mem_close(crypto_mem *mem)
{
    hi_log_chk_param_return(mem == HI_NULL);

    return crypto_mem_unmap(mem);
}

hi_s32 crypto_mem_attach(crypto_mem *mem, hi_void *buffer)
{
    hi_log_chk_param_return(mem == HI_NULL);

    mem->user_buf = buffer;

    return HI_SUCCESS;
}

hi_s32 crypto_mem_phys(crypto_mem *mem, compat_addr *dma_addr)
{
    hi_log_chk_param_return(mem == HI_NULL);

    dma_addr->phy = addr_u64(mem->dma_addr);

    return HI_SUCCESS;
}

hi_void *crypto_mem_virt(const crypto_mem *mem)
{
    if (mem == HI_NULL) {
        return HI_NULL;
    }

    return mem->dma_virt;
}

hi_s32 crypto_copy_from_user(hi_void *to, unsigned long to_len,
    const hi_void *from, unsigned long from_len)
{
    if (from_len == 0) {
        return HI_SUCCESS;
    }

    hi_log_chk_param_return(to == HI_NULL);
    hi_log_chk_param_return(from == HI_NULL);
    hi_log_chk_param_return(from_len > MAX_COPY_FROM_USER_SIZE);
    hi_log_chk_param_return(from_len > to_len);

    return (hi_s32)copy_from_user(to, from, from_len);
}

hi_s32 crypto_copy_to_user(hi_void *to, unsigned long to_len,
    const hi_void *from, unsigned long from_len)
{
    if (from_len == 0) {
        return HI_SUCCESS;
    }

    hi_log_chk_param_return(to == HI_NULL);
    hi_log_chk_param_return(from == HI_NULL);
    hi_log_chk_param_return(from_len > MAX_COPY_FROM_USER_SIZE);
    hi_log_chk_param_return(from_len > to_len);

    return (hi_s32)copy_to_user(to, from, from_len);
}

hi_u32 crypto_is_sec_cpu(hi_void)
{
    return module_get_secure();
}

hi_void smmu_get_table_addr(hi_phys_addr_t *rdaddr, hi_phys_addr_t *wraddr, hi_u64 *table)
{
#ifdef CRYPTO_SMMU_SUPPORT
    hi_u32 smmu_e_raddr, smmu_e_waddr, mmu_pgtbl;
    HI_DRV_SMMU_GetPageTableAddr(&mmu_pgtbl, &smmu_e_raddr, &smmu_e_waddr);

    *rdaddr = smmu_e_raddr;
    *wraddr = smmu_e_waddr;
    *table = mmu_pgtbl;
#else
    *rdaddr = 0x00;
    *wraddr = 0x00;
    *table  = 0x00;
#endif
}

hi_s32 crypto_waitdone_callback(hi_void *param)
{
    hi_u32 *done = param;

    return (*done != HI_FALSE);
}

hi_s32 cipher_check_mmz_phy_addr(hi_phys_addr_t phy_addr, hi_u32 length)
{
#ifdef CIPHER_CHECK_MMZ_PHY
    hil_mmb_t *mmb = HI_NULL;
    unsigned long mmb_offset = 0;

    /* Check whether the start address is within the MMZ range of the current system. */
    mmb = hil_mmb_getby_phys_2(phy_addr, &mmb_offset);
    if (mmb != NULL) {
        /* Check whether the end address is within the MMZ range of the current system. */
        mmb = hil_mmb_getby_phys_2(phy_addr + length - 1, &mmb_offset);
        if (mmb == NULL) {
            hi_log_print_func_err(hil_mmb_getby_phys_2, HI_ERR_CIPHER_INVALID_ADDR);
            return HI_ERR_CIPHER_INVALID_ADDR;
        }
    } else { /* Whether the starting address is within the MMZ range of other systems. */
        if (hil_map_mmz_check_phys(phy_addr, length)) {
            hi_log_print_func_err(hil_map_mmz_check_phys, HI_ERR_CIPHER_INVALID_ADDR);
            return HI_ERR_CIPHER_INVALID_ADDR;
        }
    }
#endif

#ifdef CIPHER_BUILTIN
    /* check physical addr is ram region. */
    if (pfn_valid(phy_addr >> PAGE_SHIFT) || pfn_valid(length + (phy_addr >> PAGE_SHIFT))) {
#if defined(CONFIG_CMA) && defined(CONFIG_ARCH_HISI_BVT)
        if (is_hicma_address(phy_addr, length)) {
            return HI_SUCCESS;
        } else {
            hi_log_print_func_err(is_hicma_address, HI_ERR_CIPHER_INVALID_ADDR);
            return HI_ERR_CIPHER_INVALID_ADDR;
        }
#endif
        hi_log_error("physical addr is ram region.\n");
        return HI_ERR_CIPHER_INVALID_ADDR;
    } else {
        return HI_SUCCESS;
    }
#endif

    return HI_SUCCESS;
}

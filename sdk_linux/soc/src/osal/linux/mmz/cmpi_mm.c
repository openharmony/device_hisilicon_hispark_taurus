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
#include <linux/module.h>

#include "hi_osal.h"
#include "osal_mmz.h"

#include "hi_debug.h"
#include "hi_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

hi_u64 cmpi_mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong size)
{
    hil_mmb_t *pmmb = NULL;

    pmmb = hil_mmb_alloc(buf_name, size, 0, 0, mmz_name);
    if (pmmb == NULL) {
        osal_printk("mmz malloc failed!\n");
        return HI_NULL;
    }

    return hil_mmb_phys(pmmb);
}
EXPORT_SYMBOL(cmpi_mmz_malloc);

hi_void cmpi_mmz_free(hi_u64 phy_addr, hi_void *vir_addr)
{
    if (vir_addr != NULL) {
        hil_mmb_t *pmmb = hil_mmb_getby_kvirt(vir_addr);

        if (pmmb != NULL) {
            hil_mmb_unmap(pmmb);
        }
    }

    if (phy_addr != 0) {
        hil_mmb_freeby_phys(phy_addr);
    }
}
EXPORT_SYMBOL(cmpi_mmz_free);

hi_void *cmpi_remap_cached(hi_u64 phy_addr, hi_ulong size)
{
    hi_u32 diff;
    hi_u64 page_phy;
    hi_u8 *page_addr;
    hi_ulong page_size;

    /* the mmap address should align with page */
    page_phy = phy_addr & 0xfffffffffffff000ULL;
    diff = phy_addr - page_phy;

    /* the mmap size should be multiples of 1024 */
    page_size = ((size + diff - 1) & 0xfffff000UL) + 0x1000;

    page_addr = hil_mmf_map2kern_cache(page_phy, page_size);
    if (page_addr == NULL) {
        osal_printk("phy(0x%llx) size(0x%lx) map cache err!\n", page_phy, page_size);
        return NULL;
    }

    return (page_addr + diff);
}
EXPORT_SYMBOL(cmpi_remap_cached);

hi_void *cmpi_remap_nocache(hi_u64 phy_addr, hi_ulong size)
{
    hi_u32 diff;
    hi_u64 page_phy;
    hi_u8 *page_addr;
    hi_ulong page_size;

    /* the mmap address should align with page */
    page_phy = phy_addr & 0xfffffffffffff000ULL;
    diff = phy_addr - page_phy;

    /* the mmap size should be multiples of 1024 */
    page_size = ((size + diff - 1) & 0xfffff000UL) + 0x1000;

    page_addr = hil_mmf_map2kern_nocache(page_phy, page_size);
    if (page_addr == NULL) {
        osal_printk("phy(0x%llx) size(0x%lx) map nocache err!\n", page_phy, page_size);
        return NULL;
    }

    return (page_addr + diff);
}
EXPORT_SYMBOL(cmpi_remap_nocache);

hi_void cmpi_unmap(hi_void *virt_addr)
{
    hil_mmf_unmap(virt_addr);
}
EXPORT_SYMBOL(cmpi_unmap);

hi_s32 cmpi_mmz_malloc_nocache(const hi_char *cp_mmz_name, const hi_char *buf_name,
                               hi_u64 *phy_addr, hi_void **pp_vir_addr, hi_ulong len)
{
    hil_mmb_t *pmmb = NULL;

    pmmb = hil_mmb_alloc(buf_name, len, 0, 0, cp_mmz_name);
    if (pmmb == NULL) {
        osal_printk("mmz malloc failed!\n");
        return HI_FAILURE;
    }

    // the buffer allocated by mmz is 4k align
    *pp_vir_addr = hil_mmb_map2kern(pmmb);
    if (*pp_vir_addr == NULL) {
        hil_mmb_free(pmmb);
        osal_printk("mmz remap no cache failed!\n");
        return HI_FAILURE;
    }
    *phy_addr = hil_mmb_phys(pmmb);

    return HI_SUCCESS;
}
EXPORT_SYMBOL(cmpi_mmz_malloc_nocache);


hi_s32 cmpi_mmz_malloc_cached(const hi_char *cp_mmz_name, const hi_char *buf_name,
                              hi_u64 *phy_addr, hi_void **pp_vir_addr, hi_ulong len)
{
    hil_mmb_t *pmmb = NULL;

    pmmb = hil_mmb_alloc(buf_name, len, 0, 0, cp_mmz_name);
    if (pmmb == NULL) {
        osal_printk("mmz malloc failed!\n");
        return HI_FAILURE;
    }

    // the buffer allocated by mmz is 4k align
    *pp_vir_addr = hil_mmb_map2kern_cached(pmmb);
    if (*pp_vir_addr == NULL) {
        hil_mmb_free(pmmb);
        osal_printk("mmz remap cache failed!\n");
        return HI_FAILURE;
    }
    *phy_addr = hil_mmb_phys(pmmb);

    return HI_SUCCESS;
}
EXPORT_SYMBOL(cmpi_mmz_malloc_cached);

int cmpi_check_mmz_phy_addr(unsigned long long phy_addr, unsigned int len)
{
    /* if address in mmz of current system */
    if (hil_is_phys_in_mmz(phy_addr, len)) {
        /* if in other system */
        if (hil_map_mmz_check_phys(phy_addr, len)) {
            return -1;
        }
    }

    return 0;
}
EXPORT_SYMBOL(cmpi_check_mmz_phy_addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


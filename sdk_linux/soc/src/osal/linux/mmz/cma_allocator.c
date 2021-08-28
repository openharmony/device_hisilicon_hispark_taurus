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

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/list.h>
#include <asm/cacheflush.h>

#include <asm/memory.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <linux/dma-mapping.h>
#include <asm/memory.h>
#ifndef CONFIG_64BIT
#include <asm/highmem.h>
#include <asm/pgtable.h>
#endif
#include <asm/tlbflush.h>
#include <asm/pgtable.h>
#include <linux/vmalloc.h>
#include "securec.h"
#include "allocator.h"

struct cma_zone {
    struct device pdev;
    char name[NAME_LEN_MAX];
    unsigned long gfp;
    unsigned long phys_start;
    unsigned long nbytes;
    unsigned int alloc_type;
    unsigned long block_align;
};

extern struct osal_list_head g_mmz_list;

extern int anony;

unsigned long max_malloc_size = 0x40000000UL;


#ifndef CONFIG_64BIT
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
static int __dma_update_pte(pte_t* pte, unsigned long addr, void* data)
#else
static int __dma_update_pte(pte_t* pte, pgtable_t token,
                            unsigned long addr, void* data)
#endif
{
    struct page* page = virt_to_page(addr);
    pgprot_t prot = *(pgprot_t*)data;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
    osal_unused(token);
#endif
    set_pte_ext(pte, mk_pte(page, prot), 0);

    return 0;
}

void __dma_clear_buffer(struct page* page, size_t size)
{
    osal_unused(page);
    osal_unused(size);
    return;
}

extern void hisi_flush_tlb_kernel_range(unsigned long start, unsigned long end);
static void __dma_remap(struct page* page, size_t size, pgprot_t prot)
{
    unsigned long start = (unsigned long) page_address(page);
    unsigned end = start + size;

    apply_to_page_range(&init_mm, start, size, __dma_update_pte, &prot);
    dsb();
    hisi_flush_tlb_kernel_range(start, end);
}
#else /* CONFIG_64BIT */


#endif /* CONFIG_64BIT */

static int do_mmb_alloc(hil_mmb_t* mmb)
{
    hil_mmb_t* p = NULL;
    mmz_trace_func();

    /* add mmb sorted */
    osal_list_for_each_entry(p, &mmb->zone->mmb_list, list)
    {
        if (mmb->phys_addr < p->phys_addr)
            break;
        if (mmb->phys_addr == p->phys_addr)
            osal_trace(KERN_ERR "ERROR:cma allocator bad in %s(%s, %d)",
                    mmb->zone->name,  __func__, __LINE__);
    }
    osal_list_add(&mmb->list, p->list.prev);

    mmz_trace(1, HIL_MMB_FMT_S, hil_mmb_fmt_arg(mmb));

    return 0;
}

static hil_mmb_t *__mmb_alloc(const char *name,
                              unsigned long size,
                              unsigned long align,
                              unsigned long gfp,
                              const char *mmz_name,
                              hil_mmz_t *_user_mmz)
{
    hil_mmz_t *mmz = NULL;
    hil_mmb_t *mmb = NULL;

    unsigned long order;
    size_t count;
    struct page *page = NULL;

    unsigned long fixed_start;

    hil_mmz_t *fixed_mmz = NULL;
    errno_t err_value;

    mmz_trace_func();

    /*
     * no more than 1GB
     */
    if (size == 0 || size > max_malloc_size) {
        return NULL;
    }

    if (align == 0) {
        align = MMZ_GRAIN;
    }

    size = mmz_grain_align(size);
    order = get_order(size);
    count = size >> PAGE_SHIFT;

    mmz_trace(1, "anonymous=%s,size=%luKB,align=%lu", mmz_name, size / SZ_1K, align);

    begin_list_for_each_mmz(mmz, gfp, mmz_name)

    if ((_user_mmz != NULL) && (_user_mmz != mmz)) {
        continue;
    }

    page = dma_alloc_from_contiguous(mmz->cma_dev, count, order, 0);
    if (page == NULL) {
        break;
    }
    fixed_mmz = mmz;
    fixed_start = page_to_phys(page);
    break;

    end_list_for_each_mmz()

    if (fixed_mmz == NULL)
        return NULL;

    if (page != NULL) {
#ifndef CONFIG_64BIT
        __dma_clear_buffer(page, size);
#else
        (void)memset_s(page_address(page), size, 0, size);
        __flush_dcache_area(page_address(page), size);
#endif
    }

    mmb = kmalloc(sizeof(hil_mmb_t), GFP_KERNEL);
    if (mmb == NULL) {
        goto cma_free;
    }
    (void)memset_s(mmb, sizeof(hil_mmb_t), 0, sizeof(hil_mmb_t));

    mmb->zone = fixed_mmz;
    mmb->phys_addr = fixed_start;
    mmb->length = size;

    if (name != NULL) {
        err_value = strncpy_s(mmb->name, HIL_MMB_NAME_LEN, name, HIL_MMB_NAME_LEN - 1);
    } else {
        err_value = strncpy_s(mmb->name, HIL_MMB_NAME_LEN, "<null>", HIL_MMB_NAME_LEN - 1);
    }

    if ((err_value != EOK) || do_mmb_alloc(mmb)) {
        goto mmb_free;
    }

    return mmb;
mmb_free:
    kfree(mmb);
    mmb = NULL;

cma_free:
    dma_release_from_contiguous(mmz->cma_dev, page, count);
    return NULL;
}

static hil_mmb_t *__mmb_alloc_v2(const char *name,
                                 unsigned long size,
                                 unsigned long align,
                                 unsigned long gfp,
                                 const char *mmz_name,
                                 hil_mmz_t *_user_mmz,
                                 unsigned int order)
{
    hil_mmz_t *mmz = NULL;
    hil_mmb_t *mmb = NULL;
    unsigned int i;
    unsigned long cma_order;
    size_t count = size >> PAGE_SHIFT;
    struct page *page = NULL;
    unsigned long fixed_start = 0;
    hil_mmz_t *fixed_mmz = NULL;
    errno_t err_value;

    mmz_trace_func();

    if ((size == 0) || (size > max_malloc_size))
        return NULL;

    if (align == 0) {
        align = 1;
    }

    size = mmz_grain_align(size);
    mmz_trace(1, "size=%luKB, align=%lu", size / SZ_1K, align);
    begin_list_for_each_mmz(mmz, gfp, mmz_name)
        if ((_user_mmz != NULL) && (_user_mmz != mmz))
            continue;

        if (mmz->alloc_type == SLAB_ALLOC) {
            if ((size - 1) & size) {
                for (i = 1; i < 32; i++) { /* 32: the max size is 2^(32-1) */
                    if (!((size >> i) & ~0)) {
                        size = 1 << i;
                        break;
                    }
                }
            }
        } else if (mmz->alloc_type == EQ_BLOCK_ALLOC) {
            size = mmz_align2(size, mmz->block_align);
        }

        cma_order = get_order(size);

        page = dma_alloc_from_contiguous(mmz->cma_dev, count, cma_order, 0);
        if (page == NULL) {
            return NULL;
        }
        fixed_mmz = mmz;
        fixed_start = page_to_phys(page);
        break;
    end_list_for_each_mmz()

    if (fixed_mmz == NULL) {
        return NULL;
    }

    mmb = kmalloc(sizeof(hil_mmb_t), GFP_KERNEL);
    if (mmb == NULL) {
        goto cma_free;
    }
    (void)memset_s(mmb, sizeof(hil_mmb_t), 0, sizeof(hil_mmb_t));

    mmb->zone = fixed_mmz;
    mmb->phys_addr = fixed_start;
    mmb->length = size;
    mmb->order = order;

    if (name != NULL) {
        err_value = strncpy_s(mmb->name, HIL_MMB_NAME_LEN, name, HIL_MMB_NAME_LEN - 1);
    } else {
        err_value = strncpy_s(mmb->name, HIL_MMB_NAME_LEN, "<null>", HIL_MMB_NAME_LEN - 1);
    }
    if ((err_value != EOK) || do_mmb_alloc(mmb)) {
        goto mmb_free;
    }
    return mmb;

mmb_free:
    kfree(mmb);
    mmb = NULL;
cma_free:
    dma_release_from_contiguous(mmz->cma_dev, page, count);

    return NULL;
}

static void __mmb_free(hil_mmb_t *mmb)
{
    size_t count = mmb->length >> PAGE_SHIFT;
    struct page *page = phys_to_page(mmb->phys_addr);

    hil_mmz_t *mmz = mmb->zone;

    if (mmb->flags & HIL_MMB_MAP2KERN_CACHED) {
#ifdef CONFIG_64BIT
        __flush_dcache_area((void *)mmb->kvirt, (size_t)mmb->length);
#else
        __cpuc_flush_dcache_area((void*)mmb->kvirt, (size_t)mmb->length);
        outer_flush_range(mmb->phys_addr, mmb->phys_addr + mmb->length);
#endif
    }

    dma_release_from_contiguous(mmz->cma_dev, page, count);

    osal_list_del(&mmb->list);
    kfree(mmb);
}

static int __mmb_unmap(hil_mmb_t* mmb);
#define MAX_KMALLOC_PAGENR  4
static void *__mmb_map2kern(hil_mmb_t *mmb, int cached)
{
    pgprot_t prot;
    struct page* page = phys_to_page(mmb->phys_addr);

    if (mmb->flags & HIL_MMB_MAP2KERN) {
        if ((!!cached * HIL_MMB_MAP2KERN_CACHED) != (mmb->flags & HIL_MMB_MAP2KERN_CACHED)) {
            osal_trace(KERN_ERR "mmb<%s> has been kernel-mapped %s, can not be re-mapped as %s.",
                mmb->name, (mmb->flags&HIL_MMB_MAP2KERN_CACHED) ? "cached" : "non-cached",
                (cached) ? "cached" : "non-cached");
            return NULL;
        }
        mmb->map_ref++;
        return mmb->kvirt;
    }

#ifdef CONFIG_64BIT

    if (cached) {
        mmb->flags |= HIL_MMB_MAP2KERN_CACHED;
        prot = PAGE_KERNEL;
    } else {
        mmb->flags &= ~HIL_MMB_MAP2KERN_CACHED;
        prot = __pgprot(PROT_NORMAL_NC);
    }

#else

    if (cached) {
        mmb->flags |= HIL_MMB_MAP2KERN_CACHED;
        prot = pgprot_kernel;
    } else {
        mmb->flags &= ~HIL_MMB_MAP2KERN_CACHED;
        prot = pgprot_noncached(pgprot_kernel);
    }

    __dma_remap(page, mmb->length, prot);
#endif
    /*
     * Map into vmalloc space
     */
    {
        unsigned int i;
        struct page **pages = NULL;
        unsigned int pagesnr = mmb->length / PAGE_SIZE;
        struct page *tmp = page;
        int array_size = sizeof(struct page *) * pagesnr;
        struct vm_struct *area = NULL;

        /*
         * Noted: mmb->length would be very large in some cases(for example:
         * more than one Giga Bytes). and array_size would be very large as
         * well. So, don't use kmalloc here.
         */
        pages = vmalloc(array_size);
        if (pages == NULL) {
            osal_trace(KERN_ERR "ptr array(0x%x) vmalloc failed.\n", array_size);
            return NULL;
        }

        for (i = 0; i < pagesnr; i++) {
            *(pages + i) = tmp;
            tmp++;
        }

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,8,0)
        area = __get_vm_area_caller((pagesnr << PAGE_SHIFT), VM_MAP, VMALLOC_START, VMALLOC_END,
									__builtin_return_address(0));
#else
        area = __get_vm_area((pagesnr << PAGE_SHIFT), VM_MAP, VMALLOC_START, VMALLOC_END);
#endif
        if (area == NULL) {
            osal_trace(KERN_ERR "get vm area from high failed!\n");
            vfree(pages);
            return NULL;
        }
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,8,0)
        if (map_kernel_range((unsigned long)area->addr, get_vm_area_size(area), prot, pages)) {
#else
        if (map_vm_area(area, prot, pages)) {
#endif
            osal_trace(KERN_ERR "map vm area to mmz pages failed!\n");
            vunmap(area->addr);
            vfree(pages);
            return NULL;
        }
        mmb->kvirt = area->addr;
        vfree(pages);
    }

    if (mmb->kvirt == NULL) {
        /*
         * you should never get here.
         */
        osal_trace(KERN_ERR "mmb[0x%lx, 0x%lx] map to kernel failed\n",
                mmb->phys_addr, mmb->length);
        return NULL;
    }

    mmb->flags |= HIL_MMB_MAP2KERN;
    mmb->map_ref++;

    return mmb->kvirt;
}

static int __mmb_unmap(hil_mmb_t *mmb)
{
    int ref;

    if (mmb->flags & HIL_MMB_MAP2KERN_CACHED) {
#ifdef CONFIG_64BIT
        __flush_dcache_area((void *)mmb->kvirt, (size_t)mmb->length);
#else
        __cpuc_flush_dcache_area((void*)mmb->kvirt, (size_t)mmb->length);
        outer_flush_range(mmb->phys_addr, mmb->phys_addr + mmb->length);
#endif
    }

    if (mmb->flags & HIL_MMB_MAP2KERN) {
        ref = --mmb->map_ref;
        if (mmb->map_ref != 0)
            return ref;
    }

    /*
     * unmap from vmalloc space.
     */
    {
        vunmap(mmb->kvirt);
    }
    mmb->kvirt = NULL;
    mmb->flags &= ~HIL_MMB_MAP2KERN;
    mmb->flags &= ~HIL_MMB_MAP2KERN_CACHED;

    if ((mmb->flags & HIL_MMB_RELEASED) && (mmb->phy_ref == 0)) {
        __mmb_free(mmb);
    }

    return 0;
}

static void *__mmf_map(phys_addr_t phys, int len, int cache)
{
    struct page **pages = NULL;
    unsigned int pagesnr = len / PAGE_SIZE;
    unsigned int i;
    void *virt = NULL;
    pgprot_t prot;
    struct page* page = phys_to_page(phys);
    struct page *tmp = page;
    int array_size = sizeof(struct page *) * pagesnr;
    struct vm_struct *area = NULL;
#ifdef CONFIG_64BIT
    if (cache) {
        prot = PAGE_KERNEL;
    } else {
        prot = __pgprot(PROT_NORMAL_NC);
    }
#else
    if (cache) {
        prot = pgprot_kernel;
    } else {
        prot = pgprot_noncached(pgprot_kernel);
    }
#endif

    /*
     * Map into vmalloc space.
     *
     *
     * Noted: length of region may be very large in some cases(for example:
     * more than one Giga Bytes). and array_size would be very large as
     * well. So, don't use kmalloc here.
     */
    pages = vmalloc(array_size);
    if (pages == NULL)    {
        osal_trace(KERN_ERR "ptr vmalloc %d failed.\n", array_size);
        return NULL;
    }

    for (i = 0; i < pagesnr; i++) {
        *(pages + i) = tmp;
        tmp++;
    }

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,8,0)
    area = __get_vm_area_caller((pagesnr << PAGE_SHIFT), VM_MAP, VMALLOC_START, VMALLOC_END, __builtin_return_address(0));
#else
    area = __get_vm_area((pagesnr << PAGE_SHIFT), VM_MAP, VMALLOC_START, VMALLOC_END);
#endif
    if (area == NULL) {
        osal_trace(KERN_ERR "get vm area from high failed!\n");
        vfree(pages);
        pages = NULL;
        return NULL;
    }

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,8,0)
    if (map_kernel_range((unsigned long)area->addr, get_vm_area_size(area), prot, pages)) {
#else
    if (map_vm_area(area, prot, pages)) {
#endif
        osal_trace(KERN_ERR "map vm area to mmz pages failed!\n");
        vunmap(area->addr);
        vfree(pages);
        pages = NULL;
        return NULL;
    }
    virt = area->addr;
    vfree(pages);
    pages = NULL;

    return virt;
}

/*
 * this function is added for some extra-requirements from
 * some customers, and the implementation is not strictly
 * in accordance with our early design.
 */
static void __mmf_unmap(void *virt)
{
    unsigned long vaddr = (unsigned long)(uintptr_t)virt;
    if ((vaddr >= VMALLOC_START) && (vaddr < VMALLOC_END))
        vunmap(virt);
}

static int __allocator_init(char *s)
{
#ifdef CONFIG_CMA
    hil_mmz_t *zone = NULL;
    char *line = NULL;
    struct cma_zone *cma_zone = NULL;

    while ((line = strsep(&s, ":")) != NULL) {
        int i;
        char *argv[6];
        extern struct cma_zone *hisi_get_cma_zone(const char *name);
        /*
         * We got 4 args in "line", formatted as
         * "argv[0],argv[1],argv[2],argv[3],argv[4]".
         * eg: "<mmz_name>,<gfp>,<phys_start>,<size>,<alloc_type>"
         * For more convenient, "hard code" are used such as "arg[0]", i.e.
         */
        for (i = 0; (argv[i] = strsep(&line, ",")) != NULL;) {
            if (++i == ARRAY_SIZE(argv)) {
                break;
            }
        }

        cma_zone = hisi_get_cma_zone(argv[0]);
        if (cma_zone == NULL) {
            osal_trace(KERN_ERR"can't get cma zone info:%s\n", argv[0]);
            continue;
        }

        if (i == 4) { /* 4: had parse four args */
            zone = hil_mmz_create("null", 0, 0, 0);
            if (zone == NULL) {
                continue;
            }

            if (strncpy_s(zone->name, HIL_MMZ_NAME_LEN, argv[0], HIL_MMZ_NAME_LEN - 1) != EOK) {
                osal_trace("%s - strncpy_s failed!\n", __FUNCTION__);
                hil_mmz_destroy(zone);
                continue;
            }

            osal_trace("cmz zone gfp 0x%lx, phys 0x%lx, nbytes 0x%lx\n",
                cma_zone->gfp, cma_zone->phys_start, cma_zone->nbytes);
            zone->gfp = cma_zone->gfp;
            zone->phys_start = cma_zone->phys_start;
            zone->nbytes = cma_zone->nbytes;
            zone->cma_dev = &cma_zone->pdev;
            if (zone->nbytes > max_malloc_size) {
                max_malloc_size = zone->nbytes;
            }
        } else if (i == 6) { /* 6: had parse six args */
            zone = hil_mmz_create_v2("null", 0, 0, 0, 0, 0);
            if (zone == NULL) {
                continue;
            }

            if (strncpy_s(zone->name, HIL_MMZ_NAME_LEN, argv[0], HIL_MMZ_NAME_LEN - 1) != EOK) {
                osal_trace("%s - strncpy_s failed!\n", __FUNCTION__);
                hil_mmz_destroy(zone);
                continue;
            }

            zone->gfp = cma_zone->gfp;
            zone->phys_start = cma_zone->phys_start;
            zone->nbytes = cma_zone->nbytes;
            zone->alloc_type = cma_zone->alloc_type;
            zone->block_align = cma_zone->block_align;
            zone->cma_dev = &cma_zone->pdev;
            if (zone->nbytes > max_malloc_size) {
                max_malloc_size = zone->nbytes;
            }
        } else {
            osal_trace(KERN_ERR "Input parameter num incorrect!\n");
            continue;
        }

        if (hil_mmz_register(zone)) {
            osal_trace(KERN_WARNING "Add MMZ failed: " HIL_MMZ_FMT_S "\n",
                    hil_mmz_fmt_arg(zone));
            hil_mmz_destroy(zone);
        }

        zone = NULL;
    }
#endif
    return 0;
}

int cma_allocator_setopt(struct mmz_allocator *allocator)
{
    allocator->init = __allocator_init;
    allocator->mmb_alloc = __mmb_alloc;
    allocator->mmb_alloc_v2 = __mmb_alloc_v2;
    allocator->mmb_map2kern = __mmb_map2kern;
    allocator->mmb_unmap = __mmb_unmap;
    allocator->mmb_free = __mmb_free;
    allocator->mmf_map = __mmf_map;
    allocator->mmf_unmap = __mmf_unmap;
    return 0;
}


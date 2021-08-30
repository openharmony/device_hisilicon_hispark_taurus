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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include "securec.h"
#include "osal_mmz.h"
#include "hi_osal.h"

#define error_mmz(s...)                                        \
    do {                                                   \
        osal_trace(KERN_ERR "mmz_userdev:%s: ", __FUNCTION__); \
        osal_trace(s);                                         \
    } while (0)
#define warning(s...)                                          \
    do {                                                       \
        osal_trace(KERN_WARNING "mmz_userdev:%s: ", __FUNCTION__); \
        osal_trace(s);                                             \
    } while (0)

struct mmz_userdev_info {
    pid_t pid;
    pid_t mmap_pid;
    struct semaphore sem;
    struct osal_list_head list;
};

static int mmz_flush_dcache_mmb_dirty(struct dirty_area *p_area)
{
    if (p_area == NULL) {
        return -EINVAL;
    }

#ifdef CONFIG_64BIT
    __flush_dcache_area(p_area->dirty_virt_start, p_area->dirty_size);
#else
    /* flush l1 cache, use vir addr */
    __cpuc_flush_dcache_area(p_area->dirty_virt_start, p_area->dirty_size);

#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
    /* flush l2 cache, use paddr */
    outer_flush_range(p_area->dirty_phys_start,
                      p_area->dirty_phys_start + p_area->dirty_size);
#endif
#endif
    return 0;
}

static int mmz_flush_dcache_mmb(struct mmb_info *pmi)
{
    hil_mmb_t *mmb = NULL;

    if (pmi == NULL) {
        return -EINVAL;
    }

    mmb = pmi->mmb;
    if ((mmb == NULL) || (pmi->map_cached == 0)) {
        osal_trace("%s->%d,error!\n", __func__, __LINE__);
        return -EINVAL;
    }

#ifdef CONFIG_64BIT
    __flush_dcache_area(pmi->mapped, (size_t)pmi->size);
#else
    /* flush l1 cache, use vir addr */
    __cpuc_flush_dcache_area(pmi->mapped, (size_t)pmi->size);

#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
    /* flush l2 cache, use paddr */
    outer_flush_range(mmb->phys_addr, mmb->phys_addr + mmb->length);
#endif
#endif
    return 0;
}
#ifdef CONFIG_64BIT

void flush_cache_all (void)
{
}

#endif

/*
 * this function should never be called with local irq disabled,
 * because on_each_cpu marco will raise ipi interrupt.
 */
int mmz_flush_dcache_all(void)
{
#ifdef CONFIG_64BIT
    on_each_cpu((smp_call_func_t)flush_cache_all, NULL, 1);
#else
    on_each_cpu((smp_call_func_t)__cpuc_flush_kern_all, NULL, 1);
    outer_flush_all();
#endif /* CONFIG_64BIT */
    return 0;
}

static int mmz_userdev_open(struct inode *inode, struct file *file)
{
    struct mmz_userdev_info *pmu = NULL;

    osal_unused(inode);

    pmu = kmalloc(sizeof(*pmu), GFP_KERNEL);
    if (pmu == NULL) {
        error_mmz("alloc mmz_userdev_info failed!\n");
        return -ENOMEM;
    }
    (void)memset_s(pmu, sizeof(*pmu), 0, sizeof(*pmu));
    pmu->pid = current->pid;
    pmu->mmap_pid = 0;
    sema_init(&pmu->sem, 1);
    OSAL_INIT_LIST_HEAD(&pmu->list);

    /* This file could be opened just for once */
    file->private_data = (void *)pmu;

    return 0;
}

static int ioctl_mmb_alloc(struct file *file,
                           struct mmb_info *pmi)
{
    struct mmz_userdev_info *pmu = file->private_data;
    struct mmb_info *new_mmbinfo = NULL;
    hil_mmb_t *mmb = NULL;

    mmb = hil_mmb_alloc(pmi->mmb_name, pmi->size,
                        pmi->align, pmi->gfp, pmi->mmz_name);
    if (mmb == NULL) {
#if defined(KERNEL_BIT_64) && defined(USER_BIT_32)
        error_mmz("hil_mmb_alloc(%s, %llu, 0x%llx, %lu, %s) failed!\n",
              pmi->mmb_name, pmi->size, pmi->align,
              pmi->gfp, pmi->mmz_name);
#else
        error_mmz("hil_mmb_alloc(%s, %lu, 0x%lx, %lu, %s) failed!\n",
              pmi->mmb_name, pmi->size, pmi->align,
              pmi->gfp, pmi->mmz_name);
#endif
        return -ENOMEM;
    }

    new_mmbinfo = kmalloc(sizeof(*new_mmbinfo), GFP_KERNEL);
    if (new_mmbinfo == NULL) {
        hil_mmb_free(mmb);
        error_mmz("alloc mmb_info failed!\n");
        return -ENOMEM;
    }

    (void)memcpy_s(new_mmbinfo, sizeof(*new_mmbinfo), pmi, sizeof(*new_mmbinfo));
    new_mmbinfo->phys_addr = hil_mmb_phys(mmb);
    new_mmbinfo->mmb = mmb;
    new_mmbinfo->prot = PROT_READ;
    new_mmbinfo->flags = MAP_SHARED;
    osal_list_add_tail(&new_mmbinfo->list, &pmu->list);

    pmi->phys_addr = new_mmbinfo->phys_addr;

    hil_mmb_get(mmb);

    return 0;
}

static int ioctl_mmb_alloc_v2(struct file *file, struct mmb_info *pmi)
{
    struct mmz_userdev_info *pmu = file->private_data;
    struct mmb_info *new_mmbinfo = NULL;
    hil_mmb_t *mmb = NULL;

    mmb = hil_mmb_alloc_v2(pmi->mmb_name, pmi->size, pmi->align,
                           pmi->gfp, pmi->mmz_name, pmi->order);
    if (mmb == NULL) {
#if defined(KERNEL_BIT_64) && defined(USER_BIT_32)
        error_mmz("hil_mmb_alloc(%s, %llu, 0x%llx, %lu, %s) failed!\n",
              pmi->mmb_name, pmi->size, pmi->align,
              pmi->gfp, pmi->mmz_name);
#else
        error_mmz("hil_mmb_alloc(%s, %lu, 0x%lx, %lu, %s) failed!\n",
              pmi->mmb_name, pmi->size, pmi->align,
              pmi->gfp, pmi->mmz_name);
#endif
        return -ENOMEM;
    }

    new_mmbinfo = kmalloc(sizeof(*new_mmbinfo), GFP_KERNEL);
    if (new_mmbinfo == NULL) {
        hil_mmb_free(mmb);
        error_mmz("alloc mmb_info failed!\n");
        return -ENOMEM;
    }

    (void)memcpy_s(new_mmbinfo, sizeof(*new_mmbinfo), pmi, sizeof(*new_mmbinfo));
    new_mmbinfo->phys_addr = hil_mmb_phys(mmb);
    new_mmbinfo->mmb = mmb;
    new_mmbinfo->prot = PROT_READ;
    new_mmbinfo->flags = MAP_SHARED;
    osal_list_add_tail(&new_mmbinfo->list, &pmu->list);

    pmi->phys_addr = new_mmbinfo->phys_addr;

    hil_mmb_get(mmb);

    return 0;
}

static struct mmb_info *get_mmbinfo(unsigned long addr,
                                    struct mmz_userdev_info *pmu)
{
    struct mmb_info *p = NULL;

    osal_list_for_each_entry(p, &pmu->list, list) {
        if ((addr >= p->phys_addr) && (addr < (p->phys_addr + p->size))) {
            break;
        }
    }
    if (&p->list == &pmu->list) {
        return NULL;
    }

    return p;
}

static struct mmb_info *get_mmbinfo_safe(unsigned long addr,
                                         struct mmz_userdev_info *pmu)
{
    struct mmb_info *p = NULL;

    p = get_mmbinfo(addr, pmu);
    if (p == NULL) {
        error_mmz("mmb(0x%08lX) not found!\n", addr);
        return NULL;
    }

    return p;
}

static int ioctl_mmb_user_unmap(struct file *file, struct mmb_info *pmi);

static int _usrdev_mmb_free(struct mmb_info *p)
{
    int ret;

    osal_list_del(&p->list);
    hil_mmb_put(p->mmb);
    ret = hil_mmb_free(p->mmb);
    kfree(p);

    return ret;
}

static int ioctl_mmb_free(struct file *file, struct mmb_info *pmi)
{
    int ret;
    struct mmz_userdev_info *pmu = file->private_data;
    struct mmb_info *p = get_mmbinfo_safe(pmi->phys_addr, pmu);

    if (p == NULL) {
        return -EPERM;
    }

    if (p->delayed_free) {
        warning("mmb<%s> is delayed_free, can not free again!\n", p->mmb->name);
        return -EBUSY;
    }

    if ((p->map_ref > 0) || (p->mmb_ref > 0)) {
        warning("mmb<%s> is still in use!\n", p->mmb->name);
        p->delayed_free = 1;
        return -EBUSY;
    }

    ret = _usrdev_mmb_free(p);

    return ret;
}

static int ioctl_mmb_attr(struct file *file, struct mmb_info *pmi)
{
    struct mmz_userdev_info *pmu = file->private_data;
    struct mmb_info *p = NULL;

    if ((p = get_mmbinfo_safe(pmi->phys_addr, pmu)) == NULL) {
        return -EPERM;
    }

    (void)memcpy_s(pmi, sizeof(*pmi), p, sizeof(*pmi));
    return 0;
}

static int ioctl_mmb_user_remap(struct file *file,
                                struct mmb_info *pmi,
                                int cached)
{
    struct mmz_userdev_info *pmu = file->private_data;
    struct mmb_info *p = NULL;
    unsigned long addr, len, prot, flags, pgoff;

    if ((p = get_mmbinfo_safe(pmi->phys_addr, pmu)) == NULL) {
        return -EPERM;
    }
    /*
     * mmb could be remapped for more than once, but should not
     * be remapped with confusing cache type.
     */
    if (p->mapped && (p->map_ref > 0)) {
        if (cached != p->map_cached) {
            error_mmz("mmb<%s> already mapped as %s, cannot remap as %s.\n",
                  p->mmb->name,
                  p->map_cached ? "cached" : "non-cached",
                  cached ? "cached" : "non-cached");
            return -EINVAL;
        }

        p->map_ref++;
        p->mmb_ref++;

        hil_mmb_get(p->mmb);

        /*
         * pmi->phys may not always start at p->phys,
         * and may start with offset from p->phys.
         * so, we need to calculate with the offset.
         */
        pmi->mapped = p->mapped + (pmi->phys_addr - p->phys_addr);

        return 0;
    }

    if (p->phys_addr & ~PAGE_MASK) {
        return -EINVAL;
    }

    addr = 0;
    len = PAGE_ALIGN(p->size);

    prot = pmi->prot;
    flags = pmi->flags;
    if (prot == 0) {
        prot = p->prot;
    }
    if (flags == 0) {
        flags = p->flags;
    }

    pmu->mmap_pid = current->pid;
    p->map_cached = cached;

    pgoff = p->phys_addr;
    addr = vm_mmap(file, addr, len, prot, flags, pgoff);

    pmu->mmap_pid = 0;

    if (IS_ERR_VALUE((uintptr_t)addr)) {
        error_mmz("vm_mmap(file, 0, %lu, 0x%08lX, 0x%08lX, 0x%08lX) return 0x%08lX\n",
              len, prot, flags, pgoff, addr);
        return addr;
    }

    p->mapped = (void *)(uintptr_t)addr;
    p->prot = prot;
    p->flags = flags;

    p->map_ref++;
    p->mmb_ref++;
    hil_mmb_get(p->mmb);

    /*
     * pmi->phys may not always start at p->phys,
     * and may start with offset from p->phys.
     * so, we need to calculate with the offset.
     */
    pmi->mapped = p->mapped + (pmi->phys_addr - p->phys_addr);

    return 0;
}

static int ioctl_mmb_user_unmap(struct file *file, struct mmb_info *pmi)
{
    int ret;
    unsigned long addr, len;
    struct mmb_info *p = NULL;
    struct mmz_userdev_info *pmu = file->private_data;

    p = get_mmbinfo_safe(pmi->phys_addr, pmu);
    if (p == NULL) {
        return -EPERM;
    }

    if (p->mapped == NULL) {
#if defined(KERNEL_BIT_64) && defined(USER_BIT_32)
        warning("mmb(0x%llx) isn't user-mapped!\n", p->phys_addr);
#else
        warning("mmb(0x%lx) isn't user-mapped!\n", p->phys_addr);
#endif
        pmi->mapped = NULL;
        return -EIO;
    }

    if (!((p->map_ref > 0) && (p->mmb_ref > 0))) {
        error_mmz("mmb<%s> has invalid refer: map_ref=%d, mmb_ref=%d.\n",
              p->mmb->name, p->map_ref, p->mmb_ref);
        return -EIO;
    }

    p->map_ref--;
    p->mmb_ref--;
    hil_mmb_put(p->mmb);

    if (p->map_ref > 0) {
        return 0;
    }

    addr = (unsigned long)(uintptr_t)p->mapped;
    len = PAGE_ALIGN(p->size);

    /* before unmap, refresh cache manually */
    if (p->map_cached) {
        struct mm_struct *mm = current->mm;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
        down_read(&mm->mmap_lock);
        if (hil_vma_check(addr, addr + len)) {
            error_mmz("mmb<%s> vma is invalid.\n", p->mmb->name);
            up_read(&mm->mmap_lock);
#else
        down_read(&mm->mmap_sem);
        if (hil_vma_check(addr, addr + len)) {
            error_mmz("mmb<%s> vma is invalid.\n", p->mmb->name);
            up_read(&mm->mmap_sem);
#endif
            return -EPERM;
        }
#ifdef CONFIG_64BIT
        __flush_dcache_area ((void *)(uintptr_t)addr, (size_t)len);
#else
        __cpuc_flush_dcache_area ((void *)(uintptr_t)addr, (size_t)len);
#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
        outer_flush_range(p->phys_addr, p->phys_addr + len);
#endif
#endif /* CONFIG_64BIT */
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
        up_read(&mm->mmap_lock);
#else
        up_read(&mm->mmap_sem);
#endif
    }

    ret = vm_munmap(addr, len);
    if (!IS_ERR_VALUE((uintptr_t)ret)) {
        p->mapped = NULL;
        pmi->mapped = NULL;
    }

    if (p->delayed_free && (p->map_ref == 0) && (p->mmb_ref == 0)) {
        _usrdev_mmb_free(p);
    }

    return ret;
}

static int ioctl_mmb_virt2phys(struct file *file, struct mmb_info *pmi)
{
    int ret = 0;
    unsigned long virt, phys;
    unsigned long offset = 0;

    osal_unused(file);
    virt = (unsigned long)(uintptr_t)pmi->mapped;
    phys = usr_virt_to_phys(virt);
    if (!phys) {
        ret = -ENOMEM;
    }

    if (hil_mmb_getby_phys_2(phys, &offset) == NULL) {
        error_mmz("Not mmz alloc memory[0x%lx 0x%lx]! 0x%lx\n", virt, phys, offset);
        return -EINVAL;
    }

    pmi->phys_addr = phys;

    return ret;
}

int ioctl_mmb_sys_flush_cache(__phys_addr_type__ phy_addr, void *vir_addr, unsigned int size)
{
    unsigned long end_vir_addr;
    unsigned long end_phy_addr;
    unsigned long trans_phy;

    /* check start address */
    trans_phy = usr_virt_to_phys((unsigned int)(uintptr_t)vir_addr);
    if (trans_phy == 0) {
        error_mmz("start virtual address %p is err.\n", vir_addr);
        return -1;
    }

    if ((trans_phy & 0xFFFFFFFFFFFFFFFEULL) != phy_addr) {
        error_mmz("trans_phy 0x%lx and phy_addr 0x%lx are not equal!\n", trans_phy, phy_addr);
        return -1;
    }

    /* check end address */
    end_vir_addr = (unsigned int)(uintptr_t)vir_addr + size - CACHE_LINE_SIZE;
    trans_phy = usr_virt_to_phys(end_vir_addr);
    if (trans_phy == 0) {
        error_mmz("end virtual address 0x%lx is err.\n", end_vir_addr);
        return -1;
    }

    end_phy_addr = phy_addr + size - CACHE_LINE_SIZE;
    if ((trans_phy & 0xFFFFFFFFFFFFFFFEULL) != end_phy_addr) {
        error_mmz("trans_phy 0x%lx and end_phy_addr 0x%lx are not equal!\n", trans_phy, end_phy_addr);
        return -1;
    }

    return hil_mmb_flush_dcache_byaddr_safe(vir_addr, phy_addr, size);
}

int ioctl_mmb_check_mmz_phy_addr(unsigned long long phy_addr, unsigned int len)
{
    return cmpi_check_mmz_phy_addr(phy_addr, len);
}

int ioctl_mmb_invalid_cache_byaddr(void *kvirt, unsigned long phys_addr, unsigned long len)
{
    return hil_mmb_invalid_cache_byaddr(kvirt, phys_addr, len);
}

int ioctl_mmb_check_phy_in_priv(struct file const *file, struct mmb_info const *pmi)
{
    struct mmb_info *p = NULL;
    struct mmz_userdev_info *pmu = file->private_data;

    p = get_mmbinfo_safe(pmi->phys_addr, pmu);
    if (p == NULL) {
        return -1;
    }

    return 0;
}

int ioctl_mmb_user_mmf_map(struct file const *file, struct mmb_info *pmi)
{
    osal_unused(file);
    pmi->mapped = cmpi_remap_nocache(pmi->phys_addr, pmi->size);

    if (pmi->mapped == NULL) {
        return -1;
    }

    return 0;
}

int ioctl_mmb_user_mmf_map_cache(struct file const *file, struct mmb_info *pmi)
{
    osal_unused(file);
    pmi->mapped = cmpi_remap_cached(pmi->phys_addr, pmi->size);

    if (pmi->mapped == NULL) {
        return -1;
    }

    return 0;
}

int ioctl_mmb_user_mmf_unmap(void *virt_addr)
{
    hil_mmf_unmap(virt_addr);
    return 0;
}

static int mmz_userdev_ioctl_m(struct file *file, unsigned int cmd, struct mmb_info *pmi)
{
    int ret;

    switch (_IOC_NR(cmd)) {
        case _IOC_NR(IOC_MMB_ALLOC):
            ret = ioctl_mmb_alloc(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_ALLOC_V2):
            ret = ioctl_mmb_alloc_v2(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_ATTR):
            ret = ioctl_mmb_attr(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_FREE):
            ret = ioctl_mmb_free(file, pmi);
            break;

        case _IOC_NR(IOC_MMB_USER_REMAP):
            ret = ioctl_mmb_user_remap(file, pmi, 0);
            break;
        case _IOC_NR(IOC_MMB_USER_REMAP_CACHED):
            ret = ioctl_mmb_user_remap(file, pmi, 1);
            break;
        case _IOC_NR(IOC_MMB_USER_UNMAP):
            ret = ioctl_mmb_user_unmap(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_VIRT_GET_PHYS):
            ret = ioctl_mmb_virt2phys(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_SYS_FLUSH_CACHE):
            ret = ioctl_mmb_sys_flush_cache(pmi->phys_addr, pmi->mapped, pmi->size);
            break;
        case _IOC_NR(IOC_MMB_BASE_CHECK_ADDR):
            ret = ioctl_mmb_check_mmz_phy_addr(pmi->phys_addr, pmi->size);
            break;
        case _IOC_NR(IOC_MMB_INVALID_CACHE):
            ret = ioctl_mmb_invalid_cache_byaddr(pmi->mapped, pmi->phys_addr, pmi->size);
            break;
        case _IOC_NR(IOC_MMB_CHECK_PHY_ALLOC):
            ret = ioctl_mmb_check_phy_in_priv(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_MMF_REMAP):
            ret = ioctl_mmb_user_mmf_map(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_MMF_REMAP_CACHED):
            ret = ioctl_mmb_user_mmf_map_cache(file, pmi);
            break;
        case _IOC_NR(IOC_MMB_MMF_UNMAP):
            ret = ioctl_mmb_user_mmf_unmap(pmi->mapped);
            break;

        default:
            error_mmz("invalid ioctl cmd = %08X\n", cmd);
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int mmz_userdev_ioctl_r(struct file *file, unsigned int cmd, struct mmb_info *pmi)
{
    osal_unused(file);
    switch (_IOC_NR(cmd)) {
        case _IOC_NR(IOC_MMB_ADD_REF):
            pmi->mmb_ref++;
            hil_mmb_get(pmi->mmb);
            break;
        case _IOC_NR(IOC_MMB_DEC_REF):
            if (pmi->mmb_ref <= 0) {
                error_mmz("mmb<%s> mmb_ref is %d!\n", pmi->mmb->name, pmi->mmb_ref);
                return -EPERM;
            }
            pmi->mmb_ref--;
            hil_mmb_put(pmi->mmb);
            if (pmi->delayed_free && (pmi->mmb_ref == 0) && (pmi->map_ref == 0)) {
                _usrdev_mmb_free(pmi);
            }
            break;
        default:
            return -EINVAL;
            break;
    }

    return 0;
}

/* just for test */
static int mmz_userdev_ioctl_t(struct file *file, unsigned int cmd, struct mmb_info *pmi);

static long mmz_userdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct mmz_userdev_info *pmu = file->private_data;

    down(&pmu->sem);

    if (_IOC_TYPE(cmd) == 'm') {
        struct mmb_info mi = { 0 };

        if ((_IOC_SIZE(cmd) > sizeof(mi)) || (arg == 0)) {
            error_mmz("_IOC_SIZE(cmd)=%d, arg==0x%08lX\n", _IOC_SIZE(cmd), arg);
            ret = -EINVAL;
            goto __error_exit;
        }
        (void)memset_s(&mi, sizeof(mi), 0, sizeof(mi));
        if (copy_from_user(&mi, (void *)(uintptr_t)arg, _IOC_SIZE(cmd))) {
            osal_trace("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
        }
        mi.mmz_name[HIL_MMZ_NAME_LEN - 1] = '\0';
        mi.mmb_name[HIL_MMB_NAME_LEN - 1] = '\0';
        ret = mmz_userdev_ioctl_m(file, cmd, &mi);
        if (!ret && (cmd & IOC_OUT)) {
            if (copy_to_user((void *)(uintptr_t)arg, &mi, _IOC_SIZE(cmd))) {
                osal_trace("\nmmz_userdev_ioctl: copy_to_user error.\n");
                ret = -EFAULT;
                goto __error_exit;
            }
        }
    } else if (_IOC_TYPE(cmd) == 'r') {
        struct mmb_info *pmi = NULL;

        pmi = get_mmbinfo_safe(arg, pmu);
        if (pmi == NULL) {
            ret = -EPERM;
            goto __error_exit;
        }

        ret = mmz_userdev_ioctl_r(file, cmd, pmi);
    } else if (_IOC_TYPE(cmd) == 'c') {
        struct mmb_info *pmi = NULL;
        if (arg == 0) {
            mmz_flush_dcache_all();
            goto __error_exit;
        }

        pmi = get_mmbinfo_safe(arg, pmu);
        if (pmi == NULL) {
            ret = -EPERM;
            goto __error_exit;
        }

        switch (_IOC_NR(cmd)) {
            case _IOC_NR(IOC_MMB_FLUSH_DCACHE):
                mmz_flush_dcache_mmb(pmi);
                break;
            default:
                ret = -EINVAL;
                break;
        }
    } else if (_IOC_TYPE(cmd) == 'd') {
        hil_mmb_t *mmb = NULL;
        struct mmb_info *pmi = NULL;
        struct dirty_area area;
        __phys_addr_type__ orig_addr;
        unsigned long virt_addr, offset;
        struct mm_struct *mm = current->mm;

        if ((_IOC_SIZE(cmd) != sizeof(area)) || (arg == 0)) {
            error_mmz("_IOC_SIZE(cmd)=%d, arg==0x%08lx\n", _IOC_SIZE(cmd), arg);
            ret = -EINVAL;
            goto __error_exit;
        }
        (void)memset_s(&area, sizeof(area), 0, sizeof(area));
        if (copy_from_user(&area, (void *)(uintptr_t)arg, _IOC_SIZE(cmd))) {
            osal_trace(KERN_WARNING "\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
        }

        mmb = hil_mmb_getby_phys_2(area.dirty_phys_start, &offset);
        if (mmb == NULL) {
#if defined(KERNEL_BIT_64) && defined(USER_BIT_32)
            error_mmz("dirty_phys_addr=0x%llx\n", area.dirty_phys_start);
#else
            error_mmz("dirty_phys_addr=0x%lx\n", area.dirty_phys_start);
#endif
            ret = -EFAULT;
            goto __error_exit;
        }

        pmi = get_mmbinfo_safe(mmb->phys_addr, pmu);
        if (pmi == NULL) {
            ret = -EPERM;
            goto __error_exit;
        }
        if ((uintptr_t)area.dirty_virt_start != (uintptr_t)pmi->mapped + offset) {
            osal_trace(KERN_WARNING "dirty_virt_start addr was not consistent with dirty_phys_start addr!\n");
            ret = -EFAULT;
            goto __error_exit;
        }
        if (area.dirty_phys_start + area.dirty_size > mmb->phys_addr + mmb->length) {
            osal_trace(KERN_WARNING "\ndirty area overflow!\n");
            ret = -EFAULT;
            goto __error_exit;
        }

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
        down_read(&mm->mmap_lock);
#else
        down_read(&mm->mmap_sem);
#endif

        if (hil_vma_check((uintptr_t)area.dirty_virt_start, (uintptr_t)area.dirty_virt_start + area.dirty_size)) {
            osal_trace(KERN_WARNING "\ndirty area[0x%lx,0x%lx] overflow!\n",
                   (unsigned long)(uintptr_t)area.dirty_virt_start,
                   (unsigned long)(uintptr_t)area.dirty_virt_start + area.dirty_size);
            ret = -EFAULT;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
            up_read(&mm->mmap_lock);
#else
            up_read(&mm->mmap_sem);
#endif
            goto __error_exit;
        }

        /* cache line aligned */
        orig_addr = area.dirty_phys_start;
        area.dirty_phys_start &= ~(CACHE_LINE_SIZE - 1);
        virt_addr = (unsigned long)(uintptr_t)area.dirty_virt_start;
        virt_addr &= ~(CACHE_LINE_SIZE - 1);
        area.dirty_virt_start = (void *)(uintptr_t)virt_addr;
        area.dirty_size = (area.dirty_size + (orig_addr - area.dirty_phys_start) +
                          (CACHE_LINE_SIZE - 1)) & ~(CACHE_LINE_SIZE - 1);

        mmz_flush_dcache_mmb_dirty(&area);
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
        up_read(&mm->mmap_lock);
#else
        up_read(&mm->mmap_sem);
#endif
    } else if (_IOC_TYPE(cmd) == 't') {
        struct mmb_info mi;

        if ((_IOC_SIZE(cmd) != sizeof(mi)) || (arg == 0)) {
            error_mmz("_IOC_SIZE(cmd)=%d, arg==0x%08lx\n", _IOC_SIZE(cmd), arg);
            ret = -EINVAL;
            goto __error_exit;
        }

        (void)memset_s(&mi, sizeof(mi), 0, sizeof(mi));
        if (copy_from_user(&mi, (void *)(uintptr_t)arg, sizeof(mi))) {
            osal_trace("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
        }

        if ((get_mmbinfo_safe(mi.phys_addr, pmu)) == NULL) {
            ret = -EPERM;
            goto __error_exit;
        }
        ret = mmz_userdev_ioctl_t(file, cmd, &mi);
    } else {
        ret = -EINVAL;
    }

__error_exit:

    up(&pmu->sem);

    return ret;
}

#ifdef CONFIG_COMPAT
static long compat_mmz_userdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return mmz_userdev_ioctl(file, cmd, (unsigned long)(uintptr_t)compat_ptr(arg));
}
#endif

int mmz_userdev_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct mmb_info *p = NULL;
    struct mmz_userdev_info *pmu = file->private_data;
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    hil_mmb_t *mmb = NULL;
    int mmb_cached = 0;

    p = get_mmbinfo(offset, pmu);
    if (p == NULL) {
        unsigned long mmb_offset;
        mmb = hil_mmb_getby_phys_2(offset, &mmb_offset);
        if (mmb == NULL) {
            /* Allow mmap MMZ allocated by other core. */
            if (hil_map_mmz_check_phys(offset, vma->vm_end - vma->vm_start)) {
                return -EPERM;
            }
        } else {
            mmb_cached = mmb->flags & HIL_MMB_MAP2KERN_CACHED;
        }
    } else {
        if (p->mapped != NULL) {
            if (p->map_cached) {
                error_mmz("mmb(0x%08lX) have been mapped already and cache_type is %u?!\n", offset, p->map_cached);
                return -EIO;
            }
        }
        mmb_cached = p->map_cached;
    }

    if (file->f_flags & O_SYNC) {
#ifdef CONFIG_64BIT
        vma->vm_page_prot = __pgprot(pgprot_val(vma->vm_page_prot)
                                     | PTE_WRITE | PTE_DIRTY);
#endif
        vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    } else {
#ifdef CONFIG_64BIT
        vma->vm_page_prot = __pgprot(pgprot_val(vma->vm_page_prot)
                                     | PTE_WRITE | PTE_DIRTY);
#else
        vma->vm_page_prot = __pgprot(pgprot_val(vma->vm_page_prot)
                                     | L_PTE_PRESENT | L_PTE_YOUNG
                                     | L_PTE_DIRTY | L_PTE_MT_DEV_CACHED);
#endif
        if (mmb_cached == 0) {
            /*
             * pagetable property changes from <normal nocache> to
             * <strong order>, for enhance ddr access performance,
             */
            /*  <normal nocache> */
            vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
        }
    }

    if (pfn_valid(vma->vm_pgoff)) {
        unsigned long start = vma->vm_start;
        unsigned long pfn = vma->vm_pgoff;
        size_t size = vma->vm_end - vma->vm_start;

        while (size) {
            if (pfn_valid(pfn)) {
                if (vm_insert_page(vma, start, pfn_to_page(pfn))) {
                    error_mmz("insert page failed.\n");
                    break;
                }
            } else {
#ifdef CONFIG_64BIT
                error_mmz("vm map failed for phy address(0x%llx)\n", __pfn_to_phys(pfn));
#else
                error_mmz("vm map failed for phy address(0x%x)\n", __pfn_to_phys(pfn));
#endif
            }

            start += PAGE_SIZE;
            size = (size < PAGE_SIZE) ? 0 : (size - PAGE_SIZE);
            pfn++;
        }
    } else {
        size_t size = vma->vm_end - vma->vm_start;
        if (size == 0) {
            return -EPERM;
        }
        /*
         * Remap-pfn-range will mark the range
         * as VM_IO and VM_RESERVED
         */
        if (remap_pfn_range(vma,
                            vma->vm_start,
                            vma->vm_pgoff,
                            vma->vm_end - vma->vm_start,
                            vma->vm_page_prot)) {
            return -EAGAIN;
        }
    }

    return 0;
}

static int mmz_userdev_release(struct inode *inode, struct file *file)
{
    struct mmz_userdev_info *pmu = file->private_data;
    struct mmb_info *p = NULL;
    struct mmb_info *n = NULL;

    osal_unused(inode);
    list_for_each_entry_safe(p, n, &pmu->list, list) {
        error_mmz("MMB LEAK(pid=%d): 0x%lX, %lu bytes, '%s'\n",
              pmu->pid, hil_mmb_phys(p->mmb),
              hil_mmb_length(p->mmb),
              hil_mmb_name(p->mmb));

        /*
         * we do not need to release mapped-area here,
         * system will do it for us
         */
        if (p->mapped != NULL) {
#if defined(KERNEL_BIT_64) && defined(USER_BIT_32)
            warning("mmb<0x%llx> mapped to userspace 0x%pK will be unmapped!\n",
                    p->phys_addr, p->mapped);
#else
            warning("mmb<0x%lx> mapped to userspace 0x%pK will be unmapped!\n",
                    p->phys_addr, p->mapped);
#endif
        }
        for (; p->mmb_ref > 0; p->mmb_ref--) {
            hil_mmb_put(p->mmb);
        }
        _usrdev_mmb_free(p);
    }

    file->private_data = NULL;
    kfree(pmu);
    pmu = NULL;

    return 0;
}

static struct file_operations g_mmz_userdev_fops = {
    .owner = THIS_MODULE,
    .open = mmz_userdev_open,
    .release = mmz_userdev_release,
#ifdef CONFIG_COMPAT
    .compat_ioctl = compat_mmz_userdev_ioctl,
#endif
    .unlocked_ioctl = mmz_userdev_ioctl,
    .mmap = mmz_userdev_mmap,
};

static struct miscdevice g_mmz_userdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &g_mmz_userdev_fops,
    .name = "mmz_userdev"
};

int mmz_userdev_init(void)
{
    int ret;
    ret = misc_register(&g_mmz_userdev);
    if (ret) {
        osal_trace("register mmz dev failure!\n");
        return -1;
    }

    return 0;
}

void mmz_userdev_exit(void)
{
    misc_deregister(&g_mmz_userdev);
}

/* Test func */
static int mmz_userdev_ioctl_t(struct file *file, unsigned int cmd, struct mmb_info *pmi)
{
    osal_unused(file);
    osal_unused(pmi);
    osal_unused(cmd);
    return 0;
}


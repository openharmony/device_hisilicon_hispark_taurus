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

#include "hifb_init.h"
#include <linux/poll.h>
#include <linux/fb.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include "securec.h"
#include <linux/of_platform.h>
#include <linux/memblock.h>
#include <linux/highmem.h>
#include <linux/version.h>

bool g_temp_update_rotate_rect = HI_FALSE;
char g_tmp_video[64] = "hifb:vram0_size:8100"; /* 64 The length of the array */

module_param(g_temp_update_rotate_rect, bool, S_IRUGO);
module_param_string(video, g_tmp_video, 64, 0); /* 64 The length of the array */

char *apsz_layer_mmz_names[HIFB_MAX_LAYER_NUM] = { [0 ... HIFB_MAX_LAYER_NUM - 1] = HI_NULL };

module_param_array(apsz_layer_mmz_names, charp, HI_NULL, S_IRUGO);
MODULE_PARM_DESC(apsz_layer_mmz_names, "The mmz names for the graphics layers.");

char *hifb_get_layer_mmz_names(hi_u32 layer_id)
{
    if (layer_id >= HIFB_MAX_LAYER_NUM) {
        return HI_NULL;
    }
    return apsz_layer_mmz_names[layer_id];
}

#define VSYNC_LEN 64
static unsigned int g_vsync_major = 288;
static unsigned int g_vsync_minor = 0;
static dev_t g_vsync_devno;
static struct class *g_vsync_cls;
static struct device *g_vsync_device;

static unsigned int g_pts_major = 289;
static unsigned int g_pts_minor = 0;
static dev_t g_pts_devno;
static struct class *g_pts_cls;
static struct device *g_pts_device;

static char g_vdata[VSYNC_LEN] = {0};
static char g_nowdata[VSYNC_LEN] = {0};

static spinlock_t g_vsync_lock;
static wait_queue_head_t g_wait;

static unsigned long long int g_timestamp;
static bool g_bnewdata = 0;

static int vsync_open(struct inode *inode, struct file *file)
{
    hi_unused(inode);
    hi_unused(file);
    return 0;
}

static int vsync_release(struct inode *inode, struct file *file)
{
    hi_unused(inode);
    hi_unused(file);
    return 0;
}

static int vsync_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    unsigned int len;
    unsigned long syncflags = 0;
    hi_unused(file);
    hi_unused(offset);
    spin_lock_irqsave(&g_vsync_lock, syncflags);
    (hi_void)memset_s(g_vdata, VSYNC_LEN, 0, VSYNC_LEN);
    if (snprintf_s(g_vdata, VSYNC_LEN, VSYNC_LEN - 1, "%llu\n", g_timestamp) < 0) {
        osal_printk("%s:%d:snprintf_s failure\n", __FUNCTION__, __LINE__);
        spin_unlock_irqrestore(&g_vsync_lock, syncflags);
        return HI_FAILURE;
    }
    spin_unlock_irqrestore(&g_vsync_lock, syncflags);

    len = strnlen(g_vdata, (VSYNC_LEN - 1)) + 1;
    if ((buf == HI_NULL) || (count < len)) {
        return 0;
    }

    if (osal_copy_to_user(buf, g_vdata, len)) {
        osal_printk("copy to user err\n");
        len = 0;
    }

    spin_lock_irqsave(&g_vsync_lock, syncflags);
    g_bnewdata = 0;
    spin_unlock_irqrestore(&g_vsync_lock, syncflags);

    return len;
}

static unsigned int vsync_poll(struct file *file, struct poll_table_struct *table)
{
    unsigned int mask = 0;
    unsigned long syncflags = 0;

    poll_wait(file, &g_wait, table);

    spin_lock_irqsave(&g_vsync_lock, syncflags);
    if (g_bnewdata) {
        mask |= (POLLIN | POLLRDNORM);
    }
    spin_unlock_irqrestore(&g_vsync_lock, syncflags);

    return mask;
}

static struct file_operations g_vsync_ops = {
    .owner = THIS_MODULE,
    .open = vsync_open,
    .read = (hi_void *)vsync_read,
    .release = vsync_release,
    .poll = vsync_poll,
};

inline static unsigned long long int get_cur_pts(void)
{
    unsigned long long int time_now;

    time_now = osal_sched_clock();
    do_div(time_now, 1000); /* 1000 alg data */

    return time_now;
}

int hifb_vsync_notify(void)
{
    unsigned long syncflags = 0;

    spin_lock_irqsave(&g_vsync_lock, syncflags);
    g_timestamp = get_cur_pts();
    g_bnewdata = 1;
    spin_unlock_irqrestore(&g_vsync_lock, syncflags);

    wake_up_interruptible(&g_wait);

    return 0;
}

int hifb_vsync_init(void)
{
    int ret;

    g_vsync_devno = MKDEV(g_vsync_major, g_vsync_minor);
    ret = register_chrdev(g_vsync_major, "vsync", &g_vsync_ops);
    if (ret < 0) {
        osal_printk("Unable to register characterdevice!\n");
        return ret;
    }
    g_vsync_cls = class_create(THIS_MODULE, "vsync");
    if (IS_ERR(g_vsync_cls)) {
        unregister_chrdev(g_vsync_major, "vsync");
        return -EBUSY;
    }

    g_vsync_device = device_create(g_vsync_cls, HI_NULL, g_vsync_devno, HI_NULL, "vsync"); /* mknod /dev/vsync */
    if (IS_ERR(g_vsync_device)) {
        class_destroy(g_vsync_cls);
        unregister_chrdev(g_vsync_major, "vsync");
        return -EBUSY;
    }

    init_waitqueue_head(&g_wait);
    spin_lock_init(&g_vsync_lock);

    return 0;
}

void hifb_vsync_exit(void)
{
    device_destroy(g_vsync_cls, g_vsync_devno);
    class_destroy(g_vsync_cls);
    unregister_chrdev(g_vsync_major, "vsync");
}

static int pts_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    unsigned long long int time_now;
    unsigned int len;
    time_now = get_cur_pts();
    hi_unused(file);
    hi_unused(offset);

    (hi_void)memset_s(g_nowdata, sizeof(g_nowdata), 0, VSYNC_LEN);
    if (snprintf_s(g_nowdata, sizeof(g_nowdata), VSYNC_LEN - 1, "%llu\n", time_now) < 0) {
        osal_printk("%s:%d:snprintf_s failure\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }
    len = strnlen(g_nowdata, (VSYNC_LEN - 1)) + 1;
    if ((buf == HI_NULL) || (count < len)) {
        return 0;
    }

    if (osal_copy_to_user(buf, g_nowdata, len)) {
        osal_printk("copy to user err\n");
        len = 0;
    }

    return len;
}

static struct file_operations g_pts_ops = {
    .owner = THIS_MODULE,
    .read = (hi_void *)pts_read,
};

int hifb_pts_init(void)
{
    int ret;

    g_pts_devno = MKDEV(g_pts_major, g_pts_minor);
    ret = register_chrdev(g_pts_major, "vsync_pts", &g_pts_ops);
    if (ret < 0) {
        osal_printk("Unable to register characterdevice!\n");
        return ret;
    }

    g_pts_cls = class_create(THIS_MODULE, "vsync_pts");
    if (IS_ERR(g_pts_cls)) {
        unregister_chrdev(g_pts_major, "vsync_pts");
        return -EBUSY;
    }

    g_pts_device = device_create(g_pts_cls, HI_NULL, g_pts_devno, HI_NULL, "vsync_pts"); /* mknod /dev/vsync_pts */
    if (IS_ERR(g_pts_device)) {
        class_destroy(g_pts_cls);
        unregister_chrdev(g_pts_major, "vsync_pts");
        return -EBUSY;
    }

    return 0;
}

void hifb_pts_exit(void)
{
    device_destroy(g_pts_cls, g_pts_devno);
    class_destroy(g_pts_cls);
    unregister_chrdev(g_pts_major, "vsync_pts");
}

#ifdef CONFIG_HI_VO_FB_SEPARATE
#define HIFB_INT_NAME_LENGTH 10

static int hi35xx_hifb_probe(struct platform_device *pdev)
{
    hi_char hifb_int_name[HIFB_INT_NAME_LENGTH] = "hifb";
    int temp_hifb_irq;
#ifdef CONFIG_HIFB_SOFT_IRQ_SUPPORT
    unsigned int temp_hifb_soft_irq;
#endif
    set_update_rotate_rect(g_temp_update_rotate_rect);
    osal_platform_get_modparam_string(pdev, "video", 64, g_tmp_video); /* 64 video length */
    set_video_name(g_tmp_video);
    temp_hifb_irq = osal_platform_get_irq_byname(pdev, hifb_int_name);
    if (temp_hifb_irq <= 0) {
        dev_err(&pdev->dev, "cannot find hifb IRQ\n");
        return HI_FAILURE;
    }

#ifdef CONFIG_HIFB_SOFT_IRQ_SUPPORT
    temp_hifb_soft_irq = osal_platform_get_irq_byname(pdev, "hifb_soft");
    if (temp_hifb_soft_irq <= 0) {
        return HI_FAILURE;
    }
    set_hifb_soft_irq(temp_hifb_soft_irq);
#endif
    set_hifb_irq((unsigned int)temp_hifb_irq);
    if (hifb_init() != HI_SUCCESS) {
        osal_printk("hifb_init HI_FAILURE!\n");
    }

    return 0;
}

static int hi35xx_hifb_remove(struct platform_device *pdev)
{
    hi_unused(pdev);
    hifb_cleanup();
    return 0;
}

static const struct of_device_id g_hi35xx_hifb_match[] = {
    {.compatible = "hisilicon,hisi-hifb"},
    {},
};

MODULE_DEVICE_TABLE(of, g_hi35xx_hifb_match);

static struct platform_driver g_hi35xx_hifb_driver = {
    .probe = hi35xx_hifb_probe,
    .remove = hi35xx_hifb_remove,
    .driver =
        {
            .name = "hi35xx_hifb",
            .of_match_table = g_hi35xx_hifb_match,
        },
};

osal_module_platform_driver(g_hi35xx_hifb_driver);

MODULE_LICENSE("GPL");
#endif

/****************************** dmabuf start **************************/
typedef struct {
    phys_addr_t mem_base;
} hifb_mem_block_pdata;

static struct sg_table *hifb_memblock_map(struct dma_buf_attachment *buf_attach,
                                          enum dma_data_direction data_direction)
{
    hi_s32 i = 0;
    hi_s32 ret;
    hi_ulong page_pfn;
    hifb_mem_block_pdata *mem_block_data = HI_NULL;
    struct page *fb_page = HI_NULL;
    struct sg_table *fb_table = HI_NULL;
    struct scatterlist *sg_list = HI_NULL;

    hi_unused(data_direction);

    if ((buf_attach == HI_NULL) || (buf_attach->dmabuf == HI_NULL) || (buf_attach->dmabuf->priv == HI_NULL)) {
        return HI_NULL;
    }

    mem_block_data = buf_attach->dmabuf->priv;
    if (mem_block_data == HI_NULL) {
        return HI_NULL;
    }

    page_pfn = PFN_DOWN(mem_block_data->mem_base);
    fb_page = pfn_to_page(page_pfn);
    if (fb_page == HI_NULL) {
        return HI_NULL;
    }

    fb_table = kzalloc(sizeof(*fb_table), GFP_KERNEL);
    if (fb_table == HI_NULL) {
        return ERR_PTR(-ENOMEM);
    }

    ret = sg_alloc_table(fb_table, 1, GFP_KERNEL);
    if (ret < 0) {
        kfree(fb_table);
        fb_table = HI_NULL;
        return ERR_PTR(ret);
    }

    sg_set_page(fb_table->sgl, fb_page, buf_attach->dmabuf->size, 0);
    for_each_sg(fb_table->sgl, sg_list, fb_table->nents, i) {
        if (sg_list != HI_NULL) {
            sg_dma_address(sg_list) = sg_phys(sg_list);
        }
    }
    return fb_table;
}

static hi_void hifb_memblock_unmap(struct dma_buf_attachment *buf_attach, struct sg_table *fb_table,
                                   enum dma_data_direction data_direction)
{
    hi_unused(buf_attach);
    hi_unused(data_direction);
    if (fb_table != HI_NULL) {
        sg_free_table(fb_table);
    }
}

static hi_void __init_memblock hifb_memblock_release(struct dma_buf *fb_dma_buf)
{
    hi_unused(fb_dma_buf);
}

static hi_void *hifb_memblock_do_kmap(struct dma_buf *fb_dma_buf, hi_ulong pgoffset, bool atomic)
{
    hi_ulong page_pfn;
    hifb_mem_block_pdata *mem_block_data = HI_NULL;
    struct page *fb_page = HI_NULL;

    if ((fb_dma_buf == HI_NULL) || (fb_dma_buf->priv == HI_NULL)) {
        return HI_NULL;
    }

    mem_block_data = fb_dma_buf->priv;
    if (mem_block_data == HI_NULL) {
        return HI_NULL;
    }

    page_pfn = PFN_DOWN(mem_block_data->mem_base) + pgoffset;
    fb_page = pfn_to_page(page_pfn);
    if (fb_page == HI_NULL) {
        return HI_NULL;
    }

    if (atomic) {
        return kmap_atomic(fb_page);
    } else {
        return kmap(fb_page);
    }
}

static hi_void *hifb_memblock_kmap(struct dma_buf *fb_dma_buf, hi_ulong pgoffset)
{
    return hifb_memblock_do_kmap(fb_dma_buf, pgoffset, false);
}

static hi_void hifb_memblock_kunmap(struct dma_buf *fb_dma_buf, hi_ulong pgoffset, hi_void *vir_addr)
{
    hi_unused(fb_dma_buf);
    hi_unused(pgoffset);
    if (vir_addr != HI_NULL) {
        kunmap(vir_addr);
    }
}

static inline int hifb_valid_mmap_phys_addr_range(unsigned long pfn, size_t size)
{
    return (pfn + (size >> PAGE_SHIFT)) <= (1 + (PHYS_MASK >> PAGE_SHIFT));
}

static hi_s32 hifb_memblock_mmap(struct dma_buf *fb_dma_buf, struct vm_area_struct *vma_area)
{
    hi_ulong size;
    hifb_mem_block_pdata *mem_block_data = HI_NULL;

    if ((fb_dma_buf == HI_NULL) || (vma_area == HI_NULL)) {
        return -1;
    }

    mem_block_data = fb_dma_buf->priv;
    if (mem_block_data == HI_NULL) {
        return -1;
    }

    if (mem_block_data->mem_base == 0) {
        return -1;
    }

    size = vma_area->vm_end - vma_area->vm_start;
    if (size == 0) {
        return -1;
    }

    if (!hifb_valid_mmap_phys_addr_range(vma_area->vm_pgoff, size)) {
        return -1;
    }
    vma_area->vm_page_prot = pgprot_writecombine(vma_area->vm_page_prot);

    return remap_pfn_range(vma_area, vma_area->vm_start, PFN_DOWN(mem_block_data->mem_base), size,
                           vma_area->vm_page_prot);
}

struct dma_buf_ops g_hifb_mem_block_ops = {
    .map_dma_buf = hifb_memblock_map,
    .unmap_dma_buf = hifb_memblock_unmap,
    .release = hifb_memblock_release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
    .map = hifb_memblock_kmap,
    .unmap = hifb_memblock_kunmap,
#endif
    .mmap = hifb_memblock_mmap,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
struct dma_buf *hifb_memblock_export(phys_addr_t mem_base, size_t mem_size, hi_s32 flags)
{
    struct hifb_mem_block_pdata *mem_block_data = HI_NULL;
    struct dma_buf *fb_dma_buf = HI_NULL;

    if ((mem_base != PAGE_ALIGN(mem_base)) || (mem_size != PAGE_ALIGN(mem_size))) {
        return ERR_PTR(-EINVAL);
    }
    mem_block_data = kzalloc(sizeof(*mem_block_data), GFP_KERNEL);
    if (mem_block_data == HI_NULL) {
        return ERR_PTR(-ENOMEM);
    }

    mem_block_data->mem_base = mem_base;
    fb_dma_buf = dma_buf_export(mem_block_data, &g_hifb_mem_block_ops, mem_size, flags, HI_NULL);
    if (IS_ERR(fb_dma_buf)) {
        kfree(mem_block_data);
        mem_block_data = HI_NULL;
    }
    return fb_dma_buf;
}
#else
struct dma_buf *hifb_memblock_export(phys_addr_t mem_base, size_t mem_size, hi_s32 flags)
{
    DEFINE_DMA_BUF_EXPORT_INFO(export_info);
    struct dma_buf *fb_dma_buf = HI_NULL;

    if ((mem_base != PAGE_ALIGN(mem_base)) || (mem_size != PAGE_ALIGN(mem_size))) {
        return ERR_PTR(-EINVAL);
    }

    export_info.ops = &g_hifb_mem_block_ops;
    export_info.size = mem_size;
    export_info.flags = flags;
    export_info.resv = HI_NULL;
    export_info.priv = (hi_void *)mem_base;

    fb_dma_buf = dma_buf_export(&export_info);

    return fb_dma_buf;
}
#endif

/****************************** dmabuf end **************************/


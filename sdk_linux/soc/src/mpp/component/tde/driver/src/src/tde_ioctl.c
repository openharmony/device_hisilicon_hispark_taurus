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

#ifndef HI_BUILD_IN_BOOT
#include <linux/version.h>
#ifndef __RTOS__
#include "tde_init.h"
#ifdef CONFIG_64BIT
#include <asm/compat.h>
#endif
#endif
#include "tde_ioctl.h" /* because of redefined warning, tde_ioctl.h not go first. */
#include "drv_tde_type.h"
#include "tde_osictl.h"
#include "tde_osilist.h"
#include "tde_hal.h"
#include "tde_handle.h"
#include "tde_adp.h"
#include "hi_common.h"
#include "hi_tde_ext.h"
#include "drv_tde_ioctl.h"
#include "tde_osictl_k.h"
#include "tde_hal_k.h"
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
#include "tde_proc.h"
#endif
#define TDE_NAME "HI_TDE"

static osal_spinlock_t g_task_let_lock;

static osal_spinlock_t g_tde_ref_lock;

typedef hi_s32 (*tde_ctrl_ptr_func)(unsigned long argp, hi_void *private_data);
typedef struct {
    hi_u32 tde_cmd;
    tde_ctrl_ptr_func tde_ctl_ptr_func;
} tde_ctl_func_dispatch_item;

hi_s32 tde_osr_begin_job(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_blit_blit(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_solid_draw(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_quick_copy(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_quick_resize(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_quick_fill(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_quick_deflicker(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_mb_blit_blit(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_end_job(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_wait_for_done(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_cancel_job(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_bit_map_mask_rop(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_bit_map_mask_blend(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_wait_all_done(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_reset(unsigned long argp, hi_void *private_data);

static hi_s32 tde_osr_set_deflicker_level(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_get_deflicker_level(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_set_alpha_threshold_value(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_get_alpha_threshold_value(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_set_alpha_threshold_state(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_get_alpha_threshold_state(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_pattern_fill(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_enable_region_deflicker(unsigned long argp, hi_void *private_data);
static hi_s32 tde_osr_rotate(unsigned long argp, hi_void *private_data);

tde_ctl_func_dispatch_item g_tde_ctl_func_dispatch_item[] = {
    {0, HI_NULL},
    {TDE_BEGIN_JOB, tde_osr_begin_job},
    {TDE_BIT_BLIT, tde_osr_blit_blit},
    {TDE_SOLID_DRAW, tde_osr_solid_draw},
    {TDE_QUICK_COPY, tde_osr_quick_copy},
    {TDE_QUICK_RESIZE, tde_osr_quick_resize},
    {TDE_QUICK_FILL, tde_osr_quick_fill},
    {TDE_QUICK_DEFLICKER, tde_osr_quick_deflicker},
    {TDE_MB_BITBLT, tde_osr_mb_blit_blit},
    {TDE_END_JOB, tde_osr_end_job},
    {TDE_WAITFORDONE, tde_osr_wait_for_done},
    {TDE_CANCEL_JOB, tde_osr_cancel_job},
    {TDE_BITMAP_MASKROP, tde_osr_bit_map_mask_rop},
    {TDE_BITMAP_MASKBLEND, tde_osr_bit_map_mask_blend},
    {TDE_WAITALLDONE, tde_osr_wait_all_done},
    {TDE_RESET, tde_osr_reset},
    {0, HI_NULL},
    {TDE_SET_DEFLICKERLEVEL, tde_osr_set_deflicker_level},
    {TDE_GET_DEFLICKERLEVEL, tde_osr_get_deflicker_level},
    {TDE_SET_ALPHATHRESHOLD_VALUE, tde_osr_set_alpha_threshold_value},
    {TDE_GET_ALPHATHRESHOLD_VALUE, tde_osr_get_alpha_threshold_value},
    {TDE_SET_ALPHATHRESHOLD_STATE, tde_osr_set_alpha_threshold_state},
    {TDE_GET_ALPHATHRESHOLD_STATE, tde_osr_get_alpha_threshold_state},
    {TDE_PATTERN_FILL, tde_osr_pattern_fill},
    {TDE_ENABLE_REGIONDEFLICKER, tde_osr_enable_region_deflicker},
    {TDE_ROTATE, tde_osr_rotate},
    {0, HI_NULL},
    {0, HI_NULL}
};

#define TDE_IOCTL_FUNC_NUM (sizeof(g_tde_ctl_func_dispatch_item) / sizeof(g_tde_ctl_func_dispatch_item[0]))

static int tde_osr_isr(int irq, hi_void *dev_id);
static void tde_tasklet_func(unsigned long int_status);

/* TDE equipment quoted count */
static osal_atomic_t g_tde_count = OSAL_ATOMIC_INIT(0);
#ifdef TDE_TIME_COUNT
tde_timeval g_time_start;
tde_timeval g_time_end;
hi_u64 g_time_diff;
#endif

struct osal_fileops g_tde_fops = {
    .open = tde_open,
    .release = tde_release,
#ifdef CONFIG_COMPAT
    .compat_ioctl = tde_ioctl, /* tde_compactioctl */
#endif
    .unlocked_ioctl = tde_ioctl,
};

unsigned int g_tde_irq = TDE_INTNUM;
static osal_dev_t *g_tde_device;
#ifndef __RTOS__
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,9,0)
DECLARE_TASKLET_OLD(tde_tasklet, tde_tasklet_func);
#else
DECLARE_TASKLET(tde_tasklet, tde_tasklet_func, 0);
#endif
#endif

#ifdef CONFIG_TDE_TDE_EXPORT_FUNC
static hi_tde_export_func g_tde_export_funce = {
    .drv_tde_module_open = tde_osi_open,
    .drv_tde_module_close = tde_osi_close,
    .drv_tde_module_begin_job = tde_osi_begin_job_ex,
    .drv_tde_module_end_job = tde_osi_end_job,
    .drv_tde_module_cancel_job = tde_osi_cancel_job,
    .drv_tde_module_wait_for_done = tde_osi_wait_for_done,
    .drv_tde_module_wait_all_done = tde_osi_wait_all_done,
    .drv_tde_module_quick_copy = tde_osi_quick_copy,
    .drv_tde_module_quick_fill = tde_osi_quick_fill,
    .drv_tde_module_quick_resize = tde_osi_quick_resize,
#if (TDE_CAPABILITY & DEFLICKER)
    .drv_tde_module_quick_flicker = tde_osi_quick_flicker,
#endif
    .drv_tde_module_blit = tde_osi_blit,
    .drv_tde_module_mb_blit = tde_osi_mb_blit,
    .drv_tde_module_solid_draw = tde_osi_solid_draw,
#if (TDE_CAPABILITY & DEFLICKER)
    .drv_tde_module_set_deflicker_level = tde_osi_set_deflicker_level,
#endif
    .drv_tde_module_enable_region_deflicker = tde_osi_enable_region_deflicker,
    .drv_tde_module_rotate = tde_osi_quick_rotate,
    .drv_tde_module_set_alpha_threshold_value = tde_osi_set_alpha_threshold_value,
    .drv_tde_module_set_alpha_threshold_state = tde_osi_set_alpha_threshold_state,
    .drv_tde_module_get_alpha_threshold_state = tde_osi_get_alpha_threshold_state,
    .drv_tde_module_cal_scale_rect = tde_cal_scale_rect,
#ifdef CONFIG_TDE_PM_ENABLE
    .drv_tde_module_suspend = tde_pm_suspend,
    .drv_tde_module_resume = tde_pm_resume,
#endif

#ifdef CONFIG_TDE_BLIT_EX
    .drv_tde_module_single_blit_ex = tde_osi_single_blit_ex,
#endif

#ifdef TDE_HWC_COOPERATE
    .drv_tde_module_lock_working_flag = tde_osi_list_lock_working_flag,
    .drv_tde_module_unlock_working_flag = tde_osi_list_unlock_working_flag,
    .drv_tde_module_get_working_flag = tde_osi_list_get_working_flag
#endif
};
#endif

hi_void set_tde_irq(unsigned int temp_tde_irq)
{
    g_tde_irq = temp_tde_irq;
}

static hi_s32 gfx_init(hi_void *args)
{
    hi_unused(args);
    return HI_SUCCESS;
}

static hi_void gfx_exit(hi_void)
{
    return;
}

static hi_void gfx_notify(mod_notice_id notice)
{
    hi_unused(notice);
    return;
}

static hi_void gfx_query_state(mod_state *state)
{
    if (state == HI_NULL) {
        return;
    }
    *state = MOD_STATE_FREE;
    return;
}

static hi_u32 gfx_get_ver_magic(hi_void)
{
    return VERSION_MAGIC;
}

static hi_s32 tde_freeze(osal_dev_t *pdev)
{
    hi_unused(pdev);
    return HI_SUCCESS;
}

static hi_s32 tde_restore(osal_dev_t *pdev)
{
    hi_unused(pdev);
    return HI_SUCCESS;
}

struct osal_pmops g_tde_drv_ops = {
    .pm_freeze = tde_freeze,
    .pm_restore = tde_restore,
};

static umap_module g_module = {
    .mod_id = HI_ID_TDE,
    .mod_name = "tde",
    .pfn_init = gfx_init,
    .pfn_exit = gfx_exit,
    .pfn_query_state = gfx_query_state,
    .pfn_notify = gfx_notify,
    .pfn_ver_checker = gfx_get_ver_magic,
    .export_funcs = &g_tde_export_funce,
    .data = HI_NULL,
};

static hi_s32 tde_init_create(hi_void)
{
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_proc_entry_t *proc = HI_NULL;

    proc = osal_create_proc_entry("tde", HI_NULL);
    if (proc == HI_NULL) {
        osal_printk("TDE module create proc failure!\n");
        osal_deregisterdevice(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        osal_destroydev(g_tde_device);
        return HI_FAILURE;
    }
    proc->read = tde_read_proc;
#endif
    if (cmpi_register_module(&g_module)) {
        osal_printk("%s %d cmpi_register_module failed\n", __func__, __LINE__);
        osal_remove_proc_entry("tde", HI_NULL);
        osal_deregisterdevice(g_tde_device);
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        osal_destroydev(g_tde_device);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

hi_s32 tde_init_module_k(hi_void)
{
    hi_s32 ret;

    if (tde_hal_init(TDE_REG_BASEADDR) < 0) {
        tde_error("tde_hal_init failure!\n");
        return HI_FAILURE;
    }

    if (osal_request_irq(g_tde_irq, tde_osr_isr, HI_NULL, TDE_IRQ_NAME, tde_osr_isr) != 0) {
        tde_error("request_irq for TDE failure!\n");
        tde_hal_release();
        return HI_FAILURE;
    }

    if (tde_osi_list_init() != 0) {
        tde_error("TdeOsiListInit failure!\n");
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return HI_FAILURE;
    }

    /* register tde device */
    g_tde_device = osal_createdev("hi_tde");
    if (g_tde_device == HI_NULL) {
        osal_printk("\n  osal_createdev failure\n");
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        return HI_FAILURE;
    }
    g_tde_device->minor = 255; /* 255 max bits */
    g_tde_device->fops = &g_tde_fops;
    g_tde_device->osal_pmops = &g_tde_drv_ops;

    ret = osal_registerdevice(g_tde_device);
    if (ret) {
        osal_printk("\n  osal_registerdevice failure\n");
        tde_osi_list_term();
        osal_free_irq(g_tde_irq, tde_osr_isr);
        tde_hal_release();
        osal_destroydev(g_tde_device);
        return HI_FAILURE;
    }
    if (tde_init_create() != HI_SUCCESS) {
        return HI_FAILURE;
    }
    osal_spin_lock_init(&g_task_let_lock);
    osal_spin_lock_init(&g_tde_ref_lock);
    osal_atomic_init(&g_tde_count);
    return HI_SUCCESS;
}

hi_void tde_cleanup_module_k(hi_void)
{
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_remove_proc_entry("tde", HI_NULL);
#endif
    tde_osi_list_term();
    osal_free_irq(g_tde_irq, tde_osr_isr);

    cmpi_unregister_module(conver_id(HIGFX_TDE_ID));

    osal_spin_lock_destroy(&g_task_let_lock);
    osal_spin_lock_destroy(&g_tde_ref_lock);
    osal_atomic_destroy(&g_tde_count);
    /* cleanup_module is never called if registering failed */
    osal_deregisterdevice(g_tde_device);
    osal_destroydev(g_tde_device);
    tde_hal_release();
    return;
}

int tde_open(hi_void *private_data)
{
    hi_unused(private_data);
    if (osal_atomic_inc_return(&g_tde_count) == 1) {
        (hi_void) tde_hal_open();
    }

    return HI_SUCCESS;
}

int tde_release(hi_void *private_data)
{
    if (private_data != HI_NULL) {
        tde_free_pending_job(private_data);
    }

    if (osal_atomic_read(&g_tde_count) < 0) {
        osal_atomic_set(&g_tde_count, 0);
    }

    return HI_SUCCESS;
}

hi_void convert_surface(hi_tde_surface *src_surface, drv_tde_ioctl_surface *dst_surface)
{
    hi_u64 clutphy_addr;

    clutphy_addr = dst_surface->clut_phy_addr;

    src_surface->support_alpha_ext_1555 = dst_surface->support_alpha_ext_1555;
    src_surface->alpha_max_is_255 = dst_surface->alpha_max_is_255;
    src_surface->is_ycbcr_clut = dst_surface->is_ycbcr_clut;
    src_surface->color_fmt = dst_surface->color_fmt;
    src_surface->clut_phy_addr = clutphy_addr;
    src_surface->cbcr_phy_addr = dst_surface->cbcr_phy_addr;
    src_surface->cbcr_stride = dst_surface->cbcr_stride;
    src_surface->height = dst_surface->height;
    src_surface->phy_addr = dst_surface->phy_addr;
    src_surface->stride = dst_surface->stride;
    src_surface->width = dst_surface->width;
    src_surface->alpha0 = dst_surface->alpha0;
    src_surface->alpha1 = dst_surface->alpha1;
}

typedef union {
    drv_tde_bitblit_cmd blit_cmd;
    drv_tde_solid_draw_cmd draw_cmd;
    drv_tde_quick_deflicker_cmd deflicker_cmd;
    drv_tde_quick_copy_cmd copy_cmd;
    drv_tde_quick_resize_cmd resize_cmd;
    drv_tde_quick_fill_cmd fill_cmd;
    drv_tde_end_job_cmd end_cmd;
    drv_tde_mb_blit_cmd mb_blit_cmd;
    drv_tde_bitmap_maskrop_cmd mask_rop_cmd;
    drv_tde_bitmap_maskblend_cmd mask_blend_cmd;
    drv_tde_pattern_fill_cmd pattern_cmd;
    tde_rotate_cmd rotate_cmd;
} tde_ioctl_cmd;

long tde_ioctl(unsigned int cmd, unsigned long arg, hi_void *private_data)
{
    hi_u8 tmp_cmd = _IOC_NR(cmd);
    if ((tmp_cmd < 1) || (tmp_cmd >= TDE_IOCTL_FUNC_NUM)) {
        return HI_FAILURE;
    }
    if (cmd != g_tde_ctl_func_dispatch_item[tmp_cmd].tde_cmd) {
        return -EFAULT;
    }

    if (g_tde_ctl_func_dispatch_item[tmp_cmd].tde_ctl_ptr_func == HI_NULL) {
        return -EFAULT;
    }

    return g_tde_ctl_func_dispatch_item[tmp_cmd].tde_ctl_ptr_func(arg, private_data);
}

hi_s32 tde_osr_begin_job(unsigned long argp, hi_void *private_data)
{
    hi_s32 ret;
    hi_s32 *handle = HI_NULL;

    if (argp == 0) {
        return -EFAULT;
    }
    handle = (hi_s32 *)(uintptr_t)argp;
    ret = tde_osi_begin_job(handle, private_data);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osr_blit_blit(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface back_ground_surface = {0};
    hi_tde_surface fore_ground_surface = {0};
    hi_tde_surface dst_surface = {0};
    hi_tde_double_src double_src;
    hi_tde_opt *opt = HI_NULL;
    drv_tde_bitblit_cmd *bit_blt = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    bit_blt = (drv_tde_bitblit_cmd *)(uintptr_t)argp;

    convert_surface(&back_ground_surface, &(bit_blt->back_ground_surface));
    convert_surface(&fore_ground_surface, &(bit_blt->fore_ground_surface));
    convert_surface(&dst_surface, &(bit_blt->dst_surface));

    double_src.bg_surface = ((bit_blt->null_indicator >> 1) & 1) ? HI_NULL : &(back_ground_surface);
    /* 2 Take the high */
    double_src.bg_rect = ((bit_blt->null_indicator >> 2) & 1) ? HI_NULL : &(bit_blt->back_ground_rect);
    /* 3 Take the high */
    double_src.fg_surface = ((bit_blt->null_indicator >> 3) & 1) ? HI_NULL : &(fore_ground_surface);
    double_src.fg_rect = ((bit_blt->null_indicator >> 4) & 1) ? HI_NULL : /* 4 Take the high */
                         &(bit_blt->fore_ground_rect);
    opt = ((bit_blt->null_indicator >> 7) & 1) ? HI_NULL : &(bit_blt->option); /* 7 Take the high */

    double_src.dst_surface = &(dst_surface);
    double_src.dst_rect = &(bit_blt->dst_rect);
    return tde_osi_blit(bit_blt->handle, &double_src, opt);
}

static hi_s32 tde_osr_solid_draw(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface fore_ground_surface = {0};
    hi_tde_surface dst_surface = {0};
    hi_tde_fill_color *fill_color = HI_NULL;
    hi_tde_opt *opt = HI_NULL;
    drv_tde_solid_draw_cmd *solid_draw = HI_NULL;
    hi_tde_single_src single_src;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    solid_draw = (drv_tde_solid_draw_cmd *)(uintptr_t)argp;

    convert_surface(&fore_ground_surface, &(solid_draw->fore_ground_surface));
    convert_surface(&dst_surface, &(solid_draw->dst_surface));

    single_src.src_surface = ((solid_draw->null_indicator >> 1) & 1) ? HI_NULL : &fore_ground_surface;
    single_src.src_rect = ((solid_draw->null_indicator >> 2) & 1) ? HI_NULL : /* 2 Take the high */
                          &(solid_draw->fore_ground_rect);
    /* 5 Take the high */
    fill_color = ((solid_draw->null_indicator >> 5) & 1) ? HI_NULL : &(solid_draw->fill_color);
    opt = ((solid_draw->null_indicator >> 6) & 1) ? HI_NULL : &(solid_draw->option); /* 6 Take the high */

    single_src.dst_rect = &(solid_draw->dst_rect);
    single_src.dst_surface = &(dst_surface);
    return tde_osi_solid_draw(solid_draw->handle, &single_src, fill_color, opt);
}

static hi_s32 tde_osr_quick_copy(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface dst_surface = {0};
    hi_tde_surface src_surface = {0};
    drv_tde_quick_copy_cmd *quick_copy = HI_NULL;
    hi_tde_single_src single_src;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    quick_copy = (drv_tde_quick_copy_cmd *)(uintptr_t)argp;

    convert_surface(&src_surface, &(quick_copy->src_surface));
    convert_surface(&dst_surface, &(quick_copy->dst_surface));

    single_src.src_surface = &(src_surface);
    single_src.src_rect = &(quick_copy->src_rect);
    single_src.dst_surface = &(dst_surface);
    single_src.dst_rect = &(quick_copy->dst_rect);
    return tde_osi_quick_copy(quick_copy->handle, &single_src);
}

static hi_s32 tde_osr_quick_resize(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface dst_surface = {0};
    hi_tde_surface src_surface = {0};
    drv_tde_quick_resize_cmd *quick_resize = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    quick_resize = (drv_tde_quick_resize_cmd *)(uintptr_t)argp;
    convert_surface(&src_surface, &(quick_resize->src_surface));
    convert_surface(&dst_surface, &(quick_resize->dst_surface));

    return tde_osi_quick_resize(quick_resize->handle, &(src_surface), &(quick_resize->src_rect), &(dst_surface),
                                &(quick_resize->dst_rect));
}

static hi_s32 tde_osr_quick_fill(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface dst_surface = {0};
    drv_tde_quick_fill_cmd *quick_fill = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    quick_fill = (drv_tde_quick_fill_cmd *)(uintptr_t)argp;

    convert_surface(&dst_surface, &(quick_fill->dst_surface));

    return tde_osi_quick_fill(quick_fill->handle, &(dst_surface),
                              &(quick_fill->dst_rect), quick_fill->fill_data);
}

static hi_s32 tde_osr_quick_deflicker(unsigned long argp, hi_void *private_data)
{
#if (TDE_CAPABILITY & DEFLICKER)
    hi_tde_surface dst_surface = {0};
    hi_tde_surface src_surface = {0};
    drv_tde_quick_deflicker_cmd *deflicker = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    deflicker = (drv_tde_quick_deflicker_cmd *)(uintptr_t)argp;

    convert_surface(&src_surface, &(deflicker->src_surface));
    convert_surface(&dst_surface, &(deflicker->dst_surface));

    return tde_osi_quick_flicker(deflicker->handle, &(src_surface), &(deflicker->src_rect), &(dst_surface),
                                 &(deflicker->dst_rect));
#else
    hi_unused(argp);
    hi_unused(private_data);
    return HI_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static hi_s32 tde_osr_mb_blit_blit(unsigned long argp, hi_void *private_data)
{
    hi_tde_mb_src mb_src;
    hi_tde_surface dst_surface = {0};
    drv_tde_mb_blit_cmd *mb_bitblt = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    mb_bitblt = (drv_tde_mb_blit_cmd *)(uintptr_t)argp;
    convert_surface(&dst_surface, &(mb_bitblt->dst_surface));
    mb_src.dst_rect = &(mb_bitblt->dst_rect);
    mb_src.dst_surface = &(dst_surface);
    mb_src.mb_surface = &(mb_bitblt->mb_src_surface);
    mb_src.src_rect = &(mb_bitblt->mb_src_rect);
    return tde_osi_mb_blit(mb_bitblt->handle, &mb_src, &(mb_bitblt->mb_option));
}

static hi_s32 tde_osr_end_job(unsigned long argp, hi_void *private_data)
{
    drv_tde_end_job_cmd *end_job = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    end_job = (drv_tde_end_job_cmd *)(uintptr_t)argp;

    return tde_osi_end_job(end_job, HI_NULL, HI_NULL);
}

static hi_s32 tde_osr_wait_for_done(unsigned long argp, hi_void *private_data)
{
    hi_s32 *handle = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    handle = (hi_s32 *)(uintptr_t)argp;

    return tde_osi_wait_for_done(*handle, TDE_MAX_WAIT_TIMEOUT);
}

static hi_s32 tde_osr_cancel_job(unsigned long argp, hi_void *private_data)
{
    hi_s32 *handle = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    handle = (hi_s32 *)(uintptr_t)argp;

    return tde_osi_cancel_job(*handle);
}

static hi_s32 tde_osr_bit_map_mask_rop(unsigned long argp, hi_void *private_data)
{
#if (TDE_CAPABILITY & MASKROP)
    hi_tde_surface bg_surface = {0};
    hi_tde_surface fg_surface = {0};
    hi_tde_surface dst_surface = {0};
    hi_tde_surface mask_surface = {0};
    drv_tde_bitmap_maskrop_cmd *bmp_mask_rop = HI_NULL;
    hi_tde_triple_src triple_src = {0};
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    bmp_mask_rop = (drv_tde_bitmap_maskrop_cmd *)(uintptr_t)argp;

    convert_surface(&bg_surface, &(bmp_mask_rop->back_ground_surface));
    convert_surface(&fg_surface, &(bmp_mask_rop->fore_ground_surface));
    convert_surface(&mask_surface, &(bmp_mask_rop->mask_surface));
    convert_surface(&dst_surface, &(bmp_mask_rop->dst_surface));

    triple_src.bg_surface = &bg_surface;
    triple_src.bg_rect = &(bmp_mask_rop->back_ground_rect);
    triple_src.fg_surface = &fg_surface;
    triple_src.fg_rect = &(bmp_mask_rop->fore_ground_rect);
    triple_src.mask_surface = &mask_surface;
    triple_src.mask_rect = &(bmp_mask_rop->mask_rect);
    triple_src.dst_surface = &dst_surface;
    triple_src.dst_rect = &(bmp_mask_rop->dst_rect);

    return tde_osi_bitmap_mask_rop(bmp_mask_rop->handle, &triple_src, bmp_mask_rop->rop_color,
                                   bmp_mask_rop->rop_alpha);
#else
    hi_unused(argp);
    hi_unused(private_data);
    return HI_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static hi_s32 tde_osr_bit_map_mask_blend(unsigned long argp, hi_void *private_data)
{
#if (TDE_CAPABILITY & MASKROP)
    hi_tde_surface bg_surface = {0};
    hi_tde_surface fg_surface = {0};
    hi_tde_surface dst_surface = {0};
    hi_tde_surface mask_surface = {0};
    drv_tde_bitmap_maskblend_cmd *bmp_mask_blend = HI_NULL;
    hi_tde_triple_src triple_src = {0};
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    bmp_mask_blend = (drv_tde_bitmap_maskblend_cmd *)(uintptr_t)argp;

    convert_surface(&bg_surface, &(bmp_mask_blend->back_ground_surface));
    convert_surface(&fg_surface, &(bmp_mask_blend->fore_ground_surface));
    convert_surface(&mask_surface, &(bmp_mask_blend->mask_surface));
    convert_surface(&dst_surface, &(bmp_mask_blend->dst_surface));

    triple_src.bg_surface = &bg_surface;
    triple_src.bg_rect = &(bmp_mask_blend->back_ground_rect);
    triple_src.fg_surface = &fg_surface;
    triple_src.fg_rect = &(bmp_mask_blend->fore_ground_rect);
    triple_src.mask_surface = &mask_surface;
    triple_src.mask_rect = &(bmp_mask_blend->mask_rect);
    triple_src.dst_surface = &dst_surface;
    triple_src.dst_rect = &(bmp_mask_blend->dst_rect);

    return tde_osi_bitmap_mask_blend(bmp_mask_blend->handle, &triple_src, bmp_mask_blend->alpha,
                                     bmp_mask_blend->blend_mode);
#else
    hi_unused(argp);
    hi_unused(private_data);
    return HI_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static hi_s32 tde_osr_wait_all_done(unsigned long argp, hi_void *private_data)
{
    hi_unused(argp);
    hi_unused(private_data);
    return tde_osi_wait_all_done(HI_FALSE);
}

static hi_s32 tde_osr_reset(unsigned long argp, hi_void *private_data)
{
    hi_unused(argp);
    hi_unused(private_data);
    return HI_SUCCESS;
}

static hi_s32 tde_osr_set_deflicker_level(unsigned long argp, hi_void *private_data)
{
#if (TDE_CAPABILITY & DEFLICKER)
    hi_tde_deflicker_level deflicker_level;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    deflicker_level = *(hi_tde_deflicker_level *)(uintptr_t)argp;

    return tde_osi_set_deflicker_level(deflicker_level);
#else
    hi_unused(argp);
    hi_unused(private_data);
    return HI_ERR_TDE_UNSUPPORTED_OPERATION;
#endif
}

static hi_s32 tde_osr_get_deflicker_level(unsigned long argp, hi_void *private_data)
{
    hi_tde_deflicker_level *deflicker_level = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    deflicker_level = (hi_tde_deflicker_level *)(uintptr_t)argp;
#if (TDE_CAPABILITY & DEFLICKER)
    if (tde_osi_get_deflicker_level(deflicker_level) != HI_SUCCESS) {
        return HI_ERR_TDE_NULL_PTR;
    }
#else
    *deflicker_level = HI_TDE_DEFLICKER_LEVEL_AUTO;
#endif
    return HI_SUCCESS;
}

static hi_s32 tde_osr_set_alpha_threshold_value(unsigned long argp, hi_void *private_data)
{
    hi_u8 alpha_threshold_value;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    alpha_threshold_value = *(hi_u8 *)(uintptr_t)argp;

    return tde_osi_set_alpha_threshold_value(alpha_threshold_value);
}

static hi_s32 tde_osr_get_alpha_threshold_value(unsigned long argp, hi_void *private_data)
{
    hi_u8 *alpha_threshold_value = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    alpha_threshold_value = (hi_u8 *)(uintptr_t)argp;

    if (tde_osi_get_alpha_threshold_value(alpha_threshold_value)) {
        return HI_ERR_TDE_NULL_PTR;
    }

    return HI_SUCCESS;
}

static hi_s32 tde_osr_set_alpha_threshold_state(unsigned long argp, hi_void *private_data)
{
    hi_bool alpha_threshold_en = HI_FALSE;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    alpha_threshold_en = *(hi_bool *)(uintptr_t)argp;

    return tde_osi_set_alpha_threshold_state(alpha_threshold_en);
}

static hi_s32 tde_osr_get_alpha_threshold_state(unsigned long argp, hi_void *private_data)
{
    hi_bool alpha_threshold_en = HI_FALSE;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    tde_osi_get_alpha_threshold_state(&alpha_threshold_en);

    *(hi_bool *)(uintptr_t)argp = alpha_threshold_en;

    return HI_SUCCESS;
}

static hi_s32 tde_osr_pattern_fill(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface back_ground_surface = {0};
    hi_tde_surface fore_ground_surface = {0};
    hi_tde_surface dst_surface = {0};

    hi_tde_double_src double_src = {0};
    hi_tde_pattern_fill_opt *opt = HI_NULL;
    drv_tde_pattern_fill_cmd *pattern_fill_cmd = HI_NULL;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    pattern_fill_cmd = (drv_tde_pattern_fill_cmd *)(uintptr_t)argp;

    convert_surface(&back_ground_surface, &(pattern_fill_cmd->back_ground_surface));
    convert_surface(&fore_ground_surface, &(pattern_fill_cmd->fore_ground_surface));
    convert_surface(&dst_surface, &(pattern_fill_cmd->dst_surface));

    double_src.bg_surface = ((pattern_fill_cmd->null_indicator >> 1) & 1) ? HI_NULL : &(back_ground_surface);
    double_src.bg_rect = ((pattern_fill_cmd->null_indicator >> 2) & 1) ? HI_NULL : /* 2 Take the high */
        &(pattern_fill_cmd->back_ground_rect);
    double_src.fg_surface = ((pattern_fill_cmd->null_indicator >> 3) & 1) ? HI_NULL : /* 3 Take the high */
        &(fore_ground_surface);
    double_src.fg_rect = ((pattern_fill_cmd->null_indicator >> 4) & 1) ? HI_NULL : /* 4 Take the high */
        &(pattern_fill_cmd->fore_ground_rect);
    double_src.dst_surface = ((pattern_fill_cmd->null_indicator >> 5) & 1) ? /* 5 Take the high */
        HI_NULL : &(dst_surface);
    double_src.dst_rect = ((pattern_fill_cmd->null_indicator >> 6) & 1) ? HI_NULL : /* 6 Take the high */
        &(pattern_fill_cmd->dst_rect);
    opt = ((pattern_fill_cmd->null_indicator >> 7) & 1) ? HI_NULL : /* 7 Take the high */
        &(pattern_fill_cmd->option);

    return tde_osi_pattern_fill(pattern_fill_cmd->handle, &double_src, opt);
}

static hi_s32 tde_osr_enable_region_deflicker(unsigned long argp, hi_void *private_data)
{
    hi_bool is_region_deflicker = HI_FALSE;
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    is_region_deflicker = *(hi_bool *)(uintptr_t)argp;

    return tde_osi_enable_region_deflicker(is_region_deflicker);
}

static hi_s32 tde_osr_rotate(unsigned long argp, hi_void *private_data)
{
    hi_tde_surface dst_surface = {0};
    hi_tde_surface src_surface = {0};
    tde_rotate_cmd *rotate_cmd = HI_NULL;
    hi_tde_single_src single_src = {0};
    hi_unused(private_data);
    if (argp == 0) {
        return -EFAULT;
    }
    rotate_cmd = (tde_rotate_cmd *)(uintptr_t)argp;

    convert_surface(&src_surface, &(rotate_cmd->src_surface));
    convert_surface(&dst_surface, &(rotate_cmd->dst_surface));

    single_src.src_surface = &src_surface;
    single_src.src_rect = &(rotate_cmd->src_rect);
    single_src.dst_surface = &dst_surface;
    single_src.dst_rect = &(rotate_cmd->dst_rect);
    return tde_osi_quick_rotate(rotate_cmd->handle, &single_src, rotate_cmd->rotate_angle);
}

#ifdef TDE_COREDUMP_DEBUG
#define tde_read_reg(base, offset) (*(volatile unsigned int *)((void *)(base) + (offset)))
#endif

static int tde_osr_isr(int irq, hi_void *dev_id)
{
    hi_u32 int_status;
    hi_unused(irq);
    hi_unused(dev_id);
#ifdef TDE_TIME_COUNT
    (hi_void) osal_gettimeofday(&g_time_start);
#endif
    int_status = tde_hal_ctl_int_status();
    if (int_status & TDE_DRV_INT_ERROR) {
#ifdef TDE_COREDUMP_DEBUG
        hi_u32 read_stats;
        hi_u32 i;
        for (i = 0; i < 74; i++) { /* 74 Error output */
            read_stats = tde_read_reg(tde_hal_get_base_vir_addr(), (0x800 + i * 4)); /* 4 alg data */
            tde_error("\n--------- ADDR:0x%x Value:0x%x---------\n", (0x800 + (i * 4)), read_stats); /* 4 alg data */
        }
#endif
        tde_error("tde interrupts coredump!\n");
        tde_hal_resume_init();

        return OSAL_IRQ_HANDLED;
    }

    if (int_status & TDE_DRV_INT_TIMEOUT) {
        tde_error("tde hardware timeout!\n");

        tde_hal_resume_init();
        return OSAL_IRQ_HANDLED;
    }

    tde_info("tde register int status: 0x%x!\n", (hi_u32)int_status);

    tde_tasklet_func(int_status);

    return OSAL_IRQ_HANDLED;
}

static void tde_tasklet_func(unsigned long int_status)
{
    unsigned long lockflags;
    tde_spin_lock(&g_task_let_lock, lockflags);

#ifndef __RTOS__
    tde_tasklet.data &= (~int_status);
#endif

    tde_spin_unlock(&g_task_let_lock, lockflags);
#ifdef TDE_TIME_COUNT
    (hi_void) osal_gettimeofday(&g_time_end);

    g_time_diff = (g_time_end.tv_sec - g_time_start.tv_sec) * 1000000 + /* 1000000 expand */
        (g_time_end.tv_usec - g_time_start.tv_usec);
    tde_info("tde int status: 0x%x, g_u64TimeDiff:%d!\n", (hi_u32)int_status, (hi_u32)g_time_diff);
#endif

#ifdef TDE_HWC_COOPERATE
    if (int_status & TDE_DRV_INT_NODE) {
        tde_osi_list_node_comp();
    }
    tde_osi_list_comp();
#else
    if (int_status & TDE_DRV_INT_NODE_COMP_AQ) {
        tde_osi_list_node_comp();
    }
#endif
}

#ifdef CONFIG_TDE_PM_ENABLE
/* tde wait for start  */
int tde_pm_suspend(pm_basedev *pdev, pm_message_t state)
{
    tde_osi_wait_all_done(HI_FALSE);

    tde_hal_suspend();

    HI_PRINT("TDE suspend OK\n");

    return HI_SUCCESS;
}

/* wait for resume */
int tde_pm_resume(pm_basedev *pdev)
{
    if (osal_atomic_read(&g_tde_count) > 0) {
        tde_hal_resume_init();
    }
    HI_PRINT("TDE resume OK\n");

    return HI_SUCCESS;
}
#endif
hi_s32 tde_osi_open(hi_void)
{
    return tde_open(HI_NULL);
}

hi_s32 tde_osi_close(hi_void)
{
    return tde_release(HI_NULL);
}

#endif

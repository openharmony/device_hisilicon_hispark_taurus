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

#include "vou.h"
#include "hi_osal.h"
#include "hi_debug_adapt.h"
#include "hi_comm_vo_adapt.h"
#include "hi_comm_video_adapt.h"

#include "mkp_vo_dev.h"
#include "hi_board.h"
#include "vou_drv.h"
#include "vou_chn.h"
#include "vou_dev_exp.h"

#include "valg_plat.h"
#ifdef CONFIG_HI_VO_GRAPH
#include "vou_graphics.h"
#endif
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
#include "vou_proc.h"
#endif
#ifdef CONFIG_DRM_HISI_HISMART
#include "drm_display_func_ext.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

hi_u32 g_vou_state = VOU_STATE_STOPPED;
static osal_atomic_t g_vou_user_ref = OSAL_ATOMIC_INIT(0);

osal_spinlock_t g_vo_lock;
osal_spinlock_t g_vou_list_lock;

struct osal_semaphore g_dev_sem;
struct osal_semaphore g_chn_sem;

static osal_dev_t *g_vo_device;

hi_vo_mod_param g_vo_mod_param = {
    .transparent_transmit = HI_FALSE,
    .exit_dev = HI_TRUE,
    .wbc_bg_black_en = HI_FALSE,
    .dev_clk_ext_en = HI_FALSE,
    .save_buf_mode = { [0 ... VO_MAX_PHY_DEV_NUM - 1] = HI_FALSE },
};

vo_dev_info g_vo_dev[VO_MAX_DEV_NUM];

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
vou_virt_dev g_vou_virt_dev[VO_MAX_VIRT_DEV_NUM];
#endif

unsigned int g_transparent_transmit = HI_FALSE;

unsigned int g_low_power_mode = HI_TRUE;

hi_u32 vo_get_vo_state(hi_void)
{
    return g_vou_state;
}

osal_spinlock_t *vo_get_dev_lock(hi_void)
{
    return &g_vo_lock;
}

osal_spinlock_t *vo_get_list_lock(hi_void)
{
    return &g_vou_list_lock;
}
struct osal_semaphore *vo_get_chn_sem(hi_void)
{
    return &g_chn_sem;
}

struct osal_semaphore *vo_get_dev_sem(hi_void)
{
    return &g_dev_sem;
}

hi_vo_mod_param *vo_get_vo_mod_param(hi_void)
{
    return &g_vo_mod_param;
}

vo_dev_info *vo_get_dev_ctx(hi_vo_dev vo_dev)
{
    return &g_vo_dev[vo_dev];
}

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
vou_virt_dev *vo_get_virt_dev_ctx(hi_vo_dev vo_dev)
{
    return &g_vou_virt_dev[vo_dev];
}
#endif

hi_u32 vo_get_transparent_transmit(hi_void)
{
    return g_transparent_transmit;
}

hi_s32 vo_def_check_dev_id(hi_vo_dev dev)
{
    if ((dev < 0) || (dev >= VO_MAX_DEV_NUM)) {
        return HI_ERR_VO_INVALID_DEVID;
    }

    return HI_SUCCESS;
}

hi_void vou_sync_resource_init(hi_void)
{
    vo_init_lock(&g_vou_list_lock);
    vo_init_lock(&g_vo_lock);

#ifdef CONFIG_HI_VO_GRAPH
    vou_graphics_lock_init();
#endif

    osal_sema_init(&g_dev_sem, 1);
    osal_sema_init(&g_chn_sem, 1);
}

hi_void vou_sync_resource_de_init(hi_void)
{
#ifdef CONFIG_HI_VO_GRAPH
    vou_graphics_lock_de_init();
#endif

    vo_deinit_lock(&g_vo_lock);
    vo_deinit_lock(&g_vou_list_lock);

    osal_sema_destroy(&g_dev_sem);
    osal_sema_destroy(&g_chn_sem);
}

static hi_s32 vou_disable_soft(hi_vo_dev dev)
{
    hi_u32 flags = 0;

    vo_check_dev_id_return(dev);

    vo_spin_lock(&g_vo_lock, &flags);

    g_vo_dev[dev].config = HI_FALSE;
    g_vo_dev[dev].vo_enable = HI_FALSE;

    vo_drv_disable(dev);

    vo_spin_unlock(&g_vo_lock, &flags);

    return HI_SUCCESS;
}

static hi_s32 vo_dev_info_init(hi_void)
{
    vo_dev_info *ctx = HI_NULL;
    hi_u32 i;

    vo_drv_dev_info_init();

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        ctx = &g_vo_dev[i];
        (hi_void)memset_s(ctx, sizeof(vo_dev_info), 0, sizeof(vo_dev_info));
    }

    return HI_SUCCESS;
}

static hi_s32 vou_dev_info_de_init(hi_void)
{
    vo_dev_info *ctx = HI_NULL;
    hi_u32 i;
    vo_drv_dev_info_de_init();
    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        ctx = &g_vo_dev[i];

        (hi_void)memset_s(ctx, sizeof(vo_dev_info), 0, sizeof(vo_dev_info));
    }

    return HI_SUCCESS;
}

static hi_s32 vou_dev_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    hi_vo_dev dev = vo_get_dev(private_data);
#ifdef CONFIG_HI_VO_GRAPH
    hi_graphic_layer gfx_layer = *(hi_u32 *)((hi_uintptr_t)(private_data));
#endif

    vo_debug_trace("cmd: %#x, dev: %d\n", cmd, dev);

    switch (cmd) {
        /* device operation */
        case VOU_DEV_SET_FD: {
            UMAP_SET_CHN(private_data, *((hi_u32 *)(hi_uintptr_t)arg));
            break;
        }
        case VOU_OPEN_CTRL: {
            return vou_enable(dev);
        }
        case VOU_CLOSE_CTRL: {
            return vou_disable(dev);
        }
        case VOU_PUB_ATTR_SET_CTRL: {
            hi_vo_pub_attr *attr = (hi_vo_pub_attr *)(hi_uintptr_t)arg;
            return vou_set_pub_attr(dev, attr);
        }
        case VOU_PUB_ATTR_GET_CTRL: {
            hi_vo_pub_attr *attr = (hi_vo_pub_attr *)(hi_uintptr_t)arg;
            return vou_get_pub_attr(dev, attr);
        }
        case VOU_DEV_FPS_SET_CTRL: {
            hi_u32 *frame_rate = (hi_u32 *)(hi_uintptr_t)arg;
            return vou_set_dev_frame_rate(dev, frame_rate);
        }
        case VOU_DEV_FPS_GET_CTRL: {
            hi_u32 *frame_rate = (hi_u32 *)(hi_uintptr_t)arg;
            return vou_get_dev_frame_rate(dev, frame_rate);
        }
        case VOU_DEV_VTTH_SET_CTRL: {
            hi_u32 *vtth = (hi_u32 *)(hi_uintptr_t)arg;
            return vou_set_vtth(dev, vtth);
        }
        case VOU_DEV_VTTH_GET_CTRL: {
            hi_u32 *vtth = (hi_u32 *)(hi_uintptr_t)arg;
            return vou_get_vtth(dev, vtth);
        }
        case VOU_DEV_VTTH2_SET_CTRL: {
            hi_u32 *vtth = (hi_u32 *)(hi_uintptr_t)arg;
            return vou_set_vtth2(dev, vtth);
        }
        case VOU_DEV_VTTH2_GET_CTRL: {
            hi_u32 *vtth = (hi_u32 *)(hi_uintptr_t)arg;
            return vou_get_vtth2(dev, vtth);
        }

        case VOU_USER_INTFSYNC_INFO_SET_CTRL: {
            hi_vo_user_intfsync_info *vo_user_sync_info = (hi_vo_user_intfsync_info *)(hi_uintptr_t)arg;
            return vou_set_user_intf_sync_info(dev, vo_user_sync_info);
        }

        /* graphic */
        case VOU_GFX_LAYER_BIND_CTRL: {
#ifdef CONFIG_HI_VO_GRAPH
            vo_check_null_ptr_return((hi_vo_dev *)(hi_uintptr_t)arg);

            dev = *(hi_vo_dev *)(hi_uintptr_t)arg;
            return vou_graphics_bind_dev(gfx_layer, dev);
#else
            vo_err_trace("hi_mpi_vo_bind_graphic_layer is not supported!\n");
            return HI_ERR_VO_NOT_SUPPORT;
#endif
        }
        case VOU_GFX_LAYER_UNBIND_CTRL: {
#ifdef CONFIG_HI_VO_GRAPH
            vo_check_null_ptr_return((hi_vo_dev *)(hi_uintptr_t)arg);

            dev = *(hi_vo_dev *)(hi_uintptr_t)arg;

            return vou_graphics_un_bind_dev(gfx_layer, dev);
#else
            vo_err_trace("hi_mpi_vo_un_bind_graphic_layer is not supported!\n");
            return HI_ERR_VO_NOT_SUPPORT;
#endif
        }
        case VOU_GFX_LAYER_CSC_SET_CTRL: {
#ifdef CONFIG_HI_VO_GRAPH

            hi_vo_csc *csc = (hi_vo_csc *)(hi_uintptr_t)arg;
            return vou_graphics_set_csc(gfx_layer, csc);

#else
            vo_err_trace("hi_mpi_vo_set_graphic_layer_csc is not supported!\n");
            return HI_ERR_VO_NOT_SUPPORT;
#endif
        }
        case VOU_GFX_LAYER_CSC_GET_CTRL: {
#ifdef CONFIG_HI_VO_GRAPH
            hi_vo_csc *csc = (hi_vo_csc *)(hi_uintptr_t)arg;
            return vou_graphics_get_csc(gfx_layer, csc);
#else
            vo_err_trace("hi_mpi_vo_get_graphic_layer_csc is not supported!\n");
            return HI_ERR_VO_NOT_SUPPORT;
#endif
        }
        case VO_MOD_PARAM_SET_CTRL: {
            hi_vo_mod_param *vo_mod_param = (hi_vo_mod_param *)(hi_uintptr_t)arg;
            return vo_set_mod_param(vo_mod_param);
        }
        case VO_MOD_PARAM_GET_CTRL: {
            hi_vo_mod_param *vo_mod_param = (hi_vo_mod_param *)(hi_uintptr_t)arg;
            return vo_get_mod_param(vo_mod_param);
        }

        default: {
            vo_warn_trace("ERR IOCTL CMD %#x!\n", cmd);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

#ifdef CONFIG_COMPAT
static hi_s32 vou_dev_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    return vou_dev_ioctl(cmd, arg, private_data);
}
#endif

static long vou_device_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    int ret;

    if (g_vou_state != VOU_STATE_STARTED) {
        vo_err_trace("vo not init !\n");
        return HI_ERR_VO_SYS_NOTREADY;
    }

    osal_atomic_inc_return(&g_vou_user_ref);

    ret = vou_dev_ioctl(cmd, arg, private_data);

    osal_atomic_dec_return(&g_vou_user_ref);

    return ret;
}

#ifdef CONFIG_COMPAT
static long vou_device_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    int ret;

    if (g_vou_state != VOU_STATE_STARTED) {
        vo_err_trace("vo not init !\n");
        return HI_ERR_VO_SYS_NOTREADY;
    }

    osal_atomic_inc_return(&g_vou_user_ref);

    ret = vou_dev_compat_ioctl(cmd, arg, private_data);

    osal_atomic_dec_return(&g_vou_user_ref);

    return ret;
}
#endif

static hi_s32 vou_open(void *private_data)
{
    hi_unused(private_data);
    return 0;
}

static hi_s32 vou_close(void *private_data)
{
    hi_unused(private_data);
    return 0;
}

static unsigned int vou_poll(osal_poll_t *osal_poll, void *private_data)
{
    hi_unused(osal_poll);
    hi_unused(private_data);
    return HI_SUCCESS;
}

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
static hi_s32 vou_freeze(osal_dev_t *dev)
{
    HI_PRINT("%s %d\n", __func__, __LINE__);
    return HI_SUCCESS;
}

static hi_s32 vou_restore(osal_dev_t *dev)
{
    hi_u32 flags = 0;

    vo_lpw_bus_reset(HI_FALSE);

    vo_spin_lock(&g_vo_lock, &flags);
    vo_drv_set_all_crg_clk(HI_TRUE);

    vou_drv_default_setting();

    if (g_low_power_mode) {
        vo_drv_set_all_crg_clk(HI_FALSE);
    }

    vo_spin_unlock(&g_vo_lock, &flags);

    HI_PRINT("%s %d\n", __func__, __LINE__);
    return HI_SUCCESS;
}
#else
static hi_s32 vou_freeze(osal_dev_t *dev)
{
    hi_unused(dev);
    return HI_SUCCESS;
}

static hi_s32 vou_restore(osal_dev_t *dev)
{
    hi_unused(dev);
    return HI_SUCCESS;
}
#endif

static struct osal_fileops g_vo_file_op = {
    .open = vou_open,
    .unlocked_ioctl = vou_device_ioctl,
    .release = vou_close,
    .poll = vou_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl = vou_device_compat_ioctl,
#endif
};

struct osal_pmops g_vou_drv_ops = {
    .pm_freeze = vou_freeze,
    .pm_restore = vou_restore,
};

hi_s32 vou_receive_sys_call_back(hi_vo_layer layer, hi_vo_chn chn, hi_bool block,
                                 mpp_data_type data_type, hi_void *data)
{
    hi_unused(data);
    return HI_SUCCESS;
}

hi_s32 vou_reset_call_back(hi_vo_layer layer, hi_vo_chn chn, hi_void *data)
{
    hi_unused(data);
    return HI_SUCCESS;
}

unsigned int g_vou_irq = VOU_IRQ_NR;

static hi_bool g_is_initialized = HI_FALSE;

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
static hi_void vou_virt_dev_context_init(hi_void)
{
    hi_s32 i = 0;

    for (i = 0; i < VO_MAX_VIRT_DEV_NUM; i++) {
        valg_timer_init(&g_vou_virt_dev[i].timer, vou_virt_dev_timer_func, i + VO_VIRT_LAYER_0);
        g_vou_virt_dev[i].scale_pts = 0;
    }
}
#endif

hi_void vou_context_init(hi_void)
{
    if ((g_is_initialized == HI_FALSE) || (g_vo_mod_param.exit_dev == HI_TRUE)) {
        vo_dev_info_init();
    } else {
        vo_drv_dev_info_part_init();
    }

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
    vou_virt_dev_context_init();
#endif
}

hi_void vou_context_de_init(hi_void)
{
    hi_s32 i = 0;

    if (g_vo_mod_param.exit_dev == HI_TRUE) {
        vou_dev_info_de_init();
    } else {
        vo_drv_dev_info_de_init();
    }

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        if (vou_drv_check_dev_id(i) == HI_SUCCESS) {
            if (g_vo_mod_param.exit_dev == HI_FALSE) {
            } else {
                vou_disable_soft(i);
            }
        }
    }

#ifdef CONFIG_HI_VO_VIRTDEV_SUPPORT
    for (i = 0; i < VO_MAX_VIRT_DEV_NUM; i++) {
        if (g_vou_virt_dev[i].timer.timer != HI_NULL) {
            valg_timer_delete(&g_vou_virt_dev[i].timer);
        }
    }
#endif
}

static hi_void vo_init_set_sys_clk(hi_void)
{
    vo_set_low_power_ctrl_clk_en(HI_TRUE);
    vo_lpw_bus_reset(HI_FALSE);
    vo_drv_set_hd_clk_sel(0);

    return;
}

static hi_void vo_init_crg_clk(hi_void)
{
    hi_u32 flags = 0;

    vo_spin_lock(&g_vo_lock, &flags);
    vo_drv_set_all_crg_clk(HI_TRUE);
    vo_spin_unlock(&g_vo_lock, &flags);
}

static hi_void vo_init_default_setting(hi_void)
{
    hi_u32 flags = 0;

    vo_spin_lock(&g_vo_lock, &flags);
    vou_drv_default_setting();
    vo_spin_unlock(&g_vo_lock, &flags);
}

static hi_void vo_set_drv_dev_int_enable(hi_void)
{
    hi_s32 i;

    if (g_vo_mod_param.exit_dev == HI_FALSE) {
        for (i = 0; i < VO_MAX_PHY_DEV_NUM; i++) {
            if (vou_drv_check_dev_id(i) == HI_SUCCESS) {
                vo_drv_dev_int_enable(i, HI_TRUE);
            }
        }
    }
}

static hi_s32 vo_register_receiver(hi_void)
{
    bind_receiver_info receiver_info;

    receiver_info.mod_id = HI_ID_VO_DEV;
    receiver_info.max_dev_cnt = VO_MAX_DEV_NUM;
    receiver_info.max_chn_cnt = VO_MAX_DEV_NUM;
    receiver_info.support_delay_data = HI_TRUE;
    receiver_info.call_back = vou_receive_sys_call_back;
    receiver_info.reset_call_back = vou_reset_call_back;

    return call_sys_register_receiver(&receiver_info);
}

static hi_s32 vo_register_sender(hi_void)
{
    bind_sender_info sender_info;

    sender_info.mod_id = HI_ID_VO_DEV;
    sender_info.max_dev_cnt = VO_MAX_DEV_NUM;
    sender_info.max_chn_cnt = VO_MAX_DEV_NUM;

    return call_sys_register_sender(&sender_info);
}

static hi_s32 vo_init_regitster(hi_void)
{
    hi_s32 ret;

    ret = vo_register_receiver();
    if (ret != HI_SUCCESS) {
        vo_alert_trace("register receiver failed!\n");
        return HI_FAILURE;
    }

    ret = vo_register_sender();
    if (ret != HI_SUCCESS) {
        vo_alert_trace("register sender failed!\n");
        call_sys_unregister_receiver(HI_ID_VO_DEV);
    }

    return HI_SUCCESS;
}

hi_s32 vo_init(hi_void)
{
    hi_s32 ret;

    vo_init_set_sys_clk();
    if (g_vou_state == VOU_STATE_STARTED) {
        return HI_SUCCESS;
    }
    vo_init_crg_clk();
    vou_context_init();
    vo_init_default_setting();
    ret = vo_init_regitster();
    if (ret != HI_SUCCESS) {
        return ret;
    }
    g_vou_state = VOU_STATE_STARTED;
#ifdef CONFIG_HI_VO_GRAPH
    vo_graphics_init();
#endif

    g_is_initialized = HI_TRUE;

    vo_set_drv_dev_int_enable();
    return HI_SUCCESS;
}

static hi_void vo_exit_dev(hi_void)
{
    hi_s32 i;

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        if (vou_drv_check_dev_id(i) != HI_SUCCESS) {
            continue;
        }

        if (g_vo_dev[i].vo_enable) {
            if (g_vo_mod_param.exit_dev == HI_FALSE) {
            } else {
                vou_disable(i);
            }
        }
    }
}

static hi_void vo_exit_unregister(hi_void)
{
    call_sys_unregister_sender(HI_ID_VO_DEV);
    call_sys_unregister_receiver(HI_ID_VO_DEV);
}

static hi_void vo_exit_set_clk(hi_void)
{
    hi_u32 flags = 0;

    if (g_vo_mod_param.exit_dev == HI_FALSE) {
    } else {
        vo_lpw_bus_reset(HI_TRUE);
        vo_set_low_power_ctrl_clk_en(HI_FALSE);

        vo_spin_lock(&g_vo_lock, &flags);
        vo_drv_set_all_crg_clk(HI_FALSE);
        vo_spin_unlock(&g_vo_lock, &flags);
    }
}

hi_void vou_exit(hi_void)
{
    hi_u32 flags = 0;
    if (g_vou_state == VOU_STATE_STOPPED) {
        return;
    }
    if (g_vo_mod_param.exit_dev == HI_TRUE) {
        vo_spin_lock(&g_vo_lock, &flags);
        vo_drv_set_all_crg_clk(HI_TRUE);
        vo_spin_unlock(&g_vo_lock, &flags);
    }
    vo_exit_dev();
    vou_drv_int_disable_all();
    vou_drv_int_clear(VOU_INTCLEAR_ALL);

    vou_context_de_init();

    vo_exit_unregister();

#ifdef CONFIG_HI_VO_GRAPH
    vo_graphics_de_init();
#endif
    vo_exit_set_clk();

    g_vou_state = VOU_STATE_STOPPED;
    vo_debug_trace("VOU exit!");

    return;
}

hi_void vou_check_module_param(hi_void)
{
    if ((g_transparent_transmit != 0) && (g_transparent_transmit != 1)) {
        osal_printk("module param g_transparent_transmit(%u) should be 0 or 1, it will be set to 1\n",
                    g_transparent_transmit);
        g_transparent_transmit = 1;
    }

    if ((g_low_power_mode != 0) && (g_low_power_mode != 1)) {
        osal_printk("module param g_low_power_mode(%u) should be 0 or 1, it will be set to 1\n", g_low_power_mode);
        g_low_power_mode = 1;
    }

    return;
}

vou_export_symbol_s g_vou_exp_symbol = {0};
vou_export_callback_s g_vo_exp_callback = {0};

vou_export_callback_s *vo_get_exp_callback(hi_void)
{
    return &g_vo_exp_callback;
}

hi_s32 vou_register_extern_call_back(vou_export_callback_s *vo_exp_callback)
{
    if (vo_exp_callback == NULL) {
        vo_err_trace("vo register export callback fail!\n");
        return HI_ERR_VO_NULL_PTR;
    }

    g_vo_exp_callback.pfnVoNotify = vo_exp_callback->pfnVoNotify;
    return HI_SUCCESS;
}

static hi_s32 vo_mod_init_device(hi_void)
{
    g_vo_device = osal_createdev(UMAP_DEVNAME_VO_DEV_BASE);
    if (g_vo_device == HI_NULL) {
        vo_alert_trace("VO create dev failed\n");
        return HI_FAILURE;
    }
    g_vo_device->fops = &g_vo_file_op;
    g_vo_device->osal_pmops = &g_vou_drv_ops;
    g_vo_device->minor = UMAP_VO_DEV_MINOR_BASE;

    return HI_SUCCESS;
}

static hi_void vo_mod_init_global_variable(hi_void)
{
    osal_atomic_set(&g_vou_user_ref, 0);

    (hi_void)memset_s(&g_vou_exp_symbol, sizeof(vou_export_symbol_s), 0, sizeof(vou_export_symbol_s));
    (hi_void)memset_s(&g_vo_exp_callback, sizeof(vou_export_callback_s), 0, sizeof(vou_export_callback_s));

    g_vou_exp_symbol.pfn_vo_register_exp_callback = vou_register_extern_call_back;
}

int vou_dev_module_init(void)
{
    vou_check_module_param();

    vou_sync_resource_init();

    if (vo_mod_init_device() != HI_SUCCESS) {
        goto out0;
    }

    if (osal_registerdevice(g_vo_device) < 0) {
        vo_alert_trace("VO register failed\n");
        goto out1;
    }

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    if (vo_mod_init_proc() != HI_SUCCESS) {
        goto out2;
    }
#endif
    vou_drv_board_init();
    vo_init();

    if (osal_atomic_init(&g_vou_user_ref)) {
        vo_alert_trace("VO mod atomic failed\n");
        goto out3;
    }

    vo_mod_init_global_variable();

#ifdef CONFIG_DRM_HISI_HISMART
    drm_hal_disp_dev_register();
#endif

    HI_PRINT("load vo_dev.ko for %s...OK!\n", CHIP_NAME);
    return HI_SUCCESS;

out3:
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_VO_DEV, HI_NULL);
#endif
out2:
    osal_deregisterdevice(g_vo_device);
out1:
    osal_destroydev(g_vo_device);
    g_vo_device = HI_NULL;
out0:
    vou_sync_resource_de_init();
    HI_PRINT("load vo_dev.ko for %s...FAILURE!\n", CHIP_NAME);
    return HI_FAILURE;
}

void vou_dev_module_exit(void)
{
#ifdef CONFIG_DRM_HISI_HISMART
    drm_hal_disp_dev_unregister();
#endif
    osal_atomic_destroy(&g_vou_user_ref);

    vou_drv_board_de_init();

    vou_exit();

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_VO_DEV, HI_NULL);
#endif
    osal_deregisterdevice(g_vo_device);

    osal_destroydev(g_vo_device);

    vo_drv_set_all_crg_clk(HI_TRUE);

    vo_lpw_bus_reset(HI_TRUE);

    vo_drv_set_all_crg_clk(HI_FALSE);

    vou_sync_resource_de_init();

    (hi_void)memset_s(&g_vou_exp_symbol, sizeof(vou_export_symbol_s), 0, sizeof(vou_export_symbol_s));
    (hi_void)memset_s(&g_vo_exp_callback, sizeof(vou_export_callback_s), 0, sizeof(vou_export_callback_s));

    HI_PRINT("unload vo_dev.ko ....OK!\n");
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

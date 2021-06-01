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

#include "sys_ext.h"
#include "securec.h"
#include "hi_osal.h"
#include "hi_comm_sys.h"
#include "mkp_sys.h"
#include "sys_drv.h"
#include "sys_bind.h"
#include "sys.h"
#include "sys_hal.h"

static sys_context g_sys_ctx = {0};

static hi_bool g_conf = HI_FALSE;
static hi_u64 g_global_pts_base = 0;
static hi_u64 g_local_pts_base = 0;
static hi_u64 g_global_pts_last = 0;
static sys_mem_ctx g_mem_ctx[HI_ID_BUTT] = {0};
static struct osal_list_head g_list_mmz;
static osal_spinlock_t g_sys_spin_lock;

#define sys_spin_lock(flags) osal_spin_lock_irqsave(&g_sys_spin_lock, &(flags))
#define sys_spin_unlock(flags) osal_spin_unlock_irqrestore(&g_sys_spin_lock, &(flags))

#define SYS_STATE_STARTED    0
#define SYS_STATE_STOPPING   1
#define SYS_STATE_STOPPED    2


static hi_u64 sys_get_sched_clock(void)
{
    return osal_sched_clock();
}

/*
 * the return value of sched_clock may loopback after some years.
 * but we should better deal with 'loopback'.
 */
static hi_u64 sys_get_local_cur_pts(void)
{
    static hi_u64 time_last = 0;
    hi_u64 time_now;

    time_now = osal_sched_clock();
    if (time_last > time_now) {
        osal_printk("sys_get_local_cur_pts osal_sched_clock error time_now=%llu,time_last=%llu\n",
                    time_now, time_last);
    }

    time_last = time_now;
    time_now = osal_div_u64(time_now, 1000); /* 1000:unit conversion */

    return time_now;
}

/* sys_get_time_stamp using sample:us */
static hi_u64 sys_get_time_stamp(void)
{
    hi_u64 pts_cur;
    unsigned long flags;

    sys_spin_lock(flags);
    pts_cur = g_global_pts_base + (sys_get_local_cur_pts() - g_local_pts_base);
    /*
     * the pts_cur will never overflow. but g_global_pts_base may be reduced.
     * the pts_cur shouldn't recycle, so we adjust it bigger than pts_last;
     */
    if ((pts_cur < g_global_pts_last) && (g_global_pts_last != 0)) {
        pts_cur = g_global_pts_last + 10; /* 10:ms */
    }

    g_global_pts_last = pts_cur;
    sys_spin_unlock(flags);
    return pts_cur;
}

/*
 * sys_sync_time_stamp using sample:
 * if no media bussines running, we should set init=HI_TRUE.
 * but if media bussines is running, we only should use init=HI_FALSE to
 * gently adjust the PTS.
 * !!!!! if no necessary, don't call it !!!!
 */
static hi_void sys_sync_time_stamp(hi_u64 base, hi_bool init)
{
    unsigned long flags;

    sys_spin_lock(flags);
    g_global_pts_base = base;
    g_local_pts_base = sys_get_local_cur_pts();
    if (init == HI_TRUE) {
        g_global_pts_last = 0;
    }
    sys_spin_unlock(flags);
}

static hi_s32 sys_user_init(hi_void)
{
    unsigned long flags;

    if (g_sys_ctx.state == SYS_STATE_STARTED) {
        sys_info_trace("sys init again!\n");
        return 0;
    }

    if (g_sys_ctx.state == SYS_STATE_STOPPING) {
        sys_err_trace("sys is busy!\n");
        return HI_ERR_SYS_BUSY;
    }

    /* init the PTS system */
    sys_spin_lock(flags);
    g_global_pts_base = sys_get_local_cur_pts();
    g_local_pts_base = g_global_pts_base;
    sys_spin_unlock(flags);

    if (cmpi_init_modules()) {
        sys_err_trace("init modules failed!\n");
        return HI_FAILURE;
    }

    g_sys_ctx.state = SYS_STATE_STARTED;
    sys_debug_trace("sys init ok!\n");

    return HI_SUCCESS;
}

static hi_s32 sys_user_exit(hi_void)
{
    hi_u32 try_times = 10;

    if (g_sys_ctx.state == SYS_STATE_STOPPED) {
        sys_info_trace("sys exit again!\n");
        return HI_SUCCESS;
    }

    /* notify all modules to stop only one time. */
    if (g_sys_ctx.state == SYS_STATE_STARTED) {
        cmpi_stop_modules();
    }

    while (try_times) {
        try_times--;
        if (!cmpi_query_modules()) {
            break;
        }
        osal_msleep(50);    /* 50:time period */
    }

    /* if timeout */
    if (!try_times) {
        g_sys_ctx.state = SYS_STATE_STOPPING;
        return HI_ERR_SYS_BUSY;
    }
    osal_msleep(50);    /* 50:time period */

    cmpi_exit_modules();
    g_sys_ctx.state = SYS_STATE_STOPPED;

    sys_drv_vi_vpss_mode_init();

    g_conf = HI_FALSE;

    sys_debug_trace("sys exit ok!\n");
    return HI_SUCCESS;
}

static hi_void sys_init_audio_mem_cnt(hi_void)
{
    g_mem_ctx[HI_ID_AIO].max_dev_cnt = AIO_MAX_NUM;
    g_mem_ctx[HI_ID_AIO].max_chn_cnt = 1;

    g_mem_ctx[HI_ID_AI].max_dev_cnt = AI_DEV_MAX_NUM;
    g_mem_ctx[HI_ID_AI].max_chn_cnt = AI_MAX_CHN_NUM;

    g_mem_ctx[HI_ID_AO].max_dev_cnt = AO_DEV_MAX_NUM;
    g_mem_ctx[HI_ID_AO].max_chn_cnt = AO_MAX_CHN_NUM;

    g_mem_ctx[HI_ID_AENC].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_AENC].max_chn_cnt = AENC_MAX_CHN_NUM;

    g_mem_ctx[HI_ID_ADEC].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_ADEC].max_chn_cnt = ADEC_MAX_CHN_NUM;
}

static hi_void sys_init_video_mem_cnt(hi_void)
{
#ifdef CONFIG_HI_AVS_SUPPORT
    g_mem_ctx[HI_ID_AVS].max_dev_cnt = AVS_MAX_GRP_NUM;
    g_mem_ctx[HI_ID_AVS].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_VDEC_SUPPORT
    g_mem_ctx[HI_ID_VDEC].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_VDEC].max_chn_cnt = VDEC_MAX_CHN_NUM;
    g_mem_ctx[HI_ID_VFMW].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_VFMW].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_VO_SUPPORT
    g_mem_ctx[HI_ID_VO].max_dev_cnt = VO_MAX_LAYER_NUM;
    g_mem_ctx[HI_ID_VO].max_chn_cnt = VO_MAX_CHN_NUM;
    g_mem_ctx[HI_ID_VO_DEV].max_dev_cnt = VO_MAX_DEV_NUM;
    g_mem_ctx[HI_ID_VO_DEV].max_chn_cnt = VO_MAX_DEV_NUM;
#endif

    g_mem_ctx[HI_ID_VI].max_dev_cnt = VI_MAX_PIPE_NUM;
    g_mem_ctx[HI_ID_VI].max_chn_cnt = VI_MAX_CHN_NUM;

    g_mem_ctx[HI_ID_VPSS].max_dev_cnt = VPSS_MAX_GRP_NUM;
    g_mem_ctx[HI_ID_VPSS].max_chn_cnt = 1;

    g_mem_ctx[HI_ID_VENC].max_dev_cnt = VENC_MAX_RECEIVE_SOURCE;
    g_mem_ctx[HI_ID_VENC].max_chn_cnt = VENC_MAX_CHN_NUM;

    g_mem_ctx[HI_ID_RGN].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_RGN].max_chn_cnt = 1;

#ifdef CONFIG_HI_HIFB_SUPPORT
    g_mem_ctx[HI_ID_FB].max_dev_cnt = VO_MAX_GRAPHICS_LAYER_NUM;
    g_mem_ctx[HI_ID_FB].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_PCIV_SUPPORT
    g_mem_ctx[HI_ID_PCIV].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_PCIV].max_chn_cnt = PCIV_MAX_CHN_NUM;
#endif

#ifdef CONFIG_HI_VGS_SUPPORT
    g_mem_ctx[HI_ID_VGS].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_VGS].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_GDC_SUPPORT
    g_mem_ctx[HI_ID_GDC].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_GDC].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_MONO_COLOR_FUSION_SUPPORT
    g_mem_ctx[HI_ID_MCF].max_dev_cnt = MCF_MAX_GRP_NUM;
    g_mem_ctx[HI_ID_MCF].max_chn_cnt = MCF_MAX_CHN_NUM;
#endif
}

static hi_void sys_init_svp_mem_cnt(hi_void)
{
#ifdef CONFIG_HI_SVP_SUPPORT
    g_mem_ctx[HI_ID_SVP].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_SVP].max_chn_cnt = 1;

#ifdef CONFIG_HI_SVP_DSP
    g_mem_ctx[HI_ID_SVP_DSP].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_SVP_DSP].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_SVP_CNN
    g_mem_ctx[HI_ID_SVP_NNIE].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_SVP_NNIE].max_chn_cnt = 1;
#endif
#endif

#ifdef CONFIG_HI_SVP_DPU_RECT
    g_mem_ctx[HI_ID_DPU_RECT].max_dev_cnt = DPU_RECT_MAX_GRP_NUM;
    g_mem_ctx[HI_ID_DPU_RECT].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_SVP_DPU_MATCH
    g_mem_ctx[HI_ID_DPU_MATCH].max_dev_cnt = DPU_MATCH_MAX_GRP_NUM;
    g_mem_ctx[HI_ID_DPU_MATCH].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_SVP_IVE
    g_mem_ctx[HI_ID_IVE].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_IVE].max_chn_cnt = 1;
#endif

#ifdef CONFIG_HI_SVP_FD
    g_mem_ctx[HI_ID_FD].max_dev_cnt = 1;
    g_mem_ctx[HI_ID_FD].max_chn_cnt = 1;
#endif
}

static hi_s32 sys_malloc_mmz_tbl(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    hi_s32 i;
    hi_u32 size;

    for (i = 0; i < HI_ID_BUTT; i++) {
        if ((g_mem_ctx[i].max_dev_cnt != 0) &&
            (g_mem_ctx[i].max_chn_cnt != 0)) {
            size = sizeof(sys_mod_chn_mmz) * g_mem_ctx[i].max_dev_cnt * g_mem_ctx[i].max_chn_cnt;

            g_mem_ctx[i].mmz_tbl = osal_kmalloc(size, osal_gfp_kernel);
            if (g_mem_ctx[i].mmz_tbl == HI_NULL) {
                sys_err_trace("no memory for mmz_tbl!\n");
                ret = HI_ERR_SYS_NOMEM;
                goto failed;
            }

            (hi_void)memset_s(g_mem_ctx[i].mmz_tbl, size, 0, size);
        }
    }

    OSAL_INIT_LIST_HEAD(&g_list_mmz);
    return ret;

failed:
    for (i = 0; i < HI_ID_BUTT; i++) {
        if ((g_mem_ctx[i].max_dev_cnt != 0) &&
            (g_mem_ctx[i].max_chn_cnt != 0)) {
            if (g_mem_ctx[i].mmz_tbl != HI_NULL) {
                osal_kfree(g_mem_ctx[i].mmz_tbl);
                g_mem_ctx[i].mmz_tbl = HI_NULL;
            }
        }
    }

    return ret;
}

hi_s32 sys_init_mem_conf(hi_void)
{
    (hi_void)memset_s(g_mem_ctx, sizeof(g_mem_ctx), 0, sizeof(g_mem_ctx));

    sys_init_audio_mem_cnt();
    sys_init_video_mem_cnt();
    sys_init_svp_mem_cnt();

    return sys_malloc_mmz_tbl();
}

hi_s32 sys_exit_mem_conf(hi_void)
{
    hi_s32 ret = HI_SUCCESS;
    hi_s32 i;
    sys_mem_node *node = HI_NULL;
    struct osal_list_head *list_tmp = HI_NULL;
    struct osal_list_head *list_node = HI_NULL;

    for (i = 0; i < HI_ID_BUTT; i++) {
        if ((g_mem_ctx[i].max_dev_cnt != 0) &&
            (g_mem_ctx[i].max_chn_cnt != 0)) {
            if (g_mem_ctx[i].mmz_tbl != HI_NULL) {
                osal_kfree(g_mem_ctx[i].mmz_tbl);
                g_mem_ctx[i].mmz_tbl = HI_NULL;
            }
        }
    }

    osal_list_for_each_safe(list_node, list_tmp, &g_list_mmz) {
        node = osal_list_entry(list_node, sys_mem_node, list);
        osal_list_del(list_node);
        osal_vfree(node);
    }

    return ret;
}

static hi_s32 sys_check_mem_chn(hi_mpp_chn *chn)
{
    if (chn->mod_id >= HI_ID_BUTT) {
        sys_err_trace("mod_id:%d is invalid !\n", chn->mod_id);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    if ((chn->dev_id < 0) || (chn->dev_id >= (hi_s32)g_mem_ctx[chn->mod_id].max_dev_cnt) ||
        (chn->chn_id < 0) || (chn->chn_id >= (hi_s32)g_mem_ctx[chn->mod_id].max_chn_cnt)) {
        sys_err_trace("mod_id:%d mod dev or chn is invalid, dev:%d, chn:%d !\n",
            chn->mod_id, chn->dev_id, chn->chn_id);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 sys_mem_get_idx(sys_mem_ctx *mem_ctx, hi_s32 dev_id, hi_s32 chn_id)
{
    return (mem_ctx->max_chn_cnt * dev_id) + chn_id;
}

static hi_s32 sys_get_valid_mmz_tbl(hi_mpp_chn *mpp_chn, sys_mod_chn_mmz **mmz_tbl)
{
    hi_s32 ret;
    hi_u32 index;
    sys_mem_ctx *mem_ctx = HI_NULL;

    ret = sys_check_mem_chn(mpp_chn);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    mem_ctx = &g_mem_ctx[mpp_chn->mod_id];
    index = sys_mem_get_idx(mem_ctx, mpp_chn->dev_id, mpp_chn->chn_id);

    *mmz_tbl = &mem_ctx->mmz_tbl[index];

    return HI_SUCCESS;
}

static hi_s32 sys_set_mem_conf(hi_mpp_chn *mpp_chn, hi_char *pc_mmz_name)
{
    hi_s32 ret;
    hi_bool find;
    sys_mod_chn_mmz *mmz_tbl = HI_NULL;
    sys_mem_node *node = HI_NULL;
    struct osal_list_head *list_tmp = HI_NULL;
    struct osal_list_head *list_node = HI_NULL;

    sys_check_null_ptr_return(mpp_chn);

    ret = sys_get_valid_mmz_tbl(mpp_chn, &mmz_tbl);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    if ((pc_mmz_name == HI_NULL) || (osal_strcmp(pc_mmz_name, "\0") == 0)) {
        mmz_tbl->mmz_name = HI_NULL;
    } else {
        find = HI_FALSE;

        osal_list_for_each_safe(list_node, list_tmp, &g_list_mmz) {
            node = osal_list_entry(list_node, sys_mem_node, list);
            if (!osal_strcmp(pc_mmz_name, node->mmz_name)) {
                find = HI_TRUE;
                mmz_tbl->mmz_name = node->mmz_name;
                break;
            }
        }

        if (!find) {
            node = osal_vmalloc(sizeof(sys_mem_node));
            if (node == HI_NULL) {
                sys_err_trace("no memory for set memconf!\n");
                ret = HI_ERR_SYS_NOMEM;
                return ret;
            }

            (hi_void)memset_s(node->mmz_name, sizeof(node->mmz_name), 0, sizeof(node->mmz_name));
            if (strncpy_s(node->mmz_name, sizeof(node->mmz_name), pc_mmz_name, sizeof(node->mmz_name) - 1) != EOK) {
                osal_vfree(node);
                return HI_ERR_SYS_ILLEGAL_PARAM;
            }
            osal_list_add_tail(&node->list, &g_list_mmz);
            mmz_tbl->mmz_name = node->mmz_name;
        }
    }

    return ret;
}

static hi_s32 sys_get_mem_conf(hi_mpp_chn *mpp_chn, hi_char *pc_mmz_name, hi_u32 mmz_name_len)
{
    hi_s32 ret;
    hi_u32 index;
    sys_mem_ctx *mem_ctx = HI_NULL;
    sys_mod_chn_mmz *mmz_tbl = HI_NULL;

    sys_check_null_ptr_return(mpp_chn);

    ret = sys_check_mem_chn(mpp_chn);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    mem_ctx = &g_mem_ctx[mpp_chn->mod_id];
    index = sys_mem_get_idx(mem_ctx, mpp_chn->dev_id, mpp_chn->chn_id);

    mmz_tbl = &mem_ctx->mmz_tbl[index];
    if (mmz_tbl == HI_NULL) {
        sys_err_trace("mmz_tbl null!\n");
        return HI_ERR_SYS_NULL_PTR;
    }

    if (mmz_tbl->mmz_name == HI_NULL) {
        if (strncpy_s(pc_mmz_name, mmz_name_len, "\0", MAX_MMZ_NAME_LEN - 1) != EOK) {
            return HI_ERR_SYS_ILLEGAL_PARAM;
        }
    } else {
        if (strncpy_s(pc_mmz_name, mmz_name_len, mmz_tbl->mmz_name, MAX_MMZ_NAME_LEN - 1) != EOK) {
            return HI_ERR_SYS_ILLEGAL_PARAM;
        }
    }

    return ret;
}

static hi_s32 sys_get_mmz_name(hi_mpp_chn *chn, hi_void **mmz_name)
{
    hi_s32 ret;
    hi_u32 index;
    sys_mem_ctx *mem_ctx = HI_NULL;
    sys_mod_chn_mmz *mmz_tbl = HI_NULL;

    sys_check_null_ptr_return(chn);
    sys_check_null_ptr_return(mmz_name);

    ret = sys_check_mem_chn(chn);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    mem_ctx = &g_mem_ctx[chn->mod_id];
    index = sys_mem_get_idx(mem_ctx, chn->dev_id, chn->chn_id);

    mmz_tbl = &mem_ctx->mmz_tbl[index];
    if (mmz_tbl == HI_NULL) {
        sys_err_trace("mmz_tbl null!\n");
        return HI_ERR_SYS_NULL_PTR;
    }
    *mmz_name = mmz_tbl->mmz_name;

    return ret;
}

static long sys_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    int ret = 0;
    hi_unused(private_data);

    if (osal_down_interruptible(&g_sys_ctx.sem)) {
        return (-ERESTARTSYS);
    }

    if (cmd == SYS_EXIT_CTRL) {
        ret = sys_user_exit();
    } else {
        osal_atomic_inc_return(&g_sys_ctx.user_ref);

        switch (cmd) {
            case SYS_INIT_CTRL:
                ret = sys_user_init();
                break;

            case SYS_EXIT_CTRL:
                ret = sys_user_exit();
                break;

            case SYS_GET_CURPTS: {
                *((hi_u64 *)(hi_uintptr_t)arg) = sys_get_time_stamp();
                ret = HI_SUCCESS;
                break;
            }

            case SYS_INIT_PTSBASE: {
                sys_sync_time_stamp(*((hi_u64 *)(hi_uintptr_t)arg), HI_TRUE);
                ret = HI_SUCCESS;
                break;
            }

            case SYS_SYNC_PTS: {
                sys_sync_time_stamp(*((hi_u64 *)(hi_uintptr_t)arg), HI_FALSE);
                ret = HI_SUCCESS;
                break;
            }

            case SYS_BIND_CTRL: {
                sys_bind_args *bind_arg = (sys_bind_args *)(hi_uintptr_t)arg;
                ret = sys_bind(&bind_arg->src_chn, &bind_arg->dest_chn);
                break;
            }

            case SYS_UNBIND_CTRL: {
                sys_bind_args *bind_arg = (sys_bind_args *)(hi_uintptr_t)arg;
                ret = sys_unbind(&bind_arg->src_chn, &bind_arg->dest_chn);
                break;
            }

            case SYS_GETBINDBYDEST: {
                sys_bind_args *bind_arg = (sys_bind_args *)(hi_uintptr_t)arg;
                ret = sys_get_bind_by_dest(&bind_arg->dest_chn, &bind_arg->src_chn, HI_FALSE);
                break;
            }

            case SYS_GETBINDBYSRC: {
                sys_bind_src_args *bind_src_arg = (sys_bind_src_args *)(hi_uintptr_t)arg;
                ret = sys_get_bind_by_src(&bind_src_arg->src_chn, &bind_src_arg->dest_chns);
                break;
            }

            case SYS_MEM_SET_CTRL: {
                sys_mem_args *mem_arg = (sys_mem_args *)(hi_uintptr_t)arg;
                ret = sys_set_mem_conf(&mem_arg->mpp_chn, mem_arg->mmz_name);
                break;
            }

            case SYS_MEM_GET_CTRL: {
                sys_mem_args *mem_arg = (sys_mem_args *)(hi_uintptr_t)arg;
                ret = sys_get_mem_conf(&mem_arg->mpp_chn, mem_arg->mmz_name, MAX_MMZ_NAME_LEN);
                break;
            }

            case SYSCONFIG_SET_VI_VPSS_WORK_MODE: {
                hi_s32 mode = *(hi_s32 *)(hi_uintptr_t)arg;
                sysconfig_drv_set_vi_vpss_mode(mode);
                break;
            }

            default:
                ret = HI_FAILURE;
                sys_err_trace("ioctl cmd %x is not supported!\n", cmd);
            }

        osal_atomic_dec_return(&g_sys_ctx.user_ref);
    }

    osal_up(&g_sys_ctx.sem);

    return ret;
}

#ifdef CONFIG_COMPAT
static long sys_compat_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
    switch (cmd) {
        case SYS_MFLUSH_CACHE: {
            sys_mem_cache_info *mem_info = (sys_mem_cache_info *)(hi_uintptr_t)arg;
            COMPAT_POINTER(mem_info->vir_addr, hi_void *);
            break;
        }

        default:
        {
            break;
        }
    }

    return sys_ioctl(cmd, arg, private_data);
}
#endif

static hi_s32 open(void *private_data)
{
    hi_unused(private_data);
    return 0;
}

static hi_s32 close(void *private_data)
{
    hi_unused(private_data);
    return 0;
}

#define SYS_PAGE_SHIFT       12

static int sys_mmap(osal_vm_t *vm, unsigned long start, unsigned long end, unsigned long vm_pgoff, void *private_data)
{
    int size = end - start;
    hi_u64 phy_addr;
    hi_unused(private_data);

    phy_addr = (hi_u64)(vm_pgoff << SYS_PAGE_SHIFT);
    if (cmpi_check_mmz_phy_addr(phy_addr, size) != HI_SUCCESS) {
        sys_err_trace("addr: %#llx, size: %d, invalid phyaddr!\n", phy_addr, size);
        return HI_FAILURE;
    }

    osal_pgprot_cached(vm);
    if (osal_remap_pfn_range(vm, start, vm_pgoff, size)) {
        return HI_FAILURE;
    }

    return 0;
}

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
static hi_void sys_mem_proc_show(osal_proc_entry_t *s)
{
    hi_s32 i;
    hi_u32 j;
    hi_u32 tbl_size;
    sys_mem_ctx *mem_ctx = HI_NULL;

    osal_seq_printf(s, "\n-----MEM TABLE------------------------------------------------------------------\n");
    osal_seq_printf(s, "%8s" "%16s" "%8s" "%8s" "%16s" "\n", "Mod", "ModName", "Dev", "Chn", "MmzName");
    for (i = 0; i < HI_ID_BUTT; i++) {
        mem_ctx = &g_mem_ctx[i];
        if ((mem_ctx->max_dev_cnt == 0) || (mem_ctx->max_chn_cnt == 0)) {
            continue;
        }

        tbl_size = mem_ctx->max_dev_cnt * mem_ctx->max_chn_cnt;

        for (j = 0; j < tbl_size; j++) {
            if (mem_ctx->mmz_tbl[j].mmz_name != HI_NULL) {
                osal_seq_printf(s, "%8d" "%16s" "%8d" "%8d" "%16s" "\n",
                    i, cmpi_get_module_name(i), (j / (mem_ctx->max_chn_cnt)),
                    (j % (mem_ctx->max_chn_cnt)), mem_ctx->mmz_tbl[j].mmz_name);
            }
        }
    }

    return;
}

char *g_sys_state_string[] = { "run", "exiting", "exited" };

static hi_s32 sys_proc_show(osal_proc_entry_t *s)
{
    osal_seq_printf(s, "\n[SYS] Version: [" MPP_VERSION "], Build Time["__DATE__
        ", "__TIME__"]\n\n");
    osal_seq_printf(s, "\n");

    if (g_sys_ctx.state != SYS_STATE_STARTED) {
        return HI_SUCCESS;
    }

    osal_seq_printf(s, "-----MODULE STATUS--------------------------------------------------------------\n");
    osal_seq_printf(s, "  Status\n");
    osal_seq_printf(s, "%8s\n", g_sys_state_string[g_sys_ctx.state]);

    sys_mem_proc_show(s);
    sys_bind_proc_show(s);

    return HI_SUCCESS;
}
#endif

static osal_fileops_t g_sys_file_op = {
    .open = open,
    .unlocked_ioctl = sys_ioctl,
    .release = close,
    .mmap = sys_mmap,

#ifdef CONFIG_COMPAT
    .compat_ioctl = sys_compat_ioctl,
#endif
};

static osal_dev_t *g_sys_device = HI_NULL;

static sys_export_func g_export_funcs = {
    .pfn_sys_get_time_stamp = sys_get_time_stamp,
    .pfn_sys_get_local_time_stamp = sys_get_local_cur_pts,
    .pfn_sys_sync_time_stamp = sys_sync_time_stamp,
    .pfn_sys_drv_ioctrl = sys_drv_drv_ioctrl,
    .pfn_sys_register_sender = sys_bind_register_sender,
    .pfn_sys_unregister_sender = sys_bind_unregister_sender,
    .pfn_sys_register_receiver = sys_bind_register_receiver,
    .pfn_sys_unregister_receiver = sys_bind_unregister_receiver,
    .pfn_sys_send_data = sys_bind_send_data,
    .pfn_sys_reset_data = sys_bind_reset_data,
    .pfn_get_bind_by_src = sys_get_bind_by_src_with_lock,
    .pfn_get_bind_num_by_src = sys_get_bind_num_by_src,
    .pfn_get_bind_by_dest = sys_get_bind_by_dest_inner,
    .pfn_get_mmz_name = sys_get_mmz_name,
    .pfn_sys_get_sched_clock = sys_get_sched_clock,
};

hi_s32 sys_do_init(hi_void *arg)
{
    hi_unused(arg);
    return 0;
}

hi_void sys_do_exit(hi_void)
{
    sys_bind_exit();
    sys_debug_trace("sys mod exit ok!\n");
    return;
}

static hi_void sys_query_state(mod_state *state)
{
    if (osal_atomic_read(&g_sys_ctx.user_ref) == 0) {
        *state = MOD_STATE_FREE;
    } else {
        *state = MOD_STATE_BUSY;
    }
    return;
}

static hi_void sys_notify(mod_notice_id notice)
{
    hi_unused(notice);
    return;
}

static umap_module g_module = {
    .mod_id = HI_ID_SYS,
    .mod_name = "sys",

    .pfn_init = sys_do_init,
    .pfn_exit = sys_do_exit,
    .pfn_query_state = sys_query_state,
    .pfn_notify = sys_notify,
    .pfn_ver_checker = HI_NULL,
    .export_funcs = &g_export_funcs,
    .data = HI_NULL,
};

static hi_s32 sys_freeze(osal_dev_t *dev)
{
    hi_unused(dev);
    return HI_SUCCESS;
}

static hi_s32 sys_restore(osal_dev_t *dev)
{
    hi_unused(dev);
    return HI_SUCCESS;
}

osal_pmops_t g_sys_drv_ops = {
    .pm_freeze = sys_freeze,
    .pm_restore = sys_restore,
};

static hi_s32 sys_lock_init(hi_void)
{
    if (osal_spin_lock_init(&g_sys_spin_lock) < 0) {
        return HI_FAILURE;
    }

    if (osal_atomic_init(&g_sys_ctx.user_ref) < 0) {
        osal_spin_lock_destroy(&g_sys_spin_lock);
        return HI_FAILURE;
    }

    osal_atomic_set(&g_sys_ctx.user_ref, 0);

    if (osal_sema_init(&g_sys_ctx.sem, 1) < 0) {
        osal_atomic_destroy(&g_sys_ctx.user_ref);
        osal_spin_lock_destroy(&g_sys_spin_lock);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void sys_lock_destory(hi_void)
{
    osal_atomic_destroy(&g_sys_ctx.user_ref);
    osal_sema_destroy(&g_sys_ctx.sem);
    osal_spin_lock_destroy(&g_sys_spin_lock);

    return;
}

static inline hi_void sys_device_deinit(hi_void)
{
    osal_deregisterdevice(g_sys_device);
    osal_destroydev(g_sys_device);
    g_sys_device = HI_NULL;
}

static hi_s32 sys_device_init(hi_void)
{
    g_sys_device = osal_createdev(UMAP_DEVNAME_SYSCTL);
    if (g_sys_device == HI_NULL) {
        sys_err_trace("SYS createdev failed!\n");
        return HI_FAILURE;
    }
    g_sys_device->fops = &g_sys_file_op;
    g_sys_device->minor = UMAP_SYS_MINOR_BASE;
    g_sys_device->osal_pmops = &g_sys_drv_ops;
    if (osal_registerdevice(g_sys_device)) {
        sys_err_trace("register system device failed!\n");
        osal_destroydev(g_sys_device);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static hi_void sys_do_mpp_deinit(hi_void)
{
    sys_bind_mod_exit();
    sys_drv_exit();
    sys_exit_mem_conf();
}

static hi_s32 sys_do_mpp_init(hi_void)
{
    if (sys_init_mem_conf() != HI_SUCCESS) {
        sys_err_trace("sys_init_mem_conf failed!\n");
        return HI_FAILURE;
    }

    if (sys_drv_init()) {
        sys_err_trace("sys_drv_init failed!\n");
        goto mem_exit;
    }

    if (sys_bind_mod_init() != HI_SUCCESS) {
        sys_err_trace("sys_bind_mod_init failed!\n");
        goto drv_exit;
    }

    sys_bind_init();

    return HI_SUCCESS;

drv_exit:
    sys_drv_exit();
mem_exit:
    sys_exit_mem_conf();

    return HI_FAILURE;
}

hi_s32 sys_do_mod_init(hi_void)
{
#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_proc_entry_t *proc = HI_NULL;
#endif

    if (sys_lock_init() != HI_SUCCESS) {
        sys_err_trace("SYS lock init failed!\n");
        return HI_FAILURE;
    }

    g_conf = HI_FALSE;
    g_sys_ctx.state = SYS_STATE_STOPPED;
    g_sys_ctx.sys_cfg.align = DEFAULT_ALIGN;

    if (sys_device_init()) {
        goto lock_destroy;
    }

    if (cmpi_register_module(&g_module)) {
        sys_err_trace("register mod failed!\n");
        goto dev_deinit;
    }

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    proc = osal_create_proc_entry(PROC_ENTRY_SYS, HI_NULL);
    if (proc == HI_NULL) {
        sys_err_trace("create proc failed!\n");
        goto mod_unregister;
    }
    proc->read = sys_proc_show;
#endif

    if (sys_do_mpp_init() != HI_SUCCESS) {
        goto proc_remove;
    }

    HI_PRINT("load sys.ko for %s...OK!\n", CHIP_NAME);
    return HI_SUCCESS;
proc_remove:
    osal_remove_proc_entry(PROC_ENTRY_SYS, HI_NULL);
mod_unregister:
    cmpi_unregister_module(HI_ID_SYS);
dev_deinit:
    sys_device_deinit();
lock_destroy:
    sys_lock_destory();
    HI_PRINT("load sys.ko for %s...FAILURE!\n", CHIP_NAME);
    return HI_FAILURE;
}

hi_void sys_do_mod_exit(hi_void)
{
    sys_do_mpp_deinit();

    if (g_sys_ctx.state != SYS_STATE_STOPPED) {
        return;
    }

    cmpi_unregister_module(HI_ID_SYS);

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
    osal_remove_proc_entry(PROC_ENTRY_SYS, HI_NULL);
#endif

    sys_device_deinit();

    sys_lock_destory();

    return;
}

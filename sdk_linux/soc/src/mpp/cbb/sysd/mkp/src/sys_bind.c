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

#include "sys_bind.h"
#include "securec.h"
#include "hi_osal.h"
#include "mm_ext.h"
#include "mod_ext.h"
#include "proc_ext.h"

#include "hi_comm_sys.h"
#include "sys_ext.h"
#include "mkp_sys.h"
#include "sys_drv.h"

#define BIND_NAME_MAX_LEN     16

static osal_spinlock_t g_sys_bind_lock;
static osal_semaphore_t g_sys_dev_sem;

#define bind_spin_lock_init()   osal_spin_lock_init(&g_sys_bind_lock)
#define bind_spin_lock_uninit() osal_spin_lock_destroy(&g_sys_bind_lock)
#define bind_spin_lock(flags) osal_spin_lock_irqsave(&g_sys_bind_lock, &(flags))
#define bind_spin_unlock(flags) osal_spin_unlock_irqrestore(&g_sys_bind_lock, &(flags))

#define sys_lock_may_in_interrupt_return(flags) \
    do {                                        \
        if (osal_in_interrupt()) {              \
            bind_spin_lock(flags);              \
        } else {                                \
            bind_sem_down_return();             \
        }                                       \
    } while (0)

#define sys_unlock_may_in_interrupt_return(flags) \
    do {                                          \
        if (osal_in_interrupt()) {                \
            bind_spin_unlock(flags);              \
        } else {                                  \
            bind_sem_up();                        \
        }                                         \
    } while (0)

static char *bind_get_mod_name(hi_mod_id mod)
{
    char *mod_name = cmpi_get_module_name(mod);
    if (mod_name == HI_NULL) {
        return "unknown mod";
    }

    return mod_name;
}

#define bind_sem_init()         osal_sema_init(&g_sys_dev_sem, 1)
#define bind_sem_uninit()       osal_sema_destroy(&g_sys_dev_sem)

#define bind_sem_down_return()                         \
    do {                                               \
        if (osal_down_interruptible(&g_sys_dev_sem)) { \
            return (-ERESTARTSYS);                     \
        }                                              \
    } while (0)

#define bind_sem_up()   osal_up(&g_sys_dev_sem)

typedef struct {
    struct osal_list_head list;
    hi_mpp_chn mpp_chn;
} sys_bind_node;

typedef struct {
    struct osal_list_head list_head;
    hi_u32 dest_num;
} sys_bind_src;

typedef struct {
    hi_bool is_bind_src;
    hi_mpp_chn bind_src;
    hi_u32 send_cnt;
    hi_u32 reset_cnt;
} sys_bind_dest;

typedef struct {
    hi_mod_id mod_id;
    hi_char name[BIND_NAME_MAX_LEN];
    hi_u32 max_dev_cnt;
    hi_u32 max_chn_cnt;
    hi_s32 (*call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_bool block, mpp_data_type data_type, hi_void *pv_data);
    hi_s32 (*reset_call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_void *v_data);
    hi_s32 (*give_bind_call_back)(hi_s32 dev_id, hi_s32 chn_id, hi_mpp_bind_dest *bind_send);

    hi_u32 tbl_size;
    sys_bind_src *src_tbl;
    sys_bind_dest *dest_tbl;

    hi_bool support_delay_data;
} sys_binder_ctx;

typedef struct {
    hi_u32 max_index;
    hi_mpp_bind_dest **send_bind_src;
} sys_binder_send;

static sys_binder_ctx *g_sender_tbl[HI_ID_BUTT] = {HI_NULL};

static sys_binder_ctx *g_receiver_tbl[HI_ID_BUTT] = {HI_NULL};

static sys_binder_send g_bind_send[HI_ID_BUTT];

static hi_s32 sys_get_idx_by_dev_chn(sys_binder_ctx *binder_ctx, hi_s32 dev_id, hi_s32 chn_id)
{
    return (binder_ctx->max_chn_cnt * dev_id) + chn_id;
}

static hi_void sys_get_dev_chn_by_idx(sys_binder_ctx *binder_ctx, hi_s32 index, hi_s32 *dev_id, hi_s32 *chn_id)
{
    if (binder_ctx->max_chn_cnt != 0) {
        *dev_id = index / binder_ctx->max_chn_cnt;
        *chn_id = index % binder_ctx->max_chn_cnt;
    }

    return;
}

static hi_s32 sys_check_bind_mod_id(hi_mod_id mod_id)
{
    if (mod_id >= HI_ID_BUTT) {
        sys_err_trace("sender/receiver mod_id:%d invalid !\n", mod_id);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 sys_check_bind_dev_chn_id(hi_mpp_chn *bind_chn, sys_binder_ctx *bind_ctx)
{
    if ((bind_chn->dev_id <= -1) ||
        (bind_chn->chn_id <= -1) ||
        ((hi_u32)bind_chn->dev_id >= bind_ctx->max_dev_cnt) ||
        ((hi_u32)bind_chn->chn_id >= bind_ctx->max_chn_cnt)) {
        sys_err_trace("bind dev or chn is invalid, dev:%d, chn:%d !\n",
            bind_chn->dev_id, bind_chn->chn_id);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 sys_check_bind_tbl_size(hi_u32 bind_tbl_size)
{
    if (bind_tbl_size == 0) {
        sys_err_trace("max_dev_cnt or max_chn_cnt invalid !\n");
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 sys_get_bind_valid_send_ctx(hi_mpp_chn *src_chn, hi_mpp_bind_dest **send_bind_src)
{
    hi_u32 src_tbl_idx;
    hi_mpp_bind_dest *send_bind_src_tmp = HI_NULL;

    if (g_sender_tbl[src_chn->mod_id] == HI_NULL) {
        sys_err_trace("mod %d have not register !\n", src_chn->mod_id);
        return HI_ERR_SYS_NOT_PERM;
    }

    if (sys_check_bind_dev_chn_id(src_chn, g_sender_tbl[src_chn->mod_id]) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    src_tbl_idx = sys_get_idx_by_dev_chn(g_sender_tbl[src_chn->mod_id], src_chn->dev_id, src_chn->chn_id);

    send_bind_src_tmp = g_bind_send[src_chn->mod_id].send_bind_src[src_tbl_idx];
    if (send_bind_src_tmp == HI_NULL) {
        sys_info_trace("mod %d, dev %d, chn %d, have not bind !\n", src_chn->mod_id, src_chn->dev_id, src_chn->chn_id);
        return HI_ERR_SYS_NOT_PERM;
    }

    *send_bind_src = send_bind_src_tmp;

    return HI_SUCCESS;
}

static hi_s32 sys_get_bind_valid_dest_tbl(hi_mpp_chn *dest_chn, sys_bind_dest **dest_bind)
{
    sys_binder_ctx *receiver_ctx = HI_NULL;
    hi_u32 dest_tbl_idx;

    if (sys_check_bind_mod_id(dest_chn->mod_id) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    receiver_ctx = g_receiver_tbl[dest_chn->mod_id];
    if (receiver_ctx == HI_NULL) {
        sys_err_trace("mod_id:%d(%s) is not supported !\n", dest_chn->mod_id, bind_get_mod_name(dest_chn->mod_id));
        return HI_ERR_SYS_NULL_PTR;
    }

    if (sys_check_bind_dev_chn_id(dest_chn, receiver_ctx) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    dest_tbl_idx = sys_get_idx_by_dev_chn(receiver_ctx, dest_chn->dev_id, dest_chn->chn_id);
    *dest_bind = &receiver_ctx->dest_tbl[dest_tbl_idx];

    return HI_SUCCESS;
}

static hi_s32 sys_get_bind_valid_src_tbl(hi_mpp_chn *src_chn, sys_bind_src **src_bind)
{
    sys_binder_ctx *sender_ctx = HI_NULL;
    hi_u32 src_tbl_idx;

    if (sys_check_bind_mod_id(src_chn->mod_id) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    sender_ctx = g_sender_tbl[src_chn->mod_id];
    if (sender_ctx == HI_NULL) {
        sys_err_trace("mod_id:%d(%s) is not supported !\n", src_chn->mod_id, bind_get_mod_name(src_chn->mod_id));
        return HI_ERR_SYS_NULL_PTR;
    }

    if (sys_check_bind_dev_chn_id(src_chn, sender_ctx) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    src_tbl_idx = sys_get_idx_by_dev_chn(sender_ctx, src_chn->dev_id, src_chn->chn_id);
    *src_bind = &sender_ctx->src_tbl[src_tbl_idx];

    return HI_SUCCESS;
}

static hi_s32 sys_get_binder_by_src(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_dest)
{
    hi_s32 ret;
    sys_bind_node *node = HI_NULL;
    sys_bind_src *src_bind = HI_NULL;
    struct osal_list_head *list_tmp = HI_NULL;
    struct osal_list_head *list_node = HI_NULL;

    sys_check_null_ptr_return(src_chn);
    sys_check_null_ptr_return(bind_dest);

    ret = sys_get_bind_valid_src_tbl(src_chn, &src_bind);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    bind_dest->num = 0;
    osal_list_for_each_safe(list_node, list_tmp, &src_bind->list_head) {
        node = osal_list_entry(list_node, sys_bind_node, list);
        (hi_void)memcpy_s(&bind_dest->mpp_chn[bind_dest->num], sizeof(hi_mpp_chn),
                          &node->mpp_chn, sizeof(hi_mpp_chn));

        bind_dest->num++;

        if (bind_dest->num >= BIND_DEST_MAXNUM) {
            sys_err_trace("bind src(mod:%d,dev:%d,chn:%d) too many \n",
                src_chn->mod_id, src_chn->dev_id, src_chn->chn_id);
            break;
        }
    }

    return ret;
}

static hi_s32 _get_bind_num_by_src(hi_mpp_chn *src_chn, hi_u32 *bind_num)
{
    hi_s32 ret;
    sys_bind_src *src_bind = HI_NULL;

    sys_check_null_ptr_return(src_chn);

    ret = sys_get_bind_valid_src_tbl(src_chn, &src_bind);
    *bind_num = src_bind->dest_num;

    return ret;
}

hi_s32 sys_get_bind_num_by_src(hi_mpp_chn *src_chn, hi_u32 *bind_num)
{
    hi_s32 ret;
    unsigned long flags;

    bind_spin_lock(flags);
    ret = _get_bind_num_by_src(src_chn, bind_num);
    bind_spin_unlock(flags);

    return ret;
}

static hi_void sys_deinit_sender(sys_binder_ctx *binder_ctx, hi_u32 bind_tbl_size)
{
    hi_u32 i;
    sys_bind_node *node = HI_NULL;
    struct osal_list_head *list_tmp = HI_NULL;
    struct osal_list_head *list_node = HI_NULL;
    sys_bind_src *src_bind = HI_NULL;

    for (i = 0; i < bind_tbl_size; i++) {
        src_bind = &binder_ctx->src_tbl[i];

        osal_list_for_each_safe(list_node, list_tmp, &src_bind->list_head) {
            node = osal_list_entry(list_node, sys_bind_node, list);
            osal_list_del(list_node);
            osal_kfree(node);
            node = HI_NULL;
        }
    }
    osal_kfree(binder_ctx->src_tbl);
    binder_ctx->src_tbl = HI_NULL;
}

static hi_void sys_deinit_receiver(sys_binder_ctx *binder_ctx)
{
    osal_kfree(binder_ctx->dest_tbl);
    binder_ctx->dest_tbl = HI_NULL;
}

static hi_s32 sys_init_sender_ctx(bind_sender_info *sender, hi_mod_id mod_id, sys_binder_ctx *binder_ctx)
{
    hi_u32 i;
    hi_u32 size;

    binder_ctx->src_tbl = (sys_bind_src *)osal_kmalloc(binder_ctx->tbl_size * sizeof(sys_bind_src), osal_gfp_kernel);
    if (binder_ctx->src_tbl == HI_NULL) {
        sys_err_trace("no memory for bind table!\n");
        return HI_ERR_SYS_NOMEM;
    }

    if (g_bind_send[mod_id].send_bind_src == HI_NULL) {
        g_bind_send[mod_id].max_index = sender->max_dev_cnt * sender->max_chn_cnt;
        g_bind_send[mod_id].send_bind_src =
            (hi_mpp_bind_dest **)osal_kmalloc(sizeof(hi_mpp_bind_dest *) * g_bind_send[mod_id].max_index,
                osal_gfp_kernel);
        if (g_bind_send[mod_id].send_bind_src == HI_NULL) {
            sys_err_trace("no memory for bind SRC!\n");
            osal_kfree(binder_ctx->src_tbl);
            binder_ctx->src_tbl = HI_NULL;
            return HI_ERR_SYS_NOMEM;
        }

        size = (sizeof(hi_mpp_bind_dest *) * g_bind_send[mod_id].max_index);
        (hi_void)memset_s(g_bind_send[mod_id].send_bind_src, size, 0, size);
    }

    for (i = 0; i < binder_ctx->tbl_size; i++) {
        OSAL_INIT_LIST_HEAD(&binder_ctx->src_tbl[i].list_head);
        binder_ctx->src_tbl[i].dest_num = 0;
    }

    binder_ctx->mod_id = sender->mod_id;
    binder_ctx->max_dev_cnt = sender->max_dev_cnt;
    binder_ctx->max_chn_cnt = sender->max_chn_cnt;
    binder_ctx->give_bind_call_back = sender->give_bind_call_back;

    return HI_SUCCESS;
}

static hi_s32 sys_init_receiver_ctx(const bind_receiver_info *receiver, sys_binder_ctx *binder_ctx)
{
    hi_u32 i;

    binder_ctx->dest_tbl = (sys_bind_dest *)osal_kmalloc(binder_ctx->tbl_size * sizeof(sys_bind_dest), osal_gfp_kernel);
    if (binder_ctx->dest_tbl == HI_NULL) {
        sys_err_trace("no memory for bind table!\n");
        return HI_ERR_SYS_NOMEM;
    }

    for (i = 0; i < binder_ctx->tbl_size; i++) {
        binder_ctx->dest_tbl[i].is_bind_src = HI_FALSE;
    }

    binder_ctx->mod_id = receiver->mod_id;
    binder_ctx->max_dev_cnt = receiver->max_dev_cnt;
    binder_ctx->max_chn_cnt = receiver->max_chn_cnt;
    binder_ctx->call_back = receiver->call_back;
    binder_ctx->reset_call_back = receiver->reset_call_back;
    binder_ctx->support_delay_data = receiver->support_delay_data;

    return HI_SUCCESS;
}

static inline hi_s32 sys_receiver_is_bind(sys_bind_dest *dest_bind)
{
    return dest_bind->is_bind_src;
}

static hi_s32 sys_bind_check_bindship(hi_mpp_chn *src_chn, sys_bind_dest *dest_bind)
{
    if ((dest_bind->bind_src.mod_id != src_chn->mod_id) ||
        (dest_bind->bind_src.dev_id != src_chn->dev_id) ||
        (dest_bind->bind_src.chn_id != src_chn->chn_id)) {
        sys_err_trace("src and dest is not bound!\n");
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static hi_s32 sys_deinit_send_bind_src_mem(hi_mpp_chn *src_chn)
{
    hi_u32 src_tbl_idx;
    sys_binder_ctx *sender_ctx = HI_NULL;

    sender_ctx = g_sender_tbl[src_chn->mod_id];
    src_tbl_idx = sys_get_idx_by_dev_chn(sender_ctx, src_chn->dev_id, src_chn->chn_id);
    if (g_bind_send[src_chn->mod_id].send_bind_src[src_tbl_idx] != HI_NULL) {
        osal_kfree(g_bind_send[src_chn->mod_id].send_bind_src[src_tbl_idx]);
        g_bind_send[src_chn->mod_id].send_bind_src[src_tbl_idx] = HI_NULL;
    }

    return HI_SUCCESS;
}

static hi_s32 sys_init_send_bind_src_mem(hi_mpp_chn *src_chn)
{
    hi_mpp_bind_dest *send_bind_src = HI_NULL;
    hi_u32 src_tbl_idx;
    sys_binder_ctx *sender_ctx = HI_NULL;

    sender_ctx = g_sender_tbl[src_chn->mod_id];
    src_tbl_idx = sys_get_idx_by_dev_chn(sender_ctx, src_chn->dev_id, src_chn->chn_id);

    send_bind_src = g_bind_send[src_chn->mod_id].send_bind_src[src_tbl_idx];
    if (send_bind_src == HI_NULL) {
        send_bind_src = (hi_mpp_bind_dest *)osal_kmalloc(sizeof(hi_mpp_bind_dest), osal_gfp_kernel);
        if (send_bind_src == HI_NULL) {
            sys_err_trace("no memory for bind SRC!\n");
            return HI_ERR_SYS_NOMEM;
        }

        (hi_void)memset_s(send_bind_src, sizeof(hi_mpp_bind_dest), 0, sizeof(hi_mpp_bind_dest));

        g_bind_send[src_chn->mod_id].send_bind_src[src_tbl_idx] = send_bind_src;
    }

    return HI_SUCCESS;
}

static inline hi_s32 sys_bind_check_sender_dest_num(sys_bind_src *src_bind)
{
    if (src_bind->dest_num > BIND_DEST_MAXNUM) {
        sys_err_trace("src bind max(%d) err!\n", src_bind->dest_num);
        return HI_ERR_SYS_NOT_PERM;
    }

    return HI_SUCCESS;
}

static hi_void sys_give_bind_call_back(hi_mpp_chn *src_chn)
{
    hi_s32 ret;
    sys_binder_ctx *sender_ctx = g_sender_tbl[src_chn->mod_id];

    if ((src_chn->mod_id == HI_ID_VPSS) || (src_chn->mod_id == HI_ID_VI)) {
        static hi_mpp_bind_dest bind_src;
        (hi_void)memset_s(&bind_src, sizeof(hi_mpp_bind_dest), 0, sizeof(hi_mpp_bind_dest));
        ret = sys_get_binder_by_src(src_chn, &bind_src);
        if (ret == HI_SUCCESS) {
            if (sender_ctx->give_bind_call_back != HI_NULL) {
                sender_ctx->give_bind_call_back(src_chn->dev_id, src_chn->chn_id, &bind_src);
            }
        }
    }
}

static inline hi_void sys_bind_reset_callback(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn)
{
    sys_binder_ctx *receiver_ctx = g_receiver_tbl[dest_chn->mod_id];

    if (src_chn->mod_id == HI_ID_VDEC) {
        if (receiver_ctx->reset_call_back != HI_NULL) {
            receiver_ctx->reset_call_back(dest_chn->dev_id, dest_chn->chn_id, HI_NULL);
        }
    }
}

static hi_s32 sys_really_unbind(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn, sys_bind_dest *dest_bind)
{
    unsigned long flags;
    hi_s32 ret;
    sys_bind_src *src_bind = HI_NULL;
    sys_bind_node *node = HI_NULL;
    struct osal_list_head *list_tmp = HI_NULL;
    struct osal_list_head *list_node = HI_NULL;

    ret = sys_get_bind_valid_src_tbl(src_chn, &src_bind);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    bind_spin_lock(flags);

    osal_list_for_each_safe(list_node, list_tmp, &src_bind->list_head) {
        node = osal_list_entry(list_node, sys_bind_node, list);
        if ((dest_chn->mod_id == node->mpp_chn.mod_id) &&
            (dest_chn->dev_id == node->mpp_chn.dev_id) &&
            (dest_chn->chn_id == node->mpp_chn.chn_id)) {
            osal_list_del(list_node);
            osal_kfree(node);
            node = HI_NULL;
            HI_ASSERT(src_bind->dest_num > 0);
            src_bind->dest_num--;
            break;
        }
    }

    dest_bind->is_bind_src = HI_FALSE;

    bind_spin_unlock(flags);

    return HI_SUCCESS;
}

static hi_s32 sys_really_bind(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn, sys_bind_src *src_bind,
    sys_bind_dest *dest_bind)
{
    hi_s32 ret;
    unsigned long flags;
    sys_bind_node *node = HI_NULL;

    ret = sys_init_send_bind_src_mem(src_chn);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    node = (sys_bind_node *)osal_kmalloc(sizeof(sys_bind_node), osal_gfp_kernel);
    if (node == HI_NULL) {
        sys_err_trace("kmalloc err!\n");
        sys_deinit_send_bind_src_mem(src_chn);
        return HI_ERR_SYS_NOMEM;
    }

    bind_spin_lock(flags);

    if (memcpy_s(&dest_bind->bind_src, sizeof(hi_mpp_chn), src_chn, sizeof(hi_mpp_chn)) != EOK) {
        bind_spin_unlock(flags);
        osal_kfree(node);
        node = HI_NULL;
        sys_deinit_send_bind_src_mem(src_chn);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }
    dest_bind->is_bind_src = HI_TRUE;
    dest_bind->send_cnt = 0;
    dest_bind->reset_cnt = 0;

    if (memcpy_s(&node->mpp_chn, sizeof(hi_mpp_chn), dest_chn, sizeof(hi_mpp_chn)) != EOK) {
        bind_spin_unlock(flags);
        osal_kfree(node);
        node = HI_NULL;
        sys_deinit_send_bind_src_mem(src_chn);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    osal_list_add_tail(&node->list, &src_bind->list_head);
    src_bind->dest_num++;

    bind_spin_unlock(flags);
    return HI_SUCCESS;
}

hi_s32 sys_bind(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn)
{
    hi_s32 ret;
    sys_bind_src *src_bind = HI_NULL;
    sys_bind_dest *dest_bind = HI_NULL;

    sys_check_null_ptr_return(src_chn);
    sys_check_null_ptr_return(dest_chn);

    ret = sys_get_bind_valid_src_tbl(src_chn, &src_bind);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    ret = sys_get_bind_valid_dest_tbl(dest_chn, &dest_bind);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    bind_sem_down_return();

    if (sys_receiver_is_bind(dest_bind) == HI_TRUE) {
        sys_err_trace("dest have bind src:(%s,%d,%d) !\n", bind_get_mod_name(dest_bind->bind_src.mod_id),
            dest_bind->bind_src.dev_id, dest_bind->bind_src.chn_id);
        ret = HI_ERR_SYS_NOT_PERM;
        goto exit;
    }

    ret = sys_bind_check_sender_dest_num(src_bind);
    if (ret != HI_SUCCESS) {
        goto exit;
    }

    ret = sys_really_bind(src_chn, dest_chn, src_bind, dest_bind);
    if (ret != HI_SUCCESS) {
        goto exit;
    }

    sys_give_bind_call_back(src_chn);

exit:
    bind_sem_up();
    return ret;
}

hi_s32 sys_unbind(hi_mpp_chn *src_chn, hi_mpp_chn *dest_chn)
{
    hi_s32 ret;
    sys_bind_dest *dest_bind = HI_NULL;

    sys_check_null_ptr_return(src_chn);
    sys_check_null_ptr_return(dest_chn);

    ret = sys_get_bind_valid_dest_tbl(dest_chn, &dest_bind);
    if (ret != HI_SUCCESS) {
        return ret;
    }

    bind_sem_down_return();

    if (sys_receiver_is_bind(dest_bind) == HI_FALSE) {
        sys_warn_trace("src and dest is not bound!\n");
        ret = HI_SUCCESS;
        goto exit;
    }

    ret = sys_bind_check_bindship(src_chn, dest_bind);
    if (ret != HI_SUCCESS) {
        goto exit;
    }

    ret = sys_really_unbind(src_chn, dest_chn, dest_bind);
    if (ret != HI_SUCCESS) {
        goto exit;
    }
    sys_give_bind_call_back(&dest_bind->bind_src);
    sys_bind_reset_callback(src_chn, dest_chn);

exit:
    bind_sem_up();
    return ret;
}

hi_s32 sys_get_bind_by_dest_inner(hi_mpp_chn *dest_chn, hi_mpp_chn *src_chn)
{
    return sys_get_bind_by_dest(dest_chn, src_chn, HI_TRUE);
}

hi_s32 sys_get_bind_by_dest(hi_mpp_chn *dest_chn, hi_mpp_chn *src_chn, hi_bool inner_call)
{
    hi_s32 ret;
    sys_bind_dest *dest_bind = HI_NULL;
    unsigned long flags;

    sys_check_null_ptr_return(src_chn);
    sys_check_null_ptr_return(dest_chn);

    bind_spin_lock(flags);

    ret = sys_get_bind_valid_dest_tbl(dest_chn, &dest_bind);
    if (ret != HI_SUCCESS) {
        bind_spin_unlock(flags);
        return ret;
    }

    if (dest_bind->is_bind_src == HI_FALSE) {
        if (inner_call) {
            sys_warn_trace("dest have not bind any src \n");
        } else {
            sys_err_trace("dest have not bind any src \n");
        }

        bind_spin_unlock(flags);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    if (memcpy_s(src_chn, sizeof(hi_mpp_chn), &dest_bind->bind_src, sizeof(hi_mpp_chn)) != EOK) {
        bind_spin_unlock(flags);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    bind_spin_unlock(flags);

    return HI_SUCCESS;
}

hi_s32 sys_get_bind_by_src(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_dest)
{
    hi_s32 ret;

    bind_sem_down_return();
    ret = sys_get_binder_by_src(src_chn, bind_dest);

    bind_sem_up();

    return ret;
}

hi_s32 sys_get_bind_by_src_with_lock(hi_mpp_chn *src_chn, hi_mpp_bind_dest *bind_dest)
{
    hi_s32 ret;
    unsigned long flags;

    bind_spin_lock(flags);
    ret = sys_get_binder_by_src(src_chn, bind_dest);

    bind_spin_unlock(flags);

    return ret;
}

hi_s32 sys_bind_register_sender(bind_sender_info *sender)
{
    hi_s32 ret;
    hi_mod_id mod_id;
    hi_u32 bind_tbl_size;
    sys_binder_ctx *binder_ctx = HI_NULL;

    sys_check_null_ptr_return(sender);

    mod_id = sender->mod_id;
    bind_tbl_size = sender->max_dev_cnt * sender->max_chn_cnt;

    if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    if (sys_check_bind_tbl_size(bind_tbl_size) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    if (g_sender_tbl[mod_id] != HI_NULL) {
        sys_err_trace("mod %d have register !\n", mod_id);
        return HI_ERR_SYS_NOT_PERM;
    }

    binder_ctx = (sys_binder_ctx *)osal_kmalloc(sizeof(sys_binder_ctx), osal_gfp_kernel);
    if (binder_ctx == HI_NULL) {
        sys_err_trace("malloc binder_ctx null !\n");
        return HI_ERR_SYS_NOMEM;
    }

    ret = snprintf_s(binder_ctx->name, BIND_NAME_MAX_LEN, BIND_NAME_MAX_LEN - 1, "%s", bind_get_mod_name(mod_id));
    if (ret < 0) {
        osal_kfree(binder_ctx);
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }
    binder_ctx->tbl_size = bind_tbl_size;

    ret = sys_init_sender_ctx(sender, mod_id, binder_ctx);
    if (ret != HI_SUCCESS) {
        goto fail;
    }
    g_sender_tbl[mod_id] = binder_ctx;
    return HI_SUCCESS;

fail:
    osal_kfree(binder_ctx);
    return ret;
}

hi_s32 sys_bind_register_receiver(bind_receiver_info *receiver)
{
    hi_s32 ret;
    hi_mod_id mod_id;
    hi_u32 bind_tbl_size;
    sys_binder_ctx *binder_ctx = HI_NULL;

    sys_check_null_ptr_return(receiver);
    sys_check_null_ptr_return(receiver->call_back);

    mod_id = receiver->mod_id;
    bind_tbl_size = receiver->max_dev_cnt * receiver->max_chn_cnt;

    if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    if (sys_check_bind_tbl_size(bind_tbl_size) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    if (g_receiver_tbl[mod_id] != HI_NULL) {
        sys_err_trace("mod %d have register !\n", mod_id);
        return HI_ERR_SYS_NOT_PERM;
    }

    binder_ctx = (sys_binder_ctx *)osal_kmalloc(sizeof(sys_binder_ctx), osal_gfp_kernel);
    if (binder_ctx == HI_NULL) {
        sys_err_trace("malloc binder_ctx null !\n");
        return HI_ERR_SYS_NOMEM;
    }

    ret = snprintf_s(binder_ctx->name, BIND_NAME_MAX_LEN, BIND_NAME_MAX_LEN - 1, "%s", bind_get_mod_name(mod_id));
    if (ret < 0) {
        osal_kfree(binder_ctx);
        binder_ctx = HI_NULL;
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }
    binder_ctx->tbl_size = bind_tbl_size;

    ret = sys_init_receiver_ctx(receiver, binder_ctx);
    if (ret != HI_SUCCESS) {
        goto fail;
    }

    g_receiver_tbl[mod_id] = binder_ctx;
    return HI_SUCCESS;

fail:
    osal_kfree(binder_ctx);
    binder_ctx = HI_NULL;
    return ret;
}

hi_s32 sys_bind_unregister_sender(hi_mod_id mod_id)
{
    unsigned long flags;

    if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    bind_spin_lock(flags);
    if (g_sender_tbl[mod_id] == HI_NULL) {
        sys_err_trace("mod:%d have not register ! \n", mod_id);
        bind_spin_unlock(flags);
        return HI_ERR_SYS_NOT_PERM;
    }

    sys_deinit_sender(g_sender_tbl[mod_id], g_sender_tbl[mod_id]->tbl_size);
    osal_kfree(g_sender_tbl[mod_id]);
    g_sender_tbl[mod_id] = HI_NULL;
    bind_spin_unlock(flags);

    return HI_SUCCESS;
}

hi_s32 sys_bind_unregister_receiver(hi_mod_id mod_id)
{
    unsigned long flags;

    if (sys_check_bind_mod_id(mod_id) != HI_SUCCESS) {
        return HI_ERR_SYS_ILLEGAL_PARAM;
    }

    bind_spin_lock(flags);
    if (g_receiver_tbl[mod_id] == HI_NULL) {
        sys_err_trace("mod:%d have not register ! \n", mod_id);
        bind_spin_unlock(flags);
        return HI_ERR_SYS_NOT_PERM;
    }

    sys_deinit_receiver(g_receiver_tbl[mod_id]);

    osal_kfree(g_receiver_tbl[mod_id]);
    g_receiver_tbl[mod_id] = HI_NULL;
    bind_spin_unlock(flags);

    return HI_SUCCESS;
}

static hi_s32 sys_send_check_reciever_valid(hi_mod_id mod_id, hi_u32 flag)
{
    sys_binder_ctx *receiver = g_receiver_tbl[mod_id];

    if (receiver == HI_NULL) {
        sys_warn_trace("mod %d have not register !\n", mod_id);
        return HI_FAILURE;
    }

    if (receiver->call_back == HI_NULL) {
        return HI_FAILURE;
    }

    if ((((flag & SYS_SEND_DATA_DELAY_MASK) == SYS_SEND_DATA_LOWDELAY) &&
        (!receiver->support_delay_data)) ||
        (((flag & SYS_SEND_DATA_DELAY_MASK) == SYS_SEND_DATA_LOWDELAY_FINISH) &&
        (receiver->support_delay_data))) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 sys_bind_really_send_data(hi_mpp_bind_dest *send_bind_src, hi_u32 flag, mpp_data_type data_type,
    hi_void *v_data)
{
    hi_u32 i;
    unsigned long flags;
    hi_s32 ret = HI_SUCCESS;
    hi_s32 tmp_ret;
    hi_u32 dest_tbl_idx;
    sys_bind_dest *dest_bind = HI_NULL;
    sys_binder_ctx *receiver = HI_NULL;

    if (send_bind_src->num == 0) {
        sys_warn_trace("have not binder !\n");
        return HI_ERR_SYS_NOT_PERM;
    }

    if ((flag & SYS_SEND_DATA_BLOCK_MASK) == SYS_SEND_DATA_NOBLOCK) {
        ret = HI_FAILURE;
    }

    for (i = 0; i < send_bind_src->num; i++) {
        bind_spin_lock(flags);

        HI_ASSERT(send_bind_src->mpp_chn[i].mod_id < HI_ID_BUTT);
        receiver = g_receiver_tbl[send_bind_src->mpp_chn[i].mod_id];

        if (sys_send_check_reciever_valid(send_bind_src->mpp_chn[i].mod_id, flag) != HI_SUCCESS) {
            bind_spin_unlock(flags);
            continue;
        }

        dest_tbl_idx = sys_get_idx_by_dev_chn(receiver, send_bind_src->mpp_chn[i].dev_id,
            send_bind_src->mpp_chn[i].chn_id);
        dest_bind = &receiver->dest_tbl[dest_tbl_idx];
        dest_bind->send_cnt++;

        bind_spin_unlock(flags);
        tmp_ret = receiver->call_back(send_bind_src->mpp_chn[i].dev_id,
            send_bind_src->mpp_chn[i].chn_id, (flag & SYS_SEND_DATA_BLOCK_MASK), data_type, v_data);
        if ((flag & SYS_SEND_DATA_BLOCK_MASK) == SYS_SEND_DATA_NOBLOCK) {
            ret = (hi_u32)ret & (hi_u32)tmp_ret;
        } else {
            ret = (hi_u32)ret | (hi_u32)tmp_ret;
        }
    }

    return ret;
}

hi_s32 sys_bind_send_data(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_u32 flag,
    mpp_data_type data_type, hi_void *v_data)
{
    hi_s32 ret;
    hi_mpp_chn bind_chn;
    hi_mpp_bind_dest *send_bind_src = HI_NULL;
    unsigned long flags;

    HI_ASSERT(mod_id < HI_ID_BUTT);
    HI_ASSERT(data_type < MPP_DATA_BUTT);
    sys_check_null_ptr_return(v_data);

    bind_adjust_src_dev_id(mod_id, dev_id);
    bind_adjust_src_chn_id(mod_id, chn_id);

    bind_spin_lock(flags);

    bind_chn.mod_id = mod_id;
    bind_chn.dev_id = dev_id;
    bind_chn.chn_id = chn_id;
    ret = sys_get_bind_valid_send_ctx(&bind_chn, &send_bind_src);
    if (ret != HI_SUCCESS) {
        bind_spin_unlock(flags);
        return HI_ERR_SYS_NOT_PERM;
    }

    send_bind_src->num = 0;
    ret = sys_get_binder_by_src(&bind_chn, send_bind_src);
    HI_ASSERT(ret == HI_SUCCESS);

    bind_spin_unlock(flags);

    return sys_bind_really_send_data(send_bind_src, flag, data_type, v_data);
}

hi_s32 sys_bind_really_reset_data(hi_mpp_bind_dest *send_bind_src, hi_void *private)
{
    hi_u32 i;
    unsigned long flags;
    sys_bind_dest *dest_bind = HI_NULL;
    sys_binder_ctx *receiver = HI_NULL;

    if (send_bind_src->num == 0) {
        sys_warn_trace("have not binder !\n");
        return HI_SUCCESS;
    }

    for (i = 0; i < send_bind_src->num; i++) {
        sys_lock_may_in_interrupt_return(flags);

        HI_ASSERT(send_bind_src->mpp_chn[i].mod_id < HI_ID_BUTT);
        receiver = g_receiver_tbl[send_bind_src->mpp_chn[i].mod_id];

        if (sys_get_bind_valid_dest_tbl(&send_bind_src->mpp_chn[i], &dest_bind)) {
            sys_unlock_may_in_interrupt_return(flags);
            continue;
        }
        dest_bind->reset_cnt++;

        if (receiver->reset_call_back == HI_NULL) {
            sys_unlock_may_in_interrupt_return(flags);
            continue;
        }

        sys_unlock_may_in_interrupt_return(flags);

        receiver->reset_call_back(send_bind_src->mpp_chn[i].dev_id, send_bind_src->mpp_chn[i].chn_id, private);
    }

    return HI_SUCCESS;
}

hi_s32 sys_bind_reset_data(hi_mod_id mod_id, hi_s32 dev_id, hi_s32 chn_id, hi_void *private)
{
    hi_s32 ret;
    hi_mpp_chn bind_chn;
    hi_mpp_bind_dest *send_bind_src = HI_NULL;
    unsigned long flags;

    HI_ASSERT(mod_id < HI_ID_BUTT);
    sys_check_null_ptr_return(private);

    bind_adjust_src_dev_id(mod_id, dev_id);
    bind_adjust_src_chn_id(mod_id, chn_id);

    sys_lock_may_in_interrupt_return(flags);

    bind_chn.mod_id = mod_id;
    bind_chn.dev_id = dev_id;
    bind_chn.chn_id = chn_id;
    ret = sys_get_bind_valid_send_ctx(&bind_chn, &send_bind_src);
    if (ret != HI_SUCCESS) {
        sys_unlock_may_in_interrupt_return(flags);
        return ret;
    }

    send_bind_src->num = 0;
    ret = sys_get_binder_by_src(&bind_chn, send_bind_src);
    HI_ASSERT(ret == HI_SUCCESS);

    sys_unlock_may_in_interrupt_return(flags);

    return sys_bind_really_reset_data(send_bind_src, private);
}

hi_void sys_proc_show_sec_bindship(osal_proc_entry_t *s, sys_bind_node *node, sys_binder_ctx *sec_sender,
    sys_bind_dest *dest_bind, hi_mpp_chn *fir_mpp_chn)
{
    hi_u32 k;
    hi_s32 sec_index, sec_src_chn;
    struct osal_list_head *list_tmp2 = HI_NULL;
    struct osal_list_head *list_node2 = HI_NULL;
    sys_bind_src *sec_bind_src = HI_NULL;
    sys_bind_node *node2 = HI_NULL;
    hi_bool print_sec = HI_FALSE;

    for (k = 0; k < sec_sender->max_chn_cnt; k++) {
        sec_src_chn = k;
        sec_index = k + sec_sender->max_chn_cnt * node->mpp_chn.dev_id;
        sec_bind_src = &sec_sender->src_tbl[sec_index];

        bind_adjust_dest_chn_id(node->mpp_chn.mod_id, sec_src_chn);

        if ((node->mpp_chn.chn_id == sec_src_chn) &&
            (!osal_list_empty(&sec_bind_src->list_head))) {
            osal_list_for_each_safe(list_node2, list_tmp2, &sec_bind_src->list_head) {
                node2 = osal_list_entry(list_node2, sys_bind_node, list);
                osal_seq_printf(s, "%8s" "%8d" "%8d" "%8s" "%8d" "%8u" "%8s" "%8d" "%8d" "%11d" "%11u" "\n",
                    bind_get_mod_name(fir_mpp_chn->mod_id), fir_mpp_chn->dev_id, fir_mpp_chn->chn_id,
                    bind_get_mod_name(node->mpp_chn.mod_id), node->mpp_chn.dev_id, k,
                    bind_get_mod_name(node2->mpp_chn.mod_id), node2->mpp_chn.dev_id, node2->mpp_chn.chn_id,
                    dest_bind->send_cnt, dest_bind->reset_cnt);
            }
            print_sec = HI_TRUE;
        }
    }

    if (print_sec == HI_FALSE) {
        osal_seq_printf(s, "%8s" "%8d" "%8d" "%8s" "%8d" "%8d" "%8s" "%8d" "%8d" "%11d" "%11u" "\n",
            bind_get_mod_name(fir_mpp_chn->mod_id), fir_mpp_chn->dev_id, fir_mpp_chn->chn_id,
            bind_get_mod_name(node->mpp_chn.mod_id), node->mpp_chn.dev_id, node->mpp_chn.chn_id,
            "null", 0, 0, dest_bind->send_cnt, dest_bind->reset_cnt);
    }
}

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
hi_void sys_bind_proc_show(osal_proc_entry_t *s)
{
    hi_u32 i, j;
    hi_mpp_chn fir_mpp_chn;
    hi_u32 dest_tbl_idx;
    sys_binder_ctx *fir_sender = HI_NULL;
    sys_binder_ctx *sec_sender = HI_NULL;
    sys_binder_ctx *receiver = HI_NULL;
    sys_bind_dest *dest_bind = HI_NULL;
    sys_bind_src *fir_bind_src = HI_NULL;
    struct osal_list_head *list_tmp = HI_NULL;
    struct osal_list_head *list_node = HI_NULL;
    sys_bind_node *node = HI_NULL;
    unsigned long flags;

    osal_seq_printf(s, "\n-----BIND RELATION TABLE--------------------------------------------------------\n");
    osal_seq_printf(s, "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%11s" "%11s" "\n",
        "FirMod", "FirDev", "FirChn", "SecMod", "SecDev", "SecChn", "TirMod", "TirDev", "TirChn", "SendCnt", "rstCnt");

    bind_spin_lock(flags);

    for (i = 0; i < HI_ID_BUTT; i++) {
        fir_sender = g_sender_tbl[i];
        if (fir_sender == HI_NULL) {
            continue;
        }

        fir_mpp_chn.mod_id = i;
        for (j = 0; j < fir_sender->tbl_size; j++) {
            fir_bind_src = &fir_sender->src_tbl[j];

            sys_get_dev_chn_by_idx(fir_sender, j, &fir_mpp_chn.dev_id, &fir_mpp_chn.chn_id);

            osal_list_for_each_safe(list_node, list_tmp, &fir_bind_src->list_head) {
                node = osal_list_entry(list_node, sys_bind_node, list);

                receiver = g_receiver_tbl[node->mpp_chn.mod_id];
                if (receiver == HI_NULL) {
                    continue;
                }
                dest_tbl_idx = sys_get_idx_by_dev_chn(receiver, node->mpp_chn.dev_id, node->mpp_chn.chn_id);
                dest_bind = &receiver->dest_tbl[dest_tbl_idx];

                sec_sender = g_sender_tbl[node->mpp_chn.mod_id];
                if (sec_sender == HI_NULL) {
                    osal_seq_printf(s, "%8s" "%8d" "%8d" "%8s" "%8d" "%8d" "%8s" "%8d" "%8d" "%11d" "%11u" "\n",
                        bind_get_mod_name(i), fir_mpp_chn.dev_id, fir_mpp_chn.chn_id,
                        bind_get_mod_name(node->mpp_chn.mod_id), node->mpp_chn.dev_id, node->mpp_chn.chn_id,
                        "null", 0, 0, dest_bind->send_cnt, dest_bind->reset_cnt);
                    continue;
                }

                sys_proc_show_sec_bindship(s, node, sec_sender, dest_bind, &fir_mpp_chn);
            }
        }
    }
    bind_spin_unlock(flags);
}
#endif

hi_s32 sys_bind_mod_init(hi_void)
{
    if (bind_spin_lock_init() < 0) {
        return HI_FAILURE;
    }

    if (bind_sem_init() < 0) {
        bind_spin_lock_uninit();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_s32 sys_bind_init(hi_void)
{
    (hi_void)memset_s(g_sender_tbl, sizeof(g_sender_tbl), 0, sizeof(g_sender_tbl));
    (hi_void)memset_s(g_receiver_tbl, sizeof(g_receiver_tbl), 0, sizeof(g_receiver_tbl));
    (hi_void)memset_s(g_bind_send, sizeof(g_bind_send), 0, sizeof(g_bind_send));
    return HI_SUCCESS;
}

hi_void sys_bind_mod_exit(hi_void)
{
    bind_spin_lock_uninit();
    bind_sem_uninit();

    return;
}

static hi_void sys_free_mod_send_src_bind_mem(MOD_ID_E mod)
{
    hi_u32 j;

    for (j = 0; j < g_bind_send[mod].max_index; j++) {
        if (g_bind_send[mod].send_bind_src[j] != HI_NULL) {
            osal_kfree(g_bind_send[mod].send_bind_src[j]);
            g_bind_send[mod].send_bind_src[j] = HI_NULL;
        }
    }
}

hi_void sys_bind_exit(hi_void)
{
    hi_s32 i;

    for (i = 0; i < HI_ID_BUTT; i++) {
        if (g_bind_send[i].send_bind_src != HI_NULL) {
            sys_free_mod_send_src_bind_mem(i);
            osal_kfree(g_bind_send[i].send_bind_src);
            g_bind_send[i].send_bind_src = HI_NULL;
        }
    }
}

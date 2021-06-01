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

#include "tde_osilist.h"
#ifdef HI_BUILD_IN_BOOT
#include "exports.h"
#endif
#ifdef HI_BUILD_IN_MINI_BOOT
#include "delay.h"
#endif
#include "tde_handle.h"
#include "tde_buffer.h"
#include "tde_hal.h"
#include "wmalloc.h"
#include "tde_handle.h"

/* JOB LIST head node definition */
typedef struct {
    struct osal_list_head list;
    hi_s32 handle_last;     /* job handle wait for last submit */
    hi_s32 handle_finished; /* job handle last completed */
    hi_u32 job_num;         /* job number in queue */
#ifndef HI_BUILD_IN_BOOT
    osal_spinlock_t lock;
#endif
    tde_swjob *job_committed; /* last submitted job node pointer, which is the first job */
    tde_swjob *job_to_commit;  /* job node pointer wait for submit,which is the first job */
    hi_u32 job_finished;
    tde_swjob *job_last; /* last job in the list */
} tde_swjoblist;

#ifndef HI_BUILD_IN_BOOT
typedef hi_void (*TDE_WQ_CB)(hi_u32);

typedef struct {
    hi_u32 count;
    TDE_WQ_CB wqcb;
    struct osal_work_struct work;
} tdefreewq;

static osal_wait_t g_tde_block_job_wq; /* wait queue used to block */
#endif
static tde_swjoblist *g_tde_osi_job_list;

#ifdef TDE_HWC_COOPERATE
static hi_bool g_working_flag = HI_FALSE;
osal_spinlock_t g_working_flag_lock;
#endif

static hi_void tde_osi_list_do_free_phy_buff(hi_u32 buff_num);
static hi_void tde_osi_list_safe_destroy_job(tde_swjob *job);
static hi_void tde_osi_list_release_handle(hi_handle_mgr *job_header);
static hi_void tde_osi_list_add_job(tde_swjob *job);
static hi_void tde_osi_list_destroy_job(tde_swjob *job);

static hi_void tde_osi_list_safe_destroy_job(tde_swjob *job)
{
    if (job == HI_NULL) {
        return;
    }
#ifndef HI_BUILD_IN_BOOT
    /* if user query this job, release job in query function */
    if (job->wait_for_done_count != 0) {
        tde_info("query handle %d complete!\n", job->handle);
        job->noti_type = TDE_JOB_NOTIFY_BUTT;
        osal_wakeup(&job->query);
        return;
    }
#endif

    tde_osi_list_destroy_job(job);

    return;
}

static hi_void tde_osi_list_release_handle(hi_handle_mgr *job_header)
{
    if (job_header == HI_NULL) {
        tde_error("pstJobHeader is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    if (job_header != HI_NULL) {
        if (tde_release_handle(job_header->handle)) {
            tde_free(job_header);
        }
    }
    return;
}

static hi_void tde_osi_list_do_free_phy_buff(hi_u32 buff_num)
{
    hi_u32 i;
    for (i = 0; i < buff_num; i++) {
        tde_free_physic_buff();
    }
    return;
}

hi_s32 tde_osi_list_init(hi_void)
{
#ifndef HI_BUILD_IN_BOOT
    if (osal_wait_init(&g_tde_block_job_wq) != 0) {
        return HI_FAILURE;
    }
#endif
    if (!tde_initial_handle()) {
#ifndef HI_BUILD_IN_BOOT
        osal_wait_destroy(&g_tde_block_job_wq);
#endif
        return HI_FAILURE;
    }

    g_tde_osi_job_list = (tde_swjoblist *)tde_malloc(sizeof(tde_swjoblist));
    if (g_tde_osi_job_list == HI_NULL) {
#ifndef HI_BUILD_IN_BOOT
        osal_wait_destroy(&g_tde_block_job_wq);
#endif
        tde_destroy_handle();
        return HI_FAILURE;
    }

    OSAL_INIT_LIST_HEAD(&g_tde_osi_job_list->list);
#ifndef HI_BUILD_IN_BOOT
    osal_spin_lock_init(&g_tde_osi_job_list->lock);
    osal_spin_lock_init(&g_tde_buff_lock);

#ifdef TDE_HWC_COOPERATE
    osal_spin_lock_init(&g_working_flag_lock);
#endif
#endif

    return HI_SUCCESS;
}

static hi_void tde_osi_list_free_node(tde_swnode *node)
{
    if (node == HI_NULL) {
        tde_error("pNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    osal_list_del_init(&node->list);
    if (node->node_buf.buf != HI_NULL) {
        tde_free(node->node_buf.buf);
    }

    tde_osi_list_put_phy_buff(node->phy_buff_num);

    tde_free(node);
}

/*
 * Function:      tde_osi_list_free_serial_cmd
 * Description:   release from FstCmd to LastCmd
 * Input:         fst_cmd: first node
 *                last_cmd:last node
 */
hi_void tde_osi_list_free_serial_cmd(tde_swnode *fst_cmd, tde_swnode *last_cmd)
{
    tde_swnode *next_cmd = HI_NULL;
    tde_swnode *cur_cmd = HI_NULL;

    if ((fst_cmd == HI_NULL) || (last_cmd == HI_NULL)) {
        return;
    }

    cur_cmd = next_cmd = fst_cmd;
    while (next_cmd != last_cmd) {
        next_cmd = osal_list_entry(cur_cmd->list.next, tde_swnode, list);
        if (next_cmd == HI_NULL) {
            return;
        }
        tde_osi_list_free_node(cur_cmd);
        cur_cmd = next_cmd;
    }

    tde_osi_list_free_node(last_cmd);
    return;
}

#ifndef HI_BUILD_IN_BOOT
hi_void tde_osi_list_term(hi_void)
{
    tde_swjob *job = HI_NULL;

#ifndef HI_BUILD_IN_BOOT
    osal_wait_destroy(&g_tde_block_job_wq);
#endif
    osal_spin_lock_destroy(&g_tde_buff_lock);
#ifdef TDE_HWC_COOPERATE
    osal_spin_lock_destroy(&g_working_flag_lock);
#endif
    if (g_tde_osi_job_list == HI_NULL) {
        return;
    }

    while (!osal_list_empty(&g_tde_osi_job_list->list)) {
        job = osal_list_entry(g_tde_osi_job_list->list.next, tde_swjob, list);
        osal_list_del_init(&job->list);
        tde_osi_list_destroy_job(job);
    }
    osal_spin_lock_destroy(&g_tde_osi_job_list->lock);

    tde_free(g_tde_osi_job_list);
    g_tde_osi_job_list = HI_NULL;

    tde_destroy_handle();
    return;
}

hi_void tde_osi_list_flush_node(tde_swnode *node)
{
    tde_node_buf *node_buf_info = HI_NULL;
    hi_u32 cur_node_phy; /* The current node corresponding physical addresses */

    if (node == HI_NULL) {
        tde_error("pNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    node_buf_info = &node->node_buf;

    if (node_buf_info->buf != HI_NULL) {
        cur_node_phy = wgetphy(node_buf_info->buf);
        tde_flush_cached_mmb((hi_u8 *)node_buf_info->buf, cur_node_phy,
                             node_buf_info->node_sz + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE);
    }
    return;
}

hi_void tde_osi_list_flush_job(tde_swjob *job)
{
    tde_swnode *next_cmd = HI_NULL;
    tde_swnode *cur_cmd = HI_NULL;
    tde_swnode *fst_cmd = HI_NULL;
    tde_swnode *last_cmd = HI_NULL;

    if (job == HI_NULL) {
        tde_error("pstJob is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    fst_cmd = job->first_cmd;
    last_cmd = job->tail_node;

    if ((fst_cmd == HI_NULL) || (last_cmd == HI_NULL)) {
        return;
    }

    cur_cmd = next_cmd = fst_cmd;
    while (next_cmd != last_cmd) {
        next_cmd = osal_list_entry(cur_cmd->list.next, tde_swnode, list);
        tde_osi_list_flush_node(cur_cmd);
        cur_cmd = next_cmd;
    }

    tde_osi_list_flush_node(last_cmd);
    return;
}
#endif

static hi_void tde_osi_list_add_job(tde_swjob *job)
{
    osal_list_add_tail(&job->list, &g_tde_osi_job_list->list);
    g_tde_osi_job_list->job_num++;
    g_tde_osi_job_list->handle_last = job->handle;
    g_tde_osi_job_list->job_last = job;
}

hi_s32 tde_osi_list_begin_job(hi_s32 *handle, hi_void *private_data)
{
    hi_handle_mgr *handle_mgr = HI_NULL;
    tde_swjob *job = HI_NULL;
    hi_s32 ret;

    if (handle == HI_NULL) {
        return HI_ERR_TDE_NULL_PTR;
    }
    handle_mgr = (hi_handle_mgr *)tde_malloc(sizeof(hi_handle_mgr) + sizeof(tde_swjob));
    if (handle_mgr == HI_NULL) {
        tde_error("TDE BegJob Malloc Fail!\n");
        return HI_ERR_TDE_NO_MEM;
    }
    job = (tde_swjob *)((hi_u8 *)handle_mgr + sizeof(hi_handle_mgr));
    handle_mgr->res = (hi_void *)job;
    tde_get_handle(handle_mgr, handle);
#ifndef HI_BUILD_IN_BOOT
    ret = osal_wait_init(&job->query);
    if (ret != 0) {
        tde_error("osal_wait_init Fail!\n");
        tde_osi_list_release_handle(handle_mgr);
        return HI_ERR_TDE_NO_MEM;
    }
#endif
    OSAL_INIT_LIST_HEAD(&job->list);
    job->handle = *handle;
    if (private_data != HI_NULL) {
        job->private_data = private_data;
    }
    return HI_SUCCESS;
}

static hi_void tde_osi_list_destroy_job(tde_swjob *job)
{
    hi_handle_mgr *handle_mgr = HI_NULL;

#ifdef CONFIG_GFX_MMU_SUPPORT
    tde_hw_node *hw_node = HI_NULL;
#endif

    if (!tde_query_handle(job->handle, &handle_mgr)) {
        tde_info("handle %d does not exist!\n", job->handle);
        return;
    }

#ifdef CONFIG_GFX_MMU_SUPPORT
    hw_node = (tde_hw_node *)(job->first_cmd->node_buf.buf + TDE_NODE_HEAD_BYTE);
    tde_hal_free_tmp_buf(hw_node);
#endif

    tde_osi_list_free_serial_cmd(job->first_cmd, job->tail_node);

#ifndef HI_BUILD_IN_BOOT
    osal_wait_destroy(&job->query);
#endif

    tde_osi_list_release_handle(handle_mgr);

    return;
}

#ifndef HI_BUILD_IN_BOOT
hi_s32 tde_osi_list_cancel_job(hi_s32 handle)
{
    hi_handle_mgr *handle_mgr = HI_NULL;
    tde_swjob *job = HI_NULL;
#ifndef HI_BUILD_IN_BOOT
    unsigned long lockflags;
#endif

    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    if (!tde_query_handle(handle, &handle_mgr)) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return HI_SUCCESS;
    }
    job = (tde_swjob *)handle_mgr->res;

    if (job->submitted) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        tde_error("Handle %d has been submitted!\n", handle);
        return HI_FAILURE;
    }

    tde_osi_list_free_serial_cmd(job->first_cmd, job->tail_node);

#ifndef HI_BUILD_IN_BOOT
    osal_wait_destroy(&job->query);
#endif

    tde_osi_list_release_handle(handle_mgr);
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    return HI_SUCCESS;
}
#else
static hi_void tde_osi_list_wait_tde_idle(hi_void)
{
    while (1) {
        udelay(10 * 1000); /* delay 10*1000 ms */
        if (tde_hal_ctl_is_idle_safely()) {
            return;
        }
    }
}
#endif

hi_s32 tdeosi_wait_call_back(const hi_void *param)
{
    tde_swjob *job = HI_NULL;

    if (param == HI_NULL) {
        return HI_FAILURE;
    }
    job = (tde_swjob *)param;
    return (job->noti_type == TDE_JOB_NOTIFY_BUTT);
}

static hi_void tde_job_to_commit(tde_swjob *job, tde_func_cb func_compl_cb, hi_void *func_para,
                                 tde_notify_mode noti_type)
{
    hi_u8 *next_node_buf = HI_NULL;

    job->submitted = HI_TRUE;
    job->noti_type = noti_type;
    job->func_compl_cb = func_compl_cb;
    job->func_para = func_para;
    /* If the job to commit is not null,join the current job to the tail node of the last job. */
    if (g_tde_osi_job_list->job_to_commit != HI_NULL) {
        tde_swnode *tail_node_in_job_list = g_tde_osi_job_list->job_last->tail_node;
        next_node_buf = (hi_u8 *)tail_node_in_job_list->node_buf.buf + TDE_NODE_HEAD_BYTE;
        tde_hal_next_node_addr((hi_void *)next_node_buf, job->first_cmd->node_buf.phy_addr);
        if (job->aq_use_buff) {
            g_tde_osi_job_list->job_to_commit->aq_use_buff = HI_TRUE;
        }
#ifdef TDE_CACH_STRATEGY
        /* Flush all hw will be the job of the corresponding node into memory, ensure correct hardware can access */
        tde_osi_list_flush_job(g_tde_osi_job_list->job_last);
#endif
    } else {
        g_tde_osi_job_list->job_to_commit = job;
    }

#ifdef TDE_CACH_STRATEGY
    /* Flush all hw will be the job of the corresponding node into memory, ensure correct hardware can access */
    tde_osi_list_flush_job(job);
#endif
    tde_osi_list_add_job(job);
    return;
}

#if (TDE_CAPABILITY & SYNC)
static hi_s32 tde_list_set_job(tde_swjob **job, hi_s32 handle, hi_bool is_sync)
#else
static hi_s32 tde_list_set_job(tde_swjob **job, hi_s32 handle)
#endif
{
    hi_bool valid = HI_FALSE;
    hi_handle_mgr *handle_mgr = HI_NULL;
    tde_swnode *tail_node = HI_NULL;
    hi_u8 *buf = HI_NULL;

    valid = tde_query_handle(handle, &handle_mgr);
    if (!valid) {
        tde_error("invalid handle %d!\n", handle);
        return HI_ERR_TDE_INVALID_HANDLE;
    }

    *job = (tde_swjob *)handle_mgr->res;
    if ((*job)->submitted) {
        tde_error("job %d already submitted!\n", handle);
        return HI_ERR_TDE_INVALID_HANDLE;
    }
    if ((*job)->first_cmd == HI_NULL) {
        tde_info("no cmd !\n");

#ifndef HI_BUILD_IN_BOOT
        osal_wait_destroy(&(*job)->query);
#endif
        tde_osi_list_release_handle(handle_mgr);
        return HI_FAILURE;
    }
    tail_node = (*job)->tail_node;

    buf = (hi_u8 *)tail_node->node_buf.buf + TDE_NODE_HEAD_BYTE;
    tde_hal_node_enable_complete_int((hi_void *)buf);

#if (TDE_CAPABILITY & SYNC)
    if (is_sync) {
        tde_hal_node_enable_sync((hi_void *)buf);
    }
#endif
    return HI_SUCCESS;
}

static hi_void tde_list_submit_first_cmd(hi_void)
{
    hi_s32 ret;
#ifdef TDE_HWC_COOPERATE
    unsigned long lock;
#endif

#ifdef TDE_HWC_COOPERATE
    tde_osi_list_lock_working_flag(&lock);
    if (!g_working_flag) {
#endif
        ret = tde_hal_node_execute(g_tde_osi_job_list->job_to_commit->first_cmd->node_buf.phy_addr,
                                   g_tde_osi_job_list->job_to_commit->first_cmd->node_buf.update,
                                   g_tde_osi_job_list->job_to_commit->aq_use_buff);
        if (ret == HI_SUCCESS) {
            g_tde_osi_job_list->job_committed = g_tde_osi_job_list->job_to_commit;
            g_tde_osi_job_list->job_to_commit = HI_NULL;
            g_tde_osi_job_list->job_finished = 0x0;
#ifdef TDE_HWC_COOPERATE
            g_working_flag = 1;
#endif
        }
#ifdef TDE_HWC_COOPERATE
    }
#endif

#ifdef TDE_HWC_COOPERATE
    tde_osi_list_unlock_working_flag(&lock);
#endif
    return;
}

static hi_s32 tde_osi_list_event(tde_swjob *job, tde_notify_mode noti_type, hi_bool asynflag, hi_u32 time_out)
{
    unsigned long lockflags;
    hi_s32 ret;
    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    if (noti_type == TDE_JOB_WAKE_NOTIFY) {
        job->wait_for_done_count++;
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        ret = osal_wait_event_timeout_uninterruptible(&g_tde_block_job_wq, tdeosi_wait_call_back, (hi_void *)job,
                                                      time_out);
        tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
        job->wait_for_done_count--;
        if (job->noti_type == TDE_JOB_NOTIFY_BUTT) {
            tde_info("handle:%d complete!\n", job->handle);
            if (job->wait_for_done_count == 0) {
                tde_osi_list_destroy_job(job);
            }
            tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
            return -1;
        } else {
            job->noti_type = TDE_JOB_COMPL_NOTIFY;
            tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
            if (ret == (-ERESTARTSYS)) {
                tde_error("handle:%d interrupt!\n", job->handle);
                return HI_ERR_TDE_INTERRUPT;
            }
            if (asynflag == 1) {
                /* If from a non-blocking way to block the way, no overtime information */
                return -1;
            }
            tde_error("handle:%d timeout!\n", job->handle);
            return HI_ERR_TDE_JOB_TIMEOUT;
        }
    }
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    return HI_SUCCESS;
}

static hi_void tde_set_job_wake_notify(tde_swjob *job, tde_notify_mode *noti_type, hi_u32 *time_out,
                                       hi_bool *asynflag)
{
#ifndef HI_BUILD_IN_BOOT
    if (*noti_type != TDE_JOB_WAKE_NOTIFY) {
        if ((!osal_in_interrupt()) && (wgetfreenum() < 5)) { /* 5 num */
            job->noti_type = TDE_JOB_WAKE_NOTIFY;
            *noti_type = TDE_JOB_WAKE_NOTIFY;
            *time_out = 1000; /* timeout 1000ms */
            *asynflag = 1; /* By sign non-blocking way to block the way */
        }
    }
#endif
    return;
}

#ifdef HI_BUILD_IN_BOOT
static hi_void tde_osi_idle(tde_swjob *job)
{
    /* Waiting for the TDE task to complete */
    tde_osi_list_wait_tde_idle();
    /* Deleted from the list the job node, prevent again traverse to the node */
    osal_list_del_init(&job->list);
    /* release job */
    tde_osi_list_safe_destroy_job(job);

    tde_info("handle:%d complete!\n", job->handle);
}
#endif

#if (TDE_CAPABILITY & SYNC)
hi_s32 tde_osi_list_submit_job(drv_tde_end_job_cmd *end_job, tde_func_cb func_compl_cb, hi_void *func_para,
    tde_notify_mode noti_type, hi_bool is_sync)
#else
hi_s32 tde_osi_list_submit_job(drv_tde_end_job_cmd *end_job, tde_func_cb func_compl_cb, hi_void *func_para,
    tde_notify_mode noti_type)
#endif
{
    tde_swjob *job = HI_NULL;
    hi_s32 ret;
    hi_s32 handle = end_job->handle;
    hi_u32 time_out = end_job->time_out;
#ifndef HI_BUILD_IN_BOOT
    unsigned long lockflags;
    hi_bool asynflag = HI_FALSE;
#endif

    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
#if (TDE_CAPABILITY & SYNC)
    ret = tde_list_set_job(&job, handle, is_sync);
    if (ret != HI_SUCCESS) {
#else
    ret = tde_list_set_job(&job, handle);
    if (ret != HI_SUCCESS) {
#endif
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        if (ret == -1) {
            return HI_SUCCESS;
        }
        return ret;
    }
    tde_job_to_commit(job, func_compl_cb, func_para, noti_type);
    tde_set_job_wake_notify(job, &noti_type, &time_out, &asynflag);
    tde_list_submit_first_cmd();

#ifndef HI_BUILD_IN_BOOT
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    ret = tde_osi_list_event(job, noti_type, asynflag, time_out);
    if (ret != HI_SUCCESS) {
        if (ret == -1) {
            return HI_SUCCESS;
        }
        return ret;
    }
#else
    tde_osi_idle(job);
#endif
    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
/*
 * Prototype    : tde_osi_list_wait_all_done
 * Description  : wait for all TDE operate is completed,that is to say wait for the last job to be completed.
 * Return Value : HI_SUCCESS,TDE operate completed
 */
hi_s32 tde_osi_list_wait_all_done(hi_void)
{
    hi_s32 wait_handle;
    unsigned long lockflags;

    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    wait_handle = g_tde_osi_job_list->handle_last;
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    if (wait_handle == -1) {
        return HI_SUCCESS;
    }
    return tde_osi_list_wait_for_done(wait_handle, 0);
}

static hi_s32 tde_osi_list_timeout(tde_swjob *job, hi_u32 time_out)
{
    hi_s32 ret;
    unsigned long lockflags;

    ret = osal_wait_event_timeout_uninterruptible(&(job->query), tdeosi_wait_call_back, (hi_void *)job, time_out);
    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    job->wait_for_done_count--;

    if (job->noti_type != TDE_JOB_NOTIFY_BUTT) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        if (ret == (-ERESTARTSYS)) {
            tde_error("query handle (%d) interrupt!\n", job->handle);
            return HI_ERR_TDE_INTERRUPT;
        } else {
            tde_error("query handle (%d) time out!\n", job->handle);
        }
        return HI_ERR_TDE_QUERY_TIMEOUT;
    }

    /* complete */
    if (job->wait_for_done_count == 0) {
        tde_osi_list_destroy_job(job);
    }
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    return HI_SUCCESS;
}

/*
 * Function:      tde_osi_list_wait_for_done
 * Description:   block to wait for job done
 * Input:         handle: job handle
 *                time_out: timeout value
 * Return:        =0,successfully completed <0,error
 */
hi_s32 tde_osi_list_wait_for_done(hi_s32 handle, hi_u32 time_out)
{
    tde_swjob *job = HI_NULL;
    hi_handle_mgr *handle_mgr = HI_NULL;
    hi_s32 ret;
    hi_bool valid = HI_FALSE;
    unsigned long lockflags;

    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    valid = tde_query_handle(handle, &handle_mgr);
    if (!valid) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return HI_SUCCESS;
    }

    job = (tde_swjob *)handle_mgr->res;
    if (!job->submitted) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        tde_error("job %d has no submitted!\n", handle);
        return HI_ERR_TDE_INVALID_HANDLE;
    }
    job->wait_for_done_count++;
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    if (time_out) {
        ret = tde_osi_list_timeout(job, time_out);
        if (ret != HI_SUCCESS) {
            return ret;
        }
        return HI_SUCCESS;
    }
    ret = osal_wait_event_interruptible(&(job->query), tdeosi_wait_call_back, (hi_void *)job);
    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    job->wait_for_done_count--;
    if (ret == (-ERESTARTSYS)) {
        if ((job->noti_type == TDE_JOB_NOTIFY_BUTT) && (job->wait_for_done_count == 0)) {
            tde_osi_list_destroy_job(job);
        }
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        tde_error("query handle (%d) interrupt!\n", job->handle);
        return HI_ERR_TDE_INTERRUPT;
    }

    if (job->wait_for_done_count == 0) {
        tde_osi_list_destroy_job(job);
    }
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    return HI_SUCCESS;
}

/*
 * Prototype    : tde_osi_list_comp_proc
 * Description  : list complete interrupt servic, mainly complete switch on hardware lists
 * Modification : Created function
 */
hi_void tde_osi_list_comp_proc()
{
    hi_s32 ret;

    if (g_tde_osi_job_list->job_to_commit != HI_NULL) {
        ret = tde_hal_node_execute(g_tde_osi_job_list->job_to_commit->first_cmd->node_buf.phy_addr,
                                   g_tde_osi_job_list->job_to_commit->first_cmd->node_buf.update,
                                   g_tde_osi_job_list->job_to_commit->aq_use_buff);
        if (ret == HI_SUCCESS) {
            g_tde_osi_job_list->job_committed = g_tde_osi_job_list->job_to_commit;
            g_tde_osi_job_list->job_to_commit = HI_NULL;
            g_tde_osi_job_list->job_finished = 0x0;

#ifdef TDE_HWC_COOPERATE
            g_working_flag = 1;
#endif
        }
    }

    return;
}

#ifdef TDE_HWC_COOPERATE
hi_void tde_osi_list_comp(hi_void)
{
    unsigned long lockflags;
    unsigned long lock;

    tde_spin_lock(&g_tde_osi_job_list->lock, lock);
    tde_osi_list_lock_working_flag(&lockflags);
    if (!g_working_flag) {
        if (tde_hal_ctl_is_idle_safely()) {
            tde_osi_list_comp_proc();
        }
    }

    tde_osi_list_unlock_working_flag(&lockflags);
    tde_spin_unlock(&g_tde_osi_job_list->lock, lock);
    return;
}
#endif

static hi_s32 tde_list_comp_get_finished_handle(hi_s32 *finished_handle, hi_handle_mgr *handle_mgr)
{
    tde_swjob *job = HI_NULL;
    tde_swjob *del_job = HI_NULL;

    job = (tde_swjob *)handle_mgr->res;
    if (job->list.prev == &g_tde_osi_job_list->list) {
        tde_info("No pre Job left, finishedhandle:%d\n", *finished_handle);
        return HI_FAILURE;
    }

    del_job = osal_list_entry(job->list.prev, tde_swjob, list);
    *finished_handle = del_job->handle;
    g_tde_osi_job_list->job_finished = 0x0;
    return HI_SUCCESS;
}

static hi_void tde_list_comp_wakeup(tde_swjob *del_job, hi_size_t *lockflags)
{
    tde_info("handle:%d!\n", del_job->handle);
    del_job->noti_type = TDE_JOB_NOTIFY_BUTT;
    if (del_job->func_compl_cb != HI_NULL) {
        tde_info("handle:%d has callback func!\n", del_job->handle);
        osal_spin_unlock_irqrestore(&g_tde_osi_job_list->lock, lockflags);
        del_job->func_compl_cb(del_job->func_para, &(del_job->handle));
        osal_spin_lock_irqsave(&g_tde_osi_job_list->lock, lockflags);
    }

    if (del_job->wait_for_done_count != 0) {
        tde_info("query handle %d complete!\n", del_job->handle);
        osal_wakeup(&del_job->query);
    }

    osal_wakeup(&g_tde_block_job_wq);
}

static hi_void tde_list_comp_proc_job(hi_void)
{
    if (tde_hal_ctl_is_idle_safely()) {
        tde_hal_ctl_int_status();
        tde_osi_list_comp_proc();
    }
}

static hi_s32 tde_list_comp_delete_job(hi_bool work, hi_s32 finished_handle, hi_size_t *lockflags)
{
    tde_swjob *del_job = HI_NULL;
    hi_s32 delhandle;
#ifdef TDE_HWC_COOPERATE
    unsigned long lock;
#endif

    while (!osal_list_empty(&g_tde_osi_job_list->list)) {
        del_job = osal_list_entry(g_tde_osi_job_list->list.next, tde_swjob, list);
        delhandle = del_job->handle;
        g_tde_osi_job_list->job_num--;
        if (delhandle == g_tde_osi_job_list->handle_last) {
            g_tde_osi_job_list->handle_last = -1;
        }
        osal_list_del_init(&del_job->list);
        if (del_job->noti_type == TDE_JOB_WAKE_NOTIFY) {
            tde_list_comp_wakeup(del_job, lockflags);
        } else if (del_job->noti_type == TDE_JOB_COMPL_NOTIFY) {
            tde_info("handle:%d!\n", del_job->handle);
            if (del_job->func_compl_cb != HI_NULL) {
                tde_info("handle:%d has callback func!\n", del_job->handle);
                osal_spin_unlock_irqrestore(&g_tde_osi_job_list->lock, lockflags);
                del_job->func_compl_cb(del_job->func_para, &(del_job->handle));
                osal_spin_lock_irqsave(&g_tde_osi_job_list->lock, lockflags);
            }
            tde_osi_list_safe_destroy_job(del_job);
        } else {
            tde_error("Error Status!!\n");
        }
        if (delhandle == finished_handle) {
            if (!work) {
#ifdef TDE_HWC_COOPERATE
                tde_osi_list_lock_working_flag(&lock);
                g_working_flag = 0;
                tde_osi_list_unlock_working_flag(&lock);
#else
                tde_list_comp_proc_job();
#endif
            }
            osal_spin_unlock_irqrestore(&g_tde_osi_job_list->lock, lockflags);
            return HI_SUCCESS;
        }
    }
    return HI_FAILURE;
}

/*
 * Function:      tde_osi_list_node_comp
 * Description:   node complete interrupt service, maily complete deleting node and resume suspending,free node
 * Return:        create job handle
 * Modification : Created function
 */
hi_void tde_osi_list_node_comp()
{
    hi_handle_mgr *handle_mgr = HI_NULL;
    hi_s32 finished_handle;
    hi_u64 running_sw_node_addr;
    hi_bool work = HI_TRUE;
    hi_u32 *finish_handle = HI_NULL;
    unsigned long lockflags;

    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    if (tde_hal_ctl_is_idle_safely()) {
        work = HI_FALSE;
    }
    running_sw_node_addr = tde_hal_cur_node();
    if ((running_sw_node_addr == 0) || (running_sw_node_addr == g_tde_osi_job_list->job_finished)) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return;
    }

    finish_handle = (hi_u32 *)wgetvrt(running_sw_node_addr - TDE_NODE_HEAD_BYTE + 8); /* 8 alg data */
    if (finish_handle == HI_NULL) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return;
    }

    finished_handle = *finish_handle;
    if (!tde_query_handle(finished_handle, &handle_mgr)) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return;
    }
    if (!work) {
        g_tde_osi_job_list->job_finished = running_sw_node_addr;
    } else if (tde_list_comp_get_finished_handle(&finished_handle, handle_mgr) < 0) {
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return;
    }

    if (!tde_query_handle(finished_handle, &handle_mgr)) {
        tde_info("handle %d already delete!\n", finished_handle);
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return;
    }

    g_tde_osi_job_list->handle_finished = finished_handle;
    if (tde_list_comp_delete_job(work, finished_handle, &lockflags) == HI_SUCCESS) {
        return;
    }
    if (tde_hal_ctl_is_idle_safely()) {
        tde_info("tde_hal_ctl_is_idle_safely HI_FALSE!\r\n");
    }
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
}
#endif

/*
 * Function:      tde_osi_list_get_phy_buff
 * Description:   get one physical buffer, to deflicker and zoom
 * Return:        created job handle
 */
hi_u32 tde_osi_list_get_phy_buff(hi_u32 cb_cr_offset)
{
#ifndef HI_BUILD_IN_BOOT
    if (osal_in_interrupt()) {
        return 0;
    }
#endif
    return tde_alloc_physic_buff(cb_cr_offset);
}

#ifndef HI_BUILD_IN_BOOT
void tde_osi_list_freevmem(struct osal_work_struct *work)
{
    tdefreewq *w_queue_info = osal_container_of(work, tdefreewq, work);
    if (w_queue_info != HI_NULL) {
        w_queue_info->wqcb(w_queue_info->count);
        tde_free(w_queue_info);
    }
    return;
}

void tde_osi_list_hsr(hi_void *func, hi_u32 data)
{
    tdefreewq *wq = HI_NULL;

    wq = tde_malloc(sizeof(tdefreewq));
    if (wq == HI_NULL) {
        tde_error("Malloc TDEFREEWQ_S failed!\n");
        return;
    }
    wq->count = data;
    wq->wqcb = (TDE_WQ_CB)func;
    OSAL_INIT_WORK(&wq->work, tde_osi_list_freevmem);
    osal_schedule_work(&wq->work);
    return;
}
#endif

hi_void tde_osi_list_put_phy_buff(hi_u32 buff_num)
{
    if (buff_num == 0) {
        return;
    }
#ifndef HI_BUILD_IN_BOOT
    tde_osi_list_hsr((hi_void *)tde_osi_list_do_free_phy_buff, buff_num);
#else
    tde_osi_list_do_free_phy_buff(buff_num);
#endif
    return;
}

#ifdef TDE_HWC_COOPERATE
hi_s32 tde_osi_list_lock_working_flag(unsigned long *lock)
{
    if (lock == HI_NULL) {
        return HI_FAILURE;
    }
    tde_spin_lock(&g_working_flag_lock, *lock);

    return HI_SUCCESS;
}

hi_s32 tde_osi_list_unlock_working_flag(unsigned long *lock)
{
    if (lock == HI_NULL) {
        return HI_FAILURE;
    }
    tde_spin_unlock(&g_working_flag_lock, *lock);

    return HI_SUCCESS;
}

hi_s32 tde_osi_list_get_working_flag(hi_bool *flag)
{
    if (flag == HI_NULL) {
        return HI_FAILURE;
    }
    *flag = g_working_flag;

    return HI_SUCCESS;
}
#endif

hi_void tde_free_pending_job(hi_void *private_data)
{
    hi_handle_mgr *handle = HI_NULL;
    hi_handle_mgr *save = HI_NULL;
    tde_swjob *job = HI_NULL;
    unsigned long lockflags;
    hi_handle_mgr *tde_handle_list = tde_get_handle_list();

    if (tde_handle_list == HI_NULL) {
        return;
    }
    tde_spin_lock(&g_tde_osi_job_list->lock, lockflags);
    handle = osal_list_entry(tde_handle_list->header.next, hi_handle_mgr, header);
    if (handle == HI_NULL) {
        tde_info("No pending job!!\n");
        tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
        return;
    }
    while (handle != tde_handle_list) {
        save = handle;
        job = (tde_swjob *)save->res;
        /* debug */
        if (job == HI_NULL) {
            tde_error("ERR:pstJob Null Pointer!!!\n");
            tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
            return;
        }

        /* END */
        handle = osal_list_entry(handle->header.next, hi_handle_mgr, header);
        /* free when it is not submitted */
#ifndef __RTOS__
        if ((!job->submitted) && (private_data == job->private_data)) {
#else
        if (!job->submitted) {
            hi_unused(private_data);
#endif
            /* free handle resource */
            tde_osi_list_free_serial_cmd(job->first_cmd, job->tail_node);
#ifndef HI_BUILD_IN_BOOT
            osal_wait_destroy(&job->query);
#endif
            osal_list_del_init(&save->header);
            tde_free(save);
        }

        if (handle == HI_NULL) {
            tde_error("ERR: pstHandle Null Pointer!!\n");
            tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
            return;
        }
    }
    tde_spin_unlock(&g_tde_osi_job_list->lock, lockflags);
    return;
}

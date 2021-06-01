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

#ifndef __TDE_OSILIST_H__
#define __TDE_OSILIST_H__

#include "drv_tde_ioctl.h"
#include "tde_define.h"
#include "drv_tde_type.h"
#include "osal_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Data struct of software list node */
typedef struct {
    struct osal_list_head list;
    hi_s32 handle;                   /* Job handle of the instruct */
    hi_s32 index;                    /*
                                      * Instruct serial number in job,
                                      * form one on start, the same number is the same instruct
                                      */
    tde_node_subm_type subm_type;   /* current node type */
    tde_node_buf node_buf;          /* Node of operate config */
    hi_u32 phy_buff_num;            /* Number of physical buffer distributed */
} tde_swnode;

/* Job definition */
typedef struct {
    struct osal_list_head list;
    hi_s32 handle;                /* Job handle */
    tde_func_cb func_compl_cb;     /* Pointer of callback function */
    hi_void *func_para;           /* Arguments of callback function */
    tde_notify_mode noti_type; /* Notice type after node completed */
    hi_u32 cmd_num;             /* Instruct number of job */
    hi_u32 node_num;            /* Node number of job */
    tde_swnode *first_cmd;    /* Software node of first instruct in job */
    tde_swnode *last_cmd;     /* Software node of last instruct in job  */
    tde_swnode *tail_node;    /* Last software node of job */
#ifndef HI_BUILD_IN_BOOT
    osal_wait_t query; /* Wait queue used in query */
#endif
    hi_bool submitted;       /* If have submitted */
    hi_bool aq_use_buff;       /* If using temporary buffer */
    hi_u8 wait_for_done_count; /* wait job count */
    hi_void *private_data;
} tde_swjob;

/*
 * Function:      tde_osi_list_init
 * Description:   Initialize all lists inside software to use as TDE operation cache
 * Return:        Success/fail
 */
hi_s32 tde_osi_list_init(hi_void);

/*
 * Function:      tde_osi_list_term
 * Description:   Release all lists inside software to use as TDE operation cache
 */
hi_void tde_osi_list_term(hi_void);

hi_s32 tde_osi_list_begin_job(hi_s32 *handle, hi_void *private_data);

/*
 * Function:      tde_osi_list_cancel_job
 * Description:   Delete list of handle point
 * Input:         handle: the list wait for delete
 * Return:        success/fail
 */
hi_s32 tde_osi_list_cancel_job(hi_s32 handle);

/*
 * Function:      tde_osi_list_submit_job
 * Description:   Create list, return head pointer of list
 * Input:         handle: Head pointer of job needing submit
 *                is_block: if block
 *                time_out: time out
 *                func_compl_cb: callback function of complete operate
 * Return:        success/fail/time out
 */
#if (TDE_CAPABILITY & SYNC)
hi_s32 tde_osi_list_submit_job(drv_tde_end_job_cmd *end_job, tde_func_cb func_compl_cb, hi_void *func_para,
                               tde_notify_mode noti_type, hi_bool is_sync);
#else
hi_s32 tde_osi_list_submit_job(drv_tde_end_job_cmd *end_job, tde_func_cb func_compl_cb, hi_void *func_para,
                               tde_notify_mode noti_type);
#endif

/*
 * Prototype    : func_compl_cb
 * Description  : wait for all TDE operate completed
 * Return Value : HI_SUCCESS,TDE operate completed
 */
#ifndef HI_BUILD_IN_BOOT
hi_s32 tde_osi_list_wait_all_done(hi_void);

/*
 * Function:      tde_osi_list_wait_for_done
 * Description:   Query if submitted TDE operate is completed
 * Input:         handle: task handle
 *                time_out: if Time out
 * Return:        Success/fail
 */
hi_s32 tde_osi_list_wait_for_done(hi_s32 handle, hi_u32 time_out);

/*
 * Function:      tde_osi_list_comp_proc
 * Description:   Interrupt to handling all list operate is completed
 * Return:        task handle is created
 */
hi_void tde_osi_list_comp_proc(hi_void);
#ifdef TDE_HWC_COOPERATE
hi_void tde_osi_list_comp(hi_void);
#endif
hi_void tde_osi_list_node_comp(hi_void);
#endif

/*
 * Function:      tde_osi_list_get_phy_buff
 * Description:   Get one physical buffer, used in deflicker and zoom
 * Return:        physical address assigned
 */
hi_u32 tde_osi_list_get_phy_buff(hi_u32 cb_cr_offset);

hi_void tde_osi_list_put_phy_buff(hi_u32 buff_num);

hi_void tde_osi_list_free_serial_cmd(tde_swnode *fst_cmd, tde_swnode *last_cmd);
hi_s32 tde_osi_list_lock_working_flag(unsigned long *lock);

hi_s32 tde_osi_list_unlock_working_flag(unsigned long *lock);

hi_s32 tde_osi_list_get_working_flag(hi_bool *flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TDE_OSILIST_H_ */

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
#ifndef __DRV_HDMI_EVENT_H__
#define __DRV_HDMI_EVENT_H__

#include "hi_type.h"
#include "drv_hdmi_common.h"
#include "hdmi_product_define.h"

#define HDMI_EVENT_POOL_SIZE 10
#define HDMI_EVENT_ID_EXIST  0xff
#define HDMI_EVENT_POOL_CNT  10

typedef struct {
    hi_u32 hpd_rd_cnt;
    hi_u32 hpd_wr_cnt;
    hi_u32 unhpd_rd_cnt;
    hi_u32 unhpd_wr_cnt;
    hi_u32 edid_fail_rd_cnt;
    hi_u32 edid_fail_wr_cnt;
    hi_u32 rsen_con_rd_cnt;
    hi_u32 rsen_con_wr_cnt;
    hi_u32 rsen_dis_rd_cnt;
    hi_u32 rsen_dis_wr_cnt;
    hi_u32 err_rd_cnt;
    hi_u32 err_wd_cnt;
} hdmi_event_run_cnt;

typedef struct {
    hi_u32     pool_id;
    hi_bool    wakeup_flag;
    hi_u32     readable_cnt;
    hi_u32     read_ptr;
    hi_u32     write_ptr;
    hdmi_event event_pool[HDMI_EVENT_POOL_SIZE];
} hdmi_event_run_ctrl;

typedef struct {
    hdmi_event_run_cnt run_cnt;
    hdmi_event_run_ctrl ctrl;
} hdmi_event_pool;

typedef struct {
    osal_semaphore_t event_mutex; /* lock to avoid competition */
    osal_wait_t      wr_queue;    /* wait queue to wake up the sleeping process */
    hi_bool          wakeup_all;  /* wakeup all pool flag */
    hi_bool          init;        /* is the struct init */
    hi_u32           total;       /* total pools in the list */
    hdmi_event_pool  pool[HDMI_EVENT_POOL_CNT];
    hdmi_event       event;
} hdmi_event_info;

typedef struct {
    hdmi_event_info *evt_info;
    hdmi_event_pool *tmp_pool;
} hdmi_event_wait_callback;

hi_s32 drv_hdmi_event_init(hdmi_device_id hdmi_id);

hi_s32 drv_hdmi_event_deinit(hdmi_device_id hdmi_id);

hi_s32 drv_hdmi_event_pool_malloc(hdmi_device_id hdmi_id, hi_u32 *pool_id);

hi_s32 drv_hdmi_event_pool_free(hdmi_device_id hdmi_id, hi_u32 pool_id);

hi_s32 drv_hdmi_event_pool_write(hdmi_device_id hdmi_id, hdmi_event event);

hi_s32 drv_hdmi_event_pool_read(hdmi_device_id hdmi_id, hi_u32 pool_id, hdmi_event *event);

hi_s32 drv_hdmi_event_pool_status_get(hdmi_device_id hdmi_id, hi_u32 pool_num,
                                      hdmi_event_run_ctrl *ctrl, hdmi_event_run_cnt *cnt);

hi_s32 drv_hdmi_event_pool_total_get(hdmi_device_id hdmi_id, hi_u32 *total);

hi_s32 drv_hdmi_event_pool_id_get(hdmi_device_id hdmi_id, hi_u32 *thread_id);

#endif  // __DRV_HDMI_EVENT_H__


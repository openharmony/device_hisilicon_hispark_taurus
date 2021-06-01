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

#ifndef __VALG_PLAT_H__
#define __VALG_PLAT_H__

#include "hi_osal.h"

#include "hi_type.h"
#include "hi_errno.h"
#include "hi_debug.h"
#include "hi_board.h"

#include "hi_common.h"
#include "hi_comm_video.h"
#include "mm_ext.h"
#include "dev_ext.h"
#include "proc_ext.h"

#ifdef __KERNEL__
#else
#include <sys/time.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define VALG_TRACE(level, mod_id, fmt, ...)                                                                \
    do {                                                                                                    \
        HI_TRACE(level, mod_id, "[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#define VALG_ERR_LEVEL_NOTE 0


typedef osal_spinlock_t valg_lock;
typedef unsigned long VALG_LOCK_FLAG;


__inline static hi_s32 valg_create_lock(valg_lock *lock, hi_bool locked)
{
    hi_unused(locked);
    return osal_spin_lock_init(lock);
}

__inline static hi_s32 valg_lock_irqsave(valg_lock *lock, VALG_LOCK_FLAG *flag)
{
    osal_spin_lock_irqsave(lock, flag);
    return HI_SUCCESS;
}

__inline static hi_s32 valg_unlock(valg_lock *lock, VALG_LOCK_FLAG *flag)
{
    osal_spin_unlock_irqrestore(lock, flag);
    return HI_SUCCESS;
}

__inline static hi_void valg_delete_lock(valg_lock *lock)
{
    osal_spin_lock_destroy(lock);
    return;
}
#define forceinline         __inline__ __attribute__((always_inline))

typedef osal_timer_t valg_timer;
typedef hi_void (*PTR_TIMER_CALLBACK)(unsigned long data);

__inline static hi_s32 valg_timer_init(valg_timer *ph_timer, PTR_TIMER_CALLBACK ptr_timer, hi_ulong ul_data)
{
    hi_s32 ret;

    ret = osal_timer_init(ph_timer);
    if (ret) {
        return ret;
    }

    ph_timer->data = ul_data;
    ph_timer->function = ptr_timer;
    return HI_SUCCESS;
}

__inline static hi_s32 valg_timer_start_once(valg_timer *ph_timer, hi_ulong ul_interval)
{
    return osal_set_timer(ph_timer, ul_interval);
}

__inline static hi_void valg_timer_delete(valg_timer *ph_timer)
{
    osal_del_timer(ph_timer);
    osal_timer_destroy(ph_timer);

    return;
}

__inline static hi_s32 valg_thread_init(osal_task_t **thread, hi_void *task_function, hi_void *data, char *task_name)
{
    *thread = osal_kthread_create(task_function, data, task_name);
    if (*thread == NULL) {
        osal_printk("create thread fail!!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

__inline static hi_u32 valg_thread_delete(osal_task_t *task, hi_bool stop_flag)
{
    osal_kthread_destroy(task, stop_flag);
    return HI_SUCCESS;
}

typedef hi_u32 VALG_POOL_HANDLE;
typedef int irqreturn_t;
typedef osal_irq_handler_t PTR_INT_CALLBACK;

__inline static hi_s32 valg_irq_init(hi_s32 irq, PTR_INT_CALLBACK ptr_call_back, hi_void *data)
{
    return osal_request_irq(irq, ptr_call_back, NULL, "vedu", data);
}

__inline static hi_s32 valg_irq_delete(hi_s32 irq, void *dev_id)
{
    osal_free_irq(irq, dev_id);
    return HI_SUCCESS;
}

__inline static hi_u64 get_sys_time_by_sec(void)
{
#ifdef __KERNEL__
    osal_timeval_t time;
    osal_gettimeofday(&time);
#else
    struct timeval time;
    gettimeofday(&time, NULL);
#endif
    return (hi_u64)time.tv_sec;
}

__inline static hi_u64 get_sys_time_by_usec(void)
{
#ifdef __KERNEL__
    osal_timeval_t time;
    osal_gettimeofday(&time);
#else
    struct timeval time;
    gettimeofday(&time, NULL);
#endif
    return (hi_u64)((time.tv_sec * 1000000LLU) + time.tv_usec);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __VALG_PLAT_H__ */

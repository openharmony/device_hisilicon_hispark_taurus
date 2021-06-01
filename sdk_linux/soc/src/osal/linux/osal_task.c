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

#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include "securec.h"

osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const char *name)
{
    struct task_struct *k = NULL;
    osal_task_t *p = (osal_task_t *)kmalloc(sizeof(osal_task_t), GFP_KERNEL);
    if (p == NULL) {
        osal_trace("%s - kmalloc error!\n", __FUNCTION__);
        return NULL;
    }
    (void)memset_s(p, sizeof(osal_task_t), 0, sizeof(osal_task_t));

    k = kthread_run(thread, data, name);
    if (IS_ERR(k)) {
        osal_trace("%s - kthread create error!\n", __FUNCTION__);
        kfree(p);
        p = NULL;
        return NULL;
    }
    p->task_struct = k;
    return p;
}
EXPORT_SYMBOL(osal_kthread_create);

void osal_kthread_destroy(osal_task_t *task, unsigned int stop_flag)
{
    if (task == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
    /* note: When you call the Kthread_stop function, the thread function cannot be finished, otherwise it will oops. */
    if (stop_flag != 0) {
        kthread_stop ((struct task_struct *)(task->task_struct));
    }
    task->task_struct = NULL;
    kfree(task);
}
EXPORT_SYMBOL(osal_kthread_destroy);

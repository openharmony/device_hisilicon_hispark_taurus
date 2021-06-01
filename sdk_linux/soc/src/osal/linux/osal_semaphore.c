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
#include <linux/semaphore.h>
#include <linux/slab.h>

int osal_sema_init(osal_semaphore_t *sem, int val)
{
    struct semaphore *p = NULL;

    if (sem == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (p == NULL) {
        osal_trace("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    sema_init(p, val);
    sem->sem = p;
    return 0;
}
EXPORT_SYMBOL(osal_sema_init);
int osal_down(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    down(p);
    return 0;
}
EXPORT_SYMBOL(osal_down);
int osal_down_interruptible(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    return down_interruptible(p);
}
EXPORT_SYMBOL(osal_down_interruptible);
int osal_down_trylock(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    return down_trylock(p);
}
EXPORT_SYMBOL(osal_down_trylock);
void osal_up(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;

    p = (struct semaphore *)(sem->sem);
    up(p);
}
EXPORT_SYMBOL(osal_up);
void osal_sema_destroy(osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;

    p = (struct semaphore *)(sem->sem);
    kfree(p);
    p = NULL;
    sem->sem = NULL;
}
EXPORT_SYMBOL(osal_sema_destroy);

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
#include <asm/atomic.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include "securec.h"

int osal_atomic_init(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if (atomic == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)kmalloc(sizeof(atomic_t), GFP_KERNEL);
    if (p == NULL) {
        osal_trace("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    (void)memset_s(p, sizeof(atomic_t), 0, sizeof(atomic_t));
    atomic->atomic = p;
    return 0;
}
EXPORT_SYMBOL(osal_atomic_init);
void osal_atomic_destroy(osal_atomic_t *atomic)
{
    if (atomic != NULL) {
        kfree(atomic->atomic);
        atomic->atomic = NULL;
    }
}
EXPORT_SYMBOL(osal_atomic_destroy);
int osal_atomic_read(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if (atomic == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_read(p);
}
EXPORT_SYMBOL(osal_atomic_read);
void osal_atomic_set(osal_atomic_t *atomic, int i)
{
    atomic_t *p = NULL;
    if (atomic != NULL) {
        p = (atomic_t *)(atomic->atomic);
        atomic_set(p, i);
    }
}
EXPORT_SYMBOL(osal_atomic_set);
int osal_atomic_inc_return(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if (atomic == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_inc_return(p);
}
EXPORT_SYMBOL(osal_atomic_inc_return);
int osal_atomic_dec_return(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if (atomic == NULL) {
        osal_trace("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_dec_return(p);
}
EXPORT_SYMBOL(osal_atomic_dec_return);

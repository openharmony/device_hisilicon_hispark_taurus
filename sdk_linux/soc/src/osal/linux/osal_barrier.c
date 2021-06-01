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
#include <asm/barrier.h>

void osal_mb(void)
{
    mb();
}
EXPORT_SYMBOL(osal_mb);
void osal_rmb(void)
{
    rmb();
}
EXPORT_SYMBOL(osal_rmb);
void osal_wmb(void)
{
    wmb();
}
EXPORT_SYMBOL(osal_wmb);
void osal_smp_mb(void)
{
    smp_mb();
}
EXPORT_SYMBOL(osal_smp_mb);
void osal_smp_rmb(void)
{
    smp_rmb();
}
EXPORT_SYMBOL(osal_smp_rmb);
void osal_smp_wmb(void)
{
    smp_wmb();
}
EXPORT_SYMBOL(osal_smp_wmb);
void osal_isb(void)
{
    isb();
}
EXPORT_SYMBOL(osal_isb);
void osal_dsb(void)
{
#ifdef CONFIG_64BIT
    dsb(sy);
#else
    dsb();
#endif
}
EXPORT_SYMBOL(osal_dsb);
void osal_dmb(void)
{
#ifdef CONFIG_64BIT
    dmb(sy);
#else
    dmb();
#endif
}
EXPORT_SYMBOL(osal_dmb);

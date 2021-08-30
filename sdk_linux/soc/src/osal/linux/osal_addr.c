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
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)

#ifndef CONFIG_64BIT
#include <mach/io.h>
#endif

#endif

void *osal_ioremap(unsigned long phys_addr, unsigned long size)
{
    return ioremap(phys_addr, size);
}
EXPORT_SYMBOL(osal_ioremap);

void *osal_ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
    return ioremap(phys_addr, size);
#else
    return ioremap_nocache(phys_addr, size);
#endif
}
EXPORT_SYMBOL(osal_ioremap_nocache);

void *osal_ioremap_cached(unsigned long phys_addr, unsigned long size)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    return ioremap_cached(phys_addr, size);
#else
    return ioremap_cache(phys_addr, size);
#endif
}

void *osal_ioremap_wc(unsigned long phys_addr, unsigned long size)
{
    return ioremap_wc(phys_addr, size);
}
EXPORT_SYMBOL(osal_ioremap_wc);

EXPORT_SYMBOL(osal_ioremap_cached);

void osal_iounmap(void *addr, unsigned long size)
{
    iounmap(addr);
}
EXPORT_SYMBOL(osal_iounmap);

unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n)
{
    return copy_from_user(to, from, n);
}
EXPORT_SYMBOL(osal_copy_from_user);

unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n)
{
    return copy_to_user(to, from, n);
}
EXPORT_SYMBOL(osal_copy_to_user);

int osal_access_ok(int type, const void *addr, unsigned long size)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0)
    return access_ok(addr, size);
#else
    return access_ok(type, addr, size);
#endif
}
EXPORT_SYMBOL(osal_access_ok);

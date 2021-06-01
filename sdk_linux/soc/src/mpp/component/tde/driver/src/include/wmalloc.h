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

#ifndef __WMALLOC_H__
#define __WMALLOC_H__

#include "hi_type.h"
#include "hi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* CNcomment:
 * Function: wmeminit
 * Description: init memory pool. CNcomment: TDE memory pool initialization
 * Return: success/error code. CNcomment: Success/error code
 */
hi_s32 wmeminit(void);

/*
 * Function: wmemterm
 * Description: Deinit memory pool. CNcomment:TDE memory pool initialization
 */
hi_void wmemterm(void);

/*
 * Function: wgetphy
 * Description: get the physics address of memory. CNcomment: To obtain the physical address of memory
 * Input: ptr  the pointer of memory. CNcomment: Pointer to the memory
 * Output:  physics address. CNcomment: Physical address
 */
hi_u64 wgetphy(hi_void *ptr);

/*
 * Function: wgetphy
 * Description: get the virtual address of memory. CNcomment: Access to the virtual address of memory
 * Input: phyaddr  physics address. CNcomment: Physical address
 * Output: virtual address. CNcomment: A virtual address
 */
hi_void *wgetvrt(hi_u64 phyaddr);

/*
 * Function: wgetfreenum
 * Description:get the number of the free unit. CNcomment: Memory for the rest of the Unit cell number
 * Return: the number of the free unit. CNcomment: The rest of the unit number
 */
hi_u32 wgetfreenum(hi_void);
#ifndef TDE_BOOT
osal_proc_entry_t *wprintinfo(osal_proc_entry_t *page);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __WMALLOC_H__ */

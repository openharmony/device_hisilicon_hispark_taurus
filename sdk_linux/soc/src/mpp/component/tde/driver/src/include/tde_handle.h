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

#ifndef __TDE_HANDLE_H__
#define __TDE_HANDLE_H__

#include "tde_osilist.h"
#include "tde_define.h"
#include "osal_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    struct osal_list_head header; /* Use to organiza handle */
#ifndef HI_BUILD_IN_BOOT
    osal_spinlock_t lock;
#endif
    hi_s32 handle;
    hi_void *res;
} hi_handle_mgr;

#define TDE_MAX_HANDLE_VALUE 0x7fffffff

hi_bool tde_initial_handle(hi_void);

hi_void tde_get_handle(hi_handle_mgr *res, hi_s32 *handle);

/*
 * Function:      query_handle
 * Description:   Query the job according to the job handle and get the job resource
 * Input:         handle:job ID res:job strcut pointer
 * Output:        res:job resource
 * Return:        Success/fail
 */
hi_bool tde_query_handle(hi_s32 handle, hi_handle_mgr **res);

/*
 * Function:      release_handle
 * Description:   Delete the handle node from the global handle list according to the handle value.
 * Input:         handle:job ID
 * Return:        Success/fail
 */
hi_bool tde_release_handle(hi_s32 handle);

/*
 * Function:      destroy_handle
 * Description:   Free global handle list head node
 */
hi_void tde_destroy_handle(hi_void);

hi_handle_mgr *tde_get_handle_list(hi_void);

#ifndef HI_BUILD_IN_BOOT
/*
 * Function:      TdeFreePendingJob
 * Description:   Free the job which is not submitted  when execute Ctrl +C (kill the current process).
 */
hi_void tde_free_pending_job(hi_void *private_data);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __TDE_HANDLE_H__ */

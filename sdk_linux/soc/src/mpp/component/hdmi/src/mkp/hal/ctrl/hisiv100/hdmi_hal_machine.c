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
#include "hdmi_hal_machine.h"
#include "hdmi_osal.h"
#include "drv_hdmi_common.h"

#define MACHINE_MAX_ID_NUM     19
#define MACHIME_NULL_TINTERVAL 20
#define MACHIME_MIN_TINTERVAL  5

typedef struct {
    osal_semaphore_t mach_mutex;
    hi_bool init;
    hi_u32 total;
    hdmi_mach_elem mach_elem[MACHINE_MAX_ID_NUM];
} hdmi_mach_info;

static hdmi_mach_info g_machine_info;

hi_s32 hal_hdmi_mach_init(hi_void)
{
    hdmi_mach_info *mach_info = &g_machine_info;

    if (!mach_info->init) {
        osal_sema_init(&mach_info->mach_mutex, 1);
        (hi_void)memset_s(mach_info->mach_elem, MACHINE_MAX_ID_NUM * sizeof(hdmi_mach_elem), 0,
            MACHINE_MAX_ID_NUM * sizeof(hdmi_mach_elem));
        mach_info->total = 0;
        mach_info->init  = HI_TRUE;
    }

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_deinit(hi_void)
{
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_false_return(mach_info->init, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    (hi_void)memset_s(mach_info->mach_elem, sizeof(mach_info->mach_elem), 0, sizeof(mach_info->mach_elem));
    mach_info->total = 0;
    mach_info->init = HI_FALSE;
    hdmi_mutex_unlock(mach_info->mach_mutex);
    osal_sema_destroy(&mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_invoke(hi_void)
{
    hi_u32 i;
    hdmi_mach_elem *tmp_elem  = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_false_return(mach_info->init, HI_FAILURE);

    if (mach_info->total == 0) {
        osal_msleep(MACHIME_NULL_TINTERVAL);
    } else {
        for (i = 0; i < MACHINE_MAX_ID_NUM; ++i) {
            tmp_elem = &mach_info->mach_elem[i];
            if (!(tmp_elem->mach_run.valid_id) || !(tmp_elem->mach_run.enable) ||
                ((hdmi_osal_get_time_in_ms() - tmp_elem->mach_run.last_time) <= tmp_elem->mach_ctrl.interval)) {
                continue;
            }
            if (tmp_elem->mach_ctrl.callback != HI_NULL) {
                tmp_elem->mach_ctrl.callback(tmp_elem->mach_ctrl.data);
            }
            hdmi_mutex_lock(mach_info->mach_mutex);
            tmp_elem->mach_run.last_time = hdmi_osal_get_time_in_ms();
            tmp_elem->mach_run.timestamp[tmp_elem->mach_run.stamp_idx] = hdmi_osal_get_time_in_ms();
            tmp_elem->mach_run.stamp_idx = (tmp_elem->mach_run.stamp_idx + 1) % HDMI_MACH_MAX_STAMPE_NUM;
            tmp_elem->mach_run.run_cnt++;
            hdmi_mutex_unlock(mach_info->mach_mutex);
        }
    }

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_register(const hdmi_mach_ctrl *mach_ctrl, hi_u32 *mach_id)
{
    errno_t errnumber;
    hi_u32 i;
    hi_s32 name_len, ret;
    hdmi_mach_elem *tmp_elem  = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_null_return(mach_ctrl, HI_FAILURE);
    hdmi_if_null_return(mach_id, HI_FAILURE);
    hdmi_if_false_return(mach_info->init, HI_FAILURE);

    if (mach_info->total >= MACHINE_MAX_ID_NUM) {
        hdmi_warn("FULL num=%u! register machine fail! \n", MACHINE_MAX_ID_NUM);
        ret = HI_FAILURE;
    } else {
        hdmi_mutex_lock(mach_info->mach_mutex);
        for (i = 0; i < MACHINE_MAX_ID_NUM; ++i) {
            tmp_elem = &mach_info->mach_elem[i];
            if (!tmp_elem->mach_run.valid_id) {
                (hi_void)memset_s(&tmp_elem->mach_run, sizeof(tmp_elem->mach_run), 0, sizeof(hdmi_mach_run));
                errnumber = memcpy_s(&tmp_elem->mach_ctrl, sizeof(tmp_elem->mach_ctrl),
                    mach_ctrl, sizeof(hdmi_mach_ctrl));
                hdmi_unlock_unequal_eok_return(errnumber, mach_info->mach_mutex, HI_ERR_HDMI_INVALID_PARA);
                hdmi_unlock_if_null_return(tmp_elem->mach_ctrl.name, mach_info->mach_mutex, HI_FAILURE);
                name_len = osal_strlen(tmp_elem->mach_ctrl.name);
                name_len = (name_len < HDMI_MACH_MAX_NAME_SIZE) ? name_len : (HDMI_MACH_MAX_NAME_SIZE - 1);
                ret = strncpy_s(tmp_elem->mach_run.name, HDMI_MACH_MAX_NAME_SIZE, tmp_elem->mach_ctrl.name, name_len);
                hdmi_unlock_unequal_eok_return(ret, mach_info->mach_mutex, HI_ERR_HDMI_INVALID_PARA);
                tmp_elem->mach_run.name[name_len] = '\0';
                tmp_elem->mach_ctrl.name = tmp_elem->mach_run.name;
                mach_info->total++;
                tmp_elem->mach_run.valid_id = HI_TRUE;
                *mach_id = i;
                break;
            }
        }
        hdmi_mutex_unlock(mach_info->mach_mutex);

        if (i < MACHINE_MAX_ID_NUM) {
            hdmi_info("register new machine id=%u success!\n", *mach_id);
            ret = HI_SUCCESS;
        } else {
            hdmi_info("register new machine id=%u fail!\n", *mach_id);
            ret = HI_FAILURE;
        }
    }

    return ret;
}

hi_s32 hal_hdmi_mach_unregister(hi_u32 mach_id)
{
    hdmi_mach_elem *tmp_elem = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_false_return(mach_info->init, HI_FAILURE);
    hdmi_check_max_return(mach_id, MACHINE_MAX_ID_NUM - 1, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    tmp_elem = &mach_info->mach_elem[mach_id];
    (hi_void)memset_s(&tmp_elem->mach_run, sizeof(tmp_elem->mach_run), 0, sizeof(hdmi_mach_run));
    (hi_void)memset_s(&tmp_elem->mach_ctrl, sizeof(tmp_elem->mach_ctrl), 0, sizeof(hdmi_mach_ctrl));
    mach_info->total--;
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_cfg_get(hi_u32 mach_id, hdmi_mach_ctrl *mach_ctrl)
{
    errno_t errnumber;
    hdmi_mach_elem *tmp_elem = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_null_return(mach_ctrl, HI_FAILURE);
    hdmi_if_false_return(mach_info->init, HI_FAILURE);
    hdmi_check_max_return(mach_id, MACHINE_MAX_ID_NUM - 1, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    tmp_elem = &mach_info->mach_elem[mach_id];
    errnumber = memcpy_s(mach_ctrl, sizeof(hdmi_mach_ctrl), &tmp_elem->mach_ctrl, sizeof(hdmi_mach_ctrl));
    hdmi_unlock_unequal_eok_return(errnumber, mach_info->mach_mutex, HI_ERR_HDMI_INVALID_PARA);
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_cfg_set(hi_u32 mach_id, const hdmi_mach_ctrl *mach_ctrl)
{
    errno_t errnumber;
    hdmi_mach_elem *tmp_elem = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_null_return(mach_ctrl, HI_FAILURE);
    hdmi_if_false_return(mach_info->init, HI_FAILURE);
    hdmi_check_max_return(mach_id, MACHINE_MAX_ID_NUM - 1, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    tmp_elem = &mach_info->mach_elem[mach_id];
    errnumber = memcpy_s(&tmp_elem->mach_ctrl, sizeof(hdmi_mach_ctrl), mach_ctrl, sizeof(hdmi_mach_ctrl));
    hdmi_unlock_unequal_eok_return(errnumber, mach_info->mach_mutex, HI_ERR_HDMI_INVALID_PARA);
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_start(hi_u32 mach_id)
{
    hdmi_mach_elem *tmp_elem = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_false_return(mach_info->init, HI_FAILURE);
    hdmi_check_max_return(mach_id, MACHINE_MAX_ID_NUM - 1, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    tmp_elem = &mach_info->mach_elem[mach_id];
    tmp_elem->mach_run.enable = HI_TRUE;
    tmp_elem->mach_run.enable_time = hdmi_osal_get_time_in_ms();
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_stop(hi_u32 mach_id)
{
    hdmi_mach_elem *tmp_elem = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_false_return(mach_info->init, HI_FAILURE);
    hdmi_check_max_return(mach_id, MACHINE_MAX_ID_NUM - 1, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    tmp_elem = &mach_info->mach_elem[mach_id];
    tmp_elem->mach_run.enable = HI_FALSE;
    tmp_elem->mach_run.disable_time = hdmi_osal_get_time_in_ms();
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_status_get(hdmi_mach_status *status)
{
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_null_return(status, HI_FAILURE);
    hdmi_if_false_return(mach_info->init, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    status->init = mach_info->init;
    status->total = mach_info->total;
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_mach_elem_status_get(hi_u32 mach_id, hdmi_mach_elem_status *status)
{
    errno_t errnumber;
    hdmi_mach_elem *tmp_elem = HI_NULL;
    hdmi_mach_info *mach_info = &g_machine_info;

    hdmi_if_null_return(status, HI_FAILURE);
    hdmi_if_false_return(mach_info->init, HI_FAILURE);
    hdmi_check_max_return(mach_id, MACHINE_MAX_ID_NUM - 1, HI_FAILURE);

    hdmi_mutex_lock(mach_info->mach_mutex);
    tmp_elem = &mach_info->mach_elem[mach_id];
    errnumber = memcpy_s(&status->mach_ctrl, sizeof(hdmi_mach_ctrl), &tmp_elem->mach_ctrl, sizeof(hdmi_mach_ctrl));
    hdmi_unlock_unequal_eok_return(errnumber, mach_info->mach_mutex, HI_ERR_HDMI_INVALID_PARA);
    errnumber = memcpy_s(&status->mach_run, sizeof(hdmi_mach_run), &tmp_elem->mach_run, sizeof(hdmi_mach_run));
    hdmi_unlock_unequal_eok_return(errnumber, mach_info->mach_mutex, HI_ERR_HDMI_INVALID_PARA);
    hdmi_mutex_unlock(mach_info->mach_mutex);

    return HI_SUCCESS;
}

hi_u64 hal_hdmi_mach_ms_get(hi_void)
{
    return (hi_u64)hdmi_osal_get_time_in_ms();
}


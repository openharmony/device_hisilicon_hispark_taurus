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

#include "drv_trng_v200.h"
#include "drv_trng.h"
#include "securec.h"

#ifdef CHIP_TRNG_VER_V200

/* ************************** Internal Structure Definition *************************** */
/* Define the osc sel */
#define TRNG_OSC_SEL                0x02
/* ****************************** API Declaration **************************** */
hi_s32 drv_trng_randnum(hi_u32 *randnum, hi_u32 timeout)
{
    hisec_com_trng_data_st stat;
    hisec_com_trng_ctrl ctrl;
    static hi_u32 last = 0x0A; // 0x0A for check
    hi_u32 times = 0;

    hi_log_func_enter();

    hi_log_chk_param_return(randnum == HI_NULL);

    ctrl.u32 = trng_read(HISEC_COM_TRNG_CTRL);
    if (ctrl.u32 != last) {
        module_enable(CRYPTO_MODULE_ID_TRNG);

        ctrl.bits.mix_enable = 0x00;
        ctrl.bits.drop_enable = 0x00;
        ctrl.bits.pre_process_enable = 0x00;
        ctrl.bits.post_process_enable = 0x00;
        ctrl.bits.post_process_depth = 0x00;
        ctrl.bits.drbg_enable = 0x01;
        ctrl.bits.osc_sel = TRNG_OSC_SEL;
        trng_write(HISEC_COM_TRNG_CTRL, ctrl.u32);
        last = ctrl.u32;
    }

    if (timeout == 0) { /* unblock */
        /* trng number is valid ? */
        stat.u32 = trng_read(HISEC_COM_TRNG_DATA_ST);
        if (stat.bits.trng_fifo_data_cnt == 0) {
            return HI_ERR_CIPHER_NO_AVAILABLE_RNG;
        }
    } else { /* block */
        while (times++ < timeout) {
            /* trng number is valid ? */
            stat.u32 = trng_read(HISEC_COM_TRNG_DATA_ST);
            if (stat.bits.trng_fifo_data_cnt > 0) {
                break;
            }
        }

        /* time out */
        if (times >= timeout) {
            return HI_ERR_CIPHER_NO_AVAILABLE_RNG;
        }
    }

    /* read valid randnum */
    *randnum = trng_read(HISEC_COM_TRNG_FIFO_DATA);
    hi_log_info("randnum: 0x%x\n", *randnum);

    hi_log_func_exit();
    return HI_SUCCESS;
}

void drv_trng_get_capacity(trng_capacity *capacity)
{
    (hi_void)memset_s(capacity, sizeof(trng_capacity), 0,  sizeof(trng_capacity));

    capacity->trng = CRYPTO_CAPACITY_SUPPORT;

    return;
}
#endif /* End of CHIP_TRNG_VER_V200 */

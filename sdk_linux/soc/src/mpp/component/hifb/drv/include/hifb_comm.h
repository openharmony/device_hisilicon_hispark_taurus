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

#ifndef __HIFB_COMM_H__
#define __HIFB_COMM_H__

#include "hi_type.h"
#include "hifb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define hifb_unequal_eok_return(ret)                       \
    do {                                                   \
        if ((ret) != EOK) {                                \
            hifb_error("secure function failure\n");       \
            return HI_FAILURE;                             \
        }                                                  \
    } while (0)

#define hifb_unequal_eok_return_void(ret)                  \
    do {                                                   \
        if ((ret) != EOK) {                                \
            hifb_error("secure function failure\n");       \
            return;                             \
        }                                                  \
    } while (0)

#define hifb_unlock_unequal_eok_return(ret, lock, flag)           \
    do {                                                               \
        if ((ret) != EOK) {                                            \
            hifb_error("secure function failure\n");                   \
            hifb_spin_unlock_irqrestore(lock, flag);              \
            return HI_FAILURE;                                         \
        }                                                              \
    } while (0)

#define hifb_unlock_unequal_eok_return_void(ret, lock, flag)     \
    do {                                                              \
        if ((ret) != EOK) {                                            \
            hifb_error("secure function failure\n");                   \
            hifb_spin_unlock_irqrestore(lock, flag);              \
            return;                                                    \
        }                                                              \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __HIFB_COMM_H__ */

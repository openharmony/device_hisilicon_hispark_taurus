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

#ifndef __MIPI_TX_H__
#define __MIPI_TX_H__

#include "hi_osal.h"
#include "type.h"
#include "hi_mipi_tx.h"

#define mipi_tx_unused(x) (void)(x)

#define hi_mipi_tx_err(x...) \
    do { \
        osal_printk("%s(%d): ", __FUNCTION__, __LINE__); \
        osal_printk(x); \
    } while (0)

#define mipi_tx_check_null_ptr_return(ptr) \
    do { \
        if ((ptr) == NULL) { \
            hi_mipi_tx_err("NULL point \r\n"); \
            return (-1); \
        } \
    } while (0)

typedef struct {
    combo_dev_cfg_t dev_cfg;
} mipi_tx_dev_ctx_t;

#endif

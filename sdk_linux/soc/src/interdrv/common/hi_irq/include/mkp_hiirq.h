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

#ifndef __MKP_HIIRQ_H__
#define __MKP_HIIRQ_H__

#include "hi_type.h"
#include "hi_osal.h"
#include "hi_comm_irq.h"
#include "hi_common.h"
#include "hi_errno.h"

/* define device name */
#define HIIRQ_DEVICE_NAME "hi_irq"

#define IOC_TYPE_IRQ 'Z'
#define MAX_IRQ_NAME_LEN 50

typedef struct {
    unsigned int irq_num;
    hi_int_reg_info reg_info;
} hiirq_set_irq_reg_info;

typedef enum {
    IRQ_TRIG_ONECE = 0,
    IRQ_TRIG_AS_CNT = 1
} irq_trig_mode;

typedef enum {
    IRQ_WAIT_TIMEOUT = 0,
    IRQ_WAIT_FOREVER = 1
} irq_wait_mode;

typedef struct {
    char          irq_name[MAX_IRQ_NAME_LEN];
    unsigned int  irq_num;
    hi_irq_arg    *dev;
    irq_trig_mode irq_mod; // 0,trigger once,1,trigger as count
    irq_wait_mode wait_mode; // 0,wait timeout,1,wait forever
    unsigned int  enable_flag; // 0 disable,1,enable
} hiirq_irq_attr;

#define HI_ERR_IRQ_UNEXIST HI_DEF_ERR(HI_ID_IRQ, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)

#define IRQ_IOC_REQUEST_FREE_IRQ         0x01
#define IRQ_IOC_WAIT_IRQ                 0x02
#define IRQ_IOC_GET_IRQ_NUM              0x03
#define IRQ_IOC_VENC_CACHE_FALG          0x04
#define IRQ_IOC_SET_IRQ_REG              0x05

#define IRQ_REQUEST_OR_FREE_IRQ_CTRL    _IOW(IOC_TYPE_IRQ, IRQ_IOC_REQUEST_FREE_IRQ, hiirq_irq_attr)
#define IRQ_WAIT_IRQ_CTRL               _IOWR(IOC_TYPE_IRQ, IRQ_IOC_WAIT_IRQ, hiirq_irq_attr)
#define IRQ_IOC_GET_IRQ_NUM_CTRL        _IOW(IOC_TYPE_IRQ, IRQ_IOC_GET_IRQ_NUM, hiirq_irq_attr)
#define IRQ_IOC_SET_IRQ_REG_CTRL        _IOW(IOC_TYPE_IRQ, IRQ_IOC_SET_IRQ_REG, hiirq_set_irq_reg_info)


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* End of #ifndef __MKP_HIIRQ_H__ */

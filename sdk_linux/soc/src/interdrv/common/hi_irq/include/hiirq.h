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

#ifndef __HI_IRQ_H__
#define __HI_IRQ_H__

#include <linux/types.h>
#include "hi_type.h"
#include "hi_osal.h"
#include "hi_comm_irq.h"
#include "mkp_hiirq.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int hiirq_init(struct platform_device *pdev);
void hiirq_exit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* End of #ifndef __HI_IRQ_H__ */

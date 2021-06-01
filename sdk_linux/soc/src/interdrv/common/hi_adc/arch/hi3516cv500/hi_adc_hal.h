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

#ifndef __HI_ADC_HAL_H__
#define __HI_ADC_HAL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define LSADC_IRQ_ID        65
#define LSADC_BASE_ADDR     0x120e0000

#define LSADC_CRG_ADDR      0x120101BC
#define ADC_BIT             23

#define LSADC_MAX_CHN_NUM   2
#define LSADC_NUM_BITS      10
#define LSADC_CHN_MASK      0x3

#define GLITCH              0x30
#define TIME_SCAN           0x200

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_ADC_HAL_H */

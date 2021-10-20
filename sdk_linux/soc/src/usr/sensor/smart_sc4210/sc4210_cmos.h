/*
 * Copyright (c) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SC4210_CMOS_H_
#define __SC4210_CMOS_H_

#include "hi_comm_isp.h"
#include "hi_sns_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SC4210_I2C_ADDR    0x60
#define SC4210_ADDR_BYTE   2
#define SC4210_DATA_BYTE   1
#define SC4210_SENSOR_GET_CTX(dev, pstCtx)   ((pstCtx) = sc4210_get_ctx(dev))

ISP_SNS_STATE_S *sc4210_get_ctx(VI_PIPE vi_pipe);
ISP_SNS_COMMBUS_U *sc4210_get_bus_Info(VI_PIPE vi_pipe);

void sc4210_init(VI_PIPE vi_pipe);
void sc4210_exit(VI_PIPE vi_pipe);
void sc4210_standby(VI_PIPE vi_pipe);
void sc4210_restart(VI_PIPE vi_pipe);
int  sc4210_write_register(VI_PIPE vi_pipe, HI_U32 addr, HI_U32 data);
int  sc4210_read_register(VI_PIPE vi_pipe, HI_U32 addr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif /* __sc4210_CMOS_H_ */

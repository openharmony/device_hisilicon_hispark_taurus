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

#ifndef __IMX458_CMOS_H_
#define __IMX458_CMOS_H_

#include "hi_comm_isp.h"
#include "hi_sns_ctrl.h"
#include "imx458_cmos_priv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IMX458_I2C_ADDR    0x34
#define IMX458_ADDR_BYTE   2
#define IMX458_DATA_BYTE   1

#define IMX458_SENSOR_GET_CTX(dev, pstCtx)   ((pstCtx) = imx458_get_ctx(dev))

ISP_SNS_STATE_S *imx458_get_ctx(VI_PIPE vi_pipe);
ISP_SNS_COMMBUS_U *imx458_get_bus_Info(VI_PIPE vi_pipe);
const IMX458_VIDEO_MODE_TBL_S *imx458_get_mode_tb1(HI_U8 u8ImgMode);

void imx458_init(VI_PIPE vi_pipe);
void imx458_exit(VI_PIPE vi_pipe);
void imx458_standby(VI_PIPE vi_pipe);
void imx458_restart(VI_PIPE vi_pipe);
void imx458_mirror_flip(VI_PIPE vi_pipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
int  imx458_write_register(VI_PIPE vi_pipe, HI_U32 addr, HI_U32 data);
int  imx458_read_register(VI_PIPE vi_pipe, HI_U32 addr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif /* __IMX458_CMOS_H_ */

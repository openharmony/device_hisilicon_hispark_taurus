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

#ifndef __IMX335_FORCAR_CMOS_H_
#define __IMX335_FORCAR_CMOS_H_

#include "hi_comm_isp.h"
#include "hi_sns_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IMX335_FORCAR_I2C_ADDR    0x34
#define IMX335_FORCAR_ADDR_BYTE   2
#define IMX335_FORCAR_DATA_BYTE   1
#define IMX335_FORCAR_SENSOR_GET_CTX(dev, pstCtx)   ((pstCtx) = imx335_forcar_get_ctx(dev))

#define IMX335_FULL_LINES_MAX             0xFFFF

#define IMX335_INCREASE_LINES             0 /* make real fps less than stand fps because NVR require */

#define IMX335_VMAX_5M_30FPS_12BIT_LINEAR (0x1194 + IMX335_INCREASE_LINES)
#define IMX335_VMAX_5M_30FPS_10BIT_WDR    (0x1194 + IMX335_INCREASE_LINES)

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ?  (b) : (a))
#endif

typedef enum {
    IMX335_5M_30FPS_12BIT_LINEAR_MODE = 0,
    IMX335_5M_30FPS_10BIT_WDR_MODE,
    IMX335_MODE_BUTT
} IMX335_FORCAR_RES_MODE_E;

typedef struct hiIMX335_FORCAR_VIDEO_MODE_TBL_S {
    HI_U32      u32VertiLines;
    HI_U32      u32MaxVertiLines;
    HI_FLOAT    f32MaxFps;
    HI_FLOAT    f32MinFps;
    HI_U32      u32Width;
    HI_U32      u32Height;
    HI_U8       u8SnsMode;
    WDR_MODE_E  enWDRMode;
    const char *pszModeName;
} IMX335_FORCAR_VIDEO_MODE_TBL_S;

ISP_SNS_COMMBUS_U *imx335_forcar_get_bus_Info(VI_PIPE vi_pipe);
ISP_SNS_STATE_S *imx335_forcar_get_ctx(VI_PIPE vi_pipe);

void IMX335_forcar_init(VI_PIPE vi_pipe);
void IMX335_forcar_exit(VI_PIPE vi_pipe);
void IMX335_forcar_standby(VI_PIPE vi_pipe);
void IMX335_forcar_restart(VI_PIPE vi_pipe);
int  IMX335_forcar_write_register(VI_PIPE vi_pipe, HI_U32 addr, HI_U32 data);
int  IMX335_forcar_read_register(VI_PIPE vi_pipe, HI_U32 addr);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif  /* __IMX335_FORCAR_CMOS_H_ */

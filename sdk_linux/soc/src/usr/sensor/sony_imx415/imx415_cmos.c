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

#include "imx415_cmos.h"
#include "imx415_cmos_ex.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"

#define IMX415_ID  415

ISP_SNS_STATE_S *g_pastImx415[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define IMX415_SENSOR_GET_CTX(dev, pstCtx)   ((pstCtx) = g_pastImx415[dev])
#define IMX415_SENSOR_SET_CTX(dev, pstCtx)   (g_pastImx415[dev] = (pstCtx))
#define IMX415_SENSOR_RESET_CTX(dev)         (g_pastImx415[dev] = HI_NULL)

static HI_U32 g_au32InitExposure[ISP_MAX_PIPE_NUM]  = {0};
static HI_U32 g_au32LinesPer500ms[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16InitWBGain[ISP_MAX_PIPE_NUM][ISP_RGB_CHN_NUM] = {{0}};
static HI_U16 g_au16SampleRgain[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16SampleBgain[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_init_ccm[ISP_MAX_PIPE_NUM][CCM_MATRIX_SIZE] = {{0}};
static HI_BOOL g_quick_start_en[ISP_MAX_PIPE_NUM] = {HI_FALSE};

static HI_BOOL g_abAERouteExValid[ISP_MAX_PIPE_NUM] = {0};
static ISP_AE_ROUTE_S g_astInitAERoute[ISP_MAX_PIPE_NUM] = {{0}};
static ISP_AE_ROUTE_S g_astInitAERouteSF[ISP_MAX_PIPE_NUM] = {{0}};
static ISP_AE_ROUTE_EX_S g_astInitAERouteEx[ISP_MAX_PIPE_NUM] = {{0}};
static ISP_AE_ROUTE_EX_S g_astInitAERouteSFEx[ISP_MAX_PIPE_NUM] = {{0}};

ISP_SNS_COMMBUS_U g_aunImx415BusInfo[ISP_MAX_PIPE_NUM] = {
    [0] = { .s8I2cDev = 0 },
    [1 ... ISP_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1 }
};

static ISP_FSWDR_MODE_E genFSWDRMode[ISP_MAX_PIPE_NUM] = {
    [0 ... ISP_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};

typedef struct hiIMX415_STATE_S {
    HI_U32      u32BRL;
    HI_U32      u32RHS1_MAX;
    HI_U32      u32deltaRHS1;
} IMX415_STATE_S;

IMX415_STATE_S g_astimx415State[ISP_MAX_PIPE_NUM] = {{0}};
static HI_U32 g_au32STimeFps[ISP_MAX_PIPE_NUM] = {[0 ...(ISP_MAX_PIPE_NUM - 1)] = 20}; /* wdr fps 20 */
static HI_U32 g_au32MaxTimeGetCnt[ISP_MAX_PIPE_NUM] = {0};

ISP_SNS_COMMBUS_U *imx415_get_bus_Info(VI_PIPE vi_pipe)
{
    return &g_aunImx415BusInfo[vi_pipe];
}

ISP_SNS_STATE_S *imx415_get_ctx(VI_PIPE vi_pipe)
{
    return g_pastImx415[vi_pipe];
}

const IMX415_VIDEO_MODE_TBL_S g_astImx415ModeTbl[IMX415_MODE_BUTT] = {
    {0x08CA, 0, 2, 8, 30.00, 0.07, 0, "4K2K_10BIT_30FPS"  }, /* 891Mbps 4lane 37.125MHz */
    {0x08F8, 0, 2, 8, 60.00, 0.14, 0,   "2M_12BIT_60FPS"  }, /* 891Mbps 4lane 37.125MHz */
    {0x0B2E, 0, 8, 8, 20.00, 10.2, 2,   "8M_12BIT_20FPS"  }, /* 1440Mbps 4lane 24MHz */
};

const IMX415_VIDEO_MODE_TBL_S *imx415_get_mode_tb1(HI_U8 u8ImgMode)
{
    return &g_astImx415ModeTbl[u8ImgMode];
}

#define IMX415_FULL_LINES_MAX  0xFFFFF
#define IMX415_WDR_FULL_LINES_MAX  0xFFFFF

#define IMX415_ERR_MODE_PRINT(pstSensorImageMode) \
    do { \
        SNS_ERR_TRACE("Not support! Width:%d, Height:%d, Fps:%f, SnsMode:%d\n", \
                      (pstSensorImageMode)->u16Width,  \
                      (pstSensorImageMode)->u16Height, \
                      (pstSensorImageMode)->f32Fps,    \
                      (pstSensorImageMode)->u8SnsMode); \
    } while (0)

static HI_VOID cmos_get_ae_comm_default(VI_PIPE vi_pipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft,
    ISP_SNS_STATE_S *pstSnsState)
{
    HI_U32 u32Fll;
    HI_FLOAT f32MaxFps;
    u32Fll = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    f32MaxFps = g_astImx415ModeTbl[pstSnsState->u8ImgMode].f32MaxFps;

    if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
        pstSnsState->u32FLStd = u32Fll * 2; /* x2 */
        pstAeSnsDft->u32LinesPer500ms = u32Fll * f32MaxFps; /* Because one FSC is equal to 2 VMAX */
        g_astimx415State[vi_pipe].u32RHS1_MAX = g_astimx415State[vi_pipe].u32BRL * 2; /* x2 */
        g_astimx415State[vi_pipe].u32deltaRHS1 =
            (u32Fll * 2) - (g_astimx415State[vi_pipe].u32BRL * 2) - 2; /* Lines * 2 - BRL * 2 - 2 */
    } else {
        pstSnsState->u32FLStd = u32Fll;
        pstAeSnsDft->u32LinesPer500ms = u32Fll * f32MaxFps / 2; /* div 2 */
    }

    pstAeSnsDft->stIntTimeAccu.f32Offset = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32Offset;
    pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit;

    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    pstAeSnsDft->u32HmaxTimes = (1000000000) / SNS_DIV_0_TO_1_FLOAT(u32Fll * f32MaxFps); /* 1000000000ns */

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

    pstAeSnsDft->u32ISPDgainShift = 8; /* accuracy: 8 */
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift; /* max 2 */

    (hi_void)memcpy_s(&pstAeSnsDft->stPirisAttr, sizeof(ISP_PIRIS_ATTR_S),
                      &g_gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    pstAeSnsDft->bAERouteExValid = HI_FALSE;
    pstAeSnsDft->stAERouteAttr.u32TotalNum = 0;
    pstAeSnsDft->stAERouteAttrEx.u32TotalNum = 0;
    if (g_au32InitExposure[vi_pipe] == 0) {
        pstAeSnsDft->u32InitExposure = 1000000; /* init 1000000 */
    } else {
        pstAeSnsDft->u32InitExposure = g_au32InitExposure[vi_pipe];
    }

    if (g_au32LinesPer500ms[vi_pipe] == 0) {
        pstAeSnsDft->u32LinesPer500ms = ((HI_U64)(u32Fll * f32MaxFps)) >> 1;
    } else {
        pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[vi_pipe];
    }
    pstAeSnsDft->stQuickStart.bQuickStartEn = g_quick_start_en[vi_pipe];
    pstAeSnsDft->stQuickStart.u8BlackFrameNum = 0;
    return;
}

static HI_VOID cmos_get_ae_linear_default(VI_PIPE vi_pipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft,
    ISP_SNS_STATE_S *pstSnsState)
{
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60; /* index 2 */
    pstAeSnsDft->au8HistThresh[3] = 0x80; /* index 3 */

    pstAeSnsDft->u8AeCompensation = 0x2D;
    pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;

    pstAeSnsDft->u32MinIntTime = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32ExpLineMin;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535; /* max 65535 */
    pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

    pstAeSnsDft->u32MaxAgain = IMX415_AGAIN_MAX;
    pstAeSnsDft->u32MinAgain = IMX415_AGAIN_MIN;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->u32MaxDgain = IMX415_DGAIN_MAX;  /*  if Dgain enable,please set ispdgain bigger than 1 */
    pstAeSnsDft->u32MinDgain = IMX415_DGAIN_MIN;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
    pstAeSnsDft->bAERouteExValid = g_abAERouteExValid[vi_pipe];
    (hi_void)memcpy_s(&pstAeSnsDft->stAERouteAttr, sizeof(ISP_AE_ROUTE_S),
                      &g_astInitAERoute[vi_pipe], sizeof(ISP_AE_ROUTE_S));
    (hi_void)memcpy_s(&pstAeSnsDft->stAERouteAttrEx, sizeof(ISP_AE_ROUTE_EX_S),
                      &g_astInitAERouteEx[vi_pipe], sizeof(ISP_AE_ROUTE_EX_S));
    return;
}

static HI_VOID cmos_get_ae_2to1_line_wdr_default(VI_PIPE vi_pipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft,
    ISP_SNS_STATE_S *pstSnsState)
{
    HI_U32 u32Fll = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    HI_U32 u32MaxFps = 20;

    pstSnsState->u32FLStd = u32Fll * u32MaxFps / SNS_DIV_0_TO_1(g_au32STimeFps[vi_pipe]);

    if ((pstSnsState->u32FLStd % 4) != 0) { /* 4 */
        pstSnsState->u32FLStd = pstSnsState->u32FLStd - (pstSnsState->u32FLStd % 4) + 4; /* 4 */
    }
    pstSnsState->u32FLStd = pstSnsState->u32FLStd * 2; /* 2 */

    pstAeSnsDft->au8HistThresh[0] = 0xc;
    pstAeSnsDft->au8HistThresh[1] = 0x18;
    pstAeSnsDft->au8HistThresh[2] = 0x60; /* index 2 */
    pstAeSnsDft->au8HistThresh[3] = 0x80; /* index 3 */

    pstAeSnsDft->u8AeCompensation = 0x38;

    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 2; /* Accuracy 2 */
    pstAeSnsDft->u32MinIntTime = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32ExpLineMin;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535; /* max 65535 */
    pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;
    pstAeSnsDft->stIntTimeAccu.f32Offset = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32Offset;

    pstAeSnsDft->u32MaxIntTimeStep = 1000; /* MaxIntTimeStep 1000 */
    pstAeSnsDft->u32LFMinExposure = 15000000; /* LFMinExposure 15000000 */
    pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;

    pstAeSnsDft->u32MaxAgain = IMX415_AGAIN_MAX;
    pstAeSnsDft->u32MinAgain = IMX415_AGAIN_MIN;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->u32MaxDgain = IMX415_DGAIN_MAX;  /* if Dgain enable,please set ispdgain bigger than 1 */
    pstAeSnsDft->u32MinDgain = IMX415_DGAIN_MIN;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift; /* max 16 << shift */
    pstAeSnsDft->bDiffGainSupport = HI_FALSE;

    pstAeSnsDft->u32ExpRatioMin = 0x80;  /* The exposure ration from 2x to convergence */

    if (genFSWDRMode[vi_pipe] != ISP_FSWDR_LONG_FRAME_MODE) {
        pstAeSnsDft->u16ManRatioEnable = HI_TRUE;
        pstAeSnsDft->au32Ratio[0] = 0x400;
        pstAeSnsDft->au32Ratio[1] = 0x40;
        pstAeSnsDft->au32Ratio[2] = 0x40; /* array index 2 */
    }
    pstAeSnsDft->bAERouteExValid = g_abAERouteExValid[vi_pipe];
    memcpy_s(&pstAeSnsDft->stAERouteAttr, sizeof(ISP_AE_ROUTE_S), &g_astInitAERoute[vi_pipe],  sizeof(ISP_AE_ROUTE_S));
    memcpy_s(&pstAeSnsDft->stAERouteAttrEx, sizeof(ISP_AE_ROUTE_EX_S),
        &g_astInitAERouteEx[vi_pipe],  sizeof(ISP_AE_ROUTE_EX_S));
    memcpy_s(&pstAeSnsDft->stAERouteSFAttr, sizeof(ISP_AE_ROUTE_S),
        &g_astInitAERouteSF[vi_pipe], sizeof(ISP_AE_ROUTE_S));
    memcpy_s(&pstAeSnsDft->stAERouteSFAttrEx, sizeof(ISP_AE_ROUTE_EX_S),
        &g_astInitAERouteSFEx[vi_pipe], sizeof(ISP_AE_ROUTE_EX_S));
    return;
}

static HI_S32 cmos_get_ae_default(VI_PIPE vi_pipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    cmos_get_ae_comm_default(vi_pipe, pstAeSnsDft, pstSnsState);

    switch (pstSnsState->enWDRMode) {
        default:
        case WDR_MODE_NONE:
            cmos_get_ae_linear_default(vi_pipe, pstAeSnsDft, pstSnsState);
            break;

        case WDR_MODE_2To1_LINE:
            /* FS WDR mode */
            cmos_get_ae_2to1_line_wdr_default(vi_pipe, pstAeSnsDft, pstSnsState);
            break;
    }
    return HI_SUCCESS;
}

static HI_VOID cmos_config_vmax(ISP_SNS_STATE_S *pstSnsState, HI_U32 u32VMAX)
{
    if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = IMX415_LOW_8BITS(u32VMAX); /* index 5 */
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = IMX415_MID_8BITS(u32VMAX); /* index 6 */
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = IMX415_HIG_4BITS(u32VMAX); /* index 7 */
    } else {
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = IMX415_LOW_8BITS(u32VMAX); /* index 8 */
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = IMX415_MID_8BITS(u32VMAX); /* index 9 */
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = IMX415_HIG_4BITS(u32VMAX); /* index 10 */
    }

    return;
}

/* the function of sensor set fps */
static HI_VOID cmos_fps_set(VI_PIPE vi_pipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_FLOAT f32MaxFps, f32MinFps;
    HI_U32 u32Lines, u32VMAX;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    f32MaxFps = g_astImx415ModeTbl[pstSnsState->u8ImgMode].f32MaxFps;
    f32MinFps = g_astImx415ModeTbl[pstSnsState->u8ImgMode].f32MinFps;
    u32Lines = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    /* SHR 16bit, So limit full_lines as 0xFFFF */
    if (f32Fps > f32MaxFps || f32Fps < f32MinFps) {
        SNS_ERR_TRACE("Not support Fps: %f\n", f32Fps);
        return;
    }

    u32VMAX = u32Lines * f32MaxFps / SNS_DIV_0_TO_1_FLOAT(f32Fps);
    u32VMAX = (u32VMAX > IMX415_FULL_LINES_MAX) ? IMX415_FULL_LINES_MAX : u32VMAX;
    if (pstSnsState->u8ImgMode == IMX415_8M_20FPS_12BIT_WDR_MODE) {
        u32VMAX = ((u32VMAX % 4) != 0) ? (u32VMAX - (u32VMAX % 4) + 4) : u32VMAX; /* u32VMAX % 4 */
    }

    cmos_config_vmax(pstSnsState, u32VMAX);

    if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
        pstSnsState->u32FLStd = u32VMAX * 2; /* x2 */
        pstAeSnsDft->u32LinesPer500ms = u32Lines * f32MaxFps; /* Because one FSC is equal to 2 VMAX */
        g_astimx415State[vi_pipe].u32RHS1_MAX = g_astimx415State[vi_pipe].u32BRL * 2; /* x2 */
        g_astimx415State[vi_pipe].u32deltaRHS1 =
            (u32VMAX * 2) - (g_astimx415State[vi_pipe].u32BRL * 2) - 2; /* VMAX * 2 - BRL * 2 - 2 */
    } else {
        pstSnsState->u32FLStd = u32VMAX;
        pstAeSnsDft->u32LinesPer500ms = u32Lines * f32MaxFps / 2; /* div 2 */
    }

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32HmaxTimes =
        (1000000000) / SNS_DIV_0_TO_1_FLOAT(pstSnsState->u32FLStd * f32Fps); /* 1000000000ns */

    return;
}

static HI_VOID cmos_slow_framerate_set(VI_PIPE vi_pipe, HI_U32 u32FullLines,
                                       AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
        u32FullLines = (u32FullLines > (2 * 2 * g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax)) ? /* x2 x2 */
            (2 * 2 * g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax) : u32FullLines; /* x2 x2 */
        if ((u32FullLines % 4) != 0) { /* %4 */
            u32FullLines = u32FullLines + (4 - (u32FullLines % 4));  /* FL + 4 - FL % 4 */
        }
        pstSnsState->au32FL[0] = u32FullLines;
        g_astimx415State[vi_pipe].u32RHS1_MAX = g_astimx415State[vi_pipe].u32BRL * 2; /* x2 */
        g_astimx415State[vi_pipe].u32deltaRHS1 =
            u32FullLines - (g_astimx415State[vi_pipe].u32BRL * 2) - 2; /* FL - BRL * 2 - 2 */
    } else {
        u32FullLines = (u32FullLines > IMX415_FULL_LINES_MAX) ? IMX415_FULL_LINES_MAX : u32FullLines;
        pstSnsState->au32FL[0] = u32FullLines;
    }

    if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = IMX415_LOW_8BITS(pstSnsState->au32FL[0]); /* index 5 */
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = IMX415_MID_8BITS(pstSnsState->au32FL[0]); /* index 6 */
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = IMX415_HIG_4BITS(pstSnsState->au32FL[0]); /* index 7 */
    } else {
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = IMX415_LOW_8BITS(pstSnsState->au32FL[0] >> 1); /* index 8 */
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = IMX415_MID_8BITS(pstSnsState->au32FL[0] >> 1); /* index 9 */
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = /* index 10 */
            IMX415_HIG_4BITS(pstSnsState->au32FL[0] >> 1);
    }

    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32MaxIntTime = pstSnsState->au32FL[0] - g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit;

    return;
}

static HI_VOID cmos_inttime_update_linear(VI_PIPE vi_pipe, HI_U32 u32IntTime)
{
    HI_U32 u32Value;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32Value = pstSnsState->au32FL[0] - u32IntTime;
    u32Value = MIN(u32Value, IMX415_FULL_LINES_MAX);
    u32Value = MIN(MAX(u32Value, 9), (pstSnsState->au32FL[0] - 1)); /* max 9 */

    pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = IMX415_LOW_8BITS(u32Value);   /* SHR0 */
    pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = IMX415_MID_8BITS(u32Value);
    pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = IMX415_HIG_4BITS(u32Value); /* index 2 */

    return;
}

static HI_VOID cmos_inttime_update_reg_2to1_line(HI_U32 u32SHR1, HI_U32 u32RHS1, HI_U32 u32SHR0,
    ISP_SNS_STATE_S *pstSnsState)
{
    pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = IMX415_LOW_8BITS(u32SHR0);
    pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = IMX415_MID_8BITS(u32SHR0);
    pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = IMX415_HIG_4BITS(u32SHR0); /* index 2 */

    pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = IMX415_LOW_8BITS(u32SHR1); /* index 5 */
    pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = IMX415_MID_8BITS(u32SHR1); /* index 6 */
    pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = IMX415_HIG_4BITS(u32SHR1); /* index 7 */

    pstSnsState->astRegsInfo[0].astI2cData[11].u32Data = IMX415_LOW_8BITS(u32RHS1); /* index 11 */
    pstSnsState->astRegsInfo[0].astI2cData[12].u32Data = IMX415_MID_8BITS(u32RHS1); /* index 12 */
    pstSnsState->astRegsInfo[0].astI2cData[13].u32Data = IMX415_HIG_4BITS(u32RHS1); /* index 13 */
    return;
}

static HI_VOID cmos_inttime_update_2to1_line_long(VI_PIPE vi_pipe, HI_U32 u32IntTime)
{
    HI_U32 u32RHS1, u32SHR0, u32SHR1, u32RHS1Limit, u32ShortExpMod4;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    static HI_U32 u32PreRHS1 = 0;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->au32WDRIntTime[1] = u32IntTime;
    u32SHR0 = pstSnsState->au32FL[1] - u32IntTime + (u32IntTime % 2); /* IntTime % 2 */
    u32ShortExpMod4 = pstSnsState->au32WDRIntTime[0] % 4; /* au32WDRIntTime % 4 */

    u32SHR1 = 9; /* min 9 */
    u32RHS1 = 9 + pstSnsState->au32WDRIntTime[0] - u32ShortExpMod4; /* min 9 */

    u32RHS1Limit =  pstSnsState->au32WDRIntTime[0] - (IMX415_WDR_BRL * 2) - 2; /* 2 from sensor limit */

    if (u32PreRHS1 > u32RHS1) {
        if ((u32PreRHS1 - u32RHS1) > u32RHS1Limit) {
            u32RHS1 = u32PreRHS1 - (u32RHS1Limit - (u32RHS1Limit % 4)); /* 4x */
        }
    } else {
        if ((u32RHS1 - u32PreRHS1) > u32RHS1Limit) {
            u32RHS1 = u32PreRHS1 + (u32RHS1Limit - (u32RHS1Limit % 4)); /* 4x */
        }
    }

    if ((HI_S32)(u32SHR0 - u32PreRHS1) < 9) { /* difference must > 9 */
        u32SHR0 = u32PreRHS1 + 9; /* difference must > 9 */
    }

    pstSnsState->au32WDRIntTime[0] = u32RHS1 - u32SHR1;
    pstSnsState->au32WDRIntTime[1] = pstSnsState->au32FL[0] - u32SHR0;
    cmos_inttime_update_reg_2to1_line(u32SHR1, u32RHS1, u32SHR0, pstSnsState);

    u32PreRHS1 = u32RHS1;
    return;
}

static HI_VOID cmos_inttime_update_2to1_line(VI_PIPE vi_pipe, HI_U32 u32IntTime)
{
    static HI_BOOL bFirst[ISP_MAX_PIPE_NUM] = { [0 ...(ISP_MAX_PIPE_NUM - 1)] = 1 };

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (bFirst[vi_pipe]) { /* short exposure */
        pstSnsState->au32WDRIntTime[0] = u32IntTime;
        bFirst[vi_pipe] = HI_FALSE;
    } else { /* long exposure */
        cmos_inttime_update_2to1_line_long(vi_pipe, u32IntTime);
        bFirst[vi_pipe] = HI_TRUE;
    }

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID cmos_inttime_update(VI_PIPE vi_pipe, HI_U32 u32IntTime)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
        cmos_inttime_update_2to1_line(vi_pipe, u32IntTime);
    } else {
        cmos_inttime_update_linear(vi_pipe, u32IntTime);
    }

    return;
}

/* Again and Dgain use the same table, Step is 0.3db */
static HI_U32 g_ad_gain_table[IMX415_AD_GAIN_TBL_RANGE] = {
    1024,   1059,   1097,   1135,   1175,   1217,   1259,   1304,   1349,   1397,   1446,   1497,   1549,   1604,
    1660,   1719,   1779,   1842,   1906,   1973,   2043,   2114,   2189,   2266,   2345,   2428,   2513,   2601,
    2693,   2788,   2886,   2987,   3092,   3201,   3313,   3430,   3550,   3675,   3804,   3938,   4076,   4219,
    4368,   4521,   4680,   4845,   5015,   5191,   5374,   5562,   5758,   5960,   6170,   6387,   6611,   6843,
    7084,   7333,   7591,   7857,   8133,   8419,   8715,   9021,   9338,   9667,  10006,  10358,  10722,  11099,
    11489,  11893,  12311,  12743,  13191,  13655,  14135,  14631,  15146,  15678,  16229,  16799,  17390,  18001,
    18633,  19288,  19966,  20668,  21394,  22146,  22924,  23730,  24564,  25427,  26320,  27245,  28203,  29194,
    30220,  31282,  32381,  41519,  34697,  35917,  37179,  38485,  39838,  41238,  42687,  44187,  45740,  47347,
    49011,  50734,  52517,  54362,  56273,  58250,  60297,  62416,  64610,  66880,  69230,  71663,  74182,  76789,
    79487,  82281,  85172,  88165,  91264,  94471,  97791, 101228, 104785, 108468, 112279, 116225, 120310, 124537,
    128914, 133444, 138134, 142988, 148013, 153215, 158599, 164172, 169942, 175914, 182096, 188495, 195119, 201976,
    209074, 216421, 224027, 231900, 240049, 248485, 257217, 266256, 275613, 285299, 295325, 305703, 316446, 327567,
    339078, 350994, 363329, 376097, 389314, 402995, 417157, 431817, 446992, 462700, 478961, 495793, 513216, 531251,
    549921, 569246, 589250, 609958, 631393, 653582, 676550, 700326, 724936, 750412, 776783, 804081, 832338, 861589,
    891867, 923209, 955652, 989236, 1024000, 1059985, 1097236, 1135795, 1175709, 1217026, 1259795, 1304067, 1349895,
    1397333, 1446438, 1497269, 1549887, 1604353, 1660734, 1719095, 1779508, 1842044, 1906777, 1973786, 2043149,
    2114949, 2189273, 2266209,  2345848, 2428287, 2513622, 2601956, 2693394, 2788046, 2886024, 2987445, 3092431,
    3201105, 3313599, 3430046, 3550585, 3675361, 3804521, 3938220, 4076617
};

static HI_VOID cmos_again_calc_table(VI_PIPE vi_pipe, HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32AgainLin);
    CMOS_CHECK_POINTER_VOID(pu32AgainDb);
    hi_unused(vi_pipe);

    if (*pu32AgainLin >= g_ad_gain_table[IMX415_AGAIN_TBL_RANGE]) {
        *pu32AgainLin = g_ad_gain_table[IMX415_AGAIN_TBL_RANGE];
        *pu32AgainDb = IMX415_AGAIN_TBL_RANGE;
    } else {
        for (i = 1; i < (IMX415_AGAIN_TBL_RANGE + 1); i++) {
            if (*pu32AgainLin < g_ad_gain_table[i]) {
                *pu32AgainLin = g_ad_gain_table[i - 1];
                *pu32AgainDb = i - 1;
                break;
            }
        }
    }

    return;
}

static HI_VOID cmos_dgain_calc_table(VI_PIPE vi_pipe, HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32DgainLin);
    CMOS_CHECK_POINTER_VOID(pu32DgainDb);
    hi_unused(vi_pipe);

    if (*pu32DgainLin >= g_ad_gain_table[IMX415_DGAIN_TBL_RANGE]) {
        *pu32DgainLin = g_ad_gain_table[IMX415_DGAIN_TBL_RANGE];
        *pu32DgainDb = IMX415_DGAIN_TBL_RANGE;
    } else {
        for (i = 1; i < (IMX415_DGAIN_TBL_RANGE + 1); i++) {
            if (*pu32DgainLin < g_ad_gain_table[i]) {
                *pu32DgainLin = g_ad_gain_table[i - 1];
                *pu32DgainDb = i - 1;
                break;
            }
        }
    }

    return;
}

static HI_VOID cmos_gains_update(VI_PIPE vi_pipe, HI_U32 u32Again, HI_U32 u32Dgain)
{
    static HI_BOOL bFirstGain[ISP_MAX_PIPE_NUM] = { [0 ...(ISP_MAX_PIPE_NUM - 1)] = 1 };
    HI_U32 u32Tmp;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
        if (bFirstGain[vi_pipe]) {
            u32Tmp = u32Again + u32Dgain;
            pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = IMX415_LOW_8BITS(u32Tmp); /* index 3 */
            pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = IMX415_MID_8BITS(u32Tmp); /* index 4 */
            pstSnsState->astRegsInfo[0].astI2cData[14].u32Data = IMX415_LOW_8BITS(u32Tmp); /* index 14 */
            pstSnsState->astRegsInfo[0].astI2cData[15].u32Data = IMX415_MID_8BITS(u32Tmp); /* index 15 */
            bFirstGain[vi_pipe] = HI_FALSE;
        } else {
            u32Tmp = u32Again + u32Dgain;
            pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = IMX415_LOW_8BITS(u32Tmp); /* index 3 */
            pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = IMX415_MID_8BITS(u32Tmp); /* index 4 */
            pstSnsState->astRegsInfo[0].astI2cData[14].u32Data = IMX415_LOW_8BITS(u32Tmp); /* index 14 */
            pstSnsState->astRegsInfo[0].astI2cData[15].u32Data = IMX415_MID_8BITS(u32Tmp); /* index 15 */
            bFirstGain[vi_pipe] = HI_TRUE;
        }
    } else {
        u32Tmp = u32Again + u32Dgain;
        pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = IMX415_LOW_8BITS(u32Tmp); /* index 3 */
        pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = IMX415_MID_8BITS(u32Tmp); /* index 4 */
    }

    return;
}

static HI_U32 u32LastIntTimeMaxTmp[ISP_MAX_PIPE_NUM] = { [0 ...(ISP_MAX_PIPE_NUM - 1)] = 4 }; /* LastIntTimeMaxTmp 4 */
static HI_VOID cmos_get_inttime_max_2to1_line_long_frame(VI_PIPE vi_pipe,
    HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax,
    HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 u32ShortMax0, u32ShortMax, u32ShortTimeMinLimit;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);
    CMOS_CHECK_POINTER_VOID(au32Ratio);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);

    u32ShortTimeMinLimit = 8; /* u32ShortTimeMinLimit 8 */

    u32ShortMax0 = pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]; /* sub 50 */
    u32ShortMax = pstSnsState->au32FL[0] - 50; /* sub 50 */
    u32ShortMax = (u32ShortMax0 < u32ShortMax) ? u32ShortMax0 : u32ShortMax;
    au32IntTimeMax[0] = u32ShortMax;
    au32IntTimeMin[0] = u32ShortTimeMinLimit;
    u32LastIntTimeMaxTmp[vi_pipe] = u32ShortTimeMinLimit;

    return;
}

static HI_VOID cmos_get_inttime_max_2to1_line_auto_long_frame(VI_PIPE vi_pipe, HI_U32 *au32Ratio,
    HI_U32 *au32IntTimeMax, HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 u32ShortMax0, u32ShortMax, u32ShortTimeMinLimit, u32delta, u32deltaRHS1;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);

    u32ShortTimeMinLimit = 8; /* u32ShortTimeMinLimit 8 */
    u32deltaRHS1 = g_astimx415State[vi_pipe].u32deltaRHS1 - (g_astimx415State[vi_pipe].u32deltaRHS1 % 4); /* RHS % 4 */

    if (pstSnsState->au32WDRIntTime[0] == u32ShortTimeMinLimit && au32Ratio[0] == 0x40) {
        u32ShortMax0 = pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]; /* sub 50 */
        u32ShortMax = pstSnsState->au32FL[0] - 50; /* sub 50 */
        u32ShortMax = (u32ShortMax0 < u32ShortMax) ? u32ShortMax0 : u32ShortMax;
        au32IntTimeMax[0] = u32ShortMax;
        au32IntTimeMin[0] = u32ShortTimeMinLimit;
        u32LastIntTimeMaxTmp[vi_pipe] = u32ShortTimeMinLimit;
        return;
    }
    u32ShortMax0 = ((pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]) * 0x40) / /* sub 50 */
        SNS_DIV_0_TO_1(au32Ratio[0]);
    u32ShortMax = ((pstSnsState->au32FL[0] - 50) * 0x40) / SNS_DIV_0_TO_1(au32Ratio[0] + 0x40); /* sub 50 */
    u32ShortMax = (u32ShortMax0 < u32ShortMax) ? u32ShortMax0 : u32ShortMax;
    u32ShortMax = (u32ShortMax > (g_astimx415State[vi_pipe].u32RHS1_MAX - 9)) ? /* sub 9 */
        (g_astimx415State[vi_pipe].u32RHS1_MAX - 9) : u32ShortMax; /* sub 9 */
    if (u32ShortMax > u32LastIntTimeMaxTmp[vi_pipe]) {
        u32delta = u32ShortMax - u32LastIntTimeMaxTmp[vi_pipe];
        if (u32delta > g_astimx415State[vi_pipe].u32deltaRHS1) {
            u32ShortMax = u32LastIntTimeMaxTmp[vi_pipe] + u32deltaRHS1;
        }
    } else if (u32LastIntTimeMaxTmp[vi_pipe] > u32ShortMax) {
        u32delta = u32LastIntTimeMaxTmp[vi_pipe] - u32ShortMax;
        if (u32delta > g_astimx415State[vi_pipe].u32deltaRHS1) {
            u32ShortMax = u32LastIntTimeMaxTmp[vi_pipe] - u32deltaRHS1;
        }
    }
    u32LastIntTimeMaxTmp[vi_pipe] = u32ShortMax ? u32ShortMax : 1;

    if (u32ShortMax >= u32ShortTimeMinLimit) {
        au32IntTimeMax[0] = u32ShortMax;
        au32IntTimeMax[1] = (au32IntTimeMax[0] * au32Ratio[0]) >> 6; /* shift 6 */
        au32IntTimeMin[0] = u32ShortTimeMinLimit;
        au32IntTimeMin[1] = (au32IntTimeMin[0] * au32Ratio[0]) >> 6; /* shift 6 */
        return;
    }

    u32ShortMax = u32ShortTimeMinLimit;
    au32IntTimeMax[0] = u32ShortMax;
    au32IntTimeMax[1] = (au32IntTimeMax[0] * 0xFFF) >> 6; /* shift 6 */
    au32IntTimeMin[0] = au32IntTimeMax[0];
    au32IntTimeMin[1] = au32IntTimeMax[1];

    return;
}

static HI_VOID cmos_get_inttime_max_2to1_line_normal(VI_PIPE vi_pipe,
    HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax,
    HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 u32ShortMax0, u32ShortMax, u32ShortTimeMinLimit;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32ShortTimeMinLimit = 8; /* u32ShortTimeMinLimit 8 */

    u32ShortMax0 = ((pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]) * 0x40) / /* sub 50 */
        SNS_DIV_0_TO_1(au32Ratio[0]);
    u32ShortMax = ((pstSnsState->au32FL[0] - 50) * 0x40) / SNS_DIV_0_TO_1(au32Ratio[0] + 0x40); /* sub 50 */
    u32ShortMax = (u32ShortMax0 < u32ShortMax) ? u32ShortMax0 : u32ShortMax;
    u32ShortMax = (u32ShortMax > (g_astimx415State[vi_pipe].u32RHS1_MAX - 9)) ? /* sub 9 */
        (g_astimx415State[vi_pipe].u32RHS1_MAX - 9) : u32ShortMax; /* sub 9 */
    u32ShortMax = (u32ShortMax == 0) ? 1 : u32ShortMax;
    u32LastIntTimeMaxTmp[vi_pipe] = u32ShortMax;
    *pu32LFMaxIntTime = g_astimx415State[vi_pipe].u32RHS1_MAX - 9; /* sub 9 */

    if (u32ShortMax >= u32ShortTimeMinLimit) {
        au32IntTimeMax[0] = u32ShortMax;
        au32IntTimeMax[1] = (au32IntTimeMax[0] * au32Ratio[0]) >> 6; /* shift 6 */
        au32IntTimeMin[0] = u32ShortTimeMinLimit;
        au32IntTimeMin[1] = (au32IntTimeMin[0] * au32Ratio[0]) >> 6; /* shift 6 */
        return;
    }

    u32ShortMax = u32ShortTimeMinLimit;
    au32IntTimeMax[0] = u32ShortMax;
    au32IntTimeMax[1] = (au32IntTimeMax[0] * 0xFFF) >> 6; /* shift 6 */
    au32IntTimeMin[0] = au32IntTimeMax[0];
    au32IntTimeMin[1] = au32IntTimeMax[1];

    return;
}

static HI_VOID cmos_get_inttime_max_2to1_line(VI_PIPE vi_pipe,
    HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax,
    HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    switch (genFSWDRMode[vi_pipe]) {
        case ISP_FSWDR_LONG_FRAME_MODE:
            cmos_get_inttime_max_2to1_line_long_frame(vi_pipe, au32Ratio, au32IntTimeMax, au32IntTimeMin,
                pu32LFMaxIntTime);
            break;
        case ISP_FSWDR_AUTO_LONG_FRAME_MODE:
            cmos_get_inttime_max_2to1_line_auto_long_frame(vi_pipe, au32Ratio, au32IntTimeMax, au32IntTimeMin,
                pu32LFMaxIntTime);
            break;
        case ISP_FSWDR_NORMAL_MODE:
            cmos_get_inttime_max_2to1_line_normal(vi_pipe, au32Ratio, au32IntTimeMax, au32IntTimeMin,
                pu32LFMaxIntTime);
            break;
        default:
            break;
    }

    return;
}

static HI_VOID cmos_get_inttime_max(VI_PIPE vi_pipe, HI_U16 u16ManRatioEnable,
    HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax,
    HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    CMOS_CHECK_POINTER_VOID(au32Ratio);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMax);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMin);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);
    hi_unused(u16ManRatioEnable);

    switch (pstSnsState->enWDRMode) {
        case WDR_MODE_2To1_LINE:
            cmos_get_inttime_max_2to1_line(vi_pipe, au32Ratio, au32IntTimeMax, au32IntTimeMin, pu32LFMaxIntTime);
            break;
        default:
            break;
    }

    return;
}

/* Only used in FSWDR mode */
static HI_VOID cmos_ae_fswdr_attr_set(VI_PIPE vi_pipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr)
{
    CMOS_CHECK_POINTER_VOID(pstAeFSWDRAttr);

    genFSWDRMode[vi_pipe] = pstAeFSWDRAttr->enFSWDRMode;
    g_au32MaxTimeGetCnt[vi_pipe] = 0;
    return;
}

static HI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    CMOS_CHECK_POINTER(pstExpFuncs);

    (hi_void)memset_s(pstExpFuncs, sizeof(AE_SENSOR_EXP_FUNC_S), 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
    pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
    pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

    return HI_SUCCESS;
}

/* AWB default parameter and function */
#define CALIBRATE_STATIC_TEMP      4983
#define CALIBRATE_STATIC_WB_R_GAIN 440
#define CALIBRATE_STATIC_WB_GR_GAIN 256
#define CALIBRATE_STATIC_WB_GB_GAIN 256
#define CALIBRATE_STATIC_WB_B_GAIN 559

/* Calibration results for Auto WB Planck */
#define CALIBRATE_AWB_P1 8
#define CALIBRATE_AWB_P2 217
#define CALIBRATE_AWB_Q1 (-31)
#define CALIBRATE_AWB_A1 196503
#define CALIBRATE_AWB_B1 128
#define CALIBRATE_AWB_C1 (-144504)

/* Rgain and Bgain of the golden sample */
#define GOLDEN_RGAIN 0
#define GOLDEN_BGAIN 0
static HI_S32 cmos_get_awb_default(VI_PIPE vi_pipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_S32 i;

    CMOS_CHECK_POINTER(pstAwbSnsDft);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    (hi_void)memset_s(pstAwbSnsDft, sizeof(AWB_SENSOR_DEFAULT_S), 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = CALIBRATE_STATIC_TEMP;

    pstAwbSnsDft->au16GainOffset[0] = CALIBRATE_STATIC_WB_R_GAIN;
    pstAwbSnsDft->au16GainOffset[1] = CALIBRATE_STATIC_WB_GR_GAIN;
    pstAwbSnsDft->au16GainOffset[2] = CALIBRATE_STATIC_WB_GB_GAIN; /* index 2 */
    pstAwbSnsDft->au16GainOffset[3] = CALIBRATE_STATIC_WB_B_GAIN; /* index 3 */

    pstAwbSnsDft->as32WbPara[0] = CALIBRATE_AWB_P1;
    pstAwbSnsDft->as32WbPara[1] = CALIBRATE_AWB_P2;
    pstAwbSnsDft->as32WbPara[2] = CALIBRATE_AWB_Q1; /* index 2 */
    pstAwbSnsDft->as32WbPara[3] = CALIBRATE_AWB_A1; /* index 3 */
    pstAwbSnsDft->as32WbPara[4] = CALIBRATE_AWB_B1; /* index 4 */
    pstAwbSnsDft->as32WbPara[5] = CALIBRATE_AWB_C1; /* index 5 */
    pstAwbSnsDft->u16GoldenRgain = GOLDEN_RGAIN;
    pstAwbSnsDft->u16GoldenBgain = GOLDEN_BGAIN;

    switch (pstSnsState->enWDRMode) {
        default:
        case WDR_MODE_NONE:
            (hi_void)memcpy_s(&pstAwbSnsDft->stCcm, sizeof(AWB_CCM_S), &g_stAwbCcm, sizeof(AWB_CCM_S));
            (hi_void)memcpy_s(&pstAwbSnsDft->stAgcTbl, sizeof(AWB_AGC_TABLE_S),
                              &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
            break;

        case WDR_MODE_2To1_LINE:
            (hi_void)memcpy_s(&pstAwbSnsDft->stCcm, sizeof(AWB_CCM_S), &g_stAwbCcmFsWdr, sizeof(AWB_CCM_S));
            (hi_void)memcpy_s(&pstAwbSnsDft->stAgcTbl, sizeof(AWB_AGC_TABLE_S),
                              &g_stAwbAgcTableFSWDR, sizeof(AWB_AGC_TABLE_S));
            break;
    }
    pstAwbSnsDft->u16InitRgain = g_au16InitWBGain[vi_pipe][0];
    pstAwbSnsDft->u16InitGgain = g_au16InitWBGain[vi_pipe][1];
    pstAwbSnsDft->u16InitBgain = g_au16InitWBGain[vi_pipe][2]; /* index 2 */
    pstAwbSnsDft->u16SampleRgain = g_au16SampleRgain[vi_pipe];
    pstAwbSnsDft->u16SampleBgain = g_au16SampleBgain[vi_pipe];
    for (i = 0; i < CCM_MATRIX_SIZE; i++) {
        pstAwbSnsDft->au16InitCCM[i] = g_init_ccm[vi_pipe][i];
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_get_awb_spec_default(VI_PIPE vi_pipe, AWB_SPEC_SENSOR_DEFAULT_S *pstAwbSpecSnsDft)
{
    CMOS_CHECK_POINTER(pstAwbSpecSnsDft);
    hi_unused(vi_pipe);

    return HI_SUCCESS;
}

static HI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    CMOS_CHECK_POINTER(pstExpFuncs);

    (hi_void)memset_s(pstExpFuncs, sizeof(AWB_SENSOR_EXP_FUNC_S), 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
    pstExpFuncs->pfn_cmos_get_awb_spec_default = cmos_get_awb_spec_default;

    return HI_SUCCESS;
}

static ISP_CMOS_DNG_COLORPARAM_S g_stDngColorParam = {
    {378, 256, 430},
    {439, 256, 439}
};
static HI_VOID cmos_get_isp_dng_default(ISP_SNS_STATE_S *pstSnsState, ISP_CMOS_DEFAULT_S *pstDef)
{
    (hi_void)memcpy_s(&pstDef->stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S),
                      &g_stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S));

    switch (pstSnsState->u8ImgMode) {
        default:
            pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 12; /* 12bit */
            pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 4095; /* max 4095 */
            break;

        case IMX415_8M_20FPS_12BIT_WDR_MODE:
            pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 10; /* 10bit */
            pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 1023; /* max 1023 */
            break;
    }

    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Denominator = 1;
    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Numerator = 1;
    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Denominator = 1;
    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Numerator = 1;
    pstDef->stSensorMode.stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimRows = 2; /* pattern 2 */
    pstDef->stSensorMode.stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimCols = 2; /* pattern 2 */
    pstDef->stSensorMode.stDngRawFormat.stBlcRepeatDim.u16BlcRepeatRows = 2; /* pattern 2 */
    pstDef->stSensorMode.stDngRawFormat.stBlcRepeatDim.u16BlcRepeatCols = 2; /* pattern 2 */
    pstDef->stSensorMode.stDngRawFormat.enCfaLayout = CFALAYOUT_TYPE_RECTANGULAR;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPlaneColor[0] = 0;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPlaneColor[1] = 1;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPlaneColor[2] = 2; /* index 2, CfaPlaneColor 2 */
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[0] = 1;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[1] = 2; /* index 1, CfaPattern 2 */
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[2] = 0; /* index 2, CfaPattern 0 */
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[3] = 1; /* index 3, CfaPattern 1 */
    pstDef->stSensorMode.bValidDngRawFormat = HI_TRUE;
    return;
}

static void cmos_get_isp_linear_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    pstDef->unKey.bit1Demosaic       = 1;
    pstDef->pstDemosaic              = &g_stIspDemosaic;
    pstDef->unKey.bit1Sharpen        = 1;
    pstDef->pstSharpen               = &g_stIspYuvSharpen;
    pstDef->unKey.bit1Drc            = 1;
    pstDef->pstDrc                   = &g_stIspDRC;
    pstDef->unKey.bit1Gamma          = 1;
    pstDef->pstGamma                 = &g_stIspGamma;
    pstDef->unKey.bit1BayerNr        = 1;
    pstDef->pstBayerNr               = &g_stIspBayerNr;
    pstDef->unKey.bit1AntiFalseColor = 1;
    pstDef->pstAntiFalseColor        = &g_stIspAntiFalseColor;
    pstDef->unKey.bit1Ldci           = 1;
    pstDef->pstLdci                  = &g_stIspLdci;
    pstDef->unKey.bit1Dehaze         = 1;
    pstDef->pstDehaze                = &g_stIspDehaze;
    pstDef->unKey.bit1Lcac           = 1;
    pstDef->pstLcac                  = &g_stIspLCac;
    (hi_void)memcpy_s(&pstDef->stNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S),
                      &g_stIspNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
    return;
}

static void cmos_get_isp_2to1_line_wdr_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    pstDef->unKey.bit1Dpc            = 1;
    pstDef->pstDpc                   = &g_stCmosDpcWDR;
    pstDef->unKey.bit1Demosaic       = 1;
    pstDef->pstDemosaic              = &g_stIspDemosaicWdr;
    pstDef->unKey.bit1Sharpen        = 1;
    pstDef->pstSharpen               = &g_stIspYuvSharpenWdr;
    pstDef->unKey.bit1Drc            = 1;
    pstDef->pstDrc                   = &g_stIspDRCWDR;
    pstDef->unKey.bit1Gamma          = 1;
    pstDef->pstGamma                 = &g_stIspGammaFSWDR;
#ifdef CONFIG_HI_ISP_PREGAMMA_SUPPORT
    pstDef->unKey.bit1PreGamma       = 0;
    pstDef->pstPreGamma              = &g_stPreGamma;
#endif
    pstDef->unKey.bit1BayerNr        = 1;
    pstDef->pstBayerNr               = &g_stIspBayerNrWdr2To1;
    pstDef->unKey.bit1Detail         = 1;
    pstDef->pstDetail                = &g_stIspDetailWdr2To1;
#ifdef CONFIG_HI_ISP_CR_SUPPORT
    pstDef->unKey.bit1Ge             = 1;
    pstDef->pstGe                    = &g_stIspWdrGe;
#endif
    pstDef->unKey.bit1AntiFalseColor = 1;
    pstDef->pstAntiFalseColor        = &g_stIspWdrAntiFalseColor;
    pstDef->unKey.bit1Ldci           = 0;
    pstDef->pstLdci                  = &g_stIspWdrLdci;
    pstDef->unKey.bit1Dehaze         = 0;
    pstDef->pstDehaze                = &g_stIspDehazeWDR;

    pstDef->stWdrSwitchAttr.au32ExpRatio[0] = 0x400;

    (hi_void)memcpy_s(&pstDef->stNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S), &g_stIspNoiseCalibration,
                      sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
    return;
}

static HI_S32 cmos_get_isp_default(VI_PIPE vi_pipe, ISP_CMOS_DEFAULT_S *pstDef)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    CMOS_CHECK_POINTER(pstDef);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);
    (hi_void)memset_s(pstDef, sizeof(ISP_CMOS_DEFAULT_S), 0, sizeof(ISP_CMOS_DEFAULT_S));
#ifdef CONFIG_HI_ISP_CA_SUPPORT
    pstDef->unKey.bit1Ca       = 1;
    pstDef->pstCa              = &g_stIspCA;
#endif
    pstDef->unKey.bit1Clut     = 1;
    pstDef->pstClut            = &g_stIspCLUT;
    pstDef->unKey.bit1Dpc      = 1;
    pstDef->pstDpc             = &g_stCmosDpc;
#ifdef CONFIG_HI_ISP_HLC_SUPPORT
    pstDef->unKey.bit1Hlc      = 0;
    pstDef->pstHlc             = &g_stIspHlc;
#endif
    pstDef->unKey.bit1Lsc      = 1;
    pstDef->pstLsc             = &g_stCmosLsc_8M;
#ifdef CONFIG_HI_ISP_EDGEMARK_SUPPORT
    pstDef->unKey.bit1EdgeMark = 0;
    pstDef->pstEdgeMark        = &g_stIspEdgeMark;
#endif
#ifdef CONFIG_HI_ISP_CR_SUPPORT
    pstDef->unKey.bit1Ge       = 1;
    pstDef->pstGe              = &g_stIspGe;
#endif
    pstDef->unKey.bit1Detail   = 1;
    pstDef->pstDetail          = &g_stIspDetail;
    switch (pstSnsState->enWDRMode) {
        default:
        case WDR_MODE_NONE:
            cmos_get_isp_linear_default(pstDef);
            break;

        case WDR_MODE_2To1_LINE:
            cmos_get_isp_2to1_line_wdr_default(pstDef);
            break;
    }
    pstDef->stSensorMode.u32SensorID = IMX415_ID;
    pstDef->stSensorMode.u8SensorMode = pstSnsState->u8ImgMode;

    cmos_get_isp_dng_default(pstSnsState, pstDef);
    return HI_SUCCESS;
}

static HI_S32 cmos_get_isp_black_level(VI_PIPE vi_pipe, ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    CMOS_CHECK_POINTER(pstBlackLevel);
    hi_unused(vi_pipe);

    /* It need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    pstBlackLevel->au16BlackLevel[0] = 200; /* index 0, blc 200 */
    pstBlackLevel->au16BlackLevel[1] = 200; /* index 1, blc 200 */
    pstBlackLevel->au16BlackLevel[2] = 200; /* index 2, blc 200 */
    pstBlackLevel->au16BlackLevel[3] = 200; /* index 3, blc 200 */

    return HI_SUCCESS;
}

static HI_VOID cmos_set_pixel_detect(VI_PIPE vi_pipe, HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = 0;
    HI_U32 u32MaxIntTime_5Fps;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) {
        return;
    } else {
        u32FullLines_5Fps = (g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax * 30) / 5; /* 30fps, 5fps */
    }

    u32MaxIntTime_5Fps = u32FullLines_5Fps - 8; /* u32FullLines_5Fps sub 8 */

    if (bEnable) { /* setup for ISP pixel calibration mode */
        imx415_write_register(vi_pipe, IMX415_GAIN_LONG_LOW, 0x00); /* gain */
        imx415_write_register(vi_pipe, IMX415_GAIN_LONG_HIGH, 0x00);

        imx415_write_register(vi_pipe, IMX415_VMAX_ADDR_L, IMX415_LOW_8BITS(u32FullLines_5Fps));    /* VMAX */
        imx415_write_register(vi_pipe, IMX415_VMAX_ADDR_M, IMX415_MID_8BITS(u32FullLines_5Fps));
        imx415_write_register(vi_pipe, IMX415_VMAX_ADDR_H, IMX415_HIG_4BITS(u32FullLines_5Fps));

        imx415_write_register(vi_pipe, IMX415_SHR0_LOW, IMX415_LOW_8BITS(u32MaxIntTime_5Fps));
        imx415_write_register(vi_pipe, IMX415_SHR0_MIDDLE, IMX415_MID_8BITS(u32MaxIntTime_5Fps));
        imx415_write_register(vi_pipe, IMX415_SHR0_MIDDLE, IMX415_HIG_4BITS(u32MaxIntTime_5Fps));
    } else { /*  setup for ISP 'normal mode'  */
        pstSnsState->u32FLStd = (pstSnsState->u32FLStd > IMX415_FULL_LINES_MAX) ?
                                IMX415_FULL_LINES_MAX : pstSnsState->u32FLStd;
        imx415_write_register(vi_pipe, IMX415_VMAX_ADDR_L, IMX415_LOW_8BITS(pstSnsState->u32FLStd));
        imx415_write_register(vi_pipe, IMX415_VMAX_ADDR_M, IMX415_MID_8BITS(pstSnsState->u32FLStd));
        imx415_write_register(vi_pipe, IMX415_VMAX_ADDR_H, IMX415_HIG_4BITS(pstSnsState->u32FLStd));
        pstSnsState->bSyncInit = HI_FALSE;
    }

    return;
}

static HI_S32 cmos_set_wdr_mode(VI_PIPE vi_pipe, HI_U8 u8Mode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit = HI_FALSE;

    switch (u8Mode & 0x3F) {
        case WDR_MODE_NONE:
            pstSnsState->enWDRMode = WDR_MODE_NONE;
            printf("linear mode\n");
            break;

        case WDR_MODE_2To1_LINE:
            pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
            printf("2to1 line WDR mode\n");
            break;

        default:
            SNS_ERR_TRACE("NOT support this mode!\n");
            return HI_FAILURE;
    }

    (hi_void)memset_s(pstSnsState->au32WDRIntTime,
                      sizeof(pstSnsState->au32WDRIntTime), 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_VOID cmos_comm_sns_reg_info_init(VI_PIPE vi_pipe, ISP_SNS_STATE_S *pstSnsState)
{
    hi_u32 i;

    pstSnsState->astRegsInfo[0].enSnsType = ISP_SNS_I2C_TYPE;
    pstSnsState->astRegsInfo[0].unComBus.s8I2cDev = g_aunImx415BusInfo[vi_pipe].s8I2cDev;
    pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2; /* DelayMax 2 */
    pstSnsState->astRegsInfo[0].u32RegNum = 8; /* RegNum 8 */

    if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
        pstSnsState->astRegsInfo[0].u32RegNum += 8; /* RegNum add 8 */
    }

    for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++) {
        pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
        pstSnsState->astRegsInfo[0].astI2cData[i].u8DevAddr = IMX415_I2C_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[i].u32AddrByteNum = IMX415_ADDR_BYTE;
        pstSnsState->astRegsInfo[0].astI2cData[i].u32DataByteNum = IMX415_DATA_BYTE;
    }

    /* shutter related */
    pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;
    pstSnsState->astRegsInfo[0].astI2cData[0].u32RegAddr = IMX415_SHR0_LOW;        /* SHR0 */
    pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;
    pstSnsState->astRegsInfo[0].astI2cData[1].u32RegAddr = IMX415_SHR0_MIDDLE;
    pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 0; /* index 2 */
    pstSnsState->astRegsInfo[0].astI2cData[2].u32RegAddr = IMX415_SHR0_HIGH; /* index 2 */

    pstSnsState->astRegsInfo[0].astI2cData[3].u8DelayFrmNum = 0; /* index 3 */
    pstSnsState->astRegsInfo[0].astI2cData[3].u32RegAddr = IMX415_GAIN_LONG_LOW; /* index 3 */
    pstSnsState->astRegsInfo[0].astI2cData[4].u8DelayFrmNum = 0; /* index 4 */
    pstSnsState->astRegsInfo[0].astI2cData[4].u32RegAddr = IMX415_GAIN_LONG_HIGH; /* index 4 */

    /* Vmax */
    pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 0; /* index 5 */
    pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = IMX415_VMAX_ADDR_L; /* index 5 */
    pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 0; /* index 6 */
    pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = IMX415_VMAX_ADDR_M; /* index 6 */
    pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0; /* index 7 */
    pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = IMX415_VMAX_ADDR_H; /* index 7 */
    pstSnsState->bSyncInit = HI_TRUE;
    return;
}

static HI_VOID cmos_2to1_line_wdr_sns_reg_info_init(VI_PIPE vi_pipe, ISP_SNS_STATE_S *pstSnsState)
{
    hi_unused(vi_pipe);
    pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 0; /* index 5 */
    pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = IMX415_SHR1_LOW; /* index 5 */
    pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 0; /* index 6 */
    pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = IMX415_SHR1_MIDDLE; /* index 6 */
    pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0; /* index 7 */
    pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = IMX415_SHR1_HIGH; /* index 7 */

    pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 1; /* index 8 */
    pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = IMX415_VMAX_ADDR_L; /* index 8 */
    pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 1; /* index 9 */
    pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = IMX415_VMAX_ADDR_M; /* index 9 */
    pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 1; /* index 10 */
    pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = IMX415_VMAX_ADDR_H; /* index 10 */

    pstSnsState->astRegsInfo[0].astI2cData[11].u8DelayFrmNum = 0; /* index 11 */
    pstSnsState->astRegsInfo[0].astI2cData[11].u32RegAddr = IMX415_RHS1_LOW; /* index 11 */
    pstSnsState->astRegsInfo[0].astI2cData[12].u8DelayFrmNum = 0; /* index 12 */
    pstSnsState->astRegsInfo[0].astI2cData[12].u32RegAddr = IMX415_RHS1_MIDDLE; /* index 12 */
    pstSnsState->astRegsInfo[0].astI2cData[13].u8DelayFrmNum = 0; /* index 13 */
    pstSnsState->astRegsInfo[0].astI2cData[13].u32RegAddr = IMX415_RHS1_HIGH; /* index 13 */

    pstSnsState->astRegsInfo[0].astI2cData[14].u8DelayFrmNum = 0; /* index 14 */
    pstSnsState->astRegsInfo[0].astI2cData[14].u32RegAddr = IMX415_GAIN_SHORT_LOW; /* index 14 */
    pstSnsState->astRegsInfo[0].astI2cData[15].u8DelayFrmNum = 0; /* index 15 */
    pstSnsState->astRegsInfo[0].astI2cData[15].u32RegAddr = IMX415_GAIN_SHORT_HIGH; /* index 15 */
    return;
}

static HI_VOID cmos_sns_reg_info_update(VI_PIPE vi_pipe, ISP_SNS_STATE_S *pstSnsState)
{
    hi_u32 i;
    hi_unused(vi_pipe);

    for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++) {
        if (pstSnsState->astRegsInfo[0].astI2cData[i].u32Data ==
            pstSnsState->astRegsInfo[1].astI2cData[i].u32Data) {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_FALSE;
        } else {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
        }
    }

    return;
}

static HI_S32 cmos_get_sns_regs_info(VI_PIPE vi_pipe, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSnsRegsInfo);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((pstSnsState->bSyncInit == HI_FALSE) || (pstSnsRegsInfo->bConfig == HI_FALSE)) {
        cmos_comm_sns_reg_info_init(vi_pipe, pstSnsState);
        if (pstSnsState->enWDRMode == WDR_MODE_2To1_LINE) {
            cmos_2to1_line_wdr_sns_reg_info_init(vi_pipe, pstSnsState);
        }
    } else {
        cmos_sns_reg_info_update(vi_pipe, pstSnsState);
    }
    pstSnsRegsInfo->bConfig = HI_FALSE;
    (hi_void)memcpy_s(pstSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S),
                      &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    (hi_void)memcpy_s(&pstSnsState->astRegsInfo[1], sizeof(ISP_SNS_REGS_INFO_S),
                      &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));

    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode(VI_PIPE vi_pipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32W, u32H;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    u8SensorImageMode = pstSnsState->u8ImgMode;
    pstSnsState->bSyncInit = HI_FALSE;

    u32H      = pstSensorImageMode->u16Height;
    u32W      = pstSensorImageMode->u16Width;

    switch (pstSnsState->enWDRMode) {
        case WDR_MODE_2To1_LINE:
            if (IMX415_RES_IS_8M(u32W, u32H)) {
                u8SensorImageMode               = IMX415_8M_20FPS_12BIT_WDR_MODE;
                pstSnsState->u32FLStd          = 0x0B2E;
                break;
            }
            IMX415_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
        case WDR_MODE_NONE:
            if (imx415_res_is_2m(u32W, u32H)) {
                u8SensorImageMode               = IMX415_2M_60FPS_12BIT_LINEAR_MODE;
                pstSnsState->u32FLStd          = 0x08F8;
                break;
            } else {
                u8SensorImageMode               = IMX415_8M_30FPS_10BIT_LINEAR_MODE;
                pstSnsState->u32FLStd          = 0x08CA;
                break;
            }
            IMX415_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
        default:
            IMX415_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
    }

    /*  Switch SensorImageMode  */
    if ((pstSnsState->bInit == HI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
        /*  Don't need to switch SensorImageMode  */
        return ISP_DO_NOT_NEED_SWITCH_IMAGEMODE;
    }

    g_astimx415State[vi_pipe].u32BRL = 2228; /* BRL 2228 */
    pstSnsState->u8ImgMode = u8SensorImageMode;
    pstSnsState->u32FLStd  = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_VOID sensor_global_init(VI_PIPE vi_pipe)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->bInit = HI_FALSE;
    pstSnsState->bSyncInit = HI_FALSE;
    pstSnsState->u8ImgMode = IMX415_8M_30FPS_10BIT_LINEAR_MODE;
    pstSnsState->enWDRMode = WDR_MODE_NONE;
    pstSnsState->u32FLStd = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    pstSnsState->au32FL[0] = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    pstSnsState->au32FL[1] = g_astImx415ModeTbl[pstSnsState->u8ImgMode].u32VMax;

    (hi_void)memset_s(&pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S), 0, sizeof(ISP_SNS_REGS_INFO_S));
    (hi_void)memset_s(&pstSnsState->astRegsInfo[1], sizeof(ISP_SNS_REGS_INFO_S), 0, sizeof(ISP_SNS_REGS_INFO_S));
    return;
}

static HI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    CMOS_CHECK_POINTER(pstSensorExpFunc);

    (hi_void)memset_s(pstSensorExpFunc, sizeof(ISP_SENSOR_EXP_FUNC_S), 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = imx415_init;
    pstSensorExpFunc->pfn_cmos_sensor_exit = imx415_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = cmos_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;

    return HI_SUCCESS;
}

static HI_S32 imx415_set_bus_info(VI_PIPE vi_pipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
    g_aunImx415BusInfo[vi_pipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

    return HI_SUCCESS;
}

static HI_S32 sensor_ctx_init(VI_PIPE vi_pipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pastSnsStateCtx);

    if (pastSnsStateCtx == HI_NULL) {
        pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
        if (pastSnsStateCtx == HI_NULL) {
            SNS_ERR_TRACE("Isp[%d] SnsCtx malloc memory failed!\n", vi_pipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    (hi_void)memset_s(pastSnsStateCtx, sizeof(ISP_SNS_STATE_S), 0, sizeof(ISP_SNS_STATE_S));

    IMX415_SENSOR_SET_CTX(vi_pipe, pastSnsStateCtx);

    return HI_SUCCESS;
}

static HI_VOID sensor_ctx_exit(VI_PIPE vi_pipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    IMX415_SENSOR_GET_CTX(vi_pipe, pastSnsStateCtx);
    SENSOR_FREE(pastSnsStateCtx);
    IMX415_SENSOR_RESET_CTX(vi_pipe);
    return;
}

static HI_S32 sensor_register_callback(VI_PIPE vi_pipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
    HI_S32 s32Ret;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;
    ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

    CMOS_CHECK_POINTER(pstAeLib);
    CMOS_CHECK_POINTER(pstAwbLib);

    s32Ret = sensor_ctx_init(vi_pipe);
    if (s32Ret != HI_SUCCESS) {
        return HI_FAILURE;
    }

    stSnsAttrInfo.eSensorId = IMX415_ID;
    s32Ret = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("cmos init exp function failed!\n");
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_ISP_SensorRegCallBack(vi_pipe, &stSnsAttrInfo, &stIspRegister);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("sensor register callback function failed!\n");
        return s32Ret;
    }

    s32Ret = cmos_init_ae_exp_function(&stAeRegister.stSnsExp);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("cmos init ae exp function failed!\n");
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_AE_SensorRegCallBack(vi_pipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = cmos_init_awb_exp_function(&stAwbRegister.stSnsExp);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("cmos init awb exp function failed!\n");
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_AWB_SensorRegCallBack(vi_pipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 sensor_unregister_callback(VI_PIPE vi_pipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
    HI_S32 s32Ret;

    CMOS_CHECK_POINTER(pstAeLib);
    CMOS_CHECK_POINTER(pstAwbLib);

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(vi_pipe, IMX415_ID);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("sensor unregister callback function failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AE_SensorUnRegCallBack(vi_pipe, pstAeLib, IMX415_ID);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(vi_pipe, pstAwbLib, IMX415_ID);
    if (s32Ret != HI_SUCCESS) {
        SNS_ERR_TRACE("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }

    sensor_ctx_exit(vi_pipe);

    return HI_SUCCESS;
}

static HI_S32 sensor_set_init(VI_PIPE vi_pipe, ISP_INIT_ATTR_S *pstInitAttr)
{
    HI_S32 i;
    CMOS_CHECK_POINTER(pstInitAttr);

    g_au32InitExposure[vi_pipe] = pstInitAttr->u32Exposure;
    g_au32LinesPer500ms[vi_pipe] = pstInitAttr->u32LinesPer500ms;
    g_au16InitWBGain[vi_pipe][0] = pstInitAttr->u16WBRgain;
    g_au16InitWBGain[vi_pipe][1] = pstInitAttr->u16WBGgain;
    g_au16InitWBGain[vi_pipe][2] = pstInitAttr->u16WBBgain; /* index 2 */
    g_au16SampleRgain[vi_pipe] = pstInitAttr->u16SampleRgain;
    g_au16SampleBgain[vi_pipe] = pstInitAttr->u16SampleBgain;
    g_quick_start_en[vi_pipe] = pstInitAttr->stQuickStart.bQuickStartEn;

    g_abAERouteExValid[vi_pipe] = pstInitAttr->bAERouteExValid;
    (hi_void)memcpy_s(&g_astInitAERoute[vi_pipe], sizeof(ISP_AE_ROUTE_S),
                      &pstInitAttr->stAERoute, sizeof(ISP_AE_ROUTE_S));
    (hi_void)memcpy_s(&g_astInitAERouteEx[vi_pipe], sizeof(ISP_AE_ROUTE_EX_S), &pstInitAttr->stAERouteEx,
                      sizeof(ISP_AE_ROUTE_EX_S));
    for (i = 0; i < CCM_MATRIX_SIZE; i++) {
        g_init_ccm[vi_pipe][i] = pstInitAttr->au16CCM[i];
    }

    return HI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsImx415Obj = {
    .pfnRegisterCallback    = sensor_register_callback,
    .pfnUnRegisterCallback  = sensor_unregister_callback,
    .pfnStandby             = imx415_standby,
    .pfnRestart             = imx415_restart,
    .pfnMirrorFlip          = imx415_mirror_flip,
    .pfnWriteReg            = imx415_write_register,
    .pfnReadReg             = imx415_read_register,
    .pfnSetBusInfo          = imx415_set_bus_info,
    .pfnSetInit             = sensor_set_init
};

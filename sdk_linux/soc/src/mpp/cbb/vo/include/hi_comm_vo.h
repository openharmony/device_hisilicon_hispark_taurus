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

#ifndef __HI_COMM_VO_H__
#define __HI_COMM_VO_H__

#include "hi_type.h"
#include "hi_common.h"
#include "hi_errno.h"
#include "hi_comm_video.h"
#include "hi_comm_vo_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum hiVO_ZOOM_IN_E {
    VO_ZOOM_IN_RECT = 0, /* Zoom in by rect */
    VO_ZOOM_IN_RATIO, /* Zoom in by ratio */
    VO_ZOOM_IN_BUTT
} VO_ZOOM_IN_E;

typedef struct hiVO_CHN_ATTR_S {
    HI_U32 u32Priority; /* Video out overlay pri sd */
    RECT_S stRect; /* Rectangle of video output channel */
    HI_BOOL bDeflicker; /* Deflicker or not sd */
} VO_CHN_ATTR_S;

typedef struct hiVO_CHN_PARAM_S {
    ASPECT_RATIO_S stAspectRatio; /* RW; aspect ratio */
} VO_CHN_PARAM_S;

typedef struct hiVO_BORDER_S {
    HI_BOOL bBorderEn; /* RW; Do frame or not */
    BORDER_S stBorder; /* RW; frame's top, bottom, left, right width and color */
} VO_BORDER_S;

typedef struct hiVO_QUERY_STATUS_S {
    HI_U32 u32ChnBufUsed; /* Channel buffer that been occupied */
} VO_QUERY_STATUS_S;

typedef struct hiVO_WBC_ATTR_S {
    SIZE_S stTargetSize; /* RW; WBC Zoom target size */
    PIXEL_FORMAT_E enPixelFormat; /* RW; the pixel format of WBC output */
    HI_U32 u32FrameRate; /* RW; frame rate control */
    DYNAMIC_RANGE_E enDynamicRange; /* RW; Write back dynamic range type */
    COMPRESS_MODE_E enCompressMode; /* RW; Write back Compressing mode */
} VO_WBC_ATTR_S;

typedef enum hiVO_WBC_MODE_E {
    VO_WBC_MODE_NORMAL = 0, /* In this mode, wbc will capture frames according to dev frame rate
                                and wbc frame rate */
    VO_WBC_MODE_DROP_REPEAT, /* In this mode, wbc will drop dev repeat frame, and capture the real frame
                                according to video layer's display rate and wbc frame rate */
    VO_WBC_MODE_PROG_TO_INTL, /* In this mode, wbc will drop dev repeat frame which repeats more than 3 times,
                                and change two progressive frames to one interlace frame */
    VO_WBC_MODE_BUTT,
} VO_WBC_MODE_E;

typedef enum hiVO_WBC_SOURCE_TYPE_E {
    VO_WBC_SOURCE_DEV = 0x0, /* WBC source is device */
    VO_WBC_SOURCE_VIDEO = 0x1, /* WBC source is video layer */
    VO_WBC_SOURCE_GRAPHIC = 0x2, /* WBC source is graphic layer. Warning: not supported */
    VO_WBC_SOURCE_BUTT
} VO_WBC_SOURCE_TYPE_E;

typedef struct hiVO_WBC_SOURCE_S {
    VO_WBC_SOURCE_TYPE_E enSourceType; /* RW; WBC source's type */
    HI_U32 u32SourceId; /* RW; WBC source's ID */
} VO_WBC_SOURCE_S;

typedef enum hiVO_PART_MODE_E {
    VO_PART_MODE_SINGLE = 0, /* single partition, which use software to make multi-picture in one hardware cell */
    VO_PART_MODE_MULTI = 1, /* multi partition, each partition is a hardware cell */
    VO_PART_MODE_BUTT
} VO_PART_MODE_E;

typedef struct hiVO_VIDEO_LAYER_ATTR_S {
    RECT_S stDispRect; /* RW; Display resolution */
    SIZE_S stImageSize; /* RW; Canvas size of the video layer */
    HI_U32 u32DispFrmRt; /* RW; Display frame rate */
    PIXEL_FORMAT_E enPixFormat; /* RW; Pixel format of the video layer */
    HI_BOOL bDoubleFrame; /* RW; Whether to double frames */
    HI_BOOL bClusterMode; /* RW; Whether to take Cluster way to use memory */
    DYNAMIC_RANGE_E enDstDynamicRange; /* RW; Video Layer output dynamic range type */
} VO_VIDEO_LAYER_ATTR_S;

typedef struct hiVO_LAYER_PARAM_S {
    ASPECT_RATIO_S stAspectRatio; /* RW; aspect ratio */
} VO_LAYER_PARAM_S;

typedef struct hiVO_ZOOM_RATIO_S {
    HI_U32 u32XRatio; /* RW; Range: [0, 1000]; XRatio = x * 1000 / W,
                              x means the start point to be zoomed, W means displaying channel's width. */
    HI_U32 u32YRatio; /* RW; Range: [0, 1000]; YRatio = y * 1000 / H,
                              y means the start point to be zoomed, H means displaying channel's height. */
    HI_U32 u32WRatio; /* RW; Range: [0, 1000]; WRatio = w * 1000 / W,
                              w means the width to be zoomed, W means displaying channel's width. */
    HI_U32 u32HRatio; /* RW; Range: [0, 1000]; HRatio = h * 1000 / H,
                              h means the height to be zoomed, H means displaying channel's height. */
} VO_ZOOM_RATIO_S;

typedef struct hiVO_ZOOM_ATTR_S {
    VO_ZOOM_IN_E enZoomType; /* choose the type of zoom in */
    union {
        RECT_S stZoomRect; /* zoom in by rect */
        VO_ZOOM_RATIO_S stZoomRatio; /* zoom in by ratio */
    };
} VO_ZOOM_ATTR_S;

typedef struct hiVO_REGION_INFO_S {
    HI_U32 u32RegionNum; /* count of the region */
    RECT_S *ATTRIBUTE pstRegion; /* region attribute */
} VO_REGION_INFO_S;

typedef struct hiVO_LAYER_BOUNDARY_S {
    HI_U32 u32Width;
    HI_U32 u32Color[2]; /* RW; 2 color indexes */
} VO_LAYER_BOUNDARY_S;

typedef struct hiVO_CHN_BOUNDARY_S {
    HI_BOOL bBoundaryEn; /* RW; do Frame or not */
    HI_U32 u32ColorIndex; /* RW; the index of Frame color,{0,1} */
} VO_CHN_BOUNDARY_S;

typedef enum hiVO_MIRROR_MODE_E {
    VO_MIRROR_NONE = 0, /* Mirror mode is none */
    VO_MIRROR_HOR,      /* Mirror mode is horizontal mirror */
    VO_MIRROR_BUTT
} VO_MIRROR_MODE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_COMM_VO_H__ */

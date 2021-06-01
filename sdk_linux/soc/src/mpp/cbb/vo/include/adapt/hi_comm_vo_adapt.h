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

#ifndef __HI_COMM_VO_ADAPT_H__
#define __HI_COMM_VO_ADAPT_H__

#include "hi_type.h"
#include "hi_comm_video_adapt.h"
#include "hi_comm_vo.h"
#include "hi_comm_vo_dev_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef VO_ZOOM_IN_E hi_vo_zoom_in;

typedef struct {
    hi_u32 priority; /* video out overlay pri sd */
    hi_rect rect; /* rectangle of video output channel */
    hi_bool deflicker; /* deflicker or not sd */
} hi_vo_chn_attr;

typedef struct {
    hi_aspect_ratio aspect_ratio; /* RW; aspect ratio */
} hi_vo_chn_param;

typedef struct {
    hi_bool border_en; /* RW; do frame or not */
    hi_border border; /* RW; frame's top, bottom, left, right width and color */
} hi_vo_border;

typedef struct {
    hi_u32 chn_buf_used; /* channel buffer that been occupied */
} hi_vo_query_status;

typedef struct {
    hi_size target_size; /* RW; WBC zoom target size */
    hi_pixel_format pixel_format; /* RW; the pixel format of WBC output */
    hi_u32 frame_rate; /* RW; frame rate control */
    hi_dynamic_range dynamic_range; /* RW; write back dynamic range type */
    hi_compress_mode compress_mode; /* RW; write back compressing mode */
} hi_vo_wbc_attr;

typedef VO_WBC_MODE_E hi_vo_wbc_mode;

typedef VO_WBC_SOURCE_TYPE_E hi_vo_wbc_source_type;

typedef struct {
    hi_vo_wbc_source_type source_type; /* RW; WBC source's type */
    hi_u32 source_id; /* RW; WBC source's ID */
} hi_vo_wbc_source;

typedef VO_PART_MODE_E hi_vo_part_mode;

typedef struct {
    hi_rect disp_rect; /* RW; display resolution */
    hi_size image_size; /* RW; canvas size of the video layer */
    hi_u32 disp_frm_rt; /* RW; display frame rate */
    hi_pixel_format pix_format; /* RW; pixel format of the video layer */
    hi_bool double_frame; /* RW; whether to double frames */
    hi_bool cluster_mode; /* RW; whether to take cluster way to use memory */
    hi_dynamic_range dst_dynamic_range; /* RW; video layer output dynamic range type */
} hi_vo_video_layer_attr;

typedef struct {
    hi_aspect_ratio aspect_ratio; /* RW; aspect ratio */
} hi_vo_layer_param;

typedef struct {
    hi_u32 x_ratio; /* RW; range: [0, 1000]; x_ratio = x * 1000 / W,
                              x means the start point to be zoomed, W means displaying channel's width. */
    hi_u32 y_ratio; /* RW; range: [0, 1000]; y_ratio = y * 1000 / H,
                              y means the start point to be zoomed, H means displaying channel's height. */
    hi_u32 w_ratio; /* RW; range: [0, 1000]; w_ratio = w * 1000 / W,
                              w means the width to be zoomed, W means displaying channel's width. */
    hi_u32 h_ratio; /* RW; range: [0, 1000]; h_ratio = h * 1000 / H,
                              h means the height to be zoomed, H means displaying channel's height. */
} hi_vo_zoom_ratio;

typedef struct {
    hi_vo_zoom_in zoom_type; /* choose the type of zoom in */
    union {
        hi_rect zoom_rect; /* zoom in by rect */
        hi_vo_zoom_ratio zoom_ratio; /* zoom in by ratio */
    };
} hi_vo_zoom_attr;

typedef struct {
    hi_u32 region_num; /* count of the region */
    hi_rect *ATTRIBUTE region; /* region attribute */
} hi_vo_region_info;

typedef struct {
    hi_u32 width;
    hi_u32 color[2]; /* RW; 2 color indexes */
} hi_vo_layer_boundary;

typedef struct {
    hi_bool boundary_en; /* RW; do frame or not */
    hi_u32 color_index; /* RW; the index of frame color, {0,1} */
} hi_vo_chn_boundary;

typedef VO_MIRROR_MODE_E hi_vo_mirror_mode;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __HI_COMM_VO_ADAPT_H__ */

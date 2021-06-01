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
#include "drv_hdmi_infoframe.h"
#include "drv_hdmi_intf.h"
#include "drv_hdmi_common.h"
#include "hdmi_hal.h"

#define HDMI_AVI_VERSION3               3
#define HDMI_AVI_VERSION4               4
#define HDMI_AVI_Y2_MASK                (0x1 << 2)
#define AVI_INFORFRAME_CHARACTER_MASK_1 0x3
#define AVI_INFORFRAME_CHARACTER_MASK_2 0xf
#define AVI_INFORFRAME_CHARACTER_MASK_3 0x7
#define AVI_INFORFRAME_BAR_MODE_MASK    0xff
#define AVI_INFORFRAME_BAR_MODE_OFFSET  8
#define AVI_INFORFRAME_LENGTH           13
#define AUDIO_INFORFRAME_LENGTH         10
#define DYN_RAN_INFORFRAME_LENGTH       26
#define DRM_INFORFRAME_META_MASK        0xff
#define DRM_INFORFRAME_META_OFFSET      8
#define HDMI_INFOFRAME_BUFFER_SIZE      32
#define HDMI_INFOFRAME_CHECCKSUM        256

hdmi_video_timing drv_hdmi_video_timing_lookup(hdmi_video_code_vic vic, hdmi_picture_aspect aspect)
{
    return drv_hdmi_video_timing_get(vic, aspect);
}

hdmi_video_timing drv_hdmi_vsif_video_timing_lookup(hdmi_vsif_vic vic)
{
    return drv_hdmi_vsif_video_timing_get(vic);
}

static hi_void hdmi_infoframe_checksum(hi_void *buffer, hi_u32 size)
{
    hi_u8 *ptr = HI_NULL;
    hi_u8 checksum = 0;
    hi_u32 i;

    ptr = buffer;
    /* compute checksum */
    for (i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    if (checksum > 0) {
        ptr[3] = HDMI_INFOFRAME_CHECCKSUM - checksum;
    } else {
        ptr[3] = 0;
    }
}

/*
 * hdmi_avi_infoframe_pack() - write HDMI AVI infoframe to binary buffer
 * @frame: HDMI AVI infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 * packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 * returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
static hdmi_video_code_vic avi_header_pack(struct hdmi_avi_infoframe *frame, hi_void *buffer, hdmi_3d_mode _3d_mode)
{
    hi_u8 *ptr = HI_NULL;
    hi_bool _3d_enable = HI_FALSE;
    hdmi_video_code_vic video_code;

    _3d_enable = (_3d_mode != HDMI_3D_BUTT) ? HI_FALSE : HI_TRUE;
    video_code = drv_hdmi_vic_search(frame->video_timing, frame->picture_aspect, _3d_enable);

    ptr = buffer;
    ptr[0] = frame->type;
    if (((hi_u32)frame->colorspace & HDMI_AVI_Y2_MASK) || (video_code > HDMI_5120X2160P100_64_27)) {
        /* Y2 = 1 or vic >= 128, version shall use 3 */
        frame->version = HDMI_AVI_VERSION3;
    } else if (frame->colorimetry == HDMI_COLORIMETRY_EXTEND &&
               frame->extended_colorimetry == HDMI_EXTENDED_COLORIMETRY_ADDITIONAL) {
        /* (C1,C0)=(1,1) and (EC2,EC1,EC0)=(1,1,1), version shall use 4 */
        frame->version = HDMI_AVI_VERSION4;
    }
    ptr[1] = frame->version;
    ptr[2] = frame->length;
    ptr[3] = 0; /* checksum */

    return video_code;
}

static hi_s32 hdmi_avi_infoframe_pack(struct hdmi_avi_infoframe *frame, hi_void *buffer,
    hi_u32 size, hdmi_3d_mode _3d_mode)
{
    hi_u32              length;
    hi_u8              *ptr = HI_NULL;
    hdmi_video_code_vic video_code;

    length = frame->length + HDMI_INFOFRAME_HEADER_SIZE;
    if (size < length) {
        return -1;
    }
    (hi_void)memset_s(buffer, size, 0, size);
    video_code = avi_header_pack(frame, buffer, _3d_mode);
    /* start infoframe payload */
    ptr = buffer;
    ptr += HDMI_INFOFRAME_HEADER_SIZE;
    ptr[0] = (((hi_u32)frame->colorspace & AVI_INFORFRAME_CHARACTER_MASK_1) << 5) | /* 5'b, bit[7:6] */
             ((hi_u32)frame->scan_mode & AVI_INFORFRAME_CHARACTER_MASK_1);
    if (frame->active_info_valid) {
        hdmi_set_bit(ptr[0], 4); /* 4'b, bit[5] */
    }
    if (frame->horizontal_bar_valid) {
        hdmi_set_bit(ptr[0], 3); /* 3'b, bit[4] */
    }
    if (frame->vertical_bar_valid) {
        hdmi_set_bit(ptr[0], 2); /* 2'b, bit[3] */
    }
    ptr[1] = (((hi_u32)frame->colorimetry & AVI_INFORFRAME_CHARACTER_MASK_1) << 6) | /* 6-7 bit is colorimetry */
             /* 4-5 is coded frame aspect ratio */
             (((hi_u32)frame->picture_aspect & AVI_INFORFRAME_CHARACTER_MASK_1) << 4) |
             ((hi_u32)frame->active_aspect & AVI_INFORFRAME_CHARACTER_MASK_2);
    /* 4-7 bit is Extended Colorimetry */
    ptr[2] = (((hi_u32)frame->extended_colorimetry & AVI_INFORFRAME_CHARACTER_MASK_3) << 4) |
    /* 2-3 bit is RGB quantization range */
             (((hi_u32)frame->quantization_range & AVI_INFORFRAME_CHARACTER_MASK_1) << 2) |
             ((hi_u32)frame->nups & AVI_INFORFRAME_CHARACTER_MASK_1);
    if (frame->itc) {
        hdmi_set_bit(ptr[2], 7); /* 7'b, bit[8] */
    }
    ptr[3] = (hi_u32)video_code & 0xff; /* frame->video_code */
    /* 6-7 bit is quantization range */
    ptr[4] = (((hi_u32)frame->ycc_quantization_range & AVI_INFORFRAME_CHARACTER_MASK_1) << 6) |
        (((hi_u32)frame->content_type & AVI_INFORFRAME_CHARACTER_MASK_1) << 4) | /* 4-5 bit is quantization range */
        ((frame->pixel_repeat - 1) & AVI_INFORFRAME_CHARACTER_MASK_2); /* pixel repeat mode */
    ptr[5] = frame->top_bar & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[6] = (frame->top_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[7] = frame->bottom_bar & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[8] = (frame->bottom_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[9] = frame->left_bar & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[10] = (frame->left_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[11] = frame->right_bar & AVI_INFORFRAME_BAR_MODE_MASK;
    ptr[12] = (frame->right_bar >> AVI_INFORFRAME_BAR_MODE_OFFSET) & AVI_INFORFRAME_BAR_MODE_MASK;
    hdmi_infoframe_checksum(buffer, length);

    return length;
}

#ifdef HDMI_HDR_SUPPORT
hi_s32 hdmi_drm_infoframe_pack(hdmi_drm_infoframe *frame, hi_void *buffer, hi_u32 size)
{
    hi_u32 length;
    hi_u8 *ptr = HI_NULL;
    hdmi_meta_des_1st *meta_des_1st = HI_NULL;

    hdmi_if_null_return(frame, HI_FAILURE);
    hdmi_if_null_return(buffer, HI_FAILURE);

    ptr = buffer;
    meta_des_1st = &frame->meta_descriptor.type1;
    hdmi_if_null_return(meta_des_1st, HI_FAILURE);

    length = HDMI_INFOFRAME_HEADER_SIZE + frame->length;
    if (size < length) {
        return -1;
    }
    (hi_void)memset_s(buffer, size, 0, size);

    ptr[0] = frame->type;
    ptr[1] = frame->version;
    ptr[2] = frame->length;
    ptr[3] = 0; /* checksum */

    /* start infoframe payload */
    ptr += HDMI_INFOFRAME_HEADER_SIZE;
    ptr[0]  = frame->eotf_type;
    ptr[1]  = frame->metadata_type;
    ptr[2]  = meta_des_1st->primaries0_x & DRM_INFORFRAME_META_MASK;
    ptr[3]  = (meta_des_1st->primaries0_x >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[4]  = meta_des_1st->primaries0_y & DRM_INFORFRAME_META_MASK;
    ptr[5]  = (meta_des_1st->primaries0_y >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[6]  = meta_des_1st->primaries1_x & DRM_INFORFRAME_META_MASK;
    ptr[7]  = (meta_des_1st->primaries1_x >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[8]  = meta_des_1st->primaries1_y & DRM_INFORFRAME_META_MASK;
    ptr[9]  = (meta_des_1st->primaries1_y >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[10] = meta_des_1st->primaries2_x & DRM_INFORFRAME_META_MASK;
    ptr[11] = (meta_des_1st->primaries2_x >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[12] = meta_des_1st->primaries2_y & DRM_INFORFRAME_META_MASK;
    ptr[13] = (meta_des_1st->primaries2_y >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[14] = meta_des_1st->white_point_x & DRM_INFORFRAME_META_MASK;
    ptr[15] = (meta_des_1st->white_point_x >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[16] = meta_des_1st->white_point_y & DRM_INFORFRAME_META_MASK;
    ptr[17] = (meta_des_1st->white_point_y >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[18] = meta_des_1st->max_luminance & DRM_INFORFRAME_META_MASK;
    ptr[19] = (meta_des_1st->max_luminance >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[20] = meta_des_1st->min_luminance & DRM_INFORFRAME_META_MASK;
    ptr[21] = (meta_des_1st->min_luminance >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[22] = meta_des_1st->max_light_level & DRM_INFORFRAME_META_MASK;
    ptr[23] = (meta_des_1st->max_light_level >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    ptr[24] = meta_des_1st->average_light_level & DRM_INFORFRAME_META_MASK;
    ptr[25] = (meta_des_1st->average_light_level >> DRM_INFORFRAME_META_OFFSET) & DRM_INFORFRAME_META_MASK;
    hdmi_infoframe_checksum(buffer, length);

    return length;
}
#endif

/*
 * hdmi_audio_infoframe_pack() - write HDMI audio infoframe to binary buffer
 * @frame: HDMI audio infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 * packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 * returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
static hi_s32 hdmi_audio_infoframe_pack(struct hdmi_audio_infoframe *frame, hi_void *buffer, hi_u32 size)
{
    hi_u32 length;
    hi_u8 *ptr = HI_NULL;

    ptr = buffer;
    length = HDMI_INFOFRAME_HEADER_SIZE + frame->length;
    if (size < length) {
        return -1;
    }
    (hi_void)memset_s(buffer, size, 0, size);

    ptr[0] = frame->type;
    ptr[1] = frame->version;
    ptr[2] = frame->length;
    ptr[3] = 0; /* checksum */

    /* start infoframe payload */
    ptr += HDMI_INFOFRAME_HEADER_SIZE;
    ptr[0] = (((hi_u32)frame->coding_type & 0xf) << 4) | (frame->channels & 0x7); /* 4'b, bit[8:5] */
    ptr[1] = (((hi_u32)frame->sample_frequency & 0x7) << 2) | ((hi_u32)frame->sample_size & 0x3); /* 2'b, bit[5:3] */
    ptr[2] = (hi_u32)frame->coding_type_ext & 0x1f;
    ptr[3] = frame->channel_allocation;
    ptr[4] = (frame->level_shift_value & 0xf) << 3; /* 3'b, bit[7:4] */
    ptr[4] |= (hi_u32)frame->lfe_playback_level & 0x3;
    if (frame->downmix_inhibit) {
        hdmi_set_bit(ptr[4], 7); /* 7'b, bit[8] */
    }
    hdmi_infoframe_checksum(buffer, length);

    return length;
}

/*
 * hdmi_vendor_specific_infoframe_pack() - write a HDMI vendor infoframe to binary buffer
 * @frame: HDMI vendor infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 * packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 * returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
static hi_s32 hdmi_vendor_specific_infoframe_pack(struct hdmi_vendor_specific_infoframe *frame,
    hi_void *buffer, hi_u32 size)
{
    errno_t ret;
    hi_u32 length;
    hi_u8 *ptr = HI_NULL;

    ptr = buffer;
    length = HDMI_INFOFRAME_HEADER_SIZE + frame->length;
    if (size < length) {
        return -1;
    }
    (hi_void)memset_s(buffer, size, 0, size);

    ptr[0] = frame->type;
    ptr[1] = frame->version;
    ptr[2] = frame->length;
    ptr[3] = 0; /* checksum */

    if (frame->vsif_content.h14_vsif_content.ieee == HDMI14_IEEE_OUI) {
        hdmi_14_vsif_content *vsif_content = &frame->vsif_content.h14_vsif_content;
        hdmi_user_vsif_content *user_content = &frame->vsif_content.cea861_vsif;

        /* IEEE OUI */
        ptr[4] = HDMI14_IEEE_OUI & 0xff;
        ptr[5] = (HDMI14_IEEE_OUI >> 8) & 0xff; /* 8'b, bit[16:9] */
        ptr[6] = (HDMI14_IEEE_OUI >> 16) & 0xff; /* 16'b, bit[8:1] */
        /* hdmi video format */
        ptr[7] = ((hi_u32)vsif_content->format & 0x7) << 5; /* 5'b, bit[8:6] */
        if (vsif_content->format == HDMI_VIDEO_FORMAT_4K) {
            ptr[8] = vsif_content->vic;
        } else if (vsif_content->format == HDMI_VIDEO_FORMAT_3D) {
            ptr[8] = ((hi_u32)vsif_content->_3d_structure & 0xf) << 4; /* 4'b, bit[8:5] */
        }
        /* if the 3D_Meta_present bit is clear(=0),mean that there have no 3D_Metadata */
        if (vsif_content->_3d_meta_present == 0) {
            /* have user data  */
            if ((user_content->len != 0) && (length + user_content->len <= size)) {
                ptr[2] += user_content->len;
                ret = memcpy_s(&ptr[length], (size - length), user_content->data, user_content->len);
                hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
                length += user_content->len;
            }
        }
    }
    hdmi_infoframe_checksum(buffer, length);

    return length;
}

hi_void drv_hdmi_infoframe_send(hdmi_infoframe_type *info_frame,
    hdmi_infoframe_id infoframe_id, hdmi_infoframe *infoframe)
{
    hi_u8 buffer[HDMI_INFOFRAME_BUFFER_SIZE] = {0};
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_if_null_return_void(info_frame);
    hdmi_if_null_return_void(infoframe);

    hdmi_dev = osal_container_of(info_frame, __typeof__(*hdmi_dev), info_frame);
    hdmi_if_null_return_void(hdmi_dev);

    /* pack infoframe */
    switch (infoframe_id) {
        case HDMI_INFOFRAME_TYPE_VENDOR:
            infoframe->vendor_infoframe.type = HDMI_INFOFRAME_TYPE_VENDOR;
            infoframe->vendor_infoframe.version = 1;
            hdmi_vendor_specific_infoframe_pack(&infoframe->vendor_infoframe, buffer, HDMI_INFOFRAME_BUFFER_SIZE);
            break;
        case HDMI_INFOFRAME_TYPE_AVI:
            infoframe->avi_infoframe.type = HDMI_INFOFRAME_TYPE_AVI;
            infoframe->avi_infoframe.version = 2; /* avi infoframe version is 1 */
            infoframe->avi_infoframe.length = AVI_INFORFRAME_LENGTH;
            hdmi_avi_infoframe_pack(&infoframe->avi_infoframe, buffer, HDMI_INFOFRAME_BUFFER_SIZE,
                hdmi_dev->attr.vo_attr.stereo_mode);
            break;
        case HDMI_INFOFRAME_TYPE_AUDIO:
            infoframe->audio_infoframe.type = HDMI_INFOFRAME_TYPE_AUDIO;
            infoframe->audio_infoframe.version = 1;
            infoframe->audio_infoframe.length = AUDIO_INFORFRAME_LENGTH;
            hdmi_audio_infoframe_pack(&infoframe->audio_infoframe, buffer, HDMI_INFOFRAME_BUFFER_SIZE);
            break;
#ifdef HDMI_HDR_SUPPORT
        case HDMI_INFOFRAME_TYPE_DRM:
            infoframe->drm_infoframe.type = HDMI_INFOFRAME_TYPE_DRM;
            infoframe->drm_infoframe.version = 1;
            infoframe->drm_infoframe.length = DYN_RAN_INFORFRAME_LENGTH;
            hdmi_drm_infoframe_pack(&infoframe->drm_infoframe, buffer, HDMI_INFOFRAME_BUFFER_SIZE);
            break;
#endif
        default:
            return;
    }
    drv_hdmi_infoframe_enable_set(info_frame, infoframe_id, HI_FALSE);
    hal_call_void(hal_hdmi_infoframe_set, hdmi_dev->hal, infoframe_id, buffer, HDMI_INFOFRAME_BUFFER_SIZE);
    drv_hdmi_infoframe_enable_set(info_frame, infoframe_id, HI_TRUE);

    return;
}

hi_void drv_hdmi_infoframe_enable_set(hdmi_infoframe_type *info_frame, hdmi_infoframe_id infoframe_id,
    hi_bool enable)
{
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_if_null_return_void(info_frame);

    hdmi_dev = osal_container_of(info_frame, __typeof__(*hdmi_dev), info_frame);
    hal_call_void(hal_hdmi_infoframe_enable_set, hdmi_dev->hal, infoframe_id, enable);

    return;
}

#ifdef HDMI_HDR_SUPPORT
hi_s32 drv_hdmi_drm_infoframe_send(hdmi_infoframe_type *info_frame, hi_bool enable)
{
    errno_t ret;
    hdmi_infoframe      infoframe     = {0};
    hdmi_device        *hdmi_dev      = HI_NULL;
    drv_hdmi_hdr_attr  *hdr_attr      = HI_NULL;
    hdmi_drm_infoframe *drm_infoframe = HI_NULL;

    hdmi_if_null_return(info_frame, HI_FAILURE);

    if (!enable) {
        drv_hdmi_infoframe_enable_set(info_frame, HDMI_INFOFRAME_TYPE_DRM, HI_FALSE);
        return HI_SUCCESS;
    }
    hdmi_dev = osal_container_of(info_frame, __typeof__(*hdmi_dev), info_frame);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    hdr_attr = &hdmi_dev->attr.hdr_attr;
    drm_infoframe = &info_frame->drm_infoframe;
    (hi_void)memset_s(drm_infoframe, sizeof(info_frame->drm_infoframe), 0, sizeof(hdmi_drm_infoframe));
    drm_infoframe->eotf_type = hdr_attr->eotf_type;
    drm_infoframe->metadata_type = hdr_attr->metadata_id;
    drm_infoframe->meta_descriptor = hdr_attr->un_descriptor;
    ret = memcpy_s(&infoframe.drm_infoframe, sizeof(infoframe.drm_infoframe),
        drm_infoframe, sizeof(hdmi_drm_infoframe));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    drv_hdmi_infoframe_send(info_frame, HDMI_INFOFRAME_TYPE_DRM, &infoframe);

    return HI_SUCCESS;
}
#endif

#ifdef HDMI_HDR_SUPPORT
static hi_void avi_hdr_send(hdmi_avi_infoframe *avi_infoframe, hdmi_attr *attr)
{
    drv_hdmi_hdr_attr *hdr_attr = HI_NULL;
    hdmi_vo_attr      *vo_attr  = HI_NULL;
    hdmi_app_attr     *app_attr = HI_NULL;

    hdr_attr = &attr->hdr_attr;
    app_attr = &attr->app_attr;
    vo_attr  = &attr->vo_attr;
    switch (hdr_attr->hdr_mode) {
        case HDMI_HDR_MODE_CEA_861_3:
        case HDMI_HDR_MODE_CEA_861_3_AUTHEN:
            avi_infoframe->colorimetry = vo_attr->colorimetry;
            avi_infoframe->extended_colorimetry = vo_attr->extended_colorimetry;
            avi_infoframe->colorspace = app_attr->out_color_space;
            if (hdr_attr->hdr_mode == HDMI_HDR_MODE_CEA_861_3_AUTHEN) {
                avi_infoframe->colorspace = HDMI_COLORSPACE_YCBCR422;
            }
            avi_infoframe->quantization_range = vo_attr->rgb_quantization;
            avi_infoframe->ycc_quantization_range = vo_attr->ycc_quantization;
            hdmi_info("HDR mode, avi infoframe force to change outcolorspace(%u), extended_colorimetry(%u).\n",
                      avi_infoframe->colorspace, avi_infoframe->extended_colorimetry);
            break;
        case HDMI_HDR_MODE_DOLBY_NORMAL:
            avi_infoframe->colorspace = HDMI_COLORSPACE_YCBCR422;
            avi_infoframe->colorimetry = app_attr->xvycc_mode ? HDMI_COLORIMETRY_EXTEND : vo_attr->colorimetry;
            avi_infoframe->extended_colorimetry = vo_attr->extended_colorimetry;
            avi_infoframe->quantization_range = vo_attr->rgb_quantization;
            avi_infoframe->ycc_quantization_range = HDMI_YCC_QUANTIZATION_RANGE_FULL;
            hdmi_info("dolby normal, avi infoframe force to change outcolorspace(%u), ycc_quantization_range(%u).\n",
                      avi_infoframe->colorspace, avi_infoframe->ycc_quantization_range);
            break;
        case HDMI_HDR_MODE_DOLBY_TUNNELING:
            avi_infoframe->colorspace = HDMI_COLORSPACE_RGB;
            avi_infoframe->colorimetry = app_attr->xvycc_mode ? HDMI_COLORIMETRY_EXTEND : vo_attr->colorimetry;
            avi_infoframe->extended_colorimetry = vo_attr->extended_colorimetry;
            avi_infoframe->quantization_range = HDMI_QUANTIZATION_RANGE_FULL;
            avi_infoframe->ycc_quantization_range = vo_attr->ycc_quantization;
            hdmi_info("dolby tunnel, avi infoframe force to change outcolorspace(%u), quantization_range(%u).\n",
                      avi_infoframe->colorspace, avi_infoframe->quantization_range);
            break;
        default:
            avi_infoframe->extended_colorimetry = vo_attr->extended_colorimetry;
            avi_infoframe->quantization_range = vo_attr->rgb_quantization;
            avi_infoframe->ycc_quantization_range = vo_attr->ycc_quantization;
            avi_infoframe->colorspace = app_attr->out_color_space;
            avi_infoframe->colorimetry = app_attr->xvycc_mode ? HDMI_COLORIMETRY_EXTEND : vo_attr->colorimetry;
            break;
    }

    return;
}
#endif

hi_s32 drv_hdmi_avi_infoframe_send(hdmi_infoframe_type *info_frame, hi_bool enable)
{
    errno_t ret;
    hdmi_infoframe     infoframe      = {0};
    hdmi_device        *hdmi_dev      = HI_NULL;
    hdmi_vo_attr       *vo_attr       = HI_NULL;
    hdmi_avi_infoframe *avi_infoframe = HI_NULL;

    hdmi_if_null_return(info_frame, HI_FAILURE);
    if (!enable) {
        drv_hdmi_infoframe_enable_set(info_frame, HDMI_INFOFRAME_TYPE_AVI, HI_FALSE);
        return HI_SUCCESS;
    }

    hdmi_dev = osal_container_of(info_frame, __typeof__(*hdmi_dev), info_frame);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    vo_attr  = &hdmi_dev->attr.vo_attr;

    avi_infoframe = &info_frame->avi_infoframe;
    (hi_void)memset_s(avi_infoframe, sizeof(info_frame->avi_infoframe), 0, sizeof(hdmi_avi_infoframe));
    avi_infoframe->disp_fmt          = vo_attr->disp_fmt;
    avi_infoframe->video_timing      = vo_attr->video_timing;
    avi_infoframe->picture_aspect    = vo_attr->picture_aspect;
    avi_infoframe->active_aspect     = vo_attr->active_aspect;
    avi_infoframe->nups              = vo_attr->picture_scaling;
    avi_infoframe->pixel_repeat      = vo_attr->pixel_repeat;
    avi_infoframe->active_info_valid = HI_TRUE;

#ifdef HDMI_HDR_SUPPORT
    avi_hdr_send(avi_infoframe, &hdmi_dev->attr);
#else
    avi_infoframe->extended_colorimetry = vo_attr->extended_colorimetry;
    avi_infoframe->quantization_range = vo_attr->rgb_quantization;
    avi_infoframe->ycc_quantization_range = vo_attr->ycc_quantization;
    avi_infoframe->colorspace  = hdmi_dev->attr.app_attr.out_color_space;
    avi_infoframe->colorimetry = hdmi_dev->attr.app_attr.xvycc_mode ? HDMI_COLORIMETRY_EXTEND : vo_attr->colorimetry;
#endif
    ret = memcpy_s(&infoframe.avi_infoframe, sizeof(infoframe.avi_infoframe),
        avi_infoframe, sizeof(hdmi_avi_infoframe));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    drv_hdmi_infoframe_send(info_frame, HDMI_INFOFRAME_TYPE_AVI, &infoframe);

    return HI_SUCCESS;
}

static hi_u8 vsif_length(const hdmi_14_vsif_content *h14_vsif_content, hi_bool dolby_enable)
{
    hi_u8 length;

#ifdef HDMI_HDR_SUPPORT
    if (dolby_enable) {
        length = 0x18;
    } else if (h14_vsif_content->format == HDMI_VIDEO_FORMAT_4K) {
        length = 0x05;
    } else if (h14_vsif_content->format == HDMI_VIDEO_FORMAT_3D) {
        length = 0x07;
    } else {
        length = 0x04;
    }
#else
    length = 0x07;

    hi_unused(h14_vsif_content);
    hi_unused(dolby_enable);
#endif

    return length;
}

hi_s32 drv_hdmi_vendor_infoframe_send(hdmi_infoframe_type *info_frame, hi_bool enable, hi_bool dolby_enable)
{
    errno_t ret;
    hdmi_infoframe         infoframe        = {0};
    hdmi_vo_attr          *vo_attr          = HI_NULL;
    hdmi_device           *hdmi_dev         = HI_NULL;
    hdmi_vendor_infoframe *vsif             = HI_NULL;
    hdmi_14_vsif_content  *h14_vsif_content = HI_NULL;
    hi_u32                 vic, cnt;
    hdmi_video_4k_def     *video            = HI_NULL;

    hdmi_if_null_return(info_frame, HI_FAILURE);

    if (!enable) {
        drv_hdmi_infoframe_enable_set(info_frame, HDMI_INFOFRAME_TYPE_VENDOR, HI_FALSE);
        return HI_SUCCESS;
    }
    hdmi_dev = osal_container_of(info_frame, __typeof__(*hdmi_dev), info_frame);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    vo_attr = &hdmi_dev->attr.vo_attr;
    vsif = &info_frame->vendor_infoframe;
    (hi_void)memset_s(vsif, sizeof(info_frame->vendor_infoframe), 0, sizeof(hdmi_vendor_infoframe));

    h14_vsif_content = &vsif->vsif_content.h14_vsif_content;
    h14_vsif_content->ieee = HDMI14_IEEE_OUI;
    vic = drv_hdmi_vic_search(vo_attr->video_timing, vo_attr->picture_aspect, HI_FALSE);  // find the real vic
    if (((vic >= HDMI_3840X2160P24_16_9 && vic <= HDMI_3840X2160P30_16_9) || vic == HDMI_4096X2160P24_256_135) &&
        vo_attr->stereo_mode == HDMI_3D_BUTT) {
        h14_vsif_content->format = HDMI_VIDEO_FORMAT_4K;
        for (cnt = 0; cnt <= CEA861_F_VIDEO_CODES_MAX_4K; cnt++) {
            video = drv_hdmi_video_codes_4k_get(cnt);
            if (video != HI_NULL && video->timing == vo_attr->video_timing) {
                h14_vsif_content->vic = video->hdmi_vic;
                break;
            }
        }
    } else if (vo_attr->stereo_mode < HDMI_3D_BUTT) {  // common 3D
        h14_vsif_content->format        = HDMI_VIDEO_FORMAT_3D;
        h14_vsif_content->_3d_structure = vo_attr->stereo_mode;
    } else {
        h14_vsif_content->format        = HDMI_VIDEO_FORMAT_NONE;
        h14_vsif_content->_3d_structure = vo_attr->stereo_mode;
    }
    vsif->length = vsif_length(h14_vsif_content, dolby_enable);
    /* add user vendor data */
    vsif->vsif_content.cea861_vsif.len = hdmi_dev->vendor_data.len;
    ret = memcpy_s(vsif->vsif_content.cea861_vsif.data, HDMI_VENDOR_USER_DATA_MAX_LEN,
        hdmi_dev->vendor_data.data, hdmi_dev->vendor_data.len);
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    ret = memcpy_s(&infoframe.vendor_infoframe, sizeof(hdmi_vendor_infoframe), vsif, sizeof(*vsif));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    drv_hdmi_infoframe_send(info_frame, HDMI_INFOFRAME_TYPE_VENDOR, &infoframe);

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_audio_infoframe_send(hdmi_infoframe_type *info_frame, hi_bool enable)
{
    errno_t ret;
    hdmi_infoframe infoframe = {0};
    hdmi_ao_attr *ao_attr  = HI_NULL;
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_audio_infoframe *audio_infoframe = HI_NULL;

    hdmi_if_null_return(info_frame, HI_FAILURE);

    if (!enable) {
        drv_hdmi_infoframe_enable_set(info_frame, HDMI_INFOFRAME_TYPE_AUDIO, HI_FALSE);
        return HI_SUCCESS;
    }
    hdmi_dev = osal_container_of(info_frame, __typeof__(*hdmi_dev), info_frame);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    ao_attr = &hdmi_dev->attr.ao_attr;
    audio_infoframe = &info_frame->audio_infoframe;
    (hi_void)memset_s(audio_infoframe, sizeof(info_frame->audio_infoframe), 0, sizeof(hdmi_audio_infoframe));

    if (ao_attr->sound_intf == HDMI_AUDIO_INTF_SPDIF || ao_attr->sound_intf == HDMI_AUDIO_INTF_HBRA) {
        hdmi_info("audio channel refer to stream \n");
    } else {
        hdmi_info("audio channel %u \n", ao_attr->channels);
        audio_infoframe->channels = (ao_attr->channels >= 2) ? (ao_attr->channels - 1) : 0; /* 2,1,0, channels number */
    }

    if (ao_attr->audio_code == HDMI_AUDIO_CODING_TYPE_AC3 || ao_attr->audio_code == HDMI_AUDIO_CODING_TYPE_DTS ||
        ao_attr->audio_code == HDMI_AUDIO_CODING_TYPE_EAC3 || ao_attr->audio_code == HDMI_AUDIO_CODING_TYPE_DTS_HD) {
        audio_infoframe->coding_type = ao_attr->audio_code;
    } else {
        audio_infoframe->coding_type = HDMI_AUDIO_CODING_TYPE_STREAM;
    }
    hdmi_info("aduio code : %u \n", ao_attr->audio_code);

    switch (ao_attr->channels) {
        case HDMI_AUDIO_FORMAT_3CH:
            audio_infoframe->channel_allocation = 0x01; /* 1 channel */
            break;
        case HDMI_AUDIO_FORMAT_6CH:
            audio_infoframe->channel_allocation = 0x0b; /* 3, 4 channel */
            break;
        case HDMI_AUDIO_FORMAT_8CH:
            audio_infoframe->channel_allocation = 0x13; /* 1, 2, 5 channel */
            break;
        default:
            audio_infoframe->channel_allocation = 0x00; /* 0 channel */
            break;
    }
    ret = memcpy_s(&infoframe.audio_infoframe, sizeof(infoframe.audio_infoframe),
        audio_infoframe, sizeof(hdmi_audio_infoframe));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    drv_hdmi_infoframe_send(info_frame, HDMI_INFOFRAME_TYPE_AUDIO, &infoframe);

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_null_packet_send(hdmi_infoframe_type *info_frame, hi_bool enable)
{
    hdmi_if_null_return(info_frame, HI_FAILURE);
    drv_hdmi_infoframe_enable_set(info_frame, HDMI_INFOFRAME_TYPE_NULL, enable);
    return HI_SUCCESS;
}


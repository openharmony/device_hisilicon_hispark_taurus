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
#ifndef __HDMI_EXT_H__
#define __HDMI_EXT_H__

#include <hi_common.h>

typedef enum {
    HI_HDMI_DEV_ID_0,
    HI_HDMI_DEV_ID_BUTT
} hdmi_dev_id;

typedef enum {
    HI_HDMI_COLORIMETRY_ITU601 = 1,              /* colorimetry ITU601 option */
    HI_HDMI_COLORIMETRY_ITU709,                  /* colorimetry ITU709 option */
    HI_HDMI_COLORIMETRY_2020_CONST_LUMINOUS = 9, /* colorimetry ITU2020 extended option */
    HI_HDMI_COLORIMETRY_2020_NON_CONST_LUMINOUS  /* colorimetry ITU2020 extended option */
} hdmi_colorimetry;

/* EOTF type */
typedef enum {
    HI_HDMI_EOTF_SDR_LUMIN,     /* traditional gamma - SDR luminance range */
    HI_HDMI_EOTF_HDR_LUMIN,     /* traditional gamma - HDR luminance range */
    HI_HDMI_EOTF_SMPTE_ST_2048, /* SMPTE ST 2084 */
    HI_HDMI_EOTF_HLG,           /* hybrid log-gamma(HLG) */
    HI_HDMI_EOTF_BUTT
} hdmi_eote_type;

/* metadata descriptor type(ID) */
typedef enum {
    HI_HDMI_HDR_METADATA_ID_0, /* static metadata type 1 */
    HI_HDMI_HDR_METADATA_ID_BUTT
} hdmi_hdr_metadata_id;

/* static metadata descriptor type 1 */
typedef struct {
    hi_u16 primaries0_x;        /* display_primaries_x[0], range[0,50000],in units of 0.00002 */
    hi_u16 primaries0_y;        /* display_primaries_y[0], range[0,50000],in units of 0.00002 */
    hi_u16 primaries1_x;        /* display_primaries_x[1], range[0,50000],in units of 0.00002 */
    hi_u16 primaries1_y;        /* display_primaries_y[1], range[0,50000],in units of 0.00002 */
    hi_u16 primaries2_x;        /* display_primaries_x[2], range[0,50000],in units of 0.00002 */
    hi_u16 primaries2_y;        /* display_primaries_y[2], range[0,50000],in units of 0.00002 */
    hi_u16 white_point_x;       /* white_point_x, range[0,50000], in units of 0.00002 */
    hi_u16 white_point_y;       /* white_point_y, range[0,50000], in units of 0.00002 */
    hi_u16 max_luminance;       /* max_display_mastering_luminance, in units of 1 cd/m^2 */
    hi_u16 min_luminance;       /* min_display_mastering_luminance, in units of 0.0001 cd/m^2 */
    hi_u16 max_light_level;     /* maximum content light level, in units of 1 cd/m^2 */
    hi_u16 average_light_level; /* maximum frame-average light level, in units of 1 cd/m^2 */
} hdmi_meta_descirper_1st;

/* static metadata descriptor type */
typedef union {
    hdmi_meta_descirper_1st type1; /* static metadata descriptor type1 */
} hdmi_meta_descirper;

/* HDR mode */
typedef enum {
    HI_HDMI_HDR_MODE_DISABLE,          /* default.HDR & dolby mode disable */
    HI_HDMI_HDR_MODE_DOLBY_NORMAL,     /* dolby normal(ycbcr422-12bit) mode enable */
    HI_HDMI_HDR_MODE_DOLBY_TUNNELING,  /* dolby tunneling(RGB-8bit) mode enable */
    HI_HDMI_HDR_MODE_CEA_861_3,        /* HDR standard mode enable(according to <CEA-861-3.2015>) */
    HI_HDMI_HDR_MODE_CEA_861_3_AUTHEN, /* HDR authen mode */
    HI_HDMI_HDR_MODE_BUTT
} hdmi_hdr_mode;

/* HDR attribute */
typedef struct {
    hdmi_hdr_mode        hdr_mode;
    hdmi_eote_type       eotf_type;
    hdmi_hdr_metadata_id metadata_id;
    hdmi_meta_descirper  descriptor;
    hdmi_colorimetry     colorimetry;
} hdmi_hdr_attr;

typedef enum {
    HI_HDMI_COLORSPACE_RGB,
    HI_HDMI_COLORSPACE_YCBCR444 = 2,
    HI_HDMI_COLORSPACE_BUTT
} hdmi_pixel_encoding;

typedef enum {
    HI_HDMI_QUANT_RANGE_LIMITED = 1,
    HI_HDMI_QUANT_RANGE_FULL,
    HI_HDMI_QUANT_RANGE_BUTT
} hdmi_quant_range;

typedef struct {
    hdmi_colorimetry colorimetry;  /* colorimetry from VDP */
    hdmi_quant_range quantization; /* quantization from VDP */
} hdmi_csc_param;

typedef struct {
    hdmi_pixel_encoding pixel_encoding; /* pixel_encoding from VDP */
} hdmi_video_param;

/* initial and exit service of hdmi module, called by SYS & VO module */
typedef hi_u32 fn_hdmi_init(hi_void);
typedef hi_u32 fn_hdmi_exit(hi_void);
typedef hi_s32 (*fn_hdmi_stop)(hdmi_dev_id hdmi);
typedef hi_s32 (*fn_hdmi_set_hdr_attr)(hdmi_dev_id hdmi, const hdmi_hdr_attr *hdr_attr);
typedef hi_s32 (*fn_hdmi_set_csc_param)(hdmi_dev_id hdmi, const hdmi_csc_param *csc_param);
typedef hi_s32 (*fn_hdmi_set_video_param)(hdmi_dev_id hdmi, const hdmi_video_param *video_param);

typedef struct {
    fn_hdmi_init           *pfn_init;
    fn_hdmi_exit           *pfn_exit;
    fn_hdmi_stop            pfn_stop;
    fn_hdmi_set_hdr_attr    pfn_set_hdr_attr;
    fn_hdmi_set_csc_param   pfn_csc_param_set;
    fn_hdmi_set_video_param pfn_video_param_set;
} hdmi_export_func;

#define ckfn_hdmi_entry() check_func_entry(HI_ID_HDMI)

#define ckfn_hdmi_init() (func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_init != NULL)

#define call_hdmi_init() func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_init()

#define ckfn_hdmi_deinit() (func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_exit != NULL)

#define call_hdmi_deinit() func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_exit()

#define ckfn_hdmi_stop(hdmi) (func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_stop != NULL)

#define call_hdmi_stop(hdmi) func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_stop(hdmi)

#define ckfn_hdmi_set_hdr_attr(hdmi, hdr_attr) (func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_set_hdr_attr != NULL)

#define call_hdmi_set_hdr_attr(hdmi, hdr_attr) \
    func_entry(hdmi_export_func, HI_ID_HDMI)->pfn_set_hdr_attr(hdmi, hdr_attr)

#endif /* __HDMI_EXT_H__ */


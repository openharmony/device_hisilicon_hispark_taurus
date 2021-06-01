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
#ifndef __DRV_HDMI_EDID_H__
#define __DRV_HDMI_EDID_H__

#include "hi_type.h"
#include "drv_hdmi_infoframe.h"
#include "drv_hdmi_common.h"

#define HDMI_EDID_BLOCK_SIZE              128
#define HDMI_EDID_MAX_BLOCK_NUM           4
#define HDMI_EDID_TOTAL_SIZE              (HDMI_EDID_BLOCK_SIZE * HDMI_EDID_MAX_BLOCK_NUM)
#define HDMI_EDID_MAX_HDMI14_TMDS_RATE    340000  // in khz
#define HDMI_EDID_MAX_HDMI20_TMDS_RATE    600000
#define HDMI_EDID_MAX_STDTIMNG_COUNT      8
#define HDMI_EDID_MAX_VIC_COUNT           128
#define HDMI_EDID_MAX_AUDIO_CAP_COUNT     16
#define HDMI_EDID_MAX_SAMPE_RATE_NUM      8
#define HDMI_EDID_MAX_BIT_DEPTH_NUM       6
#define HDMI_EDID_MAX_DETAIL_TIMING_COUNT 10
#define HDMI_EDID_MAX_SINKNAME_COUNT      14
#define HDMI_EDID_MAX_AUDIO_SPEAKER_COUNT 12
/* virtual video code, private define for VSDB 4k-format */
#define HDMI_EDID_MAX_REAL_VIC            255U
#define HDMI_VIC_VIRTUAL_BASE             (HDMI_EDID_MAX_REAL_VIC)
#define DOLBY_IEEE_OUI                    0x00D046

typedef enum {
    HDMI_EDID_ESTABTIMG_VESA_800X600_60,
    HDMI_EDID_ESTABTIMG_VESA_800X600_56,
    HDMI_EDID_ESTABTIMG_VESA_640X480_75,
    HDMI_EDID_ESTABTIMG_VESA_640X480_72,
    HDMI_EDID_ESTABTIMG_VGA_640X480_67,
    HDMI_EDID_ESTABTIMG_XGA_640X480_60,
    HDMI_EDID_ESTABTIMG_XGA2_720X400_88,
    HDMI_EDID_ESTABTIMG_XGA_720X400_70,
    HDMI_EDID_ESTABTIMG_VESA_1280X1024_75,
    HDMI_EDID_ESTABTIMG_VESA_1024X768_75,
    HDMI_EDID_ESTABTIMG_VESA_1024X768_70,
    HDMI_EDID_ESTABTIMG_VESA_1024X768_60,
    HDMI_EDID_ESTABTIMG_1024X768_87,
    HDMI_EDID_ESTABTIMG_832X624_75,
    HDMI_EDID_ESTABTIMG_VESA_800X600_75,
    HDMI_EDID_ESTABTIMG_VESA_800X600_72,
    HDMI_EDID_ESTABTIMG_VESA_1152X870_75,
    HDMI_EDID_ESTABTIMG_BUTT
} hdmi_edid_estab_timing;

typedef struct {
    hi_u32 vfb;            /* vertical front blank */
    hi_u32 vbb;            /* vertical back blank */
    hi_u32 vact;           /* vertical active area */
    hi_u32 hfb;            /* horizontal front blank */
    hi_u32 hbb;            /* horizontal back blank */
    hi_u32 hact;           /* horizontal active area */
    hi_u32 vpw;            /* vertical sync pulse width */
    hi_u32 hpw;            /* horizontal sync pulse width */
    hi_bool idv;           /* flag of data valid signal is needed flip */
    hi_bool ihs;           /* flag of horizontal sync pulse is needed flip */
    hi_bool ivs;           /* flag of vertical sync pulse is needed flip */
    hi_u32 image_width;    /* image width */
    hi_u32 image_height;   /* image height */
    hi_u32 aspect_ratio_w; /* aspect ratio width */
    hi_u32 aspect_ratio_h; /* aspect ratio height */
    hi_bool interlace;     /* flag of interlace */
    hi_u32 pixel_clk;      /* pixelc clk for this timing */
} hdmi_edid_pre_timing;

typedef struct {
    hi_u32 hor_active;
    hi_u32 ver_active;
    hi_u32 refresh_rate;
} hdmi_edid_std_timing;

typedef enum {
    HDMI_EDID_VIRTUAL_VIC_3840X2160_30 = HDMI_VIC_VIRTUAL_BASE + 1,
    HDMI_EDID_VIRTUAL_VIC_3840X2160_25,
    HDMI_EDID_VIRTUAL_VIC_3840X2160_24,
    HDMI_EDID_VIRTUAL_VIC_4096X2160_24,
    HDMI_EDID_VIRTUAL_VIC_VSDB_BUTT
} hdmi_edid_virtual_vic;

typedef struct {
    hdmi_audio_format_code aud_fmt_code;
    hdmi_sample_rate       support_sample_rate[HDMI_EDID_MAX_SAMPE_RATE_NUM];
    hdmi_audio_bit_depth   support_bit_depth[HDMI_EDID_MAX_BIT_DEPTH_NUM];
    hi_u32                 support_sample_rate_num;
    hi_u8                  aud_channel; /* aud channel of the coding type */
    hi_u32                 support_bit_depth_num;
    hi_u32                 max_bit_rate; /* khz */
} hdmi_edid_audio_info;

typedef enum {
    HDMI_EDID_AUDIO_SPEAKER_FL_FR,
    HDMI_EDID_AUDIO_SPEAKER_LFE,
    HDMI_EDID_AUDIO_SPEAKER_FC,
    HDMI_EDID_AUDIO_SPEAKER_RL_RR,
    HDMI_EDID_AUDIO_SPEAKER_RC,
    HDMI_EDID_AUDIO_SPEAKER_FLC_FRC,
    HDMI_EDID_AUDIO_SPEAKER_RLC_RRC,
    HDMI_EDID_AUDIO_SPEAKER_FLW_FRW,
    HDMI_EDID_AUDIO_SPEAKER_FLH_FRH,
    HDMI_EDID_AUDIO_SPEAKER_TC,
    HDMI_EDID_AUDIO_SPEAKER_FCH,
    HDMI_EDID_AUDIO_SPEAKER_BUTT
} hdmi_edid_audio_speaker;

typedef struct {
    hi_char mfrs_name[4];
    hi_u32  product_code;
    hi_u32  serial_number;
    hi_u32  week;
    hi_u32  year;
    hi_u8   sink_name[HDMI_EDID_MAX_SINKNAME_COUNT];
} hdmi_edid_manufacture_info;

typedef struct {
    hi_bool deep_color_y444;
    hi_bool deep_color30_bit;
    hi_bool deep_color36_bit;
    hi_bool deep_color48_bit;
} hdmi_edid_deep_color;

typedef struct {
    hi_bool xvycc601;
    hi_bool xvycc709;
    hi_bool sycc601;
    hi_bool adoble_ycc601;
    hi_bool adoble_rgb;
    hi_bool bt2020c_ycc;
    hi_bool bt2020_ycc;
    hi_bool bt2020_rgb;
} hdmi_edid_colorimetry;

typedef struct {
    hi_bool rgb444;
    hi_bool ycbcr422;
    hi_bool ycbcr444;
    hi_bool ycbcr420;
} hdmi_edid_color_space;

typedef struct {
    hi_bool phy_addr_valid;
    hi_u8 phy_addr_a;
    hi_u8 phy_addr_b;
    hi_u8 phy_addr_c;
    hi_u8 phy_addr_d;
} hdmi_edid_cec_address;

typedef struct {
    hi_bool deep_color30_bit;
    hi_bool deep_color36_bit;
    hi_bool deep_color48_bit;
} hdmi_edid_deep_color_y420;

typedef struct {
    hi_bool dsc_1p2;
    hi_bool dsc_native_420;
    hi_bool dsc_all_bpp;
    hi_bool dsc_10bpc;
    hi_bool dsc_12bpc;
    hi_bool dsc_16bpc;
    hi_u8 dsc_max_slices;
    hi_u8 dsc_max_frl_rate;
    hi_u8 dsc_total_chunk_k_bytes;
} hdmi_edid_dsc_info;

typedef enum {
    HDMI_EDID_3D_FRAME_PACKETING,                 /* 3d type:frame packing */
    HDMI_EDID_3D_FIELD_ALTERNATIVE,               /* 3d type:field alternative */
    HDMI_EDID_3D_LINE_ALTERNATIVE,                /* 3d type:line alternative */
    HDMI_EDID_3D_SIDE_BY_SIDE_FULL,               /* 3d type:side by side full */
    HDMI_EDID_3D_L_DEPTH,                         /* 3d type:L+depth */
    HDMI_EDID_3D_L_DEPTH_GRAPHICS_GRAPHICS_DEPTH, /* 3d type:L+depth+graphics+graphics-depth */
    HDMI_EDID_3D_TOP_AND_BOTTOM,                  /* 3d type:top and bottom */
    HDMI_EDID_3D_SIDE_BY_SIDE_HALF = 0x08,        /* 3d type:side by side half */
    HDMI_EDID_3D_BUTT
} hdmi_edid_3d_type;

typedef struct {
    hi_bool support_3d;
    hi_bool support_3d_type[HDMI_EDID_3D_BUTT];
} hdmi_edid_3d_info;

typedef enum {
    HDMI_EDID_DATA_VALIDSINK,
    HDMI_EDID_DATA_VALIDTEST,
    HDMI_EDID_DATA_INVALID,
    HDMI_EDID_DATA_BUTT
} hdmi_edid_data;

typedef enum {
    HDMI_EDID_UPDATE_SINK,  /* update from SINK by DDC(HAL support interface). */
    HDMI_EDID_UPDATE_TEST0, /* update from EDID_TEST,and the following same. */
    HDMI_EDID_UPDATE_TEST1,
    HDMI_EDID_UPDATE_TEST2,
    HDMI_EDID_UPDATE_TEST3,
    HDMI_EDID_UPDATE_TEST4,
    HDMI_EDID_UPDATE_TEST5,
    HDMI_EDID_UPDATE_TEST6,
    HDMI_EDID_UPDATE_TEST7,
    HDMI_EDID_UPDATE_TEST8,
    HDMI_EDID_UPDATE_TEST9,
    HDMI_EDID_UPDATE_TEST10,
    HDMI_EDID_UPDATE_TEST11,
    HDMI_EDID_UPDATE_TEST12,
    HDMI_EDID_UPDATE_TEST13,
    HDMI_EDID_UPDATE_TEST14,
    HDMI_EDID_UPDATE_TEST15,
    HDMI_EDID_UPDATE_TEST16,
    HDMI_EDID_UPDATE_TEST17,
    HDMI_EDID_UPDATE_TEST18,
    HDMI_EDID_UPDATE_TEST19,
    HDMI_EDID_UPDATE_TEST20,
    HDMI_EDID_UPDATE_TEST21,
    HDMI_EDID_UPDATE_TEST22,
    HDMI_EDID_UPDATE_TEST23,
    HDMI_EDID_UPDATE_TEST24,
    HDMI_EDID_UPDATE_DEBUG,
    HDMI_EDID_UPDATE_BUTT
} hdmi_edid_updata_mode;

/*
 * EDID parse-error number.
 * NOTE:parse-error meaning that something we don't expect occurs,
 * and it can lead other's raw field to parse error. so we stop and return FAILURE when occurred.
 * user should read the status when returned  FAILURE.
 */
typedef enum {
    EDID_PARSE_ERR_NONE,
    EDID_PARSE_ERR_CHECKSUM,
    EDID_PARSE_ERR_HEADER,
    EDID_PARSE_ERR_FST_BLK_VER,
    EDID_PARSE_ERR_TAG_UNKNOWN,
    EDID_PARSE_ERR_CEA_REVERION,
    EDID_PARSE_ERR_BUTT
} hdmi_edid_parse_err;

/*
 * EDID parse-warning information bit shift code.
 * NOTE:parse-warning meaning that something we don't expect occurs,
 * but it can't lead other's raw field to parse error.
 * user should read the status to check when parsing is finished.
 */
typedef enum {
    EDID_PARSE_WARN_NONE,
    EDID_PARSE_WARN_VENDOR_INVALID,
    EDID_PARSE_WARN_DTD_OVER,
    EDID_PARSE_WARN_DTD_INVALID,
    EDID_PARSE_WARN_EXT_BLK_ZERO,
    EDID_PARSE_WARN_EXT_BLK_OVER,
    EDID_PARSE_WARN_AUDIO_CNT_OVER,
    EDID_PARSE_WARN_AUDIO_FMT_INVALID,
    EDID_PARSE_WARN_VIC_CNT_OVER,
    EDID_PARSE_WARN_VIC_INVALID,
    EDID_PARSE_WARN_VSDB_INVALID,
    EDID_PARSE_WARN_HFVSDB_INVALID,
    EDID_PARSE_WARN_SPKDB_INVALID,
    EDID_PARSE_WARN_Y420VDB_VIC_OVER,
    EDID_PARSE_WARN_BLOCKLEN_INVALID,
    EDID_PARSE_WARN_BUTT
} hdmi_edid_parse_warn;

typedef struct {
    hi_bool audio_fmt_supported[HDMI_EDID_MAX_AUDIO_CAP_COUNT - 1];
    hi_u32 audio_sample_rate_supported[HDMI_EDID_MAX_SAMPE_RATE_NUM]; /* PCM smprate capability */
    hi_u32 max_pcm_channels;
} hdmi_audio_capability;

typedef struct {
    hi_bool cap_sink;  /* [current] is capability parse from SINK or EDID_TEST */
    hi_bool cap_valid; /* [current] is capability valid */
    hi_bool raw_valid; /* [current] is raw data valid */
    hi_u32  raw_len;
    hdmi_edid_parse_err parse_err; /* [current] parse error code,in byte */
    hi_u32 parse_warn;             /* [current] parse warning ,in bits shift,see hdmi_edid_parse_warn */
    hi_u32 raw_get_err_cnt;        /* error from raw data getting count */
    hi_u32 cap_get_err_cnt;        /* error from capability getting count */
    hi_u32 raw_update_err_cnt;     /* error from raw data updating count */
} hdmi_edid_status;

typedef struct {
    hi_u16 red_x;
    hi_u16 red_y;
    hi_u16 green_x;
    hi_u16 green_y;
    hi_u16 blue_x;
    hi_u16 blue_y;
    hi_u16 white_x;
    hi_u16 white_y;
} hdmi_edid_phosphor;

typedef struct {
    /* the major version of display management implemented. only support when vsvdb_version = 0. */
    hi_u8 d_mmajor_version;
    /* the minor version of display management implemented. only support when vsvdb_version = 0. */
    hi_u8 d_mminor_version;
    /*
     * white point chromaticity coordinate x, bit[11:0]valid. real value = SUM OF bit[N]*2^-(12-N),
     * only support when vsvdb_version = 0.
     */
    hi_u16 white_x;
    /*
     * white point  chromaticity coordinate y, bit[11:0]valid. real value = SUM OF bit[N]*2^-(12-N),
     * only support when vsvdb_version = 0.
     */
    hi_u16 white_y;
} hdmi_edid_dolby_caps_ver_type0;

typedef struct {
    /*
     * 0:based on display management v2.x;
     * 1:based on the video and blending pipeline v3.x;
     * 2-7: reserved. only support when vsvdb_version = 1.
     */
    hi_u8 dm_version;
    /*
     * this bit is valid only vsvdb_version = 1.
     * 0: dolby vision HDMI sink's colorimetry is close to rec.709,
     * 1: EDR HDMI sink's colorimetry is close to P3, if byte[9] to byte[14] are present, ignores this bit.
     */
    hi_bool colorimetry;
} hdmi_edid_dolby_caps_ver_type1;

typedef union {
    hdmi_edid_dolby_caps_ver_type0 dolby_caps_ver0;
    hdmi_edid_dolby_caps_ver_type1 dolby_caps_ver1;
} hdmi_edid_dolby;

typedef struct {
    /*
     * the version of VSVDB, if this equal to 1, the value of min_luminance and max_luminance only 7 bits,
     * the rx, ry, gx, gy, bx, by only 8 bits. see < dolby  vision  HDMI transmission specification issue 2.6
     */
    hi_u8 vsvdb_version;
    /* support(HI_TRUE) or not support(HI_FALSE) a YUV422-12_bit dolby signal */
    hi_bool yuv422_12_bit;
    /* capable of processing a max timing 3840X2160p60(HI_TRUE) /3840X2160p30(HI_FALSE) */
    hi_bool b2160_p60;
    /* support(HI_TRUE) or not support(HI_FALSE) global dimming. */
    hi_bool global_dimming;
    /* red primary chromaticity coordinate x, bit[11:0]valid.real value =SUM OF bit[N]*2^-(12-N) */
    hi_u16 red_x;
    /* red primary chromaticity coordinate y, bit[11:0]valid.real value =SUM OF bit[N]*2^-(12-N) */
    hi_u16 red_y;
    /* green primary chromaticity coordinate x, bit[11:0]valid.real value =SUM OF bit[N]*2^-(12-N) */
    hi_u16 green_x;
    /* green primary chromaticity coordinate y, bit[11:0]valid.real value =SUM OF bit[N]*2^-(12-N) */
    hi_u16 green_y;
    /* blue primary chromaticity coordinate x, bit[11:0]valid.real value =SUM OF bit[N]*2^-(12-N) */
    hi_u16 blue_x;
    /* blue primary chromaticity coordinate y, bit[11:0]valid.real value =SUM OF bit[N]*2^-(12-N) */
    hi_u16 blue_y;
    /* minimum luminance value of the target display, [0,4095] cd/m^2 */
    hi_u16 min_luminance;
    /* maximum luminance value of the target display, [0,4095] cd/m^2 */
    hi_u16 max_luminance;
    /* other caps in dolby VSVDB. distinction between V0(vsvdb_version=0) and V1(vsvdb_version=1). */
    hdmi_edid_dolby un_other_caps;
    /* dolby OUI */
    hi_u32 dolby_oui;
} hdmi_edid_dolby_cap;

typedef struct {
    hi_bool eotf_sdr;          /* traditional gamma - SDR luminance range */
    hi_bool eotf_hdr;          /* traditional gamma - HDR luminance range */
    hi_bool eotf_smpte_st2084; /* SMPTE ST 2084 */
    hi_bool eotf_hlg;          /* hybrid log-gamma */
    hi_bool eotf_future;       /* future EOTF */
} hdmi_edid_eotf;

typedef struct {
    hi_bool descriptor_type1; /* static metadata type 1 */
} hdmi_edid_hdr_metadata_type;

typedef struct {
    /* EOTF support. */
    hdmi_edid_eotf eotf;
    /* static metadata descriptor type(ID) support. */
    hdmi_edid_hdr_metadata_type metadata;
    /* desired content max luminance data. real value(in 1cd/m^2) = 50.2^(CV/32) */
    hi_u8 max_luminance_cv;
    /* desired content max frame-average luminance. real value(in 1cd/m^2) = 50.2^(CV/32) */
    hi_u8 average_lumin_cv;
    /*
     * desired content min luminance data.
     * real value(in 1cd/m^2) = max_luminance_cv * (min_luminance_cv/255)^2 / 100
     */
    hi_u8 min_luminance_cv;
} hdmi_edid_hdr_cap;

typedef struct {
    hi_bool               dolby_support;
    hi_bool               hdr_support;
    hdmi_edid_dolby_cap   dolby_caps;
    hdmi_edid_hdr_cap     hdr_caps;
    hdmi_edid_colorimetry color_metry;
} hdmi_video_capability;

typedef struct {
    hi_u8 max_image_width;  /* the disp image max width  (0~255)cm */
    hi_u8 max_image_height; /* the disp image max height (0~255)cm */
} hdmi_edid_base_disp_para;

typedef struct {
    hi_bool support_hdmi;
    hi_u32  native_format;
    hi_u32  estab_num;
    hi_u32  estab_timing[HDMI_EDID_ESTABTIMG_BUTT];
    hi_u32  support_vic_num;
    hi_u32  support_format[HDMI_EDID_MAX_VIC_COUNT];
    hdmi_edid_std_timing  std_timing[HDMI_EDID_MAX_STDTIMNG_COUNT];
    hdmi_edid_3d_info     _3d_info;
    hdmi_edid_deep_color  deep_color;
    hdmi_edid_colorimetry color_metry;
    hdmi_edid_color_space color_space;
    hdmi_edid_audio_info  audio_info[HDMI_EDID_MAX_AUDIO_CAP_COUNT];
    hi_u8   md_bit;
    hi_u32  audio_info_num;
    hi_bool support_audio_speaker[HDMI_EDID_MAX_AUDIO_SPEAKER_COUNT];
    hi_u8   ext_block_num;
    hi_u8   version;
    hi_u8   revision;
    hdmi_edid_manufacture_info mfrs_info;
    hdmi_edid_cec_address      cec_addr;
    hdmi_edid_pre_timing       perfer_timing[HDMI_EDID_MAX_DETAIL_TIMING_COUNT];
    hi_bool support_dvi_dual;
    hi_bool supports_ai;
    hi_u32  perfer_timing_num;
    hi_u32  max_tmds_clock;
    hi_bool i_latency_fields_present;
    hi_bool latency_fields_present;
    hi_bool hdmi_video_present;
    hi_u8   video_latency;
    hi_u8   audio_latency;
    hi_u8   interlaced_video_latency;
    hi_u8   interlaced_audio_latency;
    hi_bool support_scramble;
    hi_bool support_hdmi_20;
    hi_bool support_scdc;
    hi_bool support_rr_capable;
    hi_bool support_lte340_mcsc_scrameble;
    hi_bool support3d_osd_disparity;
    hi_bool support3d_dual_view;
    hi_bool support3d_independent_view;
    hi_u32  support_y420_vic_num;
    hi_u32  support_y420_format[HDMI_EDID_MAX_VIC_COUNT];
    hi_u32  only_support_y420_vic_num;
    hi_u32  only_support_y420_format[HDMI_EDID_MAX_VIC_COUNT];
    hi_bool ycc_qrange_selectable;
    hi_bool rgb_qrange_selectable;
    hi_bool support_ffe;
    hi_bool ccbpci;
    hi_u8   max_frl_rate;
    hi_bool fapa_start_location;
    hi_bool allm;
    hi_bool fva;
    hi_bool cnm_vrr;
    hi_bool cinema_vrr;
    hi_bool m_delta;
    hi_u8   vrr_min;
    hi_u16  vrr_max;
    hdmi_edid_dsc_info        dsc_info;
    hdmi_edid_deep_color_y420 deep_color_y420;
    hdmi_edid_phosphor        phos_or_chrom_cap;
    hdmi_edid_hdr_cap         hdr_cap;
    hdmi_edid_dolby_cap       dolby_cap;
    hdmi_hdcp_capability      hdcp_support;
    hdmi_edid_base_disp_para  basic_disp_para;
} hdmi_sink_capability;

typedef struct {
    hdmi_edid_status status;
    hi_u8 edid_raw[HDMI_EDID_TOTAL_SIZE];
    hdmi_sink_capability capability;
} hdmi_edid_info;

hdmi_edid_data drv_hdmi_edid_capability_get(hdmi_edid_info *edid_info, hdmi_sink_capability **capability);

hi_s32 drv_hdmi_edid_raw_get(hdmi_edid_info *edid_info, hi_u8 *raw_data, hi_u32 len);

hi_s32 drv_hdmi_edid_update(hdmi_edid_info *edid_info, hdmi_edid_updata_mode mode);

hi_s32 dfrv_hdmi_edid_status_get(hdmi_edid_info *edid_info, hdmi_edid_status *status);

hi_s32 drv_hdmi_edid_reset(hdmi_edid_info *edid_info);

#endif /* __DRV_HDMI_EDID_H__ */


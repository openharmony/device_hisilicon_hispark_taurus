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
#ifndef __HI_COMM_HDMI_ADAPT_H__
#define __HI_COMM_HDMI_ADAPT_H__

#include "hi_type.h"
#include "hi_comm_hdmi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef HI_HDMI_ID_E                 hi_hdmi_id;
typedef HI_HDMI_INFOFRAME_TYPE_E     hi_hdmi_infoframe_type;
typedef HI_HDMI_FORCE_ACTION_E       hi_hdmi_force_active;
typedef HI_HDMI_TRACE_LEN_E          hi_hdmi_trace_len;
typedef HI_HDMI_VIDEO_FMT_E          hi_hdmi_video_fmt;
typedef HI_HDMI_AUDIO_FORMAT_CODE_E  hi_hdmi_audio_format_code;
typedef HI_HDMI_SAMPLE_RATE_E        hi_hdmi_sample_rate;
typedef HI_HDMI_BIT_DEPTH_E          hi_hdmi_bit_depth;
typedef HI_HDMI_VIDEO_MODE_E         hi_hdmi_video_mode;
typedef HI_HDMI_QUANTIZATION_E       hi_hdmi_quantization;
typedef HI_HDMI_DEEP_COLOR_E         hi_hdmi_deep_color;
typedef HI_HDMI_SND_INTERFACE_E      hi_hdmi_snd_inteface;
typedef HI_HDMI_AUDIO_CHANEL_CNT_E   hi_hdmi_audio_chanel_cnt;
typedef HI_HDMI_CODING_TYPE_E        hi_hdmi_coding_type;
typedef HI_HDMI_AUDIO_SAMPLE_SIZE_E  hi_hdmi_audio_sample_size;
typedef HI_HDMI_AUDIO_SAMPLE_FREQ_E  hi_hdmi_audio_sample_freq;
typedef HI_HDMI_LEVEL_SHIFT_VALUE_E  hi_hdmi_level_shift_value;
typedef HI_HDMI_LFE_PLAYBACK_LEVEL_E hi_hdmi_lfe_playback_level;
typedef HI_HDMI_COLOR_SPACE_E        hi_hdmi_color_space;
typedef HI_HDMI_BARINFO_E            hi_hdmi_bar_info;
typedef HI_HDMI_SCANINFO_E           hi_hdmi_scan_info;
typedef HI_HDMI_COLORIMETRY_E        hi_hdmi_colormetry;
typedef HI_HDMI_EXT_COLORIMETRY_E    hi_hdmi_ext_colormetry;
typedef HI_HDMI_PIC_ASPECT_RATIO_E   hi_pic_aspect_ratio;
typedef HI_HDMI_ACT_ASPECT_RATIO_E   hi_hdmi_act_aspect_ratio;
typedef HI_HDMI_PICTURE_SCALING_E    hi_hdmi_picture_scaline;
typedef HI_HDMI_RGB_QUAN_RAGE_E      hi_hdmi_rgb_quan_rage;
typedef HI_HDMI_PIXEL_REPETITION_E   hi_hdmi_pixel_repetition;
typedef HI_HDMI_CONTENT_TYPE_E       hi_hdmi_content_type;
typedef HI_HDMI_YCC_QUAN_RAGE_E      hi_hdmi_ycc_quan_rage;
typedef HI_CEC_LOGICALADD_E          hi_cec_logical_addr;
typedef HI_HDMI_EVENT_TYPE_E         hi_hdmi_event_type;

typedef struct {
    hi_u8 length;
    hi_u8 data[15]; // 15 for max data length
} hi_cec_raw_data;

typedef struct {
    hi_cec_logical_addr src_addr;
    hi_cec_logical_addr dst_addr;
    hi_u8 opcode;
    hi_cec_raw_data raw_data;
} hi_mpi_hdmi_cec_cmd;

typedef void (*hi_hdmi_callback)(hi_hdmi_event_type event, hi_void *private_data);
typedef hi_void (*hi_hdmi_cec_callback)(hi_hdmi_id hdmi, hi_mpi_hdmi_cec_cmd *cec_cmd, hi_void *data);

typedef struct {
    /* Audio coding type */
    hi_hdmi_audio_format_code aud_fmt_code;
    /* Audio sample rate */
    hi_hdmi_sample_rate support_sample_rate[HI_HDMI_MAX_SAMPLE_RATE_NUM];
    /* Aud Channel of the coding type */
    hi_u8 aud_channel;
    /* sample bit depth,when audio format code is HI_UNF_EDID_AUDIO_FORMAT_CODE_PCM */
    hi_hdmi_bit_depth support_bit_depth[HI_HDMI_MAX_BIT_DEPTH_NUM];
    /* sample bit depth Num,when audio format code is HI_UNF_EDID_AUDIO_FORMAT_CODE_PCM */
    hi_u32 support_bit_depth_num;
    /*
     * enter max bit rate, when audio format code is
     * HI_UNF_EDID_AUDIO_FORMAT_CODE_AC3 - HI_UNF_EDID_AUDIO_FORMAT_CODE_ATRAC
     */
    hi_u32 max_bit_rate;
} hi_hdmi_audio_info;

typedef struct {
    hi_bool eotf_sdr;          /* Traditional gamma - SDR Luminance Range. */
    hi_bool eotf_hdr;          /* Traditional gamma - HDR Luminance Range. */
    hi_bool eotf_smpte_st2084; /* SMPTE ST 2084 */
    hi_bool eotf_hlg;          /* Hybrid Log-Gamma(HLG) based on Recommendation ITU-R BT.2100-0 */
    hi_bool eotf_future;       /* Future EOTF */
} hi_hdmi_eotf;

typedef struct {
    hi_bool descriptor_type1; /* Support Static Metadata Type 1 or not. */
} hi_hdmi_hdr_matadata_type;

typedef struct {
    /* EOTF support. */
    hi_hdmi_eotf eotf;
    /* static metadata descriptor type(ID) support. */
    hi_hdmi_hdr_matadata_type metadata;
    /* Desired Content Max Luminance Data. Real value(in 1cd/m^2) = 50.2^(CV/32) */
    hi_u8 max_luminance_cv;
    /* Desired Content Max Frame-average Luminance. Real value(in 1cd/m^2) = 50.2^(CV/32) */
    hi_u8 average_lumin_cv;
    /*
     * Desired Content Min Luminance Data.
     * Real value(in 1cd/m^2) = u8MaxLuminance_CV * (u8MinLuminance_CV/255)^2 / 100
     */
    hi_u8 min_luminance_cv;
} hi_hdmi_hdr_cap;

typedef struct {
    hi_u32  vfb;            /* vertical front blank */
    hi_u32  vbb;            /* vertical back blank */
    hi_u32  vact;           /* vertical active area */
    hi_u32  hfb;            /* horizontal front blank */
    hi_u32  hbb;            /* horizontal back blank */
    hi_u32  hact;           /* horizontal active area */
    hi_u32  vpw;            /* vertical sync pulse width */
    hi_u32  hpw;            /* horizontal sync pulse width */
    hi_bool idv;            /* flag of data valid signal is needed flip */
    hi_bool ihs;            /* flag of horizontal sync pulse is needed flip */
    hi_bool ivs;            /* flag of vertical sync pulse is needed flip */
    hi_u32  image_width;    /* image width */
    hi_u32  image_height;   /* image height */
    hi_u32  aspect_ratio_w; /* aspect ratio width */
    hi_u32  aspect_ratio_h; /* aspect ratio height */
    hi_bool interlace;      /* flag of interlace */
    hi_s32  pixel_clk;      /* pixelc clk for this timing, unit:KHz */
} hi_hdmi_timing_info;

typedef struct {
    hi_u32 det_timing_num;
    hi_hdmi_timing_info det_timing[10];
} hi_hdmi_det_timing;

typedef struct {
    /* Whether the devices are connected. */
    hi_bool connected;
    /* Whether the HDMI is supported by the device. If the HDMI is not supported by the device, the device is DVI. */
    hi_bool support_hdmi;
    /* Whether the sink device is powered on. */
    hi_bool is_sink_power_on;
    /*
     * Whether the EDID obtains the flag from the sink device.
     * HI_TRUE: The EDID information is correctly read.
     * HI_FALSE: default settings
     */
    hi_bool is_real_edid;
    /* Physical resolution of the display device. */
    hi_hdmi_video_fmt native_video_format;
    /*
     * Video capability set.
     * HI_TRUE: This display format is supported.
     * HI_FALSE: This display format is not supported.
     */
    hi_bool video_fmt_supported[HI_HDMI_VIDEO_FMT_BUTT];
    /*
     * Whether the YCBCR display is supported.
     * HI_TRUE: The YCBCR display is supported.
     * HI_FALSE: Only red-green-blue (RGB) is supported.
     */
    hi_bool support_ycbcr;
    /* Whether the xvYCC601 color format is supported. */
    hi_bool support_xvycc601;
    /* Whether the xvYCC709 color format is supported. */
    hi_bool support_xvycc709;
    hi_bool support_bt2020_ycc;
    /* Transfer profile supported by xvYCC601. 1: P0; 2: P1; 4: P2. */
    hi_u8 md_bit;
    /* Num of audio Info */
    hi_u32 audio_info_num;
    /* Audio Info.For details, see Table 37 in EIA-CEA-861-D. */
    hi_hdmi_audio_info audio_info[HI_HDMI_MAX_AUDIO_CAP_COUNT];
    /* Speaker position. For details, see the definition of SpeakerDATABlock in EIA-CEA-861-D. */
    hi_bool speaker[HDMI_AUDIO_SPEAKER_BUTT];
    /* Device vendor flag */
    hi_u8 id_manufacture_name[4];
    /* Device ID. */
    hi_u32 id_product_code;
    /* Device sequence number. */
    hi_u32 id_serial_number;
    /* Device production data (week). */
    hi_u32 week_of_manufacture;
    /* Set the production data (year). */
    hi_u32 year_of_manufacture;
    /* Device version number */
    hi_u8 version;
    /* Device sub version number */
    hi_u8 revision;
    /* EDID extended block number */
    hi_u8 edid_extern_block_num;
    /* Valid flag of the consumer electronics control (CEC) physical address */
    hi_bool is_phy_addr_valid;
    /* CEC physical address A */
    hi_u8 phy_addr_a;
    /* CEC physical address B */
    hi_u8 phy_addr_b;
    /* CEC physical address C */
    hi_u8 phy_addr_c;
    /* CEC physical address D */
    hi_u8 phy_addr_d;
    /* Whether to support the DVI dual-link operation. */
    hi_bool support_dvi_dual;
    /* Whether to support the YCBCR 4:4:4 deep-color mode. */
    hi_bool support_deep_color_ycbcr444;
    /* Whether to support the deep-color 30-bit mode. */
    hi_bool support_deep_color_30bit;
    /* Whether to support the deep-color 36-bit mode. */
    hi_bool support_deep_color_36bit;
    /* Whether to support the deep-color 48-bit mode. */
    hi_bool support_deep_color_48bit;
    /* Whether to support the Supports_AI mode. */
    hi_bool support_ai;
    /* Maximum TMDS clock. */
    hi_u32 max_tmds_clock;
    /* Delay flag bit. */
    hi_bool i_latency_fields_present;
    /* whether Video_Latency and Audio_Latency fields are present */
    hi_bool latency_fields_present;
    /* Special video format */
    hi_bool hdmi_video_present;
    /* Video delay */
    hi_u8 video_latency;
    /* Audio delay */
    hi_u8 audio_latency;
    /* Video delay in interlaced video mode */
    hi_u8 interlaced_video_latency;
    /* Audio delay in interlaced video mode */
    hi_u8 interlaced_audio_latency;
    /* Whether to support the YUV420 deep-color 30-bit mode. */
    hi_bool support_y420_dc_30bit;
    /* Whether to support the YUV420 deep-color 36-bit mode. */
    hi_bool support_y420_dc_36bit;
    /* Whether to support the YUV420 deep-color 48-bit mode. */
    hi_bool support_y420_dc_48bit;
    /* Whether to support HDMI2.0. */
    hi_bool support_hdmi_20;
    /* the format support YUV420 */
    hi_bool support_y420_format[HI_HDMI_VIDEO_FMT_BUTT];
    /* the format only support YUV420 */
    hi_bool only_support_y420_format[HI_HDMI_VIDEO_FMT_BUTT];
    /*
     * flag of RGB Quantization Range selectable.
     * when HI_TRUE,sink expect full/limited range is selectable;
     * or sink expect default range.
     */
    hi_bool ycc_qrange_selectable;
    /*
     * flag of YCC Quantization Range selectable.
     * when HI_TRUE,sink expect full/limited range is selectable;or sink expect default range.
     */
    hi_bool rgb_qrange_selectable;
    /* HDR support flag.When HDR Static Metadata Data Block is indicated,flag HI_TRUE;or,HI_FALSE. */
    hi_bool hdr_support;
    /* HDR capability */
    hi_hdmi_hdr_cap hdr;
    /* Detailed timing information */
    hi_hdmi_det_timing detailed_timing;
} hi_hdmi_sink_capability;

typedef struct {
    /* Whether to forcibly output the video over the HDMI. */
    hi_bool enable_hdmi;
    /*
     * Whether to output video.
     * The value must be HI_TRUE.If the value is HI_FALSE, the HDMI is forcibly set to HI_TRUE.
     */
    hi_bool enable_video;
    /* Video norm. This value of the video norm must be consistent with the norm of the video output. */
    hi_hdmi_video_fmt video_fmt;
    /* HDMI input video input mode.VIDEO_MODE_YCBCR444,VIDEO_MODE_YCBCR422,VIDEO_MODE_YCBCR420,VIDEO_MODE_RGB444 */
    hi_hdmi_video_mode vid_in_mode;
    /* HDMI output video output mode.VIDEO_MODE_YCBCR444,VIDEO_MODE_YCBCR422,VIDEO_MODE_YCBCR420,VIDEO_MODE_RGB444 */
    hi_hdmi_video_mode vid_out_mode;
    /* CSC output quantization range HDMI_QUANTIZATION_RANGE_LIMITED, HDMI_QUANTIZATION_RANGE_FULL>**/
    hi_hdmi_quantization out_csc_quantization;
    /* DeepColor output mode.It is HI_HDMI_DEEP_COLOR_24BIT by default. */
    hi_hdmi_deep_color deep_color_mode;
    /* Whether to enable the xvYCC output mode.It is HI_FALSE by default. */
    hi_bool xvycc_mode;
    /* Whether to enable the audio. */
    hi_bool enable_audio;
    /* HDMI audio source.It is set to HI_HDMI_SND_INTERFACE_I2S, which needs to be consistent with the VO interface. */
    hi_hdmi_snd_inteface sound_intf;
    /* Multi-channel or stereo 0: stereo 1: eight-channel fixed for multi-channel */
    hi_bool is_multi_channel;
    /* Audio sampling rate. This parameter needs to be consistent with that of the VO. */
    hi_hdmi_sample_rate sample_rate;
    /* Audio down sampling rate parameter. It is 0 by default. */
    hi_u8 down_sample_parm;
    /* Audio bit width. It is 16 by default. This parameter needs to be consistent with that of the VO. */
    hi_hdmi_bit_depth bit_depth;
    /* Reserved. It is set to 0. I2S control (0x7A:0x1D) */
    hi_u8 i2s_ctl_vbit;
    /* Whether to enable AVI InfoFrame.It is recommended to enable this function. */
    hi_bool enable_avi_infoframe;
    /* Whether to enable AUDIO InfoFrame.It is recommended to enable this function. */
    hi_bool enable_aud_infoframe;
    /* Whether to enable SPD InfoFrame.It is recommended to disable this function. */
    hi_bool enable_spd_infoframe;
    /* Whether to enable MPEG InfoFrame.It is recommended to disable this function. */
    hi_bool enable_mpeg_infoframe;
    /* Whether to enable the debug information in the HDMI.It is recommended to disable this function. */
    hi_bool debug_flag;
    /* Whether to enable the HDCP.0: disabled 1: enabled */
    hi_bool enable_hdcp;
    /* Whether to enable 3D mode. 0: disabled 1: enabled */
    hi_bool enable_3d;
    /* 3D Parameter. The default value is 9. */
    hi_u8 param_3d;
    /* When get capability fail,HDMI forcibly(priority) judgments output video mode.Default is HI_HDMI_FORCE_HDMI. */
    hi_hdmi_force_active default_mode;
    /* Whether to enable auth mode. 0: disabled 1: enabled */
    hi_bool auth_mode;
    /*
     * Enable flag of video mode & DVI adapting case of user setting incorrect,
     * default:HI_TRUE.When user have no any adapting strategy,suggestion HI_TRUE
     */
    hi_bool enable_vid_mode_adapt;
    /*
     * Enable flag of deep color mode adapting case of user setting incorrect,
     * default: HI_FALSE.When user have no any adapting strategy,suggestion HI_TRUE
     */
    hi_bool enable_deep_color_adapt;
    /*
     * Pixclk of enVideoFmt(unit is kHz).
     * (This param is valid only when enVideoFmt is HI_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE)
     */
    hi_u32 pix_clk;
} hi_hdmi_attr;

typedef struct {
    hi_bool edid_valid;
    hi_u32  edid_length;
    hi_u8   edid[512];
} hi_hdmi_edid;

typedef struct {
    hi_hdmi_audio_chanel_cnt   channel_count;
    hi_hdmi_coding_type        coding_type;
    hi_hdmi_audio_sample_size  sample_size;
    hi_hdmi_audio_sample_freq  sampling_frequency;
    hi_u8                      channel_alloc; /* Channel/Speaker Allocation.Range [0,255] */
    hi_hdmi_level_shift_value  level_shift;
    hi_hdmi_lfe_playback_level lfe_playback_level;
    hi_bool                    down_mix_inhibit;
} hi_hdmi_aud_infoframe_ver1;

typedef struct {
    hi_hdmi_video_fmt        timing_mode;
    hi_hdmi_color_space      color_space;
    hi_bool                  active_info_present;
    hi_hdmi_bar_info         bar_info;
    hi_hdmi_scan_info        scan_info;
    hi_hdmi_colormetry       colorimetry;
    hi_hdmi_ext_colormetry   ext_colorimetry;
    hi_pic_aspect_ratio      aspect_ratio;
    hi_hdmi_act_aspect_ratio active_aspect_ratio;
    hi_hdmi_picture_scaline  picture_scaling;
    hi_hdmi_rgb_quan_rage    rgb_quantization;
    hi_bool                  is_it_content;
    hi_hdmi_pixel_repetition pixel_repetition;
    hi_hdmi_content_type     content_type;
    hi_hdmi_ycc_quan_rage    ycc_quantization;
    hi_u16                   line_n_end_of_top_bar;
    hi_u16                   line_n_start_of_bot_bar;
    hi_u16                   pixel_n_end_of_left_bar;
    hi_u16                   pixel_n_start_of_right_bar;
} hi_hdmi_avi_infoframe_ver2;

typedef struct {
    hi_u8 vendor_name[HI_HDMI_VENDOR_NAME_SIZE];
    hi_u8 product_description[HI_HDMI_VENDOR_DESC_SIZE];
} hi_hdmi_spd_infoframe;

typedef struct {
    hi_u32  mpeg_bit_rate;
    hi_bool is_field_repeated;
} hi_hdmi_mpegsource_infoframe;

typedef struct {
    hi_u8 data_len;
    hi_u8 user_data[HI_HDMI_VENDOR_USER_DATA_MAX_LEN];
} hi_hdmi_vendorspec_infoframe;

typedef union {
    hi_hdmi_avi_infoframe_ver2   avi_infoframe;
    hi_hdmi_aud_infoframe_ver1   aud_infoframe;
    hi_hdmi_spd_infoframe        spd_infoframe;
    hi_hdmi_mpegsource_infoframe mpeg_source_infoframe;
    hi_hdmi_vendorspec_infoframe vendor_spec_infoframe;
} hi_hdmi_infoframe_unit;

typedef struct {
    hi_hdmi_infoframe_type infoframe_type; /* InfoFrame type */
    hi_hdmi_infoframe_unit infor_unit;     /* InfoFrame date */
} hi_hdmi_infoframe;

typedef struct {
    hi_hdmi_callback pfn_hdmi_event_callback; /* Event handling callback function */
    hi_void *private_data;                    /* Private data of the callback functions and parameters */
} hi_hdmi_callback_func;

typedef struct {
    hi_hdmi_cec_callback pfn_cec_callback; /* Event handling callback function */
    hi_void *private_data;                 /* Private data of the CEC callback functions and parameters */
} hi_hdmi_cec_callback_func;

typedef struct {
    hi_bool enable;
    hi_u8 physical_addr[4];                 /* CEC physic addr */
    hi_u8 logical_addr;                     /* CEC logic addr, default:0x03. */
    hi_u8 net_work[HI_CEC_LOGICALADD_BUTT]; /* CEC's net work, 1:this device can answer cec command */
} hi_hdmi_cec_status;

typedef struct {
    hi_hdmi_trace_len trace_len; /* Phy parameters selection. */
} hi_hdmi_mod_param;

typedef struct {
    hi_hdmi_force_active default_mode;
} adapt_hdmi_open_para;

typedef struct {
    hi_bool connected;     /* The Device is connected or disconnected */
    hi_bool sink_power_on; /* The sink is PowerOn or not */
    hi_bool authed;        /* HDCP Authentication */
    hi_u8   bksv[5];       /* Bksv of sink 40bits */
} hi_hdmi_status;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif


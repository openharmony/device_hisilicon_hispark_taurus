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

#include "drv_hdmi_proc.h"
#include "hdmi_osal.h"
#include "drv_hdmi_intf.h"
#include "drv_hdmi_common.h"
#include "drv_hdmi_event.h"
#include "drv_hdmi_edid.h"
#include "drv_hdmi_compatibility.h"

#define PROC_FEILD_RATE_DEVIATION 50
#define PROC_PIXL_FREQ_DEVIATION  100
#define HDMI_VIC_PC_BASE          ((HDMI_VIC_VIRTUAL_BASE) + 0x100)
#define STR_YES                   "YES"
#define STR_NO                    "NO"
#define STR_NA                    "N/A"
#define STR_ERROR                 "ERROR"
#define STR_RUN                   "RUN"
#define STR_STOP                  "STOP"
#define STR_ON                    "ON"
#define STR_OFF                   "OFF"
#define STR_LINE_COUNT            8
#define STR_LEN_SUPERADDITION     4
#define PROC_LENGTH_SIZE          21
#define PROC_ONE_LINE_STR_COUNT   4
#define PROC_TEMP_STR_LEN         25

typedef union {
    struct {
        hi_u8 scan_info            : 2; // [0..1]
        hi_u8 bar_data_present     : 2; // [2..3]
        hi_u8 act_fmt_info_present : 1; // [5]
        hi_u8 color_space          : 3; // [6..7]
    } bits;
    hi_u8 u8;
} hdmi_avi_info_byte1;

typedef union {
    struct {
        hi_u8 act_asp_ratio        : 4; // [0..3]
        hi_u8 picture_aspect_ratio : 2; // [4..5]
        hi_u8 colorimetry          : 2; // [6..7]
    } bits;
    hi_u8 u8;
} hdmi_avi_info_byte2;

typedef union {
    struct {
        hi_u8 picture_scaling  : 2; // [0..1]
        hi_u8 rgb_quantization : 2; // [2..3]
        hi_u8 ext_colorimetry  : 3; // [4..6]
        hi_u8 it_content_valid : 1; // [7]
    } bits;
    hi_u8 u8;
} hdmi_avi_info_byte3;

typedef union {
    struct {
        hi_u8 pixel_repetition       : 4; // [0..3]
        hi_u8 it_content_type        : 2; // [4..5]
        hi_u8 ycc_quantization_range : 2; // [6..7]
    } bits;
    hi_u8 u8;
} hdmi_avi_info_byte5;

typedef union {
    struct {
        hi_u8 audio_chanel_cnt  : 3; // [0..2]
        hi_u8 f13               : 1; // [3]
        hi_u8 audio_coding_type : 4; // [4..7]
    } bits;
    hi_u8 u8;
} hdmi_audio_info_byte1;

typedef union {
    struct {
        hi_u8 audio_samping_size : 2; // [0..1]
        hi_u8 audio_samping_freq : 3; // [2..4]
        hi_u8 f25to_f27          : 3; // [5..7]
    } bits;
    hi_u8 u8;
} hdmi_audio_info_byte2;

/* EIA-CEA-861-F P66 */
typedef union {
    struct {
        hi_u8 lfe_play_back     : 2; // [0..1]
        hi_u8 f52               : 1; // [2]
        hi_u8 level_shift_value : 4; // [3..6]
        hi_u8 down_mix_inhibit  : 1; // [7]
    } bits;
    hi_u8 u8;
} hdmi_audio_info_byte5;

typedef struct {
    hi_u32   vic;
    hi_u32   pixl_freq; /* khz */
    hi_u32   hor_active;
    hi_u32   ver_active;
    hi_u32   field_rate; /* 0.01hz */
    hi_char *format;
} hdmi_fmt_name_param;

static const hi_char *g_deep_color[]       = { "24", "30", "36", "48", "OFF", "UNKNOWN" };
static const hi_char *g_scan_info_str[]    = { "NONE", "OVER_SCAN", "UNDERS_SCAN", "Reserved" };
static const hi_char *g_dither_mode_str[]  = { "12_10", "12_8", "10_8", "disable" };
static const hi_char *g_lfe_playback_str[] = { "UNKNOWN", "0 dB", "+10 dB", "Reserved" };
static const hi_char *g_audio_chn_str[]    = { "STR_HEADER", "2_CH", "3_CH", "4_CH", "5_CH", "6_CH", "7_CH", "8_CH" };
static const hi_char *g_run_state_str[]    = { "INIT", "OPEN", "START", "STOP", "CLOSE", "DEINIT" };
static const hi_char *g_tmds_mode_str[]    = { "NONE", "DVI", "HDMI1.4", "HDMI2.0", "AUTO", "HDMI2.1", "UNKNOWN" };
static const hi_char *g_color_space_str[]  = { "RGB", "YCbCr422", "YCbCr444", "YCbCr420", "UNKNOWN" };
static const hi_char *g_sound_intf_str[]   = { "I2S", "SPDIF", "HBRA", "UNKNOWN" };
static const hi_char *g_colorimetry_str[]  = { "No Data", "SMPTE 170M", "ITU-R BT.709", "Extended" };
static const hi_char *g_bit_depth_str[]    = { "08 Bit", "10 Bit", "12 Bit", "16 Bit", "08 Bit", "UNKNOWN" };
static const hi_char *g_deep_color_str[]   = { "24 Bit", "30 Bit", "36 Bit", "48 Bit", "24 Bit(OFF)", "UNKNOWN" };

static const hi_char *g_default_action_str[]     = { "NONE", "HDMI", "DVI", "UNKNOWN" };
static const hi_char *g_transition_sate_str[]    = { "BOOT->MCE", "MCE->APP", "BOOT->APP" };
static const hi_char *g_picture_scaling_str[]    = { "UNKNOWN", "HpVn", "HnVp ", "HpVp" };
static const hi_char *g_rgb_quantization_str[]   = { "DEFAULT", "LIMITED", "FULL", "UNKNOWN" };
static const hi_char *g_pic_aspect_ratio_str[]   = { "NONE", "4:3", "16:9", "64:27", "256:135", "FUTURE", "UNKNOWN" };
static const hi_char *g_bar_data_present_str[]   = { "NONE", "HnVp", "HpVn", "HpVp" };
static const hi_char *g_it_content_type_str[]    = { "GRAPHICS", "PHOTO", "CINEMA", "GAME" };
static const hi_char *g_ycc_quantization_str[]   = { "LIMITED", "FULL", "UNKNOWN", "UNKNOWN" };
static const hi_char *g_audio_samping_size_str[] = { "STR_HEADER", "16 bit", "20 bit", "24 bit" };

static const hi_char *g_act_aspect_ratio_str[] = {
    "Reserved",
    "Reserved",
    "16:9_TOP",
    "14:9_TOP",
    "16:9_CENTER",
    "Reserved",
    "Reserved",
    "Reserved",
    "PICTURE",
    "4:3",
    "16:9",
    "14:9",
    "Reserved",
    "4:3_SP_14_9",
    "16:9_SP_14_9",
    "16:9_SP_4_3",
    "UNKNOWN"
};

static const hi_char *g_pixel_repetition_str[] = {
    "No Repetition",
    "2 Times",
    "3 Times",
    "4 Times",
    "5 Times",
    "6 Times",
    "7 Times",
    "8 Times",
    "9 Times",
    "10 Times",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

static const hi_char *g_audio_code_type_str[] = {
    "STREAM",
    "L-PCM",
    "AC3",
    "MPEG1",
    "MP3",
    "MPEG2",
    "AAC_LC",
    "DTS",
    "ATRAC",
    "OneBitAudio",
    "EAC3",
    "DTS-HD",
    "MAT",
    "DST",
    "WMA_PRO",
    "Reserved",
    "UNKNOWN"
};

static const hi_char *g_audio_samping_freq_str[] = {
    "STR_HEADER",
    "32 KHz",
    "44.1 kHz",
    "48 KHz",
    "88.2 KHz",
    "96 KHz",
    "176.4 KHz",
    "192 KHz"
};

static const hi_char *g_ext_colorimetry_str[] = {
    "XV_YCC601",
    "XV_YCC709",
    "S_YCC601",
    "ADOBE_YCC601",
    "ADOBE_RGB",
    "BT2020_cYCC",
    "BT2020_RGB/YCC",
    "ADDITIONAL",
    "UNKNOWN"
};

static const hi_char *g_str_speaker_str[] = {
    "FL_FR",
    "LFE",
    "FC",
    "RL_RR",
    "RC",
    "FLC_FRC",
    "RLC_RRC",
    "FLW_FRW",
    "FLH_FRH",
    "TC",
    "FCH",
    "UNKNOWN"
};

static const hi_char *g_stere_mode_str[] = {
    "FRAME_PACK",  /* 3d type:Frame Packing */
    "FIELD_ALTER", /* 3d type:Field alternative */
    "LINE_ALTERN", /* 3d type:Line alternative */
    "SBS_FULL",    /* 3d type:Side by side full */
    "L_DEPTH",     /* 3d type:L+depth */
    "L_DEPTH_GGD", /* 3d type:L+depth+Graphics+Graphics-depth */
    "TAndB",       /* 3d type:Top and Bottom */
    "Reserved",
    "SByS_HALF",   /* HDMI_EDID_3D_SIDE_BY_SIDE_HALF */
    "NONE"         /* HDMI_3D_BUTT */
};

static const hi_char *g_estab_timing_str[] = {
    "V800X600_60",
    "V800X600_56",
    "V640X480_75",
    "V640X480_72",
    "V640X480_67",
    "V640X480_60",
    "V720X400_88",
    "V720X400_70",
    "V1280X1024_75",
    "V1024X768_75",
    "V1024X768_70",
    "V1024X768_60",
    "V1024X768_87",
    "V832X624_75",
    "V800X600_75",
    "V800X600_72",
    "V1152X870_75",
    "UNKNOWN"
};

static const hi_char *g_disp_fmt_str[] = {
    "1080P@60",
    "1080P@50",
    "1080P@30",
    "1080P@25",
    "1080P@24",
    "1080I@60",
    "1080I@50",
    "720P@60",
    "720P@50",
    "576P@50",
    "480P@60",
    "PAL",
    "PAL_N",
    "PAL_NC",
    "NTSC",
    "NTSC_J",
    "PAL_M",
    "SECAM_SIN",
    "SECAM_COS",
    "640X480@60",
    "800X600@60",
    "1024X768@60",
    "1280X720@60",
    "1280X800@60",
    "1280X1024@60",
    "1366X768@60",
    "1440X900@60",
    "1440X900@60_RB",
    "1600X900@60_RB",
    "1600X1200@60",
    "1680X1050@60",
    "1920X1080@60",
    "1920X1200@60",
    "2048X1152@60",
    "2560x1440@30",
    "2560x1440@60",
    "2560x1600@60",
    "1920x2160@30",
    "3840X2160P@24",
    "3840X2160P@25",
    "3840X2160P@30",
    "3840X2160P@50",
    "3840X2160P@60",
    "4096X2160P@24",
    "4096X2160P@25",
    "4096X2160P@30",
    "4096X2160P@50",
    "4096X2160P@60",
    "3840X2160P@120",
    "4096X2160P@120",
    "7680X4320P@30",
    "CUSTOMER_DEFINE",
    "FMT_UNKNOWN"
};

static const hi_char *g_audio_channel_str[] = {
    "Reserved",
    "Reserved",
    "2_CH",
    "3_CH",
    "4_CH",
    "5_CH",
    "6_CH",
    "7_CH",
    "8_CH",
    "UNKNOWN"
};

#ifdef HDMI_HDR_SUPPORT
static const hi_char *g_hdr_user_mode_str[] = { "SDR", "HDR10", "HLG", "DOLBY", "UNKNOWN" };
static const hi_char *g_hdr_debug_mode_str[] = { "DISABLE", "OE", "AVMUTE", "UNKNOWN" };
static const hi_char *g_hdr_eotf_type_str[] = { "SDR_LUMIN", "HDR_LUMIN", "SMPTE_ST_2084", "HLG", "FUTURE", "UNKNOWN" };

static const hi_char *g_hdr_mode_str[] = {
    "DISABLE",
    "DOLBY_NORMAL",
    "DOLBY_TUNNELING",
    "HDR",
    "HDR_AUTHEN",
    "UNKNOWN"
};

static const hi_char *g_hdr_colorimetry_str[] = {
    "NONE",
    "ITU_601",
    "ITU_709",
    "EXTENDED",
    "XV_YCC_601",
    "XV_YCC_709",
    "S_YCC_601",
    "ADOBE_YCC_601",
    "ADOBE_RGB",
    "2020_const_luminous",
    "2020_nconst_luminous"
};
#endif

#ifdef HDMI_FRL_SUPPORT
static const hi_char *g_train_status_str[]  = { "NONE", "FAIL", "SUCCESS", "BUSY", "BUTT" };
static const hi_char *g_rate_sel_str[]      = { "LITTLE", "BIG", "BUTT" };
static const hi_char *g_frl_sratepy_str[]   = { ">600M", ">340M", "N-INTR", "BUTT" };
static const hi_char *g_work_mode_str[]     = { "NONE", "3L3G", "3L6G", "4L6G", "4L8G", "4L10G", "4L12G", "BUTT" };
static const hi_char *g_frl_mach_mode_str[] = { "Step", "Timeout", "BUTT" };
static const hi_char *g_frl_rate_str[]      = { "NONE", "3L3G", "3L6G", "4L6G", "4L8G", "4L10G", "4L12G", "BUTT" };
static const hi_char *g_frl_sw_train_mode_str[] = { "Delay", "Timer", "BUTT" };

static const hi_char *g_train_mach_str[] = {
    "ReadyCheck",
    "TrainStart",
    "ResultChk",
    "RateChange",
    "ResultHan",
    "RetrainChk",
    "Stop",
    "BUTT"
};
#endif

static const hdmi_fmt_name_param g_fmt_param[] = {
    /* VIC pix_ffeq HACT VACT fld_rate    fmt */
    {   1,   25175,  640,  480, 6000, "640X480_60 4:3"     },
    {   2,   27000,  720,  480, 6000, "480P_60 4:3"        },
    {   3,   27000,  720,  480, 6000, "480P_60 16:9"       },
    {   4,   74170, 1280,  720, 6000, "720P_60 16:9"       },
    {   5,   74170, 1920, 1080, 6000, "1080i_60 16:9"      },
    {   6,   27000,  720,  480, 6000, "NTSC 4:3"           },
    {   7,   27000,  720,  480, 6000, "NTSC 16:9"          },
    {   8,   27000, 1440,  240, 6000, "240P_60 4:3"        },
    {   9,   27000, 1440,  240, 6000, "240P_60 16:9"       },
    {  10,   54000,  720,  480, 6000, "2880_x480i_60 4:3"  },
    {  11,   54000,  720,  480, 6000, "2880_x480i_60 16:9" },
    {  12,   54000, 2880,  240, 6000, "2880X240P_60 4:3"   },
    {  13,   54000, 2880,  240, 6000, "2880X240P_60 16:9"  },
    {  14,   54000, 1440,  480, 6000, "1440X480P_60 4:3"   },
    {  15,   54000, 1440,  480, 6000, "1440X480P_60 16:9"  },
    {  16,  148500, 1920, 1080, 6000, "1080P_60 16:9"      },
    {  17,   27000,  720,  576, 5000, "576P_50 4:3"        },
    {  18,   27000,  720,  576, 5000, "576P_50 16:9"       },
    {  19,   74250, 1280,  720, 5000, "720P_50 16:9"       },
    {  20,   74250, 1920, 1080, 5000, "1080i_50 16:9"      },
    {  21,   27000, 1440,  576, 5000, "PAL 4:3"            },
    {  22,   27000, 1440,  576, 5000, "PAL 16:9"           },
    {  31,  148500, 1920, 1080, 5000, "1080P_50 16:9"      },
    {  32,   74250, 1920, 1080, 2400, "1080P_24 16:9"      },
    {  33,   74250, 1920, 1080, 2500, "1080P_25 16:9"      },
    {  34,   74250, 1920, 1080, 3000, "1080P_30 16:9"      },
    {  60,   59400, 1280,  720, 2400, "720P_24 16:9"       },
    {  61,   74250, 1280,  720, 2500, "720P_25 16:9"       },
    {  62,   74250, 1280,  720, 3000, "720P_30 16:9"       },
    {  93,  297000, 3840, 2160, 2400, "3840X2160P24 16:9"  },
    {  94,  297000, 3840, 2160, 2500, "3840X2160P25 16:9"  },
    {  95,  297000, 3840, 2160, 3000, "3840X2160P30 16:9"  },
    {  96,  594000, 3840, 2160, 5000, "3840X2160P50 16:9"  },
    {  97,  594000, 3840, 2160, 6000, "3840X2160P60 16:9"  },
    {  98,  297000, 4096, 2160, 2400, "4096X2160P24 256:135" },
    {  99,  297000, 4096, 2160, 2500, "4096X2160P25 256:135" },
    { 100,  297000, 4096, 2160, 3000, "4096X2160P30 256:135" },
    { 101,  594000, 4096, 2160, 5000, "4096X2160P50 256:135" },
    { 102,  594000, 4096, 2160, 6000, "4096X2160P60 256:135" },
    { 103,  297000, 3840, 2160, 2400, "3840X2160P24 64:27"   },
    { 104,  297000, 3840, 2160, 2500, "3840X2160P25 64:27"   },
    { 105,  297000, 3840, 2160, 3000, "3840X2160P30 64:27"   },
    { 106,  594000, 3840, 2160, 5000, "3840X2160P50 64:27"   },
    { 107,  594000, 3840, 2160, 6000, "3840X2160P60 64:27"   },
    { 194, 1188000, 7680, 4320, 2400, "7680X4320P24 16:9"    },
    { 195, 1188000, 7680, 4320, 2500, "7680X4320P25 16:9"    },
    { 196, 1188000, 7680, 4320, 3000, "7680X4320P30 16:9"    },
    { 198, 2376000, 7680, 4320, 5000, "7680X4320P50 16:9"    },
    { 199, 2376000, 7680, 4320, 6000, "7680X4320P60 16:9"    },
#ifdef DVI_SUPPORT
    { HDMI_VIC_PC_BASE +  9,  40000,  800,  600, 6000, "800X600_60"   },
    { HDMI_VIC_PC_BASE + 13,  65000, 1024,  768, 6000, "1024X768_60"  },
    { HDMI_VIC_PC_BASE + 18, 162000, 1600, 1200, 6000, "1600X1200_60" },
    { HDMI_VIC_PC_BASE + 25, 108000, 1280, 1024, 6000, "1280X1024_60" },
    { HDMI_VIC_PC_BASE + 28,  85500, 1360,  768, 6000, "1366X768_60"  },
    { HDMI_VIC_PC_BASE + 41, 154000, 1920, 1200, 6000, "1920X1200_60" },
#endif
    /* for 2k & 4k ,we use VIRTUAL VIC */
    { HDMI_EDID_VIRTUAL_VIC_3840X2160_30, 297000, 3840, 2160, 3000, "3840X2160P_30" },
    { HDMI_EDID_VIRTUAL_VIC_3840X2160_25, 297000, 3840, 2160, 2500, "3840X2160P_25" },
    { HDMI_EDID_VIRTUAL_VIC_3840X2160_24, 297000, 3840, 2160, 2400, "3840X2160P_24" },
    { HDMI_EDID_VIRTUAL_VIC_4096X2160_24, 297000, 4096, 2160, 2400, "4096X2160P_24" }
};

static const hi_char *g_sz_version = "HDMI version:   " MAKE_VERSION;

static hi_char *proc_vicformat(hi_u32 vic)
{
    hi_u32 i;
    hdmi_fmt_name_param *fmt_param = HI_NULL;

    for (i = 0; i < hdmi_array_size(g_fmt_param); i++) {
        fmt_param = (hdmi_fmt_name_param *)&g_fmt_param[i];
        if ((fmt_param != HI_NULL) && (fmt_param->vic == vic)) {
            return fmt_param->format;
        }
    }

    return HI_NULL;
}

static hi_char *proc_std_timingformat(const hdmi_edid_std_timing *std_timing)
{
    hi_u32 i;
    hdmi_fmt_name_param *fmt_param = HI_NULL;

    for (i = 0; i < hdmi_array_size(g_fmt_param); i++) {
        fmt_param = (hdmi_fmt_name_param *)&g_fmt_param[i];
        if ((fmt_param != HI_NULL) &&
            (fmt_param->hor_active == std_timing->hor_active) &&
            (fmt_param->ver_active == std_timing->ver_active) &&
            (fmt_param->field_rate >= std_timing->refresh_rate) &&
            ((fmt_param->field_rate - PROC_FEILD_RATE_DEVIATION) <= std_timing->refresh_rate)) {
            return fmt_param->format;
        }
    }

    return HI_NULL;
}

static hi_void proc_sw_version(hdmi_proc_entry *file)
{
    osal_seq_printf(file, "%s\n", g_sz_version);
    return;
}

static hi_void proc_app_attr_info(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u32 index;
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_app_attr *app_attr = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);

    app_attr = &(hdmi_dev->attr.app_attr);
    osal_seq_printf(file, "-------------------------- APPAttr -----------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-15s: %-20s  ", "HDMIEnable", app_attr->enable_hdmi ? STR_YES : STR_NO);
    index = min((hi_s32)app_attr->hdmi_action, HDMI_DEFAULT_ACTION_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "DefaultAction", g_default_action_str[index]);

    /* line 2 */
    osal_seq_printf(file, "%-15s: %-20s  ", "VideoEnable", app_attr->enable_video ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "AudioInfoEnable", app_attr->enable_audio ? STR_YES : STR_NO);

    /* line 3 */
    osal_seq_printf(file, "%-15s: %-20s  ", "avi_info_enable", app_attr->enable_avi_infoframe ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "audio_info_enable", app_attr->enable_aud_infoframe ? STR_YES : STR_NO);

    /* line 4 */
    osal_seq_printf(file, "%-15s: %-20s  ", "xvYCCMode", app_attr->xvycc_mode ? STR_YES : STR_NO);
    osal_seq_printf(file, "\n");
    /* line 5 */
    index = min((hi_s32)app_attr->deep_color_mode, HDMI_DEEP_COLOR_48BIT + 1);
    osal_seq_printf(file, "%-15s: %-20s  ", "DeepColorMode", g_deep_color[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "SpdInfoEnable", app_attr->enable_spd_infoframe ? STR_YES : STR_NO);

    /* line 6 */
    index = min((hi_s32)app_attr->out_color_space, HDMI_COLORSPACE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "OutColorSpace", g_color_space_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "MpegInfoEnable", app_attr->enable_mpeg_infoframe ? STR_YES : STR_NO);

    /* line 7 */
    osal_seq_printf(file, "%-15s: %-20s  ", "ColorSpaceAdapt", app_attr->enable_clr_space_adapt ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "DeepColorAdapt", app_attr->enable_deep_clr_adapt ? STR_YES : STR_NO);

    /* line 8 */
    osal_seq_printf(file, "%-15s: %-20s  ", "DebugEnable", app_attr->debug_flag ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "CtsAuthEnable", app_attr->auth_mode ? STR_YES : STR_NO);
#ifdef HDMI_HDR_SUPPORT
    osal_seq_printf(file, "%-15s: %-20s  ", "DrmInfoEnable", app_attr->enable_drm_infoframe ? STR_YES : STR_NO);
#endif
    osal_seq_printf(file, "\n");

    return;
}

static hi_void proc_run_state(hdmi_proc_entry *file, hi_u32 state)
{
    hi_u32 i;

    if (state > 0) {
        for (i = 0; i < hdmi_array_size(g_run_state_str); i++) {
            if (state & (1 << i)) {
                osal_seq_printf(file, " %s", g_run_state_str[i]);
            }
        }
    } else {
        osal_seq_printf(file, " %s", "NONE");
    }
}

static hi_void proc_transition_state(hdmi_proc_entry *file, hi_u32 state)
{
    hi_u32 i;

    if (state > 0) {
        for (i = 0; i < hdmi_array_size(g_transition_sate_str); i++) {
            if (state & (1 << i)) {
                osal_seq_printf(file, " %s", g_transition_sate_str[i]);
            }
        }
    } else {
        osal_seq_printf(file, " %s", "NONE");
    }
}

static hi_void proc_sw_status(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u32 index;
    hdmi_device *hdmi_dev = HI_NULL;
#ifdef HDMI_FRL_SUPPORT
    hdmi_frl_info *frl = HI_NULL;
#endif
    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
#ifdef HDMI_FRL_SUPPORT
    frl = &hdmi_dev->frl_info;
#endif

    osal_seq_printf(file, "-------------------------- SWStatus ----------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-15s: %-20s  ", "ThreadRun",
        (hdmi_dev->thread_info.thread_timer_sate == HDMI_THREAD_STATE_RUN) ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s:", "RunStatus");
    proc_run_state(file, hdmi_dev->run_state);
    osal_seq_printf(file, "\n");

    /* line 2 */
#ifdef HDMI_FRL_SUPPORT
    index = (frl->mode == HDMI_FRL_MODE_FRL) ? HDMI_TMDS_MODE_HDMI_2_1 : hdmi_dev->tmds_mode;
    index = min((hi_s32)index, HDMI_TMDS_MODE_BUTT);
#else
    index = min((hi_s32)hdmi_dev->tmds_mode, HDMI_TMDS_MODE_BUTT);
#endif
    osal_seq_printf(file, "%-15s: %-20s  \n", "TMDSMode", g_tmds_mode_str[index]);
    /* line 3 */
    osal_seq_printf(file, "%-15s: %-20d  ", "KernelCnt", hdmi_dev->kernel_cnt);
    osal_seq_printf(file, "%-15s: %-20d \n", "UserCnt", hdmi_dev->user_cnt);
    /* line 4 */
    osal_seq_printf(file, "%-15s: %-20s  ", "KCallBack", (hdmi_dev->k_callback != HI_NULL) ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20d \n", "UCallBackCnt", hdmi_dev->user_callback_cnt);
    /* line 5 */
    osal_seq_printf(file, "%-15s:", "TransitState");
    proc_transition_state(file, hdmi_dev->transition_state);
    osal_seq_printf(file, "\n");
    /* line 6 */
    osal_seq_printf(file, "%-15s: %-20s  ", "EmiEnable", hdmi_dev->emi_enable ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20d \n", "PcbLen", hdmi_dev->mode_param.trace_len);

    return;
}

#if defined HDMI_HDR_SUPPORT
static hi_void proc_hdr_status(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u32 index;
    hdmi_device *hdmi_dev = HI_NULL;
    drv_hdmi_hdr_info *hdr_info = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    hdr_info = &(hdmi_dev->hdr_info);

    /* line 1 */
    index = min((hi_s32)hdr_info->hdr_debug_mode, HDMI_HDR_DEBUG_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "HdrDebugMode", g_hdr_debug_mode_str[index]);
    osal_seq_printf(file, "%-15s: %-20d \n", "ZeroDrmSendTime", hdr_info->zero_drm_if_timer.time);
    /* line 2 */
    osal_seq_printf(file, "%-15s: %-20d  ", "HdrModeChnTime", hdr_info->hdr_mode_chn_timer.time);
    osal_seq_printf(file, "\n");

    return;
}
#endif

#ifdef HDMI_FRL_SUPPORT
static hi_void proc_frl_status(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u32                    index;
    hdmi_device              *hdmi_dev    = HI_NULL;
    hdmi_frl_info            *frl         = HI_NULL;
    compatibility_info       *compat_info = HI_NULL;
    hdmi_frl_state_mach_info *mach_info   = HI_NULL;

    compat_info = compat_info_get(hdmi_id);
    hdmi_if_null_warn_return_void(compat_info);
    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    frl = &hdmi_dev->frl_info;
    mach_info = &frl->state_mach_info;

    /* line 1 */
    osal_seq_printf(file, "%-15s: %-20s  ", "MachRun", mach_info->start_mach ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "FRLEnable", frl->start ? STR_YES : STR_NO);
    /* line 2 */
    index = min((hi_s32)frl->state_mach_info.mach_mode, HDMI_FRL_MACH_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "MachMode", g_frl_mach_mode_str[index]);
    index = min((hi_s32)frl->state_mach_info.sw_train_mode, FRL_SW_TRAIN_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "SwTrainMode", g_frl_sw_train_mode_str[index]);
    /* line 3 */
    index = min((hi_s32)frl->rate_select, HDMI_FRL_RATE_SELECT_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "RateSelect", g_rate_sel_str[index]);
    index = min((hi_s32)frl->strategy_mode, HDMI_FRL_STRATEGY_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "FrlStrategy", g_frl_sratepy_str[index]);
    /* line 4 */
    osal_seq_printf(file, "%-15s: %-20d  ", "MaxFailTime", frl->max_fail_times);
    osal_seq_printf(file, "%-15s: %-20d \n", "WaitHandTime", mach_info->wait_handle_ms);
    /* line 5 */
    osal_seq_printf(file, "%-15s: %-20d  ", "WaitReadyTime", mach_info->wait_ready_ms);
    osal_seq_printf(file, "%-15s: %-20d \n", "WaitRetrainTime", mach_info->wait_retrain_ms);
    /* line 6 */
    index = min((hi_s32)frl->rate_info.cur_rate, FRL_WORK_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "CurFrlRate", g_frl_rate_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "DscEnable", frl->dsc_enable ? STR_YES : STR_NO);
    /* line 7 */
    osal_seq_printf(file, "%-15s: %-20s  ", "FrlCtsEnable", frl->cts_mode ? STR_YES : STR_NO);
    index = min((hi_s32)mach_info->train_status, HDMI_FRL_TRAIN_STEP_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "MachStatus", g_train_mach_str[index]);
    /* line 8 */
    index = min((hi_s32)frl->train_status.frl_train_status, HDMI_FRL_TRAIN_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "TrainStatus", g_train_status_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "AviSend", (compat_info->avi_use_gen5 == HI_FALSE) ? "DEFAULT" : "GEN5");
}
#endif

static hi_void proc_detail_timing(hdmi_proc_entry *file, const hdmi_video_status *video_status)
{
#if defined(HDMI_SUPPORT_LOGIC_HISIV100)
    osal_seq_printf(file, "-------------------------- Detect Timming -----------------------\n");
    osal_seq_printf(file, "%-15s: %-20s  ", "SyncSwEnable", video_status->sync_sw_enable ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "HsyncPolarity", video_status->hsync_polarity ? "P" : "N");
    osal_seq_printf(file, "%-15s: %-20s  ", "Progressive", video_status->progressive ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "VsyncPolarity", video_status->vsync_polarity ? "P" : "N");
    osal_seq_printf(file, "%-15s: %-20u  ", "HsyncTotal", video_status->hsync_total);
    osal_seq_printf(file, "%-15s: %-20u \n", "HactiveCnt", video_status->hactive_cnt);
    osal_seq_printf(file, "%-15s: %-20u  ", "VsyncTotal", video_status->vsync_total);
    osal_seq_printf(file, "%-15s: %-20u \n", "VactiveCnt", video_status->vactive_cnt);
#endif

    return;
}

static hi_void proc_hw_status(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u32             index;
    hdmi_device        *hdmi_dev = HI_NULL;
    hdmi_common_status *com_status = HI_NULL;
    hdmi_phy_status    *curr_phy_status = HI_NULL;
    hdmi_emi_status    emi_status = {0};
    hdmi_hardware_status hw_status = {0};

#ifdef HDMI_FRL_SUPPORT
    hdmi_frl_status *frl_status = HI_NULL;
#endif
    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);

    hal_call_void(hal_hdmi_hardware_status_get, hdmi_dev->hal, &hw_status);
    com_status = &(hw_status.common_status);
    curr_phy_status = &(hw_status.phy_status);

    osal_seq_printf(file, "-------------------------- HWStatus ----------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-15s: %-20s  ", "HotPlug", com_status->hotplug ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "Rsen", com_status->rsen ? STR_YES : STR_NO);
    /* line 2 */
    osal_seq_printf(file, "%-15s: %-20s  ", "PhyOutputEnable", curr_phy_status->phy_oe ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "PhyPowerEnable", curr_phy_status->phy_power_on ? STR_YES : STR_NO);
    /* line 3 */
#ifdef HDMI_FRL_SUPPORT
    frl_status = &hw_status.frl_status;
    osal_seq_printf(file, "%-15s: %-20s  ", "WorkEn", frl_status->work_en ? STR_YES : STR_NO);
    index = min((hi_s32)frl_status->work_mode, FRL_WORK_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "WorkMode", g_work_mode_str[index]);
    /* tmds_mode is HDMI2.1 when phy work at FRL mode. */
    if (frl_status->frl_start == HI_TRUE) {
        com_status->tmds_mode = HDMI_TMDS_MODE_HDMI_2_1;
    }
#endif
    index = min((hi_s32)com_status->tmds_mode, HDMI_TMDS_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "TMDSMode", g_tmds_mode_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "AvMute", com_status->avmute ? STR_YES : STR_NO);
    proc_detail_timing(file, &hw_status.video_status);
    hal_call_void(hal_hdmi_emi_status_get, hdmi_dev->hal, &emi_status);
    osal_seq_printf(file, "%-15s: %-20s \n", "EmiEnable", emi_status.hw_emi_enable ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s  ", "EmiDebugEnable", emi_status.debug_enable ? STR_YES : STR_NO);
    osal_seq_printf(file, "\n");

    return;
}

static hi_void proc_if_raw_data(hdmi_proc_entry *file, hi_u32 *index, hi_u32 *line, hi_u8 *data)
{
    hi_u32 len = data[AVI_OFFSET_LENGTH] + STR_LEN_SUPERADDITION;

    for (; ((*index) < len) && ((*index) < HDMI_INFO_FRAME_MAX_SIZE); (*index)++) {
        if ((*index) >= *line) {
            *line += STR_LINE_COUNT;
            break;
        } else if ((*index) % 4 == 0) { /* 4, " " is printed every 4 index */
            osal_seq_printf(file, " ");
        }
        osal_seq_printf(file, "%02x ", data[(*index)]);
    }
    osal_seq_printf(file, "\n");

    return;
}

static hi_void proc_vic_timing(hdmi_proc_entry *file, const hdmi_vo_attr *vo_attr,
    const hdmi_infoframe_status *info_status)
{
    hi_u32 i, index;
    hi_u32 vic, hdmi_vic;
    enum hdmi_video_format format;
    hi_char *fmt_str = HI_NULL;
    hi_char *vsif_fmt_str = HI_NULL;
    hdmi_video_def *video = HI_NULL;
    hdmi_video_4k_def *video_4k = HI_NULL;

    /* line 1 */
    index = min((hi_s32)vo_attr->video_timing, HDMI_VIDEO_TIMING_BUTT);
    vic = drv_hdmi_vic_search(index, vo_attr->picture_aspect, HI_FALSE);  // search vic
    i = drv_hdmi_vic_to_index(vic);
    video =  drv_hdmi_comm_format_param_get(vic);
    if (video != HI_NULL && i < CEA_VIDEO_CODE_MAX) {
        osal_seq_printf(file, "%-15s: %-20s |", "VideoTiming", (video->fmt_str != HI_NULL) ? video->fmt_str : "NONE");
    } else {
        osal_seq_printf(file, "%-15s: %-20s |", "VideoTiming", "NONE");
    }
    osal_seq_printf(file, "%-15s: %-20s \n", "AVIInfoEnable", info_status->avi_enable ? STR_YES : STR_NO);
    /* line 2 */
    index = min((hi_s32)vo_attr->disp_fmt, (hi_s32)(hdmi_array_size(g_disp_fmt_str) - 1));
    osal_seq_printf(file, "%-15s: %-20s |", "DispFmt", g_disp_fmt_str[index]);

    /*
     * to modify the proc info when the video format is 4k x 2k
     * find vic according to info_frame
     * 4k x 2k
     */
    format = (info_status->vsif[7] >> 5) & 0x3; /* 5'b, BIT[3:1] */
    if (format == HDMI_VIDEO_FORMAT_4K) {
        hdmi_vic = info_status->vsif[8];  // HDMI_VIC
        video_4k = drv_hdmi_video_codes_4k_get(hdmi_vic);
        if (video_4k != HI_NULL && hdmi_vic >= 1 && hdmi_vic <= CEA861_F_VIDEO_CODES_MAX_4K) {
            vsif_fmt_str = video_4k->fmt_str;
        }
    } else {
        i = drv_hdmi_vic_to_index(info_status->avi[7]);
        if (video != HI_NULL && i < CEA_VIDEO_CODE_MAX) {
            fmt_str = video->fmt_str;
        } else {
            fmt_str = 0;
        }
    }
    osal_seq_printf(file, "%-15s: %s(VIC=%2d)\n", "CurrentFormat",
        (fmt_str == HI_NULL) ? "" : fmt_str, info_status->avi[7]);

    /* line 3 */
    osal_seq_printf(file, "%-15s: %-20d |", "PixelClk", vo_attr->clk_fs);
    osal_seq_printf(file, "%-15s: %s(HDMI_VIC=%2d)\n", "VSIFormat", (vsif_fmt_str == HI_NULL) ? "" : vsif_fmt_str,
        (format == HDMI_VIDEO_FORMAT_4K) ? info_status->vsif[8] : 0);

    return;
}

static hi_void proc_video_attr(hdmi_proc_entry *file, const hdmi_vo_attr *vo_attr,
    const hdmi_infoframe_status *info_status)
{
    hi_u32 index;
    hdmi_avi_info_byte1 byte1;
    hdmi_avi_info_byte2 byte2;
    hdmi_avi_info_byte3 byte3;
    hdmi_avi_info_byte5 byte5;

    byte1.u8 = info_status->avi[4];
    byte2.u8 = info_status->avi[5];
    byte3.u8 = info_status->avi[6];
    byte5.u8 = info_status->avi[8];

    index = min((hi_s32)vo_attr->in_bit_depth, HDMI_VIDEO_BITDEPTH_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "InBitDepth", g_bit_depth_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "BarDataPresent",
        g_bar_data_present_str[byte1.bits.bar_data_present]);
    /* line 5 */
    index = min((hi_s32)vo_attr->in_color_space, HDMI_COLORSPACE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "InColorSpace", g_color_space_str[index]);
    index = byte1.bits.color_space & 0x3;
    osal_seq_printf(file, "%-15s: %-20s \n", "ColorSpace", g_color_space_str[index]);
    /* line 6 */
    index = min((hi_s32)vo_attr->colorimetry, HDMI_COLORIMETRY_EXTEND);
    osal_seq_printf(file, "%-15s: %-20s |", "Colorimetry", g_colorimetry_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "Colorimetry", g_colorimetry_str[byte2.bits.colorimetry]);
    /* line 7 */
    index = min((hi_s32)vo_attr->picture_aspect, HDMI_PICTURE_ASPECT_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "PicAspectRatio", g_pic_aspect_ratio_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "PicAspectRatio",
        g_pic_aspect_ratio_str[byte2.bits.picture_aspect_ratio]);
    /* line 8 */
    index = min((hi_s32)vo_attr->active_aspect, HDMI_ACTIVE_ASPECT_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "ActAspectRatio", g_act_aspect_ratio_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "ActAspectRatio", g_act_aspect_ratio_str[byte2.bits.act_asp_ratio]);
    /* line 9 */
    osal_seq_printf(file, "%-15s: %-20d |", "PixelRepeat", vo_attr->pixel_repeat);
    osal_seq_printf(file, "%-15s: %-20s \n", "PixelRepeat", g_pixel_repetition_str[byte5.bits.pixel_repetition]);
    /* line 10 */
    index = min((hi_s32)vo_attr->ycc_quantization, HDMI_YCC_QUANTIZATION_RANGE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "YCCQuantization", g_ycc_quantization_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "YCCQuantization",
        g_ycc_quantization_str[byte5.bits.ycc_quantization_range]);
    /* line 11 */
    index = min((hi_s32)vo_attr->rgb_quantization, HDMI_QUANTIZATION_RANGE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "RGBQuantization", g_rgb_quantization_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "RGBQuantization", g_rgb_quantization_str[byte3.bits.rgb_quantization]);
    /* line 12 */
    index = min((hi_s32)vo_attr->extended_colorimetry, HDMI_EXTENDED_COLORIMETRY_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "ExtColorimetry", g_ext_colorimetry_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "ExtColorimetry", g_ext_colorimetry_str[byte3.bits.ext_colorimetry]);
    /* line 13 */
    index = min((hi_s32)vo_attr->stereo_mode, HDMI_3D_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "StereoMode", g_stere_mode_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "ItContentValid", byte3.bits.it_content_valid ? STR_YES : STR_NO);
    /* line 14 */
    osal_seq_printf(file, "%-15s: %-20d |", "bVSyncPol", vo_attr->v_sync_pol);
    osal_seq_printf(file, "%-15s: %-20d \n", "bHSyncPol", vo_attr->h_sync_pol);

    return;
}

static hi_void proc_video_path(hdmi_proc_entry *file, hdmi_hardware_status *hw_status)
{
    hi_u32 i, index, line;
    hdmi_avi_info_byte1 un_avi_byte1;
    hdmi_avi_info_byte3 un_avi_byte3;
    hdmi_avi_info_byte5 un_avi_byte5;
    hi_u8 *data = HI_NULL;

    un_avi_byte1.u8 = hw_status->info_frame_status.avi[4];
    un_avi_byte3.u8 = hw_status->info_frame_status.avi[6];
    un_avi_byte5.u8 = hw_status->info_frame_status.avi[8];

    osal_seq_printf(file, "%-15s: %-20s \n", "ITContentType", g_it_content_type_str[un_avi_byte5.bits.it_content_type]);
    /* line 16 */
    osal_seq_printf(file, "%-15s: %-20s |", "VideoMute", hw_status->video_status.video_mute ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "PicScaling", g_picture_scaling_str[un_avi_byte3.bits.picture_scaling]);
    /* line 17 */
    index = min((hi_s32)hw_status->video_status.out_bit_depth, HDMI_VIDEO_BITDEPTH_BUTT);
    if (hw_status->video_status.out_color_space == HDMI_COLORSPACE_YCBCR422) {
        index = HDMI_VIDEO_BITDEPTH_12;
    }
    osal_seq_printf(file, "%-15s: %-20s |", "OutBitDepth", g_bit_depth_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "ActFmtPresent",
        un_avi_byte1.bits.act_fmt_info_present ? STR_YES : STR_NO);
    /* line 18 */
    index = min((hi_s32)hw_status->video_status.out_color_space, HDMI_COLORSPACE_BUTT);

    osal_seq_printf(file, "%-15s: %-20s |", "OutColorSpace", g_color_space_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "ScanInfo", g_scan_info_str[un_avi_byte1.bits.scan_info]);
    /* line 19 */
    osal_seq_printf(file, "%-15s: %-20s |", "YCbCr420_422", hw_status->video_status.ycbcr420_422 ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s:\n", "AVIInfoRawData");
    /* line 20 */
    osal_seq_printf(file, "%-15s: %-20s |", "YCbCr422_444", hw_status->video_status.ycbcr422_444 ? STR_YES : STR_NO);
    data = hw_status->info_frame_status.avi;
    line = STR_LINE_COUNT;
    i = 0;
    proc_if_raw_data(file, &i, &line, data);
    /* line 21 */
    osal_seq_printf(file, "%-15s: %-20s |", "YCbCr444_422", hw_status->video_status.ycbcr444_422 ? STR_YES : STR_NO);
    proc_if_raw_data(file, &i, &line, data);
    /* line 22 */
    osal_seq_printf(file, "%-15s: %-20s |", "YCbCr422_420", hw_status->video_status.ycbcr422_420 ? STR_YES : STR_NO);
    proc_if_raw_data(file, &i, &line, data);
    /* line 23 */
    osal_seq_printf(file, "%-15s: %-20s |", "RGB2YCbCr", hw_status->video_status.rgb2ycbcr ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s:\n", "VSInfoRawData");
    /* line 24 */
    osal_seq_printf(file, "%-15s: %-20s |", "YCbCr2RGB", hw_status->video_status.ycbcr2rgb ? STR_YES : STR_NO);
    data = hw_status->info_frame_status.vsif;
    line = STR_LINE_COUNT;
    i = 0;
    proc_if_raw_data(file, &i, &line, data);
    /* line 25 */
    index = min((hi_s32)hw_status->video_status.dither, HDMI_VIDEO_DITHER_DISABLE);
    osal_seq_printf(file, "%-15s: %-20s |", "Dither", g_dither_mode_str[index]);
    proc_if_raw_data(file, &i, &line, data);
    index = min((hi_s32)hw_status->video_status.out_bit_depth, HDMI_VIDEO_BITDEPTH_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |\n", "DeepColorMode", g_deep_color_str[index]);

    return;
}

static hi_void proc_video_info(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hdmi_vo_attr               *vo_attr  = HI_NULL;
    hdmi_device                *hdmi_dev = HI_NULL;
    static hdmi_hardware_status hw_status;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    vo_attr = &(hdmi_dev->attr.vo_attr);
    (hi_void)memset_s(&hw_status, sizeof(hdmi_hardware_status), 0, sizeof(hdmi_hardware_status));
    hal_call_void(hal_hdmi_hardware_status_get, hdmi_dev->hal, &hw_status);

    osal_seq_printf(file, "------------ VideoAttr --------------------------- AVIInfo --------------\n");
    /* line 1~3 */
    proc_vic_timing(file, vo_attr, &hw_status.info_frame_status);
    /* line 4~14 */
    proc_video_attr(file, vo_attr, &hw_status.info_frame_status);
    /* line 15~25 */
    osal_seq_printf(file, "------------ VideoPath ---------------|");
    proc_video_path(file, &hw_status);

    return;
}

#ifdef HDMI_HDR_SUPPORT
static hi_void proc_hdr_of_data(hdmi_proc_entry *file, hi_u8 *data)
{
    hi_u8 i;
    hi_u8 len = data[2] + STR_LEN_SUPERADDITION;

    for (i = 0; (i < len) && (i < HDMI_INFO_FRAME_MAX_SIZE); i++) {
        if ((i % STR_LEN_SUPERADDITION == 0) && (i != 0)) {
            osal_seq_printf(file, " ");
        }
        osal_seq_printf(file, "%02x ", data[i]);
        if (i % 15 == 0 && i != 0) { /* about the 15 length proc log */
            osal_seq_printf(file, "\n                ");
        }
    }

    return;
}

static hi_void proc_hdr_info(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u8 index;
    hi_u8                *data = HI_NULL;
    hdmi_hal             *hal  = HI_NULL;
    hdmi_device          *hdmi_dev = HI_NULL;
    drv_hdmi_hdr_attr    *hdr_attr = HI_NULL;
    hdmi_hardware_status hw_status = {0};

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    hdr_attr = &hdmi_dev->attr.hdr_attr;
    hal = hdmi_dev->hal;
    hdmi_if_null_warn_return_void(hal);

    hal->hal_hdmi_hardware_status_get(hal, &hw_status);
    osal_seq_printf(file, "--------------------------------- HDRAttr -------------------------------\n");
    /* line 1 */
    index = min((hi_s32)hdr_attr->user_hdr_mode, HDMI_HDR_USERMODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "UserHdrMode", g_hdr_user_mode_str[index]);
    /* line 2 */
    index = min((hi_s32)hdr_attr->hdr_mode, HDMI_HDR_MODE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s  ", "HdrMode", g_hdr_mode_str[index]);
    index = min((hi_s32)hdr_attr->eotf_type, HDMI_EOTF_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "HdrEotfType", g_hdr_eotf_type_str[index]);
    /* line 3 */
    osal_seq_printf(file, "%-15s: %-20d  ", "HdrMetaDataID", hdr_attr->metadata_id);
    index = min((hi_s32)hdr_attr->colorimetry, HDMI_HDR_EXTENDED_COLORIMETRY_2020_NON_CONST_LUMINOUW);
    osal_seq_printf(file, "%-15s: %-20s \n", "HdrColorimetry", g_hdr_colorimetry_str[index]);
    /* line 4 */
    osal_seq_printf(file, "%-15s: %-20d  ", "DispPrim0_X", hdr_attr->un_descriptor.type1.primaries0_x);
    osal_seq_printf(file, "%-15s: %-20d \n", "DispPrim0_Y", hdr_attr->un_descriptor.type1.primaries0_y);
    /* line 5 */
    osal_seq_printf(file, "%-15s: %-20d  ", "DispPrim1_X", hdr_attr->un_descriptor.type1.primaries1_x);
    osal_seq_printf(file, "%-15s: %-20d \n", "DispPrim1_Y", hdr_attr->un_descriptor.type1.primaries1_y);
    /* line 6 */
    osal_seq_printf(file, "%-15s: %-20d  ", "DispPrim2_X", hdr_attr->un_descriptor.type1.primaries2_x);
    osal_seq_printf(file, "%-15s: %-20d \n", "DispPrim2_Y", hdr_attr->un_descriptor.type1.primaries2_y);
    /* line 7 */
    osal_seq_printf(file, "%-15s: %-20d  ", "WhitePoint_X", hdr_attr->un_descriptor.type1.white_point_x);
    osal_seq_printf(file, "%-15s: %-20d \n", "WhitePoint_Y", hdr_attr->un_descriptor.type1.white_point_y);
    /* line 8 */
    osal_seq_printf(file, "%-15s: %-20d  ", "MaxLuminance", hdr_attr->un_descriptor.type1.max_luminance);
    osal_seq_printf(file, "%-15s: %-20d \n", "MinLuminance", hdr_attr->un_descriptor.type1.min_luminance);
    /* line 9 */
    osal_seq_printf(file, "%-15s: %-20d  ", "MaxLightLevel", hdr_attr->un_descriptor.type1.max_light_level);
    osal_seq_printf(file, "%-15s: %-20d \n", "AverLightLevel", hdr_attr->un_descriptor.type1.average_light_level);
    osal_seq_printf(file, "--------------------------------- DRMInfo -------------------------------\n");
    /* line 10 */
    osal_seq_printf(file, "%-15s: %-20s  ", "DRMInfoEnable", hw_status.info_frame_status.drm_enable ? STR_YES : STR_NO);
    index = min((hi_s32)hw_status.info_frame_status.drm[4], HDMI_EOTF_BUTT);
    osal_seq_printf(file, "%-15s: %-20s \n", "EotfType", g_hdr_eotf_type_str[index]);
    /* line 11 */
    osal_seq_printf(file, "%-15s: %-20d \n", "MetadataID", hw_status.info_frame_status.drm[5]);
    /* line 12 */
    osal_seq_printf(file, "%-15s: ", "DRMInfoRawData");
    data = hw_status.info_frame_status.drm;
    proc_hdr_of_data(file, data);
    osal_seq_printf(file, "\n");
}
#endif

static hi_void porc_auido_attr(hdmi_proc_entry *file, const hdmi_ao_attr *ao_attr,
    const hdmi_hardware_status *hw_status)
{
    hi_u32 index;
    hdmi_audio_info_byte1 un_audio_byte1;
    hdmi_audio_info_byte2 un_audio_byte2;

    un_audio_byte1.u8 = hw_status->info_frame_status.audio[4];
    un_audio_byte2.u8 = hw_status->info_frame_status.audio[5];

    /* line 1 */
    index = min((hi_s32)ao_attr->sound_intf, HDMI_AUDIO_INTF_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "SoundIntf", g_sound_intf_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "AudioInfoEnable",
        hw_status->info_frame_status.audio_enable ? STR_YES : STR_NO);
    /* line 2 */
    index = min((hi_s32)ao_attr->audio_code, HDMI_AUDIO_CODING_TYPE_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "CodeType", g_audio_code_type_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "CodeType", g_audio_code_type_str[un_audio_byte1.bits.audio_coding_type]);
    /* line 3 */
    index = min((hi_s32)ao_attr->channels, HDMI_AUDIO_FORMAT_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "ChannelCnt", g_audio_channel_str[index]);
    osal_seq_printf(file, "%-15s: %-20s \n", "ChannelCnt", g_audio_chn_str[un_audio_byte1.bits.audio_chanel_cnt]);
    /* line 4 */
    osal_seq_printf(file, "%-15s: %-20d |", "SampleFreq", ao_attr->sample_fs);
    osal_seq_printf(file, "%-15s: %-20s \n", "SampleFreq",
        g_audio_samping_freq_str[un_audio_byte2.bits.audio_samping_freq]);
    /* line 5 */
    osal_seq_printf(file, "%-15s: %-20d |", "SampleDepth", ao_attr->sample_depth);
    osal_seq_printf(file, "%-15s: %-20d \n", "SampleDepth", hw_status->audio_status.sample_depth);
    /* line 6 */
    osal_seq_printf(file, "%-15s: %-20s |", "DownSample", ao_attr->down_sample ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "SampleSize",
        g_audio_samping_size_str[un_audio_byte2.bits.audio_samping_size]);

    return;
}

static hi_void proc_audio_path(hdmi_proc_entry *file, hdmi_hardware_status *hw_status)
{
    hi_u8 i;
    hi_u32 index;
    hi_u8 *data = HI_NULL;
    hdmi_audio_info_byte5 un_audio_byte5;

    un_audio_byte5.u8 = hw_status->info_frame_status.audio[8];

    osal_seq_printf(file, "%-15s: %-20s \n", "DownMixInhibit", un_audio_byte5.bits.down_mix_inhibit ? STR_YES : STR_NO);
    /* line 8 */
    osal_seq_printf(file, "%-15s: %-20s |", "AudioEnable",
        hw_status->info_frame_status.audio_enable ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20d \n", "LevelShiftValue", un_audio_byte5.bits.level_shift_value);
    /* line 9 */
    osal_seq_printf(file, "%-15s: %-20s |", "AudioMute", hw_status->audio_status.audio_mute ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-15s: %-20s \n", "LFEPlayBack", g_lfe_playback_str[un_audio_byte5.bits.lfe_play_back]);

    /* line 10 */
    index = min((hi_s32)hw_status->audio_status.sound_intf, HDMI_AUDIO_INTF_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "SoundIntf", g_sound_intf_str[index]);
    osal_seq_printf(file, "%-15s: 0x%02x (%u)\n", "Channel/SpeakerAlloc", hw_status->info_frame_status.audio[7],
        hw_status->info_frame_status.audio[7]);
    /* line 11 */
    index = min((hi_s32)hw_status->audio_status.layout, HDMI_AUDIO_FORMAT_BUTT);
    osal_seq_printf(file, "%-15s: %-20s |", "ChannelCnt", g_audio_channel_str[index]);
    osal_seq_printf(file, "%-15s: \n", "AudioInfoRawData");
    /* line 12 */
    osal_seq_printf(file, "%-15s: %-20d |", "SampleFreq", hw_status->audio_status.sample_fs);
    data = hw_status->info_frame_status.audio;
    for (i = 0; (i < (data[2] + STR_LEN_SUPERADDITION)) && (i < HDMI_INFO_FRAME_MAX_SIZE); i++) {
        if (i >= 8) { /* the 8~31 chars length proc log none " " */
            break;
        } else if ((i % STR_LEN_SUPERADDITION) == 0) {
            osal_seq_printf(file, " ");
        }
        osal_seq_printf(file, "%02x ", data[i]);
    }
    osal_seq_printf(file, "\n");
    /* line 13 */
    osal_seq_printf(file, "%-15s: %-20d |", "SampleDepth", hw_status->audio_status.sample_depth);
    for (; (i < (data[2] + STR_LEN_SUPERADDITION)) && (i < HDMI_INFO_FRAME_MAX_SIZE); i++) {
        if (i >= 16) { /* the 16~31 chars length proc log none " " */
            break;
        } else if ((i % STR_LEN_SUPERADDITION) == 0) {
            osal_seq_printf(file, " ");
        }
        osal_seq_printf(file, "%02x ", data[i]);
    }
    osal_seq_printf(file, "\n");
    /* line 14 */
    osal_seq_printf(file, "%-15s: %-20s |\n", "DownSample", hw_status->audio_status.down_sample ? STR_YES : STR_NO);
#if defined(HDMI_SUPPORT_LOGIC_HISIV100)
    /* line 15 */
    osal_seq_printf(file, "%-15s: %-20u |\n", "Ref_CTS", hw_status->audio_status.ref_cts);
    /* line 16 */
    osal_seq_printf(file, "%-15s: %-20u |\n", "Reg_CTS", hw_status->audio_status.reg_cts);
    /* line 17 */
    osal_seq_printf(file, "%-15s: %-20u |\n", "Ref_N", hw_status->audio_status.ref_n);
    /* line 18 */
    osal_seq_printf(file, "%-15s: %-20u |\n", "Reg_N", hw_status->audio_status.reg_n);
#endif

    return;
}

static hi_void proc_audio_info(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hdmi_device                *hdmi_dev = HI_NULL;
    hdmi_ao_attr               *ao_attr  = HI_NULL;
    static hdmi_hardware_status hw_status;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    ao_attr = &(hdmi_dev->attr.ao_attr);
    hal_call_void(hal_hdmi_hardware_status_get, hdmi_dev->hal, &hw_status);

    osal_seq_printf(file, "------------ AudioAttr --------------------------- AudioIfno ------------\n");
    /* line 1~6 */
    porc_auido_attr(file, ao_attr, &hw_status);
    /* line 7~18 */
    osal_seq_printf(file, "------------ AudioPath ---------------|");
    proc_audio_path(file, &hw_status);

    return;
}

static hi_void proc_event_info(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_s32              ret;
    hi_u32              i, j;
    hi_u32              total_pool = 0;
    hdmi_device        *hdmi_dev   = HI_NULL;
    hdmi_event_run_cnt  run_cnt    = {0};
    hdmi_event_run_ctrl ctrl       = {0};

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    ret = drv_hdmi_event_pool_total_get(hdmi_id, &total_pool);
    hdmi_if_failure_warn_return_void(ret);

    if (total_pool >= HDMI_EVENT_POOL_CNT) {
        hdmi_err("u32TotalPool(%u) must less than %u.\n", total_pool, HDMI_EVENT_POOL_CNT);
        return;
    }

    for (i = 0; i < total_pool; i++) {
        if (drv_hdmi_event_pool_status_get(hdmi_id, i + 1, &ctrl, &run_cnt) != HI_SUCCESS) {
            continue;
        }
        osal_seq_printf(file,
            "----------------- TaskID=%u Event Pool[%u] Status ----------------------\n", ctrl.pool_id, i);
        /* line1 */
        osal_seq_printf(file, "CNT|%-8s|%-3s|%-5s|%-8s|%-8s|%-8s|%-7s|%-7s|%-7s\n",
            "ErrTotal", "HPD", "UnHPD", "EdidFail", "HdcpFail", "HdcpSucc", "RsenCon", "RsenDis", "HdcpUsr");
        /* line2 */
        osal_seq_printf(file, "WR:|%-8u|%-3u|%-5u|%-8u|%-8u|%-8u|%-7u|%-7u|%-7u\n",
            run_cnt.err_wd_cnt, run_cnt.hpd_wr_cnt, run_cnt.unhpd_wr_cnt,
            run_cnt.edid_fail_wr_cnt, 0, 0, run_cnt.rsen_con_wr_cnt, run_cnt.rsen_dis_wr_cnt, 0);
        /* line3 */
        osal_seq_printf(file, "RD:|%-8u|%-3u|%-5u|%-8u|%-8u|%-8u|%-7u|%-7u|%-7u\n",
            run_cnt.err_rd_cnt, run_cnt.hpd_rd_cnt, run_cnt.unhpd_rd_cnt,
            run_cnt.edid_fail_rd_cnt, 0, 0, run_cnt.rsen_con_rd_cnt, run_cnt.rsen_dis_rd_cnt, 0);
        /* line4 */
        osal_seq_printf(file, "Memory[WkFlg=%1d |RdAble=%2d| RdPtr=%-2d| WrPtr=%-2d]:",
            ctrl.wakeup_flag, ctrl.readable_cnt, ctrl.read_ptr, ctrl.write_ptr);
        for (j = 0; j < hdmi_array_size(ctrl.event_pool); j++) {
            if (ctrl.event_pool[j] < HDMI_EVENT_BUTT) {
                osal_seq_printf(file, " 0x%02x", ctrl.event_pool[j]);
            }
        }
        osal_seq_printf(file, "\n");
    }

    return;
}

static hi_void proc_edid_raw_data(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_u32          i, raw_len;
    hi_s32          ret;
    hi_u8          *edid_raw  = HI_NULL;
    hdmi_device    *hdmi_dev  = HI_NULL;
    hdmi_edid_info *edid_info = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    edid_info = &hdmi_dev->edid_info;
    edid_raw = (hi_u8 *)osal_vmalloc(HDMI_EDID_TOTAL_SIZE);
    hdmi_if_null_warn_return_void(edid_raw);
    (hi_void)memset_s(edid_raw, HDMI_EDID_TOTAL_SIZE, 0, HDMI_EDID_TOTAL_SIZE);

    raw_len = HDMI_EDID_TOTAL_SIZE;
    ret = drv_hdmi_edid_raw_get(edid_info, edid_raw, raw_len);
    if (ret == HI_ERR_HDMI_NUL_PTR || ret == HI_ERR_HDMI_INVALID_PARA || ret == HI_FAILURE) {
        hdmi_err("edid raw get fail!\n");
        osal_vfree(edid_raw);
        return;
    }
    raw_len = (hi_u32)ret;

    /* raw data */
    osal_seq_printf(file,
        "--------------------------------------- EDIDRawData -----------------------------------------\n");
    if (raw_len != 0xffffffff) {
        for (i = 0; i < raw_len && i < HDMI_EDID_TOTAL_SIZE; i++) {
            if (i == 0) {
                osal_seq_printf(file, "/* %02xH: */ ", i);
            }
            osal_seq_printf(file, "0x%02x,", edid_raw[i]);
            if ((((i + 1) % STR_LEN_SUPERADDITION) == 0) && ((i + 1) < raw_len)) {
                osal_seq_printf(file, "  ");
            }
            if ((((i + 1) % 16) == 0) && ((i + 1) < raw_len)) { /* 16 eq 4*4, means oneline display 4 EDID format */
                osal_seq_printf(file, "\n/* %02xH: */ ", i);
            }
        }
        osal_seq_printf(file, "\n");
    }
    osal_vfree(edid_raw);

    return;
}

static hi_void proc_edid_status(hdmi_proc_entry *file, hdmi_device_id hdmi_id)
{
    hi_s32           ret;
    hdmi_edid_status status    = {0};
    hdmi_device     *hdmi_dev  = HI_NULL;
    hdmi_edid_info  *edid_info = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_warn_return_void(hdmi_dev);
    edid_info = &hdmi_dev->edid_info;
    ret = dfrv_hdmi_edid_status_get(edid_info, &status);
    if (ret != HI_SUCCESS) {
        hdmi_err("dfrv_hdmi_edid_status_get failed!!!\n");
        return;
    }

    osal_seq_printf(file, "------------------------------ SWStatus -------------------------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-20s: %-20s ", "CapFromSink", status.cap_sink ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20d \n", "RawUpdateErrCnt", status.raw_update_err_cnt);
    /* line 2 */
    osal_seq_printf(file, "%-20s: %-20s ", "CapIsValid", status.cap_valid ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20d \n", "ParseErrorType", status.parse_err);
    /* line 3 */
    osal_seq_printf(file, "%-20s: %-20s ", "RawIsValid", status.raw_valid ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: 0x%08x\n", "ParseWarnType", status.parse_warn);
    /* line 4 */
    osal_seq_printf(file, "%-20s: %-20d ", "RawGetErrCnt", status.raw_get_err_cnt);
    osal_seq_printf(file, "%-20s: %-20d \n", "RawLength", status.raw_len);

    return;
}

static hi_s32 proc_get_cap_info(hdmi_device_id hdmi_id, hdmi_sink_capability **cap)
{
    hi_s32 ret;
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_edid_info *edid_info = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    if (hdmi_dev == HI_NULL) {
        hdmi_err("get hdmi_dev=null!\n");
        return HI_FAILURE;
    }

    edid_info = &hdmi_dev->edid_info;
    ret = drv_hdmi_edid_capability_get(edid_info, cap);
    if (ret == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("current capability invalid, force to get raw!!!\n");
        ret = drv_hdmi_edid_update(edid_info, HDMI_EDID_UPDATE_SINK);
        if (ret != HI_SUCCESS) {
            hdmi_warn("please make sure HDMI cable connected & start!\n");
            return HI_FAILURE;
        }

        ret = drv_hdmi_edid_capability_get(edid_info, cap);
        if (ret == HDMI_EDID_DATA_INVALID) {
            hdmi_warn("get the edid capability error, ret=%d, cap=%p!\n", ret, cap);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static hi_void proc_sink_basic_cap(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 i;
    errno_t ret;
    hi_char str_tmp_array[PROC_TEMP_STR_LEN];

    /* basic */
    osal_seq_printf(file, "------------------------------ BasicCap -------------------------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-20s: %-20s ", "HDMI1.4Support", cap->support_hdmi ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-1d.%-1d\n", "1stBlockVersion", cap->version, cap->revision);
    /* line 2 */
    osal_seq_printf(file, "%-20s: %-20s ", "HDMI2.0Support", cap->support_hdmi_20 ? STR_YES : STR_NO);
    (hi_void)memset_s(str_tmp_array, sizeof(str_tmp_array), '\0', sizeof(str_tmp_array));
    ret = strncpy_s(str_tmp_array, PROC_TEMP_STR_LEN, cap->mfrs_info.mfrs_name, 4); /* 4, mfrs_name[4] size 4 type */
    hdmi_unequal_eok_return_void(ret);
    osal_seq_printf(file, "%-20s: %-20s \n", "ManufacturerName", str_tmp_array);
    /* line 3 */
    osal_seq_printf(file, "%-20s: %-20d ", "MaxTMDSClock(MHz)", cap->max_tmds_clock);
    osal_seq_printf(file, "%-20s: %-20u \n", "ProductCode", cap->mfrs_info.product_code);
    /* line 4 */
    osal_seq_printf(file, "%-20s: %-20u ", "SerialNumber", cap->mfrs_info.serial_number);
    osal_seq_printf(file, "%-20s: %-20u \n", "WeekOfManufacture", cap->mfrs_info.week);
    /* line 5 */
    osal_seq_printf(file, "%-20s: %-20u ", "MaxDispWidth", cap->basic_disp_para.max_image_width);
    osal_seq_printf(file, "%-20s: %-20u \n", "MaxDispHeight", cap->basic_disp_para.max_image_height);
    /* line 6 */
    osal_seq_printf(file, "%-20s: %-20s ", "SCDCSupport", cap->support_scdc ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20u \n", "YearOfManufacture", cap->mfrs_info.year);
    /* line 7 */
    osal_seq_printf(file, "%-20s: %-20s ", "DVIDualSupport", cap->support_dvi_dual ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s \n", "CECAddrIsValid", cap->cec_addr.phy_addr_valid ? STR_YES : STR_NO);
    /* line 8 */
    osal_seq_printf(file, "%-20s: %-20s ", "AISupport", cap->supports_ai ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %02x.%02x.%02x.%02x\n", "CECAddr", cap->cec_addr.phy_addr_a,
        cap->cec_addr.phy_addr_b, cap->cec_addr.phy_addr_c, cap->cec_addr.phy_addr_d);
    /* line 9 */
    osal_seq_printf(file, "%-20s: %-20u ", "ExtBlockCnt", cap->ext_block_num);
    /* speaker */
    osal_seq_printf(file, "%-20s:", "SpeakerSupport");
    for (i = 0; i < hdmi_array_size(cap->support_audio_speaker); i++) {
        if (i < hdmi_array_size(g_str_speaker_str)) {
            osal_seq_printf(file, " %s", cap->support_audio_speaker[i] ? g_str_speaker_str[i] : "");
        }
    }
    osal_seq_printf(file, "\n");
    /* line 10 */
    osal_seq_printf(file, "%-20s: %-20s ", "RgbQrangeSelectable", cap->rgb_qrange_selectable ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s \n", "YccQrangeSelectable", cap->ycc_qrange_selectable ? STR_YES : STR_NO);

    return;
}

static hi_void proc_sink_video_deepcolor(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    /* line 3 deep_color */
    osal_seq_printf(file, "%-20s:", "DeepColor");
    if (cap->deep_color.deep_color30_bit) {
        osal_seq_printf(file, " %-s", "RGB_30Bit");
    }
    if (cap->deep_color.deep_color36_bit) {
        osal_seq_printf(file, " %-s", "RGB_36Bit");
    }
    if (cap->deep_color.deep_color48_bit) {
        osal_seq_printf(file, " %-s", "RGB_48Bit");
    }
    if (cap->deep_color.deep_color_y444) {
        osal_seq_printf(file, " %-s", "YCbCr444_SameRGB");
    }
    osal_seq_printf(file, "\n");
    /* line 4 deep_color_y420 */
    osal_seq_printf(file, "%-20s:", "YCbCr420DeepColor");
    if (cap->deep_color_y420.deep_color30_bit) {
        osal_seq_printf(file, " %-s", "30Bit");
    }
    if (cap->deep_color_y420.deep_color36_bit) {
        osal_seq_printf(file, " %-s", "36Bit");
    }
    if (cap->deep_color_y420.deep_color48_bit) {
        osal_seq_printf(file, " %-s", "48Bit");
    }
    osal_seq_printf(file, "\n");

    return;
}

static hi_void proc_sink_video_colorimetry(hdmi_proc_entry *file, const hdmi_edid_colorimetry *colorimetry)
{
    osal_seq_printf(file, "%-20s:", "Colorimetry");
    if (colorimetry->xvycc601) {
        osal_seq_printf(file, " %-s", "xvYCC601");
    }
    if (colorimetry->xvycc709) {
        osal_seq_printf(file, " %-s", "xvYCC709");
    }
    if (colorimetry->sycc601) {
        osal_seq_printf(file, " %-s", "sYCC601");
    }
    if (colorimetry->adoble_ycc601) {
        osal_seq_printf(file, " %-s", "AdobleYCC601");
    }
    if (colorimetry->adoble_rgb) {
        osal_seq_printf(file, " %-s", "AdobleRGB");
    }
    if (colorimetry->bt2020c_ycc) {
        osal_seq_printf(file, " %-s", "BT2020cYCC");
    }
    if (colorimetry->bt2020_ycc) {
        osal_seq_printf(file, " %-s", "BT2020YCC");
    }
    if (colorimetry->bt2020_rgb) {
        osal_seq_printf(file, " %-s", "BT2020RGB");
    }
    osal_seq_printf(file, "\n");

    return;
}

static hi_void proc_sink_video_cap(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 i;
    hi_char *tmp_fmt = HI_NULL;

    tmp_fmt = proc_vicformat(cap->native_format);
    hdmi_if_null_warn_return_void(tmp_fmt);

    osal_seq_printf(file, "------------------------------ VidoCap --------------------------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-20s: %-s(VIC %2d)\n", "NativeFormat",
        (tmp_fmt == HI_NULL ? "" : tmp_fmt), cap->native_format);
    /* line 2 color_space */
    osal_seq_printf(file, "%-20s:", "ColorSpace");
    if (cap->color_space.rgb444) {
        osal_seq_printf(file, " %-s", "RGB444");
    }
    if (cap->color_space.ycbcr444) {
        osal_seq_printf(file, " %-s", "YCbCr444");
    }
    if (cap->color_space.ycbcr422) {
        osal_seq_printf(file, " %-s", "YCbCr422");
    }
    if (cap->only_support_y420_vic_num || cap->support_y420_vic_num) {
        osal_seq_printf(file, " %-s", "YCbCr420");
    }
    osal_seq_printf(file, "\n");
    /* line 3~4 deep_color */
    proc_sink_video_deepcolor(file, cap);

    /* line 5 y420_format[also] */
    osal_seq_printf(file, "%-20s:", "YCbCr420[Also]");
    for (i = 0; (i < cap->support_y420_vic_num) && (i < HDMI_EDID_MAX_VIC_COUNT); i++) {
        osal_seq_printf(file, " %u", cap->support_y420_format[i]);
    }
    osal_seq_printf(file, "\n");
    /* line 6 y420_format[only] */
    osal_seq_printf(file, "%-20s:", "YCbCr420[Only]");
    for (i = 0; (i < cap->only_support_y420_vic_num) && (i < HDMI_EDID_MAX_VIC_COUNT); i++) {
        osal_seq_printf(file, " %u", cap->only_support_y420_format[i]);
    }
    osal_seq_printf(file, "\n");
    /* line 7 colorimetry */
    proc_sink_video_colorimetry(file, &cap->color_metry);

    return;
}

static hi_bool proc_sink_std_format(hdmi_proc_entry *file, const hdmi_edid_std_timing *std_timing)
{
    errno_t ret;
    hi_bool line_feed = HI_FALSE;
    hi_char *fmt_str = HI_NULL;
    hi_char tmp_array[PROC_TEMP_STR_LEN];

    fmt_str = proc_std_timingformat(std_timing);
    if (fmt_str != HI_NULL) {
        osal_seq_printf(file, "%-20s ", fmt_str);
        line_feed = HI_TRUE;
    } else if ((std_timing->hor_active != 0) && (std_timing->ver_active != 0)) {
        int n;
        (hi_void)memset_s(tmp_array, sizeof(tmp_array), '\0', sizeof(tmp_array));
        n = snprintf_s(tmp_array, PROC_TEMP_STR_LEN, PROC_TEMP_STR_LEN - 1, "%ux%u",
            std_timing->hor_active, std_timing->ver_active);
        if (n < 0) {
            hdmi_err("snprintf_s err\n");
            return HI_FALSE;
        }
        if ((std_timing->refresh_rate) != 0) {
            ret = snprintf_s(tmp_array + n, PROC_LENGTH_SIZE - n,
                PROC_LENGTH_SIZE - n - 1, "_%u", std_timing->refresh_rate);
            if (ret < 0) {
                hdmi_err("snprintf_s err\n");
                return HI_FALSE;
            }
        }
        osal_seq_printf(file, "%-20s ", tmp_array);
        line_feed = HI_TRUE;
    }

    return line_feed;
}

static hi_void proc_sink_format_cap(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 i;
    hi_s32 j = 0;
    hi_char *fmt_str = HI_NULL;

    fmt_str = proc_vicformat(cap->native_format);
    hdmi_if_null_warn_return_void(fmt_str);

    osal_seq_printf(file, "------------------------------ FormatCap ------------------------------------------\n");
    /* line 1 */
    for (i = 0; (i < cap->support_vic_num) && (i < HDMI_EDID_MAX_VIC_COUNT); i++) {
        fmt_str = proc_vicformat(cap->support_format[i]);
        if (fmt_str != HI_NULL) {
            osal_seq_printf(file, "%-20s ", fmt_str);
        } else {
            osal_seq_printf(file, "(VIC)%-15u ", cap->support_format[i]);
        }
        j++;
        if ((j % PROC_ONE_LINE_STR_COUNT) == 0) {
            osal_seq_printf(file, "\n");
        }
    }

    for (i = 0; i < hdmi_array_size(cap->std_timing); i++) {
        if (proc_sink_std_format(file, &cap->std_timing[i]) == HI_TRUE) {
            j++;
            if ((j % PROC_ONE_LINE_STR_COUNT) == 0) {
                osal_seq_printf(file, "\n");
            }
        }
    }

    /* VESA enhanced EDID standard release A, rev.1 page 16 */
    for (i = 0;
         (i < cap->estab_num) && (i < HDMI_EDID_ESTABTIMG_BUTT) &&
         (cap->estab_timing[i] < hdmi_array_size(g_estab_timing_str));
         i++) {
        osal_seq_printf(file, "%-20s ", g_estab_timing_str[i]);
        j++;
        if ((j % PROC_ONE_LINE_STR_COUNT) == 0) {
            osal_seq_printf(file, "\n");
        }
    }
    if ((j % PROC_ONE_LINE_STR_COUNT) != 0) {
        osal_seq_printf(file, "\n");
    }

    return;
}

#ifdef HDMI_3D_SUPPORT
static hi_void proc_sink_3d_cap(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 i;

    osal_seq_printf(file, "------------------------------ 3DCap ----------------------------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-20s: %-20s ", "3DSupport", cap->_3d_info.support3_d ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s \n", "3DOsdDisparity", cap->support3d_osd_disparity ? STR_YES : STR_NO);
    /* line 2 */
    osal_seq_printf(file, "%-20s: %-20s ", "3DDualView ", cap->support3d_dual_view ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s \n", "3DIndepView", cap->support3d_independent_view ? STR_YES : STR_NO);
    /* line 3 */
    osal_seq_printf(file, "%-20s:", "3DTypeSupport");
    for (i = 0; i < hdmi_array_size(cap->_3d_info.support3_d_type); i++) {
        if (cap->_3d_info.support3_d_type[i] && i < hdmi_array_size(g_stere_mode_str)) {
            osal_seq_printf(file, " %s", g_stere_mode_str[i]);
        }
    }
    osal_seq_printf(file, "\n");
}
#endif

static hi_void proc_sink_audio_cap(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 i, j;
    hdmi_audio_format_code aud_fmt_code;
    const hdmi_edid_audio_info *audio_info = HI_NULL;

    osal_seq_printf(file, "------------------------------ AudioCap -------------------------------------------\n");
    /* audio info */
    for (i = 0; (i < cap->audio_info_num) && (i < HDMI_EDID_MAX_AUDIO_CAP_COUNT); i++) {
        /* line 1 */
        osal_seq_printf(file, "%s.%-d: \n", "NO", i);
        aud_fmt_code = cap->audio_info[i].aud_fmt_code;
        audio_info = &cap->audio_info[i];
        /* line 2 */
        osal_seq_printf(file, "%-20s: %-20s ", "CodeType", g_audio_code_type_str[aud_fmt_code]);
        osal_seq_printf(file, "%-20s: %-20d \n", "MaxChannelNum", audio_info->aud_channel);
        /* line 3 */
        if (audio_info->max_bit_rate) {
            osal_seq_printf(file, "%-20s: %-20d ", "MaxBitRate(KHz)", audio_info->max_bit_rate);
        } else {
            osal_seq_printf(file, "%-20s: %-20s ", "MaxBitRate(KHz)", STR_NA);
        }
        osal_seq_printf(file, "%-20s:", "BitDepth");
        if (audio_info->support_bit_depth_num == 0) {
            osal_seq_printf(file, " %s", STR_NA);
        }
        for (j = 0; (j < audio_info->support_bit_depth_num) && (j < HDMI_EDID_MAX_BIT_DEPTH_NUM); j++) {
            osal_seq_printf(file, " %u", audio_info->support_bit_depth[j]);
        }
        osal_seq_printf(file, "\n");
        /* line 4 */
        osal_seq_printf(file, "%-20s:", "SampleRate(Hz)");
        for (j = 0; (j < audio_info->support_sample_rate_num) && (j < HDMI_EDID_MAX_SAMPE_RATE_NUM); j++) {
            osal_seq_printf(file, " %u", audio_info->support_sample_rate[j]);
        }
        osal_seq_printf(file, "\n");
    }

    return;
}

#ifdef HDMI_HDR_SUPPORT
static hi_void proc_sink_hdr_cap(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    osal_seq_printf(file, "------------------------------- HdrCap -------------------------------------------\n");
    /* line 1 */
    osal_seq_printf(file, "%-20s: %-20s ", "HdrEotfSdr", cap->hdr_cap.eotf.eotf_sdr ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s \n", "HdrEotfHdr", cap->hdr_cap.eotf.eotf_hdr ? STR_YES : STR_NO);
    /* line 2 */
    osal_seq_printf(file, "%-20s: %-20s ", "HdrEotfSt2084", cap->hdr_cap.eotf.eotf_smpte_st2084 ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s \n", "HdrEotfHLG", cap->hdr_cap.eotf.eotf_hlg ? STR_YES : STR_NO);
    /* line 3 */
    osal_seq_printf(file, "%-20s: %-20d ", "MaxLum", cap->hdr_cap.max_luminance_cv);
    osal_seq_printf(file, "%-20s: %-20d \n", "AvgLum", cap->hdr_cap.average_lumin_cv);
    /* line 4 */
    osal_seq_printf(file, "%-20s: %-20d \n", "MinLum", cap->hdr_cap.min_luminance_cv);
    osal_seq_printf(file, "------------------------------- DolbyCap -----------------------------------------\n");
    osal_seq_printf(file, "%-20s: 0x%-18x ", "DolbyOUI", cap->dolby_cap.dolby_oui);
    osal_seq_printf(file, "%-20s: %-20d\n", "DolbyCapsVer", cap->dolby_cap.vsvdb_version);
    /* line 1 */
    osal_seq_printf(file, "%-20s: %-20s ", "DolbySu_Y422", cap->dolby_cap.yuv422_12_bit ? STR_YES : STR_NO);
    osal_seq_printf(file, "%-20s: %-20s\n", "DolbySu_2160P60", cap->dolby_cap.b2160_p60 ? STR_YES : STR_NO);
    /* line 2 */
    osal_seq_printf(file, "%-20s: %-20d ", "DolbyRed_X", cap->dolby_cap.red_x);
    osal_seq_printf(file, "%-20s: %-20d\n", "DolbyRed_Y", cap->dolby_cap.red_y);
    /* line 3 */
    osal_seq_printf(file, "%-20s: %-20d ", "DolbyGreen_X", cap->dolby_cap.green_x);
    osal_seq_printf(file, "%-20s: %-20d\n", "DolbyGreen_Y", cap->dolby_cap.green_y);
    /* line 4 */
    osal_seq_printf(file, "%-20s: %-20d ", "DolbyBlue_X", cap->dolby_cap.blue_x);
    osal_seq_printf(file, "%-20s: %-20d\n", "DolbyBlue_Y", cap->dolby_cap.blue_y);
    /* line 5 */
    osal_seq_printf(file, "%-20s: %-20d ", "DolbyMinLum", cap->dolby_cap.min_luminance);
    osal_seq_printf(file, "%-20s: %-20d\n", "DolbyMaxLum", cap->dolby_cap.max_luminance);
    if (cap->dolby_cap.vsvdb_version == 0) {
        /* line 6 */
        osal_seq_printf(file, "%-20s: %-20d ", "DolbyWhite_X", cap->dolby_cap.un_other_caps.dolby_caps_ver0.white_x);
        osal_seq_printf(file, "%-20s: %-20d\n", "DolbyWhite_Y", cap->dolby_cap.un_other_caps.dolby_caps_ver0.white_y);
        /* line 7 */
        osal_seq_printf(file, "%-20s: %-20d ", "DMmajorVer",
            cap->dolby_cap.un_other_caps.dolby_caps_ver0.d_mmajor_version);
        osal_seq_printf(file, "%-20s: %-20d\n", "DMminorVer",
            cap->dolby_cap.un_other_caps.dolby_caps_ver0.d_mminor_version);
    } else if (cap->dolby_cap.vsvdb_version == 1) {
        /* line 8 */
        osal_seq_printf(file, "%-20s: %-20d ", "DMVersion", cap->dolby_cap.un_other_caps.dolby_caps_ver1.dm_version);
        osal_seq_printf(file, "%-20s: %-20d\n", "Colorimetry",
            cap->dolby_cap.un_other_caps.dolby_caps_ver1.colorimetry);
    }
}
#endif

#ifdef HDMI_FRL_SUPPORT
static hi_void proc_sink_frl_caps(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 index;
    index = min((hi_s32)cap->max_frl_rate, FRL_WORK_MODE_BUTT);
    osal_seq_printf(file, "%-20s: %-20s ", "MaxFrlRate", g_frl_rate_str[index]);
    osal_seq_printf(file, "%-20s: %-20s\n", "DSCSupport", cap->dsc_info.dsc_1p2 ? STR_YES : STR_NO);
}
#endif

static hi_void proc_sink_detail_timing(hdmi_proc_entry *file, const hdmi_sink_capability *cap)
{
    hi_u32 i;

    /* detail timing */
    if (cap->perfer_timing_num > 0) {
        /* line1 */
        osal_seq_printf(file,
            "------------------------------ DetailTiming ------------------------------------------\n");
        /* line2 */
        osal_seq_printf(file,
            "%-5s:%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s|%-4s\n",
            "[NO.]", "HACT", "VACT", "P/I", "PClk", "AspW", "AspH",
            "HFB", "HPW", "HBB", "VFB", "VPW", "VBB", "img_w", "img_h", "IHS", "IVS", "IDV");
        for (i = 0; (i < cap->perfer_timing_num) && (i < HDMI_EDID_MAX_DETAIL_TIMING_COUNT); i++) {
            osal_seq_printf(file,
                "[%3u]:%-4u|%-4u|%-4c|%-3uM|%-4u|%-4u|%-4u|%-4u|%-4u|%-4u|%-4u|%-4u|%-4u|%-4u|%-4s|%-4s|%-4s\n",
                i,
                cap->perfer_timing[i].hact,
                cap->perfer_timing[i].vact,
                cap->perfer_timing[i].interlace ? 'I' : 'P',
                cap->perfer_timing[i].pixel_clk / HDMI_THOUSAND,
                cap->perfer_timing[i].aspect_ratio_w,
                cap->perfer_timing[i].aspect_ratio_h,
                cap->perfer_timing[i].hfb,
                cap->perfer_timing[i].hpw,
                cap->perfer_timing[i].hbb,
                cap->perfer_timing[i].vfb,
                cap->perfer_timing[i].vpw,
                cap->perfer_timing[i].vbb,
                cap->perfer_timing[i].image_width,
                cap->perfer_timing[i].image_height,
                cap->perfer_timing[i].ihs ? STR_YES : STR_NO,
                cap->perfer_timing[i].ivs ? STR_YES : STR_NO,
                cap->perfer_timing[i].idv ? STR_YES : STR_NO);
        }
    }

    return;
}

hi_s32 drv_hdmi_read_proc(hdmi_proc_entry *file)
{
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_if_null_warn_return(file, HI_FAILURE);
    hdmi_dev = get_hdmi_device(HDMI_DEVICE_ID0);
    hdmi_if_null_warn_return(hdmi_dev, HI_FAILURE);
    /* to avoid the hal pointer is null when call the proc and deinit at the same time. */
    hdmi_mutex_lock(hdmi_dev->mutex_proc);

    osal_seq_printf(file, "\n%s\n\n", VERSION_STRING);
    proc_sw_version(file);
    proc_app_attr_info(file, HDMI_DEVICE_ID0);
    proc_sw_status(file, HDMI_DEVICE_ID0);
#if defined HDMI_HDR_SUPPORT
    proc_hdr_status(file, HDMI_DEVICE_ID0);
#endif
#ifdef HDMI_FRL_SUPPORT
    proc_frl_status(file, HDMI_DEVICE_ID0);
#endif
    proc_hw_status(file, HDMI_DEVICE_ID0);
    proc_event_info(file, HDMI_DEVICE_ID0);
    hdmi_mutex_unlock(hdmi_dev->mutex_proc);

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_vo_read_proc(hdmi_proc_entry *file)
{
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_if_null_warn_return(file, HI_FAILURE);
    hdmi_dev = get_hdmi_device(HDMI_DEVICE_ID0);
    hdmi_if_null_warn_return(hdmi_dev, HI_FAILURE);
    /* to avoid the hal pointer is null when call the proc and deinit at the same time. */
    hdmi_mutex_lock(hdmi_dev->mutex_proc);
    osal_seq_printf(file, "\n%s\n\n", VERSION_STRING);

    proc_sw_version(file);
    proc_video_info(file, HDMI_DEVICE_ID0);
#ifdef HDMI_HDR_SUPPORT
    proc_hdr_info(file, HDMI_DEVICE_ID0);
#endif
    hdmi_mutex_unlock(hdmi_dev->mutex_proc);

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_ao_read_proc(osal_proc_entry_t *file)
{
    hdmi_device *hdmi_dev = HI_NULL;

    hdmi_if_null_warn_return(file, HI_FAILURE);
    hdmi_dev = get_hdmi_device(HDMI_DEVICE_ID0);
    hdmi_if_null_warn_return(hdmi_dev, HI_FAILURE);
    /* to avoid the hal pointer is null when call the proc and deinit at the same time. */
    hdmi_mutex_lock(hdmi_dev->mutex_proc);
    osal_seq_printf(file, "\n%s\n\n", VERSION_STRING);

    proc_sw_version(file);
    proc_audio_info(file, HDMI_DEVICE_ID0);

    hdmi_mutex_unlock(hdmi_dev->mutex_proc);

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_sink_read_proc(osal_proc_entry_t *file)
{
    hi_s32 ret;
    hdmi_device          *hdmi_dev = HI_NULL;
    hdmi_sink_capability *cap      = HI_NULL;

    hdmi_if_null_warn_return(file, HI_FAILURE);
    hdmi_dev = get_hdmi_device(HDMI_DEVICE_ID0);
    hdmi_if_null_warn_return(hdmi_dev, HI_FAILURE);
    /* to avoid the hal pointer is null when call the proc and deinit at the same time. */
    hdmi_mutex_lock(hdmi_dev->mutex_proc);

    osal_seq_printf(file, "\n%s\n\n", VERSION_STRING);

    proc_sw_version(file);
    proc_edid_raw_data(file, HDMI_DEVICE_ID0);
    proc_edid_status(file, HDMI_DEVICE_ID0);
    ret = proc_get_cap_info(HDMI_DEVICE_ID0, &cap);
    if (ret == HI_SUCCESS && cap != HI_NULL) {
        proc_sink_basic_cap(file, cap);
#ifdef HDMI_FRL_SUPPORT
        proc_sink_frl_caps(file, cap);
#endif
        proc_sink_video_cap(file, cap);
        proc_sink_format_cap(file, cap);
#ifdef HDMI_3D_SUPPORT
        proc_sink_3d_cap(file, cap);
#endif
        proc_sink_audio_cap(file, cap);
#ifdef HDMI_HDR_SUPPORT
        proc_sink_hdr_cap(file, cap);
#endif
        proc_sink_detail_timing(file, cap);
    } else {
        hdmi_warn("call proc_get_cap_info failed! ret: %d, cap: %p\n", ret, cap);
    }
    hdmi_mutex_unlock(hdmi_dev->mutex_proc);

    return HI_SUCCESS;
}


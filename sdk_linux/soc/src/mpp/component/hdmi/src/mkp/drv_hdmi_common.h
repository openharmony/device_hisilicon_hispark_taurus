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
#ifndef __DRV_HDMI_COMMON_H_
#define __DRV_HDMI_COMMON_H_

#include "hi_type.h"
#include "drv_hdmi_infoframe.h"
#include "hdmi_ext.h"
#include "securec.h"
#include "hi_comm_hdmi.h"

#define HDMI_VER_MAJOR    2
#define HDMI_VER_MINOR    0
#define HDMI_VER_REVISE   0
#define HDMI_VER_DATE     20201202
#define HDMI_VER_TIMES    0

#define make_ver_bit(x)     #x
#define make_macro2str(exp) make_ver_bit(exp)
#define MAKE_VERSION                    \
    make_macro2str(HDMI_VER_MAJOR) "."  \
    make_macro2str(HDMI_VER_MINOR) "."  \
    make_macro2str(HDMI_VER_REVISE) "." \
    make_macro2str(HDMI_VER_DATE) "."   \
    make_macro2str(HDMI_VER_TIMES)

#define MAX_FRL_RATE                6
#define HDMI_FRL_LANE_MAX_NUM       4
#define CEA_VIDEO_CODE_MAX          44
#define VESA_VIDEO_CODE_MAX         31
#define CEA861_F_VIDEO_CODES_MAX_4K 4
#define HDMI_INFO_FRAME_MAX_SIZE    31
#define SCDC_TMDS_BIT_CLK_RATIO_10X 10
#define SCDC_TMDS_BIT_CLK_RATIO_40X 40
#ifdef CONFIG_HI_PLATFORM_H8
#define HI_ERR_NULL_PTR HI_ERR_HDMI_NULL_PTR
#else
#define HI_ERR_NULL_PTR HI_ERR_HDMI_NUL_PTR
#endif
#define HDMI_DECIMAL           10
#define HDMI_HUNDRED           100
#define HDMI_THOUSAND          1000
#define FMT_PIX_CLK_13400      13400
#define FMT_PIX_CLK_74250      74250
#define FMT_PIX_CLK_165000     165000
#define FMT_PIX_CLK_190000     190000
#define FMT_PIX_CLK_297000     297000
#define FMT_PIX_CLK_340000     340000
#define ZERO_DRMIF_SEND_TIME   2000 /* unit: ms */
#define HDRMODE_CHANGE_TIME    500  /* unit: ms */
#define HDMI_EDID_BLOCK_SIZE   128
#define HDMI_EDID_TOTAL_BLOCKS 4
#define HDMI_EDID_SIZE         (HDMI_EDID_BLOCK_SIZE * HDMI_EDID_TOTAL_BLOCKS)
#define HDMI_REGISTER_SIZE     4
#define hdmi_array_size(a)     (sizeof(a) / sizeof(a[0]))
#ifdef HDMI_FPGA_SUPPORT
#define FPGA_SUPPORT HI_TRUE
#else
#define FPGA_SUPPORT HI_FALSE
#endif
#define FRL_CTRL_TYPE_COMPRESS_ALL       0x00
#define FRL_CTRL_TYPE_COMPRESS_HW        0x01
#define FRL_CTRL_TYPE_COMPRESS_NON       0x03
#define HDMI_FRL_COMPRESS_DEBUG_MASK 0x4

/* AVI InfoFrame Packet byte offset define */
#define AVI_OFFSET_TYPE 0
#define AVI_OFFSET_VERSION 1
#define AVI_OFFSET_LENGTH 2
#define AVI_OFFSET_CHECKSUM 3

#define hdmi_unequal_eok_return(ret, err_code)             \
    do {                                                   \
        if ((ret) != EOK) {                                \
            hdmi_err("secure function error:%d\n", (ret)); \
            return (err_code);                             \
        }                                                  \
    } while (0)

#define hdmi_unequal_eok_return_void(ret)                  \
    do {                                                   \
        if ((ret) != EOK) {                                \
            hdmi_err("secure function error:%d\n", (ret)); \
            return;                                        \
        }                                                  \
    } while (0)

#define hdmi_unlock_unequal_eok_return(ret, mutex, err_code) \
    do {                                                     \
        if ((ret) != EOK) {                                  \
            hdmi_err("secure function error:%d\n", (ret));   \
            hdmi_mutex_unlock((mutex));                      \
            return (err_code);                               \
        }                                                    \
    } while (0)

#define hdmi_unlock_if_null_return(p, mutex, err_code) \
    do {                                               \
        if ((p) == HI_NULL) {                          \
            hdmi_err("This is null pointer!\n");       \
            hdmi_mutex_unlock((mutex));                \
            return (err_code);                         \
        }                                              \
    } while (0)

#define hdmi_if_null_return(p, ret)                \
    do {                                           \
        if ((p) == HI_NULL) {                      \
            hdmi_err("%s is null pointer!\n", #p); \
            return (ret);                          \
        }                                          \
    } while (0)

#define hdmi_if_null_return_void(p)                \
    do {                                           \
        if ((p) == HI_NULL) {                      \
            hdmi_err("%s is null pointer!\n", #p); \
            return;                                \
        }                                          \
    } while (0)

#define hdmi_if_null_warn_return(p, ret)            \
    do {                                            \
        if ((p) == HI_NULL) {                       \
            hdmi_warn("%s is null pointer!\n", #p); \
            return (ret);                           \
        }                                           \
    } while (0)

#define hdmi_if_null_warn_return_void(p)            \
    do {                                            \
        if ((p) == HI_NULL) {                       \
            hdmi_warn("%s is null pointer!\n", #p); \
            return;                                 \
        }                                           \
    } while (0)

#define hdmi_if_false_return_void(b)        \
    do {                                    \
        if ((b) != HI_TRUE) {               \
            hdmi_err("%s is FALSE!\n", #b); \
            return;                         \
        }                                   \
    } while (0)

#define hdmi_if_false_return(tmp, ret)        \
    do {                                      \
        if ((tmp) != HI_TRUE) {               \
            hdmi_err("%s is FALSE!\n", #tmp); \
            return (ret);                     \
        }                                     \
    } while (0)

#define hdmi_if_false_warn_return(tmp, ret)    \
    do {                                       \
        if ((tmp) != HI_TRUE) {                \
            hdmi_warn("%s is FALSE!\n", #tmp); \
            return (ret);                      \
        }                                      \
    } while (0)

#define hdmi_if_false_warn_return_void(tmp)    \
    do {                                       \
        if ((tmp) != HI_TRUE) {                \
            hdmi_warn("%s is FALSE!\n", #tmp); \
            return;                            \
        }                                      \
    } while (0)

#define hdmi_check_is_change_return(tmp0, tmp1, ret)                                            \
    do {                                                                                        \
        if ((tmp0) != (tmp1)) {                                                                 \
            hdmi_info("%s change, old(%u)->new(%u) \n", #tmp0, (hi_u32)(tmp0), (hi_u32)(tmp1)); \
            return (ret);                                                                       \
        }                                                                                       \
    } while (0)

#define hdmi_check_max_return(value, max, ret)                    \
    do {                                                          \
        if ((value) > (max)) {                                    \
            hdmi_warn("value %u exceed max!\n", (hi_u32)(value)); \
            return (ret);                                         \
        }                                                         \
    } while (0)

#define hdmi_if_failure_return(tmp, ret)        \
    do {                                        \
        if ((tmp) != HI_SUCCESS) {              \
            hdmi_err("%s is failure!\n", #tmp); \
            return (ret);                       \
        }                                       \
    } while (0)

#define hdmi_if_failure_return_void(tmp)        \
    do {                                        \
        if ((tmp) != HI_SUCCESS) {              \
            hdmi_err("%s is failure!\n", #tmp); \
            return;                             \
        }                                       \
    } while (0)
#define hdmi_if_failure_warn_return_void(tmp)    \
    do {                                         \
        if ((tmp) != HI_SUCCESS) {               \
            hdmi_warn("%s is failure!\n", #tmp); \
            return;                              \
        }                                        \
    } while (0)

#define hdmi_set_bit(var, bit) \
    do {                       \
        (var) |= 1 << (bit);   \
    } while (0)

#define hdmi_clr_bit(var, bit)  \
    do {                        \
        (var) &= ~(1 << (bit)); \
    } while (0)

#define hdmi_if_fpga_return(ret)      \
    do {                              \
        if (FPGA_SUPPORT) {           \
            hdmi_warn("FPGA CFG!\n"); \
            return(ret);              \
        }                             \
    } while (0)

#define hdmi_if_fpga_return_void()    \
    do {                              \
        if (FPGA_SUPPORT) {           \
            hdmi_warn("FPGA CFG!\n"); \
            return;                   \
        }                             \
    } while (0)

#define is_bit_set(var, bit) ({ (var) & (0x1 << (bit)) ? HI_TRUE : HI_FALSE; })

#ifdef HDMI_LITEOS_SUPPORT
#define max(a, b) (((a) < (b)) ? (b) : (a))
#define min(a, b) (((a) > (b)) ? (b) : (a))
#endif

#define hal_call_ret(ret, func, param...)                      \
    do {                                                       \
        if (hdmi_dev != HI_NULL && hdmi_dev->hal != HI_NULL && \
            hdmi_dev->hal->func != HI_NULL) {                  \
            ret = hdmi_dev->hal->func(param);                  \
        } else {                                               \
            ret = HI_ERR_NULL_PTR;                             \
        }                                                      \
    } while (0)

#define hal_call_void(func, param...)                          \
    do {                                                       \
        if (hdmi_dev != HI_NULL && hdmi_dev->hal != HI_NULL && \
            hdmi_dev->hal->func != HI_NULL) {                  \
            hdmi_dev->hal->func(param);                        \
        } else {                                               \
            hdmi_warn("null pointer! \n");                     \
        }                                                      \
    } while (0)

#define hdmi_if_zero_return_void(x)        \
    do {                                   \
        if ((x) == 0) {                    \
            hdmi_err("%s is zero!\n", #x); \
            return;                        \
        }                                  \
    } while (0)

typedef enum {
    CHIP_HI3559AV100,
    CHIP_HI3516CV500,
    CHIP_HI3519AV100,
    CHIP_HI3531DV200,
    CHIP_BUTT
} hdmi_chip_type;

typedef enum {
    HDMI_DEVICE_ID0,
    HDMI_DEVICE_ID_BUTT
} hdmi_device_id;

typedef enum {
    HDMI_THREAD_STATE_RUN,
    HDMI_THREAD_STATE_STOP
} hdmi_thread_state;

typedef enum {
    HDMI_EVENT_HOTPLUG = 0x10,
    HDMI_EVENT_HOTUNPLUG,
    HDMI_EVENT_EDID_FAIL,
    HDMI_EVENT_HDCP_FAIL,
    HDMI_EVENT_HDCP_SUCCESS,
    HDMI_EVENT_RSEN_CONNECT,
    HDMI_EVENT_RSEN_DISCONNECT,
    HDMI_EVENT_HDCP_USERSETTING,
    HDMI_EVENT_HDCP_OFF,
    HDMI_EVENT_SCRAMBLE_FAIL,
    HDMI_EVENT_SCRAMBLE_SUCCESS,
    HDMI_EVENT_ZERO_DRMIF_TIMEOUT,
    HDMI_EVENT_SWITCH_TO_HDRMODE_TIMEOUT,
    HDMI_EVENT_BUTT
} hdmi_event;

typedef enum {
    HDMI_INT_TYPE_NONE,
    HDMI_INT_TYPE_HPD_RSEN,
    HDMI_INT_TYPE_HDCP,
    HDMI_INT_TYPE_HDCP2X,
    HDMI_INT_TYPE_CEC
} hdmi_int_type;

typedef hi_s32 (*hdmi_callback)(hi_void *, hdmi_event);

typedef struct {
    hi_void      *event_data;
    hdmi_callback event_callback;
    hi_u32        hdmi_dev_id;
    hi_char      *base_addr;
    hi_char      *phy_addr;
} hdmi_hal_init;

typedef struct {
    hdmi_colorimetry    colorimetry;
    hdmi_quant_range    quantization;
    hdmi_pixel_encoding pixel_encoding;
} hdmi_csc_attr;

typedef struct {
    hi_u8  edid_valid;
    hi_u32 edid_len;
    hi_u8  edid[HDMI_EDID_SIZE];
} hdmi_edid_raw_data;

typedef struct {
    hi_u8 *input_encrypted_key_addr;
    hi_u32 key_length;
} hdmi_load_key;

typedef struct {
    hi_u32 input_encrypted_key_addr;
    hi_u32 key_length;
} hdmi_compat_load_key;

typedef enum {
    HDMI_HDCP_VERSION_NONE,
    HDMI_HDCP_VERSION_1_4,
    HDMI_HDCP_VERSION_2_2,
    HDMI_HDCP_VERSION_BUTT
} hdmi_hdcp_version;

typedef struct {
    hi_bool           connected;
    hi_bool           sink_power_on;
    hi_bool           authed;
    hi_u8             bksv[5];
    hdmi_hdcp_version hdcp_version;
} hdmi_status;

typedef struct {
    hi_u32 i_de_main_clk;
    hi_u32 i_de_main_data;
    hi_u32 i_main_clk;
    hi_u32 i_main_data;
    hi_u32 ft_cap_clk;
    hi_u32 ft_cap_data;
} hdmi_hw_param;

typedef struct {
    hdmi_hw_param hw_param[4];
} hdmi_hw_spec;

typedef struct {
    hdmi_hw_spec  hwspec_user;
    hdmi_hw_spec  hwspec_def;
    hdmi_hw_param hwparam_cur;
} hdmi_hwspec;

/* HDMI deep color mode */
typedef enum {
    HDMI_DEEP_COLOR_24BIT,
    HDMI_DEEP_COLOR_30BIT,
    HDMI_DEEP_COLOR_36BIT,
    HDMI_DEEP_COLOR_48BIT,
    HDMI_DEEP_COLOR_OFF = 0xff,
    HDMI_DEEP_COLOR_BUTT
} hdmi_deep_color;

typedef enum {
    HDMI_VIDEO_BITDEPTH_8,
    HDMI_VIDEO_BITDEPTH_10,
    HDMI_VIDEO_BITDEPTH_12,
    HDMI_VIDEO_BITDEPTH_16,
    HDMI_VIDEO_BITDEPTH_OFF,
    HDMI_VIDEO_BITDEPTH_BUTT
} hdmi_video_bit_depth;

typedef enum {
    HDMI_HV_SYNC_POL_HPVP,
    HDMI_HV_SYNC_POL_HPVN,
    HDMI_HV_SYNC_POL_HNVP,
    HDMI_HV_SYNC_POL_HNVN,
    HDMI_HV_SYNC_POL_BUTT
} hdmi_hvsync_polarity;

typedef enum {
    HDMI_PICTURE_NON_UNIFORM__SCALING,
    HDMI_PICTURE_SCALING__H,
    HDMI_PICTURE_SCALING__V,
    HDMI_PICTURE_SCALING__HV
} hdmi_picture_scaling;

typedef struct {
    hi_u32                      clk_fs; /* VDP setting(in) */
    hi_u32                      tmds_clk;
    hi_u32                      hdmi_adapt_pix_clk; /* HDMI adapt setting(out) */
    hi_u32                      pixel_repeat;
    hi_bool                     v_sync_pol;
    hi_bool                     h_sync_pol;
    hi_bool                     de_pol;
    hdmi_video_timing           video_timing;
    hdmi_3d_mode                stereo_mode;
    hdmi_colorspace             in_color_space;
    hdmi_colormetry             colorimetry;
    hdmi_extended_colormetry    extended_colorimetry;
    hdmi_quantization_range     rgb_quantization;
    hdmi_ycc_quantization_range ycc_quantization;
    hdmi_picture_aspect         picture_aspect;
    hdmi_active_aspect          active_aspect;
    hdmi_picture_scaling        picture_scaling;
    hdmi_video_bit_depth        in_bit_depth;
    hdmi_disp_format            disp_fmt;
} hdmi_vo_attr;

typedef enum {
    HDMI_AUDIO_FORMAT_2CH = 0x02,
    HDMI_AUDIO_FORMAT_3CH,
    HDMI_AUDIO_FORMAT_4CH,
    HDMI_AUDIO_FORMAT_5CH,
    HDMI_AUDIO_FORMAT_6CH,
    HDMI_AUDIO_FORMAT_7CH,
    HDMI_AUDIO_FORMAT_8CH,
    HDMI_AUDIO_FORMAT_BUTT
} hdmi_audio_ch;

typedef enum {
    HDMI_AUDIO_INTF_I2S,
    HDMI_AUDIO_INTF_SPDIF,
    HDMI_AUDIO_INTF_HBRA,
    HDMI_AUDIO_INTF_BUTT
} hdmi_audio_interface;

typedef enum {
    HDMI_AUDIO_BIT_DEPTH_UNKNOWN,
    HDMI_AUDIO_BIT_DEPTH_8  = 8,
    HDMI_AUDIO_BIT_DEPTH_16 = 16,
    HDMI_AUDIO_BIT_DEPTH_18 = 18,
    HDMI_AUDIO_BIT_DEPTH_20 = 20,
    HDMI_AUDIO_BIT_DEPTH_24 = 24,
    HDMI_AUDIO_BIT_DEPTH_32 = 32,
    HDMI_AUDIO_BIT_DEPTH_BUTT
} hdmi_audio_bit_depth;

typedef enum {
    HDMI_SAMPLE_RATE_UNKNOWN,
    HDMI_SAMPLE_RATE_8K   = 8000,
    HDMI_SAMPLE_RATE_11K  = 11025,
    HDMI_SAMPLE_RATE_12K  = 12000,
    HDMI_SAMPLE_RATE_16K  = 16000,
    HDMI_SAMPLE_RATE_22K  = 22050,
    HDMI_SAMPLE_RATE_24K  = 24000,
    HDMI_SAMPLE_RATE_32K  = 32000,
    HDMI_SAMPLE_RATE_44K  = 44100,
    HDMI_SAMPLE_RATE_48K  = 48000,
    HDMI_SAMPLE_RATE_88K  = 88200,
    HDMI_SAMPLE_RATE_96K  = 96000,
    HDMI_SAMPLE_RATE_176K = 176400,
    HDMI_SAMPLE_RATE_192K = 192000,
    HDMI_SAMPLE_RATE_768K = 768000,
    HDMI_SAMPLE_RATE_BUTT
} hdmi_sample_rate;

typedef struct {
    hi_bool                down_sample;
    hdmi_sample_rate       sample_fs;
    hdmi_audio_ch          channels;
    hdmi_audio_interface   sound_intf;
    hdmi_audio_bit_depth   sample_depth;
    hdmi_audio_format_code audio_code;
} hdmi_ao_attr;

typedef enum {
    HDMI_TMDS_MODE_NONE,
    HDMI_TMDS_MODE_DVI,
    HDMI_TMDS_MODE_HDMI_1_4,
    HDMI_TMDS_MODE_HDMI_2_0,
    HDMI_TMDS_MODE_AUTO,
    HDMI_TMDS_MODE_HDMI_2_1,
    HDMI_TMDS_MODE_BUTT
} hdmi_tmds_mode;

typedef enum {
    HDMI_HDCP_MODE_AUTO,
    HDMI_HDCP_MODE_1_4,
    HDMI_HDCP_MODE_2_2,
    HDMI_HDCP_MODE_BUTT
} hdmi_hdcp_mode;

typedef enum {
    HDMI_DEFAULT_ACTION_NULL,
    HDMI_DEFAULT_ACTION_HDMI,
    HDMI_DEFAULT_ACTION_DVI,
    HDMI_DEFAULT_ACTION_BUTT
} hdmi_default_action;

typedef enum {
    HDMI_VIDEO_DITHER_12_10,
    HDMI_VIDEO_DITHER_12_8,
    HDMI_VIDEO_DITHER_10_8,
    HDMI_VIDEO_DITHER_DISABLE
} hdmi_video_dither;

typedef struct {
    hi_bool                 enable_hdmi;
    hi_bool                 enable_video;
    hi_bool                 enable_audio;
    hdmi_colorspace         out_color_space;
    hdmi_quantization_range out_csc_quantization;
    hdmi_deep_color         deep_color_mode;
    hi_bool                 xvycc_mode;
    hi_bool                 enable_avi_infoframe;
    hi_bool                 enable_spd_infoframe;
    hi_bool                 enable_mpeg_infoframe;
    hi_bool                 enable_aud_infoframe;
    hi_u32                  debug_flag;
    hi_bool                 hdcp_enable;
    hdmi_default_action     hdmi_action;
    hi_bool                 enable_clr_space_adapt;
    hi_bool                 enable_deep_clr_adapt;
    hi_bool                 auth_mode;
    hi_bool                 enable_drm_infoframe;
    hdmi_hdcp_mode          hdcp_mode;
    hi_bool                 enable_vsif_dolby;
} hdmi_app_attr;

typedef enum {
    HDMI_HDR_COLORIMETRY_NONE,
    HDMI_HDR_COLORIMETRY_ITU_601,
    HDMI_HDR_COLORIMETRY_ITU_709,
    HDMI_HDR_COLORIMETRY_EXTENDED,
    HDMI_HDR_EXTENDED_COLORIMETRY_XV_YCC_601,
    HDMI_HDR_EXTENDED_COLORIMETRY_XV_YCC_709,
    HDMI_HDR_EXTENDED_COLORIMETRY_S_YCC_601,
    HDMI_HDR_EXTENDED_COLORIMETRY_ADOBE_YCC_601,
    HDMI_HDR_EXTENDED_COLORIMETRY_ADOBE_RGB,
    HDMI_HDR_EXTENDED_COLORIMETRY_2020_CONST_LUMINOUS, /* BT2020 c_ycc */
    HDMI_HDR_EXTENDED_COLORIMETRY_2020_NON_CONST_LUMINOUW
} hdmi_hdr_colormetry;

typedef enum {
    HDMI_HDR_MODE_DISABLE,          /* HDR & dolby mode disable */
    HDMI_HDR_MODE_DOLBY_NORMAL,     /* dolby normal(ycbcr422-12bit) mode enable */
    HDMI_HDR_MODE_DOLBY_TUNNELING,  /* dolby tunneling(RGB-8bit) mode enable */
    HDMI_HDR_MODE_CEA_861_3,        /* HDR standard mode enable(according to <CEA-861-3.2015>) */
    HDMI_HDR_MODE_CEA_861_3_AUTHEN, /* HDR authen mode */
    HDMI_HDR_MODE_BUTT
} drv_hdmi_hdr_mode;

typedef enum {
    HDMI_HDR_USERMODE_SDR,
    HDMI_HDR_USERMODE_HDR10,
    HDMI_HDR_USERMODE_DOLBY,
    HDMI_HDR_USERMODE_BUTT
} drv_hdmi_hdr_user_mode;

typedef struct {
    drv_hdmi_hdr_mode hdr_mode; /* HDR mode */
    drv_hdmi_hdr_user_mode user_hdr_mode; /* user HDR mode */
    hdmi_eotf_type eotf_type; /* EOTF type. when hdr_mode==HI_UNF_HDMI_HDR_MODE_CEA_861_3 valid */
    /* metadata descriptor type(ID). when hdr_mode==HI_UNF_HDMI_HDR_MODE_CEA_861_3 valid */
    hdmi_static_meta_type_id metadata_id;
    /* static metadata descriptor type. when hdr_mode==HI_UNF_HDMI_HDR_MODE_CEA_861_3 valid */
    hdmi_meta_descriptor un_descriptor;
    hdmi_hdr_colormetry colorimetry; /* colorimetry. when hdr_mode==HI_UNF_HDMI_HDR_MODE_CEA_861_3 valid */
} drv_hdmi_hdr_attr;

typedef struct {
    hi_bool                 enable_hdmi;
    hi_bool                 enable_video;
    hdmi_disp_format        disp_fmt;
    hdmi_video_timing       video_timing;
    hi_u32                  pix_clk;
    hdmi_colorspace         in_color_space;
    hdmi_colorspace         out_color_space;
    hdmi_deep_color         deep_color_mode;
    hdmi_quantization_range out_csc_quantization;
    hi_bool                 enable_audio;
    hdmi_sample_rate        sample_rate;
    hdmi_audio_bit_depth    bit_depth;
    hi_bool                 enable_avi_infoframe;
    hi_bool                 enable_aud_infoframe;
    hdmi_default_action     hdmi_action;
    hi_bool                 enable_vid_mode_adapt;
    hi_bool                 enable_deep_clr_adapt;
    hi_bool                 auth_mode;
} hdmi_property;

typedef struct {
    hdmi_ao_attr  ao_attr;
    hdmi_vo_attr  vo_attr;
    hdmi_app_attr app_attr;
#ifdef HDMI_HDR_SUPPORT
    drv_hdmi_hdr_attr hdr_attr;
#endif
} hdmi_attr;

typedef enum {
    HDMI_CEC_LOGICALADD_TV,
    HDMI_CEC_LOGICALADD_RECORDDEV_1,
    HDMI_CEC_LOGICALADD_RECORDDEV_2,
    HDMI_CEC_LOGICALADD_TUNER_1,
    HDMI_CEC_LOGICALADD_PLAYDEV_1,
    HDMI_CEC_LOGICALADD_AUDIOSYSTEM,
    HDMI_CEC_LOGICALADD_TUNER_2,
    HDMI_CEC_LOGICALADD_TUNER_3,
    HDMI_CEC_LOGICALADD_PLAYDEV_2,
    HDMI_CEC_LOGICALADD_RECORDDEV_3,
    HDMI_CEC_LOGICALADD_TUNER_4,
    HDMI_CEC_LOGICALADD_PLAYDEV_3,
    HDMI_CEC_LOGICALADD_RESERVED_1,
    HDMI_CEC_LOGICALADD_RESERVED_2,
    HDMI_CEC_LOGICALADD_SPECIALUSE,
    HDMI_CEC_LOGICALADD_BROADCAST,
    HDMI_CEC_LOGICALADD_BUTT
} hdmi_cec_logical_addr;

typedef enum {
    HDMI_CEC_UICMD_SELECT,
    HDMI_CEC_UICMD_UP,
    HDMI_CEC_UICMD_DOWN,
    HDMI_CEC_UICMD_LEFT,
    HDMI_CEC_UICMD_RIGHT,
    HDMI_CEC_UICMD_RIGHT_UP,
    HDMI_CEC_UICMD_RIGHT_DOWN,
    HDMI_CEC_UICMD_LEFT_UP,
    HDMI_CEC_UICMD_LEFT_DOWN,
    HDMI_CEC_UICMD_ROOT_MENU,
    HDMI_CEC_UICMD_SETUP_MENU,
    HDMI_CEC_UICMD_CONTENTS_MENU,
    HDMI_CEC_UICMD_FAVORITE_MENU,
    HDMI_CEC_UICMD_EXIT,
    HDMI_CEC_UICMD_NUM_0 = 0x20,
    HDMI_CEC_UICMD_NUM_1,
    HDMI_CEC_UICMD_NUM_2,
    HDMI_CEC_UICMD_NUM_3,
    HDMI_CEC_UICMD_NUM_4,
    HDMI_CEC_UICMD_NUM_5,
    HDMI_CEC_UICMD_NUM_6,
    HDMI_CEC_UICMD_NUM_7,
    HDMI_CEC_UICMD_NUM_8,
    HDMI_CEC_UICMD_NUM_9,
    HDMI_CEC_UICMD_DOT,
    HDMI_CEC_UICMD_ENTER,
    HDMI_CEC_UICMD_CLEAR,
    HDMI_CEC_UICMD_NEXT_FAVORITE = 0x2F,
    HDMI_CEC_UICMD_CHANNEL_UP,
    HDMI_CEC_UICMD_CHANNEL_DOWN,
    HDMI_CEC_UICMD_PREVIOUS_CHANNEL,
    HDMI_CEC_UICMD_SOUND_SELECT,
    HDMI_CEC_UICMD_INPUT_SELECT,
    HDMI_CEC_UICMD_DISPLAY_INFORMATION,
    HDMI_CEC_UICMD_HELP,
    HDMI_CEC_UICMD_PAGE_UP,
    HDMI_CEC_UICMD_PAGE_DOWN,
    HDMI_CEC_UICMD_POWER = 0x40,
    HDMI_CEC_UICMD_VOLUME_UP,
    HDMI_CEC_UICMD_VOLUME_DOWN,
    HDMI_CEC_UICMD_MUTE,
    HDMI_CEC_UICMD_PLAY,
    HDMI_CEC_UICMD_STOP,
    HDMI_CEC_UICMD_PAUSE,
    HDMI_CEC_UICMD_RECORD,
    HDMI_CEC_UICMD_REWIND,
    HDMI_CEC_UICMD_FAST_FORWARD,
    HDMI_CEC_UICMD_EJECT,
    HDMI_CEC_UICMD_FORWARD,
    HDMI_CEC_UICMD_BACKWARD,
    HDMI_CEC_UICMD_STOP_RECORD,
    HDMI_CEC_UICMD_PAUSE_RECORD,
    HDMI_CEC_UICMD_ANGLE = 0x50,
    HDMI_CEC_UICMD_SUBPICTURE,
    HDMI_CEC_UICMD_VIDEO_ON_DEMAND,
    HDMI_CEC_UICMD_ELECTRONIC_PROGRAM_GUIDE,
    HDMI_CEC_UICMD_TIMER_PROGRAMMING,
    HDMI_CEC_UICMD_INITIAL_CONFIGURATION,
    HDMI_CEC_UICMD_PLAY_FUNCTION = 0x60,
    HDMI_CEC_UICMD_PAUSE_PLAY_FUNCTION,
    HDMI_CEC_UICMD_RECORD_FUNCTION,
    HDMI_CEC_UICMD_PAUSE_RECORD_FUNCTION,
    HDMI_CEC_UICMD_STOP_FUNCTION,
    HDMI_CEC_UICMD_MUTE_FUNCTION,
    HDMI_CEC_UICMD_RESTORE_VOLUME_FUNCTION,
    HDMI_CEC_UICMD_TUNE_FUNCTION,
    HDMI_CEC_UICMD_SELECT_MEDIA_FUNCTION,
    HDMI_CEC_UICMD_SELECT_AV_INPUT_FUNCTION,
    HDMI_CEC_UICMD_SELECT_AUDIO_INPUT_FUNCTION,
    HDMI_CEC_UICMD_POWER_TOGGLE_FUNCTION,
    HDMI_CEC_UICMD_POWER_OFF_FUNCTION,
    HDMI_CEC_UICMD_POWER_ON_FUNCTION,
    HDMI_CEC_UICMD_F1_BLUE = 0x71,
    HDMI_CEC_UICMD_F2_RED,
    HDMI_CEC_UICMD_F3_GREEN,
    HDMI_CEC_UICMD_F4_YELLOW,
    HDMI_CEC_UICMD_F5,
    HDMI_CEC_UICMD_DATA,
    HDMI_CEC_UICMD_REPORT_PHYADDR = 0x84
} hdmi_cec_ui_cmd;

typedef struct {
    hi_u8 length;
    hi_u8 data[15];
} hdmi_cec_raw_data;

typedef struct {
    hi_u8                 opcode;
    hdmi_cec_logical_addr src_addr;
    hdmi_cec_logical_addr dst_addr;
    hdmi_cec_raw_data     raw_data;
} hdmi_cec_cmd;

typedef struct {
    hi_bool cec_enable;
    hi_u8   physical_addr[4];
    hi_u32  recieve_cnt; /* receive message count */
    hi_bool net_valid;   /* logical_addr & au8_network valid flag */
    hi_u8   logical_addr;
    hi_u8   network[HDMI_CEC_LOGICALADD_BUTT];
} hdmi_cec_info;

typedef enum {
    HDMI_TRANSITION_NONE,
    HDMI_TRANSITION_BOOT_MCE,
    HDMI_TRANSITION_MCE_APP,
    HDMI_TRANSITION_BOOT_APP = 0x4
} hdmi_transition_state;

typedef enum {
    HDMI_RUN_STATE_NONE,
    HDMI_RUN_STATE_INIT,
    HDMI_RUN_STATE_OPEN,
    HDMI_RUN_STATE_START = 0x04,
    HDMI_RUN_STATE_STOP = 0x08,
    HDMI_RUN_STATE_CLOSE = 0x10,
    HDMI_RUN_STATE_DEINIT = 0x20
} hdmi_run_state;

typedef struct {
    hi_u16 length;
    hi_u8 *list;
    hi_u8 *list_start;
} hdmi_hdcp_ksv_list;

typedef enum {
    HDMI_HDCP_SINK_UNKNOWN,
    HDMI_HDCP_SINK_SUPPORT,
    HDMI_HDCP_SINK_NOSUPPORT
} hdmi_hdcp_support;

typedef struct {
    hi_bool           hdcp_is_on;
    hi_bool           hdcp_was_on;
    hi_bool           hdcp1_prep;
    hi_bool           hdcp2_prep;
    hi_bool           hdcp_auth_suc;
    hdmi_hdcp_support hdcp14_support;
    hdmi_hdcp_support hdcp22_support;
    hdmi_hdcp_mode    hdcp_mode;
} hdmi_hdcp_info;

typedef enum {
    HDMI_TX_SUPPORT_HDMI1_4    = 0x1,
    HDMI_TX_SUPPORT_HDMI2_0    = 0x2,
    HDMI_TX_SUPPORT_HDCP1_4    = 0x4,
    HDMI_TX_SUPPORT_HDCP2_2    = 0x8,
    HDMI_TX_SUPPORT_YUV420     = 0x10,
    HDMI_TX_SUPPORT_YUV420_422 = 0x20,
    HDMI_TX_SUPPORT_YUV422_444 = 0x40,
    HDMI_TX_SUPPORT_YUV444_422 = 0x80,
    HDMI_TX_SUPPORT_YUV422_420 = 0x100,
    HDMI_TX_SUPPORT_YUV2RGB    = 0x200,
    HDMI_TX_SUPPORT_RGB2YUV    = 0x400,
    HDMI_TX_SUPPORT_SCRAMBLE   = 0x800
} hdmi_tx_capability;

typedef struct {
    hi_bool tx_hdmi_14;
    hi_bool tx_hdmi_20;
    hi_bool tx_hdmi_21;
    hi_bool tx_hdcp_14;
    hi_bool tx_hdcp_22;
    hi_bool tx_rgb444;
    hi_bool tx_ycbcr444;
    hi_bool tx_ycbcr422;
    hi_bool tx_ycbcr420;
    hi_bool tx_deep_clr10_bit;
    hi_bool tx_deep_clr12_bit;
    hi_bool tx_deep_clr16_bit;
    hi_bool tx_rgb_ycbcr444;
    hi_bool tx_ycbcr444_422;
    hi_bool tx_ycbcr422_420;
    hi_bool tx_ycbcr420_422;
    hi_bool tx_ycbcr422_444;
    hi_bool tx_ycbcr444_rgb;
    hi_bool tx_scdc;
    hi_u32  tx_max_tmds_clk;
    hi_u32  tx_max_frl_rate;
} hdmi_tx_capability_data;

typedef enum {
    HDMI_CONV_STD_BT_709,
    HDMI_CONV_STD_BT_601,
    HDMI_CONV_STD_BT_2020_NON_CONST_LUMINOUS,
    HDMI_CONV_STD_BT_2020_CONST_LUMINOUS,
    HDMI_CONV_STD_BUTT
} hdmi_conversion_stb;

typedef struct {
    hdmi_video_timing       timing;
    hi_u32                  pixel_clk;
    hi_u32                  tmds_clk;
    hi_bool                 v_sync_pol;
    hi_bool                 h_sync_pol;
    hi_bool                 de_pol;
    hdmi_conversion_stb     conv_std;
    hdmi_quantization_range quantization;
    hdmi_colorspace         in_color_space;
    hdmi_colorspace         out_color_space;
    hdmi_deep_color         deep_color;
    hdmi_video_bit_depth    in_bit_depth;
    hdmi_quantization_range out_csc_quantization;
    hi_bool                 emi_enable;
} hdmi_video_config;

typedef struct {
    hi_bool              enable_audio;
    hi_bool              down_sample;
    hi_u32               tmds_clk;
    hi_u32               pixel_repeat;
    hdmi_sample_rate     sample_fs;
    hdmi_audio_ch        layout;
    hdmi_audio_interface sound_intf;
    hdmi_audio_bit_depth sample_depth;
} hdmi_audio_config;

typedef enum {
    HDMI_FRL_MODE_TMDS,
    HDMI_FRL_MODE_FRL,
    HDMI_FRL_MODE_BUTT
} hdmi_frl_mode;

typedef struct {
    hdmi_sample_rate sample_rate;
    hdmi_frl_mode    hdmi_mode;
    hi_u8            frl_rate;
    hi_u32           pixel_clk;
} hdmi_audio_ncts;

typedef struct {
    hi_bool              phy_oe;
    hi_bool              phy_power_on;
    hdmi_video_bit_depth deep_color;
} hdmi_phy_status;

typedef struct {
    hi_bool sw_emi_enable;
    hi_bool hw_emi_enable;
    hi_bool debug_enable;
} hdmi_emi_status;

typedef struct {
    hi_bool                 video_mute;
    hi_bool                 ycbcr2rgb;
    hi_bool                 rgb2ycbcr;
    hi_bool                 ycbcr444_422;
    hi_bool                 ycbcr422_420;
    hi_bool                 ycbcr420_422;
    hi_bool                 ycbcr422_444;
    hi_bool                 in420_ydemux;
    hi_bool                 out420_ydemux;
    hdmi_video_dither       dither;
    hi_bool                 v_sync_pol;
    hi_bool                 h_sync_pol;
    hi_bool                 sync_pol;
    hi_bool                 de_pol;
    hi_bool                 swap_hs_cs;
    hdmi_colorspace         in_color_space;
    hdmi_colorspace         out_color_space;
    hdmi_video_bit_depth    out_bit_depth;
    hdmi_hvsync_polarity    hv_sync_pol;
    hdmi_quantization_range out_csc_quantization;
#if defined(HDMI_SUPPORT_LOGIC_HISIV100)
    /* detect timing */
    hi_bool sync_sw_enable;
    hi_bool vsync_polarity; /* when sync_sw_enable==0,indicates hw;or ,indicates sw */
    hi_bool hsync_polarity; /* when sync_sw_enable==0,indicates hw;or ,indicates sw */
    hi_bool progressive;
    hi_u32  hsync_total;
    hi_u32  hactive_cnt;
    hi_u32  vsync_total;
    hi_u32  vactive_cnt;
#endif
} hdmi_video_status;

typedef struct {
    hi_bool hdcp14_support;
    hi_bool hdcp22_support;
} hdmi_hdcp_capability;

typedef struct {
    hi_bool              audio_mute;
    hi_bool              audio_enable;
    hi_bool              down_sample;
    hdmi_sample_rate     sample_fs;
    hdmi_audio_ch        layout;
    hdmi_audio_interface sound_intf;
    hdmi_audio_bit_depth sample_depth;
    hi_u32               ref_n;
    hi_u32               reg_n;
    hi_u32               ref_cts;
    hi_u32               reg_cts;
} hdmi_audio_status;

typedef struct {
    hi_bool hotplug;
    hi_bool rsen;
    hi_bool avmute;
    hdmi_tmds_mode tmds_mode;
} hdmi_common_status;

typedef struct {
    hi_bool source_scramble_on;
    hi_bool sink_scramble_on;
    hi_u8   tmds_bit_clk_ratio;
#if defined(HDMI_SUPPORT_LOGIC_HISIV100)
    hi_bool sink_read_quest;
    /* in unit of ms.for [0,200], force to default 200; or, set the value cfg(>200). */
    hi_u32 scramble_timeout;
    /* in unit of ms, range[20,200). for [0,20] or >=200, force to default 20; or, set the value cfg[20,200). */
    hi_u32 scramble_interval;
#endif
} hdmi_scdc_status;

typedef struct {
    hi_bool avi_enable;
    hi_bool audio_enable;
    hi_bool vsif_enable;
    hi_bool spd_enable;
    hi_bool mpeg_enable;
    hi_bool gbd_enable;
    hi_u8   avi[HDMI_INFO_FRAME_MAX_SIZE];
    hi_u8   audio[HDMI_INFO_FRAME_MAX_SIZE];
    hi_u8   vsif[HDMI_INFO_FRAME_MAX_SIZE];
    hi_u8   spd[HDMI_INFO_FRAME_MAX_SIZE];
    hi_u8   mpeg[HDMI_INFO_FRAME_MAX_SIZE];
    hi_u8   gdb[HDMI_INFO_FRAME_MAX_SIZE];
    hi_bool drm_enable;
    hi_u8   drm[HDMI_INFO_FRAME_MAX_SIZE];
} hdmi_infoframe_status;

typedef struct {
    hi_bool hdcp22_enable;
    hi_bool hdcp14_enable;
    hi_bool repeater_on;
    hi_u8   bksv[5];
    hi_u8   aksv[5];
    hi_u8   hdcp_status;
} hdmi_hdcp_status;

typedef enum {
    FRL_WORK_MODE_NONE,
    FRL_WORK_MODE_3L3G,
    FRL_WORK_MODE_3L6G,
    FRL_WORK_MODE_4L6G,
    FRL_WORK_MODE_4L8G,
    FRL_WORK_MODE_4L10G,
    FRL_WORK_MODE_4L12G,
    FRL_WORK_MODE_BUTT
} hdmi_work_mode;

typedef struct {
    hi_bool frl_start;
    hi_bool work_en;
    hdmi_work_mode work_mode;
} hdmi_frl_status;

typedef enum {
    FRL_TXFFE_MODE_0,
    FRL_TXFFE_MODE_1,
    FRL_TXFFE_MODE_2,
    FRL_TXFFE_MODE_3,
    FRL_TXFFE_MODE_BUTT
} hdmi_txfff_mode;

typedef struct {
    hdmi_common_status    common_status;
    hdmi_phy_status       phy_status;
    hdmi_video_status     video_status;
    hdmi_audio_status     audio_status;
    hdmi_infoframe_status info_frame_status;
    hdmi_hdcp_status      hdcp_status;
    hdmi_hwspec           phy_hwspec;
#ifdef HDMI_FRL_SUPPORT
    hdmi_frl_status       frl_status;
#endif
} hdmi_hardware_status;

typedef struct {
    hi_u32  max_tmds_character_rate;
    hi_bool scdc_present;
    hi_bool rr_capable;
    hi_bool lte340_mcsc_scramble;
    hi_bool _3d_osd_disparity;
    hi_bool dual_view;
    hi_bool independent_view;
    hi_bool dc30bit420;
    hi_bool dc36bit420;
    hi_bool dc48bit420;
    hi_bool scdc_enable;
} hdmi_scdc_config;

typedef struct {
    hi_u32  mute_delay;       /* delay for avmute */
    hi_u32  fmt_delay;        /* delay for setformat */
    hi_bool force_fmt_delay;  /* force setformat delay mode */
    hi_bool force_mute_delay; /* force avmute delay mode */
} hdmi_delay;

typedef enum {
    HDMI_VIDEO_UNKNOWN,
    HDMI_VIDEO_PROGRESSIVE,
    HDMI_VIDEO_INTERLACE,
    HDMI_VIDEO_BUTT
} hdmi_video_format_type;

typedef struct {
    hdmi_video_code_vic video_code;
    hi_u32 pixclk;
    hi_u32 rate;
    hi_u32 hactive;
    hi_u32 vactive;
    hi_u32 hblank;
    hi_u32 vblank;
    hi_u32 hfront;
    hi_u32 hsync;
    hi_u32 hback;
    hi_u32 vfront;
    hi_u32 vsync;
    hi_u32 vback;
    hdmi_picture_aspect    aspect_ratio;
    hdmi_video_timing      timing;
    hdmi_video_format_type pi_type;
    hi_char *fmt_str;
} hdmi_video_def;

typedef struct {
    hdmi_vsif_vic          hdmi_vic;
    hdmi_video_code_vic    equivalent_video_code;  // real vic
    hi_u32                 pixclk;
    hi_u32                 rate;
    hi_u32                 hactive;
    hi_u32                 vactive;
    hdmi_picture_aspect    aspect_ratio;
    hdmi_video_timing      timing;
    hdmi_video_format_type pi_type;
    hi_char *fmt_str;
} hdmi_video_4k_def;

typedef struct {
    hi_u32 attach_in_time;
    hi_u32 attach_out_time;
    hi_u32 de_attach_in_time;
    hi_u32 de_attach_out_time;
    hi_u32 preformat_in_time;
    hi_u32 preformat_out_time;
    hi_u32 setformat_in_time;
    hi_u32 setformat_out_time;
    hi_u32 suspend_in_time;
    hi_u32 suspend_out_time;
    hi_u32 resume_in_time;
    hi_u32 resume_out_time;
} hdmi_intf_status;

typedef struct {
    hi_bool black_enable;
    hi_u8   in_color_space;
    hi_u8   in_bit_depth;
    hi_u8   in_quantization;
} hdmi_black_frame_info;

typedef struct {
    hi_u32 stop_delay;
    hdmi_intf_status intf_status;
} hdmi_debug;

typedef enum {
    HDMI_TIMER_ZERO_DRMIF,
    HDMI_TIMER_SDR_TO_HDR10,
    HDMI_TIMER_TYPE_BUTT
} hdmi_timer_type;

typedef struct {
    hdmi_timer_type timer_type;
    hi_bool timer_start;
    hi_u32  time;
} hdmi_timer_config;

typedef enum {
    HDMI_HDR_DEBUG_MODE_NONE,   /* HDR mode change, SDR->HDR10 don't change anything */
    HDMI_HDR_DEBUG_MODE_OE,     /* HDR mode change, SDR->HDR10 output disable and enable */
    HDMI_HDR_DEBUG_MODE_AVMUTE, /* HDR mode change, SDR->HDR10 send avmute */
    HDMI_HDR_DEBUG_MODE_BUTT
} hdmi_hdr_debug_mode;

typedef struct {
    /* HDR debug mode. to solve the problem of some TV can not switch smoothly from non-HDR10 to HDR10. */
    hdmi_hdr_debug_mode hdr_debug_mode;
    /* should send zero DRM infoframe when the HDR mode from HDR10 to non-HDR10. */
    hdmi_timer_config zero_drm_if_timer;
    hdmi_timer_config hdr_mode_chn_timer; /* HDR debug timer. */
} drv_hdmi_hdr_info;

typedef struct {
    hi_bool mute_pkg_en;
    hi_bool mute_set;
    hi_bool mute_clr;
    hi_bool mute_rpt_en;
    hi_u32  rpt_cnt;
} hdmi_avmute_cfg;

typedef enum {
    HDMI_FRL_SCDC_TYPE_SINK_VERSION,
    HDMI_FRL_SCDC_TYPE_SOURCE_VERSION,
    HDMI_FRL_SCDC_TYPE_UPDATE_FLAGS,
    HDMI_FRL_SCDC_TYPE_STATUS_FLAGS,
    HDMI_FRL_SCDC_TYPE_CONFIG,
    HDMI_FRL_SCDC_TYPE_TXFFE_REQ,
    HDMI_FRL_FLAGS_TYPE_BUTT
} hdmi_frl_scdc_type;

typedef enum {
    HDMI_FRL_TRAIN_PATTERN_NONE,
    HDMI_FRL_TRAIN_PATTERN_LP1,
    HDMI_FRL_TRAIN_PATTERN_LP2,
    HDMI_FRL_TRAIN_PATTERN_LP3,
    HDMI_FRL_TRAIN_PATTERN_LP4,
    HDMI_FRL_TRAIN_PATTERN_LP5,
    HDMI_FRL_TRAIN_PATTERN_LP6,
    HDMI_FRL_TRAIN_PATTERN_LP7,
    HDMI_FRL_TRAIN_PATTERN_LP8,
    HDMI_FRL_TRAIN_PATTERN_RESERVED,
    HDMI_FRL_TRAIN_PATTERN_0E = 0xE,
    HDMI_FRL_TRAIN_PATTERN_0F = 0xF,
    HDMI_FRL_TRAIN_PATTERN_BUTT
} hdmi_frl_train_pattern;

typedef enum {
    HDMI_FRL_TRAIN_NONE,
    HDMI_FRL_TRAIN_FAIL,
    HDMI_FRL_TRAIN_SUCCESS,
    HDMI_FRL_TRAIN_BUSY,
    HDMI_FRL_TRAIN_BUTT
} hdmi_frl_train_status;

typedef enum {
    HDMI_FRL_TRAIN_FAIL_NORMAL,
    HDMI_FRL_TRAIN_FAIL_FLTTIMEOUT,
    HDMI_FRL_TRAIN_FAIL_FLTSTEPTIMEOUT,
    HDMI_FRL_TRAIN_FAIL_RATECHANGE,
    HDMI_FRL_TRAIN_FAIL_FFECHANGE,
    HDMI_FRL_TRAIN_FAIL_BUTT
} hdmi_frl_train_fail_code;

typedef enum {
    HDMI_FRL_TRAIN_SEL_SW,
    HDMI_FRL_TRAIN_SEL_HW,
    HDMI_FRL_TRAIN_SEL_BUTT
} hdmi_frl_train_sel;

typedef enum {
    FRL_DEBUG_RATE,
    FRL_DEBUG_SW_TRAIN_SEL,
    FRL_DEBUG_LTP_PATTERN,
    FRL_DEBUG_SELECT_CHANNEL,
    FRL_DEBUG_LTS3_INTERVAL,
    FRL_DEBUG_LTS3_TIMEOUT,
    FRL_DEBUG_TRAINING_BREAK,
    FRL_DEBUG_LM_TABLE_GET,
    FRL_DEBUG_CTRL_TYPE_CONFIG,
    FRL_DEBUG_BUTT
} frl_debug_cmd;

typedef enum {
    FRL_SW_TRAIN_DELAY,
    FRL_SW_TRAIN_TIMER,
    FRL_SW_TRAIN_BUTT
} frl_sw_train_mode;

typedef enum {
    FRL_CHL_SEL_NORMAL,
    FRL_CHL_SEL_RX_TMDS,
    FRL_CHL_SEL_BUTT
} frl_channel_sel;

typedef struct {
    hdmi_frl_train_status    frl_train_status;
    hdmi_frl_train_pattern   train_pattern[HDMI_FRL_LANE_MAX_NUM];
    hdmi_frl_train_fail_code train_fail_res;
} hdmi_frl_train;

typedef enum {
    HDMI_FRL_MACH_MODE_STEP,
    HDMI_FRL_MACH_MODE_TIMEOUT,
    HDMI_FRL_MACH_MODE_BUTT
} hdmi_frl_mach_mode;

typedef struct {
    hi_bool            frl_no_timeout;
    hi_u8              frl_rate;
    hi_u8              ffe_levels;
    hi_u32             train_timeout;
    hdmi_frl_mach_mode mach_mode;
    frl_sw_train_mode  sw_train_mode;
    hi_u8              ctl_type_config;
} hdmi_frl_train_config;

typedef struct {
    frl_debug_cmd     debug_cmd;
    hi_u8             rate;
    hi_u8             ltp;
    hi_u8             lane_idx;
    hi_u8             training_break;
    frl_sw_train_mode sw_train_mode;
    frl_channel_sel   channel_sel;
    hi_u32            lts3_interval;
    hi_u32            lts3_timeout;
    hi_u8             crtl_type_config;
    hi_u8             avi_send_by_gen5;
} frl_debug;

typedef enum {
    SCDC_CMD_SET_SOURCE_VER,
    SCDC_CMD_GET_SOURCE_VER,
    SCDC_CMD_GET_SINK_VER,
    SCDC_CMD_SET_FLT_UPDATE,
    SCDC_CMD_GET_FLT_UPDATE,
    SCDC_CMD_SET_FLT_UPDATE_TRIM,
    SCDC_CMD_GET_FLT_UPDATE_TRIM,
    SCDC_CMD_SET_FRL_START,
    SCDC_CMD_GET_FRL_START,
    SCDC_CMD_SET_CONFIG1,
    SCDC_CMD_GET_CONFIG1,
    SCDC_CMD_GET_TEST_CONFIG,
    SCDC_CMD_GET_FLT_READY,
    SCDC_CMD_GET_LTP_REQ,
    SCDC_CMD_BUTT
} scdc_cmd;

typedef struct {
    hi_u8 frl_rate;
    hi_u8 ffe_levels;
} scdc_config1;

typedef struct {
    hi_bool pre_shoot_only;
    hi_bool de_emphasis_only;
    hi_bool no_ffe;
    hi_bool flt_no_timeout;
    hi_bool dsc_frl_max;
    hi_bool frl_max;
} scdc_test_config;

typedef struct {
    hi_u8 ln0_ltp;
    hi_u8 ln1_ltp;
    hi_u8 ln2_ltp;
    hi_u8 ln3_ltp;
} scdc_ltp_req;

typedef enum {
    HDMI_PHY_MODE_CFG_TMDS,
    HDMI_PHY_MODE_CFG_FRL,
    HDMI_PHY_MODE_CFG_TXFFE
} hdmi_phy_mode_cfg;

typedef enum {
    HDMI_TRACE_LEN_0, /* 1.0 inch */
    HDMI_TRACE_LEN_1, /* 1.5 inch */
    HDMI_TRACE_LEN_2, /* 2.0 inch */
    HDMI_TRACE_LEN_3, /* 2.5 inch */
    HDMI_TRACE_LEN_4, /* 3.0 inch */
    HDMI_TRACE_LEN_5, /* 3.5 inch */
    HDMI_TRACE_LEN_6, /* 4.0 inch */
    HDMI_TRACE_LEN_7, /* 4.5 inch */
    HDMI_TRACE_LEN_8, /* 5.0 inch */
    HDMI_TRACE_BUTT
} hdmi_trace_len;

typedef struct {
    hdmi_trace_len trace_len;
} hdmi_mode_param;

typedef struct {
    hi_u32            pixel_clk;
    hi_u32            tmds_clk;      /* TMDS colck,in k_hz */
    hi_bool           emi_enable;
    hdmi_deep_color   deep_color;    /* deep color(color depth) */
    hdmi_phy_mode_cfg mode_cfg;      /* TMDS/FRL/tx_ffe */
    hdmi_trace_len    trace_len;
#ifdef HDMI_FRL_SUPPORT
    hdmi_colorspace   color_space;
    hdmi_work_mode    rate;          /* lane and rate */
    hdmi_txfff_mode   aen_tx_ffe[4]; /* tx_ffe */
#endif
} hdmi_phy_cfg;

hdmi_video_code_vic drv_hdmi_vic_search(hdmi_video_timing, hdmi_picture_aspect, hi_bool);

hi_void hdmi_reg_write(volatile hi_void *reg_addr, hi_u32 value);

hi_u32 hdmi_reg_read(volatile hi_void *reg_addr);

hi_void drv_hdmi_milli_delay(hi_u32 m_sec);

hi_u32 drv_hdmi_vic_to_index(hi_u32 vic);

hdmi_video_timing drv_hdmi_video_timing_get(hdmi_video_code_vic vic, hdmi_picture_aspect aspect);

hdmi_video_timing drv_hdmi_vsif_video_timing_get(hdmi_vsif_vic vic);

hdmi_video_4k_def *drv_hdmi_video_codes_4k_get(hi_u32 cnt);

hdmi_video_def *drv_hdmi_comm_format_param_get(hdmi_video_code_vic vic);

#ifdef HDMI_FRL_SUPPORT
hdmi_video_def *drv_hdmi_vesa_format_param_get(hdmi_video_timing timing);
#endif

#endif  /* __DRV_HDMI_COMMON_H_ */


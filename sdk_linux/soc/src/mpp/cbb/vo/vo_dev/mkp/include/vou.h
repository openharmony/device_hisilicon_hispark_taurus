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

#ifndef __VOU_VOU_H__
#define __VOU_VOU_H__

#include "hi_osal.h"
#include "hi_defines.h"
#include "hi_debug_adapt.h"
#include "vou_drv.h"
#include "hi_common_adapt.h"
#include "hi_math_adapt.h"
#include "vou_dev_exp.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VOU_STATE_STARTED                   0
#define VOU_STATE_STOPPING                  1
#define VOU_STATE_STOPPED                   2

#define MDDRC_ZONE_MAX_NUM                  32
#define VHD_MAX_CHN_NUM                     1 /* max VHD chn num */
#define VO_MAX_GFX_LAYER_PER_DEV            1
#define VO_MIN_CHN_LINE                     3
#define VO_DEF_CHN_BUF_LEN                  8
#define VO_DEF_DISP_BUF_LEN                 5
#define VO_DEF_VIRT_BUF_LEN                 3
#define VO_DEF_WBC_DEPTH_LEN                8

/* WBC channel id */
#define VO_WBC_CHN_ID                       (VO_MAX_CHN_NUM + 1)
#define VO_DEFAULT_CHN                      (-1) /* use vo buffer as pip buffer */

#define VO_INVALID_DEV                      (-1) /* invalid device tag */

#define VO_CHN_MAX_BLK                      4 /* channel buffer node limit */
#define VO_CHN_MAX_NODE                     10 /* channel  free list node limit */
#define VO_HD_DISP_MIN_BLK                  4 /* display buffer node limit */
#define VO_SD_DISP_MIN_BLK                  3 /* display buffer node limit */
#define VO_SD_DISP_MIN_BLK_SBM              2 /* display buffer node limit save buff mode */

#define VO_DISP_MAX_BLK                     15 /* display buffer node limit */

#define VO_PRIORITY_MAX                     VO_MAX_CHN_NUM /* priority max value on device */
#define VO_CHN_PRIORITY_MAX                 VO_MAX_CHN_NUM

#define VO_HD_PIP_MAX_SIZE                  2073600 /* 1920 * 1080 */

#define VO_HD_DISP_THD                      3

#define PTS_SPAN_TOLERATION                 10000000 /* tolerance of error frame PTS, set 10s */

#define VO_PIP_MIN_WIDTH                    32 /* minimize pip width */
#define VO_PIP_MIN_HEIGHT                   32 /* minimize pip height */

#define VO_BKGRD_COLOR_RED                  0xFF0000 /* red background color */
#define VO_BKGRD_COLOR_GREEN                0x00FF00 /* green background color */
#define VO_BKGRD_COLOR_BLUE                 0x0000FF /* blue background color */
#define VO_BKGRD_COLOR_YELLOW               ((VO_BKGRD_COLOR_RED) + (VO_BKGRD_COLOR_GREEN))
#define VO_BKGRD_COLOR_CYAN                 ((VO_BKGRD_COLOR_GREEN) + (VO_BKGRD_COLOR_BLUE))
#define VO_BKGRD_COLOR_MAGENTA              ((VO_BKGRD_COLOR_BLUE) + (VO_BKGRD_COLOR_RED))

#define VO_BKGRD_COLOR_WHITE                0xFFFFFF /* white background color */
#define VO_BKGRD_COLOR_BLACK                0x000000 /* black back groud color */

#define VO_ALPHA_TRANSPARENT                0x0 /* transparent alpha */
#define VO_ALPHA_OPACITY                    0xFF /* opacity alpha */

#define VO_CHN_VI_BUF_LINE                  VO_MIN_CHN_LINE /* channel buffer waterline of preview */

#define VO_TIMER_EXPIRES                    10 /* virtual device timer expires (ms) */

#define VO_MAX_SYNC_WIDTH                   5000
#define VO_MAX_SYNC_HEIGHT                  5000

#define VOU_MAX_OSD_NUM                     OVERLAYEX_MAX_NUM_VO
#define VOU_MAX_COVER_NUM                   COVEREX_MAX_NUM_VO
#define VOU_LUMA_STAT_NUM                   64

#define VOU_REGION_LUMA_BUFFER_SIZE         (VOU_LUMA_STAT_NUM * LUMA_PHY_ALIGN)

/* the align format of memory assign */
#define VO_W_ALIGN_16BYTE                   16
#define vo_align(x, a) (((x) + (a)-1) & (~((a) - 1)))
#define VO_MAX_LINE_FOR_ALIGN               (64 * VO_W_ALIGN_16BYTE)

#define VO_HD_MIN_VTTH_WATERLINE            240
#define VO_HD_MAX_VTTH_WATERLINE            8191
#define VO_SD_MIN_VTTH_WATERLINE            100
#define VO_SD_MAX_VTTH_WATERLINE            8191
#define VO_HD_MIN_VTTH_WATERLINE2           2
#define VO_SD_MIN_VTTH_WATERLINE2           2

#define VO_MAX_MUX_NUM                      3
#define VO_MAX_INTFNAME_LEN                 10
#define VO_MAX_INTF_TYPE_NUM                16

#define VO_WBC_OUT_MAX_WIDTH                4096
#define VO_WBC_OUT_MAX_HEIGHT               4096

#define VO_RECT_ALIGN                       2
#define VO_RGN_LUMA_ALIGN                   2
#define VO_CHN_DISP_POS_ALIGN               2

#define VO_MAX_ALPHA_VAL                    255

#define VO_CHN_QRY_WAIT_TIMEOUT             4000
#define VO_WBC_BG_BLACK_WAIT_TIMEOUT        4000
#define VO_DISABLE_LAYER_WAIT_TIMEOUT       1000
#define VO_CLEAR_BUF_WAIT_TIMEOUT           1000
#define VO_DISABLE_WBC_TIMEOUT              500000
#define VO_TIME_ONE_SECOND_UNIT_MICROSECOND         1000000
#define VO_TIME_TEN_SECONDS_UNIT_MICROSECOND        10000000ul

typedef struct {
    hi_s32 layer_id;
} vo_wait_layer;

typedef struct {
    hi_s32 layer_id;
    hi_s32 chn_id;
    hi_s32 last_node_num;
    hi_s32 unique_id;
} vo_wait_chn;

typedef struct {
    hi_s32 wbc_id;
} vo_wait_wbc;

/* typical value of display frequency */
typedef enum {
    VO_DISP_FREQ_VGA75 = 75,
    VO_DISP_FREQ_VGA = 60,
    VO_DISP_FREQ_VGA30 = 30,
    VO_DISP_FREQ_VGA25 = 25,
    VO_DISP_FREQ_24P = 24,
    VO_DISP_FREQ_50P = 50,
    VO_DISP_FREQ_PAL = 25,
    VO_DISP_FREQ_NTSC = 30,
    VO_DISP_FREQ_BUTT
} vo_disp_freq;

/* typical value of video size */
typedef enum {
    VO_DISP_1080_WIDTH = 1920,
    VO_DISP_1080_HEIGHT = 1080,
    VO_DISP_720P_WIDTH = 1280,
    VO_DISP_720P_HEIGHT = 720,
    VO_DISP_D1_WIDTH = 720,
    VO_DISP_D1P_HEIGH = 576,
    VO_DISP_D1N_HEIGH = 480,
    VO_DISP_VGA_1024 = 1024,
    VO_DISP_VGA_768 = 768,
    VO_DISP_VGA_800 = 800,
    VO_DISP_VGA_600 = 600,
    VO_DISP_VIRT_WIDTH = 5000,
    VO_DISP_VIRT_HEIGHT = 5000,
    VO_DISP_8K_WIDTH = 8192,
    VO_DISP_8K_HEIGHT = 4320,
    VO_DISP_BUTT
} vo_disp_size;

/* define vo receive frame info */
typedef struct {
    hi_bool suc;
    hi_mod_id mod_id;
    hi_bool block;
    hi_video_frame_info *video_frame[2];
    hi_u64 node_index;
} vo_recv_frame_info;

typedef struct {
    hi_u32 luma; /* luminance: 0 ~ 100 default: 50 */
    hi_u32 contrast; /* contrast: 0 ~ 100 default: 50 */
    hi_u32 hue; /* hue: 0 ~ 100 default: 50 */
    hi_u32 satuature; /* satuature: 0 ~ 100 default: 50 */
} vo_csc_val;

typedef struct {
    hi_u64 scale_pts; /* scale pts which will be display next time */
    osal_timer_t timer; /* virtual device timer */
} vou_virt_dev;

typedef struct {
    hi_bool load_detect_enable;
    hi_u32 int_cur_count;
    hi_u32 int_max_count;
} vou_load_detect_info;

typedef struct {
    hi_u32 int_rate;
    hi_u32 int_count;
    hi_u64 last_rate_time;

    hi_u32 int_tmp;
    hi_u32 int_time_per_min_tmp;
    hi_u32 int_time_per_min;

    hi_u32 int_time;
    hi_u32 max_int_time;

    hi_u64 last_int_time;
    hi_u32 int_gap_time;
    hi_u32 max_int_gap_time;

    hi_u64 last_time;
} vo_debug_info;

typedef struct {
    hi_bool vo_enable;
    hi_bool config;

    hi_bool wbc_bind;
    hi_vo_wbc wbc;

    hi_vo_pub_attr vou_attr;

    hi_u32 max_width;
    hi_u32 max_height;

    hi_u32 full_frame_rate;

    vo_int_mode vo_int_type;

    hi_u32 time_ref;

    hi_u64 coef_addr_phys;
    hi_void *coef_addr_virt;

    vou_load_detect_info load_detect_info;
    vo_debug_info debug_info;

    vo_dither_info dither_info;

    hi_vo_user_intfsync_info vo_user_sync_info;
} vo_dev_info;

typedef struct {
    hi_vo_intf_sync index;
    hi_char *name;
    hi_u32 width;
    hi_u32 height;
    hi_u32 frame_rate;
} vo_sync_basic_info;

__inline static hi_s32 vo_init_lock(osal_spinlock_t *lock)
{
    osal_spin_lock_init(lock);
    return HI_SUCCESS;
}

__inline static hi_s32 vo_deinit_lock(osal_spinlock_t *lock)
{
    osal_spin_lock_destroy(lock);
    return HI_SUCCESS;
}

__inline static hi_s32 vo_spin_lock(osal_spinlock_t *lock, hi_u32 *flag)
{
    unsigned long tmp_flag = (unsigned long)(*flag);
    osal_spin_lock_irqsave(lock, &tmp_flag);
    *flag = (hi_u32)tmp_flag;
    return HI_SUCCESS;
}

__inline static hi_s32 vo_spin_unlock(osal_spinlock_t *lock, hi_u32 *flag)
{
    unsigned long tmp_flag = (unsigned long)(*flag);
    osal_spin_unlock_irqrestore(lock, &tmp_flag);
    *flag = (hi_u32)tmp_flag;
    return HI_SUCCESS;
}

hi_u32 vo_get_vo_state(hi_void);
osal_spinlock_t *vo_get_dev_lock(hi_void);
osal_spinlock_t *vo_get_list_lock(hi_void);
osal_spinlock_t *vo_get_wbc_lock(hi_void);
struct osal_semaphore *vo_get_chn_sem(hi_void);
struct osal_semaphore *vo_get_dev_sem(hi_void);
hi_vo_mod_param *vo_get_vo_mod_param(hi_void);
vo_dev_info *vo_get_dev_ctx(hi_vo_dev vo_dev);
vou_virt_dev *vo_get_virt_dev_ctx(hi_vo_dev vo_dev);
hi_u32 vo_get_transparent_transmit(hi_void);
vou_export_callback_s *vo_get_exp_callback(hi_void);
vo_sync_basic_info *vo_get_sync_basic_info(hi_void);

#define vo_down_sem_return()                            \
    do {                                         \
        struct osal_semaphore *dev_sem_;         \
        dev_sem_ = vo_get_dev_sem();             \
        if (osal_down_interruptible(dev_sem_))   \
            return -ERESTARTSYS;                 \
    } while (0)

#define vo_up_sem()                              \
    do {                                         \
        struct osal_semaphore *dev_sem_;         \
        dev_sem_ = vo_get_dev_sem();             \
        osal_up(dev_sem_);                       \
    } while (0)

#define vo_down_chn_sem_return()                        \
    do {                                         \
        struct osal_semaphore *chn_sem_;         \
        chn_sem_ = vo_get_chn_sem();             \
        if (osal_down_interruptible(chn_sem_))   \
            return -ERESTARTSYS;                 \
    } while (0)

#define vo_up_chn_sem()                          \
    do {                                         \
        struct osal_semaphore *chn_sem_;         \
        chn_sem_ = vo_get_chn_sem();             \
        osal_up(chn_sem_);                       \
    } while (0)

#define vo_dev_spin_lock(flags)                        \
    do {                                               \
        osal_spinlock_t * lock_ = vo_get_dev_lock();   \
        vo_spin_lock(lock_, flags);                    \
    } while (0)

#define vo_dev_spin_unlock(flags)                      \
    do {                                               \
        osal_spinlock_t * lock_ = vo_get_dev_lock();   \
        vo_spin_unlock(lock_, flags);                  \
    } while (0)

#define vo_list_spin_lock(flags)                       \
    do {                                               \
        osal_spinlock_t * lock_ = vo_get_list_lock();  \
        vo_spin_lock(lock_, flags);                    \
    } while (0)

#define vo_list_spin_unlock(flags)                     \
    do {                                               \
        osal_spinlock_t * lock_ = vo_get_list_lock();  \
        vo_spin_unlock(lock_, flags);                  \
    } while (0)

#ifdef CONFIG_HI_VO_WBC
#define vo_wbc_spin_lock(flags)                        \
    do {                                               \
        osal_spinlock_t * lock_ = vo_get_wbc_lock();   \
        vo_spin_lock(lock_, flags);                    \
    } while (0)

#define vo_wbc_spin_unlock(flags)                      \
    do {                                               \
        osal_spinlock_t * lock_ = vo_get_wbc_lock();   \
        vo_spin_unlock(lock_, flags);                  \
    } while (0)
#endif

#ifdef CONFIG_HI_VO_BATCH
hi_s32 vou_trave_batch_cmd(hi_vo_dev dev, hi_bool exc, hi_u32 *list_flag);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifdef __VOU_VOU_H__ */

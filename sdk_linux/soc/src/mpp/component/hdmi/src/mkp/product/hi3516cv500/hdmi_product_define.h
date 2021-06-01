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
#ifndef __HDMI_PRODUCT_DEFINE_H__
#define __HDMI_PRODUCT_DEFINE_H__

#ifndef HDMI_LITEOS_SUPPORT
#include <linux/kthread.h>
#include <linux/fs.h>
#else
#include <pthread.h>
#endif
#include "dev_ext.h"
#include "mod_ext.h"
#include "hi_osal.h"
#include "hi_type.h"
#include "drv_hdmi_common.h"

#define HDMI_CHIP_HI3516CV500      0x3516c500
#define HDMI_CTRL_BASE_ADDR        0x11420000
#define HDMI_ADDR_BASE_IO_CFG      0x114f0000
#define HDMI_ADDR_CRG              0x12010110 /* base addr offset is 0x110  */
#define BASE_ADDR_TX_DPHY          0x11400000
#define HDMI_COLOR_BAR_MASK        0x70000000
#define HDMI_COLOR_BAR_UPDATE_MASK 0x00000001
#define HDMI_COLOR_BAR_BASE        0x1144d000 // VDP
#define HDMI_TX_BASE_ADDR          0x11400000
#define HDMI_TX_REG_LEN            0x40000
#define HDMI_TX_BASE_ADDR_CTRL     0x20000
#define HDMI_TX_BASE_ADDR_VIDEO    0x20800
#define HDMI_TX_BASE_ADDR_AUDIO    0x21000
#define HDMI_TX_BASE_ADDR_HDMITX   0x21800
#define HDMI_TX_BASE_ADDR_HDCP1X   0x22000
#define HDMI_TX_BASE_ADDR_AON      0x24000
#define HDMI_TX_BASE_ADDR_CEC      0x24800
#define HDMI_TX_BASE_ADDR_MCU_PRAM 0x30000
#define HDMI_TX_BASE_ADDR_MCU_DRAM 0x38000
#define HDMI_TX_BASE_ADDR_MCU      0x3c000
#define HDMI_TX_BASE_ADDR_DPHY     0x0
/* HDMI PHY */
#define HDMI_TX_PHY_ADDR         (HDMI_TX_BASE_ADDR + HDMI_TX_BASE_ADDR_DPHY)
#define BASE_ADDR_TX_CTRL_REG    (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_CTRL)
#define BASE_ADDR_VIDEO_PATH_REG (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_VIDEO)
#define BASE_ADDR_AUDIO_PATH_REG (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_AUDIO)
#define BASE_ADDR_TX_HDMI_REG    (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_HDMITX)
#define BASE_ADDR_TX_HDCP_REG    (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_HDCP1X)
#define BASE_ADDR_TX_AON_REG     (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_AON)
#define BASE_ADDR_TX_CEC_REG     (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_CEC)
#define BASE_ADDR_MCU_CPU_REG    (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_MCU)
#define BASE_ADDR_MCU_PARAM      (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_MCU_PRAM)
#define BASE_ADDR_MCU_DRAM       (HDMI_TX_BASE_ADDR | HDMI_TX_BASE_ADDR_MCU_DRAM)
#ifdef HI_ADVCA_FUNCTION_RELEASE
/* PROC disable */
#define CONFIG_HDMI_PROC_DISABLE
/* LOG disable */
#define CONFIG_HDMI_DEBUG_DISABLE
#endif
#define hdmi_mutex_unlock(a) osal_up((osal_semaphore_t *)&(a))
#define HDMI_FILE_MODE       0755
#define VERSION_STRING      ("[HDMI] Version: [" MPP_VERSION "], Build Time["__DATE__", "__TIME__"]")
#ifndef HDMI_LITEOS_SUPPORT
#define hdmi_get_current_id() (get_current()->tgid)
#else
#define hdmi_get_current_id() (pthread_self() + 1)
#endif

#define hi_err_hdmi(fmt...)                                                                      \
    do {                                                                                         \
        HI_ERR_TRACE(HI_ID_HDMI, "[Func]:%s [Line]:%u [Info]:", __FUNCTION__, (hi_u32)__LINE__); \
        HI_ERR_TRACE(HI_ID_HDMI, ##fmt);                                                         \
    } while (0)

#define hi_warn_hdmi(fmt...)                                                                      \
    do {                                                                                          \
        HI_WARN_TRACE(HI_ID_HDMI, "[Func]:%s [Line]:%u [Info]:", __FUNCTION__, (hi_u32)__LINE__); \
        HI_WARN_TRACE(HI_ID_HDMI, ##fmt);                                                         \
    } while (0)

#define hi_info_hdmi(fmt...)                                                                      \
    do {                                                                                          \
        HI_INFO_TRACE(HI_ID_HDMI, "[Func]:%s [Line]:%u [Info]:", __FUNCTION__, (hi_u32)__LINE__); \
        HI_INFO_TRACE(HI_ID_HDMI, ##fmt);                                                         \
    } while (0)

#define hi_fatal_hdmi(fmt...)            \
    do {                                 \
        HI_DEBUG_TRACE(HI_ID_HDMI, fmt); \
    } while (0)

#define hdmi_mutex_lock(a)                                   \
    do {                                                     \
        hi_s32 result = osal_down((osal_semaphore_t *)&(a)); \
        if (result != 0) {                                   \
            hdmi_err("osal_down failed! ret=%d\n", result);  \
        }                                                    \
    } while (0)

#ifdef HDMI_LOG_SUPPORT
#ifndef HI_ADVCA_FUNCTION_RELEASE
#define hdmi_printk(fmt, args...) osal_printk(fmt, ##args)
#else
#define hdmi_printk(fmt, args...)
#endif
#else
#define hdmi_printk(fmt, args...)
#endif

#ifdef CONFIG_HDMI_DEBUG_DISABLE
#define edid_info(fmt...)
#define edid_warn(fmt...)
#define edid_err(fmt...)
#define edid_fatal(fmt...)
#define hdmi_info(fmt...)
#define hdmi_warn(fmt...)
#define hdmi_err(fmt...)
#define hdmi_fatal(fmt...)
#elif defined(HDMI_LOG_SUPPORT)
#define edid_info(fmt...)
#define edid_warn(fmt...)
#define edid_err(fmt...)
#define edid_fatal(fmt...)
#define hdmi_info(fmt...)  hi_info_hdmi(fmt);
#define hdmi_warn(fmt...)  hi_warn_hdmi(fmt);
#define hdmi_err(fmt...)   hi_err_hdmi(fmt);
#define hdmi_fatal(fmt...) hi_fatal_hdmi(fmt);
#else
#define edid_info(fmt...)  hi_info_hdmi(fmt)
#define edid_warn(fmt...)  hi_warn_hdmi(fmt)
#define edid_err(fmt...)   hi_err_hdmi(fmt)
#define edid_fatal(fmt...) hi_fatal_hdmi(fmt)
#define hdmi_fatal(fmt...) hi_fatal_hdmi(fmt)
#define hdmi_err(fmt...)   hi_err_hdmi(fmt)
#define hdmi_warn(fmt...)  hi_warn_hdmi(fmt)
#define hdmi_info(fmt...)  hi_info_hdmi(fmt)
#endif

typedef osal_proc_entry_t hdmi_proc_entry;

/* structure of proc item */
typedef struct {
    hi_s32 (*fn_read)(hdmi_proc_entry *file);
    hi_s32 (*fn_write)(hdmi_proc_entry *file, const char *buf, hi_u32 count, hi_s64 *data);
} hdmi_proc_item;

typedef struct {
    hi_u32 ssc_bypass_div;
    hi_u32 tmds_clk_div;
} hdmi_crg_cfg;

hi_void hdmi_tx_reg_write(hi_u32 *reg_addr, hi_u32 value);

hi_u32 hdmi_tx_reg_read(const hi_u32 *reg_addr);

hi_void hdmi_reg_write_u32(hi_u32 reg_addr, hi_u32 value);

hi_u32 hdmi_reg_read_u32(hi_u32 reg_addr);

hi_void drv_hdmi_prod_io_cfg_set(hi_void);

hi_void drv_hdmi_prod_crg_all_reset_set(hi_bool enable);

hi_void drv_hdmi_prod_crg_gate_set(hi_bool enable);

hi_void drv_hdmi_prod_crg_div_set(hdmi_crg_cfg *crg_cfg);

hi_void drv_hdmi_prod_crg_phy_reset_set(hi_bool enable);

hi_void drv_hdmi_prod_crg_phy_reset_get(hi_bool *enable);

hi_void drv_hdmi_prod_crg_init(hi_void);

hi_void drv_hdmi_hardware_reset(hi_void);

hi_void drv_hdmi_low_power_set(hi_bool enable);

hdmi_chip_type drv_hdmi_prod_chip_get(hi_void);

#endif


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

#include "hdmi_hal_scdc.h"
#include "drv_hdmi_intf.h"
#include "hdmi_osal.h"
#include "hdmi_product_define.h"
#include "hdmi_hal_phy.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_machine.h"
#include "hdmi_hal_intf.h"
#include "hdmi_reg_tx.h"

#define SCDC_HDMI20_VERSION       1
#define SCDC_UPDATE_TIME_INTERVAL 150
#define READ_SRC_VERSION_COUNT    2

typedef struct {
    hi_bool src_scramble;
    hi_bool sink_scramble;
    hi_bool tmds_clk_ratio40x;
} scdc_scramble;

static scdc_infomation g_scdc_info[HDMI_DEVICE_ID_BUTT] = {{{0}, {0}}};

static scdc_infomation *scdc_info_ptr_get(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        return &g_scdc_info[hdmi_id];
    }
    return HI_NULL;
}

static hi_s32 scdc_scramble_set(hdmi_device_id hdmi_id, const scdc_scramble *scramble)
{
    hi_s32 ret;
    scdc_tmds_config ddc_config_byte = {0};
    ddc_cfg ddc_src_cfg = {0};

    hdmi_if_null_warn_return(scramble, HI_FAILURE);

    /* source scramble */
    hdmi_reg_enc_scr_on_set(scramble->src_scramble);
    /* sink scramble */
    ddc_config_byte.u8.screambling_enable = scramble->sink_scramble ? HI_TRUE : HI_FALSE;
    ddc_config_byte.u8.tmds_bit_clock_ratio = scramble->tmds_clk_ratio40x ? HI_TRUE : HI_FALSE;
    hal_hdmi_ddc_default_cfg_get(hdmi_id, &ddc_src_cfg);
    ddc_src_cfg.func_type  = DDC_FUNC_TYPE_SCDC;
    ddc_src_cfg.issue_mode = DDC_MODE_WRITE_MUTIL_ACK;
    ddc_src_cfg.offset     = SCDC_OFFSET_TMDS_CONFIG;
    ddc_src_cfg.data       = &ddc_config_byte.byte;
    ddc_src_cfg.data_size  = 1;
    ret = hal_hdmi_ddc_issue(hdmi_id, &ddc_src_cfg);

    return (ret == ddc_src_cfg.data_size) ? HI_SUCCESS : HI_FAILURE;
}

static hi_s32 scdc_scramble_get(hdmi_device_id hdmi_id, scdc_scramble *scramble)
{
    hi_s32 ret;
    scdc_scrambler_status ddc_sramble_byte = {0};
    scdc_tmds_config      ddc_config_byte  = {0};
    ddc_cfg               ddc_sink_cfg     = {0};
    scdc_infomation      *scdc_info        = HI_NULL;

    scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    /* source scrambled */
    scramble->src_scramble = hdmi_reg_enc_scr_on_get() ? HI_TRUE : HI_FALSE;
    hal_hdmi_ddc_default_cfg_get(hdmi_id, &ddc_sink_cfg);
    /* sink scrambled status */
    ddc_sink_cfg.func_type  = DDC_FUNC_TYPE_SCDC;
    ddc_sink_cfg.issue_mode = DDC_MODE_READ_MUTIL_NO_ACK;
    ddc_sink_cfg.data_size  = 1;
    ddc_sink_cfg.offset     = SCDC_OFFSET_SCRAMBLER_STATUS;
    ddc_sramble_byte.byte   = 0;
    ddc_sink_cfg.data       = &ddc_sramble_byte.byte;
    ret = hal_hdmi_ddc_issue(hdmi_id, &ddc_sink_cfg);
    scramble->sink_scramble = ddc_sramble_byte.u8.screambling_status ? HI_TRUE : HI_FALSE;
    scdc_info->status.reg_update[scdc_info->status.reg_update_ptr].scramble_status.byte = ddc_sramble_byte.byte;

    /* sink tmds bit clock ratio & scramble cfg */
    ddc_sramble_byte.byte = 0;
    ddc_sink_cfg.data     = &ddc_config_byte.byte;
    ddc_sink_cfg.offset   = SCDC_OFFSET_TMDS_CONFIG;
    ret = hal_hdmi_ddc_issue(hdmi_id, &ddc_sink_cfg);
    scramble->tmds_clk_ratio40x = ddc_config_byte.u8.tmds_bit_clock_ratio ? HI_TRUE : HI_FALSE;
    scdc_info->status.reg_static.tmds_config0.byte = ddc_config_byte.byte;

    return (ret == ddc_sink_cfg.data_size) ? HI_SUCCESS : HI_FAILURE;
}

hi_void scdc_mach_poll_callback(hi_void *data)
{
    scdc_infomation *scdc_info = (scdc_infomation *)data;
    hdmi_if_null_warn_return_void(scdc_info);
    hdmi_if_false_warn_return_void(scdc_info->status.init);

    return;
}

hi_s32 hal_hdmi_scdc_init(hdmi_device_id hdmi_id)
{
    hi_s32 ret = HI_SUCCESS;
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);
    hdmi_mach_ctrl   mach_ctrl = {0};

    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);

    if (!scdc_info->status.init) {
        mach_ctrl.data     = scdc_info;
        mach_ctrl.interval = SCDC_DEFAULT_POLL_INTERVAL;
        mach_ctrl.callback = scdc_mach_poll_callback;
        mach_ctrl.name     = "SCDC";
        (hi_void)memset_s(scdc_info, sizeof(scdc_infomation), 0, sizeof(scdc_infomation));
        scdc_info->status.init          = HI_TRUE;
        scdc_info->status.hdmi_id       = hdmi_id;
        scdc_info->status.mach_interval = mach_ctrl.interval;
        ret = hal_hdmi_mach_register(&mach_ctrl, &scdc_info->status.mach_id);
    }

    return ret;
}

hi_s32 hal_hdmi_scdc_deinit(hdmi_device_id hdmi_id)
{
    hi_s32 ret;
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);
    ret = hal_hdmi_mach_unregister(scdc_info->status.mach_id);
    (hi_void)memset_s(scdc_info, sizeof(scdc_infomation), 0, sizeof(scdc_infomation));
    scdc_info->status.init = HI_FALSE;
    return ret;
}

hi_s32 hal_hdmi_scdc_reset(hdmi_device_id hdmi_id)
{
    hi_s32           init_back;
    hi_u32           mach_id_back;
    scdc_scramble    scramble  = {0};
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);

    hal_hdmi_mach_stop(scdc_info->status.mach_id);
    init_back    = scdc_info->status.init;
    mach_id_back = scdc_info->status.mach_id;
    (hi_void)memset_s(scdc_info, sizeof(scdc_infomation), 0, sizeof(scdc_infomation));
    scdc_info->status.init     = init_back;
    scdc_info->status.mach_id  = mach_id_back;
    scdc_info->status.hdmi_id  = hdmi_id;
    scramble.src_scramble      = HI_FALSE;
    scramble.sink_scramble     = HI_FALSE;
    scramble.tmds_clk_ratio40x = HI_FALSE;
    scdc_scramble_set(hdmi_id, &scramble);

    return HI_SUCCESS;
}

static hi_void scdc_attr_param_check(scdc_attr *attr)
{
    if ((attr->scramble_interval < SCDC_DEFAULT_SCRAMBLE_INTERVAL) ||
        (attr->scramble_interval > SCDC_DEFAULT_SCRAMBLE_TIMEOUT)) {
        attr->scramble_interval = SCDC_DEFAULT_SCRAMBLE_INTERVAL;
    }
    if (attr->scramble_timeout < SCDC_DEFAULT_SCRAMBLE_TIMEOUT) {
        attr->scramble_timeout = SCDC_DEFAULT_SCRAMBLE_TIMEOUT;
    }
    if (attr->scramble_timeout > SCDC_MAX_SCRAMBLE_TIMEOUT) {
        attr->scramble_timeout = SCDC_MAX_SCRAMBLE_TIMEOUT;
    }
}

static hi_void scdc_pre_scramble(hdmi_device_id hdmi_id, scdc_infomation *scdc_info)
{
    ddc_cfg cfg = {0};
    hi_u8 ddc_byte = 0;

    /* read sink version */
    hal_hdmi_ddc_default_cfg_get(hdmi_id, &cfg);
    cfg.data_size  = 1;
    cfg.data       = &ddc_byte;
    cfg.func_type  = DDC_FUNC_TYPE_SCDC;
    cfg.offset     = SCDC_OFFSET_SINK_VERSION;
    cfg.issue_mode = DDC_MODE_READ_MUTIL_NO_ACK;
    hal_hdmi_ddc_issue(hdmi_id, &cfg);

    scdc_info->status.reg_static.sink_version = ddc_byte;
    if (ddc_byte != SCDC_HDMI20_VERSION) {
        scdc_info->status.err_cnt.sink_un_read_cnt++;
        hdmi_warn("scdc sink version is not %u\n", SCDC_HDMI20_VERSION);
    }
    /* RR_Enable disable */
    scdc_info->attr.sink_read_quest = HI_FALSE;
    scdc_info->status.reg_static.config0.byte = 0;
    scdc_info->status.reg_static.config0.u8.rr_enable = HI_FALSE;
    cfg.offset     = SCDC_OFFSET_CONFIG_0;
    cfg.issue_mode = DDC_MODE_WRITE_MUTIL_ACK;
    cfg.data       = &scdc_info->status.reg_static.config0.byte;
    hal_hdmi_ddc_issue(hdmi_id, &cfg);

    scdc_info->status.reg_static.rr_test_cfg.byte = 0;
    cfg.offset     = SCDC_OFFSET_TEST_CONFIG_0;
    cfg.issue_mode = DDC_MODE_WRITE_MUTIL_ACK;
    cfg.data       = &scdc_info->status.reg_static.rr_test_cfg.byte;
    hal_hdmi_ddc_issue(hdmi_id, &cfg);

    return;
}

static hi_void scdc_err_record(scdc_infomation *info, scdc_error_type err)
{
    errno_t ret;
    scdc_error_record *err_cord = HI_NULL;

    if (info->status.err_record_ptr >= SCDC_MAX_ERROR_RECORD_NUM) {
        hdmi_err("scdc err record failed, invalid index!\n");
        return;
    }
    err_cord = &info->status.err_record[info->status.err_record_ptr];
    err_cord->err_type = err;
    err_cord->time_stamp = hal_hdmi_mach_ms_get();
    ret = memcpy_s(&err_cord->err_attr, sizeof(err_cord->err_attr), &info->attr, sizeof(scdc_attr));
    hdmi_unequal_eok_return_void(ret);
    switch (err_cord->err_type) {
        case SCDC_ERROR_SINK_UNREAD:
            info->status.err_cnt.sink_un_read_cnt++;
            break;
        case SCDC_ERROR_SCRAMBLE:
            info->status.err_cnt.sramble_fail_cnt++;
            break;
        case SCDC_ERROR_OE_DISABLE:
            info->status.err_cnt.oe_disable_fail_cnt++;
            break;
        case SCDC_ERROR_OE_ENABLE:
            info->status.err_cnt.oe_enable_fail_cnt++;
            break;
        default:
            break;
    }
    info->status.err_record_ptr = (info->status.err_record_ptr + 1) % SCDC_MAX_ERROR_RECORD_NUM;

    return;
}

static hi_void scdc_oe_set(hi_bool oe_en, hi_u32 timeout, scdc_infomation *info)
{
    hi_bool oe_enable = HI_FALSE;

#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_oe_set(oe_en);
    osal_msleep(timeout);
    hal_hdmi_phy_oe_get(&oe_enable);
#endif

    if (oe_enable != oe_en) {
        if (oe_en == HI_TRUE) {
            if (info->status.err_cnt.oe_enable_fail_cnt < SCDC_ERROR_MAX_NUM_OE_ENABLE) {
                scdc_err_record(info, SCDC_ERROR_OE_ENABLE);
            }
        } else {
            if (info->status.err_cnt.oe_disable_fail_cnt < SCDC_ERROR_MAX_NUM_OE_DISABLE) {
                scdc_err_record(info, SCDC_ERROR_OE_DISABLE);
            }
        }
        hdmi_err("scdc oe oe_enable=%u fail, expected %u\n", oe_enable, oe_en);
    }

    return;
}

hi_s32 hal_hdmi_scdc_attr_set(hdmi_device_id hdmi_id, scdc_attr *attr)
{
    hi_s32             ret;
    hi_u32             scdc_timestart, scdc_time_cur, timeout;
    scdc_scramble      scramble  = {0};
    scdc_infomation   *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(attr, HI_FAILURE);
    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);
    scdc_attr_param_check(attr);

    ret = memcpy_s(&scdc_info->attr, sizeof(scdc_info->attr), attr, sizeof(scdc_attr));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    scdc_pre_scramble(hdmi_id, scdc_info);
    osal_msleep(SCDC_DEFAULT_SCRAMBLE_INTERVAL);
    /* scrameble cfg */
    timeout = scdc_info->attr.scramble_interval;
    scdc_timestart = hdmi_osal_get_time_in_ms();

    do {
        /* oe disable */
        scdc_oe_set(HI_FALSE, SCDC_OE_DISABLE_WAIT_TIME, scdc_info);
        /* sink & source scramble config */
        scramble.src_scramble      = scdc_info->attr.src_scramble;
        scramble.sink_scramble     = scdc_info->attr.sink_scramble;
        scramble.tmds_clk_ratio40x = scdc_info->attr.tmds_clk_ratio40x;
        scdc_scramble_set(hdmi_id, &scramble);
        /* oe enable */
#ifndef HDMI_FPGA_SUPPORT
        osal_msleep(timeout);
#endif
        scdc_oe_set(HI_TRUE, timeout, scdc_info);
        /* poll status */
        scdc_scramble_get(hdmi_id, &scramble);
        osal_msleep(SCDC_GET_SCRAMBLE_STATUS_TIME);
        /* compatibility try again */
        ret = HI_SUCCESS;
        if ((scramble.tmds_clk_ratio40x != scdc_info->attr.tmds_clk_ratio40x)) {
            scdc_err_record(scdc_info, SCDC_ERROR_SCRAMBLE);
            ret = HI_FAILURE;
        }
        timeout += scdc_info->attr.scramble_interval;
        scdc_time_cur = hdmi_osal_get_time_in_ms();
    } while ((ret != HI_SUCCESS) && (timeout <= scdc_info->attr.scramble_timeout) &&
             ((scdc_time_cur - scdc_timestart) < SCDC_WHILE_SCRAMBLE_TIMEOUT));

    hdmi_info("\ndelay_interval=%u, timeout=%u, src_scarmble=%u, sink_scramble=%u, tmds_clk_ratio40x=%u\n",
              timeout, (scdc_time_cur - scdc_timestart), scramble.src_scramble,
              scramble.sink_scramble, scramble.tmds_clk_ratio40x);

    if (ret != HI_SUCCESS) {
        hdmi_err("scdc scramble %s fail!\n", scdc_info->attr.sink_scramble ? "ON" : "OFF");
    } else {
        hdmi_info("scdc scramble %s success!\n", scdc_info->attr.sink_scramble ? "ON" : "OFF");
    }

    if ((ret != HI_SUCCESS) || scdc_info->attr.sink_scramble == HI_FALSE) {
        ret += hal_hdmi_mach_stop(scdc_info->status.mach_id);
        scdc_info->status.mach_enable = HI_FALSE;
    }

    return (ret != HI_SUCCESS) ? HI_FAILURE : HI_SUCCESS;
}

hi_s32 hal_hdmi_scdc_attr_get(hdmi_device_id hdmi_id, scdc_attr *attr)
{
    errno_t ret;
    scdc_scramble scramble = {0};
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(attr, HI_FAILURE);
    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);

    scdc_scramble_get(hdmi_id, &scramble);
    scdc_info->attr.sink_scramble     = scramble.sink_scramble;
    scdc_info->attr.src_scramble      = scramble.src_scramble;
    scdc_info->attr.tmds_clk_ratio40x = scramble.tmds_clk_ratio40x;
    ret = memcpy_s(attr, sizeof(scdc_attr), &scdc_info->attr, sizeof(scdc_attr));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return HI_SUCCESS;
}

/* real sw */
hi_s32 hal_hdmi_scdc_all_status_get(hdmi_device_id hdmi_id, scdc_status *status)
{
    errno_t ret;
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(status, HI_FAILURE);
    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);

    ret = memcpy_s(status, sizeof(scdc_status), &scdc_info->status, sizeof(scdc_status));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_scdc_support_get(hdmi_device_id hdmi_id, hi_bool *support)
{
    hi_s32           ret;
    hi_u8            ddc_byte    = 0;
    hi_u32           re_read_cnt = 0;
    ddc_cfg          tmp         = {0};
    scdc_infomation *scdc_info   = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(support, HI_FAILURE);
    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);

    hal_hdmi_ddc_default_cfg_get(hdmi_id, &tmp);
    tmp.data_size = 1;
    tmp.data      = &ddc_byte;
    tmp.func_type = DDC_FUNC_TYPE_SCDC;
    tmp.offset    = SCDC_OFFSET_SRC_VERSION;

    do {
        /* write source version */
        tmp.issue_mode = DDC_MODE_WRITE_MUTIL_ACK;
        ddc_byte = SCDC_HDMI20_VERSION;
        ret = hal_hdmi_ddc_issue(hdmi_id, &tmp);

        /* read source version */
        tmp.issue_mode = DDC_MODE_READ_SINGLE_NO_ACK;
        ddc_byte = 0;
        ret = hal_hdmi_ddc_issue(hdmi_id, &tmp);
        re_read_cnt++;
    } while ((ret != tmp.data_size) && (re_read_cnt < READ_SRC_VERSION_COUNT));

    if (ret == tmp.data_size) {
        scdc_info->status.reg_static.src_version = SCDC_HDMI20_VERSION;
        *support = HI_TRUE;
    } else {
        scdc_info->status.reg_static.src_version = 0;
        *support = HI_FALSE;
    }

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_scdc_scramble_force(hdmi_device_id hdmi_id, hi_bool enable)
{
    scdc_scramble scramble = {0};
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);

    if (enable) {
        scramble.sink_scramble            = HI_TRUE;
        scramble.tmds_clk_ratio40x        = HI_TRUE;
        scramble.src_scramble             = HI_TRUE;
        scdc_info->attr.sink_scramble     = HI_TRUE;
        scdc_info->attr.src_scramble      = HI_TRUE;
        scdc_info->attr.tmds_clk_ratio40x = HI_TRUE;
    } else {
        scramble.sink_scramble            = HI_FALSE;
        scramble.tmds_clk_ratio40x        = HI_FALSE;
        scramble.src_scramble             = HI_FALSE;
        scdc_info->attr.sink_scramble     = HI_FALSE;
        scdc_info->attr.src_scramble      = HI_FALSE;
        scdc_info->attr.tmds_clk_ratio40x = HI_FALSE;
    }

    return scdc_scramble_set(hdmi_id, &scramble);
}

hi_s32 hal_hdmi_scdc_mach_enable(hdmi_device_id hdmi_id, hi_bool enable)
{
    hi_s32 ret;
    scdc_infomation *scdc_info = scdc_info_ptr_get(hdmi_id);

    hdmi_if_null_warn_return(scdc_info, HI_FAILURE);
    hdmi_if_false_warn_return(scdc_info->status.init, HI_FAILURE);

    if (enable) {
        ret = hal_hdmi_mach_start(scdc_info->status.mach_id);
    } else {
        ret = hal_hdmi_mach_stop(scdc_info->status.mach_id);
    }

    return ret;
}


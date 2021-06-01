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
#include "hdmi_hal_ddc.h"
#include "hdmi_osal.h"
#include "hdmi_reg_aon.h"
#include "hdmi_reg_ctrl.h"
#include "hdmi_hal_intf.h"
#include "hdmi_hal_ctrl.h"
#include "hdmi_product_define.h"

#define DDC_EDID_SALVE_ADDR  0xa0
#define DDC_HDCP_SALVE_ADDR  0x74
#define DDC_SCDC_SALVE_ADDR  0xa8
#define DDC_MAX_FIFO_SIZE    16
#define DDC_EXT_BLOCK_OFFSET 0x7e
#define DDC_MAX_EDID_EXT_NUM 3    /* 3: 4(max block num) - 1(base block) */
#define DDC_DEFAULT_DELAY    8    /* 8us */

static ddc_info g_ddc_info[HDMI_DEVICE_ID_BUTT];

typedef enum {
    DDC_CMD_READ_SINGLE_NO_ACK,
    DDC_CMD_READ_SINGLE_ACK,
    DDC_CMD_READ_MUTI_NO_ACK,
    DDC_CMD_READ_MUTI_ACK,
    DDC_CMD_READ_SEGMENT_NO_ACK,
    DDC_CMD_READ_SEGMENT_ACK,
    DDC_CMD_WRITE_MUTI_NO_ACK,
    DDC_CMD_WRITE_MUTI_ACK,
    DDC_CMD_FIFO_CLR = 0x09,
    DDC_CMD_SCL_DRV,
    DDC_CMD_MASTER_ABORT = 0x0f
} ddc_issue_cmd;

static ddc_info *ddc_info_ptr_get(hdmi_device_id hdmi)
{
    if (hdmi < HDMI_DEVICE_ID_BUTT) {
        return &g_ddc_info[hdmi];
    }
    return HI_NULL;
}

static hi_s32 ddc_access_enable_wait(hi_u32 timeout)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 tmp_time = 0;

    hdmi_reg_cpu_ddc_req_set(HI_TRUE);

    while (!hdmi_reg_cpu_ddc_req_ack_get()) {
        osal_msleep(1);
        tmp_time++;
        if (tmp_time > timeout) {
            ret = HI_FAILURE;
            break;
        }
    }
    return ret;
}

static hi_s32 ddc_access_disable_wait(hi_u32 timeout)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 tmp_time = 0;

    hdmi_reg_cpu_ddc_req_set(HI_FALSE);

    while (hdmi_reg_cpu_ddc_req_ack_get()) {
        osal_msleep(1);
        tmp_time += 1;
        if (tmp_time > timeout) {
            ret = HI_FAILURE;
            break;
        }
    }
    return ret;
}

static hi_s32 ddc_scl_wait(hi_u32 timeout)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 tmp_time = 0;

    while (!hdmi_reg_ddc_scl_st_get()) {
        osal_msleep(1);
        tmp_time += 1;
        if (tmp_time > timeout) {
            ret = HI_FAILURE;
            break;
        }
    }
    return ret;
}

static hi_s32 ddc_sda_wait(hi_u32 timeout)
{
    hi_u32 tmp_timeout = 0;
    hi_s32 ret = HI_SUCCESS;

    if (!hdmi_reg_ddc_sda_st_get()) {
        hdmi_reg_dcc_man_en_set(HI_TRUE);
        while ((!hdmi_reg_ddc_sda_st_get()) && tmp_timeout++ < timeout) {
            /* pull scl high */
            hdmi_reg_ddc_scl_oen_set(HI_TRUE);
            osal_udelay(DDC_DEFAULT_DELAY);
            /* pull scl low */
            hdmi_reg_ddc_scl_oen_set(HI_FALSE);
            osal_udelay(DDC_DEFAULT_DELAY);
        }

        /* STOP contition */
        if (tmp_timeout < timeout && (hdmi_reg_ddc_sda_st_get())) {
            /* pull sda low */
            hdmi_reg_ddc_sda_oen_set(HI_FALSE);
            osal_udelay(DDC_DEFAULT_DELAY);
            /* pull scl high */
            hdmi_reg_ddc_scl_oen_set(HI_TRUE);
            osal_udelay(DDC_DEFAULT_DELAY);
            /* pull sda high */
            hdmi_reg_ddc_sda_oen_set(HI_TRUE);
            osal_udelay(DDC_DEFAULT_DELAY);
            hdmi_info("deadlock clear success\n");
            ret = HI_SUCCESS;
        } else {
            hdmi_warn("deadlock clear fail\n");
            ret = HI_FAILURE;
        }
        hdmi_reg_dcc_man_en_set(HI_FALSE);
    }

    return ret;
}

static hi_s32 ddc_in_prog_wait(hi_u32 timeout)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 tmp_time = 0;

    while (hdmi_reg_pwd_i2c_in_prog_get()) {
        osal_msleep(1);
        tmp_time += 1;
        if (tmp_time > timeout) {
            ret = HI_FAILURE;
            break;
        }
    }

    return ret;
}

static hi_s32 ddc_cmd_issue(const ddc_cfg *cfg)
{
    hi_u32 slave_addr = 0;
    hi_u32 segment, offset, data_size;

    segment   = cfg->segment;
    offset    = cfg->offset;
    data_size = cfg->data_size;

    switch (cfg->func_type) {
        case DDC_FUNC_TYPE_EDID:
            slave_addr = DDC_EDID_SALVE_ADDR;
            break;
        case DDC_FUNC_TYPE_HDCP:
            slave_addr = DDC_HDCP_SALVE_ADDR;
            break;
        case DDC_FUNC_TYPE_SCDC:
            slave_addr = DDC_SCDC_SALVE_ADDR;
            break;
        default:
            hdmi_warn("un-known DDC function type, wrong slaveaddr!\n");
            break;
    }

    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        hdmi_reg_pwd_mst_cmd_set(DDC_CMD_FIFO_CLR);
        hdmi_reg_pwd_slave_addr_set(slave_addr);
        hdmi_reg_pwd_slave_seg_set(segment);
        hdmi_reg_pwd_slave_offset_set(offset);
        hdmi_reg_pwd_data_out_cnt_set(data_size);
        osal_udelay(DDC_DEFAULT_DELAY);
        hdmi_reg_pwd_mst_cmd_set(cfg->issue_mode);
    } else {
        hdmi_err("unknown master_mode=%u\n", cfg->master_mode);
    }

    return HI_SUCCESS;
}

static hi_s32 ddc_read(ddc_cfg *cfg)
{
    hi_u32 len;
    ddc_func_type type;
    hi_u8 *data = HI_NULL;
    hi_u32 i, retry, data_size;
    hi_u32 time_start, time_curr;

    data  = cfg->data;
    type  = cfg->func_type;
    len   = cfg->data_size;
    retry = cfg->issue_timeout < DDC_DEFAULT_TIMEOUT_ISSUE ? DDC_DEFAULT_TIMEOUT_ISSUE : cfg->sda_timeout;

    for (data_size = 0; data_size < len; data_size++, data++) {
        /* maybe msleep(1) cost 20ms, 30 times, total 600ms. */
        time_start = hdmi_osal_get_time_in_ms();
        time_curr  = time_start;
        /* when read-fifo empty, every byte wait a max timeout */
        for (i = 0;
             ((i < retry) && (hdmi_reg_pwd_fifo_empty_get() || (hdmi_reg_pwd_fifo_data_out_get() == 0))) &&
             ((time_curr - time_start) <= DDC_DEFAULT_RETRY_TIMEOUT_ISSUE);
             i++) {
            /* wait ddc status update after DDC cmd set. */
            osal_msleep(1);
            if (hdmi_reg_ddc_i2c_no_ack_get() || hdmi_reg_ddc_i2c_bus_low_get()) {
                hdmi_reg_pwd_mst_cmd_set(DDC_CMD_MASTER_ABORT);
                hdmi_warn("DDC status error!\n");
                return HI_FAILURE;
            }
            time_curr = hdmi_osal_get_time_in_ms();
        }
        if ((i >= retry) || ((time_curr - time_start) > DDC_DEFAULT_RETRY_TIMEOUT_ISSUE)) {
            if (type != DDC_FUNC_TYPE_SCDC) {
                hdmi_warn("read fifo retry=%u ms, size=%u, timeout:%u!\n",
                          retry, len, (time_curr - time_start));
            } else {
                hdmi_info("read fifo retry=%u ms, size=%u, timeout:%u!\n",
                          retry, len, (time_curr - time_start));
            }
            return HI_FAILURE;
        }
        if (data != HI_NULL) {
            *data = hdmi_reg_rdata_pwd_fifo_data_out_get();
            /*
             * the fifo status is not refresh promptly,
             * so re-read the fifo status and delay 1us if the fifo is empty,
             * wait the data ready. it must delay 1us after read fifo data.
             */
            osal_udelay(1);
        } else {
            hdmi_err("edid &data[%u]=null\n", data_size);
            return HI_FAILURE;
        }
    }

    return data_size;
}

static hi_s32 ddc_write(ddc_cfg *cfg)
{
    hi_u32 len;
    ddc_func_type type;
    hi_u8 *data = HI_NULL;
    hi_u32 i, retry, data_size;
    hi_u32 time_start, time_curr;

    data  = cfg->data;
    type  = cfg->func_type;
    len   = cfg->data_size;
    retry = cfg->issue_timeout < DDC_DEFAULT_TIMEOUT_ISSUE ? DDC_DEFAULT_TIMEOUT_ISSUE : cfg->sda_timeout;

    for (data_size = 0; data_size < len; data_size++, data++) {
        /* maybe msleep(1) cost 20ms, every byte wait a max timeout 60ms. */
        time_start = hdmi_osal_get_time_in_ms();
        time_curr = time_start;
        /* when write-fifo full, every byte wait a max timeout and retry times */
        for (i = 0;
             (((i < retry) && (hdmi_reg_pwd_fifo_data_out_get() >= DDC_MAX_FIFO_SIZE)) &&
              ((time_curr - time_start) <= DDC_DEFAULT_RETRY_TIMEOUT_ISSUE));
             i++) {
            /* wait ddc status update after DDC cmd set. */
            osal_msleep(1);
            if (hdmi_reg_ddc_i2c_no_ack_get() || hdmi_reg_ddc_i2c_bus_low_get()) {
                hdmi_reg_pwd_mst_cmd_set(DDC_CMD_MASTER_ABORT);
                hdmi_warn("DDC status error!\n");
                return HI_FAILURE;
            }
            time_curr = hdmi_osal_get_time_in_ms();
        }
        if (i >= retry || ((time_curr - time_start) > DDC_DEFAULT_RETRY_TIMEOUT_ISSUE)) {
            if (type != DDC_FUNC_TYPE_SCDC) {
                hdmi_err("write fifo retry=%u ms, size=%u, timeout:%u!\n",
                         retry, len, (time_curr - time_start));
            } else {
                hdmi_info("write fifo retry=%u ms, size=%u, timeout:%u!\n",
                          retry, len, (time_curr - time_start));
            }
            return HI_FAILURE;
        }

        if (data != HI_NULL) {
            hdmi_reg_pwd_fifo_data_in_set(*data);
            osal_udelay(1);
        } else {
            hdmi_err("edid &data[%u]=null\n", data_size);
            return HI_FAILURE;
        }
    }

    return data_size;
}

static hi_s32 ddc_data_pwd_issue(ddc_cfg *cfg, hi_bool read_issue)
{
    hi_s32 data_size;

    if (read_issue) {
        /* read issue */
        data_size = ddc_read(cfg);
    } else {
        /* write issue */
        data_size = ddc_write(cfg);
    }

    return data_size;
}

static hi_s32 ddc_data_issue(ddc_cfg *cfg, hi_bool read_issue)
{
    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        return ddc_data_pwd_issue(cfg, read_issue);
    } else {
        hdmi_err("unknown master_mode=%u,fail!\n", cfg->master_mode);
        return HI_FAILURE;
    }
}

static hi_s32 ddc_err_clean_check(hdmi_device_id hdmi)
{
    hi_s32 ret = HI_SUCCESS;
    ddc_info *tmp = ddc_info_ptr_get(hdmi);

    hdmi_if_null_warn_return(tmp, HI_FAILURE);
    hdmi_if_false_warn_return(tmp->run.init, HI_FAILURE);

    /* in prog check */
    if (ddc_in_prog_wait(DDC_DEFAULT_TIMEOUT_IN_PROG) != HI_SUCCESS) {
        hdmi_warn("error clr, wait in prog timeout!\n");
        ret = HI_FAILURE;
    }
    /* scl check */
    if (ddc_scl_wait(DDC_DEFAULT_TIMEOUT_SCL) != HI_SUCCESS) {
        hdmi_warn("error clr, wait scl timeout!\n");
        ret = HI_FAILURE;
    }
    /* sda check */
    if (ddc_sda_wait(DDC_DEFAULT_TIMEOUT_SDA) != HI_SUCCESS) {
        hdmi_warn("error clr, wait sda timeout!\n");
        ret = HI_FAILURE;
    }
    if (ret == HI_SUCCESS) {
        hdmi_info("error clr success!\n");
    }

    return ret;
}

static hi_s32 ddc_reset(hdmi_device_id hdmi)
{
    hi_u32 i;
    ddc_record_elem *tmp_elem = HI_NULL;
    ddc_info *tmp = ddc_info_ptr_get(hdmi);

    hdmi_if_null_warn_return(tmp, HI_FAILURE);
    hdmi_if_false_warn_return(tmp->run.init, HI_FAILURE);

    for (i = 0; i < DDC_MAX_RECORD_NUM; i++) {
        tmp_elem = &tmp->ddc_record[i];
        if (tmp_elem->data != HI_NULL) {
            osal_vfree(tmp_elem->data);
        }
        (hi_void)memset_s(tmp_elem, sizeof(tmp->ddc_record[i]), 0, sizeof(ddc_record_elem));
    }

    tmp->run.elem_ptr  = 0;
    tmp->run.total_num = 0;

    return ddc_err_clean_check(hdmi);
}

static hi_s32 ddc_issue(ddc_cfg *cfg, ddc_record_elem *tmp_elem)
{
    hi_s32 ret = HI_SUCCESS;
    hi_bool read_issue = HI_FALSE;

    /* access check */
    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        if (ddc_access_enable_wait(cfg->access_timeout) != HI_SUCCESS) {
            hdmi_err("wait access bus timeout!\n");
            tmp_elem->un_err.u32.access_wait_timeout = HI_TRUE;
            return ret;
        }
    }
    /* scl check */
    if (ddc_scl_wait(cfg->scl_timeout) != HI_SUCCESS) {
        hdmi_err("wait scl timeout!\n");
        tmp_elem->un_err.u32.bus_low_scl = HI_TRUE;
        return ret;
    }
    /* sda check */
    if (ddc_sda_wait(cfg->sda_timeout) != HI_SUCCESS) {
        hdmi_err("wait sda timeout!\n");
        tmp_elem->un_err.u32.bus_low_sda = HI_TRUE;
        return ret;
    }
    /* jude read/write issue */
    if (cfg->issue_mode <= DDC_MODE_READ_SEGMENT_ACK) {
        read_issue = HI_TRUE;
    } else if (cfg->issue_mode < DDC_MODE_BUTT) {
        read_issue = HI_FALSE;
    } else {
        hdmi_err("un-known ddc issue mode!\n");
        tmp_elem->un_err.u32.err_isseu_mode = HI_TRUE;
        return ret;
    }
    /* issue command */
    if (ddc_cmd_issue(cfg) != HI_SUCCESS) {
        hdmi_err("command issue fail!\n");
        return ret;
    }
    /* issue data */
    ret = ddc_data_issue(cfg, read_issue);
    if (ret <= 0) {
        hdmi_info("data issue fail!\n");
        tmp_elem->un_err.u32.issue_timeout = HI_TRUE;
        return ret;
    }

    return ret;
}

hi_s32 hal_hdmi_ddc_init(hdmi_device_id hdmi)
{
    ddc_info *tmp = ddc_info_ptr_get(hdmi);

    hdmi_if_null_warn_return(tmp, HI_FAILURE);

    if (!tmp->run.init) {
        osal_sema_init(&tmp->ddc_wr_mutex, 1);
        (hi_void)memset_s(tmp->ddc_record, sizeof(tmp->ddc_record), 0, sizeof(tmp->ddc_record));
        tmp->run.init = HI_TRUE;
        tmp->run.elem_ptr = 0;
        tmp->run.total_num = 0;
    }

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ddc_deinit(hdmi_device_id hdmi)
{
    ddc_info *tmp = ddc_info_ptr_get(hdmi);

    hdmi_if_null_warn_return(tmp, HI_FAILURE);
    hdmi_if_false_warn_return(tmp->run.init, HI_FAILURE);

    ddc_reset(hdmi);
    osal_sema_destroy(&tmp->ddc_wr_mutex);
    tmp->run.init = HI_FALSE;

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_ddc_issue(hdmi_device_id hdmi, ddc_cfg *cfg)
{
    errno_t errnumber;
    hi_s32 ret;
    ddc_record_elem *tmp_elem = HI_NULL;
    ddc_info        *tmp = ddc_info_ptr_get(hdmi);

    hdmi_if_null_warn_return(cfg, HI_FAILURE);
    hdmi_if_null_warn_return(tmp, HI_FAILURE);
    hdmi_if_false_warn_return(tmp->run.init, HI_FAILURE);

    tmp->run.elem_ptr = (1 + tmp->run.elem_ptr) % DDC_MAX_RECORD_NUM;
    if (tmp->run.total_num <= DDC_MAX_RECORD_NUM) {
        tmp->run.total_num++;
    }
    tmp_elem = &tmp->ddc_record[tmp->run.elem_ptr];
    (hi_void)memset_s(tmp_elem, sizeof(tmp->ddc_record[tmp->run.elem_ptr]), 0, sizeof(ddc_record_elem));
    errnumber = memcpy_s(&tmp_elem->cfg, sizeof(tmp_elem->cfg), cfg, sizeof(ddc_cfg));
    hdmi_unequal_eok_return(errnumber, HI_ERR_HDMI_INVALID_PARA);
    tmp_elem->start_time = hal_hdmi_mach_ms_get();

    hdmi_mutex_lock(tmp->ddc_wr_mutex);
    ret = ddc_issue(cfg, tmp_elem);
    /* in prog check */
    if (ddc_in_prog_wait(cfg->in_prog_timeout) != HI_SUCCESS) {
        hdmi_warn("wait in prog timeout!\n");
        tmp_elem->un_err.u32.in_prog_timeout = HI_TRUE;
    }
    if (tmp_elem->un_err.word) {
        hdmi_hpd_rsen hpd = {0};
        hal_hdmi_ctrl_hpd_rsen_get(hdmi, &hpd);
        if (!hpd.hpd_on) {
            hdmi_warn("hpd no exist!\n");
            tmp_elem->un_err.u32.hpd_no_exist = HI_TRUE;
        } else if (ddc_err_clean_check(hdmi) != HI_SUCCESS) {
            hdmi_warn("error clear fail!\n");
            tmp_elem->un_err.u32.clr_err_fail = 1;
        }
    }
    if (cfg->master_mode == DDC_MASTER_MODE_PWD) {
        if (ddc_access_disable_wait(cfg->access_timeout) != HI_SUCCESS) {
            hdmi_warn("wait access disable timeout!\n");
        }
    }
    tmp_elem->time_len = hal_hdmi_mach_ms_get() - tmp_elem->start_time;
    hdmi_mutex_unlock(tmp->ddc_wr_mutex);

    return ret;
}

hi_void hal_hdmi_ddc_default_cfg_get(hdmi_device_id hdmi, ddc_cfg *cfg)
{
    ddc_info *tmp = ddc_info_ptr_get(hdmi);

    hdmi_if_null_warn_return_void(tmp);
    hdmi_if_null_warn_return_void(cfg);
    hdmi_if_false_warn_return_void(tmp->run.init);

    cfg->segment         = 0;
    cfg->offset          = 0;
    cfg->func_type       = DDC_FUNC_TYPE_EDID;
    cfg->issue_mode      = DDC_MODE_READ_MUTIL_NO_ACK;
    cfg->speed           = 0;
    cfg->master_mode     = DDC_MASTER_MODE_PWD;
    cfg->access_timeout  = DDC_DEFAULT_TIMEOUT_ACCESS;
    cfg->hpd_timeout     = DDC_DEFAULT_TIMEOUT_HPD;
    cfg->in_prog_timeout = DDC_DEFAULT_TIMEOUT_IN_PROG;
    cfg->scl_timeout     = DDC_DEFAULT_TIMEOUT_SCL;
    cfg->sda_timeout     = DDC_DEFAULT_TIMEOUT_SDA;
    cfg->issue_timeout   = DDC_DEFAULT_TIMEOUT_ISSUE;
    cfg->data_size       = 0;
    cfg->data            = HI_NULL;

    return;
}

static hi_s32 edid_get_extern_blk(hdmi_device_id hdmi, const hi_u8 ext_blk_num, const hi_s32 raw_len, ddc_cfg *cfg)
{
    hi_s32 ret = raw_len;

    cfg->data      += cfg->data_size;
    cfg->offset     = 0;
    cfg->issue_mode = DDC_MODE_READ_SEGMENT_NO_ACK;
    cfg->segment    = 1;
    cfg->data_size  = HDMI_EDID_BLOCK_SIZE * ext_blk_num;
    ret += hal_hdmi_ddc_issue(hdmi, cfg);
    if (ret > 0) {
        hdmi_info("EDID read EXT-block 2~%u success!\n", ext_blk_num + 1);
    } else {
        hdmi_warn("EDID read EXT-block 2~%u fail!\n", ext_blk_num + 1);
    }

    return ret;
}

hi_s32 hal_hdmi_ddc_edid_raw_get(hdmi_device_id hdmi, hi_s32 size, hi_u8 *data)
{
    hi_u8 ext_block_num, des_block_num;
    hi_s32 ret;
    ddc_cfg cfg = {0};

    hdmi_if_null_warn_return(data, HI_FAILURE);

    hal_hdmi_ddc_default_cfg_get(hdmi, &cfg);
    cfg.segment    = 0;
    cfg.func_type  = DDC_FUNC_TYPE_EDID;
    cfg.issue_mode = DDC_MODE_READ_MUTIL_NO_ACK;
    cfg.data_size  = HDMI_EDID_BLOCK_SIZE;
    cfg.data       = &data[0];
    ret = hal_hdmi_ddc_issue(hdmi, &cfg);
    /* block 0 */
    if (ret <= 0) {
        hdmi_warn("edid block 0 read fail!\n");
        return ret;
    }
    hdmi_info("EDID block 0 read success!\n");

    ext_block_num = data[DDC_EXT_BLOCK_OFFSET];
    if (ext_block_num > DDC_MAX_EDID_EXT_NUM) {
        hdmi_warn("sink edid blocks num=%u, but we only support %u\n", ext_block_num, DDC_MAX_EDID_EXT_NUM);
        ext_block_num = DDC_MAX_EDID_EXT_NUM;
    }

    des_block_num = (size / HDMI_EDID_BLOCK_SIZE) - 1;
    if (ext_block_num > des_block_num) {
        hdmi_warn("scr block num > des block num, adapt ext_block_num to %u\n", des_block_num);
        ext_block_num = des_block_num;
    }
    if (ext_block_num == 0) {
        hdmi_warn("edid block 0 read fail!\n");
        return ret;
    }
    /* block 1 */
    cfg.data += cfg.data_size;
    cfg.offset = HDMI_EDID_BLOCK_SIZE;
    ret += hal_hdmi_ddc_issue(hdmi, &cfg);
    if (ret > 0) {
        hdmi_info("EDID EXT-block 1 read success!\n");
        ext_block_num--;
        if (ext_block_num) {
            /* block 2&3 */
            ret = edid_get_extern_blk(hdmi, ext_block_num, ret, &cfg);
        }
    } else {
        hdmi_warn("EDID EXT-block 1 read fail!\n");
    }

    return ret;
}


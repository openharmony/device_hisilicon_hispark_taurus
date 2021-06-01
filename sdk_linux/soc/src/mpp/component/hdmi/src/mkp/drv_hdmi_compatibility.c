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
#include "drv_hdmi_compatibility.h"
#include "hdmi_osal.h"

#define DEF_FILE_NAMELENGTH 32

typedef struct {
    hi_char mfrs_name[4];
    hi_u32  product_code;
    hi_u32  serial_number;
    hi_u32  week;
    hi_u32  year;
} compat_edid_manufacture_info;

typedef struct {
    compat_edid_manufacture_info edid;
    hi_u64 ms_under340_m;
    hi_u64 ms_over340_m;
} compat_hdcp_ms;

typedef struct {
    compat_edid_manufacture_info edid;
    hdmi_video_timing timing;
    hdmi_avmute_cfg avmute_cfg;
} compat_sink_avmute;

typedef struct {
    compat_edid_manufacture_info edid;
    hdmi_video_timing timing;
    hi_u32 fmt_delay;
    hi_u32 mute_delay;
} compat_sink_delay;

typedef struct {
    compat_edid_manufacture_info edid;
    hi_u32 stop_delay;
} compat_sink_stop_delay;

compatibility_info g_compat_info[HDMI_DEVICE_ID_BUTT];

static compat_hdcp_ms g_sink_hdcp_ms[] = {
    /*   name prod_code serial_no week year  under340  over340 */
    {{ "SNY", 51203,   16843009,   1, 2015 },  500,    2100 },
    {{ "MEI", 41622,   16843009,   0, 2014 }, 2000,    3000 },
    {{ "GSM",     1,   16843009,   1, 2016 },  500,    6000 }
};

static compat_sink_avmute g_sink_avmute[] = {
    /*   name  prod_code serial_no week year           timing      mute_pkg_en set(no use) mute_clr rpt rpt_cnt */
    // default config for all other TV have no problem.
    {{ "DEF",     0,        0,   0,    0 }, HDMI_VIDEO_TIMING_BUTT, { HI_TRUE,  HI_TRUE,  HI_TRUE,  HI_TRUE, 17 }},
    {{ "HEC",    48,        1,  35, 2010 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "SAM",  3140, 16777217,  42, 2014 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "SHP",  4452,        0, 255, 2016 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "MST",    48,        1,   0, 2007 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "GSM",     1, 16843009,   1, 2016 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "CHD", 36897,        1,  32, 2008 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "HEC",    48,        1,  28, 2016 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "SHP", 17446,        0, 255, 2015 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "SKY",     1, 16843009,  34, 2017 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "HRE",  1691,        1,  51, 2010 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "SKY",     1, 16843009,   2, 2016 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "GSM", 22928,   524660,   9, 2013 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }},
    {{ "GSM", 22925,   181207,  10, 2013 }, HDMI_VIDEO_TIMING_BUTT, { HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, 0 }}
};

static compat_edid_manufacture_info g_sink_scdc_pos[] = {
    /* name prod_code serial_no  week year */
    { "GSM", 1, 16843009, 1, 2016 }
};

static compat_sink_delay g_sink_delay[] = {
    /* name  prod_code serial_no week year      timing           fmt mute */
    {{ "SKW",   48,         1,  1, 2008 }, HDMI_VIDEO_TIMING_BUTT, 500, 900 },
    {{ "SKY",   48,         1, 10, 2010 }, HDMI_VIDEO_TIMING_BUTT, 200,   0 },
    {{ "TCL",    0,         0, 20, 2013 }, HDMI_VIDEO_TIMING_BUTT, 500, 500 },
    {{ "DWE",  533, 825241648,  7, 2019 }, HDMI_VIDEO_TIMING_BUTT, 600, 500 },
    {{ "ZDH", 8950,  16843009,  0, 2015 }, HDMI_VIDEO_TIMING_BUTT, 500, 500 }
};

static compat_sink_stop_delay g_sink_stop_delay[] = {
    /* name prod_code serial_no week year stop_delay */
    {{ "SKW", 48,        1, 1, 2008 }, 1000 },
    {{ "GSM",  1, 16843009, 1, 2011 }, 1000 },
    {{ "LTV", 48,        1, 6, 2014 }, 1000 }
};

static compat_edid_manufacture_info g_avi_use_gen5[] = {
    /* name  prod_code serial_no week  year */
    { "SHP",   4485,     0,       255, 2018, }, /* Sharp */
    { "SHP",   4488,     0,       255, 2018, }, /* Sharp */
    { "SHP",   4491,     0,       255, 2018, }, /* Sharp */
    { "SHP",   4494,     0,       255, 2018, }, /* Sharp */
    { "SHP",   4506,     0,       255, 2018, }, /* Sharp */
    { "SHP",   4507,     0,       255, 2018, }, /* Sharp */
    { "SHP",   9301,     0,       255, 2018, }, /* Sharp */
    { "SHP",   9304,     0,       255, 2018, }, /* Sharp */
    { "ATO",  45098,     0,        13, 2019, }, /* Astro */
};

compatibility_info *compat_info_get(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        return &g_compat_info[hdmi_id];
    }

    return HI_NULL;
}

static hi_bool compat_edid_match_chk(const compat_edid_manufacture_info *tmp_manu_info,
    const hdmi_edid_manufacture_info *sink_manu_info)
{
    hi_bool match = HI_FALSE;

    match = (tmp_manu_info->mfrs_name[0] == sink_manu_info->mfrs_name[0]) &&
            (tmp_manu_info->mfrs_name[1] == sink_manu_info->mfrs_name[1]) &&
            (tmp_manu_info->mfrs_name[2] == sink_manu_info->mfrs_name[2]) &&
            (tmp_manu_info->mfrs_name[3] == sink_manu_info->mfrs_name[3]) &&
            (tmp_manu_info->product_code == sink_manu_info->product_code) &&
            (tmp_manu_info->serial_number == sink_manu_info->serial_number) &&
            (tmp_manu_info->week == sink_manu_info->week) &&
            (tmp_manu_info->year == sink_manu_info->year);

    return match;
}

hi_s32 drv_hdmi_compat_hdcp_ms_get(hdmi_device_id hdmi_id, hi_u64 *wait_ms)
{
    hi_u32 i;
    hdmi_sink_capability         *sink_cap       = HI_NULL;
    compat_hdcp_ms               *hdcp_ms        = HI_NULL;
    compat_edid_manufacture_info *tmp_manu_info  = HI_NULL;
    hdmi_edid_manufacture_info   *sink_manu_info = HI_NULL;
    hdmi_device                  *hdmi_dev       = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    hdmi_if_null_return(wait_ms, HI_FAILURE);

    /* default value */
    *wait_ms = 500; /* default wait time 500 ms */

    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
        return HI_FAILURE;
    } else {
        sink_manu_info = &sink_cap->mfrs_info;

        for (i = 0, hdcp_ms = &g_sink_hdcp_ms[0];
             i < hdmi_array_size(g_sink_hdcp_ms);
             hdcp_ms++, i++) {
            tmp_manu_info = &hdcp_ms->edid;

            if (compat_edid_match_chk(tmp_manu_info, sink_manu_info)) {
                *wait_ms = hdmi_dev->attr.vo_attr.hdmi_adapt_pix_clk >= FMT_PIX_CLK_340000 ? hdcp_ms->ms_over340_m :
                    hdcp_ms->ms_under340_m;
                break;
            }
        }
    }

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_compat_avmute_get(hdmi_device_id hdmi_id, hi_bool avmute, hdmi_avmute_cfg *avmute_cfg)
{
    hi_u32 i;
    hdmi_sink_capability         *sink_cap       = HI_NULL;
    compat_sink_avmute           *sink_avmute    = HI_NULL;
    compat_edid_manufacture_info *tmp_manu_info  = HI_NULL;
    hdmi_edid_manufacture_info   *sink_manu_info = HI_NULL;
    hdmi_device                  *hdmi_dev       = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    hdmi_if_null_return(avmute_cfg, HI_FAILURE);

    /* default value */
    sink_avmute = &g_sink_avmute[0];
    avmute_cfg->mute_pkg_en = sink_avmute->avmute_cfg.mute_pkg_en;
    avmute_cfg->mute_set    = avmute;
    avmute_cfg->mute_clr    = (!avmute) && sink_avmute->avmute_cfg.mute_clr;
    avmute_cfg->mute_rpt_en = (!avmute);
    avmute_cfg->rpt_cnt     = sink_avmute->avmute_cfg.rpt_cnt;

    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
        return HI_FAILURE;
    }

    sink_manu_info = &sink_cap->mfrs_info;

    for (i = 1, sink_avmute = &g_sink_avmute[1];
         i < hdmi_array_size(g_sink_avmute);
         sink_avmute++, i++) {
        tmp_manu_info = &sink_avmute->edid;

        if (compat_edid_match_chk(tmp_manu_info, sink_manu_info) == HI_FALSE) {
            continue;
        }
        if ((hdmi_dev->attr.vo_attr.video_timing == sink_avmute->timing) ||
            (sink_avmute->timing == HDMI_VIDEO_TIMING_BUTT)) {
            /* HDMI_VIDEO_TIMING_BUTT means all timing use the same config to the TV. */
            avmute_cfg->mute_pkg_en = sink_avmute->avmute_cfg.mute_pkg_en;
            avmute_cfg->mute_set    = avmute;
            avmute_cfg->mute_clr    = (!avmute) && sink_avmute->avmute_cfg.mute_clr;
            avmute_cfg->mute_rpt_en = sink_avmute->avmute_cfg.mute_rpt_en;
            avmute_cfg->rpt_cnt     = sink_avmute->avmute_cfg.rpt_cnt;
            break;
        }
    }

    return HI_SUCCESS;
}

hi_bool drv_hdmi_compat_scdc_in_start_get(hdmi_device_id hdmi_id)
{
    hi_u32 i;
    hi_bool                      in_start       = HI_FALSE;
    hdmi_sink_capability         *sink_cap       = HI_NULL;
    compat_edid_manufacture_info *tmp_manu_info  = HI_NULL;
    hdmi_edid_manufacture_info   *sink_manu_info = HI_NULL;
    hdmi_device                  *hdmi_dev       = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FALSE);

    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
    } else {
        sink_manu_info = &sink_cap->mfrs_info;

        for (i = 0, tmp_manu_info = &g_sink_scdc_pos[0];
             i < hdmi_array_size(g_sink_scdc_pos);
             tmp_manu_info++, i++) {
            if (compat_edid_match_chk(tmp_manu_info, sink_manu_info)) {
                in_start = HI_TRUE;
                break;
            }
        }
    }

    return in_start;
}

hi_s32 drv_hdmi_compat_delay_get(hdmi_device_id hdmi_id, hdmi_delay *delay)
{
    hi_u32 i;
    hdmi_sink_capability         *sink_cap       = HI_NULL;
    compat_sink_delay            *sink_delay     = HI_NULL;
    compat_edid_manufacture_info *tmp_manu_info  = HI_NULL;
    hdmi_edid_manufacture_info   *sink_manu_info = HI_NULL;
    hdmi_device                  *hdmi_dev       = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);
    hdmi_if_null_return(delay, HI_FAILURE);

    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
        return HI_FAILURE;
    }
    sink_manu_info = &sink_cap->mfrs_info;
    for (i = 0, sink_delay = &g_sink_delay[0]; i < hdmi_array_size(g_sink_delay); sink_delay++, i++) {
        tmp_manu_info = &sink_delay->edid;
        if (compat_edid_match_chk(tmp_manu_info, sink_manu_info) == HI_FALSE) {
            continue;
        }
        if ((hdmi_dev->attr.vo_attr.video_timing == sink_delay->timing) ||
            (sink_delay->timing == HDMI_VIDEO_TIMING_BUTT)) {
            /* HDMI_VIDEO_TIMING_BUTT means all timing use the same config to the TV. */
            delay->fmt_delay = sink_delay->fmt_delay;
            delay->mute_delay = sink_delay->mute_delay;
            break;
        }
    }

    return HI_SUCCESS;
}

hi_s32 drv_hdmi_compat_stop_delay(hdmi_device_id hdmi_id)
{
    hi_u32 i;
    hdmi_sink_capability         *sink_cap        = HI_NULL;
    compat_sink_stop_delay       *sink_stop_delay = HI_NULL;
    compat_edid_manufacture_info *tmp_manu_info   = HI_NULL;
    hdmi_edid_manufacture_info   *sink_manu_info  = HI_NULL;
    hdmi_device                  *hdmi_dev        = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_cap) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability fail\n");
        return HI_FAILURE;
    } else {
        sink_manu_info = &sink_cap->mfrs_info;

        for (i = 0, sink_stop_delay = &g_sink_stop_delay[0];
             i < hdmi_array_size(g_sink_stop_delay);
             sink_stop_delay++, i++) {
            tmp_manu_info = &sink_stop_delay->edid;

            if (compat_edid_match_chk(tmp_manu_info, sink_manu_info)) {
                hdmi_info("set stop delay = %ums.\n", sink_stop_delay->stop_delay);
                osal_msleep(sink_stop_delay->stop_delay);
                break;
            }
        }
    }

    return HI_SUCCESS;
}

static hi_bool compat_avi_use_gen5(const hdmi_sink_capability *sink_caps)
{
    hi_u32 i;
    compat_edid_manufacture_info *tmp_manuinfo = HI_NULL;

    hdmi_if_null_return(sink_caps, HI_FALSE);

    for (i = 0, tmp_manuinfo = &g_avi_use_gen5[0];
         i < hdmi_array_size(g_avi_use_gen5);
         tmp_manuinfo++, i++) {
        if (compat_edid_match_chk(tmp_manuinfo, &sink_caps->mfrs_info) ||
            ((sink_caps->mfrs_info.mfrs_name[0] == 'S') &&
            (sink_caps->mfrs_info.mfrs_name[1] == 'H') &&
            (sink_caps->mfrs_info.mfrs_name[2] == 'P') &&
            (tmp_manuinfo->product_code == sink_caps->mfrs_info.product_code))) {
            return HI_TRUE;
        }
    }

    return HI_FALSE;
}

hi_void compatibility_info_default_set(hdmi_device_id hdmi_id)
{
    compatibility_info *compat_info = HI_NULL;

    compat_info = compat_info_get(hdmi_id);
    hdmi_if_null_return_void(compat_info);

    compat_info->avi_use_gen5 = HI_FALSE;
    compat_info->avi_use_gen5_debug = HI_FALSE;
    compat_info->ctl_type_config = FRL_CTRL_TYPE_COMPRESS_HW;

    return;
}

hi_s32 compatibility_info_update(hdmi_device_id hdmi_id)
{
    hdmi_device          *hdmi_dev    = HI_NULL;
    hdmi_sink_capability *sink_caps   = HI_NULL;
    compatibility_info   *compat_info = HI_NULL;

    hdmi_dev = get_hdmi_device(hdmi_id);
    hdmi_if_null_return(hdmi_dev, HI_FAILURE);

    compat_info = compat_info_get(hdmi_id);
    hdmi_if_null_return(compat_info, HI_FAILURE);

    if (drv_hdmi_edid_capability_get(&hdmi_dev->edid_info, &sink_caps) == HDMI_EDID_DATA_INVALID) {
        hdmi_warn("get sink capability failed.\n");
        compatibility_info_default_set(hdmi_id);
        return HI_FAILURE;
    } else {
        if (compat_avi_use_gen5(sink_caps)) {
            if (compat_info->avi_use_gen5_debug == HI_FALSE) {
                hdmi_info("use gen5 send avi.\n");
                compat_info->avi_use_gen5 = HI_TRUE;
            }
            if (!!(compat_info->ctl_type_config & HDMI_FRL_COMPRESS_DEBUG_MASK) == HI_FALSE) {
                hdmi_info("frl not compress.\n");
                compat_info->ctl_type_config = FRL_CTRL_TYPE_COMPRESS_NON;
            }
        } else {
            compat_info->avi_use_gen5 = HI_FALSE;
            compat_info->ctl_type_config = FRL_CTRL_TYPE_COMPRESS_HW;
        }
    }

    return HI_SUCCESS;
}


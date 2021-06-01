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

#include "hdmi_hal_ncts.h"
#include "hdmi_hal_intf.h"
#include "hdmi_product_define.h"

#define NCTS_TMDS_DEVIATION 20
#define NCTS_INVALID_DATA   0xffffffff
#define NCTS_CALC_FACTOR    128
#define NCTS_N_DEFAULT      6144

static hdmi_audio_cts_n g_audio_cts_n[] = {
    /* rate   TMDS    N      CTS */
    { 32000,  25174,  4576,  28125 },
    { 32000,  25200,  4096,  25200 },
    { 32000,  27000,  4096,  27000 },
    { 32000,  27027,  4096,  27027 },
    { 32000,  54000,  4096,  54000 },
    { 32000,  54054,  4096,  54054 },
    { 32000,  74175, 11648, 210937 },
    { 32000,  74250,  4096,  74250 },
    { 32000, 148351, 11648, 421875 },
    { 32000, 148500,  4096, 148500 },
    { 32000, 296703,  5824, 421875 },
    { 32000, 297000,  3072, 222750 },
    { 32000, 593406,  5824, 843750 },
    { 32000, 594000,  3072, 445500 },
    { 32000,      0,  4096,      0 },
    { 44100,  25174,  7007,  31250 },
    { 44100,  25200,  6272,  28000 },
    { 44100,  27000,  6272,  30000 },
    { 44100,  27027,  6272,  30030 },
    { 44100,  54000,  6272,  60000 },
    { 44100,  54054,  6272,  60060 },
    { 44100,  74175, 17836, 234375 },
    { 44100,  74250,  6272,  82500 },
    { 44100, 148351,  8918, 234375 },
    { 44100, 148500,  6272, 165000 },
    { 44100, 296703,  4459, 234375 },
    { 44100, 297000,  4704, 247500 },
    { 44100, 593406,  8918, 937500 },
    { 44100, 594000,  9408, 990000 },
    { 44100,      0,  6272,      0 },
    { 48000,  25174,  6864,  28125 },
    { 48000,  25200,  6144,  25200 },
    { 48000,  27000,  6144,  27000 },
    { 48000,  27027,  6144,  27027 },
    { 48000,  54000,  6144,  54000 },
    { 48000,  54054,  6144,  54054 },
    { 48000,  74175, 11648, 140625 },
    { 48000,  74250,  6144,  74250 },
    { 48000, 148351,  5824, 140625 },
    { 48000, 148500,  6144, 148500 },
    { 48000, 296703,  5824, 281250 },
    { 48000, 297000,  5120, 247500 },
    { 48000, 593406,  5824, 562500 },
    { 48000, 594000,  6144, 594000 },
    { 48000,      0,  6144,      0 }
};

hi_u32 hal_hdmi_n_value_get(hi_u32 sample_rate, hi_u32 tmds_clk)
{
    hi_u32 i;
    hdmi_audio_cts_n *audio_cts_n = &g_audio_cts_n[0];
    hi_u32 n_value = NCTS_N_DEFAULT;

    for (i = 0; i < hdmi_array_size(g_audio_cts_n); i++, audio_cts_n++) {
        if ((audio_cts_n->audio_smp_rate == sample_rate) && (audio_cts_n->tmds_clk == tmds_clk)) {
            n_value = audio_cts_n->n_value;
            break;
        } else if ((audio_cts_n->audio_smp_rate == sample_rate) &&
                   (audio_cts_n->tmds_clk >= (tmds_clk - NCTS_TMDS_DEVIATION)) &&
                   (audio_cts_n->tmds_clk <= (tmds_clk + NCTS_TMDS_DEVIATION))) {
            n_value = audio_cts_n->n_value;
            break;
        } else if ((audio_cts_n->audio_smp_rate == sample_rate) && (audio_cts_n->tmds_clk == 0)) {
            n_value = audio_cts_n->n_value;
            break;
        }
    }

    return n_value;
}

hi_u32 hal_hdmi_cts_value_get(hi_u32 sample_rate, hi_u32 tmds_clk)
{
    hi_u32 i;
    hi_u32 tmp_cts = NCTS_INVALID_DATA;
    hi_u32 tmp_n   = NCTS_INVALID_DATA;
    hdmi_audio_cts_n *audio_cts_n = &g_audio_cts_n[0];

    for (i = 0; i < hdmi_array_size(g_audio_cts_n); i++, audio_cts_n++) {
        if ((audio_cts_n->audio_smp_rate == sample_rate) && (audio_cts_n->tmds_clk == tmds_clk)) {
            tmp_cts = audio_cts_n->cts_value;
            tmp_n = audio_cts_n->n_value;
            break;
        } else if ((audio_cts_n->audio_smp_rate == sample_rate) &&
                   (audio_cts_n->tmds_clk >= (tmds_clk - NCTS_TMDS_DEVIATION)) &&
                   (audio_cts_n->tmds_clk <= (tmds_clk + NCTS_TMDS_DEVIATION))) {
            tmp_cts = audio_cts_n->cts_value;
            tmp_n = audio_cts_n->n_value;
            break;
        } else if ((audio_cts_n->audio_smp_rate == sample_rate) && (audio_cts_n->tmds_clk == 0)) {
            tmp_cts = audio_cts_n->cts_value;
            tmp_n = audio_cts_n->n_value;
            break;
        }
    }

    /*
     *      (n * tmds_clk)
     * cts =-----------------
     *      (128 * samp_rate)
     */
    if ((tmp_cts == 0) && (sample_rate >= HDMI_HUNDRED)) {
        tmp_cts = (tmp_n / NCTS_CALC_FACTOR) * tmds_clk * HDMI_DECIMAL / (sample_rate / HDMI_HUNDRED);
    } else if ((tmp_cts == NCTS_INVALID_DATA) && (sample_rate >= HDMI_HUNDRED)) {
        /* can't find, N default 48k samprate */
        hdmi_warn("can't find cts! tmp_cts=%u,sample_rate=%u,tmds_clk=%u\n", tmp_cts, sample_rate, tmds_clk);
        tmp_cts = (NCTS_N_DEFAULT / NCTS_CALC_FACTOR) * tmds_clk * HDMI_DECIMAL / (sample_rate / HDMI_HUNDRED);
    }

    return tmp_cts;
}


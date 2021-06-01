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
#include "hdmi_reg_tx.h"
#include "hdmi_product_define.h"

volatile tx_hdmi_reg_regs_type *g_tx_hdmi_regs = NULL;

int hdmi_reg_tx_hdmi_regs_init(hi_char *addr)
{
    g_tx_hdmi_regs = (volatile tx_hdmi_reg_regs_type *)(addr + HDMI_TX_BASE_ADDR_HDMITX);
    return HI_SUCCESS;
}

int hdmi_reg_tx_hdmi_regs_deinit(void)
{
    if (g_tx_hdmi_regs != HI_NULL) {
        g_tx_hdmi_regs = HI_NULL;
    }
    return HI_SUCCESS;
}

int hdmi_reg_tmds_pack_mode_set(unsigned int tmds_pack_mode)
{
    hi_u32 *reg_addr = NULL;
    tx_pack_fifo_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->pack_fifo_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.tmds_pack_mode = tmds_pack_mode;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt_header_hb_set(unsigned int hb0, unsigned int hb1, unsigned int hb2)
{
    hi_u32 *reg_addr = NULL;
    avi_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_pkt_hb2 = hb2;
    tmp.bits.avi_pkt_hb1 = hb1;
    tmp.bits.avi_pkt_hb0 = hb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt0_low_set(unsigned int avi_pkt0_pb0,
    unsigned int avi_pkt0_pb1, unsigned int avi_pkt0_pb2, unsigned int avi_pkt0_pb3)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt0_pb3 = avi_pkt0_pb3;
    tmp.bits.avi_sub_pkt0_pb2 = avi_pkt0_pb2;
    tmp.bits.avi_sub_pkt0_pb1 = avi_pkt0_pb1;
    tmp.bits.avi_sub_pkt0_pb0 = avi_pkt0_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt0_high_set(unsigned int avi_pkt0_pb4, unsigned int avi_pkt0_pb5, unsigned int avi_pkt0_pb6)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt0_pb6 = avi_pkt0_pb6;
    tmp.bits.avi_sub_pkt0_pb5 = avi_pkt0_pb5;
    tmp.bits.avi_sub_pkt0_pb4 = avi_pkt0_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt1_low_set(unsigned int avi_pkt1_pb0,
    unsigned int avi_pkt1_pb1, unsigned int avi_pkt1_pb2, unsigned int avi_pkt1_pb3)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt1_pb3 = avi_pkt1_pb3;
    tmp.bits.avi_sub_pkt1_pb2 = avi_pkt1_pb2;
    tmp.bits.avi_sub_pkt1_pb1 = avi_pkt1_pb1;
    tmp.bits.avi_sub_pkt1_pb0 = avi_pkt1_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt1_high_set(unsigned int avi_pkt1_pb4, unsigned int avi_pkt1_pb5, unsigned int avi_pkt1_pb6)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt1_pb6 = avi_pkt1_pb6;
    tmp.bits.avi_sub_pkt1_pb5 = avi_pkt1_pb5;
    tmp.bits.avi_sub_pkt1_pb4 = avi_pkt1_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt2_low_set(unsigned int avi_pkt2_pb0,
    unsigned int avi_pkt2_pb1, unsigned int avi_pkt2_pb2, unsigned int avi_pkt2_pb3)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt2_pb3 = avi_pkt2_pb3;
    tmp.bits.avi_sub_pkt2_pb2 = avi_pkt2_pb2;
    tmp.bits.avi_sub_pkt2_pb1 = avi_pkt2_pb1;
    tmp.bits.avi_sub_pkt2_pb0 = avi_pkt2_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt2_high_set(unsigned int avi_pkt2_pb4, unsigned int avi_pkt2_pb5, unsigned int avi_pkt2_pb6)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt2_pb6 = avi_pkt2_pb6;
    tmp.bits.avi_sub_pkt2_pb5 = avi_pkt2_pb5;
    tmp.bits.avi_sub_pkt2_pb4 = avi_pkt2_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt3_low_set(unsigned int avi_pkt3_pb0,
                              unsigned int avi_pkt3_pb1, unsigned int avi_pkt3_pb2, unsigned int avi_pkt3_pb3)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt3_pb3 = avi_pkt3_pb3;
    tmp.bits.avi_sub_pkt3_pb2 = avi_pkt3_pb2;
    tmp.bits.avi_sub_pkt3_pb1 = avi_pkt3_pb1;
    tmp.bits.avi_sub_pkt3_pb0 = avi_pkt3_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt3_high_set(unsigned int avi_pkt3_pb4, unsigned int avi_pkt3_pb5, unsigned int avi_pkt3_pb6)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.avi_sub_pkt3_pb6 = avi_pkt3_pb6;
    tmp.bits.avi_sub_pkt3_pb5 = avi_pkt3_pb5;
    tmp.bits.avi_sub_pkt3_pb4 = avi_pkt3_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt_header_hb_get(avi_pkt_header *avi_header)
{
    hi_u32 *reg_addr = NULL;
    avi_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_header->bits.avi_pkt_hb2 = tmp.bits.avi_pkt_hb2;
    avi_header->bits.avi_pkt_hb1 = tmp.bits.avi_pkt_hb1;
    avi_header->bits.avi_pkt_hb0 = tmp.bits.avi_pkt_hb0;

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt0_low_get(avi_sub_pkt0_low *avi_pkt0_low)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt0_low->bits.avi_sub_pkt0_pb3 = tmp.bits.avi_sub_pkt0_pb3;
    avi_pkt0_low->bits.avi_sub_pkt0_pb2 = tmp.bits.avi_sub_pkt0_pb2;
    avi_pkt0_low->bits.avi_sub_pkt0_pb1 = tmp.bits.avi_sub_pkt0_pb1;
    avi_pkt0_low->bits.avi_sub_pkt0_pb0 = tmp.bits.avi_sub_pkt0_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt0_high_get(avi_sub_pkt0_high *avi_pkt0_high)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt0_high->bits.avi_sub_pkt0_pb6 = tmp.bits.avi_sub_pkt0_pb6;
    avi_pkt0_high->bits.avi_sub_pkt0_pb5 = tmp.bits.avi_sub_pkt0_pb5;
    avi_pkt0_high->bits.avi_sub_pkt0_pb4 = tmp.bits.avi_sub_pkt0_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt1_low_get(avi_sub_pkt1_low *avi_pkt1_low)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt1_low->bits.avi_sub_pkt1_pb3 = tmp.bits.avi_sub_pkt1_pb3;
    avi_pkt1_low->bits.avi_sub_pkt1_pb2 = tmp.bits.avi_sub_pkt1_pb2;
    avi_pkt1_low->bits.avi_sub_pkt1_pb1 = tmp.bits.avi_sub_pkt1_pb1;
    avi_pkt1_low->bits.avi_sub_pkt1_pb0 = tmp.bits.avi_sub_pkt1_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt1_high_get(avi_sub_pkt1_high *avi_pkt1_h)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt1_h->bits.avi_sub_pkt1_pb6 = tmp.bits.avi_sub_pkt1_pb6;
    avi_pkt1_h->bits.avi_sub_pkt1_pb5 = tmp.bits.avi_sub_pkt1_pb5;
    avi_pkt1_h->bits.avi_sub_pkt1_pb4 = tmp.bits.avi_sub_pkt1_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_avi_pkt2_low_get(avi_sub_pkt2_low *avi_pkt2_l)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt2_l->bits.avi_sub_pkt2_pb3 = tmp.bits.avi_sub_pkt2_pb3;
    avi_pkt2_l->bits.avi_sub_pkt2_pb2 = tmp.bits.avi_sub_pkt2_pb2;
    avi_pkt2_l->bits.avi_sub_pkt2_pb1 = tmp.bits.avi_sub_pkt2_pb1;
    avi_pkt2_l->bits.avi_sub_pkt2_pb0 = tmp.bits.avi_sub_pkt2_pb0;

    return HI_SUCCESS;
}

void hdmi_reg_avi_pkt2_high_get(avi_sub_pkt2_high *avi_pkt2_h)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt2_h->bits.avi_sub_pkt2_pb6 = tmp.bits.avi_sub_pkt2_pb6;
    avi_pkt2_h->bits.avi_sub_pkt2_pb5 = tmp.bits.avi_sub_pkt2_pb5;
    avi_pkt2_h->bits.avi_sub_pkt2_pb4 = tmp.bits.avi_sub_pkt2_pb4;

    return;
}

void hdmi_reg_avi_pkt3_low_get(avi_sub_pkt3_low *avi_pkt3_low)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt3_low->bits.avi_sub_pkt3_pb3 = tmp.bits.avi_sub_pkt3_pb3;
    avi_pkt3_low->bits.avi_sub_pkt3_pb2 = tmp.bits.avi_sub_pkt3_pb2;
    avi_pkt3_low->bits.avi_sub_pkt3_pb1 = tmp.bits.avi_sub_pkt3_pb1;
    avi_pkt3_low->bits.avi_sub_pkt3_pb0 = tmp.bits.avi_sub_pkt3_pb0;

    return;
}

void hdmi_reg_avi_pkt3_high_get(avi_sub_pkt3_high *avi_pkt3_high)
{
    hi_u32 *reg_addr = NULL;
    avi_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    avi_pkt3_high->bits.avi_sub_pkt3_pb6 = tmp.bits.avi_sub_pkt3_pb6;
    avi_pkt3_high->bits.avi_sub_pkt3_pb5 = tmp.bits.avi_sub_pkt3_pb5;
    avi_pkt3_high->bits.avi_sub_pkt3_pb4 = tmp.bits.avi_sub_pkt3_pb4;

    return;
}

int hdmi_reg_audio_pkt_header_set(unsigned int hb0, unsigned int hb1, unsigned int hb2)
{
    hi_u32 *reg_addr = NULL;
    aif_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_pkt_hb2 = hb2;
    tmp.bits.aif_pkt_hb1 = hb1;
    tmp.bits.aif_pkt_hb0 = hb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt0_low_set(unsigned int audio_pkt0_pb0,
    unsigned int audio_pkt0_pb1, unsigned int audio_pkt0_pb2, unsigned int audio_pkt0_pb3)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt0_pb3 = audio_pkt0_pb3;
    tmp.bits.aif_sub_pkt0_pb2 = audio_pkt0_pb2;
    tmp.bits.aif_sub_pkt0_pb1 = audio_pkt0_pb1;
    tmp.bits.aif_sub_pkt0_pb0 = audio_pkt0_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt0_high_set(unsigned int audio_pkt0_pb4,
    unsigned int audio_pkt0_pb5, unsigned int audio_pkt0_pb6)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt0_pb6 = audio_pkt0_pb6;
    tmp.bits.aif_sub_pkt0_pb5 = audio_pkt0_pb5;
    tmp.bits.aif_sub_pkt0_pb4 = audio_pkt0_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt1_low_set(unsigned int audio_pkt1_pb0,
    unsigned int audio_pkt1_pb1, unsigned int audio_pkt1_pb2, unsigned int audio_pkt1_pb3)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt1_pb3 = audio_pkt1_pb3;
    tmp.bits.aif_sub_pkt1_pb2 = audio_pkt1_pb2;
    tmp.bits.aif_sub_pkt1_pb1 = audio_pkt1_pb1;
    tmp.bits.aif_sub_pkt1_pb0 = audio_pkt1_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt1_high_set(unsigned int audio_pkt1_pb4,
    unsigned int audio_pkt1_pb5, unsigned int audio_pkt1_pb6)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt1_pb6 = audio_pkt1_pb6;
    tmp.bits.aif_sub_pkt1_pb5 = audio_pkt1_pb5;
    tmp.bits.aif_sub_pkt1_pb4 = audio_pkt1_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt2_low_set(unsigned int audio_pkt2_pb0,
    unsigned int audio_pkt2_pb1, unsigned int audio_pkt2_pb2, unsigned int audio_pkt2_pb3)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt2_pb3 = audio_pkt2_pb3;
    tmp.bits.aif_sub_pkt2_pb2 = audio_pkt2_pb2;
    tmp.bits.aif_sub_pkt2_pb1 = audio_pkt2_pb1;
    tmp.bits.aif_sub_pkt2_pb0 = audio_pkt2_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt2_high_set(unsigned int audio_pkt2_pb4,
    unsigned int audio_pkt2_pb5, unsigned int audio_pkt2_pb6)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt2_pb6 = audio_pkt2_pb6;
    tmp.bits.aif_sub_pkt2_pb5 = audio_pkt2_pb5;
    tmp.bits.aif_sub_pkt2_pb4 = audio_pkt2_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt3_low_set(unsigned int audio_pkt3_pb0,
    unsigned int audio_pkt3_pb1, unsigned int audio_pkt3_pb2, unsigned int audio_pkt3_pb3)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt3_pb3 = audio_pkt3_pb3;
    tmp.bits.aif_sub_pkt3_pb2 = audio_pkt3_pb2;
    tmp.bits.aif_sub_pkt3_pb1 = audio_pkt3_pb1;
    tmp.bits.aif_sub_pkt3_pb0 = audio_pkt3_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_audio_pkt3_high_set(unsigned int audio_pkt3_pb4,
    unsigned int audio_pkt3_pb5, unsigned int audio_pkt3_pb6)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.aif_sub_pkt3_pb6 = audio_pkt3_pb6;
    tmp.bits.aif_sub_pkt3_pb5 = audio_pkt3_pb5;
    tmp.bits.aif_sub_pkt3_pb4 = audio_pkt3_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt_header_get(aif_pkt_header *aif_header)
{
    hi_u32 *reg_addr = NULL;
    aif_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_header->bits.aif_pkt_hb2 = tmp.bits.aif_pkt_hb2;
    aif_header->bits.aif_pkt_hb1 = tmp.bits.aif_pkt_hb1;
    aif_header->bits.aif_pkt_hb0 = tmp.bits.aif_pkt_hb0;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt0_low_get(aif_sub_pkt0_low *aif_pkt0_low)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt0_low->bits.aif_sub_pkt0_pb3 = tmp.bits.aif_sub_pkt0_pb3;
    aif_pkt0_low->bits.aif_sub_pkt0_pb2 = tmp.bits.aif_sub_pkt0_pb2;
    aif_pkt0_low->bits.aif_sub_pkt0_pb1 = tmp.bits.aif_sub_pkt0_pb1;
    aif_pkt0_low->bits.aif_sub_pkt0_pb0 = tmp.bits.aif_sub_pkt0_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt0_high_get(aif_sub_pkt0_high *aif_pkt0_high)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt0_high->bits.aif_sub_pkt0_pb6 = tmp.bits.aif_sub_pkt0_pb6;
    aif_pkt0_high->bits.aif_sub_pkt0_pb5 = tmp.bits.aif_sub_pkt0_pb5;
    aif_pkt0_high->bits.aif_sub_pkt0_pb4 = tmp.bits.aif_sub_pkt0_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt1_low_get(aif_sub_pkt1_low *aif_pkt1_low)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt1_low->bits.aif_sub_pkt1_pb3 = tmp.bits.aif_sub_pkt1_pb3;
    aif_pkt1_low->bits.aif_sub_pkt1_pb2 = tmp.bits.aif_sub_pkt1_pb2;
    aif_pkt1_low->bits.aif_sub_pkt1_pb1 = tmp.bits.aif_sub_pkt1_pb1;
    aif_pkt1_low->bits.aif_sub_pkt1_pb0 = tmp.bits.aif_sub_pkt1_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt1_high_get(aif_sub_pkt1_high *aif_pkt1_high)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt1_high->bits.aif_sub_pkt1_pb6 = tmp.bits.aif_sub_pkt1_pb6;
    aif_pkt1_high->bits.aif_sub_pkt1_pb5 = tmp.bits.aif_sub_pkt1_pb5;
    aif_pkt1_high->bits.aif_sub_pkt1_pb4 = tmp.bits.aif_sub_pkt1_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt2_low_get(aif_sub_pkt2_low *aif_pkt2_low)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt2_low->bits.aif_sub_pkt2_pb3 = tmp.bits.aif_sub_pkt2_pb3;
    aif_pkt2_low->bits.aif_sub_pkt2_pb2 = tmp.bits.aif_sub_pkt2_pb2;
    aif_pkt2_low->bits.aif_sub_pkt2_pb1 = tmp.bits.aif_sub_pkt2_pb1;
    aif_pkt2_low->bits.aif_sub_pkt2_pb0 = tmp.bits.aif_sub_pkt2_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt2_high_get(aif_sub_pkt2_high *aif_pkt2_high)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt2_high->bits.aif_sub_pkt2_pb6 = tmp.bits.aif_sub_pkt2_pb6;
    aif_pkt2_high->bits.aif_sub_pkt2_pb5 = tmp.bits.aif_sub_pkt2_pb5;
    aif_pkt2_high->bits.aif_sub_pkt2_pb4 = tmp.bits.aif_sub_pkt2_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt3_low_get(aif_sub_pkt3_low *aif_pkt3_low)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt3_low->bits.aif_sub_pkt3_pb3 = tmp.bits.aif_sub_pkt3_pb3;
    aif_pkt3_low->bits.aif_sub_pkt3_pb2 = tmp.bits.aif_sub_pkt3_pb2;
    aif_pkt3_low->bits.aif_sub_pkt3_pb1 = tmp.bits.aif_sub_pkt3_pb1;
    aif_pkt3_low->bits.aif_sub_pkt3_pb0 = tmp.bits.aif_sub_pkt3_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_aif_pkt3_high_get(aif_sub_pkt3_high *aif_pkt3_high)
{
    hi_u32 *reg_addr = NULL;
    aif_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aif_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    aif_pkt3_high->bits.aif_sub_pkt3_pb6 = tmp.bits.aif_sub_pkt3_pb6;
    aif_pkt3_high->bits.aif_sub_pkt3_pb5 = tmp.bits.aif_sub_pkt3_pb5;
    aif_pkt3_high->bits.aif_sub_pkt3_pb4 = tmp.bits.aif_sub_pkt3_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt_header_set(unsigned int hb0, unsigned int hb1, unsigned int hb2)
{
    hi_u32 *reg_addr = NULL;
    gen_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_pkt_hb2 = hb2;
    tmp.bits.gen_pkt_hb1 = hb1;
    tmp.bits.gen_pkt_hb0 = hb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt0_low_set(unsigned int gen_pkt0_pb0,
    unsigned int gen_pkt0_pb1, unsigned int gen_pkt0_pb2, unsigned int gen_pkt0_pb3)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt0_pb3 = gen_pkt0_pb3;
    tmp.bits.gen_sub_pkt0_pb2 = gen_pkt0_pb2;
    tmp.bits.gen_sub_pkt0_pb1 = gen_pkt0_pb1;
    tmp.bits.gen_sub_pkt0_pb0 = gen_pkt0_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt0_high_set(unsigned int gen_pkt0_pb4, unsigned int gen_pkt0_pb5, unsigned int gen_pkt0_pb6)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt0_pb6 = gen_pkt0_pb6;
    tmp.bits.gen_sub_pkt0_pb5 = gen_pkt0_pb5;
    tmp.bits.gen_sub_pkt0_pb4 = gen_pkt0_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt1_low_set(unsigned int gen_pkt1_pb0,
    unsigned int gen_pkt1_pb1, unsigned int gen_pkt1_pb2, unsigned int gen_pkt1_pb3)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt1_pb3 = gen_pkt1_pb3;
    tmp.bits.gen_sub_pkt1_pb2 = gen_pkt1_pb2;
    tmp.bits.gen_sub_pkt1_pb1 = gen_pkt1_pb1;
    tmp.bits.gen_sub_pkt1_pb0 = gen_pkt1_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt1_high_set(unsigned int gen_pkt1_pb4, unsigned int gen_pkt1_pb5, unsigned int gen_pkt1_pb6)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt1_pb6 = gen_pkt1_pb6;
    tmp.bits.gen_sub_pkt1_pb5 = gen_pkt1_pb5;
    tmp.bits.gen_sub_pkt1_pb4 = gen_pkt1_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt2_low_set(unsigned int gen_pkt2_pb0,
    unsigned int gen_pkt2_pb1, unsigned int gen_pkt2_pb2, unsigned int gen_pkt2_pb3)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt2_pb3 = gen_pkt2_pb3;
    tmp.bits.gen_sub_pkt2_pb2 = gen_pkt2_pb2;
    tmp.bits.gen_sub_pkt2_pb1 = gen_pkt2_pb1;
    tmp.bits.gen_sub_pkt2_pb0 = gen_pkt2_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt2_high_set(unsigned int gen_pkt2_pb4, unsigned int gen_pkt2_pb5, unsigned int gen_pkt2_pb6)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt2_pb6 = gen_pkt2_pb6;
    tmp.bits.gen_sub_pkt2_pb5 = gen_pkt2_pb5;
    tmp.bits.gen_sub_pkt2_pb4 = gen_pkt2_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt3_low_set(unsigned int gen_pkt3_pb0,
    unsigned int gen_pkt3_pb1, unsigned int gen_pkt3_pb2, unsigned int gen_pkt3_pb3)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt3_pb3 = gen_pkt3_pb3;
    tmp.bits.gen_sub_pkt3_pb2 = gen_pkt3_pb2;
    tmp.bits.gen_sub_pkt3_pb1 = gen_pkt3_pb1;
    tmp.bits.gen_sub_pkt3_pb0 = gen_pkt3_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt3_high_set(unsigned int gen_pkt3_pb4, unsigned int gen_pkt3_pb5, unsigned int gen_pkt3_pb6)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.gen_sub_pkt3_pb6 = gen_pkt3_pb6;
    tmp.bits.gen_sub_pkt3_pb5 = gen_pkt3_pb5;
    tmp.bits.gen_sub_pkt3_pb4 = gen_pkt3_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt_header_get(gen_pkt_header *gen_header)
{
    hi_u32 *reg_addr = NULL;
    gen_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_header->bits.gen_pkt_hb2 = tmp.bits.gen_pkt_hb2;
    gen_header->bits.gen_pkt_hb1 = tmp.bits.gen_pkt_hb1;
    gen_header->bits.gen_pkt_hb0 = tmp.bits.gen_pkt_hb0;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt0_low_get(gen_sub_pkt0_low *gen_pkt0_low)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt0_low->bits.gen_sub_pkt0_pb3 = tmp.bits.gen_sub_pkt0_pb3;
    gen_pkt0_low->bits.gen_sub_pkt0_pb2 = tmp.bits.gen_sub_pkt0_pb2;
    gen_pkt0_low->bits.gen_sub_pkt0_pb1 = tmp.bits.gen_sub_pkt0_pb1;
    gen_pkt0_low->bits.gen_sub_pkt0_pb0 = tmp.bits.gen_sub_pkt0_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt0_high_get(gen_sub_pkt0_high *gen_pkt0_high)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt0_high->bits.gen_sub_pkt0_pb6 = tmp.bits.gen_sub_pkt0_pb6;
    gen_pkt0_high->bits.gen_sub_pkt0_pb5 = tmp.bits.gen_sub_pkt0_pb5;
    gen_pkt0_high->bits.gen_sub_pkt0_pb4 = tmp.bits.gen_sub_pkt0_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt1_low_get(gen_sub_pkt1_low *gen_pkt1_low)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt1_low->bits.gen_sub_pkt1_pb3 = tmp.bits.gen_sub_pkt1_pb3;
    gen_pkt1_low->bits.gen_sub_pkt1_pb2 = tmp.bits.gen_sub_pkt1_pb2;
    gen_pkt1_low->bits.gen_sub_pkt1_pb1 = tmp.bits.gen_sub_pkt1_pb1;
    gen_pkt1_low->bits.gen_sub_pkt1_pb0 = tmp.bits.gen_sub_pkt1_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt1_high_get(gen_sub_pkt1_high *gen_pkt1_high)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt1_high->bits.gen_sub_pkt1_pb6 = tmp.bits.gen_sub_pkt1_pb6;
    gen_pkt1_high->bits.gen_sub_pkt1_pb5 = tmp.bits.gen_sub_pkt1_pb5;
    gen_pkt1_high->bits.gen_sub_pkt1_pb4 = tmp.bits.gen_sub_pkt1_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt2_low_get(gen_sub_pkt2_low *gen_pkt2_low)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt2_low->bits.gen_sub_pkt2_pb3 = tmp.bits.gen_sub_pkt2_pb3;
    gen_pkt2_low->bits.gen_sub_pkt2_pb2 = tmp.bits.gen_sub_pkt2_pb2;
    gen_pkt2_low->bits.gen_sub_pkt2_pb1 = tmp.bits.gen_sub_pkt2_pb1;
    gen_pkt2_low->bits.gen_sub_pkt2_pb0 = tmp.bits.gen_sub_pkt2_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt2_high_get(gen_sub_pkt2_high *gen_pkt2_high)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt2_high->bits.gen_sub_pkt2_pb6 = tmp.bits.gen_sub_pkt2_pb6;
    gen_pkt2_high->bits.gen_sub_pkt2_pb5 = tmp.bits.gen_sub_pkt2_pb5;
    gen_pkt2_high->bits.gen_sub_pkt2_pb4 = tmp.bits.gen_sub_pkt2_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt3_low_get(gen_sub_pkt3_low *gen_pkt3_low)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt3_low->bits.gen_sub_pkt3_pb3 = tmp.bits.gen_sub_pkt3_pb3;
    gen_pkt3_low->bits.gen_sub_pkt3_pb2 = tmp.bits.gen_sub_pkt3_pb2;
    gen_pkt3_low->bits.gen_sub_pkt3_pb1 = tmp.bits.gen_sub_pkt3_pb1;
    gen_pkt3_low->bits.gen_sub_pkt3_pb0 = tmp.bits.gen_sub_pkt3_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gen_pkt3_high_get(gen_sub_pkt3_high *gen_pkt3_high)
{
    hi_u32 *reg_addr = NULL;
    gen_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gen_pkt3_high->bits.gen_sub_pkt3_pb6 = tmp.bits.gen_sub_pkt3_pb6;
    gen_pkt3_high->bits.gen_sub_pkt3_pb5 = tmp.bits.gen_sub_pkt3_pb5;
    gen_pkt3_high->bits.gen_sub_pkt3_pb4 = tmp.bits.gen_sub_pkt3_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt_header_get(gamut_pkt_header *gamut_header)
{
    hi_u32 *reg_addr = NULL;
    gamut_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_header->bits.gamut_pkt_hb2 = tmp.bits.gamut_pkt_hb2;
    gamut_header->bits.gamut_pkt_hb1 = tmp.bits.gamut_pkt_hb1;
    gamut_header->bits.gamut_pkt_hb0 = tmp.bits.gamut_pkt_hb0;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt0_low_get(gamut_sub_pkt0_low *gamut_pkt0_low)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt0_low->bits.gamut_sub_pkt0_pb3 = tmp.bits.gamut_sub_pkt0_pb3;
    gamut_pkt0_low->bits.gamut_sub_pkt0_pb2 = tmp.bits.gamut_sub_pkt0_pb2;
    gamut_pkt0_low->bits.gamut_sub_pkt0_pb1 = tmp.bits.gamut_sub_pkt0_pb1;
    gamut_pkt0_low->bits.gamut_sub_pkt0_pb0 = tmp.bits.gamut_sub_pkt0_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt0_high_get(gamut_sub_pkt0_high *gamut_pkt0_high)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt0_high->bits.gamut_sub_pkt0_pb6 = tmp.bits.gamut_sub_pkt0_pb6;
    gamut_pkt0_high->bits.gamut_sub_pkt0_pb5 = tmp.bits.gamut_sub_pkt0_pb5;
    gamut_pkt0_high->bits.gamut_sub_pkt0_pb4 = tmp.bits.gamut_sub_pkt0_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt1_low_get(gamut_sub_pkt1_low *gamut_pkt1_low)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt1_low->bits.gamut_sub_pkt1_pb3 = tmp.bits.gamut_sub_pkt1_pb3;
    gamut_pkt1_low->bits.gamut_sub_pkt1_pb2 = tmp.bits.gamut_sub_pkt1_pb2;
    gamut_pkt1_low->bits.gamut_sub_pkt1_pb1 = tmp.bits.gamut_sub_pkt1_pb1;
    gamut_pkt1_low->bits.gamut_sub_pkt1_pb0 = tmp.bits.gamut_sub_pkt1_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt1_high_get(gamut_sub_pkt1_high *gamut_pkt1_high)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt1_high->bits.gamut_sub_pkt1_pb6 = tmp.bits.gamut_sub_pkt1_pb6;
    gamut_pkt1_high->bits.gamut_sub_pkt1_pb5 = tmp.bits.gamut_sub_pkt1_pb5;
    gamut_pkt1_high->bits.gamut_sub_pkt1_pb4 = tmp.bits.gamut_sub_pkt1_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt2_low_get(gamut_sub_pkt2_low *gamut_pkt2_low)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt2_low->bits.gamut_sub_pkt2_pb3 = tmp.bits.gamut_sub_pkt2_pb3;
    gamut_pkt2_low->bits.gamut_sub_pkt2_pb2 = tmp.bits.gamut_sub_pkt2_pb2;
    gamut_pkt2_low->bits.gamut_sub_pkt2_pb1 = tmp.bits.gamut_sub_pkt2_pb1;
    gamut_pkt2_low->bits.gamut_sub_pkt2_pb0 = tmp.bits.gamut_sub_pkt2_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt2_high_get(gamut_sub_pkt2_high *gamut_pkt2_high)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt2_high->bits.gamut_sub_pkt2_pb6 = tmp.bits.gamut_sub_pkt2_pb6;
    gamut_pkt2_high->bits.gamut_sub_pkt2_pb5 = tmp.bits.gamut_sub_pkt2_pb5;
    gamut_pkt2_high->bits.gamut_sub_pkt2_pb4 = tmp.bits.gamut_sub_pkt2_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt3_low_get(gamut_sub_pkt3_low *gamut_pkt3_low)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt3_low->bits.gamut_sub_pkt3_pb3 = tmp.bits.gamut_sub_pkt3_pb3;
    gamut_pkt3_low->bits.gamut_sub_pkt3_pb2 = tmp.bits.gamut_sub_pkt3_pb2;
    gamut_pkt3_low->bits.gamut_sub_pkt3_pb1 = tmp.bits.gamut_sub_pkt3_pb1;
    gamut_pkt3_low->bits.gamut_sub_pkt3_pb0 = tmp.bits.gamut_sub_pkt3_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_gamut_pkt3_high_get(gamut_sub_pkt3_high *gamut_pkt3_high)
{
    hi_u32 *reg_addr = NULL;
    gamut_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    gamut_pkt3_high->bits.gamut_sub_pkt3_pb6 = tmp.bits.gamut_sub_pkt3_pb6;
    gamut_pkt3_high->bits.gamut_sub_pkt3_pb5 = tmp.bits.gamut_sub_pkt3_pb5;
    gamut_pkt3_high->bits.gamut_sub_pkt3_pb4 = tmp.bits.gamut_sub_pkt3_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_header_set(unsigned int hb0, unsigned int hb1, unsigned int hb2)
{
    hi_u32 *reg_addr = NULL;
    vsif_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_pkt_hb2 = hb2;
    tmp.bits.vsif_pkt_hb1 = hb1;
    tmp.bits.vsif_pkt_hb0 = hb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt0_low_set(unsigned int vsif_pkt0_pb0,
    unsigned int vsif_pkt0_pb1, unsigned int vsif_pkt0_pb2, unsigned int vsif_pkt0_pb3)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt0_pb3 = vsif_pkt0_pb3;
    tmp.bits.vsif_sub_pkt0_pb2 = vsif_pkt0_pb2;
    tmp.bits.vsif_sub_pkt0_pb1 = vsif_pkt0_pb1;
    tmp.bits.vsif_sub_pkt0_pb0 = vsif_pkt0_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt0_high_set(unsigned int vsif_pkt0_pb4, unsigned int vsif_pkt0_pb5, unsigned int vsif_pkt0_pb6)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt0_pb6 = vsif_pkt0_pb6;
    tmp.bits.vsif_sub_pkt0_pb5 = vsif_pkt0_pb5;
    tmp.bits.vsif_sub_pkt0_pb4 = vsif_pkt0_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt1_low_set(unsigned int vsif_pkt1_pb0,
    unsigned int vsif_pkt1_pb1, unsigned int vsif_pkt1_pb2, unsigned int vsif_pkt1_pb3)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt1_pb3 = vsif_pkt1_pb3;
    tmp.bits.vsif_sub_pkt1_pb2 = vsif_pkt1_pb2;
    tmp.bits.vsif_sub_pkt1_pb1 = vsif_pkt1_pb1;
    tmp.bits.vsif_sub_pkt1_pb0 = vsif_pkt1_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt1_high_set(unsigned int vsif_pkt1_pb4, unsigned int vsif_pkt1_pb5, unsigned int vsif_pkt1_pb6)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt1_pb6 = vsif_pkt1_pb6;
    tmp.bits.vsif_sub_pkt1_pb5 = vsif_pkt1_pb5;
    tmp.bits.vsif_sub_pkt1_pb4 = vsif_pkt1_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt2_low_set(unsigned int vsif_pkt2_pb0,
    unsigned int vsif_pkt2_pb1, unsigned int vsif_pkt2_pb2, unsigned int vsif_pkt2_pb3)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt2_pb3 = vsif_pkt2_pb3;
    tmp.bits.vsif_sub_pkt2_pb2 = vsif_pkt2_pb2;
    tmp.bits.vsif_sub_pkt2_pb1 = vsif_pkt2_pb1;
    tmp.bits.vsif_sub_pkt2_pb0 = vsif_pkt2_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt2_high_set(unsigned int vsif_pkt2_pb4, unsigned int vsif_pkt2_pb5, unsigned int vsif_pkt2_pb6)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt2_pb6 = vsif_pkt2_pb6;
    tmp.bits.vsif_sub_pkt2_pb5 = vsif_pkt2_pb5;
    tmp.bits.vsif_sub_pkt2_pb4 = vsif_pkt2_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt3_low_set(unsigned int vsif_pkt3_pb0,
    unsigned int vsif_pkt3_pb1, unsigned int vsif_pkt3_pb2, unsigned int vsif_pkt3_pb3)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt3_pb3 = vsif_pkt3_pb3;
    tmp.bits.vsif_sub_pkt3_pb2 = vsif_pkt3_pb2;
    tmp.bits.vsif_sub_pkt3_pb1 = vsif_pkt3_pb1;
    tmp.bits.vsif_sub_pkt3_pb0 = vsif_pkt3_pb0;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt3_high_set(unsigned int vsif_pkt3_pb4, unsigned int vsif_pkt3_pb5, unsigned int vsif_pkt3_pb6)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.vsif_sub_pkt3_pb6 = vsif_pkt3_pb6;
    tmp.bits.vsif_sub_pkt3_pb5 = vsif_pkt3_pb5;
    tmp.bits.vsif_sub_pkt3_pb4 = vsif_pkt3_pb4;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_avi_rpt_en_set(unsigned int cea_avi_rpt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_avi_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_avi_rpt_en = cea_avi_rpt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_avi_en_set(unsigned int cea_avi_en)
{
    hi_u32 *reg_addr = NULL;
    cea_avi_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_avi_en = cea_avi_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt_header_get(vsif_pkt_header *vsif_header)
{
    hi_u32 *reg_addr = NULL;
    vsif_pkt_header tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt_head.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_header->bits.vsif_pkt_hb2 = tmp.bits.vsif_pkt_hb2;
    vsif_header->bits.vsif_pkt_hb1 = tmp.bits.vsif_pkt_hb1;
    vsif_header->bits.vsif_pkt_hb0 = tmp.bits.vsif_pkt_hb0;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt0_low_get(vsif_sub_pkt0_low *vsif_pkt0_low)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt0_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt0l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt0_low->bits.vsif_sub_pkt0_pb3 = tmp.bits.vsif_sub_pkt0_pb3;
    vsif_pkt0_low->bits.vsif_sub_pkt0_pb2 = tmp.bits.vsif_sub_pkt0_pb2;
    vsif_pkt0_low->bits.vsif_sub_pkt0_pb1 = tmp.bits.vsif_sub_pkt0_pb1;
    vsif_pkt0_low->bits.vsif_sub_pkt0_pb0 = tmp.bits.vsif_sub_pkt0_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt0_high_get(vsif_sub_pkt0_high *vsif_pkt0_high)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt0_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt0h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt0_high->bits.vsif_sub_pkt0_pb6 = tmp.bits.vsif_sub_pkt0_pb6;
    vsif_pkt0_high->bits.vsif_sub_pkt0_pb5 = tmp.bits.vsif_sub_pkt0_pb5;
    vsif_pkt0_high->bits.vsif_sub_pkt0_pb4 = tmp.bits.vsif_sub_pkt0_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt1_low_get(vsif_sub_pkt1_low *vsif_pkt1_low)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt1_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt1l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt1_low->bits.vsif_sub_pkt1_pb3 = tmp.bits.vsif_sub_pkt1_pb3;
    vsif_pkt1_low->bits.vsif_sub_pkt1_pb2 = tmp.bits.vsif_sub_pkt1_pb2;
    vsif_pkt1_low->bits.vsif_sub_pkt1_pb1 = tmp.bits.vsif_sub_pkt1_pb1;
    vsif_pkt1_low->bits.vsif_sub_pkt1_pb0 = tmp.bits.vsif_sub_pkt1_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt1_high_get(vsif_sub_pkt1_high *vsif_pkt1_high)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt1_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt1h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt1_high->bits.vsif_sub_pkt1_pb6 = tmp.bits.vsif_sub_pkt1_pb6;
    vsif_pkt1_high->bits.vsif_sub_pkt1_pb5 = tmp.bits.vsif_sub_pkt1_pb5;
    vsif_pkt1_high->bits.vsif_sub_pkt1_pb4 = tmp.bits.vsif_sub_pkt1_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt2_low_get(vsif_sub_pkt2_low *vsif_pkt2_low)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt2_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt2l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt2_low->bits.vsif_sub_pkt2_pb3 = tmp.bits.vsif_sub_pkt2_pb3;
    vsif_pkt2_low->bits.vsif_sub_pkt2_pb2 = tmp.bits.vsif_sub_pkt2_pb2;
    vsif_pkt2_low->bits.vsif_sub_pkt2_pb1 = tmp.bits.vsif_sub_pkt2_pb1;
    vsif_pkt2_low->bits.vsif_sub_pkt2_pb0 = tmp.bits.vsif_sub_pkt2_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt2_high_get(vsif_sub_pkt2_high *vsif_pkt2_high)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt2_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt2h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt2_high->bits.vsif_sub_pkt2_pb6 = tmp.bits.vsif_sub_pkt2_pb6;
    vsif_pkt2_high->bits.vsif_sub_pkt2_pb5 = tmp.bits.vsif_sub_pkt2_pb5;
    vsif_pkt2_high->bits.vsif_sub_pkt2_pb4 = tmp.bits.vsif_sub_pkt2_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt3_low_get(vsif_sub_pkt3_low *vsif_pkt3_low)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt3_low tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt3l.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt3_low->bits.vsif_sub_pkt3_pb3 = tmp.bits.vsif_sub_pkt3_pb3;
    vsif_pkt3_low->bits.vsif_sub_pkt3_pb2 = tmp.bits.vsif_sub_pkt3_pb2;
    vsif_pkt3_low->bits.vsif_sub_pkt3_pb1 = tmp.bits.vsif_sub_pkt3_pb1;
    vsif_pkt3_low->bits.vsif_sub_pkt3_pb0 = tmp.bits.vsif_sub_pkt3_pb0;

    return HI_SUCCESS;
}

int hdmi_reg_vsif_pkt3_high_get(vsif_sub_pkt3_high *vsif_pkt3_high)
{
    hi_u32 *reg_addr = NULL;
    vsif_sub_pkt3_high tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_pkt3h.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_pkt3_high->bits.vsif_sub_pkt3_pb6 = tmp.bits.vsif_sub_pkt3_pb6;
    vsif_pkt3_high->bits.vsif_sub_pkt3_pb5 = tmp.bits.vsif_sub_pkt3_pb5;
    vsif_pkt3_high->bits.vsif_sub_pkt3_pb4 = tmp.bits.vsif_sub_pkt3_pb4;

    return HI_SUCCESS;
}

int hdmi_reg_cea_aud_rpt_en_set(unsigned int cea_aud_rpt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_aud_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aud_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_aud_rpt_en = cea_aud_rpt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_aud_en_set(unsigned int cea_aud_en)
{
    hi_u32 *reg_addr = NULL;
    cea_aud_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aud_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_aud_en = cea_aud_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_gen_rpt_en_set(unsigned int cea_gen_rpt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_gen_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_gen_rpt_en = cea_gen_rpt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_gen_en_set(unsigned int cea_gen_en)
{
    hi_u32 *reg_addr = NULL;
    cea_gen_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_gen_en = cea_gen_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_cp_rpt_cnt_set(unsigned int cea_cp_rpt_cnt)
{
    hi_u32 *reg_addr = NULL;
    cea_cp_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->cp_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_cp_rpt_cnt = cea_cp_rpt_cnt;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_cp_rpt_en_set(unsigned int cea_cp_rpt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_cp_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->cp_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_cp_rpt_en = cea_cp_rpt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_cp_en_set(unsigned int cea_cp_en)
{
    hi_u32 *reg_addr = NULL;
    cea_cp_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->cp_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_cp_en = cea_cp_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_gamut_rpt_en_set(unsigned int cea_gamut_rpt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_gamut_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_gamut_rpt_en = cea_gamut_rpt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_gamut_en_set(unsigned int cea_gamut_en)
{
    hi_u32 *reg_addr = NULL;
    cea_gamut_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_gamut_en = cea_gamut_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_vsif_rpt_en_set(unsigned int cea_vsif_rpt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_vsif_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_vsif_rpt_en = cea_vsif_rpt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cea_vsif_en_set(unsigned int cea_vsif_en)
{
    hi_u32 *reg_addr = NULL;
    cea_vsif_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cea_vsif_en = cea_vsif_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_eess_mode_en_set(unsigned int eess_mode_en)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;
    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.eess_mode_en = eess_mode_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_hdmi_dvi_sel_set(unsigned int hdmi_dvi_sel)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.hdmi_dvi_sel = hdmi_dvi_sel;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_dc_pkt_en_set(unsigned int dc_pkt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.dc_pkt_en = dc_pkt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_null_pkt_en_set(unsigned int null_pkt_en)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;
    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.null_pkt_en = null_pkt_en;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_null_pkt_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);

    return tmp.bits.null_pkt_en;
}

int hdmi_reg_hdmi_mode_set(unsigned int hdmi_mode)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.hdmi_mode = hdmi_mode;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cp_clr_avmute_set(unsigned int cp_clr_avmute)
{
    hi_u32 *reg_addr = NULL;
    cp_pkt_avmute tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->pkt_avmute.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cp_clr_avmute = cp_clr_avmute;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_cp_set_avmute_set(unsigned int cp_set_avmute)
{
    hi_u32 *reg_addr = NULL;
    cp_pkt_avmute tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->pkt_avmute.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.cp_set_avmute = cp_set_avmute;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_enc_bypass_set(unsigned int enc_bypass)
{
    hi_u32 *reg_addr = NULL;
    hdmi_enc_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->enc_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.enc_bypass = enc_bypass;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_enc_scr_on_set(unsigned int enc_scr_on)
{
    hi_u32 *reg_addr = NULL;
    hdmi_enc_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->enc_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.enc_scr_on = enc_scr_on;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

int hdmi_reg_enc_hdmi2_on_set(unsigned int enc_hdmi2_on)
{
    hi_u32 *reg_addr = NULL;
    hdmi_enc_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->enc_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    tmp.bits.enc_hdmi2_on = enc_hdmi2_on;
    hdmi_tx_reg_write(reg_addr, tmp.u32);

    return HI_SUCCESS;
}

hi_u32 hdmi_reg_tmds_pack_mode_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_pack_fifo_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->pack_fifo_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.tmds_pack_mode;
}

hi_u32 hdmi_reg_pclk2tclk_stable_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    tx_pack_fifo_st tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->pack_fifo_status.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.pclk2tclk_stable;
}

hi_u32 hdmi_reg_cea_avi_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_avi_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avi_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cea_avi_en;
}

hi_u32 hdmi_reg_cea_aud_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_aud_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->aud_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cea_aud_en;
}

hi_u32 hdmi_reg_cea_gen_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_gen_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gen_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cea_gen_en;
}

hi_u32 hdmi_reg_cea_cp_rpt_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_cp_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->cp_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cea_cp_rpt_en;
}

hi_u32 hdmi_reg_cea_gamut_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_gamut_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->gamut_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cea_gamut_en;
}

hi_u32 hdmi_reg_cea_vsif_rpt_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_vsif_cfg tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->vsif_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cea_vsif_rpt_en;
}

hi_u32 hdmi_reg_dc_pkt_en_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.dc_pkt_en;
}

hi_u32 hdmi_reg_hdmi_mode_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cea_avmixer_config tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->avmixer_cfg.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.hdmi_mode;
}

hi_u32 hdmi_reg_cp_set_avmute_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    cp_pkt_avmute tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->pkt_avmute.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.cp_set_avmute;
}

hi_u32 hdmi_reg_enc_scr_on_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    hdmi_enc_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->enc_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.enc_scr_on;
}

hi_u32 hdmi_reg_enc_hdmi2_on_get(hi_void)
{
    hi_u32 *reg_addr = NULL;
    hdmi_enc_ctrl tmp;

    reg_addr = (hi_u32 *)&(g_tx_hdmi_regs->enc_ctrl.u32);
    tmp.u32 = hdmi_tx_reg_read(reg_addr);
    return tmp.bits.enc_hdmi2_on;
}


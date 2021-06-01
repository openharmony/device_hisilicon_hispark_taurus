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

#include "tde_hal_k.h"
#ifdef HI_BUILD_IN_BOOT
#include "higo_list.h"
#endif
#include "tde_hal.h"
#include "wmalloc.h"
#include "securec.h"
#include "hi_common.h"
#include "hi_math.h"
#include "tde_filterpara.h"
#include "tde_adp.h"
#include "tde_osictl_k.h"
#include "hi_osal.h"

/* compute struct member's offset */
#define tde_offset_of(type, member) ((hi_u32)(&((type *)0)->member))

#define UPDATE_SIZE 64

#define NO_HSCALE_STEP 0x100000
#define NO_VSCALE_STEP 0x1000

#ifndef HI_BUILD_IN_BOOT
#define drv_tde_node_blend_mode(node, a, b)                 \
        do {                                                \
            (node)->src1_cbmpara.bits.s1_blendmode = (a);   \
            (node)->src2_cbmpara.bits.s2_blendmode = (b);   \
        } while (0)
#endif

/* R/W register's encapsulation */
#define tde_read_reg(base, offset) (*(volatile hi_u32 *)((hi_u8 *)(base) + (offset)))
#define tde_write_reg(base, offset, val) (*(volatile hi_u32 *)((hi_u8 *)(base) + (offset)) = val)
/* TDE register's Address range */
#define TDE_REG_SIZE 0x4100

/* Step range's type by algorithm team */
#define TDE_RESIZE_PARA_AREA_0 4096  /* 1.0 */
#define TDE_RESIZE_PARA_AREA_1 5461  /* 0.75 */
#define TDE_RESIZE_PARA_AREA_2 8192  /* 0.5 */
#define TDE_RESIZE_PARA_AREA_3 12412 /* 0.33 */
#define TDE_RESIZE_PARA_AREA_4 16384 /* 0.25 */

/* coefficient table rangle of 8*32 area */
#define TDE_RESIZE_8X32_AREA_0 1048576 /* 1.0 */
#define TDE_RESIZE_8X32_AREA_1 1398101 /* 0.75 */
#define TDE_RESIZE_8X32_AREA_2 2097152 /* 0.5 */
#define TDE_RESIZE_8X32_AREA_3 3177503 /* 0.33 */
#define TDE_RESIZE_8X32_AREA_4 4194304 /* 0.25 */

/* Colorkey mode of CMOS chip */
#define TDE_COLORKEY_IGNORE 2
#define TDE_COLORKEY_AREA_OUT 1
#define TDE_COLORKEY_AREA_IN 0

/* Aq control mode */
#define TDE_AQ_CTRL_COMP_LIST 0x0 /* start next AQ list, after complete current list's operations */
#define TDE_AQ_CTRL_COMP_LINE 0x4 /* start next AQ list, after complete current node and line */

#define TDE_MAX_READ_STATUS_TIME 10

#define TDE_TWENTYFOUR_BITS_SHIFT 24
#define TDE_EIGHT_BITS_SHIFT 8
#define TDE_SIXTEEN_BITS_SHIFT 16

/* Head address of argument table used as config */
typedef struct {
    hi_u64 hf_coef_addr;
    hi_u64 vf_coef_addr;
#ifndef HI_BUILD_IN_BOOT

#if (TDE_CAPABILITY & DEFLICKER)
    hi_u64 deflicker_vf_coef_addr;
#endif

#endif
} tde_para_table;

typedef struct {
    hi_u32 alpha_en;
    hi_u32 luma_en;
    hi_u32 chrome_en;
} tde_filtermode;

/* Base addr of register after mapping */
volatile hi_u32 *g_base_vir_addr = HI_NULL;
static hi_bool g_use_dtsi = HI_TRUE;

/* Pointer of TDE clock register after mapping */
static volatile hi_u32 *g_tde_clock_vir = HI_NULL;
/* Head address of config argument table */
static tde_para_table g_para_table = {0};

/* Deflicker level, default is auto */
#ifndef HI_BUILD_IN_BOOT

#if (TDE_CAPABILITY & DEFLICKER)
static hi_tde_deflicker_level g_deflicker_level = HI_TDE_DEFLICKER_LEVEL_AUTO;
#endif

#endif
/* alpha threshold switch */
static hi_bool g_alpha_threshold = HI_FALSE;

/* alpha threshold value */
static hi_u8 g_alpha_threshold_value = 0xff;

static tde_alu_mode g_cbmctrl_alu_mode[TDE_BUTT + 1] = {
    TDE_SRC2_BYPASS, TDE_ALU_NONE, TDE_ALU_ROP, TDE_SRC2_BYPASS, TDE_ALU_MASK_ROP1, TDE_ALU_MASK_ROP2,
    TDE_ALU_MASK_BLEND, TDE_SRC2_BYPASS, TDE_SRC2_BYPASS, TDE_SRC2_BYPASS, TDE_SRC2_BYPASS, TDE_BUTT
};

static hi_s32 tde_hal_init_para_table(hi_void);

#ifndef HI_BUILD_IN_BOOT
static hi_void tde_hal_node_set_clutcolor_key_para(tde_hw_node *hw_node, tde_color_key_cmd *color_key);
static hi_void tde_hal_node_set_ycbcr_color_key_para(tde_hw_node *hw_node, tde_color_key_cmd *color_key);
static hi_void tde_hal_node_set_argbcolor_key_para(tde_hw_node *hw_node, tde_color_key_cmd *color_key);

static hi_u32 tde_hal_get_color_key_mode(hi_tde_color_key_comp *color_key);
static hi_u32 tde_hal_get_ycb_cr_key_mask(hi_u8 cr, hi_u8 cb, hi_u8 cy, hi_u8 alpha);
static hi_u32 tde_hal_get_clut_key_mask(hi_u8 clut, hi_u8 alpha);
static hi_u32 tde_hal_get_argb_key_mask(hi_u8 blue, hi_u8 green, hi_u8 red, hi_u8 alpha);

#endif

static hi_s32 tde_hal_getbpp_by_fmt(tde_color_fmt fmt);
static hi_u32 tde_hal_get_resize_para_htable(hi_u32 step);
static hi_u32 tde_hal_get_resize_para_vtable(hi_u32 step);

static hi_void tde_hal_init_queue(hi_void);
hi_void tde_hal_set_clock(hi_bool enable);
static hi_s32 tde_hal_getbpp_by_fmt1(tde_color_fmt fmt);
static hi_s32 tde_hal_getbpp_by_fmt2(tde_color_fmt fmt);
static hi_s32 tde_hal_getbpp_by_fmt4(tde_color_fmt fmt);
static hi_s32 tde_hal_getbpp_by_fmt8(tde_color_fmt fmt);
static hi_s32 tde_hal_getbpp_by_fmt16(tde_color_fmt fmt);
static hi_s32 tde_hal_getbpp_by_fmt24(tde_color_fmt fmt);
static hi_s32 tde_hal_getbpp_by_fmt32(tde_color_fmt fmt);
#ifndef HI_BUILD_IN_BOOT
#define registder_flush() mb()
#else
#define registder_flush() \
    {                     \
        do {              \
        } while (0);      \
    }
#endif
static hi_bool tde_hal_is_operation_support(tde_alu_mode alu_mode, hi_u32 capability, hi_u32 oper_mode);
static hi_u32 tde_hal_get_zme_in_fmt(tde_color_fmt in_drv_fmt);
static hi_u32 tde_hal_get_hpzme_mode(hi_u32 out_rect_width, hi_u32 in_rect_width);
static hi_void tde_hal_get_filter_mode(hi_tde_filter_mode filter_mode, tde_filtermode *flt_mode);
static hi_s32 tde_hal_get_ver_scale_coeff(hi_u32 out_rect_height, hi_u32 in_rect_height, hi_s32 *ver_scale_coeff);

static hi_void tde_fill_data_by_fmt(tde_hw_node *hw_node, hi_u32 data, tde_color_fmt fmt, hi_u8 flag)
{
    hi_u32 bpp, i, cell;
    bpp = tde_hal_getbpp_by_fmt(fmt);
    if (bpp != 0xffffffff) {
        cell = data & (0xffffffff >> (32 - bpp)); /* 32 bpp The not, 0xffffffff mask */
        for (i = 0; i < (32 / bpp); i++) { /* 32 bpp fmt */
            if (flag) {
                (hw_node->src2_fill.bits.src2_color_fill) |= (cell << (i * bpp));
            } else {
                (hw_node->src1_fill.bits.src1_color_fill) |= (cell << (i * bpp));
            }
        }
    } else {
        if (flag) {
            (hw_node->src2_fill.bits.src2_color_fill) = data;
        } else {
            (hw_node->src1_fill.bits.src1_color_fill) = data;
        }
    }
    return;
}

hi_void tde_hal_set_base_vir_addr(hi_u32 *temp_base_vir_addr)
{
    g_base_vir_addr = temp_base_vir_addr;
}

volatile hi_u32 *tde_hal_get_base_vir_addr(hi_void)
{
    return g_base_vir_addr;
}

/*
 * Function:      tde_hal_init
 * Description:   map the base address for tde
 * Input:         base_addr: the base address of tde
 * Return:        success/fail
 */
hi_s32 tde_hal_init(hi_u32 base_addr)
{
    /* init the pool memory of tde, CNcomment: Initialize the TDE memory pool */
    if (wmeminit() != HI_SUCCESS) {
        goto TDE_INIT_ERR;
    }
    /* config start address for the parameter, CNcomment: The first address configuration parameter table */
    if (tde_hal_init_para_table() != HI_SUCCESS) {
        wmemterm();
        goto TDE_INIT_ERR;
    }
#ifndef HI_BUILD_IN_BOOT
    /* map address for the register, CNcomment: Register mapping */
    g_use_dtsi = HI_TRUE;
    if (g_base_vir_addr == HI_NULL) {
        g_base_vir_addr = (volatile hi_u32 *)tde_reg_map(base_addr, TDE_REG_SIZE);
        g_use_dtsi = HI_FALSE;
    }

    if (g_base_vir_addr == HI_NULL) {
        wmemterm();
        goto TDE_INIT_ERR;
    }
    /* set limit of clock and div, CNcomment: Set the clock threshold, clock frequency division */
    g_tde_clock_vir = (volatile hi_u32 *)tde_reg_map(TDE_REG_CLOCK, 4); /* 4 size */
    if (g_tde_clock_vir == HI_NULL) {
        wmemterm();
        tde_reg_unmap(g_base_vir_addr);
        goto TDE_INIT_ERR;
    }
#else
    g_base_vir_addr = (hi_u32 *)base_addr;
#endif
    tde_hal_set_clock(HI_TRUE);
    return HI_SUCCESS;

TDE_INIT_ERR:
    return HI_FAILURE;
}

#ifndef HI_BUILD_IN_BOOT
hi_void tde_hal_resume_init(hi_void)
{
    tde_hal_set_clock(HI_TRUE);

    tde_hal_ctl_reset();

    tde_hal_init_queue();

    return;
}

hi_void tde_hal_suspend(hi_void)
{
    tde_hal_set_clock(HI_FALSE);
    return;
}

hi_s32 tde_hal_close(hi_void)
{
    tde_hal_set_clock(HI_FALSE);
    return HI_SUCCESS;
}

hi_s32 tde_hal_open(hi_void)
{
    return HI_SUCCESS;
}

/*
 * Function:      tde_hal_release
 * Description:   release the address that had map
 * Return:        success/fail
 */
hi_void tde_hal_release(hi_void)
{
    hi_void *buf = HI_NULL;
    if (g_para_table.hf_coef_addr != 0) {
        buf = (hi_void *)wgetvrt(g_para_table.hf_coef_addr);
        if (buf != HI_NULL) {
            tde_free(buf);
            buf = HI_NULL;
            g_para_table.hf_coef_addr = 0;
        }
    }
    if (g_para_table.vf_coef_addr != 0) {
        buf = (hi_void *)wgetvrt(g_para_table.vf_coef_addr);
        if (buf != HI_NULL) {
            tde_free(buf);
            buf = HI_NULL;
            g_para_table.vf_coef_addr = 0;
        }
    }
#ifndef HI_BUILD_IN_BOOT

#if (TDE_CAPABILITY & DEFLICKER)
    if (g_para_table.deflicker_vf_coef_addr != 0) {
        buf = (hi_void *)wgetvrt(g_para_table.deflicker_vf_coef_addr);
        if (buf != HI_NULL) {
            tde_free(buf);
            buf = HI_NULL;
            g_para_table.deflicker_vf_coef_addr = 0;
        }
    }
#endif

#endif
    /* unmap, CNcomment: Remove the mapping */
    if (g_tde_clock_vir != HI_NULL) {
        tde_reg_unmap(g_tde_clock_vir);
    }
    g_tde_clock_vir = HI_NULL;
    /* unmap the base address, CNcomment: Reflect the base address */
    if (g_use_dtsi == HI_FALSE) {
        tde_reg_unmap(g_base_vir_addr);
    }
    g_base_vir_addr = HI_NULL;

    /* free the pool of memory, CNcomment: TDE memory pool to initialize */
    wmemterm();

    return;
}
#endif /* HI_BUILD_IN_BOOT */

#if (HICHIP == HI3516C_V500)
hi_bool tde_hal_ctl_is_idle(hi_void)
{
    return (hi_bool)(!(tde_read_reg(g_base_vir_addr, TDE_STA) & 0xff));
}
#else
hi_bool tde_hal_ctl_is_idle(hi_void)
{
    return (hi_bool)(!(tde_read_reg(g_base_vir_addr, TDE_STA) & 0x1));
}
#endif

/*
 * Function:      tde_hal_ctl_is_idle_safely
 * Description:   get the state of tde one more time ,make sure it's idle
 * Return:        True: Idle/False: Busy
 */
hi_bool tde_hal_ctl_is_idle_safely(hi_void)
{
    hi_u32 i;

    /*
     * get the state of tde one more time ,make sure it's idle
     * CNcomment: Continuous read hardware status for many times, make sure complete TDE
     */
    for (i = 0; i < TDE_MAX_READ_STATUS_TIME; i++) {
        if (!tde_hal_ctl_is_idle()) {
            return HI_FALSE;
        }
    }
    return HI_TRUE;
}

hi_u32 tde_hal_ctl_int_status(hi_void)
{
    hi_u32 value;

    value = tde_read_reg(g_base_vir_addr, TDE_INT);
    /* clear all status */
    tde_write_reg(g_base_vir_addr, TDE_INTCLR, value);
    return value;
}

hi_void tde_hal_ctl_reset(hi_void)
{
    tde_write_reg(g_base_vir_addr, TDE_INTCLR, 0xf);
    tde_write_reg(g_base_vir_addr, TDE_MISCELLANEOUS, 0x0100647f);
    return;
}

/*
 * Function:      tde_hal_set_clock
 * Description:   enable or disable the clock of TDE
 * Input:         hi_bool enable:enable/disable
 */
hi_void tde_hal_set_clock(hi_bool enable)
{
    if (enable) {
        /* cancel reset */
        *g_tde_clock_vir &= ~0x1;

        /* 0x2 enable clock */
        *g_tde_clock_vir |= 0x2;
    } else {
#ifdef TDE_LOWPOWER
        /* 0x2 disable clock */
        *g_tde_clock_vir &= ~0x2;

        /* reset */
        *g_tde_clock_vir |= 0x1;
#endif
    }
}

#ifndef HI_BUILD_IN_BOOT
/*
 * Function:      tde_hal_ctl_int_clear
 * Description:   clear the state of interrupt
 * Input:         stats: state that need clear
 */
hi_void tde_hal_ctl_int_clear(hi_u32 stats)
{
    hi_u32 read_stats;

    read_stats = tde_read_reg(g_base_vir_addr, TDE_INT);
    /* 16 left to the highest level, 0x0000ffff 0xffff0000 mask */
    read_stats = (read_stats & 0x0000ffff) | ((stats << 16) & 0xffff0000);

    tde_write_reg(g_base_vir_addr, TDE_INT, read_stats);
}
#endif

/*
 * Function:      tde_hal_node_init_nd
 * Description:   init the software node struct for tde
 * Input:         hw_node: the pointer of software node struct
 */
hi_s32 tde_hal_node_init_nd(tde_hw_node **hw_node)
{
    hi_void *buf = HI_NULL;

    if (hw_node == HI_NULL) {
        return HI_FAILURE;
    }
    buf = (hi_void *)tde_malloc(sizeof(tde_hw_node) + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE);
    if (buf == HI_NULL) {
        tde_error("malloc (%lu) failed, wgetfreenum(%u)!\n",
                  (unsigned long)(sizeof(tde_hw_node) + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE), wgetfreenum());
        return HI_ERR_TDE_NO_MEM;
    }

    *hw_node = (tde_hw_node *)((hi_u8 *)buf + TDE_NODE_HEAD_BYTE);

    return HI_SUCCESS;
}

#ifdef CONFIG_GFX_MMU_SUPPORT
hi_void tde_hal_free_tmp_buf(tde_hw_node *hw_node)
{
    return;
}
#endif

/*
 * Function:      tde_hal_free_node_buf
 * Description:   Free TDE operate node buffer
 * Input:         hw_node:Node struct pointer.
 */
hi_void tde_hal_free_node_buf(tde_hw_node *hw_node)
{
    hi_void *buf = HI_NULL;

    if (hw_node == HI_NULL) {
        return;
    }
    buf = (hi_u8 *)hw_node - TDE_NODE_HEAD_BYTE;

#ifdef CONFIG_GFX_MMU_SUPPORT
    tde_hal_free_tmp_buf(hw_node);
#endif

    tde_free(buf);
    return;
}

/*
 * Function:      tde_hal_node_execute
 * Description:   start list of tde
 * Input:         nodephy_addr: the start address of head node address
 *                update:the head node update set
 *                aq_use_buff: whether use temp buffer
 */
hi_s32 tde_hal_node_execute(hi_u64 nodephy_addr, hi_u64 update, hi_bool aq_use_buff)
{
    hi_unused(update);
    hi_unused(aq_use_buff);
    /* tde is idle */
    if (tde_hal_ctl_is_idle_safely()) {
        tde_hal_set_clock(HI_TRUE);
        tde_hal_ctl_reset();
        tde_hal_init_queue();

        /* write the first node address */
        tde_write_reg(g_base_vir_addr, TDE_AQ_NADDR_LOW, GetLowAddr(nodephy_addr));
        tde_write_reg(g_base_vir_addr, TDE_AQ_NADDR_HI, GetHighAddr(nodephy_addr));
#ifndef HI_BUILD_IN_BOOT
        osal_dmb();
#endif
        osal_udelay(100); /* 100 time out */
        /* start Aq list, CNcomment : Start the Aq */
        tde_write_reg(g_base_vir_addr, TDE_CTRL, 0x1);
    } else {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*
 * Function:      tde_hal_node_enable_complete_int
 * Description:   enable the finish interrupt of node
 * Input:         buf: buffer of node
 */
hi_void tde_hal_node_enable_complete_int(hi_void *buf)
{
    tde_hw_node *hw_node = (tde_hw_node *)buf;
    if (buf == HI_NULL) {
        tde_error("pBuf is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    hw_node->tde_intmask.bits.eof_mask = 0;
    hw_node->tde_intmask.bits.timeout_mask = 0x1;
    hw_node->tde_intmask.bits.bus_err_mask = 0x1;
#ifdef TDE_HWC_COOPERATE
    hw_node->tde_intmask.bits.eof_end_mask = 0x0;
#else
    hw_node->tde_intmask.bits.eof_end_mask = 0x1;
#endif
    return;
}

hi_void tde_hal_next_node_addr(hi_void *buf, hi_u64 phy_addr)
{
    tde_hw_node *hw_node = (tde_hw_node *)buf;
    if (buf == HI_NULL) {
        tde_error("pBuf is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    hw_node->tde_pnext_low.bits.next_low = GetLowAddr(phy_addr);
    hw_node->tde_pnext_hi.bits.next_hi = GetHighAddr(phy_addr);
    return;
}

#if (TDE_CAPABILITY & SYNC)
hi_void tde_hal_node_enable_sync(hi_void *buf)
{
    tde_hw_node *hw_node = (tde_hw_node *)buf;
    if (buf == HI_NULL) {
        tde_error("pBuf is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    hw_node->des_ctrl.bits.des_bind_en = 0x1;
    if ((hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_YCBCR888) ||
        (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_YCBCR422)) {
        hw_node->des_ctrl.bits.des_bind_mode = 0x0;
    } else {
        hw_node->des_ctrl.bits.des_bind_mode = 0x1;
    }
    hw_node->DES_SAFE_DIST.all = hw_node->des_ch0_stride.all / 4; /* 4 buf narrow */
    hw_node->DES_SAFE_DIST_INVERSE.all = hw_node->des_ch0_stride.all / 4; /* 4 buf narrow */
    return;
}
#endif

/*
 * Function:      tde_hal_node_set_src1
 * Description:   set the info for source of bitmap 1
 * Input:         hw_node: pointer of node
 *                drv_surface: bitmap info
 */
hi_void tde_hal_node_set_src1(tde_hw_node *hw_node, tde_surface_msg *drv_surface)
{
    hi_u32 bpp;
    hi_u64 phy_addr;
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    if (drv_surface == HI_NULL) {
        tde_error("pDrvSurface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    /* set the source bitmap attribute info, CNcomment: Configure attribute information source bitmap */
    hw_node->src1_ctrl.bits.src1_fmt = (hi_u32)drv_surface->color_fmt;
    hw_node->src1_ctrl.bits.src1_alpha_range = 1 - (hi_u32)drv_surface->alpha_max_is_255;
    hw_node->src1_ctrl.bits.src1_h_scan_ord = (hi_u32)drv_surface->h_scan;
    hw_node->src1_ctrl.bits.src1_v_scan_ord = (hi_u32)drv_surface->v_scan;
    /*
     * file zero of low area and top area use low area extend
     * CNcomment: Have been using low filling is 0, high use low expansion mode
     */
    hw_node->src1_ctrl.bits.src1_rgb_exp = 0;

    hw_node->src1_imgsize.bits.src1_width = drv_surface->width - 1;
    hw_node->src1_imgsize.bits.src1_height = drv_surface->height - 1;

    bpp = tde_hal_getbpp_by_fmt(drv_surface->color_fmt);

    phy_addr = drv_surface->phy_addr + (hi_u64)(drv_surface->ypos) * (hi_u64)(drv_surface->pitch) +
                  (((hi_u64)(drv_surface->xpos) * (hi_u64)(bpp)) >> 3); /* bpp 2^3 */

    hw_node->src1_ch0_addr_low.bits.src1_ch0_addr_low = GetLowAddr(phy_addr);
    hw_node->src1_ch0_addr_high.bits.src1_ch0_addr_high = GetHighAddr(phy_addr);

    if (drv_surface->color_fmt <= TDE_DRV_COLOR_FMT_RGB565) {
        hw_node->src1_ctrl.bits.src1_argb_order = drv_surface->rgb_order;
    }
    /*
     * target bitmapis same with source bitmap 1,so not need set.
     * config the node, CNcomment: Configure the cache node
     */
    hw_node->src1_ch0_stride.bits.src1_ch0_stride = drv_surface->pitch;
    return;
}

static hi_void tde_hal_set_src2_ctrl(tde_hw_node *hw_node, tde_surface_msg *drv_surface)
{
    hw_node->src2_ctrl.bits.src2_fmt = (hi_u32)drv_surface->color_fmt;
    hw_node->src2_ctrl.bits.src2_alpha_range = 0;
    hw_node->src2_ctrl.bits.src2_h_scan_ord = (hi_u32)drv_surface->h_scan;
    hw_node->src2_ctrl.bits.src2_v_scan_ord = (hi_u32)drv_surface->v_scan;
}

static hi_void tde_hal_set_src2_img_size(tde_hw_node *hw_node, tde_surface_msg *drv_surface)
{
    hw_node->src2_imgsize.bits.src2_width = drv_surface->width - 1;
    hw_node->src2_imgsize.bits.src2_height = drv_surface->height - 1;
}

static hi_void tde_hal_set_src2_addr(tde_hw_node *hw_node, tde_surface_msg *drv_surface, hi_u64 *phy_addr,
    hi_u64 *cb_crphy_addr)
{
    hi_u32 bpp = tde_hal_getbpp_by_fmt(drv_surface->color_fmt);
    if (drv_surface->color_fmt >= TDE_DRV_COLOR_FMT_YCBCR400MBP) {
        *phy_addr = drv_surface->phy_addr + (hi_u64)(drv_surface->ypos) *
            (hi_u64)(drv_surface->pitch) + (((hi_u64)(drv_surface->xpos) * 8) >> 3); /* 3 8 xpos alg data */

        hw_node->src2_ch0_addr_low.bits.src2_ch0_addr_low = GetLowAddr(*phy_addr);
        hw_node->src2_ch0_addr_high.bits.src2_ch0_addr_high = GetHighAddr(*phy_addr);
        switch (drv_surface->color_fmt) {
            case TDE_DRV_COLOR_FMT_YCBCR422MBH:
                *cb_crphy_addr = drv_surface->cbcr_phy_addr +
                                 (hi_u64)(drv_surface->ypos) * (hi_u64)(drv_surface->cb_cr_pitch) +
                                 ((hi_u64)(drv_surface->xpos) / 2 * 2); /* 2 xpos alg data cb_crphy_addr */
                break;
            case TDE_DRV_COLOR_FMT_YCBCR422MBV:
                *cb_crphy_addr = drv_surface->cbcr_phy_addr +
                                  /* 2 ypos size */
                                 (hi_u64)(drv_surface->ypos / 2) * (hi_u64)(drv_surface->cb_cr_pitch) +
                                 (((hi_u64)(drv_surface->xpos) * 16) >> 3); /* 16 3 alg data cb_crphy_addr */
                break;
            case TDE_DRV_COLOR_FMT_YCBCR420MB:
                *cb_crphy_addr = drv_surface->cbcr_phy_addr +
                                 /* 2 alg data */
                                 (hi_u64)(drv_surface->ypos / 2) * (hi_u64)(drv_surface->cb_cr_pitch) +
                                 ((hi_u64)(drv_surface->xpos) / 2 * 2); /* 2 xpos alg data cb_crphy_addr */
                break;
            case TDE_DRV_COLOR_FMT_YCBCR444MB:
                *cb_crphy_addr = drv_surface->cbcr_phy_addr +
                                 (hi_u64)(drv_surface->ypos) * (hi_u64)(drv_surface->cb_cr_pitch) +
                                 (((hi_u64)(drv_surface->xpos) * 16) >> 3); /* 16 3 alg data */
                break;
            default:;
        }

        hw_node->src2_ch1_addr_low.bits.src2_ch1_addr_low = GetLowAddr(*cb_crphy_addr);
        hw_node->src2_ch1_addr_high.bits.src2_ch1_addr_high = GetHighAddr(*cb_crphy_addr);

        if (drv_surface->color_fmt == TDE_DRV_COLOR_FMT_YCBCR422MBV) {
            hw_node->src2_ctrl.bits.src2_422v_pro = 1;
        }
    } else {
        *phy_addr = drv_surface->phy_addr + (hi_u64)(drv_surface->ypos) *
            /* alg data 2^3 */
            (hi_u64)(drv_surface->pitch) + (((hi_u64)(drv_surface->xpos) * (hi_u64)(bpp)) >> 3);
        hw_node->src2_ch0_addr_low.bits.src2_ch0_addr_low = GetLowAddr(*phy_addr);
        hw_node->src2_ch0_addr_high.bits.src2_ch0_addr_high = GetHighAddr(*phy_addr);

        if (drv_surface->color_fmt <= TDE_DRV_COLOR_FMT_RGB565) {
            hw_node->src2_ctrl.bits.src2_argb_order = drv_surface->rgb_order;
        }
    }
}

/*
 * Function:      tde_hal_node_set_src2
 * Description:   set the source bitmap 2
 * Input:         hw_node: pointer of node
 *                drv_surface:  bitmap info
 */
hi_void tde_hal_node_set_src2(tde_hw_node *hw_node, tde_surface_msg *drv_surface)
{
    hi_u64 phy_addr;
    hi_u64 cb_crphy_addr;

    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    if (drv_surface == HI_NULL) {
        tde_error("pDrvSurface is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    /* set attribute info for source bitmap, CNcomment:Configure attribute information source bitmap */
    tde_hal_set_src2_ctrl(hw_node, drv_surface);
    /*
     * file zero of low area and top area use low area extend
     * CNcomment: Have been using low filling is 0, high use low expansion mode
     */
    hw_node->src2_ctrl.bits.src2_rgb_exp = 0;

    tde_hal_set_src2_img_size(hw_node, drv_surface);

    phy_addr = drv_surface->phy_addr;
    cb_crphy_addr = drv_surface->cbcr_phy_addr;

    tde_hal_set_src2_addr(hw_node, drv_surface, &phy_addr, &cb_crphy_addr);
    if (drv_surface->color_fmt <= TDE_DRV_COLOR_FMT_A1) {
        hw_node->src2_alpha.bits.src2_alpha0 = 0x00;
        hw_node->src2_alpha.bits.src2_alpha1 = 0xff;
    }
    hw_node->src2_ch0_stride.bits.src2_ch0_stride = drv_surface->pitch;
    hw_node->src2_ch1_stride.bits.src2_ch1_stride = drv_surface->cb_cr_pitch;

    return;
}

#if (TDE_CAPABILITY & COMPRESS)
hi_void tde_hal_node_set_src_to_decompress(tde_hw_node *hw_node, tde_surface_msg *drv_surface)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    tde_hal_node_set_src2(hw_node, drv_surface);

    hw_node->src2_ctrl.bits.src2_is_lossless = 0;
    hw_node->src2_ctrl.bits.src2_is_lossless_a = 0;
    hw_node->src2_ctrl.bits.src2_dcmp_en = 1;
    hw_node->src2_ctrl.bits.src2_top_pred_en = 1;

    return;
}
#endif

static hi_void tde_hal_node_set_hw_node(tde_hw_node *hw_node, tde_surface_msg *drv_surface,
    hi_tde_out_alpha_from alpha_from)
{
    /* set bitmap attribute info, CNcomment:Configure attribute information source bitmap */
    hw_node->des_ctrl.bits.des_en = 1;
    hw_node->des_ctrl.bits.des_fmt = drv_surface->color_fmt;
    hw_node->des_ctrl.bits.des_alpha_range = 1 - (hi_u32)drv_surface->alpha_max_is_255;
    hw_node->des_ctrl.bits.des_h_scan_ord = (hi_u32)drv_surface->h_scan;
    hw_node->des_ctrl.bits.des_v_scan_ord = (hi_u32)drv_surface->v_scan;
    hw_node->des_ctrl.bits.des_alpha_range = 1 - (hi_u32)drv_surface->alpha_max_is_255;
    hw_node->des_ctrl.bits.des_rgb_round = 1; /* CNcomment:Always use four shed no into the way of cutting bit */

    hw_node->cbmalupara.bits.alpha_from = (hi_u32)alpha_from;
    hw_node->des_alpha.bits.des_alpha_thd = g_alpha_threshold_value;

    /* set bitmap size info, CNcomment: Configure the bitmap size information */
    hw_node->des_imgsize.bits.des_width = (hi_u32)drv_surface->width - 1;
    hw_node->des_imgsize.bits.des_height = (hi_u32)drv_surface->height - 1;
}

/*
 * Function:      tde_hal_node_set_tqt
 * Description:   set target bitmap info
 * Input:         hw_node: pointer of node
 *                drv_surface: bitmap info
 *                alpha_from: alpha from
 */
hi_void tde_hal_node_set_tqt(tde_hw_node *hw_node, tde_surface_msg *drv_surface,
    hi_tde_out_alpha_from alpha_from)
{
    hi_u32 bpp;
    hi_u64 phy_addr;
    hi_u64 cb_crphy_addr;

    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    tde_hal_node_set_hw_node(hw_node, drv_surface, alpha_from);
    if (drv_surface->color_fmt == TDE_DRV_COLOR_FMT_AYCBCR8888) {
        drv_surface->color_fmt = TDE_DRV_COLOR_FMT_ARGB8888;
        hw_node->des_ctrl.bits.des_argb_order = 0x17;
    }

    if ((drv_surface->color_fmt <= TDE_DRV_COLOR_FMT_RGB565) ||
        (drv_surface->color_fmt == TDE_DRV_COLOR_FMT_AYCBCR8888)) {
        hw_node->des_ctrl.bits.des_argb_order = drv_surface->rgb_order;
        if (drv_surface->color_fmt == TDE_DRV_COLOR_FMT_AYCBCR8888) {
            drv_surface->color_fmt = TDE_DRV_COLOR_FMT_ARGB8888;
            hw_node->des_ctrl.bits.des_argb_order = 0x17;
        }
    }
    if (drv_surface->color_fmt == TDE_DRV_COLOR_FMT_RABG8888) {
        hw_node->des_ctrl.bits.des_argb_order = TDE_DRV_ORDER_RABG; /* RABG */
        hw_node->des_ctrl.bits.des_fmt = TDE_DRV_COLOR_FMT_ARGB8888;
    }

    bpp = tde_hal_getbpp_by_fmt(drv_surface->color_fmt);
    phy_addr = drv_surface->phy_addr + (hi_u64)(drv_surface->ypos) * (hi_u64)(drv_surface->pitch) +
                  (((hi_u64)(drv_surface->xpos) * (hi_u64)(bpp)) >> 3); /* 3 bpp narrow 8 */
    hw_node->des_ch0_addr_low.bits.des_ch0_addr_low = GetLowAddr(phy_addr);
    hw_node->des_ch0_addr_high.bits.des_ch0_addr_high = GetHighAddr(phy_addr);

    cb_crphy_addr = drv_surface->cbcr_phy_addr + (hi_u64)(drv_surface->ypos) * (hi_u64)(drv_surface->pitch) +
                    (((hi_u64)(drv_surface->xpos) * (hi_u64)(bpp)) >> 3); /* 3 bpp narrow 8 */
    hw_node->des_ch1_addr_low.bits.des_ch1_addr_low = GetLowAddr(cb_crphy_addr);
    hw_node->des_ch1_addr_high.bits.des_ch1_addr_hi = GetHighAddr(cb_crphy_addr);

    hw_node->des_ch1_stride.bits.des_ch1_stride = (hi_u32)drv_surface->pitch;

    if (drv_surface->color_fmt == TDE_DRV_COLOR_FMT_YCBCR422) {
        hw_node->des_dswm.bits.des_h_dswm_mode = 1;
    }
    hw_node->des_ch0_stride.bits.des_ch0_stride = (hi_u32)drv_surface->pitch;
    return;
}

#if (TDE_CAPABILITY & COMPRESS)
#if (HICHIP == HI3519A_V100)
static hi_void tde_hal_node_set_argb8888_320(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 994; /* 994 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 750; /* 750 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 0;

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 5; /* 5 max_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 4; /* 4 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 0;

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 0;

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 0;
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 130; /* 130 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 250; /* 250 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_argb8888_720(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 492; /* 492 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 375; /* 375 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 0;

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 5; /* 5 max_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 3; /* 3 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 0;

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 8; /* 8 adj_sad_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 20; /* 20 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 130; /* 130 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 250; /* 250 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_argb8888_3840(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 492; /* 492 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 375; /* 375 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 10; /* 10 max_trow_bits data */

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 5; /* 5 max_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 4; /* 4 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 4; /* 4 special_bits_gain data */

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 30; /* 30 adj_sad_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 40; /* 40 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 100; /* 100 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 250; /* 250 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_rgb888_320(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 930; /* 930 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 702; /* 702 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 0;

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 4; /* 4 max_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 4; /* 4 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 0;

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 0;

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 0;
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 130; /* 130 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 250; /* 250 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_rgb888_720(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 500; /* 500 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 375; /* 375 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 0;

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 4; /* 4 max_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 3; /* 3 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 0;

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 8; /* 8 adj_sad_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 20; /* 20 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 130; /* 130 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 250; /* 250 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_rgb888_3840(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 500; /* 500 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 375; /* 375 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 10; /* 10 max_trow_bits data */

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 4; /* 4 max_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 4; /* 4 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 4; /* 4 special_bits_gain data */

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 30; /* 30 adj_sad_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 40; /* 40 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 100; /* 100 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 250; /* 250 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_argb15555_or_argb4444_720(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 492; /* 492 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 375; /* 375 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 10; /* 10 max_trow_bits data */

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 1;
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 3; /* 3 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 2; /* 2 special_bits_gain data */

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 10; /* 10 adj_sad_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 20; /* 20 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 100; /* 100 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 130; /* 130 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

static hi_void tde_hal_node_set_argb15555_or_argb4444_3840(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg0.bits.budget_bits_mb = 492; /* 492 budget_bits_mb data */
    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits = 375; /* 375 min_mb_bits data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.budget_bits_mb_cap = 512; /* 512 budget_bits_mb_cap data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.max_trow_bits = 10; /* 10 max_trow_bits data */

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_qp = 1;
    hw_node->tde_od_pic_osd_rc_cfg2.bits.smth_qp = 2; /* 2 smth_qp data */

    hw_node->tde_od_pic_osd_rc_cfg2.bits.sad_bits_ngain = 10; /* 10 sad_bits_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.rc_smth_ngain = 3; /* 3 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.special_bits_gain = 2; /* 2 special_bits_gain data */

    hw_node->tde_od_pic_osd_rc_cfg3.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.min_sad_thr = 12; /* 12 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg4.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.smth_pix_num_thr = 6; /* 6 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.still_pix_num_thr = 3; /* 3 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_pix_num_thr = 24; /* 24 noise_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.large_smth_pix_num_thr = 10; /* 10 large_smth_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg6.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg7.bits.adj_sad_bits_thr = 10; /* 10 adj_sad_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_inc1_bits_thr = 20; /* 20 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec1_bits_thr = 60; /* 60 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec2_bits_thr = 100; /* 100 qp_dec2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.qp_dec3_bits_thr = 130; /* 130 qp_dec3_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr = 70; /* 70 force_qp_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.force_qp_thr_cap = 20; /* 20 force_qp_thr_cap data */
}

hi_void tde_hal_node_set_compress(tde_hw_node *hw_node)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    hw_node->tde_od_pic_osd_glb_info.bits.is_lossless = 0;
    hw_node->tde_od_pic_osd_glb_info.bits.is_lossless_alpha = hw_node->tde_od_pic_osd_glb_info.bits.is_lossless;
    hw_node->tde_od_pic_osd_glb_info.bits.cmp_mode = 0;
    hw_node->tde_od_pic_osd_glb_info.bits.osd_mode =
        (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB8888) ? 0 :
        (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_RGB888 ? 1 :
        ((hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB1555) ? 2 : 3)); /* 2 3 des_fmt value */
    hw_node->tde_od_pic_osd_glb_info.bits.partition_en = 0;
    hw_node->tde_od_pic_osd_glb_info.bits.part_num = 1;

    hw_node->tde_od_pic_osd_frame_size.bits.frame_width = hw_node->des_imgsize.bits.des_width;
    hw_node->tde_od_pic_osd_frame_size.bits.frame_height = hw_node->des_imgsize.bits.des_height;

    if (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB8888) {
        if (hw_node->des_imgsize.bits.des_width <= 320) { /* 320 set argb8888 */
            tde_hal_node_set_argb8888_320(hw_node);
        } else if (hw_node->des_imgsize.bits.des_width <= 720) { /* 720 set argb8888 */
            tde_hal_node_set_argb8888_720(hw_node);
        } else if (hw_node->des_imgsize.bits.des_width <= 3840) { /* 3840 set argb8888 */
            tde_hal_node_set_argb8888_3840(hw_node);
        }
    } else if (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_RGB888) {
        if (hw_node->des_imgsize.bits.des_width <= 320) { /* 320 set rgb888 */
            tde_hal_node_set_rgb888_320(hw_node);
        } else if (hw_node->des_imgsize.bits.des_width <= 720) { /* 720 set rgb888 */
            tde_hal_node_set_rgb888_720(hw_node);
        } else if (hw_node->des_imgsize.bits.des_width <= 3840) { /* 3840 set rgb888 */
            tde_hal_node_set_rgb888_3840(hw_node);
        }
    } else if ((hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB1555) ||
               (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB4444)) {
        if (hw_node->des_imgsize.bits.des_width <= 720) { /* 720 set argb15555 or argb4444 */
            tde_hal_node_set_argb15555_or_argb4444_720(hw_node);
        } else if (hw_node->des_imgsize.bits.des_width <= 3840) { /* 3840 set argb15555 or argb4444 */
            tde_hal_node_set_argb15555_or_argb4444_3840(hw_node);
        }
    }
}
#else
static hi_void tde_hal_node_set_cfgxx(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_rc_cfg10.bits.qp_thr0 = 2; /* 2 qp_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg10.bits.qp_thr1 = 1;
	/* 2 3 qp_thr2 data */
    hw_node->tde_od_pic_osd_rc_cfg10.bits.qp_thr2 = ((hw_node->tde_od_pic_osd_glb_info.bits.source_mode == 2) ? 2 : 3);

    hw_node->tde_od_pic_osd_rc_cfg11.bits.grph_bias_bit_thr0 = 20; /* 20 grph_bias_bit_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg11.bits.grph_bias_bit_thr1 = 50; /* 50 grph_bias_bit_thr1 data */

    hw_node->tde_od_pic_osd_rc_cfg12.bits.force_rc_en = 0;
    hw_node->tde_od_pic_osd_rc_cfg12.bits.forcerc_bits_diff_thr = 80; /* 80 forcerc_bits_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg13.bits.maxdiff_ctrl_en = 1;

    hw_node->tde_od_pic_osd_rc_cfg14.bits.init_buf_bits_cap = 2047; /* 2047 init_buf_bits_cap data */

    hw_node->tde_od_pic_osd_rc_cfg15.bits.cmplx_sad_thr = 10; /* 10 cmplx_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg15.bits.err_thr0 = 2; /* 2 err_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg15.bits.err_thr1 = 5; /* 5 err_thr1 data */

    hw_node->tde_od_pic_osd_rc_cfg16.bits.sim_num_thr = 3; /* 3 sim_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg16.bits.sum_y_err_thr = 40; /* 40 sum_y_err_thr data */
    hw_node->tde_od_pic_osd_rc_cfg16.bits.sum_c_err_thr = 24; /* 24 sum_c_err_thr data */

    hw_node->tde_od_pic_osd_rc_cfg17.bits.cpmlx_sad_thr_y = 12; /* 12 cpmlx_sad_thr_y data */
    hw_node->tde_od_pic_osd_rc_cfg17.bits.smpl_sad_thr_c = 3; /* 3 smpl_sad_thr_c data */
    hw_node->tde_od_pic_osd_rc_cfg17.bits.smpl_sumsad_thr_y = 50; /* 50 smpl_sumsad_thr_y data */
    hw_node->tde_od_pic_osd_rc_cfg17.bits.smpl_sumsad_thr_c = 40; /* 40 smpl_sumsad_thr_c data */

    hw_node->tde_od_pic_osd_rc_cfg18.bits.future_sad_y_thr0 = 14; /* 14 future_sad_y_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg18.bits.future_sad_c_thr0 = 14; /* 14 future_sad_c_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg18.bits.future_sad_y_thr1 = 14; /* 14 future_sad_y_thr1 data */
    hw_node->tde_od_pic_osd_rc_cfg18.bits.future_sad_c_thr1 = 4; /* 4 future_sad_c_thr1 data */

    hw_node->tde_od_pic_osd_rc_cfg19.bits.cmplx_sumsad_thr_y = 160; /* 160 cmplx_sumsad_thr_y data */
    hw_node->tde_od_pic_osd_rc_cfg19.bits.cmplx_sumsad_thr_c = 100; /* 100 cmplx_sumsad_thr_c data */

    hw_node->tde_od_pic_osd_rc_cfg20.bits.buffer_up_lmt_thr = 4095; /* 4095 buffer_up_lmt_thr data */
    hw_node->tde_od_pic_osd_rc_cfg20.bits.buffer_low_lmt_thr = 2048; /* 2048 buffer_low_lmt_thr data */

    hw_node->tde_od_pic_osd_rc_cfg21.bits.buffer_adj_qp_thr0 = 2; /* 2 buffer_adj_qp_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg21.bits.buffer_adj_qp_thr1 = 3; /* 3 buffer_adj_qp_thr1 data */
    hw_node->tde_od_pic_osd_rc_cfg21.bits.buffer_adj_qp_thr2 = 4; /* 4 buffer_adj_qp_thr2 data */

    hw_node->tde_od_pic_osd_rc_cfg22.bits.buffer_surplus_thr0 = 180; /* 180 buffer_surplus_thr0 data */
    hw_node->tde_od_pic_osd_rc_cfg22.bits.buffer_surplus_thr1 = 50; /* 50 buffer_surplus_thr1 data */
    hw_node->tde_od_pic_osd_rc_cfg22.bits.buffer_surplus_thr2 = 10; /* 10 buffer_surplus_thr2 data */

    hw_node->tde_od_pic_osd_rc_cfg23.bits.offset_bits_thr = 128; /* 128 offset_bits_thr data */

    hw_node->tde_od_pic_osd_stat_thr.bits.max_gap_bw_row_len_thr = 0x40;
}

static hi_void tde_hal_node_set_cfg0x(tde_hw_node *hw_node)
{
    hw_node->tde_od_pic_osd_glb_info.bits.cmp_mode = 0;
    hw_node->tde_od_pic_osd_glb_info.bits.top_pred_en = 1;
    hw_node->tde_od_pic_osd_glb_info.bits.is_lossless = 0;
    hw_node->tde_od_pic_osd_glb_info.bits.is_lossless_a = 0;
    hw_node->tde_od_pic_osd_glb_info.bits.graphic_en = 1;

    hw_node->tde_od_pic_osd_frame_size.bits.frame_width = hw_node->des_imgsize.bits.des_width;
    hw_node->tde_od_pic_osd_frame_size.bits.frame_height = hw_node->des_imgsize.bits.des_height;

    hw_node->tde_od_pic_osd_rc_cfg1.bits.sad_bits_gain = 8; /* 8 sad_bits_gain data */
    hw_node->tde_od_pic_osd_rc_cfg1.bits.rc_smth_ngain = 4; /* 4 rc_smth_ngain data */
    hw_node->tde_od_pic_osd_rc_cfg1.bits.max_trow_bits = 10; /* 10 max_trow_bits data */

    hw_node->tde_od_pic_osd_rc_cfg2.bits.max_sad_thr = 64; /* 64 max_sad_thr data */
    hw_node->tde_od_pic_osd_rc_cfg2.bits.min_sad_thr = 16; /* 16 min_sad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg3.bits.smth_thr = 3; /* 3 smth_thr data */
    hw_node->tde_od_pic_osd_rc_cfg3.bits.still_thr = 1;
    hw_node->tde_od_pic_osd_rc_cfg3.bits.big_grad_thr = 16; /* 16 big_grad_thr data */

    hw_node->tde_od_pic_osd_rc_cfg4.bits.smth_pix_num_thr = 32; /* 32 smth_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.still_pix_num_thr = 32; /* 32 still_pix_num_thr data */
    hw_node->tde_od_pic_osd_rc_cfg4.bits.noise_pix_num_thr = 32; /* 32 noise_pix_num_thr data */

    hw_node->tde_od_pic_osd_rc_cfg5.bits.noise_sad = 32; /* 32 noise_sad data */
    hw_node->tde_od_pic_osd_rc_cfg5.bits.pix_diff_thr = 50; /* 50 pix_diff_thr data */

    hw_node->tde_od_pic_osd_rc_cfg6.bits.adj_sad_bits_thr = 0;

    hw_node->tde_od_pic_osd_rc_cfg7.bits.qp_inc1_bits_thr = 62; /* 62 qp_inc1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.qp_inc2_bits_thr = 180; /* 180 qp_inc2_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.qp_dec1_bits_thr = 64; /* 64 qp_dec1_bits_thr data */
    hw_node->tde_od_pic_osd_rc_cfg7.bits.qp_dec2_bits_thr = 150; /* 150 qp_dec2_bits_thr data */

    hw_node->tde_od_pic_osd_rc_cfg8.bits.est_err_gain = 7; /* 7 est_err_gain data */
    hw_node->tde_od_pic_osd_rc_cfg8.bits.max_est_err_level = 128; /* 128 max_est_err_level data */

    hw_node->tde_od_pic_osd_rc_cfg9.bits.vbv_buf_loss1_thr = 62; /* 62 vbv_buf_loss1_thr data */
    hw_node->tde_od_pic_osd_rc_cfg9.bits.vbv_buf_loss2_thr = 67; /* 67 vbv_buf_loss2_thr data */
}

static hi_s32 tde_hal_node_get_ratio(tde_hw_node *hw_node, hi_u32 *cmp_ratio, hi_u32 *cmp_ratio_frm,
    hi_s32 *mb_org_bits)
{
    if (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB8888) {
        hw_node->tde_od_pic_osd_glb_info.bits.source_mode = 0;
        *cmp_ratio = 10 * 100; /* 10 1000 s32cmp_ratio data */
        *cmp_ratio_frm = 20 * 100; /* 20 100 s32cmp_ratio_frm data */
        *mb_org_bits = 32 * (8 + 8 + 8 + 8); /* 8 32 mb_org_bits data */
    } else if (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB1555) {
        hw_node->tde_od_pic_osd_glb_info.bits.source_mode = 1;
        *cmp_ratio = 20 * 100; /* 20 100 s32cmp_ratio data */
        *cmp_ratio_frm = 2.0 * 100; /* 2.0 100 s32cmp_ratio_frm data */
        *mb_org_bits = 32 * (1 + 5 + 5 + 5); /* 32 5 mb_org_bits data */
    } else if (hw_node->des_ctrl.bits.des_fmt == TDE_DRV_COLOR_FMT_ARGB4444) {
        hw_node->tde_od_pic_osd_glb_info.bits.source_mode = 2; /* 2 source_mode data */
        *cmp_ratio = 1.5 * 100; /* 1.5 * 100 s32cmp_ratio data */
        *cmp_ratio_frm = 1.92 * 100; /* 1.92 * 100 s32cmp_ratio_frm data */
        *mb_org_bits = 32 * (4 + 4 + 4 + 4); /* 4 32 mb_org_bits data */
    } else {
        tde_error("compress unsupported format except argb\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static hi_void tde_hal_node_frm_width(tde_hw_node *hw_node, hi_s32 *frm_width, hi_s32 *frm_width_r)
{
    hi_s32 spilt_num;
    spilt_num = (hw_node->tde_od_pic_osd_frame_size.bits.frame_width + 15) / 16; /* 15 16 for more than */
    if (((hi_u32)spilt_num & 0x1) == 0) {
        *frm_width = (spilt_num / 2) * 16; /* 2 16 four left data */
        *frm_width_r = hw_node->tde_od_pic_osd_frame_size.bits.frame_width - *frm_width;
    } else {
        *frm_width_r = (spilt_num / 2) * 16; /* 2 16 four left data */
        *frm_width = hw_node->tde_od_pic_osd_frame_size.bits.frame_width - *frm_width_r;
    }
}

hi_void tde_hal_node_set_compress(tde_hw_node *hw_node)
{
    hi_u32 mb_num_x, cmp_ratio, cmp_ratio_frm;
    hi_s32 mb_minu10, mb_bits, mb_bits_cap, ctl_delta, mb_org_bits, line_delta, ret;
    hi_s32 frm_width, frm_width_r;

    /* set bitmap attribute info, CNcomment: Configure attribute information source bitmap */
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    ret = tde_hal_node_get_ratio(hw_node, &cmp_ratio, &cmp_ratio_frm, &mb_org_bits);
    if ((ret != HI_SUCCESS) || (cmp_ratio_frm == 0) || (cmp_ratio == 0)) {
        return;
    }
    tde_hal_node_set_cfg0x(hw_node);
    tde_hal_node_set_cfgxx(hw_node);
    if (hw_node->tde_od_pic_osd_frame_size.bits.frame_height == 0) {
        return;
    }

    tde_hal_node_frm_width(hw_node, &frm_width, &frm_width_r);
    mb_num_x = (osal_min(frm_width, frm_width_r) + 31) / 32; /* 31 32 for more than */
    if (mb_num_x == 0) {
        mb_num_x = 1;
    }

    mb_minu10 = mb_num_x - 10; /* 10 narrow alg data */

    hw_node->tde_od_pic_osd_rc_cfg1.bits.max_qp = (hw_node->tde_od_pic_osd_glb_info.bits.source_mode == 0) ?
        6 : ((hw_node->tde_od_pic_osd_glb_info.bits.source_mode == 2) ? 2 : 3); /* 2 3 6 max_qp data */
    /* 16 280 alg data */
    line_delta = hw_node->tde_od_pic_osd_rc_cfg7.bits.qp_inc1_bits_thr * 16 / mb_num_x + 280 / mb_num_x;
    /* 127 1000 alg data */
    ctl_delta = (mb_org_bits * 1000 / cmp_ratio) / 127 + osal_max((hi_u32)1,
        (hi_u32)(hw_node->tde_od_pic_osd_rc_cfg14.bits.init_buf_bits_cap +
        hw_node->tde_od_pic_osd_rc_cfg7.bits.qp_inc1_bits_thr * 16 + 280) / /* 16 280 alg data */
        (hw_node->tde_od_pic_osd_frame_size.bits.frame_height * mb_num_x));
    /* 0 3 ctl_delta data */
    ctl_delta = (hw_node->tde_od_pic_osd_glb_info.bits.source_mode == 0) ? (ctl_delta + 3) : ctl_delta;

    mb_bits = (hi_s32)(mb_org_bits * 1000 / cmp_ratio); /* 1000 alg data */
    mb_bits = mb_bits - line_delta;
    hw_node->tde_od_pic_osd_rc_cfg0.bits.mb_bits =
        (hw_node->tde_od_pic_osd_glb_info.bits.is_lossless || hw_node->tde_od_pic_osd_glb_info.bits.cmp_mode) ?
        osal_min((hi_u32)1023, (hi_u32)mb_org_bits) : (hi_u32)mb_bits; /* 1023 alg data */

    mb_bits_cap = (hi_s32)(mb_org_bits * 1000 / cmp_ratio_frm); /* 1000 alg data */
    mb_bits_cap = mb_bits_cap - ctl_delta;
    mb_bits_cap = osal_max((hi_u32)170, osal_min((hi_u32)mb_bits, (hi_u32)mb_bits_cap)); /* 170 alg data */
    hw_node->tde_od_pic_osd_rc_cfg14.bits.mb_bits_cap =
        (hw_node->tde_od_pic_osd_glb_info.bits.is_lossless || hw_node->tde_od_pic_osd_glb_info.bits.cmp_mode) ?
        osal_min((hi_u32)1023, (hi_u32)mb_org_bits) : (hi_u32)mb_bits_cap; /* 1023 alg data */

    hw_node->tde_od_pic_osd_rc_cfg0.bits.min_mb_bits =
        osal_min((hi_u32)(hw_node->tde_od_pic_osd_rc_cfg14.bits.mb_bits_cap - 20), /* 20 alg data */
                 (hi_u32)(hw_node->tde_od_pic_osd_rc_cfg0.bits.mb_bits * 3 / 4));  /* 4 3 alg data */

    hw_node->tde_od_pic_osd_rc_cfg15.bits.lfw_mb_len =
        /* 2 alg data */
        osal_max((hi_u32)0, osal_min((hi_u32)(mb_num_x / 2), (hi_u32)(mb_minu10 < 0 ? 0 : mb_minu10)));
    /* 20 alg data */
    hw_node->tde_od_pic_osd_rc_cfg11.bits.grph_ideal_bit_thr = hw_node->tde_od_pic_osd_rc_cfg0.bits.mb_bits - 20;
}
#endif

hi_void tde_hal_node_set_compress_tqt(tde_hw_node *hw_node, tde_surface_msg *drv_surface,
                                      hi_tde_out_alpha_from alpha_from)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    tde_hal_node_set_tqt(hw_node, drv_surface, alpha_from);
#if (HICHIP != HI3519A_V100)
    hw_node->tde_od_pic_osd_glb_info.bits.hq_cmp_en = 1;
#endif
    hw_node->des_ctrl.bits.cmp_en = 1;
    return;
}
#endif

#if (TDE_CAPABILITY & ROTATE)
hi_void tde_hal_node_set_rotate(tde_hw_node *hw_node, hi_tde_rotate_angle rotate_angle)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }
    hw_node->src2_rtt_ctrl.bits.s2_rtt_en = 1;
    /* set bitmap attribute info, CNcomment: Configure attribute information source bitmap */
    if (TDE_DRV_COLOR_FMT_YCBCR422 == hw_node->src2_ctrl.bits.src2_fmt) {
        hw_node->src2_rtt_ctrl.bits.s2_rtt_fmt = 0;
    }
    if ((hw_node->src2_ctrl.bits.src2_fmt == TDE_DRV_COLOR_FMT_RGB888) ||
        (TDE_DRV_COLOR_FMT_YCBCR888 == hw_node->src2_ctrl.bits.src2_fmt)) {
        hw_node->src2_rtt_ctrl.bits.s2_rtt_fmt = 1;
    }
    if ((hw_node->src2_ctrl.bits.src2_fmt == TDE_DRV_COLOR_FMT_ARGB4444) ||
        (hw_node->src2_ctrl.bits.src2_fmt == TDE_DRV_COLOR_FMT_ARGB1555)) {
        hw_node->src2_rtt_ctrl.bits.s2_rtt_fmt = 2; /* 2 ARGB4444 ARGB1555 */
    }
    if (hw_node->src2_ctrl.bits.src2_fmt == TDE_DRV_COLOR_FMT_ARGB8888) {
        hw_node->src2_rtt_ctrl.bits.s2_rtt_fmt = 3; /* 3 ARGB8888 */
    }
    if (rotate_angle == HI_TDE_ROTATE_CLOCKWISE_90) {
        hw_node->src2_rtt_ctrl.bits.s2_rtt_dir = 0;
    } else {
        hw_node->src2_rtt_ctrl.bits.s2_rtt_dir = 1;
    }

    return;
}
#endif

#ifndef HI_BUILD_IN_BOOT
static hi_s32 tde_hal_node_file(tde_hw_node *hw_node, tde_base_opt_mode mode, tde_alu_mode alu,
                                tde_color_fill *color_fill, hi_u32 capability)
{
    if (mode == TDE_QUIKE_COPY) {
        if (!tde_hal_is_operation_support(alu, capability, QUICKCOPY)) {
            tde_error("It does not support QuickCopy\n");
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }
        hw_node->src1_ctrl.bits.src1_en = 1;
        hw_node->src1_ctrl.bits.src1_dma = 1;
    }
    if (mode == TDE_NORM_FILL_1OPT) {
        if (color_fill == HI_NULL) {
            tde_error("pstColorFill is null %s(line=%d)\n", __FUNCTION__, __LINE__);
            return HI_ERR_TDE_NULL_PTR;
        }
        tde_fill_data_by_fmt(hw_node, color_fill->fill_data, color_fill->drv_color_fmt, 1);
        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 1;
        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;
        hw_node->cbmctrl.bits.alu_mode = 1;
    }
    if (mode == TDE_NORM_FILL_2OPT) {
        if (color_fill == HI_NULL) {
            tde_error("pstColorFill is null %s(line=%d)\n", __FUNCTION__, __LINE__);
            return HI_ERR_TDE_NULL_PTR;
        }
        tde_fill_data_by_fmt(hw_node, color_fill->fill_data, color_fill->drv_color_fmt, 1);
        hw_node->src1_ctrl.bits.src1_en = 1;
        hw_node->src1_ctrl.bits.src1_mode = 0;
        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 1;
        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;

        hw_node->cbmctrl.bits.alu_mode = g_cbmctrl_alu_mode[alu];
    }
    return HI_SUCCESS;
}

static hi_s32 tde_hal_node_opt(tde_hw_node *hw_node, tde_base_opt_mode mode, tde_alu_mode alu,
                               hi_u32 capability)
{
    if (mode == TDE_SINGLE_SRC_PATTERN_FILL_OPT) {
        if (!tde_hal_is_operation_support(alu, capability, PATTERFILL)) {
            tde_error("It deos not support PatternFill\n");
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }

        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 2; /* 2 mode */
        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;

        hw_node->cbmctrl.bits.alu_mode = g_cbmctrl_alu_mode[alu];
    }
    if (mode == TDE_DOUBLE_SRC_PATTERN_FILL_OPT) {
        if (!tde_hal_is_operation_support(alu, capability, PATTERFILL)) {
            tde_error("It deos not support PatternFill\n");
            return HI_ERR_TDE_UNSUPPORTED_OPERATION;
        }

        hw_node->src1_ctrl.bits.src1_en = 1;
        hw_node->src1_ctrl.bits.src1_mode = 0;

        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 2; /* 2 mode */
        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;

        hw_node->cbmctrl.bits.alu_mode = g_cbmctrl_alu_mode[alu];
    }
    return HI_SUCCESS;
}
#endif

static hi_void tde_hal_node_set_base_hw(tde_hw_node *hw_node, tde_base_opt_mode mode, tde_alu_mode alu)
{
    if (mode == TDE_QUIKE_FILL) {
        hw_node->src1_ctrl.bits.src1_en = 1;
        hw_node->src1_ctrl.bits.src1_mode = 1;
        hw_node->src1_ctrl.bits.src1_dma = 1;
        return;
    }
    if (mode == TDE_NORM_BLIT_1OPT) {
        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 0;
        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;
        hw_node->cbmctrl.bits.alu_mode = 1;
        return;
    }

    if (mode == TDE_NORM_BLIT_2OPT) {
        hw_node->src1_ctrl.bits.src1_en = 1;
        hw_node->src1_ctrl.bits.src1_mode = 0;

        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 0;

        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;

        hw_node->cbmctrl.bits.alu_mode = g_cbmctrl_alu_mode[alu];
        return;
    }
    if (mode == TDE_MB_2OPT) {
        hw_node->src2_ctrl.bits.src2_en = 1;
        hw_node->src2_ctrl.bits.src2_mode = 0;

        hw_node->cbmctrl.bits.cbm_en = 1;
        hw_node->cbmctrl.bits.cbm_mode = 1;
        hw_node->cbmctrl.bits.alu_mode = 0x1;
    }
    return;
}

static hi_s32 tde_hal_quike_fill(tde_hw_node *hw_node, tde_base_opt_mode mode, tde_alu_mode alu,
    tde_color_fill *color_fill, hi_u32 capability)
{
    if (color_fill == HI_NULL) {
        tde_error("pstColorFill is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (!tde_hal_is_operation_support(alu, capability, QUICKFILL)) {
        tde_error("It deos not support QuickFill\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    tde_fill_data_by_fmt(hw_node, color_fill->fill_data, color_fill->drv_color_fmt, 0);
    tde_hal_node_set_base_hw(hw_node, mode, alu);
    return HI_SUCCESS;
}

static hi_s32 tde_hal_node_operation_support(tde_alu_mode alu, hi_u32 capability)
{
#if (TDE_CAPABILITY & ROP)
    if (!tde_hal_is_operation_support(alu, capability, MASKROP)) {
        tde_error("It deos not support MaskRop\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
#endif
#if (TDE_CAPABILITY & MASKBLEND)
    if (!tde_hal_is_operation_support(alu, capability, MASKBLEND)) {
        tde_error("It deos not support MaskBlend\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
#endif
    return HI_SUCCESS;
}

hi_s32 tde_hal_node_set_base_operate(tde_hw_node *hw_node, tde_base_opt_mode mode, tde_alu_mode alu,
                                     tde_color_fill *color_fill)
{
    hi_u32 capability = 0;
    hi_s32 ret;

    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    tde_hal_get_capability(&capability);

    switch (mode) {
        case TDE_QUIKE_FILL: /* quick file, CNcomment:Rapid filling */
            ret = tde_hal_quike_fill(hw_node, mode, alu, color_fill, capability);
            if (ret != HI_SUCCESS) {
                return ret;
            }
            break;
#ifndef HI_BUILD_IN_BOOT
        case TDE_QUIKE_COPY:
        case TDE_NORM_FILL_1OPT:
        case TDE_NORM_FILL_2OPT:
            ret = tde_hal_node_file(hw_node, mode, alu, color_fill, capability);
            if (ret != HI_SUCCESS) {
                return ret;
            }
            break;
        case TDE_SINGLE_SRC_PATTERN_FILL_OPT:
        case TDE_DOUBLE_SRC_PATTERN_FILL_OPT:
            ret = tde_hal_node_opt(hw_node, mode, alu, capability);
            if (ret != HI_SUCCESS) {
                return ret;
            }
            break;
#endif
        case TDE_NORM_BLIT_2OPT:
            ret = tde_hal_node_operation_support(alu, capability);
            if (ret != HI_SUCCESS) {
                return ret;
            }
            /* fall-through */
        case TDE_NORM_BLIT_1OPT:
        case TDE_MB_2OPT:
            tde_hal_node_set_base_hw(hw_node, mode, alu);
            break;
        default:
            break;
    }
    return HI_SUCCESS;
}

static hi_bool tde_hal_is_operation_support(tde_alu_mode alu_mode, hi_u32 capability, hi_u32 oper_mode)
{
    if (!((hi_u32)MASKBLEND & oper_mode) && !((hi_u32)MASKROP & oper_mode)) {
        return (hi_bool)((capability & oper_mode) ? HI_TRUE : HI_FALSE);
    }

#if (TDE_CAPABILITY & MASKBLEND)
    if ((hi_u32)MASKBLEND & oper_mode) {
        return (hi_bool)(!((alu_mode == TDE_ALU_MASK_BLEND) && (!(capability & oper_mode))));
    }
#endif

#if (TDE_CAPABILITY & ROP)
    if ((hi_u32)MASKROP & oper_mode) {
        return (hi_bool)(!(((alu_mode == TDE_ALU_MASK_ROP1) || (alu_mode == TDE_ALU_MASK_ROP2)) &&
                         (!(capability & oper_mode))));
    }
#endif

    return HI_TRUE;
}

hi_void tde_hal_node_set_global_alpha(tde_hw_node *hw_node, hi_u8 alpha, hi_bool enable)
{
#ifndef HI_BUILD_IN_BOOT
    hi_unused(enable);
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    /* set node, CNcomment: Configure the cache node */
    hw_node->src2_cbmpara.bits.s2_galpha = alpha;

#endif
    return;
}

hi_void tde_hal_node_set_src1_alpha(tde_hw_node *hw_node)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    /* set alpha0 alpha1, CNcomment:alpha0, alpha1 */
    hw_node->src1_alpha.bits.src1_alpha0 = 0;
    hw_node->src1_alpha.bits.src1_alpha1 = 0xff; /* 0xff alpha1 value */

    return;
}

hi_void tde_hal_node_set_src2_alpha(tde_hw_node *hw_node)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    /* set alpha0 alpha1, CNcomment:alpha0, alpha1 */
    hw_node->src2_alpha.bits.src2_alpha0 = 0;
    hw_node->src2_alpha.bits.src2_alpha1 = 0xff; /* 0xff alpha1 value */

    return;
}

/*
 * Function:      tde_hal_node_set_exp_alpha
 * Description:   extend to alpha0 and alpha1 operation when extend alpha for RGB5551
 * Input:         hw_node:pointer of node
 *                alpha0: Alpha0 value
 *                alpha1: Alpha1 value
 */
hi_void tde_hal_node_set_exp_alpha(tde_hw_node *hw_node, tde_src_mode src, hi_u8 alpha0, hi_u8 alpha1)
{
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return;
    }

    /* set alpha0 alpha1, CNcomment:alpha0, alpha1 */
    if ((hi_u32)src & TDE_DRV_SRC_S1) {
        hw_node->src1_alpha.bits.src1_alpha0 = alpha0;
        hw_node->src1_alpha.bits.src1_alpha1 = alpha1;
        hw_node->src1_ctrl.bits.src1_rgb_exp = 3; /* 3 set alpha0 alpha1 */
    }

    if ((hi_u32)src & TDE_DRV_SRC_S2) {
        hw_node->src2_alpha.bits.src2_alpha0 = alpha0;
        hw_node->src2_alpha.bits.src2_alpha1 = alpha1;
        hw_node->src2_ctrl.bits.src2_rgb_exp = 3; /* 3 set alpha0 alpha1 */
    }

    return;
}

#if (TDE_CAPABILITY & ROP)
hi_s32 tde_hal_node_set_rop(tde_hw_node *hw_node, hi_tde_rop_mode rgb_rop, hi_tde_rop_mode alpha_rop)
{
#ifndef HI_BUILD_IN_BOOT
    hi_u32 capability;
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    tde_hal_get_capability(&capability);
    if (!(capability & ROP)) {
        tde_error("It deos not support Rop\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    /* set node, CNcomment: Configure the cache node */
    hw_node->cbmctrl.bits.cbm_en = 1;
    hw_node->cbmctrl.bits.cbm_mode = 1;

    hw_node->cbmalupara.bits.rgb_rop = (hi_u32)rgb_rop;
    hw_node->cbmalupara.bits.rop = (hi_u32)alpha_rop;
#endif
    return HI_SUCCESS;
}
#endif

#ifndef HI_BUILD_IN_BOOT
static hi_void tde_hal_node_blend_mode(tde_hw_node *hw_node, hi_tde_blend_opt *blend_opt)
{
    if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_NONE) {
        /* fs: sa      fd: 1.0-sa */
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_INVSRC2ALPHA, HI_TDE_BLEND_SRC2ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_CLEAR) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ZERO, HI_TDE_BLEND_ZERO);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_SRC) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ZERO, HI_TDE_BLEND_ONE);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_SRCOVER) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_INVSRC2ALPHA, HI_TDE_BLEND_ONE);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_DSTOVER) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ONE, HI_TDE_BLEND_INVSRC1ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_SRCIN) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ZERO, HI_TDE_BLEND_SRC1ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_DSTIN) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_SRC2ALPHA, HI_TDE_BLEND_ZERO);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_SRCOUT) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ZERO, HI_TDE_BLEND_INVSRC1ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_DSTOUT) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_INVSRC2ALPHA, HI_TDE_BLEND_ZERO);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_SRCATOP) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_INVSRC2ALPHA, HI_TDE_BLEND_SRC1ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_DSTATOP) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_SRC2ALPHA, HI_TDE_BLEND_INVSRC1ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_ADD) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ONE, HI_TDE_BLEND_ONE);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_XOR) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_INVSRC2ALPHA, HI_TDE_BLEND_INVSRC1ALPHA);
    } else if (blend_opt->blend_cmd == HI_TDE_BLEND_CMD_DST) {
        drv_tde_node_blend_mode(hw_node, HI_TDE_BLEND_ONE, HI_TDE_BLEND_ZERO);
    } else {
        /* user parameter, CNcomment: The user's own configuration parameters */
        drv_tde_node_blend_mode(hw_node, blend_opt->src1_blend_mode, blend_opt->src2_blend_mode);
    }
}
#endif

hi_s32 tde_hal_node_set_blend(tde_hw_node *hw_node, hi_tde_blend_opt *blend_opt)
{
#ifndef HI_BUILD_IN_BOOT
    hi_u32 capability = 0;

    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (blend_opt == HI_NULL) {
        tde_error("pstBlendOpt is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    tde_hal_get_capability(&capability);
    if (!(capability & ALPHABLEND)) {
        tde_error("It deos not support Blend\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    hw_node->src1_cbmpara.bits.s1_palphaen = HI_TRUE;
    hw_node->src1_cbmpara.bits.s1_galphaen = HI_FALSE;
    hw_node->src1_cbmpara.bits.s1_multiglobalen = HI_FALSE;
    hw_node->src2_cbmpara.bits.s2_multiglobalen = HI_FALSE;

    hw_node->src1_cbmpara.bits.s1_premulten = blend_opt->src1_alpha_premulti;
    hw_node->src2_cbmpara.bits.s2_premulten = blend_opt->src2_alpha_premulti;
    hw_node->src2_cbmpara.bits.s2_palphaen = blend_opt->pixel_alpha_en;
    hw_node->src2_cbmpara.bits.s2_galphaen = blend_opt->global_alpha_en;

    /* set mode for src1 and src2 , CNcomment: configuration Src1,Src2 mode */
    tde_hal_node_blend_mode(hw_node, blend_opt);
    hw_node->cbmctrl.bits.cbm_en = 1;
    hw_node->cbmctrl.bits.cbm_mode = 1;
#endif
    return HI_SUCCESS;
}

#if (TDE_CAPABILITY & COLORIZE)
hi_s32 tde_hal_node_set_colorize(tde_hw_node *hw_node, hi_u32 colorize)
{
    hi_u32 capability = 0;
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    tde_hal_get_capability(&capability);
    if (!(capability & COLORIZE)) {
        tde_error("It deos not support Colorize\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    hw_node->cbmcolorize.bits.colorizeen = 1;
    hw_node->cbmcolorize.bits.colorizeb = colorize & 0xff;
    hw_node->cbmcolorize.bits.colorizeg = (colorize >> 8) & 0xff; /* 8 move to a minimum */
    hw_node->cbmcolorize.bits.colorizer = (colorize >> 16) & 0xff; /* 16 move to a minimum */
    return HI_SUCCESS;
}
#endif

hi_void tde_hal_node_enable_alpha_rop(tde_hw_node *hw_node)
{
    hw_node->cbmalupara.bits.blendropen = 1;
    return;
}

/*
 * Function:      tde_hal_node_set_clut_opt
 * Description:   set color extend or color revise parameter
 * Input:         hw_node: pointer of node
 *                clut_cmd: Clut operation parameter
 */
hi_s32 tde_hal_node_set_clut_opt(tde_hw_node *hw_node, tde_clut_cmd *clut_cmd, hi_bool reload)
{
#ifndef HI_BUILD_IN_BOOT
    hi_u32 capability = 0;
    hi_unused(reload);
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (clut_cmd == HI_NULL) {
        tde_error("pClutCmd is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    tde_hal_get_capability(&capability);
    if (!(capability & CLUT)) {
        tde_error("It deos not support Clut\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    hw_node->src2_ctrl.bits.src2_clut_mode = (hi_u32)clut_cmd->clut_mode;
    /* set node, CNcomment:Configure the cache node */
    if (clut_cmd->phy_clut_addr) {
        hw_node->tde_clut_addr_low = GetLowAddr(clut_cmd->phy_clut_addr);
        hw_node->tde_clut_addr_high = GetHighAddr(clut_cmd->phy_clut_addr);
    }

#endif
    return HI_SUCCESS;
}

/*
 * Function:      tde_hal_node_set_colorkey
 * Description:   set parameter for color key operation  according color format
 * Input:         hw_node:pointer of node
 *                fmt_cat: color format
 *                color_key: pointer of color key value
 */
hi_s32 tde_hal_node_set_colorkey(tde_hw_node *hw_node, tde_colorfmt_category fmt_cat, tde_color_key_cmd *color_key)
{
#ifndef HI_BUILD_IN_BOOT
    hi_u32 capability = 0;

    if (hw_node == HI_NULL) {
        tde_error("hw_node is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (color_key == HI_NULL) {
        tde_error("color_key is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    tde_hal_get_capability(&capability);
    if (!(capability & COLORKEY)) {
        tde_error("It deos not support ColorKey\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    hw_node->cbmkeypara.bits.keysel = (hi_u32)color_key->color_key_mode;
    hw_node->cbmkeypara.bits.keyen = 1;
    if (fmt_cat == TDE_COLORFMT_CATEGORY_ARGB) {
        tde_hal_node_set_argbcolor_key_para(hw_node, color_key);
    } else if (fmt_cat == TDE_COLORFMT_CATEGORY_CLUT) {
        tde_hal_node_set_clutcolor_key_para(hw_node, color_key);
    } else if (fmt_cat == TDE_COLORFMT_CATEGORY_YCBCR) {
        tde_hal_node_set_ycbcr_color_key_para(hw_node, color_key);
    } else {
        tde_error("The clorfmt deos not support ColorKey\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    hw_node->cbmctrl.bits.cbm_en = 1;
    hw_node->cbmctrl.bits.cbm_mode = 1;

#endif
    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
static hi_void tde_hal_node_set_ycbcr_color_key_para(tde_hw_node *hw_node, tde_color_key_cmd *color_key)
{
    hi_tde_color_key_comp cr = color_key->color_key_value.ycbcr_color_key.cr;
    hi_tde_color_key_comp cb = color_key->color_key_value.ycbcr_color_key.cb;
    hi_tde_color_key_comp y = color_key->color_key_value.ycbcr_color_key.y;
    hi_tde_color_key_comp alpha = color_key->color_key_value.ycbcr_color_key.alpha;

    hw_node->cbmkeymin.all =
        tde_hal_get_ycb_cr_key_mask(cr.component_min, cb.component_min, y.component_min, alpha.component_min);
    hw_node->cbmkeymax.all =
        tde_hal_get_ycb_cr_key_mask(cr.component_max, cb.component_max, y.component_max, alpha.component_max);
    hw_node->cbmkeymask.all =
        tde_hal_get_ycb_cr_key_mask(cr.component_mask, cb.component_mask, y.component_mask, alpha.component_mask);

    hw_node->cbmkeypara.bits.keybmode = tde_hal_get_color_key_mode(&cr);
    hw_node->cbmkeypara.bits.keygmode = tde_hal_get_color_key_mode(&cb);
    hw_node->cbmkeypara.bits.keyrmode = tde_hal_get_color_key_mode(&y);
    hw_node->cbmkeypara.bits.keyamode = tde_hal_get_color_key_mode(&alpha);

    return;
}

static hi_void tde_hal_node_set_clutcolor_key_para(tde_hw_node *hw_node, tde_color_key_cmd *color_key)
{
    hi_tde_color_key_comp clut = color_key->color_key_value.clut_color_key.clut;
    hi_tde_color_key_comp alpha = color_key->color_key_value.clut_color_key.alpha;

    hw_node->cbmkeymin.all = tde_hal_get_clut_key_mask(clut.component_min, alpha.component_min);
    hw_node->cbmkeymax.all = tde_hal_get_clut_key_mask(clut.component_max, alpha.component_max);
    hw_node->cbmkeymask.all = tde_hal_get_clut_key_mask(clut.component_mask, alpha.component_mask);

    hw_node->cbmkeypara.bits.keybmode = tde_hal_get_color_key_mode(&clut);
    hw_node->cbmkeypara.bits.keyamode = tde_hal_get_color_key_mode(&alpha);

    return;
}

static hi_void tde_hal_node_set_argbcolor_key_para(tde_hw_node *hw_node, tde_color_key_cmd *color_key)
{
    hi_tde_color_key_comp blue = color_key->color_key_value.argb_color_key.blue;
    hi_tde_color_key_comp green = color_key->color_key_value.argb_color_key.green;
    hi_tde_color_key_comp red = color_key->color_key_value.argb_color_key.red;
    hi_tde_color_key_comp alpha = color_key->color_key_value.argb_color_key.alpha;

    hw_node->cbmkeymin.all =
        tde_hal_get_argb_key_mask(blue.component_min, green.component_min, red.component_min, alpha.component_min);
    hw_node->cbmkeymax.all =
        tde_hal_get_argb_key_mask(blue.component_max, green.component_max, red.component_max, alpha.component_max);
    hw_node->cbmkeymask.all =
        tde_hal_get_argb_key_mask(blue.component_mask, green.component_mask, red.component_mask, alpha.component_mask);

    hw_node->cbmkeypara.bits.keybmode = tde_hal_get_color_key_mode(&blue);
    hw_node->cbmkeypara.bits.keygmode = tde_hal_get_color_key_mode(&green);
    hw_node->cbmkeypara.bits.keyrmode = tde_hal_get_color_key_mode(&red);
    hw_node->cbmkeypara.bits.keyamode = tde_hal_get_color_key_mode(&alpha);
    return;
}

static hi_u32 tde_hal_get_color_key_mode(hi_tde_color_key_comp *color_key)
{
    return (hi_u32)((color_key->is_component_ignore) ? TDE_COLORKEY_IGNORE :
                    (color_key->is_component_out) ? TDE_COLORKEY_AREA_OUT : TDE_COLORKEY_AREA_IN);
}

static hi_u32 tde_hal_get_ycb_cr_key_mask(hi_u8 cr, hi_u8 cb, hi_u8 cy, hi_u8 alpha)
{
    return (hi_u32)(cr | (cb << TDE_EIGHT_BITS_SHIFT) | (cy << TDE_SIXTEEN_BITS_SHIFT) |
                    (alpha << TDE_TWENTYFOUR_BITS_SHIFT));
}

static hi_u32 tde_hal_get_clut_key_mask(hi_u8 clut, hi_u8 alpha)
{
    return (hi_u32)(clut | (alpha << TDE_TWENTYFOUR_BITS_SHIFT));
}

static hi_u32 tde_hal_get_argb_key_mask(hi_u8 blue, hi_u8 green, hi_u8 red, hi_u8 alpha)
{
    return (hi_u32)(blue | (green << TDE_EIGHT_BITS_SHIFT) | (red << TDE_SIXTEEN_BITS_SHIFT) |
                    (alpha << TDE_TWENTYFOUR_BITS_SHIFT));
}
#endif

hi_s32 tde_hal_node_set_clipping(tde_hw_node *hw_node, tde_clip_cmd *clip)
{
#ifndef HI_BUILD_IN_BOOT
    hi_u32 capability = 0;

    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (clip == HI_NULL) {
        tde_error("pClip is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }

    tde_hal_get_capability(&capability);
    if (!(capability & CLIP)) {
        tde_error("It deos not support Clip\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    hw_node->des_alpha.bits.des_crop_mode = 0;
    if (!clip->inside_clip) {
        /* over clip, CNcomment: Areas outside the clip */
        hw_node->des_alpha.bits.des_crop_mode = 1;
    }
    hw_node->des_alpha.bits.des_crop_en = 1;
    hw_node->des_crop_pos_st.bits.des_crop_start_x = clip->clip_start_x;
    hw_node->des_crop_pos_st.bits.des_crop_start_y = clip->clip_start_y;
    hw_node->des_crop_pos_ed.bits.des_crop_end_x = clip->clip_end_x;
    hw_node->des_crop_pos_ed.bits.des_crop_end_y = clip->clip_end_y;
#endif
    return HI_SUCCESS;
}

#if ((HICHIP != HI3516C_V500) && (HICHIP != HI3519A_V100))
static hi_s32 tde_hal_calc_src1_set_zme(tde_hw_node *node, hi_tde_color_fmt in_fmt,
                                        hi_tde_rect *in_rect, hi_tde_rect *out_rect)
{
    tde_color_fmt in_drv_fmt;
    if (in_rect == HI_NULL) {
        tde_error("pInRect is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    if (out_rect == HI_NULL) {
        tde_error("pOutRect is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }
    if (in_fmt >= (HI_TDE_COLOR_FMT_MAX + 1)) {
        return HI_FAILURE;
    }
    in_drv_fmt = tde_get_common_drv_color_fmt((hi_u32)in_fmt);

    node->src1_vsp.bits.zme_in_fmt = tde_hal_get_zme_in_fmt(in_drv_fmt);

    node->src1_vsp.bits.zme_out_fmt = (node->src1_vsp.bits.zme_in_fmt == TDE_ZME_FMT_ARGB) ? TDE_ZME_FMT_ARGB :
                                       TDE_ZME_FMT_YUV444;

    node->src1_imgsize.bits.src1_width = in_rect->width - 1;
    node->src1_zmeireso.bits.iw = in_rect->width - 1;
    node->src1_zmeoreso.bits.ow = out_rect->width - 1;

    node->src1_imgsize.bits.src1_height = in_rect->height - 1;
    node->src1_zmeireso.bits.ih = in_rect->height - 1;
    node->src1_zmeoreso.bits.oh = out_rect->height - 1;

    node->src1_hsp.bits.hratio = (out_rect->width <= 1) ? 0 : (osal_div_u64(((hi_u64)(in_rect->width) <<
        TDE_HAL_HSTEP_FLOATLEN), (out_rect->width)));

    node->src1_vsr.bits.vratio = (out_rect->height <= 1) ? 0 : (osal_div_u64(((hi_u64)(in_rect->height) <<
        TDE_FLOAT_BITLEN), (out_rect->height)));

    node->src1_hpzme_size.bits.src1_hpzme_width = in_rect->width;
    return HI_SUCCESS;
}

hi_void tde_hal_calc_src1_filter_opt(tde_hw_node *node, hi_tde_color_fmt in_fmt,
                                     hi_tde_rect *in_rect, hi_tde_rect *out_rect, hi_tde_deflicker_mode filter_mode)
{
    tde_filtermode flt_mode = {0};
    hi_s32 ret = tde_hal_calc_src1_set_zme(node, in_fmt, in_rect, out_rect);
    if (ret != HI_SUCCESS) {
        return;
    }
    if ((out_rect->width * 16) <= in_rect->width) { /* 16 width */
        node->src1_hpzme.bits.src1_hpzme_en = 1;

        node->src1_hpzme.bits.src1_hpzme_mode = tde_hal_get_hpzme_mode(out_rect->width, in_rect->width);

        node->src1_hpzme_size.bits.src1_hpzme_width = in_rect->width / (node->src1_hpzme.bits.src1_hpzme_mode + 1) +
            ((in_rect->width % (node->src1_hpzme.bits.src1_hpzme_mode + 1)) ? 1 : 0);
    }

    node->src1_hsp.bits.hchmsc_en = 1;
    node->src1_hsp.bits.hlmsc_en = 1;

    tde_hal_get_filter_mode(filter_mode, &flt_mode);

    if (node->src1_hsp.bits.hratio != NO_HSCALE_STEP) {
        hi_u64 hf_coef_addr;
        node->src1_hsp.bits.hfir_order = 0;

        node->src1_hsp.bits.hafir_en = flt_mode.alpha_en;
        node->src1_hsp.bits.hlfir_en = flt_mode.luma_en;
        node->src1_hsp.bits.hchfir_en = flt_mode.chrome_en;

        hf_coef_addr = g_para_table.hf_coef_addr + tde_hal_get_resize_para_htable(node->src1_hsp.bits.hratio) *
                       TDE_PARA_HTABLE_SIZE;

        node->tde_src1_zme_chaddr_low = GetLowAddr(hf_coef_addr);
        node->tde_src1_zme_chaddr_high = GetHighAddr(hf_coef_addr);

        node->tde_src1_zme_lhaddr_low = node->tde_src1_zme_chaddr_low;
        node->tde_src1_zme_lhaddr_high = node->tde_src1_zme_chaddr_high;
    }

    node->src1_vsp.bits.vchmsc_en = 1;
    node->src1_vsp.bits.vlmsc_en = 1;

    if (node->src1_vsr.bits.vratio != NO_VSCALE_STEP) {
        hi_u64 vf_coef_addr;
        node->src1_vsp.bits.vafir_en = flt_mode.alpha_en;
        node->src1_vsp.bits.vlfir_en = flt_mode.luma_en;
        node->src1_vsp.bits.vchfir_en = flt_mode.chrome_en;

        vf_coef_addr = g_para_table.vf_coef_addr + tde_hal_get_resize_para_vtable(node->src1_vsr.bits.vratio) *
                       TDE_PARA_VTABLE_SIZE;

        node->tde_src1_zme_cvaddr_low = GetLowAddr(vf_coef_addr);
        node->tde_src1_zme_cvaddr_high = GetHighAddr(vf_coef_addr);

        node->tde_src1_zme_lvaddr_low = node->tde_src1_zme_cvaddr_low;
        node->tde_src1_zme_lvaddr_high = node->tde_src1_zme_cvaddr_high;
    }

    node->src1_zmeireso.bits.iw = node->src1_hpzme_size.bits.src1_hpzme_width - 1;

    node->src1_hpzme_size.bits.src1_hpzme_width = node->src1_hpzme_size.bits.src1_hpzme_width - 1;
    return;
}
#endif

static hi_s32 tde_hal_calc_src2_filter_opt_chesk(tde_hw_node *node, tde_rect_opt *rect_opt)
{
    if (node == HI_NULL) {
        tde_error("pNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (rect_opt->in_rect == HI_NULL) {
        tde_error("rect_opt->pInRect is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (rect_opt->out_rect == HI_NULL) {
        tde_error("rect_opt->pOutRect is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (rect_opt->in_fmt >= (HI_TDE_COLOR_FMT_MAX + 1)) {
        return HI_ERR_TDE_INVALID_PARA;
    }
    return HI_SUCCESS;
}

hi_s32 tde_hal_set_zme_in_limit(tde_hw_node *node, tde_rect_opt *rect_opt)
{
    hi_s32 ver_scale_coeff = 1;
    hi_s32 ret;
    if ((rect_opt->out_rect->width * 16) <= rect_opt->in_rect->width) { /* 16 2^4 alg data */
        node->src2_hpzme.bits.src2_hpzme_en = 1;
        node->src2_hpzme.bits.src2_hpzme_mode = tde_hal_get_hpzme_mode(rect_opt->out_rect->width,
            rect_opt->in_rect->width);
        if ((node->src2_hpzme.bits.src2_hpzme_mode + 1) == 0) {
            return HI_ERR_TDE_INVALID_PARA;
        }
        node->src2_hpzme_size.bits.src2_hpzme_width =
            rect_opt->in_rect->width / (node->src2_hpzme.bits.src2_hpzme_mode + 1) +
            ((rect_opt->in_rect->width % (node->src2_hpzme.bits.src2_hpzme_mode + 1)) ? 1 : 0);
        node->src2_hsp.bits.hratio = (rect_opt->out_rect->width <= 1) ? 0 :
            (osal_div_u64(((hi_u64)(node->src2_hpzme_size.bits.src2_hpzme_width) <<
            TDE_HAL_HSTEP_FLOATLEN), (rect_opt->out_rect->width)));
    }

    if ((rect_opt->out_rect->height * 16) <= rect_opt->in_rect->height) { /* 16 2^4 alg data */
        ret = tde_hal_get_ver_scale_coeff(rect_opt->out_rect->height, rect_opt->in_rect->height, &ver_scale_coeff);
        if (ret != HI_SUCCESS) {
            return ret;
        }
        if (ver_scale_coeff == 0) {
            return HI_ERR_TDE_INVALID_PARA;
        }
        node->src2_zmeireso.bits.ih = rect_opt->in_rect->height / ver_scale_coeff - 1;
        node->src2_imgsize.bits.src2_height = rect_opt->in_rect->height / ver_scale_coeff - 1;
        node->src2_ch0_stride.bits.src2_ch0_stride = node->src2_ch0_stride.bits.src2_ch0_stride * ver_scale_coeff;
        if (rect_opt->out_rect->height == 0) {
            return HI_ERR_TDE_INVALID_PARA;
        }
        node->src2_vsr.bits.vratio = osal_div_u64(((hi_u64)(node->src2_imgsize.bits.src2_height) << TDE_FLOAT_BITLEN),
            (rect_opt->out_rect->height));
    } else {
        if (rect_opt->out_rect->height == 0) {
            return HI_ERR_TDE_INVALID_PARA;
        }
        node->src2_vsr.bits.vratio = (rect_opt->out_rect->height <= 1) ? 0 :
            (osal_div_u64(((hi_u64)(rect_opt->in_rect->height) << TDE_FLOAT_BITLEN), (rect_opt->out_rect->height)));
    }
    return HI_SUCCESS;
}

hi_void tde_hal_set_zme(tde_hw_node *node, tde_rect_opt *rect_opt, hi_tde_deflicker_mode filter_mode,
    hi_bool defilicker)
{
    tde_filtermode flt_mode = {0};
    hi_u64 ch_coef_addr;
    hi_u64 cv_coef_addr;

    node->src2_hsp.bits.hchmsc_en = 1;
    node->src2_hsp.bits.hlmsc_en = 1;

    tde_hal_get_filter_mode(filter_mode, &flt_mode);

    if (node->src2_hsp.bits.hratio != NO_HSCALE_STEP) {
#if ((HICHIP == HI3516C_V500) || (HICHIP == HI3519A_V100))
        if (rect_opt->out_rect->width > MAX_LINE_BUFFER) {
            node->src2_hsp.bits.hfir_order = 1;
        }
#else
        hi_unused(rect_opt);
#endif
        if (node->src2_vsp.bits.zme_in_fmt == TDE_ZME_FMT_ARGB) {
            node->src2_hsp.bits.hlfir_en = flt_mode.alpha_en;
        } else {
            node->src2_hsp.bits.hlfir_en = flt_mode.luma_en;
        }
        node->src2_hsp.bits.hafir_en = flt_mode.alpha_en;
        node->src2_hsp.bits.hchfir_en = flt_mode.chrome_en;
        ch_coef_addr = g_para_table.hf_coef_addr + tde_hal_get_resize_para_htable(node->src2_hsp.bits.hratio) *
                       TDE_PARA_HTABLE_SIZE;

        node->tde_src2_zme_chaddr_low = GetLowAddr(ch_coef_addr);
        node->tde_src2_zme_chaddr_high = GetHighAddr(ch_coef_addr);

        node->tde_src2_zme_lhaddr_low = node->tde_src2_zme_chaddr_low;
        node->tde_src2_zme_lhaddr_high = node->tde_src2_zme_chaddr_high;
    }

    node->src2_vsp.bits.vchmsc_en = 1;
    node->src2_vsp.bits.vlmsc_en = 1;

    if ((node->src2_vsr.bits.vratio != NO_VSCALE_STEP) && (!defilicker)) {
        node->src2_vsp.bits.vafir_en = flt_mode.alpha_en;
        if (node->src2_vsp.bits.zme_in_fmt == TDE_ZME_FMT_ARGB) {
            node->src2_vsp.bits.vlfir_en = flt_mode.alpha_en;
        } else {
            node->src2_vsp.bits.vlfir_en = flt_mode.luma_en;
        }
        node->src2_vsp.bits.vchfir_en = flt_mode.chrome_en;

        cv_coef_addr = g_para_table.vf_coef_addr + tde_hal_get_resize_para_vtable(node->src2_vsr.bits.vratio) *
                       TDE_PARA_VTABLE_SIZE;
        node->tde_src2_zme_cvaddr_low = GetLowAddr(cv_coef_addr);
        node->tde_src2_zme_cvaddr_high = GetHighAddr(cv_coef_addr);

        node->tde_src2_zme_lvaddr_low = node->tde_src2_zme_cvaddr_low;
        node->tde_src2_zme_lvaddr_high = node->tde_src2_zme_cvaddr_high;
    }
}

#ifndef HI_BUILD_IN_BOOT
#if (TDE_CAPABILITY & DEFLICKER)
static hi_void tde_hal_set_defilicker(tde_hw_node *node)
{
    hi_u64 cv_deflicker_coef_addr;

    node->src2_vsp.bits.vchmsc_en = 1;
    node->src2_vsp.bits.vlmsc_en = 1;
    node->src2_vsp.bits.vafir_en = 1;
    node->src2_vsp.bits.vlfir_en = 1;
    node->src2_vsp.bits.vchfir_en = 1;

    cv_deflicker_coef_addr = g_para_table.deflicker_vf_coef_addr +
                             tde_hal_get_resize_para_vtable(node->src2_vsr.bits.vratio) * TDE_PARA_VTABLE_SIZE;

    node->tde_src2_zme_cvaddr_low = GetLowAddr(cv_deflicker_coef_addr);
    node->tde_src2_zme_cvaddr_high = GetHighAddr(cv_deflicker_coef_addr);

    node->tde_src2_zme_lvaddr_low = node->tde_src2_zme_cvaddr_low;
    node->tde_src2_zme_lvaddr_high = node->tde_src2_zme_cvaddr_high;
}
#endif
#endif

hi_s32 tde_hal_calc_src2_filter_opt(tde_hw_node *node, tde_rect_opt *rect_opt, hi_bool defilicker,
                                    hi_tde_deflicker_mode filter_mode)
{
    tde_color_fmt in_drv_fmt;
    hi_s32 ret;

    ret = tde_hal_calc_src2_filter_opt_chesk(node, rect_opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    in_drv_fmt = tde_get_common_drv_color_fmt((hi_u32)rect_opt->in_fmt);

    node->src2_vsp.bits.zme_in_fmt = tde_hal_get_zme_in_fmt(in_drv_fmt);

    node->src2_vsp.bits.zme_out_fmt = (node->src2_vsp.bits.zme_in_fmt == TDE_ZME_FMT_ARGB) ? TDE_ZME_FMT_ARGB :
                                       TDE_ZME_FMT_YUV444;

    node->src2_imgsize.bits.src2_width = rect_opt->in_rect->width - 1;
    node->src2_zmeireso.bits.iw = rect_opt->in_rect->width - 1;
    node->src2_zmeoreso.bits.ow = rect_opt->out_rect->width - 1;
    node->src2_zmeoreso.bits.oh = rect_opt->out_rect->height - 1;
    node->src2_imgsize.bits.src2_height = rect_opt->in_rect->height - 1;
    node->src2_zmeireso.bits.ih = rect_opt->in_rect->height - 1;

    node->des_imgsize.bits.des_width = rect_opt->out_rect->width - 1;
    node->des_imgsize.bits.des_height = rect_opt->out_rect->height - 1;

    node->src2_hsp.bits.hratio = (rect_opt->out_rect->width <= 1) ? 0 :
        (osal_div_u64(((hi_u64)(rect_opt->in_rect->width) << TDE_HAL_HSTEP_FLOATLEN), (rect_opt->out_rect->width)));

    node->src2_hpzme_size.bits.src2_hpzme_width = rect_opt->in_rect->width;

    ret = tde_hal_set_zme_in_limit(node, rect_opt);
    if (ret != HI_SUCCESS) {
        return ret;
    }
    tde_hal_set_zme(node, rect_opt, filter_mode, defilicker);

#ifndef HI_BUILD_IN_BOOT
#if (TDE_CAPABILITY & DEFLICKER)
    if (defilicker) {
        tde_hal_set_defilicker(node);
    }
#endif
#endif

    node->src2_zmeireso.bits.iw = node->src2_hpzme_size.bits.src2_hpzme_width - 1;

    node->src2_hpzme_size.bits.src2_hpzme_width = node->src2_hpzme_size.bits.src2_hpzme_width - 1;

    return HI_SUCCESS;
}

static hi_u32 tde_hal_get_zme_in_fmt(tde_color_fmt in_drv_fmt)
{
    hi_u32 zme_in_fmt = 0;

    hi_bool fmt_argb = (in_drv_fmt <= TDE_DRV_COLOR_FMT_ACLUT88) || ((in_drv_fmt == TDE_DRV_COLOR_FMT_YCBCR444MB)) ||
                       (in_drv_fmt == TDE_DRV_COLOR_FMT_YCBCR400MBP);
    hi_bool fmt_yuv422 = (in_drv_fmt == TDE_DRV_COLOR_FMT_YCBCR422MBH) || (in_drv_fmt == TDE_DRV_COLOR_FMT_YCBCR422);
    hi_bool fmt_yuv420 = (in_drv_fmt == TDE_DRV_COLOR_FMT_YCBCR420MB) || (in_drv_fmt == TDE_DRV_COLOR_FMT_YCBCR422MBV);

    if (fmt_argb) {
        zme_in_fmt = TDE_ZME_FMT_ARGB;
    }

    if (fmt_yuv422) {
        zme_in_fmt = TDE_ZME_FMT_YUV422;
    }

    if (fmt_yuv420) {
        zme_in_fmt = TDE_ZME_FMT_YUV420;
    }

    return zme_in_fmt;
}

static hi_u32 tde_hal_get_hpzme_mode(hi_u32 out_rect_width, hi_u32 in_rect_width)
{
    hi_u32 hpzme_mode;

    if ((out_rect_width * 32) > in_rect_width) { /* 32 out ram */
        hpzme_mode = 1;
    } else if ((out_rect_width * 64) > in_rect_width) { /* 64 out ram */
        hpzme_mode = 3; /* 3 hpzme mode */
    } else if ((out_rect_width * 128) > in_rect_width) { /* 128 out ram */
        hpzme_mode = 7; /* 7 hpzme mode */
    } else {
        hpzme_mode = 15; /* 15 hpzme mode */
    }

    return hpzme_mode;
}

static hi_void tde_hal_get_filter_mode(hi_tde_filter_mode filter_mode, tde_filtermode *flt_mode)
{
    hi_bool filter_mode_color = (filter_mode == HI_TDE_FILTER_MODE_COLOR);
    hi_bool filter_mode_alpha = (filter_mode == HI_TDE_FILTER_MODE_ALPHA);
    hi_bool filter_mode_both = (filter_mode == HI_TDE_FILTER_MODE_BOTH);
    hi_u32 alpha_en;
    hi_u32 luma_en;
    hi_u32 chrome_en;

    if (filter_mode_color) {
        chrome_en = 1;
        luma_en = 1;
        alpha_en = 0;
    } else if (filter_mode_alpha) {
        chrome_en = 0;
        luma_en = 0;
        alpha_en = 1;
    } else if (filter_mode_both) {
        chrome_en = 1;
        luma_en = 1;
        alpha_en = 1;
    } else {
        chrome_en = 0;
        luma_en = 0;
        alpha_en = 0;
    }
    flt_mode->alpha_en = alpha_en;
    flt_mode->luma_en = luma_en;
    flt_mode->chrome_en = chrome_en;

    return;
}

static hi_s32 tde_hal_get_ver_scale_coeff(hi_u32 out_rect_height, hi_u32 in_rect_height, hi_s32 *ver_scale_coeff)
{
    if ((out_rect_height * 32) >= in_rect_height) { /* 32 out ram */
        *ver_scale_coeff = 2; /* 2 ver scale coeff data */
    } else if ((out_rect_height * 64) >= in_rect_height) { /* 64 out ram */
        *ver_scale_coeff = 4; /* 4 ver scale coeff data */
    } else if ((out_rect_height * 128) >= in_rect_height) { /* 128 out ram */
        *ver_scale_coeff = 8; /* 8 ver scale coeff data */
    } else {
        tde_error("Invalid para input!\n");
        return HI_ERR_TDE_INVALID_PARA;
    }

    return HI_SUCCESS;
}

static hi_void tde_hal_node_set_color_cvt_in_rgb2yuv(tde_hw_node *hw_node)
{
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    hw_node->src1_csc_idc0.bits.cscidc0 = 0;
    hw_node->src1_csc_idc0.bits.cscidc1 = 0;
    hw_node->src1_csc_idc1.bits.cscidc2 = 0;
    hw_node->src1_csc_odc0.bits.cscodc0 = 128 * 4; /* 4 * 128 cscodc0 */
    hw_node->src1_csc_odc0.bits.cscodc1 = 128 * 4; /* 4 * 128 cscodc1 */
    hw_node->src1_csc_odc1.bits.cscodc2 = 16 * 4;  /* 4 * 16 cscodc2 */
#else
    hw_node->src1_csc_idc.bits.cscidc0 = 0;
    hw_node->src1_csc_idc.bits.cscidc1 = 0;
    hw_node->src1_csc_idc.bits.cscidc2 = 0;
    hw_node->src1_csc_odc.bits.cscodc0 = 128;   /* 128 cscodc0 */
    hw_node->src1_csc_odc.bits.cscodc1 = 128;   /* 128 cscodc1 */
    hw_node->src1_csc_odc.bits.cscodc2 = 16;    /* 16 cscodc2 */
#endif

    hw_node->src1_csc_p0.bits.cscp00 = 66 * 4;  /* 4 * 66 cscp00 */
    hw_node->src1_csc_p0.bits.cscp01 = 129 * 4; /* 4 * 129 cscp01 data */

    hw_node->src1_csc_p1.bits.cscp02 = 25 * 4;  /* 4 * 25 cscp02 */
    hw_node->src1_csc_p1.bits.cscp10 = -38 * 4; /* 4 * -38 cscp10 data */

    hw_node->src1_csc_p2.bits.cscp11 = -74 * 4; /* 4 * -74 cscp11 data */
    hw_node->src1_csc_p2.bits.cscp12 = 112 * 4; /* 112 * 4 cscp12 data */

    hw_node->src1_csc_p3.bits.cscp20 = 112 * 4; /* 4 * 112 cscp20 data */
    hw_node->src1_csc_p3.bits.cscp21 = -94 * 4; /* 4 * -94 cscp21 data */

    hw_node->src1_csc_p4.bits.cscp22 = -18 * 4; /* 4 * -18 cscp22 data */
}

static hi_void tde_hal_node_set_color_cvt_in_yuv2rgb(tde_hw_node *hw_node)
{
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    hw_node->src1_csc_idc0.bits.cscidc0 = -128 * 4; /* 4 * -128 cscidc0 data */
    hw_node->src1_csc_idc0.bits.cscidc1 = -128 * 4; /* 4 * -128 cscidc1 data */
    hw_node->src1_csc_idc1.bits.cscidc2 = -16 * 4;  /* 4 * -16 cscidc2 data */
    hw_node->src1_csc_odc0.bits.cscodc0 = 0;
    hw_node->src1_csc_odc0.bits.cscodc1 = 0;
    hw_node->src1_csc_odc1.bits.cscodc2 = 0;
#else
    hw_node->src1_csc_idc.bits.cscidc0 = -128; /* -128 cscidc0 data */
    hw_node->src1_csc_idc.bits.cscidc1 = -128; /* -128 cscidc1 data */
    hw_node->src1_csc_idc.bits.cscidc2 = -16;  /* -16 cscidc2 data */
    hw_node->src1_csc_odc.bits.cscodc0 = 0;
    hw_node->src1_csc_odc.bits.cscodc1 = 0;
    hw_node->src1_csc_odc.bits.cscodc2 = 0;
#endif

    hw_node->src1_csc_p0.bits.cscp00 = 297 * 4; /* 4 * 297 cscp00 data */
    hw_node->src1_csc_p0.bits.cscp01 = 0;

    hw_node->src1_csc_p1.bits.cscp02 = 408 * 4; /* 4 * 408 cscp02 data */
    hw_node->src1_csc_p1.bits.cscp10 = 297 * 4; /* 4 * 297 cscp10 data */

    hw_node->src1_csc_p2.bits.cscp11 = -100 * 4; /* 4 * -100 cscp11 data */
    hw_node->src1_csc_p2.bits.cscp12 = -208 * 4; /* 4 * -208 cscp12 data */

    hw_node->src1_csc_p3.bits.cscp20 = 297 * 4; /* 4 * 297 cscp20 data */
    hw_node->src1_csc_p3.bits.cscp21 = 516 * 4; /* 4 * 516 cscp21 data */

    hw_node->src1_csc_p4.bits.cscp22 = 0;
}

static hi_void tde_hal_node_set_color_cvt_out_rgb2yuv(tde_hw_node *hw_node)
{
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    hw_node->des_csc_idc0.bits.cscidc0 = -128 * 4; /* 4 * -128 cscidc0 data */
    hw_node->des_csc_idc0.bits.cscidc1 = -128 * 4; /* 4 * -128 cscidc1 data */
    hw_node->des_csc_idc1.bits.cscidc2 = -16 * 4;  /* 4 * -16 cscidc2 data */
    hw_node->des_csc_odc0.bits.cscodc0 = 0;
    hw_node->des_csc_odc0.bits.cscodc1 = 0;
    hw_node->des_csc_odc1.bits.cscodc2 = 0;
#else
    hw_node->des_csc_idc.bits.cscidc0 = -128; /* -128 cscidc0 data */
    hw_node->des_csc_idc.bits.cscidc1 = -128; /* -128 cscidc1 data */
    hw_node->des_csc_idc.bits.cscidc2 = -16;  /* -16 cscidc2 data */
    hw_node->des_csc_odc.bits.cscodc0 = 0;
    hw_node->des_csc_odc.bits.cscodc1 = 0;
    hw_node->des_csc_odc.bits.cscodc2 = 0;
#endif

    hw_node->des_csc_p0.bits.cscp00 = 297 * 4; /* 4 * 297 cscp00 data */
    hw_node->des_csc_p0.bits.cscp01 = 0;

    hw_node->des_csc_p1.bits.cscp02 = 408 * 4; /* 4 * 408 cscp02 data */
    hw_node->des_csc_p1.bits.cscp10 = 297 * 4; /* 4 * 297 cscp10 data */

    hw_node->des_csc_p2.bits.cscp11 = -100 * 4; /* 4 * -100 cscp11 data */
    hw_node->des_csc_p2.bits.cscp12 = -208 * 4; /* 4 * -208 cscp12 data */

    hw_node->des_csc_p3.bits.cscp20 = 297 * 4; /* 4 * 297 cscp20 data */
    hw_node->des_csc_p3.bits.cscp21 = 516 * 4; /* 4 * 516 cscp21 data */
    hw_node->des_csc_p4.bits.cscp22 = 0;
}

static hi_void tde_hal_node_set_color_cvt_out_yuv2rgb(tde_hw_node *hw_node)
{
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    hw_node->des_csc_idc0.bits.cscidc0 = 0;
    hw_node->des_csc_idc0.bits.cscidc1 = 0;
    hw_node->des_csc_idc1.bits.cscidc2 = 0;
    hw_node->des_csc_odc0.bits.cscodc0 = 128 * 4; /* 4 * 128 cscodc0 data */
    hw_node->des_csc_odc0.bits.cscodc1 = 128 * 4; /* 4 * 128 cscodc1 data */
    hw_node->des_csc_odc1.bits.cscodc2 = 16 * 4;  /* 4 * 16 cscodc2 data */
#else
    hw_node->des_csc_idc.bits.cscidc0 = 0;
    hw_node->des_csc_idc.bits.cscidc1 = 0;
    hw_node->des_csc_idc.bits.cscidc2 = 0;
    hw_node->des_csc_odc.bits.cscodc0 = 128; /* 128 cscodc0 data */
    hw_node->des_csc_odc.bits.cscodc1 = 128; /* 128 cscodc1 data */
    hw_node->des_csc_odc.bits.cscodc2 = 16;  /* 16 cscodc2 data */
#endif

    hw_node->des_csc_p0.bits.cscp00 = 66 * 4;  /* 4 * 66 cscp00 data */
    hw_node->des_csc_p0.bits.cscp01 = 129 * 4; /* 4 * 129 cscp01 data */

    hw_node->des_csc_p1.bits.cscp02 = 25 * 4;  /* 4 * 25 cscp02 data */
    hw_node->des_csc_p1.bits.cscp10 = -38 * 4; /* 4 * -38 cscp10 data */

    hw_node->des_csc_p2.bits.cscp11 = -74 * 4; /* 4 * -74 cscp11 data */

    hw_node->des_csc_p2.bits.cscp12 = 112 * 4; /* 4 * 112 cscp12 data */

    hw_node->des_csc_p3.bits.cscp20 = 112 * 4; /* 4 * 112 cscp20 data */

    hw_node->des_csc_p3.bits.cscp21 = -94 * 4; /* 4 * -94 cscp21 data */
    hw_node->des_csc_p4.bits.cscp22 = -18 * 4; /* 4 * -18 cscp22 data */
}

/*
 * Function:      tde_hal_node_set_color_convert
 * Description:   set parameter for color space change
 * Input:         hw_node:pointer of node
 *                conv: parameter of color space change
 */
hi_s32 tde_hal_node_set_color_convert(tde_hw_node *hw_node, tde_conv_mode_cmd *conv)
{
    hi_u32 capability = 0;
    if (hw_node == HI_NULL) {
        tde_error("pHWNode is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    if (conv == HI_NULL) {
        tde_error("pConv is null %s(line=%d)\n", __FUNCTION__, __LINE__);
        return HI_ERR_TDE_NULL_PTR;
    }
    tde_hal_get_capability(&capability);
    if (!(capability & CSCCOVERT)) {
        tde_error("It deos not support CSCCovert\n");
        return HI_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if (conv->in_conv) {
        hw_node->src2_csc_mux.bits.src2_csc_mode = 1 - (hi_u32)conv->in_src1_conv;
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
        hw_node->src1_csc_idc0.bits.csc_en = 1;
        hw_node->src1_dither_ctrl.bits.dither_en = 1;
        hw_node->src1_dither_ctrl.bits.dither_round = 0;
#else
        hw_node->src1_csc_idc.bits.csc_en = 1;
#endif
        if (conv->in_rgb2_yc) {
            tde_hal_node_set_color_cvt_in_rgb2yuv(hw_node);
        } else {
            tde_hal_node_set_color_cvt_in_yuv2rgb(hw_node);
        }
    }

    if (conv->out_conv) {
        hw_node->src2_csc_mux.bits.src2_csc_mode = 1 - (hi_u32)conv->in_src1_conv;
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
        hw_node->des_csc_idc0.bits.csc_en = 1;
        hw_node->dst_dither_ctrl.bits.dither_en = 1;
        hw_node->dst_dither_ctrl.bits.dither_round = 0;
#else
        hw_node->des_csc_idc.bits.csc_en = 1;
#endif
        if (conv->in_rgb2_yc) {
            tde_hal_node_set_color_cvt_out_rgb2yuv(hw_node);
        } else {
            tde_hal_node_set_color_cvt_out_yuv2rgb(hw_node);
        }
    }
    return HI_SUCCESS;
}

hi_s32 tde_set_node_csc(tde_hw_node *hw_node, hi_tde_csc_opt csc_opt)
{
    hi_unused(hw_node);
    hi_unused(csc_opt);
    return HI_SUCCESS;
}

static hi_u32 *tde_hal_make_hf_coef(hi_void)
{
    hi_u32 i;
    hi_u32 *hf_coef = HI_NULL;

    hf_coef = (hi_u32 *)tde_malloc(TDE_PARA_HTABLE_SIZE * TDE_PARA_HTABLE_NUM);
    if (hf_coef == HI_NULL) {
        tde_error("Alloc horizontal coef failed!HCoef table size:%d\n", TDE_PARA_HTABLE_SIZE * TDE_PARA_HTABLE_NUM);
        return HI_NULL;
    }

    for (i = 0; i < TDE_PARA_HTABLE_NUM; i++) {
         /* 4 size value */
        if (memcpy_s(hf_coef + i * (TDE_PARA_HTABLE_SIZE / 4), TDE_PARA_HTABLE_SIZE, g_tde_6x32_coef +
            i * (TDE_PARA_HTABLE_ORG_SIZE), (TDE_PARA_HTABLE_ORG_SIZE) * 4) != EOK) { /* 4 size */
            tde_error("%s:%d:memcpy_s failure\n", __FUNCTION__, __LINE__);
            tde_free(hf_coef);
            return HI_NULL;
        }
    }

#ifndef __RTOS__
    g_para_table.hf_coef_addr = wgetphy((hi_void *)hf_coef);
#else
    g_para_table.hf_coef_addr = (hi_u64)(hi_uintptr_t)hf_coef;
#endif
    return hf_coef;
}

static hi_u32 *tde_hal_make_vf_coef(hi_void)
{
    hi_u32 i;
    hi_u32 *vf_coef = HI_NULL;

    vf_coef = (hi_u32 *)tde_malloc(TDE_PARA_VTABLE_SIZE * TDE_PARA_VTABLE_NUM);
    if (vf_coef == HI_NULL) {
        tde_error("Alloc vertical coef failed!VfCoef table size:%d\n", TDE_PARA_VTABLE_SIZE * TDE_PARA_VTABLE_NUM);
        return HI_NULL;
    }
    /*
     * copy parameter according other offer way , CNcomment :According to the parameter table algorithm
     * group provides the structure of the copies
     */
    for (i = 0; i < TDE_PARA_VTABLE_NUM; i++) {
        if (memcpy_s(vf_coef + i * (TDE_PARA_VTABLE_SIZE / 4), TDE_PARA_VTABLE_SIZE, g_org_vf_coef + /* 4 size value */
            i * (TDE_PARA_VTABLE_ORG_SIZE), (TDE_PARA_VTABLE_ORG_SIZE) * 4) != EOK) { /* 4 size value */
            tde_error("secure function failure\n");
            tde_free(vf_coef);
            return HI_NULL;
        }
    }

#ifndef __RTOS__
    g_para_table.vf_coef_addr = wgetphy((hi_void *)vf_coef);
#else
    g_para_table.vf_coef_addr = (hi_u64)(hi_uintptr_t)vf_coef;
#endif
    return vf_coef;
}

#ifndef HI_BUILD_IN_BOOT
#if (TDE_CAPABILITY & DEFLICKER)
static hi_u32 *tde_hal_make_deflicker_vf_coef(hi_void)
{
    hi_u32 i;
    hi_u32 *deflicker_vf_coef = HI_NULL;

    deflicker_vf_coef = (hi_u32 *)tde_malloc(TDE_PARA_VTABLE_SIZE * TDE_PARA_VTABLE_NUM);
    if (deflicker_vf_coef == HI_NULL) {
        tde_error("Alloc Deflicker vertical coef failed!g_deflicker_vf_coef table size:%d\n",
                  TDE_PARA_VTABLE_SIZE * TDE_PARA_VTABLE_NUM);
        return HI_NULL;
    }

    for (i = 0; i < TDE_PARA_VTABLE_NUM; i++) {
        /* 4 alg data */
        if (memcpy_s(deflicker_vf_coef + i * (TDE_PARA_VTABLE_SIZE / 4), TDE_PARA_VTABLE_SIZE, g_deflicker_vf_coef +
            i * (TDE_PARA_VTABLE_ORG_SIZE), (TDE_PARA_VTABLE_ORG_SIZE) * 4) != EOK) { /* 4 alg data */
            tde_error("secure function failure\n");
            tde_free(deflicker_vf_coef);
            return HI_NULL;
        }
    }

#ifndef __RTOS__
    g_para_table.deflicker_vf_coef_addr = wgetphy((hi_void *)deflicker_vf_coef);
#else
    g_para_table.deflicker_vf_coef_addr = (hi_u32)deflicker_vf_coef;
#endif
    return deflicker_vf_coef;
}
#endif
#endif

static hi_s32 tde_hal_init_para_table(hi_void)
{
    hi_u32 *hf_coef = HI_NULL;
    hi_u32 *vf_coef = HI_NULL;
#ifndef HI_BUILD_IN_BOOT
#if (TDE_CAPABILITY & DEFLICKER)
    hi_u32 *deflicker_vf_coef = HI_NULL;
#endif
#endif
    (hi_void)memset_s(&g_para_table, sizeof(g_para_table), 0, sizeof(g_para_table));
    hf_coef = tde_hal_make_hf_coef();
    if (hf_coef == HI_NULL) {
        return HI_FAILURE;
    }
    vf_coef = tde_hal_make_vf_coef();
    if (vf_coef == HI_NULL) {
        tde_free(hf_coef);
        return HI_FAILURE;
    }
#ifndef HI_BUILD_IN_BOOT
#if (TDE_CAPABILITY & DEFLICKER)
    deflicker_vf_coef = tde_hal_make_deflicker_vf_coef();
    if (deflicker_vf_coef == HI_NULL) {
        tde_free(hf_coef);
        tde_free(vf_coef);
        return HI_FAILURE;
    }
#endif
#endif
    return HI_SUCCESS;
}

/*
 * Function:      tde_hal_cur_node
 * Description:   get the node physics address that is suspended
 * Return:        the address of current running node
 */
hi_u64 tde_hal_cur_node()
{
    hi_u32 low_addr;
    hi_u32 hi_addr;
    hi_u64 addr;

    low_addr = tde_read_reg(g_base_vir_addr, TDE_AQ_ADDR_LOW);
    hi_addr = tde_read_reg(g_base_vir_addr, TDE_AQ_ADDR_HI);
    addr = ((hi_u64)hi_addr << 32) | low_addr; /* 32 alg data */
    return addr;
}

/*
 * Function:      tde_hal_getbpp_by_fmt
 * Description:   get bpp according color of driver
 * Input:         fmt: color type
 * Output:        pitch width
 * Return:        -1: wrong format
 */
static hi_s32 tde_hal_getbpp_by_fmt(tde_color_fmt fmt)
{
    if (tde_hal_getbpp_by_fmt16(fmt) == 16) {        /* 16 fmt data */
        return 16;                                   /* 16 fmt data */
    } else if (tde_hal_getbpp_by_fmt24(fmt) == 24) { /* 24 fmt data */
        return 24;                                   /* 24 fmt data */
    } else if (tde_hal_getbpp_by_fmt32(fmt) == 32) { /* 32 fmt data */
        return 32;                                   /* 32 fmt data */
    } else if (tde_hal_getbpp_by_fmt1(fmt) == 1) {
        return 1;
    } else if (tde_hal_getbpp_by_fmt2(fmt) == 2) {   /* 2 fmt data */
        return 2;                                    /* 2 fmt data */
    } else if (tde_hal_getbpp_by_fmt4(fmt) == 4) {   /* 4 fmt data */
        return 4;                                    /* 4 fmt data */
    } else if (tde_hal_getbpp_by_fmt8(fmt) == 8) {   /* 8 fmt data */
        return 8;                                    /* 8 fmt data */
    } else {
        return -1;
    }
}

static hi_s32 tde_hal_getbpp_by_fmt16(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_RGB444:
        case TDE_DRV_COLOR_FMT_RGB555:
        case TDE_DRV_COLOR_FMT_RGB565:
        case TDE_DRV_COLOR_FMT_ARGB4444:
        case TDE_DRV_COLOR_FMT_ARGB1555:
        case TDE_DRV_COLOR_FMT_ACLUT88:
        case TDE_DRV_COLOR_FMT_YCBCR422:
        case TDE_DRV_COLOR_FMT_HALFWORD:
        case TDE_DRV_COLOR_FMT_PKGVYUY:
            return 16; /* 16 fmt data */

        default:
            return -1;
    }
}

static hi_s32 tde_hal_getbpp_by_fmt24(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_RGB888:
        case TDE_DRV_COLOR_FMT_ARGB8565:
        case TDE_DRV_COLOR_FMT_YCBCR888:
            return 24; /* 24 fmt data */
        default:
            return -1;
    }
}

static hi_s32 tde_hal_getbpp_by_fmt32(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_ARGB8888:
        case TDE_DRV_COLOR_FMT_AYCBCR8888:
        case TDE_DRV_COLOR_FMT_RABG8888:
            return 32; /* 32 fmt data */
        default:
            return -1;
    }
}

static hi_s32 tde_hal_getbpp_by_fmt1(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_CLUT1:
        case TDE_DRV_COLOR_FMT_A1:
        case TDE_DRV_COLOR_FMT_CLUT1B:
        case TDE_DRV_COLOR_FMT_A1B:
            return 1;
        default:
            return -1;
    }
}

static hi_s32 tde_hal_getbpp_by_fmt2(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_CLUT2:
        case TDE_DRV_COLOR_FMT_CLUT2B:
            return 2; /* 2 function return bpp */
        default:
            return -1; /* -1  return failure */
    }
}

static hi_s32 tde_hal_getbpp_by_fmt4(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_CLUT4:
        case TDE_DRV_COLOR_FMT_CLUT4B:
            return 4; /* 4 alg data */
        default:
            return -1;
    }
}

static hi_s32 tde_hal_getbpp_by_fmt8(tde_color_fmt fmt)
{
    switch (fmt) {
        case TDE_DRV_COLOR_FMT_CLUT8:
        case TDE_DRV_COLOR_FMT_ACLUT44:
        case TDE_DRV_COLOR_FMT_A8:
        case TDE_DRV_COLOR_FMT_BYTE:
            return 8; /* 8 fmt data */
        default:
            return -1;
    }
}

/*
 * Function:      tde_hal_get_resize_para_htable
 * Description:   get index table according the step
 * Input:         step: input step
 * Return:        index table address
 */
static hi_u32 tde_hal_get_resize_para_htable(hi_u32 step)
{
    hi_u32 index;

    if (step < TDE_RESIZE_8X32_AREA_0) {
        index = 0;
    } else if (step == TDE_RESIZE_8X32_AREA_0) {
        index = 1;
    } else if ((step > TDE_RESIZE_8X32_AREA_0) && (step <= TDE_RESIZE_8X32_AREA_1)) {
        index = 2; /* 2 ResizeParaHTable */
    } else if ((step > TDE_RESIZE_8X32_AREA_1) && (step <= TDE_RESIZE_8X32_AREA_2)) {
        index = 3; /* 3 ResizeParaHTable */
    } else if ((step > TDE_RESIZE_8X32_AREA_2) && (step <= TDE_RESIZE_8X32_AREA_3)) {
        index = 4; /* 4 ResizeParaHTable */
    } else if ((step > TDE_RESIZE_8X32_AREA_3) && (step <= TDE_RESIZE_8X32_AREA_4)) {
        index = 5; /* 5 ResizeParaHTable */
    } else {
        index = 6; /* 6 ResizeParaHTable */
    }
    return index;
}

/*
 * Function:      tde_hal_get_resize_para_vtable
 * Description:   get table of parameter for resize
 * Input:         step:input step
 * Return:        address of table
 */
static hi_u32 tde_hal_get_resize_para_vtable(hi_u32 step)
{
    hi_u32 index;

    /* get index table according step, CNcomment: According to the step length to find index table */
    if (step < TDE_RESIZE_PARA_AREA_0) {
        index = 0;
    } else if (step == TDE_RESIZE_PARA_AREA_0) {
        index = 1;
    } else if ((step > TDE_RESIZE_PARA_AREA_0) && (step <= TDE_RESIZE_PARA_AREA_1)) {
        index = 2; /* 2 index resize */
    } else if ((step > TDE_RESIZE_PARA_AREA_1) && (step <= TDE_RESIZE_PARA_AREA_2)) {
        index = 3; /* 3 index resize */
    } else if ((step > TDE_RESIZE_PARA_AREA_2) && (step <= TDE_RESIZE_PARA_AREA_3)) {
        index = 4; /* 4 index resize */
    } else if ((step > TDE_RESIZE_PARA_AREA_3) && (step <= TDE_RESIZE_PARA_AREA_4)) {
        index = 5; /* 5 index resize */
    } else {
        index = 6; /* 6 index resize */
    }
    return index;
}

/*
 * Function:      tde_hal_init_queue
 * Description:   Initialize Aq list,config the operation which is needed
 */
static hi_void tde_hal_init_queue(hi_void)
{
  /*
   * write 0 to Aq list start address register
   * CNcomment: 0 will be written Aq list first address register
   */
    tde_write_reg(g_base_vir_addr, TDE_AQ_NADDR_HI, 0);
    tde_write_reg(g_base_vir_addr, TDE_AQ_NADDR_LOW, 0);
}

#ifndef HI_BUILD_IN_BOOT
/*
 * Function:      tde_hal_set_deflicer_level
 * Description:   SetDeflicerLevel
 * Input:         deflicker_level:anti-flicker levels including:auto,low,middle,high
 */
#if (TDE_CAPABILITY & DEFLICKER)
hi_s32 tde_hal_set_deflicer_level(hi_tde_deflicker_level deflicker_level)
{
    g_deflicker_level = deflicker_level;
    return HI_SUCCESS;
}

hi_s32 tde_hal_get_deflicer_level(hi_tde_deflicker_level *deflicker_level)
{
    *deflicker_level = g_deflicker_level;
    return HI_SUCCESS;
}
#endif
#endif

hi_s32 tde_hal_set_alpha_threshold(hi_u8 alpha_threshold_value)
{
    g_alpha_threshold_value = alpha_threshold_value;

    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
hi_s32 tde_hal_get_alpha_threshold(hi_u8 *alpha_threshold_value)
{
    *alpha_threshold_value = g_alpha_threshold_value;

    return HI_SUCCESS;
}
#endif

hi_s32 tde_hal_set_alpha_threshold_state(hi_bool alpha_threshold_en)
{
    g_alpha_threshold = alpha_threshold_en;

    return HI_SUCCESS;
}

#ifndef HI_BUILD_IN_BOOT
hi_s32 tde_hal_get_alpha_threshold_state(hi_bool *alpha_threshold_en)
{
    *alpha_threshold_en = g_alpha_threshold;

    return HI_SUCCESS;
}

#ifdef CONFIG_HI_PROC_SHOW_SUPPORT
/* see define of TDE_HWNode_S */
hi_char *g_update[] = {
    "SRC1_CTRL          ",                         /* 0x100 */
    "SRC1_CH0_ADDR_HIGH ",                         /* 0x104 */
    "SRC1_CH0_ADDR_LOW  ",                         /* 0x108 */
    "SRC1_CH1_ADDR_HIGH ",                         /* 0x10c */
    "SRC1_CH1_ADDR_LOW  ",                         /* 0x110 */
    "SRC1_CH0_STRIDE    ",                         /* 0x114 */
    "SRC1_CH1_STRIDE    ",                         /* 0x118 */
    "SRC1_IMGSIZE       ",                         /* 0x11c */
    "SRC1_FILL          ",                         /* 0x120 */
    "SRC1_ALPHA         ",                         /* 0x124 */
    "SRC1_PIX_OFFSET    ",  "SRC2_CTRL          ", /* 0x150 */
    "SRC2_CH0_ADDR_HIGH ",                         /* 0x154 */
    "SRC2_CH0_ADDR_LOW  ",                         /* 0x158 */
    "SRC2_CH1_ADDR_HIGH ",                         /* 0x15c */
    "SRC2_CH1_ADDR_LOW  ",                         /* 0x160 */
    "SRC2_CH0_STRIDE    ",                         /* 0x164 */
    "SRC2_CH1_STRIDE    ",                         /* 0x168 */
    "SRC2_IMGSIZE       ",                         /* 0x16c */
    "SRC2_FILL          ",                         /* 0x170 */
    "SRC2_ALPHA         ",                         /* 0x174 */
    "SRC2_PIX_OFFSET    ",  "DES_CTRL           ", /* 0x2a0 */
    "DES_CH0_ADDR_HIGH  ",                         /* 0x2a4 */
    "DES_CH0_ADDR_LOW   ",                         /* 0x2a8 */
    "DES_CH1_ADDR_HIGH  ",                         /* 0x2ac */
    "DES_CH1_ADDR_LOW   ",                         /* 0x2b0 */
    "DES_CH0_STRIDE     ",                         /* 0x2b4 */
    "DES_CH1_STRIDE     ",                         /* 0x2b8 */
    "DES_IMGSIZE        ",                         /* 0x2bc */
    "DES_ALPHA          ",                         /* 0x2c0 */
    "DES_CROP_POS_ST    ",                         /* 0x2c4 */
    "DES_CROP_POS_ED    ",                         /* 0x2c8 */
    "DES_PIX_OFFSET     ",  "SRC1_HSP           ", /* 0x300 */
    "SRC1_HLOFFSET      ",                         /* 0x304 */
    "SRC1_HCOFFSET      ",                         /* 0x308 */
    "SRC1_VSP           ",                         /* 0x30c */
    "SRC1_VSR           ",                         /* 0x310 */
    "SRC1_VOFFSET       ",                         /* 0x314 */
    "SRC1_ZMEORESO      ",                         /* 0x318 */
    "SRC1_ZMEIRESO      ",                         /* 0x31c */
    "SRC1_HPZME         ",                         /* 0x320 */
    "SRC1_HPZME_SIZE    ",                         /* 0x324 */
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    "SRC1_CSC_IDC0      ", /* 0x328 */
    "SRC1_CSC_IDC1      ", /* 0x32c */
    "SRC1_CSC_ODC0      ", /* 0x330 */
    "SRC1_CSC_ODC1      ", /* 0x334 */
#else
    "SRC1_CSC_IDC       ", /* 0x230 */
    "SRC1_CSC_ODC       ", /* 0x234 */
#endif
    "SRC1_CSC_P0        ", /* 0x338 */
    "SRC1_CSC_P1        ", /* 0x33c */
    "SRC1_CSC_P2        ", /* 0x340 */
    "SRC1_CSC_P3        ", /* 0x344 */
    "SRC1_CSC_P4        ", /* 0x348 */
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    "SRC1_DITHER_CTRL   ", /* 0x34c */
#endif
    "SRC2_HSP           ", /* 0x350 */
    "SRC2_HLOFFSET      ", /* 0x354 */
    "SRC2_HCOFFSET      ", /* 0x358 */
    "SRC2_VSP           ", /* 0x35c */
    "SRC2_VSR           ", /* 0x360 */
    "SRC2_VOFFSET       ", /* 0x364 */
    "SRC2_ZMEORESO      ", /* 0x368 */
    "SRC2_ZMEIRESO      ", /* 0x36c */
    "SRC2_HPZME         ", /* 0x370 */
    "SRC2_HPZME_SIZE    ", /* 0x374 */
    "SRC2_CSC_MUX       ", /* 0x378 */
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    "DES_CSC_IDC0       ", /* 0x37c */
    "DES_CSC_IDC1       ", /* 0x380 */
    "DES_CSC_ODC0       ", /* 0x384 */
    "DES_CSC_ODC1       ", /* 0x388 */
#else
    "DES_CSC_IDC        ", /* 0x2c0 */
    "DES_CSC_ODC        ", /* 0x2c4 */
#endif
    "DES_CSC_P0         ", /* 0x38c */
    "DES_CSC_P1         ", /* 0x390 */
    "DES_CSC_P2         ", /* 0x394 */
    "DES_CSC_P3         ", /* 0x398 */
    "DES_CSC_P4         ", /* 0x39c */
#if ((HICHIP == HI3519A_V100) || (HICHIP == HI3516C_V500))
    "DST_DITHER_CTRL    ", /* 0x3a0 */
#endif
    "DES_DSWM           ",  /* 0x3a4 */
    "SRC2_RTT_CTRL      ",  /* 0x3f0 */
    "CBMCTRL            ",  /* 0x400 */
    "CBMBKG             ",  /* 0x404 */
    "CBMCOLORIZE        ",  /* 0x408 */
    "CBMALUPARA         ",  /* 0x40c */
    "CBMKEYPARA         ",  /* 0x410 */
    "CBMKEYMIN          ",  /* 0x414 */
    "CBMKEYMAX          ",  /* 0x418 */
    "CBMKEYMASK         ",  /* 0x41c */
    "SRC1_CBMPARA       ",  /* 0x420 */
    "SRC1_CBMSTPOS      ",  /* 0x424 */
    "SRC2_CBMPARA       ",  /* 0x428 */
    "SRC2_CBMSTPOS      ",  /* 0x32c */
    "SRC1_ZME_LHADDR_HIGH", /* 0x0 */
    "SRC1_ZME_LHADDR_LOW",  /* 0x4 */
    "SRC1_ZME_LVADDR_HIGH", /* 0x8 */
    "SRC1_ZME_LVADDR_LOW",  /* 0xc */
    "SRC1_ZME_CHADDR_HIGH", /* 0x10 */
    "SRC1_ZME_CHADDR_LOW",  /* 0x14 */
    "SRC1_ZME_CVADDR_HIGH", /* 0x18 */
    "SRC1_ZME_CVADDR_LOW",  /* 0x1c */
    "SRC2_ZME_LHADDR_HIGH", /* 0x20 */
    "SRC2_ZME_LHADDR_LOW",  /* 0x24 */
    "SRC2_ZME_LVADDR_HIGH", /* 0x28 */
    "SRC2_ZME_LVADDR_LOW",  /* 0x2c */
    "SRC2_ZME_CHADDR_HIGH", /* 0x30 */
    "SRC2_ZME_CHADDR_LOW",  /* 0x34 */
    "SRC2_ZME_CVADDR_HIGH", /* 0x38 */
    "SRC2_ZME_CVADDR_LOW",  /* 0x3c */
    "CLUT_ADDR_HIGH",       /* 0x40 */
    "CLUT_ADDR_LOW",        /* 0x44 */
};

#if (HICHIP == HI3516C_V500)
static hi_void tde_hal_node_print_cv500(osal_proc_entry_t *p, hi_u32 *cur_node)
{
    hi_s32 i;
    for (i = 0; i < 11; i++) { /* 11 SRC1_CTRL 0x100 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i], *(cur_node + i + 64)); /* 64 cur node data */
    }

    for (i = 0; i < 11; i++) { /* 11 SRC2_CTRL  0x150 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 11], *(cur_node + i + 84)); /* 11 84 cur node data */
    }

    for (i = 0; i < 12; i++) { /* 12 DES_CTRL 0x2a0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 22], *(cur_node + i + 168)); /* 22 168 cur node data */
    }

    for (i = 0; i < 42; i++) { /* 42 SRC1_HSP 0x300 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 34], *(cur_node + i + 192)); /* 34 192 cur node data */
    }
    for (i = 0; i < 1; i++) { /* 1 SRC2_RTT_CTRL 0x3f0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 76], *(cur_node + i + 252)); /* 76 252 cur node data */
    }
    for (i = 0; i < 12; i++) { /* 12 CBMCTRL 0x400 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 77], *(cur_node + i + 256)); /* 77 256 cur node data */
    }
    for (i = 0; i < 18; i++) { /* 18 TDE_SRC1_ZME_LHADDR 0x0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 89], *(cur_node + i)); /* 89 cur node data */
    }
}
#elif (HICHIP == HI3519A_V100)
static hi_void tde_hal_node_print_19a(osal_proc_entry_t *p, hi_u32 *cur_node)
{
    hi_s32 i;
    for (i = 0; i < 22; i++) { /* 22 SRC1_CTRL 0x0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i], *(cur_node + i));
    }

    for (i = 0; i < 12; i++) { /* 12 DES_CTRL  0x0f8 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 22], *(cur_node + i + 62)); /* 22 62 cur node data */
    }

    for (i = 0; i < 8; i++) { /* 8 SRC1_HSP 0x200 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 34], *(cur_node + i + 128)); /* 34 128 cur node data */
    }

    for (i = 0; i < 12; i++) { /* 12 SRC1_HPZME 0x228 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 42], *(cur_node + i + 138)); /* 42 138 cur node data */
    }

    for (i = 0; i < 8; i++) { /* 8 SRC2_HSP 0x280 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 54], *(cur_node + i + 160)); /* 54 160 cur node data */
    }

    for (i = 0; i < 14; i++) { /* 14 SRC2_HPZME 0x2a8 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 62], *(cur_node + i + 170)); /* 62 170 cur node data */
    }

    for (i = 0; i < 1; i++) { /* SRC2_RTT_CTRL 0x3f0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 76], *(cur_node + i + 252)); /* 76 252 cur node data */
    }

    for (i = 0; i < 12; i++) { /* 12 CBMCTRL 0x300 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 77], *(cur_node + i + 192)); /* 77 192 cur node data */
    }

    for (i = 0; i < 18; i++) { /* 18 TDE_SRC1_ZME_LHADDR 0x480 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 89], *(cur_node + i + 288)); /* 89 288 cur node data */
    }
}
#else
static hi_void tde_hal_node_print_all(osal_proc_entry_t *p, hi_u32 *cur_node)
{
    hi_s32 i;
    for (i = 0; i < 22; i++) { /* 22 SRC1_CTRL 0x0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i], *(cur_node + i));
    }

    for (i = 0; i < 12; i++) { /* 12 DES_CTRL  0x0f8 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 22], *(cur_node + i + 62)); /* 62 22 cur node data */
    }

    for (i = 0; i < 8; i++) { /* 8 SRC1_HSP 0x200 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 34], *(cur_node + i + 128)); /* 34 128 cur node data */
    }

    for (i = 0; i < 9; i++) { /* 9 SRC1_HPZME 0x228 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 42], *(cur_node + i + 138)); /* 42 138 cur node data */
    }

    for (i = 0; i < 8; i++) { /* 8 SRC2_HSP 0x280 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 51], *(cur_node + i + 160)); /* 51 160 cur node data */
    }

    for (i = 0; i < 3; i++) { /* 3 SRC2_HPZME 0x2a8 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 59], *(cur_node + i + 170)); /* 59 170 cur node data */
    }

    for (i = 0; i < 8; i++) { /* 8 DES_CSC_IDC 0x2c0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 62], *(cur_node + i + 176)); /* 62 176 cur node data */
    }

    for (i = 0; i < 1; i++) { /* SRC2_RTT_CTRL 0x3f0 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 70], *(cur_node + i + 252)); /* 70 252 cur node data */
    }

    for (i = 0; i < 12; i++) { /* 12 CBMCTRL 0x300 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 71], *(cur_node + i + 192)); /* 71 192 cur node data */
    }

    for (i = 0; i < 18; i++) { /* 18 TDE_SRC1_ZME_LHADDR 0x480 */
        osal_seq_printf(p, "(%s):\t0x%08x\n", g_update[i + 83], *(cur_node + i + 288)); /* 83 288 cur node data */
    }
}
#endif

osal_proc_entry_t *tde_hal_node_print_info(osal_proc_entry_t *p, hi_u32 *cur_node)
{
    if (p == HI_NULL) {
        return 0;
    }
    /* print node information */
    osal_seq_printf(p, "\n--------- Hisilicon TDE Node params Info ---------\n");
#if (HICHIP == HI3516C_V500)
    tde_hal_node_print_cv500(p, cur_node);
#elif (HICHIP == HI3519A_V100)
    tde_hal_node_print_19a(p, cur_node);
#else
    tde_hal_node_print_all(p, cur_node);
#endif
    return p;
}
#endif
#endif

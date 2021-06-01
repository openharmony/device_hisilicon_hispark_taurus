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

#include "hdmi_hal_phy.h"
#include "hdmi_reg_dphy.h"
#include "hdmi_product_define.h"
#include "hdmi_reg_tx.h"
#include "hdmi_hal_intf.h"
#include "drv_hdmi_common.h"

#define PHY_LOOP_DIV_CFG_25    0x640000  /* 25 x 4 = 100 = 0x64 */
#define PHY_LOOP_DIV_CFG_22    0x5a0000  /* 22 x 4 = 90  = 0x5a */
#define PHY_FEEDTHRU_EN_MASK   (0x1 << 2)
#define PHY_FEEDTHRU_DATA_MASK 0x3
#define PHY_HWSPEC_I_16        16
#define PHY_HWSPEC_I_28        28
#define PHY_HWSPEC_I_32        32
#define PHY_HWSPEC_I_MAX       43
#define PHY_RATIO_1_10_MASK    0x1f      /* 10bit/2 eq 1 */
#define PHY_RATIO_1_40_MASK    0xfffff   /* 40bit/2 eq 1 */
#define PHY_TXPLL_TEST_OFF     0
#define PHY_TXPLL_TEST_ON      0x30000
#define PHY_POWER_ON_MASK_CLK  0xe
#define PHY_POWER_ON_MASK_D0   0xd
#define PHY_POWER_ON_MASK_D1   0xb
#define PHY_POWER_ON_MASK_D2   0x7
#define PHY_POWER_OFF          0xf
#define PHY_POWER_ON           0
#define PHY_POWER_LDO_OFF      0x3
#define PHY_OE_OFF             0xf
#define PHY_OE_ON              0
#define PHY_I_CHARGE_PUMP      0x3
#define PHY_PLL_LOOP_DEVIDER   0x20
#define TMDS_CLOCK_25M         25000
#define TMDS_CLOCK_100M        100000
#define TMDS_CLOCK_165M        165000
#define TMDS_CLOCK_340M        340000
#define HDMI_HW_PARAM_NUM      4

typedef enum {
    PHY_CLK_RATIO_1_10,
    PHY_CLK_RATIO_1_40
} phy_clk_ratio;

typedef enum {
    INTERPOLATION_MODE_1X,
    INTERPOLATION_MODE_2X,
    INTERPOLATION_MODE_4X,
    INTERPOLATION_MODE_8X,
    INTERPOLATION_MODE_BUTT
} phy_div_mode;

typedef struct {
    phy_rterm_mode r_term_mode;
    hi_u32 mode_data_clk;
    hi_u32 mode_data_d0;
    hi_u32 mode_data_d1;
    hi_u32 mode_data_d2;
} phy_rterm_mode_data;

typedef struct {
    hi_u32  phy_gc_ldo_pd;
    hi_u32  phy_gc_de_pd;
    hi_bool phy_gc_bist_pd;
    hi_bool phy_gc_txpll_pd;
    hi_bool phy_gc_rxsense_pd;
} phy_power_cfg;

typedef struct {
    hi_u32 clk_min;
    hi_u32 clk_max;
} phy_clk_range;

typedef struct {
    phy_clk_range   phy_tmds_clk_range;
    phy_hw_spec_cfg phy_spec_cfg;
} phy_hw_spec;

typedef struct {
    phy_clk_range range;
    phy_hwspec_enhance_cfg cfg;
} phy_hwspec_enhance;

typedef struct {
    phy_clk_range phy_tmds_clk_range;
    phy_ssc_cfg   ssc_cfg;
} phy_ssc;

typedef struct {
    phy_clk_range   pix_clk_range;
    hdmi_deep_color deep_color;
    hi_u32          pixel_clk_div;
    hi_u32          ssc_byp_div;
    hi_u32          tmds_nxdiv;
    hi_u32          loop_div;
    phy_clk_ratio   clk_ratio;
    phy_div_mode    div_mode;
} phy_div_cfg;

static hdmi_phy_info g_hdmi_phy_info;

static phy_rterm_mode_data g_phy_rterm_mode_deta[] = {
    /* R term mode                clk   d0   d1     d2 */
    { PHY_RTERM_MODE_SINGLE,     0x03, 0x0c, 0x30, 0xc0 },
    { PHY_RTERM_MODE_SOURCE,     0x02, 0x08, 0x20, 0x80 },
    { PHY_RTERM_MODE_LOAD,       0x01, 0x04, 0x10, 0x40 },
    { PHY_RTERM_MODE_DOUBLEDIFF, 0x00, 0x00, 0x00, 0x00 }
};

#ifdef PHY_CRAFT_S28
/* 1.0 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_1p0[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_35,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_45, PHY_FEEDTHRU_CAP_DELAY_45, PHY_FEEDTHRU_CAP_DELAY_45, PHY_FEEDTHRU_CAP_DELAY_45,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 20, 30, 30, 30,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_35, PHY_FEEDTHRU_CAP_DELAY_35,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 1.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_1p5[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 21, 31, 31, 31,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 2.0 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_2p0[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  1,  1,  1, 20, 27, 27, 27,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 21, 31, 31, 31,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 2.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_2p5[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 27, 27, 27,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 21, 32, 32, 32,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 3.0 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_3p0[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 27, 27, 27,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 21, 32, 32, 32,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 3.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_3p5[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  1,  1,  1, 20, 27, 27, 27,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  1,  1,  1, 22, 33, 33, 33,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 4.0 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_4p0[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  1,  1,  1, 20, 28, 28, 28,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  1,  1,  1, 22, 33, 33, 33,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 4.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_4p5[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  2,  2,  2, 20, 28, 28, 28,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  2,  2,  2, 22, 34, 34, 34,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};

/* 5.0 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance_5p0[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 25, 25, 25,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  2,  2,  2, 20, 28, 28, 28,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  2,  2,  2, 22, 34, 34, 34,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};
#endif

/*
 * |-tmdsclk_range---|
 * |   min   max     |
 * |---de_main----||-i_main/0.5m_a--||---de_pre---- ||-i_pre/0.25m_a--|
 * | clk d0 d1 d2 ||  clk d0 d1 d2  || clk d0 d1 d2 ||  clk d0 d1 d2  |
 * |---de_r_pre/ohm---||---r_pre/ohm---|
 * |   clk d0 d1 d2   || clk d0 d1 d2  |
 * |---------------------------------------------r_term-------------------------------------------|
 * | clk_rterm_mode clk_rterm d0_rterm_mode d0_rterm d1_rterm_mode d1_rterm d2_rterm_mode d2_rterm|
 */
#if defined(CHIP_TYPE_hi3531dv200)
/* default 2.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 27, 27, 27,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 21, 32, 32, 32,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};
#elif defined(CHIP_TYPE_hi3535av100)
/* default 3.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 27, 27, 27,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  1,  1,  1, 22, 33, 33, 33,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};
#elif defined(CHIP_TYPE_hi3521dv200)
/* default 2.5 inch */
static phy_hwspec_enhance g_phy_hw_spec_enhance[] = {
    {{  25000, 100000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 100001, 165000 },
     { 0,  0,  0,  0, 20, 26, 26, 26,
       HI_FALSE,  HI_FALSE,  HI_FALSE,  HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, 0x50 }
    },
    {{ 165001, 340000 },
     { 0,  0,  0,  0, 20, 27, 27, 27,
       HI_FALSE,  HI_TRUE,  HI_TRUE,  HI_TRUE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55, PHY_FEEDTHRU_CAP_DELAY_55,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x30, PHY_RTERM_MODE_DOUBLEDIFF, 0x30,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x30, 0x50 }
    },
    {{ 340001, 600000 },
     { 0,  0,  0,  0, 21, 32, 32, 32,
       HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE,
       PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18, PHY_FEEDTHRU_CAP_DELAY_18,
       PHY_RTERM_MODE_LOAD, 0x60, PHY_RTERM_MODE_LOAD, 0x70, PHY_RTERM_MODE_LOAD, 0x70,
       PHY_RTERM_MODE_LOAD, 0x70, 0x50 }
    }
};
#elif defined(CHIP_TYPE_hi3516cv500)
#ifndef PHY_SPEC_DEBUG
/* 2 inch */
static phy_hw_spec g_phy_hw_spec[] = {
    {{ 25000,  100000 },
     { 0, 0, 0, 0, 22, 30, 30, 30, 0, 0,  0,  0,  20, 20, 20, 20,
       PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_LOAD, 0x00, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20 }
    },
    {{ 100001, 165000 },
     { 0, 0, 0, 0, 18, 26, 26, 27, 0, 0,  0,  0,  24, 24, 24, 24,
       PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_LOAD, 0x03, PHY_RTERM_MODE_DOUBLEDIFF, 0x60,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x60 }
    },
    {{ 165001, 340000 },
     { 0, 0, 0, 0, 18, 29, 29, 29, 0, 32, 32, 32, 20, 32, 32, 32,
       PHY_RPRE_83, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_83, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50,
       PHY_RTERM_MODE_LOAD, 0x03, PHY_RTERM_MODE_DOUBLEDIFF, 0x60,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x60 }
    },
    {{ 340001, 600000 },
     { 5, 5, 5, 5, 29, 43, 43, 43, 0, 25, 25, 25, 20, 20, 20, 20,
       PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_SOURCE, 0x00, PHY_RTERM_MODE_LOAD, 0x80, PHY_RTERM_MODE_LOAD, 0x80, PHY_RTERM_MODE_LOAD, 0x80 }
    }
};

/* 3 inch */
static phy_hw_spec g_phy_hw_spec1[] = {
    {{ 25000,  100000 },
     { 0, 0, 0, 0, 22, 30, 30, 31, 0, 0,  0,  0,  20, 20, 20, 20,
       PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_LOAD, 0x00, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20 }
    },
    {{ 100001, 165000 },
     { 0, 0, 0, 0, 18, 26, 26, 27, 0, 0,  0,  0,  24, 24, 24, 24,
       PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_LOAD, 0x03, PHY_RTERM_MODE_DOUBLEDIFF, 0x60,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x60 }
    },
    {{ 165001, 340000 },
     { 0, 0, 0, 0, 18, 29, 29, 30, 0, 32, 32, 32, 20, 32, 32, 32,
       PHY_RPRE_83, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_83, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50,
       PHY_RTERM_MODE_LOAD, 0x03, PHY_RTERM_MODE_DOUBLEDIFF, 0x60,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x60 }
    },
    {{ 340001, 600000 },
     { 5, 5, 5, 5, 29, 43, 43, 43, 0, 25, 25, 25, 20, 20, 20, 20,
       PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_SOURCE, 0x00, PHY_RTERM_MODE_LOAD, 0x80, PHY_RTERM_MODE_LOAD, 0x80, PHY_RTERM_MODE_LOAD, 0x80 }
    }
};

/* 5 inch */
static phy_hw_spec g_phy_hw_spec2[] = {
    {{ 25000,  100000 },
     { 0, 0, 0, 0, 22, 30, 30, 30, 0, 0,  0,  0,  20, 20, 20, 20,
       PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_LOAD, 0x00, PHY_RTERM_MODE_DOUBLEDIFF, 0x20,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x20, PHY_RTERM_MODE_DOUBLEDIFF, 0x20 }
    },
    {{ 100001, 165000 },
     { 0, 0, 0, 0, 18, 26, 26, 27, 0, 0,  0,  0,  24, 24, 24, 24,
       PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_LOAD, 0x03, PHY_RTERM_MODE_DOUBLEDIFF, 0x60,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x60, PHY_RTERM_MODE_DOUBLEDIFF, 0x60 }
    },
    {{ 165001, 340000 },
     { 0, 4, 4, 4, 18, 26, 26, 27, 0, 32, 32, 32, 20, 32, 32, 32,
       PHY_RPRE_83, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_83, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50,
       PHY_RTERM_MODE_LOAD, 0x03, PHY_RTERM_MODE_DOUBLEDIFF, 0x04,
       PHY_RTERM_MODE_DOUBLEDIFF, 0x04, PHY_RTERM_MODE_DOUBLEDIFF, 0x04 }
    },
    {{ 340001, 600000 },
     { 5, 5, 5, 5, 29, 43, 43, 43, 0, 25, 25, 25, 20, 20, 20, 20,
       PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_50, PHY_RPRE_83, PHY_RPRE_83, PHY_RPRE_83,
       PHY_RTERM_MODE_SOURCE, 0x00, PHY_RTERM_MODE_LOAD, 0x80, PHY_RTERM_MODE_LOAD, 0x80, PHY_RTERM_MODE_LOAD, 0x80 }
    }
};
#endif
#endif

/* spread spectrum ctrl(ssc) */
static phy_ssc g_phy_ssc_cfg[] = {
    {{ 25000,  70000  }, { 0,  0     }},  // 0ppm
    {{ 70001,  90000  }, { 33, 45000 }},  // 0.33% eq 3300ppm
    {{ 90001,  110000 }, { 25, 45000 }},  // 0.25%
    {{ 110001, 145000 }, { 20, 45000 }},  // 0.20%
    {{ 145001, 180000 }, { 12, 45000 }},  // 0.12%
    {{ 180001, 220000 }, { 10, 45000 }},  // 0.10%
    {{ 220001, 250000 }, { 8,  45000 }},  // 0.8%
    {{ 250001, 300000 }, { 4,  45000 }},  // 0.4%
    {{ 300001, 600000 }, { 0,  0     }}   // 0
};

/*
 * |-tmdsclk_range---|
 * |   min   max     |
 * |   deepcolor --  pix_div -- byp_div -- tmds_nx_div -- loopdiv -- clk_ratio -- phy_div_mode
 */
static phy_div_cfg g_phy_div_cfg[] = {
    {{ 25000,  50000  },
        HDMI_DEEP_COLOR_24BIT, 16, 10, 8, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_8X
    },
    {{ 50001,  100000 },
        HDMI_DEEP_COLOR_24BIT, 8,  10, 4, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_4X
    },
    {{ 100001, 200000 },
        HDMI_DEEP_COLOR_24BIT, 4,  10, 2, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_2X
    },
    {{ 200001, 340000 },
        HDMI_DEEP_COLOR_24BIT, 2,  10, 1, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_1X
    },
    {{ 340001, 600000 },
        HDMI_DEEP_COLOR_24BIT, 1,  5,  1, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_40, INTERPOLATION_MODE_1X
    },
    {{ 25001,  50000  },
        HDMI_DEEP_COLOR_30BIT, 16, 8, 8, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_8X
    },
    {{ 50001,  100000 },
        HDMI_DEEP_COLOR_30BIT, 8,  8, 4, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_4X
    },
    {{ 100001, 200000 },
        HDMI_DEEP_COLOR_30BIT, 4,  8, 2, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_2X
    },
    {{ 200001, 272000 },
        HDMI_DEEP_COLOR_30BIT, 2,  8, 1, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_1X
    },
    {{ 272001, 340000 },
        HDMI_DEEP_COLOR_30BIT, 2,  8, 1, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_40, INTERPOLATION_MODE_1X
    },
    {{ 340001, 600000 },
        HDMI_DEEP_COLOR_30BIT, 1,  8, 1, PHY_LOOP_DIV_CFG_25, PHY_CLK_RATIO_1_40, INTERPOLATION_MODE_1X
    },
    {{ 25001,  50000  },
        HDMI_DEEP_COLOR_36BIT, 16, 6, 8, PHY_LOOP_DIV_CFG_22, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_8X
    },
    {{ 50001,  100000 },
        HDMI_DEEP_COLOR_36BIT, 8,  6, 4, PHY_LOOP_DIV_CFG_22, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_4X
    },
    {{ 100001, 200000 },
        HDMI_DEEP_COLOR_36BIT, 4,  6, 2, PHY_LOOP_DIV_CFG_22, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_2X
    },
    {{ 200001, 227000 },
        HDMI_DEEP_COLOR_36BIT, 2,  6, 1, PHY_LOOP_DIV_CFG_22, PHY_CLK_RATIO_1_10, INTERPOLATION_MODE_1X
    },
    {{ 227001, 340000 },
        HDMI_DEEP_COLOR_36BIT, 2,  6, 1, PHY_LOOP_DIV_CFG_22, PHY_CLK_RATIO_1_40, INTERPOLATION_MODE_1X
    },
    {{ 340001, 600000 },
        HDMI_DEEP_COLOR_36BIT, 1,  3, 1, PHY_LOOP_DIV_CFG_22, PHY_CLK_RATIO_1_40, INTERPOLATION_MODE_1X
    }
};

static phy_rterm_mode_data *phy_rterm_mode_data_get(phy_rterm_mode r_term_mode)
{
    hi_u32 i;
    phy_rterm_mode_data *phy_r_term_mode_cfg = HI_NULL;

    for (i = 0, phy_r_term_mode_cfg = &g_phy_rterm_mode_deta[0];
         (phy_r_term_mode_cfg && (i < hdmi_array_size(g_phy_rterm_mode_deta)));
         phy_r_term_mode_cfg++, i++) {
        if (phy_r_term_mode_cfg->r_term_mode == r_term_mode) {
            return phy_r_term_mode_cfg;
        }
    }
    hdmi_err("can't find param, r_term_mode:%u.\n", r_term_mode);

    return &g_phy_rterm_mode_deta[0];
}

#ifndef PHY_SPEC_DEBUG
static hi_u32 phy_transfer_cur2hex(hi_u32 cur)
{
    hi_u32 tmp;

    if (cur < PHY_HWSPEC_I_16) {
        tmp = cur;
    } else if (cur < PHY_HWSPEC_I_28) {
        tmp = (cur + 0x04);
    } else if (cur < PHY_HWSPEC_I_32) {
        tmp = (cur + 0x10);
    } else if (cur <= PHY_HWSPEC_I_MAX) {
        tmp = (cur + 0x14);
    } else {
        hdmi_err("%u is exceed max value %u.\n", cur, PHY_HWSPEC_I_MAX);
        tmp = 0;
    }

    return tmp;
}

#ifdef PHY_CRAFT_S28
static hi_s32 phy_hw_enhance_spec_set(const phy_hwspec_enhance_cfg *hw_spec_cfg)
{
    hi_u32 tmp;
    errno_t ret;

    hdmi_info("PHY HWSpec DeIMain     clk:%u, d0:%u, d1:%u, d2:%u\n",
              hw_spec_cfg->i_demain_clk, hw_spec_cfg->i_demain_d0, hw_spec_cfg->i_demain_d1, hw_spec_cfg->i_demain_d2);
    hdmi_info("PHY HWSpec iMain       clk:%u, d0:%u, d1:%u, d2:%u\n",
              hw_spec_cfg->i_main_clk, hw_spec_cfg->i_main_d0, hw_spec_cfg->i_main_d1, hw_spec_cfg->i_main_d2);
    hdmi_info("PHY HWSpec bFeedThruen clk:%u, d0:%u, d1:%u, d2:%u\n",
              hw_spec_cfg->feedthru_cap_clk_en, hw_spec_cfg->feedthru_cap_d0_en,
              hw_spec_cfg->feedthru_cap_d1_en, hw_spec_cfg->feedthru_cap_d2_en);
    hdmi_info("PHY HWSpec enFeedThru  clk:%u, d0:%u, d1:%u, d2:%u\n",
              hw_spec_cfg->feedthru_cap_delay_clk, hw_spec_cfg->feedthru_cap_delay_d0,
              hw_spec_cfg->feedthru_cap_delay_d1, hw_spec_cfg->feedthru_cap_delay_d2);
    hdmi_info("PHY HWSpec RTermMode   clk:%u, d0:%u, d1:%u, d2:%u\n",
              hw_spec_cfg->rterm_mode_clk, hw_spec_cfg->rterm_mode_d0,
              hw_spec_cfg->rterm_mode_d1, hw_spec_cfg->rterm_mode_d2);
    hdmi_info("PHY HWSpec RTerm       clk:0x%x, d0:0x%x, d1:0x%x, d2:0x%x\n",
              hw_spec_cfg->rterm_clk, hw_spec_cfg->rterm_d0, hw_spec_cfg->rterm_d1, hw_spec_cfg->rterm_d2);

    /* de iMain */
    hdmi_reg_isel_main_de_clk_set(phy_transfer_cur2hex(hw_spec_cfg->i_demain_clk));
    hdmi_reg_isel_main_de_d0_set(phy_transfer_cur2hex(hw_spec_cfg->i_demain_d0));
    hdmi_reg_isel_main_de_d1_set(phy_transfer_cur2hex(hw_spec_cfg->i_demain_d1));
    hdmi_reg_isel_main_de_d2_set(phy_transfer_cur2hex(hw_spec_cfg->i_demain_d2));
    /* iMain */
    hdmi_reg_isel_main_clk_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_clk));
    hdmi_reg_isel_main_d0_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_d0));
    hdmi_reg_isel_main_d1_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_d1));
    hdmi_reg_isel_main_d2_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_d2));
    /* Feed-Through Cap Delay control */
    tmp = hw_spec_cfg->feedthru_cap_delay_clk;
    tmp = hw_spec_cfg->feedthru_cap_clk_en ? (tmp | PHY_FEEDTHRU_EN_MASK) : (tmp & (~PHY_FEEDTHRU_EN_MASK));
    hdmi_reg_rsel_pre_clk_set(tmp);
    tmp = hw_spec_cfg->feedthru_cap_delay_d0;
    tmp = hw_spec_cfg->feedthru_cap_d0_en ? (tmp | PHY_FEEDTHRU_EN_MASK) : (tmp & (~PHY_FEEDTHRU_EN_MASK));
    hdmi_reg_rsel_pre_d0_set(tmp);
    tmp = hw_spec_cfg->feedthru_cap_delay_d1;
    tmp = hw_spec_cfg->feedthru_cap_d1_en ? (tmp | PHY_FEEDTHRU_EN_MASK) : (tmp & (~PHY_FEEDTHRU_EN_MASK));
    hdmi_reg_rsel_pre_d1_set(tmp);
    tmp = hw_spec_cfg->feedthru_cap_delay_d2;
    tmp = hw_spec_cfg->feedthru_cap_d2_en ? (tmp | PHY_FEEDTHRU_EN_MASK) : (tmp & (~PHY_FEEDTHRU_EN_MASK));
    hdmi_reg_rsel_pre_d2_set(tmp);
    /* R Term */
    hdmi_reg_rt_clk_set(hw_spec_cfg->rterm_clk);
    hdmi_reg_rt_d0_set(hw_spec_cfg->rterm_d0);
    hdmi_reg_rt_d1_set(hw_spec_cfg->rterm_d1);
    hdmi_reg_rt_d2_set(hw_spec_cfg->rterm_d2);
    /* VCM sel */
    hdmi_reg_test_set(hw_spec_cfg->vcm_sel);

    /* status info set */
    ret = memcpy_s(&g_hdmi_phy_info.spec_enhance_cfg, sizeof(g_hdmi_phy_info.spec_enhance_cfg),
        hw_spec_cfg, sizeof(phy_hwspec_enhance_cfg));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return HI_SUCCESS;
}

static hi_void trace_printf(hdmi_trace_len trace_len)
{
    switch (trace_len) {
        case HDMI_TRACE_LEN_0:
            hdmi_info("1.0 inch\n");
            break;
        case HDMI_TRACE_LEN_1:
            hdmi_info("1.5 inch\n");
            break;
        case HDMI_TRACE_LEN_2:
            hdmi_info("2.0 inch\n");
            break;
        case HDMI_TRACE_LEN_3:
            hdmi_info("2.5 inch\n");
            break;
        case HDMI_TRACE_LEN_4:
            hdmi_info("3.0 inch\n");
            break;
        case HDMI_TRACE_LEN_5:
            hdmi_info("3.5 inch\n");
            break;
        case HDMI_TRACE_LEN_6:
            hdmi_info("4.0 inch\n");
            break;
        case HDMI_TRACE_LEN_7:
            hdmi_info("4.5 inch\n");
            break;
        case HDMI_TRACE_LEN_8:
            hdmi_info("5.0 inch\n");
            break;
        default:
            hdmi_info("default inch\n");
            break;
    }
}

static hi_u32 phy_hw_enhance_spec_debug_data_get(phy_hwspec_enhance **hwspec_enhance, hdmi_trace_len trace_len)
{
    hi_u32 size;
    phy_hwspec_enhance *hwspec = HI_NULL;

    trace_printf(trace_len);

    switch (trace_len) {
        case HDMI_TRACE_LEN_0:
            hwspec = &g_phy_hw_spec_enhance_1p0[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_1p0);
            break;
        case HDMI_TRACE_LEN_1:
            hwspec = &g_phy_hw_spec_enhance_1p5[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_1p5);
            break;
        case HDMI_TRACE_LEN_2:
            hwspec = &g_phy_hw_spec_enhance_2p0[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_2p0);
            break;
        case HDMI_TRACE_LEN_3:
            hwspec = &g_phy_hw_spec_enhance_2p5[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_2p5);
            break;
        case HDMI_TRACE_LEN_4:
            hwspec = &g_phy_hw_spec_enhance_3p0[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_3p0);
            break;
        case HDMI_TRACE_LEN_5:
            hwspec = &g_phy_hw_spec_enhance_3p5[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_3p5);
            break;
        case HDMI_TRACE_LEN_6:
            hwspec = &g_phy_hw_spec_enhance_4p0[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_4p0);
            break;
        case HDMI_TRACE_LEN_7:
            hwspec = &g_phy_hw_spec_enhance_4p5[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_4p5);
            break;
        case HDMI_TRACE_LEN_8:
            hwspec = &g_phy_hw_spec_enhance_5p0[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance_5p0);
            break;
        default:
            hwspec = &g_phy_hw_spec_enhance[0];
            size = hdmi_array_size(g_phy_hw_spec_enhance);
            break;
    }

    *hwspec_enhance = hwspec;

    return size;
}

static phy_hwspec_enhance_cfg *phy_hw_enhance_spec_data_get(hi_u32 tmds_clk, hdmi_trace_len trace_len)
{
    hi_u32 i;
    hi_u32 array_size;
    phy_hwspec_enhance *phy_hw_enhance_spec = HI_NULL;
    static hdmi_trace_len old_trace_len = HDMI_TRACE_BUTT;

    /* default cfg */
    if (trace_len == HDMI_TRACE_BUTT) {
        phy_hw_enhance_spec = &g_phy_hw_spec_enhance[0];
        array_size = hdmi_array_size(g_phy_hw_spec_enhance);
    } else {
        array_size = phy_hw_enhance_spec_debug_data_get(&phy_hw_enhance_spec, trace_len);
    }

    /* updata g_hdmi_phy_info just in change trace len. */
    if ((phy_hw_enhance_spec != HI_NULL) && (trace_len != old_trace_len)) {
        hdmi_info("trace change!(%u->%u), updata g_hdmi_phy_info\n", old_trace_len, trace_len);
        old_trace_len = trace_len;
        for (i = 0; i < HDMI_HW_PARAM_NUM; i++) {
            g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_clk  = phy_hw_enhance_spec[i].cfg.i_demain_clk;
            g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data = phy_hw_enhance_spec[i].cfg.i_demain_d0;
            g_hdmi_phy_info.hw_spec.hw_param[i].i_main_clk     = phy_hw_enhance_spec[i].cfg.i_main_clk;
            g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data    = phy_hw_enhance_spec[i].cfg.i_main_d0;
            g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_clk     = phy_hw_enhance_spec[i].cfg.feedthru_cap_clk_en ?
                (phy_hw_enhance_spec[i].cfg.feedthru_cap_delay_clk + 1) : 0;
            g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data    = phy_hw_enhance_spec[i].cfg.feedthru_cap_d0_en ?
                (phy_hw_enhance_spec[i].cfg.feedthru_cap_delay_d0 + 1) : 0;

            hdmi_info("stage[%u]: i_de_main_clk:%u i_de_main_data:%u\
                i_main_clk:%u i_main_data:%u ft_cap_clk:%u ft_cap_data:%u\n", i,
                g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_clk, g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data,
                g_hdmi_phy_info.hw_spec.hw_param[i].i_main_clk, g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data,
                g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_clk, g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data);
        }
    }

    for (i = 0; ((phy_hw_enhance_spec != HI_NULL) && (i < array_size)); phy_hw_enhance_spec++, i++) {
        if ((tmds_clk >= phy_hw_enhance_spec->range.clk_min) && (tmds_clk <= phy_hw_enhance_spec->range.clk_max)) {
            return (&phy_hw_enhance_spec->cfg);
        }
    }

    return HI_NULL;
}

static hi_void phy_feedthru_en(phy_hwspec_enhance_cfg *phy_spec_cfg, const hdmi_hw_param *hw_param)
{
    if (hw_param->ft_cap_clk != 0) {
        phy_spec_cfg->feedthru_cap_clk_en = HI_TRUE;
    } else {
        phy_spec_cfg->feedthru_cap_clk_en = HI_FALSE;
    }

    if (hw_param->ft_cap_data != 0) {
        phy_spec_cfg->feedthru_cap_d0_en = HI_TRUE;
        phy_spec_cfg->feedthru_cap_d1_en = HI_TRUE;
        phy_spec_cfg->feedthru_cap_d2_en = HI_TRUE;
    } else {
        phy_spec_cfg->feedthru_cap_d0_en = HI_FALSE;
        phy_spec_cfg->feedthru_cap_d1_en = HI_FALSE;
        phy_spec_cfg->feedthru_cap_d2_en = HI_FALSE;
    }

    return;
}

static hi_void phy_user_enhance_param_set(hi_u32 tmds_clk, phy_hwspec_enhance_cfg *phy_spec_cfg)
{
    hi_u8 i;
    phy_hwspec_enhance *hw_spec = HI_NULL;

    for (i = 0, hw_spec = &g_phy_hw_spec_enhance[0];
         (hw_spec != HI_NULL && (i < hdmi_array_size(g_phy_hw_spec_enhance)));
         hw_spec++, i++) {
        if ((tmds_clk >= hw_spec->range.clk_min) && (tmds_clk <= hw_spec->range.clk_max)) {
            phy_spec_cfg->i_demain_clk = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_clk;
            if (phy_spec_cfg->i_demain_d0 != g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data) {
                phy_spec_cfg->i_demain_d0  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
                phy_spec_cfg->i_demain_d1  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
                phy_spec_cfg->i_demain_d2  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
            }
            phy_spec_cfg->i_main_clk = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_clk;
            if (phy_spec_cfg->i_main_d0 != g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data) {
                phy_spec_cfg->i_main_d0  = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
                phy_spec_cfg->i_main_d1  = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
                phy_spec_cfg->i_main_d2  = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
            }
            if (g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_clk > 0) {
                phy_spec_cfg->feedthru_cap_delay_clk = g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_clk - 1;
            }
            if ((g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data > 0) &&
                (phy_spec_cfg->feedthru_cap_delay_d0 != (g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data - 1))) {
                phy_spec_cfg->feedthru_cap_delay_d0  = g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data - 1;
                phy_spec_cfg->feedthru_cap_delay_d1  = g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data - 1;
                phy_spec_cfg->feedthru_cap_delay_d2  = g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data - 1;
            }
            phy_feedthru_en(phy_spec_cfg, &g_hdmi_phy_info.hw_spec.hw_param[i]);

            break;
        }
    }

    return;
}

#else

static hi_void phy_hw_spec_print(const phy_hw_spec_cfg *hw_spec_cfg)
{
    hdmi_info("de_main   clk:%u, d0:%u, d1:%u, d2:%u\n",
        hw_spec_cfg->i_de_main_clk, hw_spec_cfg->i_de_main_d0, hw_spec_cfg->i_de_main_d1, hw_spec_cfg->i_de_main_d2);
    hdmi_info("i_main     clk:%u, d0:%u, d1:%u, d2:%u\n",
        hw_spec_cfg->i_main_clk, hw_spec_cfg->i_main_d0, hw_spec_cfg->i_main_d1, hw_spec_cfg->i_main_d2);
    hdmi_info("i_pre      clk:%u, d0:%u, d1:%u, d2:%u\n",
        hw_spec_cfg->i_pre_clk, hw_spec_cfg->i_pre_d0, hw_spec_cfg->i_pre_d1, hw_spec_cfg->i_pre_d2);
    hdmi_info("r_pre      clk:%u, d0:%u, d1:%u, d2:%u\n",
        hw_spec_cfg->r_pre_clk, hw_spec_cfg->r_pre_d0, hw_spec_cfg->r_pre_d1, hw_spec_cfg->r_pre_d2);
    hdmi_info("r_term     clk:0x%x, d0:0x%x, d1:0x%x, d2:0x%x\n",
        hw_spec_cfg->r_term_clk, hw_spec_cfg->r_term_d0, hw_spec_cfg->r_term_d1, hw_spec_cfg->r_term_d2);
}

static hi_s32 phy_hw_spec_set(const phy_hw_spec_cfg *hw_spec_cfg)
{
    hi_u32 tmp;
    errno_t ret;

    phy_hw_spec_print(hw_spec_cfg);

    /* de i_main */
    hdmi_reg_isel_main_de_clk_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_main_clk));
    hdmi_reg_isel_main_de_d0_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_main_d0));
    hdmi_reg_isel_main_de_d1_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_main_d1));
    hdmi_reg_isel_main_de_d2_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_main_d2));
    /* i_main */
    hdmi_reg_isel_main_clk_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_clk));
    hdmi_reg_isel_main_d0_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_d0));
    hdmi_reg_isel_main_d1_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_d1));
    hdmi_reg_isel_main_d2_set(phy_transfer_cur2hex(hw_spec_cfg->i_main_d2));
    /* de i_pre */
    hdmi_reg_isel_pre_de_clk_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_pre_clk));
    hdmi_reg_isel_pre_de_d0_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_pre_d0));
    hdmi_reg_isel_pre_de_d1_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_pre_d1));
    hdmi_reg_isel_pre_de_d2_set(phy_transfer_cur2hex(hw_spec_cfg->i_de_pre_d2));
    /* i_pre */
    hdmi_reg_isel_pre_clk_set(phy_transfer_cur2hex(hw_spec_cfg->i_pre_clk));
    hdmi_reg_isel_pre_d0_set(phy_transfer_cur2hex(hw_spec_cfg->i_pre_d0));
    hdmi_reg_isel_pre_d1_set(phy_transfer_cur2hex(hw_spec_cfg->i_pre_d1));
    hdmi_reg_isel_pre_d2_set(phy_transfer_cur2hex(hw_spec_cfg->i_pre_d2));
    /* de R pre */
    hdmi_reg_rsel_pre_de_clk_set((hi_u32)(hw_spec_cfg->r_de_pre_clk));
    hdmi_reg_rsel_pre_de_d0_set((hi_u32)(hw_spec_cfg->r_de_pre_d0));
    hdmi_reg_rsel_pre_de_d1_set((hi_u32)(hw_spec_cfg->r_de_pre_d1));
    hdmi_reg_rsel_pre_de_d2_set((hi_u32)(hw_spec_cfg->r_de_pre_d2));
    /* R pre */
    hdmi_reg_rsel_pre_clk_set((hi_u32)(hw_spec_cfg->r_pre_clk));
    hdmi_reg_rsel_pre_d0_set((hi_u32)(hw_spec_cfg->r_pre_d0));
    hdmi_reg_rsel_pre_d1_set((hi_u32)(hw_spec_cfg->r_pre_d1));
    hdmi_reg_rsel_pre_d2_set((hi_u32)(hw_spec_cfg->r_pre_d2));
    /* R term */
    hdmi_reg_rt_clk_set(hw_spec_cfg->r_term_clk);
    hdmi_reg_rt_d0_set(hw_spec_cfg->r_term_d0);
    hdmi_reg_rt_d1_set(hw_spec_cfg->r_term_d1);
    hdmi_reg_rt_d2_set(hw_spec_cfg->r_term_d2);
    tmp = phy_rterm_mode_data_get(hw_spec_cfg->r_term_mode_clk)->mode_data_clk;
    tmp |= phy_rterm_mode_data_get(hw_spec_cfg->r_term_mode_d0)->mode_data_d0;
    tmp |= phy_rterm_mode_data_get(hw_spec_cfg->r_term_mode_d1)->mode_data_d1;
    tmp |= phy_rterm_mode_data_get(hw_spec_cfg->r_term_mode_d2)->mode_data_d2;
    hdmi_reg_gc_pd_rterm_set(tmp);

    /* status info set */
    ret = memcpy_s(&g_hdmi_phy_info.hw_spec_cfg, sizeof(phy_hw_spec_cfg), hw_spec_cfg, sizeof(*hw_spec_cfg));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return HI_SUCCESS;
}

static phy_hw_spec_cfg *phy_hw_spec_data_get(hi_u32 tmds_clk, hdmi_trace_len trace_len)
{
    hi_u32 i;
    hi_u32 arrar_size;
    phy_hw_spec *spec = HI_NULL;

    if (trace_len == HDMI_TRACE_LEN_0) {
        spec = &g_phy_hw_spec[0];
        arrar_size = hdmi_array_size(g_phy_hw_spec);
    } else if (trace_len == HDMI_TRACE_LEN_1) {
        spec = &g_phy_hw_spec1[0];
        arrar_size = hdmi_array_size(g_phy_hw_spec1);
    } else {
        spec = &g_phy_hw_spec2[0];
        arrar_size = hdmi_array_size(g_phy_hw_spec2);
    }

    for (i = 0; (spec && (i < arrar_size)); spec++, i++) {
        if ((tmds_clk >= spec->phy_tmds_clk_range.clk_min) && (tmds_clk <= spec->phy_tmds_clk_range.clk_max)) {
            return (&spec->phy_spec_cfg);
        }
    }

    return HI_NULL;
}

static hi_void phy_user_param_set(hi_u32 tmds_clk, phy_hw_spec_cfg *phy_spec_cfg)
{
    hi_u8 i;
    phy_hw_spec *hw_spec = HI_NULL;

    for (i = 0, hw_spec = &g_phy_hw_spec[0];
        (hw_spec != HI_NULL && (i < hdmi_array_size(g_phy_hw_spec)));
        hw_spec++, i++) {
        if ((tmds_clk >= hw_spec->phy_tmds_clk_range.clk_min) && (tmds_clk <= hw_spec->phy_tmds_clk_range.clk_max)) {
            if (phy_spec_cfg->i_de_main_d0 != g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data) {
                phy_spec_cfg->i_de_main_d0  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
                phy_spec_cfg->i_de_main_d1  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
                phy_spec_cfg->i_de_main_d2  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
            }
            phy_spec_cfg->i_de_main_clk = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_clk;
            if (phy_spec_cfg->i_main_d0 != g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data) {
                phy_spec_cfg->i_main_d0  = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
                phy_spec_cfg->i_main_d1  = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
                phy_spec_cfg->i_main_d2  = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
            }
            phy_spec_cfg->i_main_clk = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_clk;
            break;
        }
    }

    return;
}
#endif

#else
static hi_void hal_hdmi_phy_spec_regs_set(const hdmi_phy_hw_spec_cfg *hdmi_spec_cfg)
{
    hi_u32 tmp;

    if ((hdmi_spec_cfg->tmds_clk >= TMDS_CLOCK_25M) && (hdmi_spec_cfg->tmds_clk <= TMDS_CLOCK_100M)) {
        hdmi_reg_write_u32(0x11400068, 0x00861865);
        hdmi_reg_write_u32(0x1140006c, 0x18180000);
        hdmi_reg_write_u32(0x11400070, 0x00000618);
        hdmi_reg_write_u32(0x11400074, 0x0001B6C0);
        hdmi_reg_write_u32(0x11400078, 0x20202000);
        tmp = hdmi_reg_read_u32(0x11400064);
        tmp |= 0xfd << 11; /* 11'b, BIT[19:12] */
        hdmi_reg_write_u32(0x11400064, tmp);
    } else if ((hdmi_spec_cfg->tmds_clk > TMDS_CLOCK_100M) && (hdmi_spec_cfg->tmds_clk <= TMDS_CLOCK_165M)) {
        hdmi_reg_write_u32(0x11400068, 0x00C71C65);
        hdmi_reg_write_u32(0x1140006c, 0x18600000);
        hdmi_reg_write_u32(0x11400070, 0x00000618);
        hdmi_reg_write_u32(0x11400074, 0x0001B780);
        hdmi_reg_write_u32(0x11400078, 0x20202000);
        tmp = hdmi_reg_read_u32(0x11400064);
        tmp |= 0xfd << 11; /* 11'b, BIT[19:12] */
        hdmi_reg_write_u32(0x11400064, tmp);
    } else if ((hdmi_spec_cfg->tmds_clk > TMDS_CLOCK_165M) && (hdmi_spec_cfg->tmds_clk <= TMDS_CLOCK_340M)) {
        hdmi_reg_write_u32(0x11400068, 0x00C71C65);
        hdmi_reg_write_u32(0x1140006c, 0x18600000);
        hdmi_reg_write_u32(0x11400070, 0x00000618);
        hdmi_reg_write_u32(0x11400074, 0x0001B6C0);
        hdmi_reg_write_u32(0x11400078, 0x60606000);
        tmp = hdmi_reg_read_u32(0x11400064);
        tmp |= 0x55 << 11; /* 11'b, BIT[19:12] */
        hdmi_reg_write_u32(0x11400064, tmp);
    }

    return;
}
#endif

static phy_ssc_cfg *phy_ssc_data_get(hi_u32 tmds_clk)
{
    hi_u32 i;
    phy_ssc *ssc_cfg = HI_NULL;

    for (i = 0, ssc_cfg = &g_phy_ssc_cfg[0]; (ssc_cfg && (i < hdmi_array_size(g_phy_ssc_cfg))); ssc_cfg++, i++) {
        if ((tmds_clk >= ssc_cfg->phy_tmds_clk_range.clk_min) && (tmds_clk <= ssc_cfg->phy_tmds_clk_range.clk_max)) {
            return (&ssc_cfg->ssc_cfg);
        }
    }

    return HI_NULL;
}

static hi_void phy_div_mode_set(phy_div_mode div_mode)
{
    hdmi_reg_divsel_set(div_mode);
    return;
}

static hi_void phy_loop_div_set(hi_u32 loop_div)
{
    hdmi_reg_fb_set(loop_div);
    return;
}

static hi_void phy_clk_ratio_set(phy_clk_ratio clk_ratio)
{
    hi_u32 phy_fifo_ck_l = 0;  // 30bit
    hi_u32 phy_fifo_ck_h = 0;  // 10bit

    switch (clk_ratio) {
        case PHY_CLK_RATIO_1_10:
            phy_fifo_ck_l = PHY_RATIO_1_10_MASK;
            phy_fifo_ck_l <<= 10; /* 10'b, BIT[15:11] */
            phy_fifo_ck_l |= PHY_RATIO_1_10_MASK;
            phy_fifo_ck_l <<= 10; /* 10'b, BIT[25:11] */
            phy_fifo_ck_l |= PHY_RATIO_1_10_MASK;
            phy_fifo_ck_h = PHY_RATIO_1_10_MASK;
            break;
        case PHY_CLK_RATIO_1_40:
            phy_fifo_ck_l = PHY_RATIO_1_40_MASK;
            phy_fifo_ck_h = 0;
            break;
        default:
            hdmi_err("clk_ratio %u is invalid\n", clk_ratio);
            return;
    }
    hdmi_reg_aphy_data_clk_height_set(phy_fifo_ck_h);
    hdmi_reg_aphy_data_clk_low_set(phy_fifo_ck_l);

    return;
}

static hi_u32 phy_transfer_hex2cur(hi_u32 reg_hex)
{
    hi_u32 cur;

    if (reg_hex < 0x10) {
        cur = reg_hex;
    } else if ((reg_hex >= 0x10) && (reg_hex < 0x20)) {
        cur = (reg_hex - 0x04);
    } else if ((reg_hex >= 0x20) && (reg_hex < 0x30)) {
        cur = (reg_hex - 0x10);
    } else if ((reg_hex >= 0x30) && (reg_hex < 0x40)) {
        cur = (reg_hex - 0x14);
    } else {
        hdmi_err("get current(%u) error.\n", reg_hex);
        cur = reg_hex;
    }

    return cur;
}

static hi_void phy_cur_hw_param_get(hdmi_hw_param *hw_param)
{
    hi_u32 tmp;

    tmp = hdmi_reg_isel_main_de_clk_get();
    hw_param->i_de_main_clk = phy_transfer_hex2cur(tmp);
    tmp = hdmi_reg_isel_main_de_d0_get();
    hw_param->i_de_main_data = phy_transfer_hex2cur(tmp);
    tmp = hdmi_reg_isel_main_clk_get();
    hw_param->i_main_clk = phy_transfer_hex2cur(tmp);
    tmp = hdmi_reg_isel_main_d0_get();
    hw_param->i_main_data = phy_transfer_hex2cur(tmp);

    tmp = hdmi_reg_rsel_pre_clk_get();
    /* if enable */
    if (tmp & PHY_FEEDTHRU_EN_MASK) {
        tmp &= PHY_FEEDTHRU_DATA_MASK; /* get data */
        tmp++; /* reg val add 1 */
        hw_param->ft_cap_clk = phy_transfer_hex2cur(tmp);
    } else {
        hw_param->ft_cap_clk = 0;
    }

    tmp = hdmi_reg_rsel_pre_d0_get();
    /* if enable */
    if (tmp & PHY_FEEDTHRU_EN_MASK) {
        tmp &= PHY_FEEDTHRU_DATA_MASK; /* get data */
        tmp++; /* reg val add 1 */
        hw_param->ft_cap_data = phy_transfer_hex2cur(tmp);
    } else {
        hw_param->ft_cap_data = 0;
    }

    return;
}

static hi_void phy_default_spec_set(hi_void)
{
    hi_u8 i;
    hdmi_hw_spec *spec = HI_NULL;

    spec = &g_hdmi_phy_info.hw_spec;
    for (i = 0; i < HDMI_HW_PARAM_NUM; i++) {
#ifdef PHY_CRAFT_S28
        spec->hw_param[i].i_de_main_clk  = g_phy_hw_spec_enhance[i].cfg.i_demain_clk;
        spec->hw_param[i].i_de_main_data = g_phy_hw_spec_enhance[i].cfg.i_demain_d0;
        spec->hw_param[i].i_main_clk     = g_phy_hw_spec_enhance[i].cfg.i_main_clk;
        spec->hw_param[i].i_main_data    = g_phy_hw_spec_enhance[i].cfg.i_main_d0;
        spec->hw_param[i].ft_cap_clk     = g_phy_hw_spec_enhance[i].cfg.feedthru_cap_clk_en ?
            (g_phy_hw_spec_enhance[i].cfg.feedthru_cap_delay_clk + 1) : 0;
        spec->hw_param[i].ft_cap_data    = g_phy_hw_spec_enhance[i].cfg.feedthru_cap_d0_en ?
            (g_phy_hw_spec_enhance[i].cfg.feedthru_cap_delay_d0 + 1) : 0;
#else
        spec->hw_param[i].i_de_main_clk  = g_phy_hw_spec[i].phy_spec_cfg.i_de_main_clk;
        spec->hw_param[i].i_de_main_data = g_phy_hw_spec[i].phy_spec_cfg.i_de_main_d0;
        spec->hw_param[i].i_main_clk     = g_phy_hw_spec[i].phy_spec_cfg.i_main_clk;
        spec->hw_param[i].i_main_data    = g_phy_hw_spec[i].phy_spec_cfg.i_main_d0;
#endif
    }

    return;
}

static hi_void hal_hdmi_phy_ssc_on(hi_u32 span_fb, hi_u32 step_fb)
{
    /* step 1 */
    hdmi_reg_step_fb_set(step_fb);
    hdmi_reg_span_fb_set(span_fb);
    /* step 2 */
    hdmi_reg_ssc_mode_fb_set(HI_TRUE);
    /* step 3 */
    hdmi_reg_load_fb_set(HI_FALSE);
    hdmi_reg_load_fb_set(HI_TRUE);
    hdmi_reg_load_fb_set(HI_FALSE);
    /* step 4 */
    hdmi_reg_gc_txpll_test_set(PHY_TXPLL_TEST_ON);
    /* step 5 */
    hdmi_reg_gc_txpll_en_sscdiv_set(HI_TRUE);

    return;
}

static hi_void hal_hdmi_phy_ssc_off(hdmi_deep_color deep_color)
{
    hdmi_reg_step_fb_set(0);
    hdmi_reg_span_fb_set(0);
    /* 12bit need to enter decimal mode */
    if (deep_color == HDMI_DEEP_COLOR_36BIT) {
        /* step 1 */
        hdmi_reg_ssc_mode_fb_set(HI_TRUE);
        /* step 2 */
        hdmi_reg_load_fb_set(HI_FALSE);
        hdmi_reg_load_fb_set(HI_TRUE);
        hdmi_reg_load_fb_set(HI_FALSE);
        /* step 3 */
        hdmi_reg_gc_txpll_test_set(PHY_TXPLL_TEST_ON);
        /* step 4 */
        hdmi_reg_gc_txpll_en_sscdiv_set(HI_TRUE);
    } else {
        hdmi_reg_gc_txpll_en_sscdiv_set(HI_FALSE);
        hdmi_reg_gc_txpll_test_set(PHY_TXPLL_TEST_OFF);
        hdmi_reg_ssc_mode_fb_set(HI_FALSE);
    }

    return;
}

static phy_div_cfg *phy_div_cfg_get(hi_u32 pix_clk, hdmi_deep_color deep_clr)
{
    hi_u32 i;
    phy_div_cfg *div_cfg = HI_NULL;

    for (i = 0, div_cfg = &g_phy_div_cfg[0]; (div_cfg && (i < hdmi_array_size(g_phy_div_cfg))); div_cfg++, i++) {
        if ((pix_clk >= div_cfg->pix_clk_range.clk_min) &&
            (pix_clk <= div_cfg->pix_clk_range.clk_max) &&
            (deep_clr == div_cfg->deep_color)) {
            return div_cfg;
        }
    }
    hdmi_err("can't find param, tmds_clk:%u, deep_clr=%u.\n", pix_clk, deep_clr);

    return &g_phy_div_cfg[1];
}

static hi_void phy_rterm_init(hi_void)
{
    hi_u32 i;
    hi_u32 tmp_regval;
    phy_rterm_mode_data rterm_mode = {0};
    phy_rterm_mode mode_clk = PHY_RTERM_MODE_SINGLE;
    phy_rterm_mode mode_d0  = PHY_RTERM_MODE_SINGLE;
    phy_rterm_mode mode_d1  = PHY_RTERM_MODE_SINGLE;
    phy_rterm_mode mode_d2  = PHY_RTERM_MODE_SINGLE;

    tmp_regval = hdmi_reg_gc_pd_rterm_get();
    rterm_mode.mode_data_clk = tmp_regval & 0x03;
    rterm_mode.mode_data_d0  = tmp_regval & 0x0C;
    rterm_mode.mode_data_d1  = tmp_regval & 0x30;
    rterm_mode.mode_data_d2  = tmp_regval & 0xC0;

    for (i = 0; i < hdmi_array_size(g_phy_rterm_mode_deta); i++) {
        if (rterm_mode.mode_data_clk == g_phy_rterm_mode_deta[i].mode_data_clk) {
            mode_clk = g_phy_rterm_mode_deta[i].r_term_mode;
        }
        if (rterm_mode.mode_data_d0 == g_phy_rterm_mode_deta[i].mode_data_d0) {
            mode_d0 = g_phy_rterm_mode_deta[i].r_term_mode;
        }
        if (rterm_mode.mode_data_d1 == g_phy_rterm_mode_deta[i].mode_data_d1) {
            mode_d1 = g_phy_rterm_mode_deta[i].r_term_mode;
        }
        if (rterm_mode.mode_data_d2 == g_phy_rterm_mode_deta[i].mode_data_d2) {
            mode_d2 = g_phy_rterm_mode_deta[i].r_term_mode;
        }
    }

#ifdef PHY_CRAFT_S28
    g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_clk = mode_clk;
    g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_d0  = mode_d0;
    g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_d1  = mode_d1;
    g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_d2  = mode_d2;
#endif

    hdmi_info("clk:%d, d0:%d, d1:%d, d2:%d, RTerm:0x%x\n",
              mode_clk, mode_d0, mode_d1, mode_d2, tmp_regval);

    return;
}

/* hisense LED32H130, this TV can't resume when RTerm is opend */
static hi_void phy_rterm_set(hi_bool enable)
{
#ifdef PHY_CRAFT_S28
    hi_u32 tmp;

    if (enable == HI_TRUE) {
        tmp = phy_rterm_mode_data_get(g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_clk)->mode_data_clk;
        tmp |= phy_rterm_mode_data_get(g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_d0)->mode_data_d0;
        tmp |= phy_rterm_mode_data_get(g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_d1)->mode_data_d1;
        tmp |= phy_rterm_mode_data_get(g_hdmi_phy_info.spec_enhance_cfg.rterm_mode_d2)->mode_data_d2;
    } else {
        tmp = 0x1FF;
    }
    hdmi_info("RTerm:0x%x\n", tmp);
    hdmi_reg_gc_pd_rterm_set(tmp);

#else
    hi_unused(enable);
#endif

    return;
}


hi_s32 hal_hdmi_phy_init(const hdmi_hal_init *hal_init)
{
    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_null_return(hal_init, HI_FAILURE);

    if (!g_hdmi_phy_info.init) {
        if (hal_init->phy_addr == HI_NULL && hal_init->base_addr != HI_NULL) {
            hdmi_reg_tx_phy_init((hal_init->base_addr + HDMI_TX_BASE_ADDR_DPHY));
        } else if (hal_init->phy_addr != HI_NULL) {
            hdmi_reg_tx_phy_init(hal_init->phy_addr);
        } else {
            hdmi_err("null pointer!\n");
            return HI_FAILURE;
        }

        /* init */
        hdmi_reg_txpll_icp_ictrl_set(PHY_I_CHARGE_PUMP);
        hdmi_reg_txpll_divsel_loop_set(PHY_PLL_LOOP_DEVIDER);
        hdmi_reg_sscin_bypass_en_set(HI_TRUE);
        hdmi_reg_pllfbmash111_en_set(HI_TRUE);
        phy_default_spec_set();
        phy_rterm_init();
        g_hdmi_phy_info.init = HI_TRUE;
    }

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_phy_deinit(hi_void)
{
    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_FAILURE);

    hal_hdmi_phy_reset();
    hdmi_reg_tx_phy_deinit();
    g_hdmi_phy_info.init = HI_FALSE;

    return HI_SUCCESS;
}

hi_void hal_hdmi_phy_reset(hi_void)
{
    hdmi_if_fpga_return_void();
    hdmi_if_false_return_void(g_hdmi_phy_info.init);

    hal_hdmi_phy_oe_set(HI_FALSE);
    hal_hdmi_phy_power_set(HI_FALSE);
    drv_hdmi_prod_crg_phy_reset_set(HI_TRUE);

    return;
}

hi_s32 hal_hdmi_phy_power_get(hi_bool *enable)
{
    phy_power_cfg power_cfg = {0};

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_FAILURE);
    hdmi_if_null_return(enable, HI_FAILURE);

    power_cfg.phy_gc_txpll_pd   = hdmi_reg_gc_txpll_pd_get();
    power_cfg.phy_gc_rxsense_pd = hdmi_reg_gc_pd_rxsense_get();
    power_cfg.phy_gc_ldo_pd     = hdmi_reg_gc_pd_ldo_get();
    power_cfg.phy_gc_de_pd      = hdmi_reg_gc_pd_de_get();
    power_cfg.phy_gc_bist_pd    = hdmi_reg_gc_pd_bist_get();

    if ((power_cfg.phy_gc_txpll_pd == PHY_POWER_ON) && (power_cfg.phy_gc_rxsense_pd == PHY_POWER_ON) &&
        (power_cfg.phy_gc_ldo_pd == PHY_POWER_ON) && power_cfg.phy_gc_bist_pd) {
        *enable = HI_TRUE;
    } else if (power_cfg.phy_gc_txpll_pd && power_cfg.phy_gc_rxsense_pd &&
        (PHY_POWER_LDO_OFF & power_cfg.phy_gc_ldo_pd) && (PHY_POWER_OFF & (hi_u32)power_cfg.phy_gc_bist_pd)) {
        *enable = HI_FALSE;
    } else {
        hdmi_warn("phy power configure is abnormal\n");
        *enable = HI_FALSE;
    }

    return HI_SUCCESS;
}

hi_void hal_hdmi_phy_power_set(hi_bool enable)
{
    hdmi_if_fpga_return_void();
    hdmi_if_false_return_void(g_hdmi_phy_info.init);

    if (enable) {
        hdmi_reg_gc_txpll_pd_set(PHY_POWER_ON);
        hdmi_reg_gc_pd_rxsense_set(PHY_POWER_ON);
        hdmi_reg_gc_pd_ldo_set(PHY_POWER_ON);
    } else {
        hdmi_reg_gc_txpll_pd_set(PHY_POWER_OFF);
        hdmi_reg_gc_pd_rxsense_set(PHY_POWER_OFF);
        hdmi_reg_gc_pd_ldo_set(PHY_POWER_OFF);
    }
    hdmi_reg_gc_pd_bist_set(PHY_POWER_OFF);
    g_hdmi_phy_info.power_enable = enable;

    return;
}

hi_void hal_hdmi_phy_oe_set(hi_bool enable)
{
    hi_u32 oe;
    hi_u32 de_main_power = PHY_POWER_OFF;

    hdmi_if_fpga_return_void();
    hdmi_if_false_return_void(g_hdmi_phy_info.init);
    oe = enable ? PHY_OE_ON : PHY_OE_OFF;
    /* de-emphasis must be power down. */
    if (enable) {
        if (hdmi_reg_isel_main_de_clk_get() != 0) {
            de_main_power &= PHY_POWER_ON_MASK_CLK;
        }
        if (hdmi_reg_isel_main_de_d0_get() != 0) {
            de_main_power &= PHY_POWER_ON_MASK_D0;
        }
        if (hdmi_reg_isel_main_de_d1_get() != 0) {
            de_main_power &= PHY_POWER_ON_MASK_D1;
        }
        if (hdmi_reg_isel_main_de_d2_get() != 0) {
            de_main_power &= PHY_POWER_ON_MASK_D2;
        }
    }
    hdmi_reg_gc_pd_de_set(de_main_power);
    phy_rterm_set(enable);
    hdmi_reg_gc_pd_set(oe);
    g_hdmi_phy_info.oe_enable = enable;

    return;
}

hi_s32 hal_hdmi_phy_oe_get(hi_bool *enable)
{
    hi_bool phy_reset = HI_FALSE;

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_FAILURE);
    hdmi_if_null_return(enable, HI_FAILURE);

    drv_hdmi_prod_crg_phy_reset_get(&phy_reset);
    if (!phy_reset) {
        *enable = (hdmi_reg_gc_pd_get() == PHY_OE_ON) ? HI_TRUE : HI_FALSE;
    } else {
        *enable = HI_FALSE;
    }

    return HI_SUCCESS;
}

/* for video path */
hi_s32 hal_hdmi_phy_tmds_set(hdmi_phy_tmds_cfg *hdmi_tmds_cfg)
{
    errno_t ret;
    phy_div_cfg         *div_cfg = HI_NULL;
    hdmi_deep_color      dp_color;
    hdmi_crg_cfg         crg_cfg = {0};
    hdmi_phy_ssc_cfg     hdmi_ssc_cfg = {0};
    hdmi_phy_hw_spec_cfg hdmi_hw_spec_cfg = {0};

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_FAILURE);
    hdmi_if_null_return(hdmi_tmds_cfg, HI_FAILURE);

    dp_color = hdmi_tmds_cfg->deep_color;
    if (dp_color == HDMI_DEEP_COLOR_OFF) {
        dp_color = HDMI_DEEP_COLOR_24BIT;
    }

    /* get config para */
    div_cfg = phy_div_cfg_get(hdmi_tmds_cfg->pixel_clk, dp_color);
    hdmi_if_null_return(div_cfg, HI_FAILURE);

    hal_hdmi_phy_power_set(HI_TRUE);
    /* set phy div mode */
    phy_div_mode_set(div_cfg->div_mode);
    /* set crg div */
    crg_cfg.ssc_bypass_div = div_cfg->ssc_byp_div;
    crg_cfg.tmds_clk_div = div_cfg->tmds_nxdiv;
    drv_hdmi_prod_crg_div_set(&crg_cfg);
    /* must delay 500us after CRG register config finished. */
    osal_msleep(1);
    /* set clk ratio */
    phy_clk_ratio_set(div_cfg->clk_ratio);
    /* set loopdiv */
    phy_loop_div_set(div_cfg->loop_div);
    /* spread spectrum ctrl(ssc) */
    hdmi_ssc_cfg.pix_clk    = hdmi_tmds_cfg->pixel_clk;
    hdmi_ssc_cfg.tmds_clk   = hdmi_tmds_cfg->tmds_clk;
    hdmi_ssc_cfg.deep_color = dp_color;
    hdmi_ssc_cfg.phy_ssc.ssc_enable   = hdmi_tmds_cfg->emi_enable;
    hdmi_ssc_cfg.phy_ssc.ssc_debug_en = HI_FALSE;
    hal_hdmi_phy_ssc_set(&hdmi_ssc_cfg);

    /* set hard_ware specification */
    hdmi_hw_spec_cfg.tmds_clk = hdmi_tmds_cfg->tmds_clk;
    hdmi_hw_spec_cfg.trace_len = hdmi_tmds_cfg->trace_len;
    hdmi_hw_spec_cfg.hdmi_phy_spec.hw_spec_debug_en = HI_FALSE;
    hal_hdmi_phy_spec_set(&hdmi_hw_spec_cfg);
    /* status info set */
    ret = memcpy_s(&g_hdmi_phy_info.tmds_cfg, sizeof(hdmi_phy_tmds_cfg), hdmi_tmds_cfg, sizeof(hdmi_phy_tmds_cfg));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return HI_SUCCESS;
}

/* spread spectrum ctrl(ssc) */
static hi_s32 hdmi_phy_spec_set(hdmi_phy_hw_spec_cfg *hdmi_spec_cfg)
{
    errno_t ret;

#ifdef PHY_CRAFT_S28
    phy_hwspec_enhance_cfg  enhance_spec = {0};
    phy_hwspec_enhance_cfg *phy_hw_enhance_spec = HI_NULL;

    phy_hw_enhance_spec =
        hdmi_spec_cfg->hdmi_phy_spec.hw_spec_debug_en ? &hdmi_spec_cfg->hdmi_phy_spec.enhance_cfg :
        phy_hw_enhance_spec_data_get(hdmi_spec_cfg->tmds_clk, hdmi_spec_cfg->trace_len);
    hdmi_if_null_return(phy_hw_enhance_spec, HI_FAILURE);
    ret = memcpy_s(&enhance_spec, sizeof(phy_hwspec_enhance_cfg), phy_hw_enhance_spec, sizeof(*phy_hw_enhance_spec));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    phy_user_enhance_param_set(hdmi_spec_cfg->tmds_clk, &enhance_spec);
    ret = phy_hw_enhance_spec_set(&enhance_spec);
#else
    phy_hw_spec_cfg  spec_cfg = {0};
    phy_hw_spec_cfg *phy_hw_spec_cfg_tmp = HI_NULL;

    phy_hw_spec_cfg_tmp =
        hdmi_spec_cfg->hdmi_phy_spec.hw_spec_debug_en ? &hdmi_spec_cfg->hdmi_phy_spec.spec_cfg :
        phy_hw_spec_data_get(hdmi_spec_cfg->tmds_clk, hdmi_spec_cfg->trace_len);
    hdmi_if_null_return(phy_hw_spec_cfg_tmp, HI_FAILURE);
    ret = memcpy_s(&spec_cfg, sizeof(phy_hw_spec_cfg), phy_hw_spec_cfg_tmp, sizeof(*phy_hw_spec_cfg_tmp));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);
    phy_user_param_set(hdmi_spec_cfg->tmds_clk, &spec_cfg);

    ret = phy_hw_spec_set(&spec_cfg);
#endif

    return ret;
}

/* for video path & debug */
hi_s32 hal_hdmi_phy_spec_set(hdmi_phy_hw_spec_cfg *hdmi_spec_cfg)
{
    hi_s32 ret = HI_SUCCESS;

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_null_return(hdmi_spec_cfg, HI_FAILURE);

#ifndef PHY_SPEC_DEBUG
    ret = hdmi_phy_spec_set(hdmi_spec_cfg);
#else
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_FAILURE);
    hdmi_if_null_return(hdmi_spec_cfg, HI_FAILURE);
    hal_hdmi_phy_spec_regs_set(hdmi_spec_cfg);
#endif

    return ret;
}

/* for video path & debug */
hi_void hal_hdmi_phy_ssc_set(hdmi_phy_ssc_cfg *hdmi_ssc_cfg)
{
    errno_t ret;
    hi_u32          phy_ref_clk = 0;
    hi_u32          span_fb, step_fb, tmp;
    hdmi_deep_color dp_color;
    phy_ssc_cfg    *phy_ssc_cfg_tmp = HI_NULL;
    phy_div_cfg    *div_cfg = HI_NULL;

    hdmi_if_fpga_return_void();
    hdmi_if_false_return_void(g_hdmi_phy_info.init);
    hdmi_if_null_return_void(hdmi_ssc_cfg);

    dp_color = hdmi_ssc_cfg->deep_color;
    if (dp_color == HDMI_DEEP_COLOR_OFF) {
        dp_color = HDMI_DEEP_COLOR_24BIT;
    }

    /* get config para */
    div_cfg = phy_div_cfg_get(hdmi_ssc_cfg->pix_clk, dp_color);
    hdmi_if_null_return_void(div_cfg);

    /* spread spectrum ctrl(ssc) */
    if (hdmi_ssc_cfg->phy_ssc.ssc_enable) {
        phy_ssc_cfg_tmp = hdmi_ssc_cfg->phy_ssc.ssc_debug_en ? &hdmi_ssc_cfg->phy_ssc.ssc_cfg :
            phy_ssc_data_get(hdmi_ssc_cfg->tmds_clk);

        hdmi_if_null_return_void(phy_ssc_cfg_tmp);

        if ((div_cfg->ssc_byp_div == 0) || (phy_ssc_cfg_tmp->ssc_freq == 0)) {
            span_fb = 0;
            step_fb = 0;
        } else {
            /*
             *  ref_clk calculation
             *  reference clock = (pixelclk(hz) * pixclkdiv) / ssc_bypass_div
             */
            phy_ref_clk = hdmi_ssc_cfg->pix_clk * div_cfg->pixel_clk_div * HDMI_THOUSAND;
            phy_ref_clk /= div_cfg->ssc_byp_div;
            /*
             * span fb calculation
             * span = ref clock/(ssc_freqence * 4)
             */
            span_fb = (phy_ref_clk / phy_ssc_cfg_tmp->ssc_freq) / 4; /* 4, according to protocol calculation */
            /*
             * step fb calculation
             * step = (((ssc_amptd * loop_div) / 10000) * ssc_freqcence) / (ref clock / 4)
             */
            tmp = (div_cfg->loop_div / HDMI_HUNDRED) * phy_ssc_cfg_tmp->ssc_amptd;
            tmp = tmp * (phy_ssc_cfg_tmp->ssc_freq / HDMI_THOUSAND);
            step_fb = (phy_ref_clk >= 4) ? (tmp / (phy_ref_clk / 4)) : 0; /* 4, (ref clock / 4) is protocol requirements */
        }

        hdmi_info("ssc info || loop_div:0x%x, ref_clk:%u.\n", div_cfg->loop_div, phy_ref_clk);
        hdmi_info("         || amptd:%u, freq:%u, span:0x%x, step:0x%x.\n",
            phy_ssc_cfg_tmp->ssc_amptd, phy_ssc_cfg_tmp->ssc_freq, span_fb, step_fb);

        hal_hdmi_phy_ssc_on(span_fb, step_fb);
    } else {
        hal_hdmi_phy_ssc_off(div_cfg->deep_color);
    }

    /* status info set */
    ret = memcpy_s(&g_hdmi_phy_info.ssc_cfg, sizeof(g_hdmi_phy_info.ssc_cfg),
        &hdmi_ssc_cfg->phy_ssc, sizeof(hdmi_phy_ssc));
    hdmi_unequal_eok_return_void(ret);

    return;
}

hi_s32 hal_hdmi_phy_ssc_get(hi_bool *enable)
{
    hi_u32 span_fb, step_fb;
    hi_bool hw_ssc_enable = HI_FALSE;

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_null_return(enable, HI_FAILURE);

    span_fb       = hdmi_reg_step_fb_get();
    step_fb       = hdmi_reg_span_fb_get();
    hw_ssc_enable = ((span_fb == 0) && (step_fb == 0)) ? HI_FALSE : HI_TRUE;
    *enable       = hw_ssc_enable;

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_phy_info_get(hdmi_phy_info *phy_status)
{
    errno_t ret;
    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_null_return(phy_status, HI_FAILURE);

    ret = memcpy_s(phy_status, sizeof(hdmi_phy_info), &g_hdmi_phy_info, sizeof(hdmi_phy_info));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_phy_params_set(hi_u32 tmds_clk, const hdmi_hw_spec *hw_spec)
{
    errno_t ret;
    hdmi_device *hdmi_dev = HI_NULL;
    hdmi_phy_hw_spec_cfg spec_cfg = {0};

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_ERR_HDMI_INVALID_PARA);
    hdmi_if_null_return(hw_spec, HI_ERR_NULL_PTR);
    hdmi_dev = get_hdmi_device(HDMI_DEVICE_ID0);
    hdmi_if_null_return(hdmi_dev, HI_ERR_NULL_PTR);

    ret = memcpy_s(&g_hdmi_phy_info.hw_spec.hw_param[0], sizeof(g_hdmi_phy_info.hw_spec.hw_param),
        &hw_spec->hw_param[0], sizeof(hdmi_hw_spec));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    spec_cfg.tmds_clk = tmds_clk;
    spec_cfg.hdmi_phy_spec.hw_spec_debug_en = HI_FALSE;
    spec_cfg.trace_len = hdmi_dev->mode_param.trace_len;
    hal_hdmi_phy_spec_set(&spec_cfg);

    return HI_SUCCESS;
}

hi_s32 hal_hdmi_phy_params_get(hdmi_phy_hw_param *hw_param)
{
    errno_t ret;
    hi_u8 i;
    hdmi_hw_spec *spec = HI_NULL;

    hdmi_if_fpga_return(HI_SUCCESS);
    hdmi_if_false_return(g_hdmi_phy_info.init, HI_ERR_NULL_PTR);
    hdmi_if_null_return(hw_param, HI_ERR_NULL_PTR);

    spec = &hw_param->hw_spec_def;
    for (i = 0; i < HDMI_HW_PARAM_NUM; i++) {
#ifndef PHY_CRAFT_S28
        spec->hw_param[i].i_de_main_clk  = g_phy_hw_spec[i].phy_spec_cfg.i_de_main_clk;
        spec->hw_param[i].i_de_main_data = g_phy_hw_spec[i].phy_spec_cfg.i_de_main_d0;
        spec->hw_param[i].i_main_clk     = g_phy_hw_spec[i].phy_spec_cfg.i_main_clk;
        spec->hw_param[i].i_main_data    = g_phy_hw_spec[i].phy_spec_cfg.i_main_d0;
#else
        spec->hw_param[i].i_de_main_clk  = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_clk;
        spec->hw_param[i].i_de_main_data = g_hdmi_phy_info.hw_spec.hw_param[i].i_de_main_data;
        spec->hw_param[i].i_main_clk     = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_clk;
        spec->hw_param[i].i_main_data    = g_hdmi_phy_info.hw_spec.hw_param[i].i_main_data;
        spec->hw_param[i].ft_cap_clk     = g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_clk;
        spec->hw_param[i].ft_cap_data    = g_hdmi_phy_info.hw_spec.hw_param[i].ft_cap_data;
#endif
    }
    ret = memcpy_s(&hw_param->hw_spec_cfg, sizeof(hdmi_hw_spec), &g_hdmi_phy_info.hw_spec, sizeof(hdmi_hw_spec));
    hdmi_unequal_eok_return(ret, HI_ERR_HDMI_INVALID_PARA);

    phy_cur_hw_param_get(&hw_param->hw_param_cur);

    return HI_SUCCESS;
}


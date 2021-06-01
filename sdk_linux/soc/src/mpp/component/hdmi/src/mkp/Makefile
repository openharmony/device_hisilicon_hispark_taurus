# Copyright (C) 2021 HiSilicon (Shanghai) Technologies CO., LIMITED.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

# $(ROOT)/mpp/code/component/hdmi/mkp/Makefile

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(HIARCH), hi3519av100)
    HDMI_CHIP := hi3519av100
    EXTRA_CFLAGS += -DHDMI_PRODUCT_HI3519A
    CTRL_DIR := hal/ctrl/hisiv100
    PHY_DIR  := hal/phy/hisiv200
    export CEC_SUPPORT=n
    export SCDC_SUPPORT=y
else ifeq ($(HIARCH), hi3516cv500)
    HDMI_CHIP := hi3516cv500
    EXTRA_CFLAGS += -DHDMI_PRODUCT_HI3516CV500
    EXTRA_CFLAGS += -DCHIP_TYPE_hi3516cv500
    CTRL_DIR := hal/ctrl/hisiv100
    PHY_DIR  := hal/phy/hisiv100
    export CEC_SUPPORT=n
    export SCDC_SUPPORT=n
else ifeq ($(HIARCH), hi3559av100)
ifeq ($(HI_SUBARCH), hi3569v100)
    export FRL_SUPPORT=n
    EXTRA_CFLAGS += -DCHIP_TYPE_hi3569v100
else
    export FRL_SUPPORT=y
    EXTRA_CFLAGS += -DCHIP_TYPE_hi3559av100
endif
    EXTRA_CFLAGS += -DHDMI_PRODUCT_HI3559A
    HDMI_CHIP := hi3559av100
    CTRL_DIR := hal/ctrl/hisiv200
    PHY_DIR  := hal/phy/hisiv200
    export SCDC_SUPPORT=y
    export HDR_SUPPORT=y
endif
####################################################################
# path
####################################################################
HDMI_VERSION := hdmi

MPP_PATH := $(SDK_PATH)/mpp
CODE_ROOT := $(MPP_PATH)/cbb

INC_PATH := -I$(PWD)/../include\
            -I$(CODE_ROOT)/include\
            -I$(CODE_ROOT)/include/adapt\
            -I$(CODE_ROOT)/base/ext_inc\
            -I$(CODE_ROOT)/base/include/adapt\
            -I$(CODE_ROOT)/vo/include/adapt\
            -I$(CODE_ROOT)/vo/vo_dev/include/adapt\
            -I$(CODE_ROOT)/base/arch/$(HDMI_CHIP)/include\
            -I$(CODE_ROOT)/base/arch/$(HIARCH)/include/$(HI_SUBARCH) \
            -I$(SDK_PATH)/osal/include\
            -I$(MPP_PATH)/cbb/vo/ext_inc\
            -I$(MPP_PATH)/cbb/vo/vo_dev/ext_inc\
            -I$(MPP_PATH)/cbb/sys/ext_inc


CUR_DIR := $(PWD)

####################################################################
# hi3559a hal
####################################################################
EXTRA_CFLAGS += -DHDMI_SUPPORT_LOGIC_HISIV100
HAL := hisiv100
HAL_DIR := hal/ctrl/$(HAL)

PRODUCT_DIR := product/$(HDMI_CHIP)
LINUX_VERSION := osal/hisiv600

INC_PATH += -I$(CUR_DIR)
INC_PATH += -I$(CUR_DIR)/hal
INC_PATH += -I$(CUR_DIR)/proc
INC_PATH += -I$(CUR_DIR)/$(LINUX_VERSION)
INC_PATH += -I$(CUR_DIR)/$(PRODUCT_DIR)
INC_PATH += -I$(CUR_DIR)/$(PRODUCT_DIR)/regs
INC_PATH += -I$(CUR_DIR)/$(CTRL_DIR)
INC_PATH += -I$(CUR_DIR)/$(PHY_DIR)
INC_PATH += -I$(CUR_DIR)/$(PHY_DIR)/regs
INC_PATH += -I$(CUR_DIR)/$(CTRL_DIR)/regs

HDMI_SRC += $(CTRL_DIR)/regs/hdmi_reg_aon.c\
            $(CTRL_DIR)/regs/hdmi_reg_audio_path.c\
            $(CTRL_DIR)/regs/hdmi_reg_ctrl.c\
            $(CTRL_DIR)/regs/hdmi_reg_tx.c\
            $(CTRL_DIR)/regs/hdmi_reg_video_path.c

HDMI_SRC += $(CTRL_DIR)/hdmi_hal_intf.c\
            $(CTRL_DIR)/hdmi_hal_machine.c\
            $(CTRL_DIR)/hdmi_hal_ddc.c\
            $(CTRL_DIR)/hdmi_hal_ncts.c\
            $(CTRL_DIR)/hdmi_hal_ctrl.c

HDMI_SRC += $(PRODUCT_DIR)/regs/hdmi_reg_crg.c\
            $(PRODUCT_DIR)/hdmi_product_define.c\
            $(PHY_DIR)/regs/hdmi_reg_dphy.c\
            $(PHY_DIR)/hdmi_hal_phy.c

HDMI_SRC += drv_hdmi_infoframe.c\
            drv_hdmi_event.c\
            drv_hdmi_edid.c\
            drv_hdmi_intf.c\
            drv_hdmi_intf_k.c\
            drv_hdmi_common.c\
            drv_hdmi_compatibility.c\
            $(LINUX_VERSION)/hdmi_osal.c

ifeq ($(CONFIG_HI_PROC_SHOW_SUPPORT),y)
HDMI_SRC += drv_hdmi_proc.c
endif

ifeq ($(FRL_SUPPORT), y)
EXTRA_CFLAGS += -DHDMI_FRL_SUPPORT
HDMI_SRC += $(CTRL_DIR)/regs/hdmi_reg_frl.c\
            $(CTRL_DIR)/hdmi_hal_frl.c\
            drv_hdmi_frl.c\
            drv_hdmi_dfm.c

# kernel open high precision

ifeq ($(HIARCH), hi3559av100)
#EXTRA_CFLAGS += -DHDMI_SYS_NOHIGH_TIMERS
HDMI_SRC += $(CTRL_DIR)/hdmi_hal_timer.c\
            $(CTRL_DIR)/regs/hdmi_reg_timer.c
endif
endif

ifeq ($(CEC_SUPPORT), y)
# if support CEC, add files
EXTRA_CFLAGS += -DHDMI_CEC_SUPPORT
EXTRA_CFLAGS += -DHDMI_PRIORITY_PLAYBACK
HDMI_SRC += $(CTRL_DIR)/regs/hdmi_reg_cec.c \
            $(CTRL_DIR)/hdmi_hal_cec.c
endif

ifeq ($(SCDC_SUPPORT), y)
EXTRA_CFLAGS += -DHDMI_SCDC_SUPPORT
HDMI_SRC += $(CTRL_DIR)/hdmi_hal_scdc.c
endif

ifeq ($(HDR_SUPPORT), y)
EXTRA_CFLAGS += -DHDMI_HDR_SUPPORT
endif

ifeq ($(OSTYPE), liteos)
EXTRA_CFLAGS += -DHDMI_LITEOS_SUPPORT
else
# if debug support, add drv_hdmi_debug.c
ifeq ($(HIARCH), hi3516cv500)
#hi3516cv500 no debug
else
EXTRA_CFLAGS += -DHDMI_DEBUG_SUPPORT
HDMI_SRC += drv_hdmi_debug.c
endif
endif
# EMI CONFIGS
#EXTRA_CFLAGS += -DHDMI_EMI_SUPPORT
EMI_DIR  := hal/emi
INC_PATH += -I$(CUR_DIR)/$(EMI_DIR)
#HDMI_SRC += $(EMI_DIR)/hdmi_emi.c

INIT_FILE := ../init/$(HIARCH)/$(OSTYPE)/hdmi_init.c

ifeq ($(CONFIG_DRM_HISI_HISMART), y)
INC_PATH += -I$(CUR_DIR)/drm_hal/
include $(CUR_DIR)/drm_hal/Makefile
endif

MOD_NAME := hi_hdmi

HDMI_SRC_O := $(HDMI_SRC)
HDMI_SRC += $(INIT_FILE)

####################################################################
# end hi3559a hal
####################################################################

EXTRA_CFLAGS += -DHDMI_DEFUALT_CALLBACK_SUPPORT
#EXTRA_CFLAGS += -Wno-error=date-time

ifeq ($(FPGA),y)
EXTRA_CFLAGS += -DHDMI_FPGA_SUPPORT
endif

#EXTRA_CFLAGS += $(MPP_CFLAGS)

EXTRA_CFLAGS += -Wall -c -O2 -Wno-date-time
EXTRA_CFLAGS += -I. $(INC_PATH) -lpthread
EXTRA_CFLAGS += -D$(HI_FPGA) -D$(HIARCH)

#export hdmi functional support switch
EXTRA_CFLAGS += -DDVI_SUPPORT
EXTRA_CFLAGS += -DHDMI_LOG_SUPPORT


# release header
INC_FILE := ../include/hi_comm_hdmi.h


define maco_cp_file
    if [ ! -z "$(1)" ];then cp -rf $(1) "$(2)"; fi
endef

#export debug switch
#EXTRA_CFLAGS += -DDEBUG_EDID

EXTRA_CFLAGS += $(MPP_CFLAGS)

EXTRA_CFLAGS += -I$(PWD)
ifeq ($(HIGDB),y)
EXTRA_CFLAGS += -g
endif

ifeq ($(CONFIG_DRIVER_SPACE_TYPE),user_space)
EXTRA_CFLAGS :=$(subst base/,based/,$(EXTRA_CFLAGS))
EXTRA_CFLAGS :=$(subst sys/,sysd/,$(EXTRA_CFLAGS))
INC_PATH :=$(subst base/,based/,$(INC_PATH))
INC_PATH :=$(subst sys/,sysd/,$(INC_PATH))
endif

$(ARCH_LIBNAME)_hdmi-objs := $(HDMI_SRC:%.c=%.o)
obj-m := $(ARCH_LIBNAME)_hdmi.o

$(MOD_NAME)-objs := $(HDMI_SRC_O:%.c=%.o)
obj-m += $(MOD_NAME).o

include $(PWD)/Makefile.$(OSTYPE)

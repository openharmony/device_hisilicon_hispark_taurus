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

# $(ROOT)/mpp/code/component/hifb/Makefile

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

INC_DIR          =$(MPP_PATH)/component/hifb/include

#*******************************************************************************
#********************* Set the source code and include path ********************
MPP_ROOT        := $(SDK_PATH)/mpp
CBB_ROOT        := $(SDK_PATH)/mpp/cbb
COMPONENT_ROOT  := $(SDK_PATH)/mpp/component

# Head file path variable's
CFG_INC += -I$(PWD)/include
CFG_INC += -I$(PWD)/src
CFG_INC += -I$(PWD)/src/init
CFG_INC += -I$(PWD)/drv/$(HIARCH)
CFG_INC += -I$(PWD)/drv/include
ifeq ($(MPP_BUILD), n)
CFG_INC += -I$(REL_INC)
endif
ifeq ($(CONFIG_SYNC_FILE),y)
CFG_INC += -I$(KERNEL_ROOT)/drivers/hi_fence
EXTRA_CFLAGS += -DCONFIG_HIFB_FENCE_SUPPORT
endif

KBUILD_EXTRA_SYMBOLS += $(HI_KBUILD_EXTRA_SYMBOLS)
#***************************************************************************#


#********************* List all the release file *******************************

ifeq ($(HIGDB),y)
EXTRA_CFLAGS += -g
endif

EXTRA_CFLAGS += $(CFG_INC)
EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += $(DRV_CFLAGS)
EXTRA_CFLAGS += -I$(MPP_PATH)/cbb/include
EXTRA_CFLAGS += -I$(MPP_PATH)/cbb/include/adapt
EXTRA_CFLAGS += -I$(CBB_ROOT)/base/arch/$(HIARCH)/include/$(HI_SUBARCH)
EXTRA_CFLAGS += -I$(CBB_ROOT)/base/ext_inc
EXTRA_CFLAGS += -I$(CBB_ROOT)/sys/include
EXTRA_CFLAGS += -I$(CBB_ROOT)/sys/include/adapt
EXTRA_CFLAGS += -I$(CBB_ROOT)/sys/ext_inc
EXTRA_CFLAGS += -I$(CBB_ROOT)/vo/include
EXTRA_CFLAGS += -I$(CBB_ROOT)/vo/vo_dev/include
EXTRA_CFLAGS += -I$(MPP_PATH)/component/tde/include
EXTRA_CFLAGS += -I$(MPP_PATH)/component/tde/driver/include

HIFB_MOD_NAME := hifb
MOD_NAME := hi_fb

EXTRA_CFLAGS += -Wall -D$(HIARCH) -D$(HI_FPGA) -Wno-date-time

ifeq ($(CONFIG_DRIVER_SPACE_TYPE),user_space)
EXTRA_CFLAGS :=$(subst base/,based/,$(EXTRA_CFLAGS))
EXTRA_CFLAGS :=$(subst sys/,sysd/,$(EXTRA_CFLAGS))
endif

-include $(PWD)/src/Makefile
-include $(PWD)/drv/$(ARCH_DIR)/Makefile

ifeq ($(CONFIG_DRM_HISI_HISMART), y)
EXTRA_CFLAGS += -I$(PWD)/drm_hal/
-include $(PWD)/drm_hal/Makefile
endif
$(HIFB_MOD_NAME)-objs = $(HIFB_SRC:%.c=%.o)

ifneq ($(KERNELRELEASE),)
    obj-m += hifb.o
endif

# compile linux or huawei_liteos
include $(PWD)/Make.$(OSTYPE)

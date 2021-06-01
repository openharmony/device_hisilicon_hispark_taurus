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

#*************************************************************************
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=../../Makefile.param
    include $(CBB_PARAM_FILE)
endif
#*************************************************************************
# local header
MKP_CFLAGS+=-I $(PWD)/mkp/include
MKP_CFLAGS+=-I $(PWD)/include/adapt
MKP_CFLAGS+=-I $(PWD)/arch/include
MKP_CFLAGS+=-I $(PWD)/arch/$(HIARCH)/include
ifeq ($(CONFIG_DRIVER_SPACE_TYPE),user_space)
MKP_CFLAGS :=$(subst base/,based/,$(MKP_CFLAGS))
MKP_CFLAGS :=$(subst sys/,sysd/,$(MKP_CFLAGS))
endif
#*************************************************************************
# all source file in this module
SRCS :=

# src relative path
CBB_VO_ROOT=.
CBB_VO_ARCH=arch/$(HIARCH)

# Makefile absolute path
include $(CBB_ROOT)/vo/vo_dev/mkp/src/Makefile
include $(CBB_ROOT)/vo/vo_dev/arch/$(HIARCH)/hal/Makefile
ifeq ($(CONFIG_DRM_HISI_HISMART), y)
MKP_CFLAGS+=-I $(CBB_ROOT)/vo/vo_dev/drm_hal
include $(CBB_ROOT)/vo/vo_dev/drm_hal/Makefile
endif

INIT_FILE := ../../init/$(HIARCH)/$(OSTYPE)/vo_dev_init.c

SRCS_O := $(SRCS)
SRCS += $(INIT_FILE)

#*************************************************************************
# release header
INC_FILE := $(wildcard include/*.h)

MOD_NAME := hi_vo_dev

TARGET   := $(ARCH_LIBNAME)_vo_dev
#*************************************************************************
# compile linux or huawei_liteos
include $(PWD)/../../Makefile.$(OSTYPE)


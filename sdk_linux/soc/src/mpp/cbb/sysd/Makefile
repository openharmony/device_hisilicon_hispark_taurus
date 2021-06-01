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
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=../Makefile.param
    include $(CBB_PARAM_FILE)
endif
#*************************************************************************
# local header
MKP_CFLAGS+=-I $(PWD)/mkp/include
MKP_CFLAGS+=-I $(PWD)/arch/include
MKP_CFLAGS+=-I $(PWD)/arch/$(HIARCH)/include
MKP_CFLAGS+=-I $(SDK_PATH)/interdrv/$(HIARCH)

MKP_CFLAGS :=$(subst base/,based/,$(MKP_CFLAGS))
MKP_CFLAGS :=$(subst sys/,sysd/,$(MKP_CFLAGS))
#*************************************************************************
# all source file in this module
#SRCS := mkp/src/sys_ext.c \
#        mkp/src/sys_bind.c
SRCS:=

# src relative path
CBB_SYS_ROOT=./
CBB_SYS_ARCH=arch/$(HIARCH)/

# Makefile absolute path
ifeq ($(CONFIG_DRIVER_SPACE_TYPE),user_space)
include $(CBB_ROOT)/sysd/mkp/src/Makefile
include $(CBB_ROOT)/sysd/arch/$(HIARCH)/hal/Makefile
else
include $(CBB_ROOT)/sys/mkp/src/Makefile
include $(CBB_ROOT)/sys/arch/$(HIARCH)/hal/Makefile
endif

INIT_FILE := ../init/$(HIARCH)/$(OSTYPE)/sys_init.c

SRCS_O := $(SRCS)
SRCS += $(INIT_FILE)

#*************************************************************************
# release header
INC_FILE := $(wildcard include/*.h)

MOD_NAME := hi_sys

TARGET   := $(ARCH_LIBNAME)_sys
#*************************************************************************
# compile linux or huawei_liteos
include $(PWD)/../Makefile.$(OSTYPE)

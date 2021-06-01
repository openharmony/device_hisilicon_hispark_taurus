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

ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

TARGET := hi_proc
#HIARCH
#*************************************************************************
# all source file in this module
SRCS := hiproc.c
#*************************************************************************
EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += $(MKP_CFLAGS)
EXTRA_CFLAGS += -I$(PWD)/../include/
EXTRA_CFLAGS += -I$(REL_INC)
EXTRA_CFLAGS += -I$(OSAL_ROOT)/include
EXTRA_CFLAGS += -I./arch/$(HIARCH)
EXTRA_CFLAGS += -I$(CBB_ROOT)/include
ifeq ($(OSTYPE), liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

.PHONY: all clean
all: $(OSTYPE)_build
clean: $(OSTYPE)_clean

#############################################
$(TARGET)-objs = $(SRCS:%.c=%.o)
ifneq ($(KERNELRELEASE),)
    obj-m += $(TARGET).o
endif

linux_build:
	@echo -e "\e[0;32;1m--Compiling '$(TARGET)'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), DBG=$(HIDBG), HI_FPGA=$(HI_FPGA)
	@echo ---- CPU_TYPE=$(CPU_TYPE)
	@echo ---- MPP_CFLAGS=$(MPP_CFLAGS)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo ---- ARCH_ROOT=$(ARCH_ROOT), ARCH_HAL=$(ARCH_HAL)
	@@echo -e "\e[0m"
#	@mkdir -p $(REL_INC) && cp -f ../include/hiproc.h $(REL_INC)/
	@$(MAKE) -C $(KERNEL_ROOT) M=$(PWD) $(COMPILER_TRIPLE) modules
	@mkdir -p $(REL_KO) && cp $(TARGET).ko $(REL_KO)
#	@$(MAKE) -C test

linux_clean:
	@rm -f *.o *.ko *.mod.c
	@rm -f *.symvers *.order
	@rm -rf .*.ko.cmd .*.o.cmd .tmp_versions
	@rm -f $(REL_KO)/$(TARGET).ko
#	@rm -f $(REL_INC)/hiproc.h
#	@$(MAKE) -C test clean
#============================================

#############################################
OBJS = $(SRCS:%.c=%.o)
LIB := lib$(TARGET).a

liteos_build: $(OBJS) $(LIB)
	@echo -e "\e[0;32;1m--Compiling '$(TARGET)'...\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), DBG=$(HIDBG), HI_FPGA=$(HI_FPGA)
	@echo ---- CPU_TYPE=$(CPU_TYPE)
	@echo ---- MPP_CFLAGS=$(MPP_CFLAGS)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@@echo -e "\e[0m"
	@mkdir -p $(REL_KO) && cp -rf $(LIB) $(REL_KO)/
	@mkdir -p $(REL_INC) && cp -f hiproc.h $(REL_INC)/

liteos_clean:
	@rm -f $(OBJS) $(LIB)
	@rm -f $(REL_KO)/$(LIB)
	@rm -f $(REL_INC)/hiproc.h

$(OBJS): %.o : %.c
	@$(CC) $(EXTRA_CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)
	#============================================

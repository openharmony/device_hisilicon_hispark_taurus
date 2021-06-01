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

CIPHER_DRV_OBJS += drv/cipher_initdevice.o

OBJS := $(CIPHER_API_OBJS)

TARGET := hi_cipher
obj-m := hi_cipher.o
hi_cipher-y += $(CIPHER_DRV_OBJS) $(MBEDTLS_LIB)

#*************************************************************************
.PHONY: all clean build_env mbedtls_prepare mbedtls_clean

all: build_env
	@cp $(MBEDTLS_DIR)/lib.a $(MBEDTLS_LIB)
	@make -C $(KERNEL_ROOT) M=$(PWD) $(COMPILER_TRIPLE) modules
	@mkdir -p $(REL_KO) && cp -rf $(TARGET).ko $(REL_KO)

clean :
	@rm -f *.asm
	@rm -f $(OBJS)
	@rm -f $(CIPHER_REL_INC)
	@rm -f $(REL_KO)/$(TARGET).ko
	@make -C $(KERNEL_ROOT) M=$(PWD) $(COMPILER_TRIPLE) clean

mbedtls_prepare:
	@cd $(MBEDTLS_DIR) && make mbedtls_patch && make

mbedtls_clean:
	@cd $(MBEDTLS_DIR) && make clean

build_env:
	@echo -e "\e[0;32;1m... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), DBG=$(HIDBG), HI_FPGA=$(HI_FPGA)
	@echo ---- CPU_TYPE=$(CPU_TYPE)
	@echo ---- MPP_CFLAGS=$(MPP_CFLAGS)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo ---- ARCH_ROOT=$(ARCH_ROOT), ARCH_HAL=$(ARCH_HAL)
	@echo -e "\e[0m"


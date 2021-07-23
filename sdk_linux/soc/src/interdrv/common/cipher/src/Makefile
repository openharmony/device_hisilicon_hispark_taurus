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

ifeq ($(KERNELRELEASE),)
export CIPHER_BASE_DIR=$(PWD)
endif

include $(CIPHER_BASE_DIR)/drv/cipher_v1.0/build.mak

MBEDTLS_DIR := $(CIPHER_BASE_DIR)/../mbedtls_prepare
MBEDTLS_LIB := libmbedcrypto.a
MBEDTLS_VER := mbedtls-2.16.10

MPP_CFLAGS += $(CIPHER_API_CFLAGS)
MPP_CFLAGS += $(CIPHER_DRV_CFLAGS)
MPP_CFLAGS += -I$(CIPHER_BASE_DIR)/../include
MPP_CFLAGS += -I$(CBB_ROOT)/include
MPP_CFLAGS += -I$(CBB_ROOT)/base/arch/$(HIARCH)/include/$(HI_SUBARCH)

EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += $(DRV_CFLAGS)
EXTRA_CFLAGS += -I$(MBEDTLS_DIR)/$(MBEDTLS_VER)/include/
EXTRA_CFLAGS += -DARCH_TYPE_$(ARM_ARCH)
ifneq ($(filter $(HI_SUBARCH), hi3569v100 hi3568v100), )
EXTRA_CFLAGS += -DCHIP_TYPE_$(HI_SUBARCH)
else
EXTRA_CFLAGS += -DCHIP_TYPE_$(INTER_DRV)
endif
ifeq ($(CONFIG_DRIVER_SPACE_TYPE),user_space)
EXTRA_CFLAGS :=$(subst base/,based/,$(EXTRA_CFLAGS))
EXTRA_CFLAGS :=$(subst sys/,sysd/,$(EXTRA_CFLAGS))
MPP_CFLAGS :=$(subst base/,based/,$(MPP_CFLAGS))
MPP_CFLAGS :=$(subst sys/,sysd/,$(MPP_CFLAGS))
endif
CIPHER_INC     := hi_types.h hi_common_cipher.h hi_unf_cipher.h
CIPHER_DIR_INC := $(patsubst %.h,$(CIPHER_BASE_DIR)/../include/%.h,$(CIPHER_INC))
CIPHER_REL_INC := $(patsubst %.h,$(REL_INC)/%.h,$(CIPHER_INC))

include $(CIPHER_BASE_DIR)/$(OSTYPE).mak

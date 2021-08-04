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
    PARAM_FILE := ../../../../mpp/Makefile.param
    include $(PARAM_FILE)
endif

OBJS_CRYPTO=    aes.o       aesni.o     arc4.o      \
        aria.o      asn1parse.o asn1write.o \
        base64.o    bignum.o    blowfish.o  \
        camellia.o  ccm.o       chacha20.o  \
        chachapoly.o    cipher.o    cipher_wrap.o   \
        cmac.o      ctr_drbg.o  des.o       \
        dhm.o       ecdh.o      ecdsa.o     \
        ecjpake.o   ecp.o               \
        ecp_curves.o    entropy.o   entropy_poll.o  \
        error.o     gcm.o       havege.o    \
        hkdf.o                      \
        hmac_drbg.o md.o        md2.o       \
        md4.o       md5.o       md_wrap.o   \
        memory_buffer_alloc.o       nist_kw.o   \
        oid.o       padlock.o   pem.o       \
        pk.o        pk_wrap.o   pkcs12.o    \
        pkcs5.o     pkparse.o   pkwrite.o   \
        platform.o  platform_util.o poly1305.o  \
        ripemd160.o rsa_internal.o  rsa.o       \
        sha1.o      sha256.o    sha512.o    \
        threading.o timing.o    version.o   \
        version_features.o      xtea.o

MBEDTLS_VER := mbedtls-2.16.10
MBEDTLS_DIR := $(PWD)/$(MBEDTLS_VER)
MBEDTLS_SRC := $(OHOS_ROOT_PATH)/third_party/mbedtls
CIPHER_DIR  := $(SDK_PATH)/interdrv/common/cipher

EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += $(DRV_CFLAGS)
EXTRA_CFLAGS += -I$(MBEDTLS_DIR)/include
EXTRA_CFLAGS += -I$(CIPHER_DIR)/include
EXTRA_CFLAGS += -I$(CIPHER_DIR)/src/api/cipher_v1.0
EXTRA_CFLAGS += -I$(CIPHER_DIR)/src/drv/cipher_v1.0/drivers/core/include
EXTRA_CFLAGS += -I$(CIPHER_DIR)/src/drv/cipher_v1.0/drivers/crypto/include
EXTRA_CFLAGS += -I$(CIPHER_DIR)/src/drv/cipher_v1.0/drivers/extend/ext_include
EXTRA_CFLAGS += -I$(CIPHER_DIR)/src/drv/cipher_v1.0/osal/include
EXTRA_CFLAGS += -I$(CIPHER_DIR)/src/drv/cipher_v1.0/compat

ifneq ($(filter $(HI_SUBARCH), hi3569v100 hi3568v100), )
EXTRA_CFLAGS += -DCHIP_TYPE_$(HI_SUBARCH)
else
EXTRA_CFLAGS += -DCHIP_TYPE_$(INTER_DRV)
endif

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

OBJS := $(patsubst %.o,$(MBEDTLS_VER)/library/%.o,$(OBJS_CRYPTO))

include $(PWD)/$(OSTYPE).mak

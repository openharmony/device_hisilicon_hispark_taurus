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

CIPHER_DRV_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/cipher_v1.0/drivers/core/include
CIPHER_DRV_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/cipher_v1.0/drivers/crypto/include
CIPHER_DRV_CFLAGS += -I$(CIPHER_BASE_DIR)/drv/cipher_v1.0/drivers/extend/include

CIPHER_DRV_OBJS +=  drv/cipher_v1.0/drivers/core/drv_symc_v100.o          \
                    drv/cipher_v1.0/drivers/core/drv_symc_v200.o          \
                    drv/cipher_v1.0/drivers/core/drv_hash_v100.o          \
                    drv/cipher_v1.0/drivers/core/drv_hash_v200.o          \
                    drv/cipher_v1.0/drivers/core/drv_ifep_rsa_v100.o      \
                    drv/cipher_v1.0/drivers/core/drv_trng_v100.o          \
                    drv/cipher_v1.0/drivers/core/drv_trng_v200.o          \
                    drv/cipher_v1.0/drivers/core/drv_lib.o

CIPHER_DRV_OBJS += drv/cipher_v1.0/drivers/crypto/cryp_symc.o             \
                   drv/cipher_v1.0/drivers/crypto/cryp_hash.o             \
                   drv/cipher_v1.0/drivers/crypto/cryp_trng.o             \
                   drv/cipher_v1.0/drivers/crypto/cryp_rsa.o

CIPHER_DRV_OBJS += drv/cipher_v1.0/drivers/kapi_symc.o                    \
                   drv/cipher_v1.0/drivers/kapi_hash.o                    \
                   drv/cipher_v1.0/drivers/kapi_rsa.o                     \
                   drv/cipher_v1.0/drivers/kapi_trng.o                    \
                   drv/cipher_v1.0/drivers/kapi_dispatch.o

CIPHER_DRV_OBJS += drv/cipher_v1.0/drivers/extend/ext_aead.o              \
                   drv/cipher_v1.0/drivers/extend/ext_hash.o              \
                   drv/cipher_v1.0/drivers/extend/ext_symc.o              \
                   drv/cipher_v1.0/drivers/extend/ext_sm3.o               \
                   drv/cipher_v1.0/drivers/extend/ext_sm4.o

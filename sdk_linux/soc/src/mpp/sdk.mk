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

# ohos makefile to build sdk
SDK_LINUX_SRC_PATH := $(OHOS_ROOT_PATH)/device/hisilicon/hispark_taurus/sdk_linux
SDK_LINUX_TMP_PATH := $(OUT_DIR)/sdk_linux/src_tmp
SDK_LINUX_MPP_PATH := $(SDK_LINUX_TMP_PATH)/src/mpp

.PHONY: sdk_linux sdk_linux_lite src_cpy
default: sdk_linux

src_cpy:
	@echo "buiding sdk..."
	@rm -rdf $(SDK_LINUX_TMP_PATH); mkdir -p $(SDK_LINUX_TMP_PATH)
	@cp -rf $(SDK_LINUX_SRC_PATH)/soc/src $(SDK_LINUX_TMP_PATH)
	@cp -rf $(SDK_LINUX_SRC_PATH)/soc/ko $(SDK_LINUX_TMP_PATH)
sdk_linux_lite: src_cpy
	@cd $(SDK_LINUX_MPP_PATH) && make clean OHOS_LITE=y && make -j OHOS_LITE=y

sdk_linux: src_cpy
	@cd $(SDK_LINUX_MPP_PATH) && make clean OHOS_LITE=n && make -j OHOS_LITE=n


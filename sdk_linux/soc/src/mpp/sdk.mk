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
SDK_LINUX_OUT_PATH := $(SDK_LINUX_TMP_PATH)/ko
SDK_LINUX_PAC_PATH := $(PRODUCT_OUT)/vendor/modules

MODULE_BASE_NAME   := hi3516cv500_base.ko
MODULE_HDMI_NAME   := hi3516cv500_hdmi.ko
MODULE_IR_NAME     := hi3516cv500_ir.ko
MODULE_SYS_NAME    := hi3516cv500_sys.ko
MODULE_TED_NAME    := hi3516cv500_tde.ko
MODULE_VODEV_NAME  := hi3516cv500_vo_dev.ko
MODULE_WDT_NAME    := hi3516cv500_wdt.ko
MODULE_CIPHER_NAME := hi_cipher.ko
MODULE_HIFB_NAME   := hifb.ko
MODULE_IRQ_NAME    := hi_irq.ko
MODULE_MIPIRX_NAME := hi_mipi_rx.ko
MODULE_MIPITX_NAME := hi_mipi_tx.ko
MODULE_OSAL_NAME   := hi_osal.ko
MODULE_PROC_NAME   := hi_proc.ko
MODULE_SYSCFG_NAME := sys_config.ko

$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME):$(KERNEL_IMAGE_FILE)
	$(hide) echo "buiding sdk..."
	$(hide) rm -rdf $(SDK_LINUX_TMP_PATH); mkdir -p $(SDK_LINUX_TMP_PATH)
	$(hide) cp -rf $(SDK_LINUX_SRC_PATH)/soc/src $(SDK_LINUX_TMP_PATH)
	$(hide) cp -rf $(SDK_LINUX_SRC_PATH)/soc/ko $(SDK_LINUX_TMP_PATH)
	$(hide) cd $(SDK_LINUX_MPP_PATH) && make clean && make -j

$(SDK_LINUX_OUT_PATH)/$(MODULE_HDMI_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_IR_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_SYS_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_TED_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_VODEV_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_WDT_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_CIPHER_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_HIFB_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_IRQ_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_MIPIRX_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_MIPITX_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_OSAL_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_PROC_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)
$(SDK_LINUX_OUT_PATH)/$(MODULE_SYSCFG_NAME):$(SDK_LINUX_OUT_PATH)/$(MODULE_BASE_NAME)


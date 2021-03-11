##########################################################################################
#	prepare param
##########################################################################################
export OSDRV_DIR=$(shell pwd)
export OSDRV_CROSS
export CHIP?=hi3516dv300
export ARCH=arm
export CROSS_COMPILE=$(OSDRV_CROSS)-
BOOT_MEDIA?=emmc

ifneq ($(BOOT_MEDIA),spi)
ifneq ($(BOOT_MEDIA),emmc)
$(error you must set valid BOOT_MEDIA:spi or emmc!)
endif
endif

ifeq ($(OSDRV_CROSS), )
OSDRV_CROSS=$(OSDRV_DIR)/../../../../../prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-7-2017-q4-major/bin/arm-none-eabi
endif

UBOOT_VER:=u-boot-2020.01
UBOOT_TAR:=$(UBOOT_VER).tgz
UBOOT:=u-boot-$(CHIP).bin
UBOOT_REG_BIN:=reg_info_$(CHIP).bin
ifeq ($(BOOT_MEDIA),emmc)
export UBOOT_CONFIG:=$(CHIP)_$(BOOT_MEDIA)_smp_config
else
export UBOOT_CONFIG:=$(CHIP)_smp_config
endif

NORMAL_UBOOT_DIR:=$(OSDRV_DIR)/../../../third_party/uboot

##########################################################################################
#	set task
##########################################################################################
all: prepare hiboot hipctools build_done

clean: hiboot_clean hipctools_clean

distclean: clean pub_clean clean_done

##########################################################################################
##task [0] prepare out directory
##########################################################################################
prepare:
	@echo "------- task [0] prepare out dir -------"
	mkdir $(OSDRV_DIR)/out/boot -p

##########################################################################################
#task [1]	build uboot
##########################################################################################
hiboot:
	@echo "---------task [1]	build boot"
	find $(NORMAL_UBOOT_DIR)/$(UBOOT_VER) | xargs touch
	make -C $(NORMAL_UBOOT_DIR)/$(UBOOT_VER) ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- $(UBOOT_CONFIG)
	pushd $(NORMAL_UBOOT_DIR)/$(UBOOT_VER);make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- -j 20 >/dev/null;popd
	chmod +x $(NORMAL_UBOOT_DIR)/$(UBOOT_VER)/arch/arm/cpu/armv7/$(CHIP)/hw_compressed/gzip
	cp $(OSDRV_DIR)/reg/$(UBOOT_REG_BIN) $(NORMAL_UBOOT_DIR)/$(UBOOT_VER)/.reg
	pushd $(NORMAL_UBOOT_DIR)/$(UBOOT_VER);make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- u-boot-z.bin >/dev/null;popd
	cp $(NORMAL_UBOOT_DIR)/$(UBOOT_VER)/u-boot-$(CHIP).bin $(OSDRV_DIR)/out/boot
	cp $(NORMAL_UBOOT_DIR)/$(UBOOT_VER)/u-boot-$(CHIP).bin  $(OSDRV_DIR)/secureboot_release

hiboot_clean:
	@cd $(NORMAL_UBOOT_DIR)/$(UBOOT_VER)/;make distclean
	@echo "---------boot clean"

##########################################################################################
##task [2] make hipctools
##########################################################################################
hipctools:prepare
	@echo "---------task [2] build tools run on pc"

hipctools_clean:

##########################################################################################
#task [4]	clean out
##########################################################################################
pub_clean:
	rm $(OSDRV_DIR)/out/* -rf

build_done:
	@echo "------- osdrv build done -------"

clean_done:
	@echo "------- clean done -------"


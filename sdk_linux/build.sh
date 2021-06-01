#!/bin/bash
# Copyright (C) 2021 Hisilicon (Shanghai) Technologies Co., Ltd. All rights reserved.

set -e

SDK_ROOT_PATH=${OHOS_ROOT_PATH}/device/hisilicon/hispark_taurus/sdk_linux

pushd ${SDK_ROOT_PATH}/soc/src/mpp && make clean && make -j && popd

# binaries substitution
pushd ${OHOS_ROOT_PATH}/device/hisilicon/hi3516dv300/sdk_linux/mpp/
    rm -f ko/*.ko
    cp ko/extdrv/hi3516cv500_drm.ko ko/
    cp -r ${SDK_ROOT_PATH}/soc/ko/* ko/
popd

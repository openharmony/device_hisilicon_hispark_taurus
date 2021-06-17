#!/bin/bash
# Copyright (C) 2021 Hisilicon (Shanghai) Technologies Co., Ltd. All rights reserved.

set -e
OHOS_ROOT_PATH=$1
OUTDIR=$2
OHOS_LITE=$3

if [ -z "${OHOS_ROOT_PATH}" ];then
    OHOS_ROOT_PATH=$(pwd)/../../../..
else
    echo "OHOS_ROOT_PATH=${OHOS_ROOT_PATH}"
fi

export OHOS_ROOT_PATH

SDK_LINUX_SRC_PATH=${OHOS_ROOT_PATH}/device/hisilicon/hispark_taurus/sdk_linux
SDK_LINUX_TMP_PATH=${OUTDIR}/sdk_linux/src_tmp
SDK_LINUX_MPP_PATH=${SDK_LINUX_TMP_PATH}/src/mpp

rm -rdf ${SDK_LINUX_TMP_PATH}; mkdir -p ${SDK_LINUX_TMP_PATH}
cp -rf ${SDK_LINUX_SRC_PATH}/soc/src ${SDK_LINUX_TMP_PATH}
cp -rf ${SDK_LINUX_SRC_PATH}/soc/ko ${SDK_LINUX_TMP_PATH}

if [ "${OHOS_LITE}" = "y" ];then
    pushd ${SDK_LINUX_MPP_PATH} && make clean OHOS_LITE=y && make -j OHOS_LITE=y && popd
else
    pushd ${SDK_LINUX_MPP_PATH} && make clean OHOS_LITE=n && make -j OHOS_LITE=n && popd
fi

# HiSpark\_taurus介绍<a name="ZH-CN_TOPIC_0000001130056937"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [约束](#section119744591305)
-   [编译构建](#section137768191623)
-   [hispark\_taurus协议说明](#section1312121216216)
    -   [third\_party许可说明](#section129654513264)

-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

上海海思媒体软件开发包用于适配不同芯片复杂的底层处理，为“媒体子系统”提供基础的多媒体处理功能。主要功能有：音视频采集、音视频编解码、音视频输出、视频前处理、封装、解封装、文件管理、存储管理、日志系统等。该软件包功能对应"媒体子系统"框架中红色框标注部分，如图1所示。

**图 1**  多媒体子系统架构图<a name="fig4460722185514"></a>  


![](figures/zh-cn_image_0000001084185956.png)

## 目录<a name="section161941989596"></a>

```
/device/hisilicon/hispark_taurus/sdk_liteos
├── config                 # Hi3516DV300设备配置信息
├── mpp
│   ├── lib               # Hi3516DV300芯片的媒体库文件、LICENSE文件
│   └── module_init       # Hi3516DV300芯片媒体各模块驱动对应的库、LICENSE文件
└── uboot
    ├── out                # 采用third_party\uboot\u-boot-2020.01编译成的uboot
    ├── reg                # uboot配置文件、LICENSE文件
    ├── secureboot_ohos    # 安全启动相关的编译脚本
    └── secureboot_release # 生成安全uboot的源代码、License目录
```

## 约束<a name="section119744591305"></a>

当前支持Hi3516DV300芯片。

## 编译构建<a name="section137768191623"></a>

-   编译uboot

1.  从开源社区\(https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads\)下载GCC工具链，当前用于编译uboot的工具链版本为gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2，也可以选择其他的GCC版本。
2.  将GCC工具链拷贝到prebuilts目录，并解压。
3.  进入hispark\_taurus\\uboot\\out\\boot目录，修改该目录下的Makefile中的OSDRV\_CROSS所定义的工具链的路径。
4.  编译uboot，命令：make clean;make all;

生成的uboot存放在hispark\_taurus\\uboot\\out\\boot目录下。

## hispark\_taurus协议说明<a name="section1312121216216"></a>

-   hispark\_taurus\\sdk\_liteos\\mpp\\module\_init\\lib和hispark\_taurus\\sdk\_liteos\\mpp\\lib里面为上海海思的自研库，遵循上海海思的LICENSE，这两个目录下均有LICENSE文件，LICENSE文件结尾可以看到版权信息：

    ```
    Copyright (C) 2020 Hisilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
    ```

-   hispark\_taurus\\sdk\_liteos\\mpp\\module\_init\\src目录下为上海海思自研代码，使用基于Apache License Version 2.0许可的Hisilicon \(Shanghai\) 版权声明，在该目录下有Apache License Version 2.0的LICENSE文件，许可信息和版权信息通常可以在文件开头看到：

    ```
     / *Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED. Licensed under the Apache License,* ... * / 
    ```

-   hispark\_taurus\\sdk\_liteos\\uboot\\reg为上海海思的二进制文件，遵循上海海思的LICENSE，该目录下有LICENSE文件，LICENSE文件结尾可以看到：

    ```
    Copyright (C) 2020 Hisilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
    ```

-   hispark\_taurus\\sdk\_liteos\\uboot\\out\\boot是由u-boot-2020.01和reg\_info\_hi3518ev300.bin编译成的uboot二进制文件，完全遵从u-boot-2020.01的整体协议，具体请参看third\_party\\uboot\\u-boot-2020.01\\Licenses目录下的README。
-   hispark\_aries\\sdk\_liteos\\uboot\\secureboot\_release为安全uboot的开源代码，其中自研的部分使用基于GPL许可的Hisilicon \(Shanghai\) 版权声明，在该目录下有License目录，许可信息和版权信息通常可以在文件开头看到：

    ```
     / *Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED. 
       *
       * This program is free software; you can redistribute  it and/or modify it
       * under  the terms of  the GNU General  Public License as published by the
       * Free Software Foundation;  either version 2 of the  License, or (at your
       * option) any later version.
       * ... * / 
    ```

-   hispark\_taurus\\NOTICE文件描述了使用到的三款开源软件：Das U-Boot 2020.01、mbed TLS 2.16.6、fdk-aac v2.0.1。

### third\_party许可说明<a name="section129654513264"></a>

third\_party\\ffmpeg\\ffmpeg-y为ffmpeg开源代码，遵循软件版本自带的开源许可声明，具体请参看third\_party\\ffmpeg\\ffmpeg-y目录下的README。

third\_party\\uboot\\u-boot-2020.01为uboot开源代码，遵循软件版本自带的开源许可声明，具体请参看third\_party\\uboot\\u-boot-2020.01\\Licenses目录下的README。

## 相关仓<a name="section1371113476307"></a>

hmf/device/hisilicon/build

hmf/device/hisilicon/drivers

hmf/device/hisilicon/hardware

**hmf/device/hisilicon/hispark\_taurus**

hmf/device/hisilicon/modules

hmf/device/hisilicon/third\_party/ffmpeg

hmf/device/hisilicon/third\_party/uboot


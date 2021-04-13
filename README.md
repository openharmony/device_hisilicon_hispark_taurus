# hispark\_taurus<a name="EN-US_TOPIC_0000001130056937"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Constraints](#section119744591305)
-   [Compilation and Building](#section137768191623)
-   [hispark\_taurus License Agreement](#section1312121216216)
    -   [third\_party License Notice](#section129654513264)

-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

The media software development kit from HiSilicon \(Shanghai\) can adapt to complex underlying layers of different chips. It also provides basic multimedia processing functions for the multimedia subsystem, including audio/video collection, audio/video encoding and decoding, audio/video output, video pre-processing, encapsulation, decapsulation, file management, storage management, and log system. The multimedia subsystem architecture is shown as  [Figure 1](#fig4460722185514)

**Figure  1**  Multimedia subsystem architecture<a name="fig4460722185514"></a>  


![](figures/en-us_image_0000001086637618.png)

## Directory Structure<a name="section161941989596"></a>

```
/device/hisilicon/hispark_taurus/sdk_liteos
├── config                 # Hi3516D V300 device configuration information
├── mpp
│   ├── lib               # Media library files and license files of Hi3516D V300
│   └── module_init       # Libraries and license files of corresponding media module drivers of Hi3516D V300
└── uboot
    ├── out                # U-Boot compiled using third_party\uboot\u-boot-2020.01
    ├── reg                # U-Boot configuration files and license files
    ├── secureboot_ohos    # Compilation scripts for securely booting OpenHarmony
    └── secureboot_release # Source code and license files for generating secure U-Boot files
```

## Constraints<a name="section119744591305"></a>

Currently, the Hi3516D V300 chip is supported.

## Compilation and Building<a name="section137768191623"></a>

-   Compiling U-Boot

1.  Download the GCC toolchain from  [https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). Currently, the toolchain version for compiling U-Boot is  **gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2**. You can also select other GCC versions.
2.  Copy the GCC toolchain to the  **prebuilts**  directory and decompress it.
3.  Go to the  **hispark\_taurus\\sdk\_liteos\\uboot\\out\\boot**  directory and modify the path of the toolchain defined by  **OSDRV\_CROSS**  in the  **makefile**  of the directory.
4.  Run the  **make clean;make all;**  command to compile the U-Boot.

The generated U-Boot is stored in the  **hispark\_taurus\\sdk\_liteos\\uboot\\out\\boot**  directory.

## hispark\_taurus License Agreement <a name="section1312121216216"></a>

-   The  **hispark\_taurus\\sdk\_liteos\\mpp\\module\_init\\lib**  and  **hispark\_taurus\\sdk\_liteos\\mpp\\lib**  directories store the HiSilicon-developed libraries, which comply with the license of HiSilicon \(Shanghai\). You can see the following license and copyright information at the end of license files that are stored in the two directories:

    ```
    Copyright (C) 2020 Hisilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
    ```

-   The  **hispark\_taurus\\sdk\_liteos\\mpp\\module\_init\\src**  directory stores the HiSilicon-developed code, which complies with the HiSilicon \(Shanghai\) copyright statement based on the Apache License Version 2.0. You can see the following license and copyright information at the beginning of a license file that is stored in this directory:

    ```
     / *Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED. Licensed under the Apache License,* ... * / 
    ```

-   The  **hispark\_taurus\\sdk\_liteos\\uboot\\reg**  directory stores a binary file, which complies with the license of HiSilicon \(Shanghai\). You can see the following copyright information at the end of a license file that is stored in this directory:

    ```
    Copyright (C) 2020 Hisilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
    ```

-   The  **hispark\_taurus\\sdk\_liteos\\uboot\\out\\boot**  directory stores a binary U-Boot file compiled using  **u-boot-2020.01**  and  **reg\_info\_hi3518ev300.bin**  and following the  **u-boot-2020.01**  protocol. For details, see the readme files in the  **third\_party\\uboot\\u-boot-2020.01\\Licenses**  directory.
-   The  **hispark\_aries\\sdk\_liteos\\uboot\\secureboot\_release**  directory stores the open-source code of secure U-Boot, in which the HiSilicon-developed code complies with the HiSilicon \(Shanghai\) copyright statement based on the GPL license. You can see the following license and copyright information at the beginning of a license file that is stored in this directory:

    ```
     / *Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED. 
       *
       * This program is free software; you can redistribute  it and/or modify it
       * under  the terms of  the GNU General  Public License as published by the
       * Free Software Foundation;  either version 2 of the  License, or (at your
       * option) any later version.
       * ... * / 
    ```

-   The notice file stored in the  **hispark\_taurus\\NOTICE**  directory describes three open-source software, namely  **Das U-Boot 2020.01**,  **mbed TLS 2.16.6**, and  **fdk-aac v2.0.1**.

### third\_party License Notice<a name="section129654513264"></a>

The  **third\_party\\ffmpeg\\ffmpeg-y**  directory stores the open-source code of  **ffmpeg**, in which the code complies with the open-source license notice of its own software version. For details, see the readme files in the  **third\_party\\ffmpeg\\ffmpeg-y**  directory.

The  **third\_party\\uboot\\u-boot-2020.01**  directory stores the open-source code of U-Boot, which complies with the open-source license notice of its own software version. For details, see the readme files in the  **third\_party\\uboot\\u-boot-2020.01\\Licenses**  directory.

## Repositories Involved<a name="section1371113476307"></a>

device/hisilicon/build

device/hisilicon/drivers

device/hisilicon/hardware

**device/hisilicon/hispark\_taurus**

device/hisilicon/modules

device/hisilicon/third\_party/ffmpeg

device/hisilicon/third\_party/uboot


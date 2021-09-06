/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "target_config.h"
#include "los_typedef.h"
#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"
#include "los_process_pri.h"
#include "disk.h"
#include "sys/mount.h"
#include "los_bootargs.h"
#include "los_rootfs.h"
#ifdef LOSCFG_DRIVERS_VIDEO
#include "fb.h"
#endif
#ifdef LOSCFG_SHELL
#include "shell.h"
#include "shcmd.h"
#endif

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_UART
#include "console.h"
#include "hisoc/uart.h"
#endif

#ifdef LOSCFG_DRIVERS_MEM
#include "los_dev_mem.h"
#endif

#ifdef LOSCFG_DRIVERS_RANDOM
#include "los_random.h"
#include "hisoc/random.h"
#endif

#ifdef LOSCFG_DRIVERS_HDF
#include "devmgr_service_start.h"
#endif

#ifdef LOSCFG_DRIVERS_QUICKSTART
#include "los_dev_quickstart.h"
#endif

#ifdef LOSCFG_DRIVERS_USB
#include "implementation/usb_init.h"
#endif

#ifdef LOSCFG_DRIVERS_MMC
#include <asm/io.h>
#define PWR_CTRL0_REG 0x12090000
#define GPIO_AT_PMC_ENABLE_BIT 0x80
#define REG_SLEEP_TIME_MS 0x30
#endif

#ifdef LOSCFG_DRIVERS_NETDEV
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"

#define SLEEP_TIME_MS 60
#define NETIF_SETUP_OVERTIME 100

void net_init(void)
{
extern void tcpip_init(tcpip_init_done_fn initfunc, void *arg);
    dprintf("\ntcpip_init start\n");
    tcpip_init(NULL, NULL);
    dprintf("\ntcpip_init end\n");
    PRINTK("Ethernet start.");
}
#endif

#ifdef LOSCFG_DRIVERS_MMC
void SDIO_setup(void)
{
    /* enable GPIO pin multiplexing in PMC module */
    unsigned int val;

    val = readl(IO_DEVICE_ADDR(PWR_CTRL0_REG));
    val = val | GPIO_AT_PMC_ENABLE_BIT;
    writel(val, IO_DEVICE_ADDR(PWR_CTRL0_REG));
    LOS_Msleep(REG_SLEEP_TIME_MS);
}
#endif

extern void SDK_init(void);
extern void CatLogShell(void);

void SystemInit_ReBootHook(void)
{
    InitRebootHook();
}

void SystemInit_IPCM(void)
{
#ifdef LOSCFG_PLATFORM_HISI_AMP
    extern int _ipcm_vdd_init(void);
    dprintf("ipcm init ...\n");
    _ipcm_vdd_init();

#ifndef LOSCFG_ARCH_CORTEX_M7
    extern int sharefs_client_init(const char *path);
    dprintf("sharefs init ...\n");
    sharefs_client_init("/sharefs");
#endif

    extern int virt_tty_dev_init(void);
    dprintf("virt tty init ...\n");
    virt_tty_dev_init();
#endif
}

void SystemInit_RandomInit(void)
{
#ifdef LOSCFG_DRIVERS_RANDOM
    dprintf("dev urandom init ...\n");
    PlatformRandomOperationsInit();
    DevUrandomRegister();
#endif
}

void SystemInit_MMCInit(void)
{
#ifdef LOSCFG_DRIVERS_MMC
    dprintf("setting SDIO register ...\n");
    SDIO_setup();
#endif
}

void SystemInit_MemDevInit(void)
{
#ifdef LOSCFG_DRIVERS_MEM
    dprintf("dev mem init ...\n");
    DevMemRegister();
#endif
}

void SystemInit_GpioDevInit(void)
{
#ifndef LOSCFG_DRIVERS_HDF
#ifdef LOSCFG_DRIVERS_GPIO
    dprintf("gpio init ...\n");
    extern int gpio_dev_init(void);
    gpio_dev_init();
#endif
#endif
}

void SystemInit_MacInit(void)
{
#ifdef LOSCFG_DRIVERS_HIEDMAC
    extern int hiedmac_init(void);
    hiedmac_init();
#endif
#ifdef LOSCFG_DRIVERS_HIDMAC
    dprintf("dmac init ...\n");
#endif
}

void SystemInit_FlashInit(void)
{
#ifdef LOSCFG_DRIVERS_MTD_NAND
    dprintf("nand init ...\n");
    extern int nand_init(void);

    if (!nand_init()) {
        extern int add_mtd_partition(char *type, UINT32 start_addr, UINT32 length, UINT32 partition_num);

        extern int mount(const char *source, const char* target,
                         const char *filesystemtype, unsigned long mountflags,
                         const void *data);
    }

#endif

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    dprintf("spi nor flash init ...\n");
    extern int spinor_init(void);

    if (!spinor_init()) {
        dprintf("spinor_init ...\n");
    }
#endif
}

void SystemInit_NetInit(void)
{
#ifdef LOSCFG_DRIVERS_NETDEV
    dprintf("net init ...\n");
    net_init();
    dprintf("\n************************************************************\n");
#endif
}

void SystemInit_SDKInit(void)
{
#ifndef LOSCFG_DRIVERS_HDF_PLATFORM_HISI_SDK
    dprintf("calling SDK_init form HISI_SDK\n");
    SDK_init();
    dprintf("calling SDK_init form HISI_SDK done!\n");
#endif
}

void SystemInit_MountRootfs(void)
{
#ifdef LOSCFG_PLATFORM_ROOTFS
    LOS_Msleep(450);
    dprintf("OsMountRootfs start ...\n");
    if (LOS_GetCmdLine()) {
        PRINT_ERR("get cmdline error!\n");
    }
    if (LOS_ParseBootargs()) {
        PRINT_ERR("parse bootargs error!\n");
    }
    if (OsMountRootfs()) {
        PRINT_ERR("mount rootfs error!\n");
    }
    dprintf("OsMountRootfs end ...\n");
#endif
}

void SystemInit_HDFInit(void)
{
#ifdef LOSCFG_DRIVERS_HDF
    DeviceManagerSetQuickLoad(1);
    if (DeviceManagerStart()) {
        PRINT_WARN("No drivers need load by hdf manager!");
    }
#endif
}

void SystemInit_HDFInit2(void)
{
#ifdef LOSCFG_DRIVERS_HDF
    if (DeviceManagerStartStep2()) {
        PRINT_WARN("No drivers need load by hdf manager!");
    }
#endif
}

void SystemInit_UsbInit(void)
{
#ifdef LOSCFG_DRIVERS_USB
    dprintf("usb init ...\n");
    usb_init(HOST, 0);
#endif
}

void SystemInit_ConsoleInit(void)
{
#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_UART
    if (virtual_serial_init(TTY_DEVICE) != 0) {
        PRINT_ERR("virtual_serial_init failed");
    }
    if (system_console_init(SERIAL) != 0) {
        PRINT_ERR("system_console_init failed\n");
    }
#endif
}

void SystemInit_CatLogShell(void)
{
#ifdef LOSCFG_SHELL
    CatLogShell();
#endif
}

void SystemInit_UserInitProcess(void)
{
    if (OsUserInitProcess()) {
        PRINT_ERR("Create user init process faialed!\n");
        return;
    }
    return;
}

void SystemInit1(void)
{
    SystemInit_ReBootHook();
    SystemInit_MacInit();
    SystemInit_FlashInit();
    SystemInit_HDFInit2();
    SystemInit_UsbInit();
    SystemInit_CatLogShell();
}

/* reserved */
void SystemInit2(void)
{
    return;
}

/* reserved */
void SystemInit3(void)
{
    return;
}

void SystemInit_QuickstartInit(void)
{
#ifdef LOSCFG_DRIVERS_QUICKSTART
    QuickstartDevRegister();
    LosSysteminitHook hook;
    hook.func[0] = SystemInit1;
    hook.func[1] = SystemInit2;
    hook.func[2] = SystemInit3;
    QuickstartHookRegister(hook);
    PRINTK("quickstart dev init OK!!!\n");
#endif
}

void SystemInit(void)
{
    SystemInit_QuickstartInit();

    SystemInit_IPCM();
    SystemInit_RandomInit();
    SystemInit_MMCInit();
    SystemInit_MemDevInit();
    SystemInit_GpioDevInit();
    SystemInit_SDKInit();
    SystemInit_HDFInit();
    SystemInit_NetInit();  /* need to check later */
    SystemInit_MountRootfs();
    SystemInit_ConsoleInit();
#ifndef LOSCFG_DRIVERS_QUICKSTART
    SystemInit1();
    SystemInit2();
    SystemInit3();
#endif
    SystemInit_UserInitProcess();
}

/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "los_config.h"
#include "gic_common.h"
#include "los_printf.h"
#include "los_smp.h"
#include "los_vm_map.h"
#include "los_vm_zone.h"
#include "los_vm_boot.h"
#include "los_mmu_descriptor_v6.h"
#include "los_init.h"

#ifdef LOSCFG_KERNEL_SMP
#define BOOTROM_PHYS_BASE   0
#define BOOTROM_VIRT_BASE   (PERIPH_UNCACHED_BASE + PERIPH_UNCACHED_SIZE)
#define BOOTROM_MEM_SIZE    0x100000

/* The startEntry is physical addr. */
STATIC VOID bootaddr_setup(UINTPTR startEntry)
{
    UINTPTR *startAddrBae = (UINTPTR *)BOOTROM_VIRT_BASE;

    /*
     * 0xe51ff004 = "ldr  pc, [pc, #-4]"
     * next addr value will be the real booting addr
     */
    *startAddrBae = 0xe51ff004;
    startAddrBae++;
    *startAddrBae = startEntry;

    DCacheInvRange(BOOTROM_VIRT_BASE, BOOTROM_VIRT_BASE + 8);
}

#ifdef LOSCFG_TEE_ENABLE
#define TSP_CPU_ON  0xb2000011UL
STATIC INT32 raw_smc_send(UINT32 cmd)
{
    register UINT32 smc_id asm("r0") = cmd;
    do {
        asm volatile (
                "mov r0, %[a0]\n"
                "smc #0\n"
                : [a0] "+r"(smc_id)
                );
    } while (0);

    return (INT32)smc_id;
}

STATIC VOID trigger_secondary_cpu(VOID)
{
    (VOID)raw_smc_send(TSP_CPU_ON);
}

LITE_OS_SEC_TEXT_INIT INT32 release_secondary_cores(UINT32 cpuNum, UINTPTR startEntry)
{
    PRINT_RELEASE("releasing %u secondary cores\n", cpuNum);

    bootaddr_setup(startEntry);

    trigger_secondary_cpu();

    return 0;
}
#else
#define CLEAR_RESET_REG_STATUS(regval) (regval) &= ~(1U << 2)
LITE_OS_SEC_TEXT_INIT INT32 release_secondary_cores(UINT32 cpuNum, UINTPTR startEntry)
{
    UINT32 regval;

    PRINT_RELEASE("releasing %u secondary cores\n", cpuNum);

    bootaddr_setup(startEntry);

    /* clear the second cpu reset status */
    READ_UINT32(regval, PERI_CRG30_BASE);
    CLEAR_RESET_REG_STATUS(regval);
    WRITE_UINT32(regval, PERI_CRG30_BASE);

    return 0;
}
#endif /* LOSCFG_TEE_ENABLE */

STATIC struct SmpOps g_hisiSmpOps = {
    .SmpCpuOn = release_secondary_cores,
};

STATIC VOID HisiSmpInit(VOID)
{
    LOS_SmpOpsSet(&g_hisiSmpOps);
}

LOS_MODULE_INIT(HisiSmpInit, LOS_INIT_LEVEL_EARLIEST);
#endif /* LOSCFG_KERNEL_SMP */

#ifdef LOSCFG_KERNEL_MMU
LosArchMmuInitMapping g_archMmuInitMapping[] = {
    {
        .phys = SYS_MEM_BASE,
        .virt = KERNEL_VMM_BASE,
        .size = KERNEL_VMM_SIZE,
        .flags = MMU_DESCRIPTOR_KERNEL_L1_PTE_FLAGS,
        .name = "KernelCached",
    },
    {
        .phys = SYS_MEM_BASE,
        .virt = UNCACHED_VMM_BASE,
        .size = UNCACHED_VMM_SIZE,
        .flags = MMU_INITIAL_MAP_NORMAL_NOCACHE,
        .name = "KernelUncached",
    },
    {
        .phys = PERIPH_PMM_BASE,
        .virt = PERIPH_DEVICE_BASE,
        .size = PERIPH_DEVICE_SIZE,
        .flags = MMU_INITIAL_MAP_DEVICE,
        .name = "PeriphDevice",
    },
    {
        .phys = PERIPH_PMM_BASE,
        .virt = PERIPH_CACHED_BASE,
        .size = PERIPH_CACHED_SIZE,
        .flags = MMU_DESCRIPTOR_KERNEL_L1_PTE_FLAGS,
        .name = "PeriphCached",
    },
    {
        .phys = PERIPH_PMM_BASE,
        .virt = PERIPH_UNCACHED_BASE,
        .size = PERIPH_UNCACHED_SIZE,
        .flags = MMU_INITIAL_MAP_STRONGLY_ORDERED,
        .name = "PeriphStronglyOrdered",
    },
#ifdef LOSCFG_KERNEL_SMP
    {
        .phys = BOOTROM_PHYS_BASE,
        .virt = BOOTROM_VIRT_BASE,
        .size = BOOTROM_MEM_SIZE,
        .flags = MMU_DESCRIPTOR_KERNEL_L1_PTE_FLAGS,
        .name = "BootRom",
    },
#endif
    {0}
};
#endif


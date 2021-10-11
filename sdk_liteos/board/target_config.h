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

#ifndef _TARGET_CONFIG_H
#define _TARGET_CONFIG_H


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_SYS_CLOCK             (50000000)
#define SYS_CTRL_REG_BASE        IO_DEVICE_ADDR(0x12020000)
#define REG_SC_CTRL              0

/* smp */
#define CRG_REG_ADDR              0x12010000
#define CRG_REG_BASE              IO_DEVICE_ADDR(CRG_REG_ADDR)
#define PERI_CRG30_BASE           IO_DEVICE_ADDR(CRG_REG_ADDR + 0x0078)  /* cpu freq-mode & reset CRG */

/* memory config */
#define CACHE_ALIGNED_SIZE        64
#ifdef LOSCFG_TEE_ENABLE
#define DDR_MEM_ADDR            0x81000000
#define DDR_MEM_SIZE            0x1f000000
#else
#define DDR_MEM_ADDR            0x80000000
#define DDR_MEM_SIZE            0x20000000
#endif
/* Peripheral register address base and size */
#define PERIPH_PMM_BASE         0x10000000
#define PERIPH_PMM_SIZE         0x10000000
#define SYS_MEM_SIZE_DEFAULT    0x07f00000
/* hwi */
/**
 * Maximum number of supported hardware devices that generate hardware interrupts.
 * The maximum number of hardware devices that generate hardware interrupts is 128.
 */
#define OS_HWI_MAX_NUM                  128
/**
 * Maximum interrupt number.
 */
#define OS_HWI_MAX                      ((OS_HWI_MAX_NUM) - 1)
/**
 * Minimum interrupt number.
 */
#define OS_HWI_MIN                      0
/**
 * Maximum usable interrupt number.
 */
#define OS_USER_HWI_MAX                 OS_HWI_MAX
/**
 * Minimum usable interrupt number.
 */
#define OS_USER_HWI_MIN                 OS_HWI_MIN

#define NUM_HAL_INTERRUPT_CNTPSIRQ      29
#define NUM_HAL_INTERRUPT_CNTPNSIRQ     30
#ifdef LOSCFG_TEE_ENABLE
#define OS_TICK_INT_NUM                 NUM_HAL_INTERRUPT_CNTPNSIRQ // use non-secure physical timer for now
#else
#define OS_TICK_INT_NUM                 NUM_HAL_INTERRUPT_CNTPSIRQ // use secure physical timer for now
#endif

#define NUM_HAL_INTERRUPT_TIMER7        36
#define NUM_HAL_INTERRUPT_UART0         38
#define NUM_HAL_INTERRUPT_TEE_SPI_NOTIFY 105

/* gic */
#define GIC_BASE_ADDR             IO_DEVICE_ADDR(0x10300000)
#define GICD_OFFSET               0x1000                          /* interrupt distributor offset */
#define GICC_OFFSET               0x2000                          /* CPU interface register offset */

/* timer */
#define TIMER7_REG_BASE           IO_DEVICE_ADDR(0x12003020)
#define TIMER6_REG_BASE           IO_DEVICE_ADDR(0x12003000)
#define TIMER5_REG_BASE           IO_DEVICE_ADDR(0x12002020)
#define TIMER4_REG_BASE           IO_DEVICE_ADDR(0x12002000)
#define TIMER3_REG_BASE           IO_DEVICE_ADDR(0x12001020)
#define TIMER2_REG_BASE           IO_DEVICE_ADDR(0x12001000)
#define TIMER1_REG_BASE           IO_DEVICE_ADDR(0x12000020)
#define TIMER0_REG_BASE           IO_DEVICE_ADDR(0x12000000)

#define BIT(n)                    (1U << (n))
#define TIMER4_ENABLE             BIT(20)
#define TIMER5_ENABLE             BIT(21)
#define TIMER7_ENABLE             BIT(23)

#define TIMER_TICK_REG_BASE       TIMER4_REG_BASE   /* timer for tick */
#define TIMER_TICK_ENABLE         TIMER4_ENABLE
#define TIMER_TIME_REG_BASE       TIMER5_REG_BASE   /* timer for time */
#define TIMER_TIME_ENABLE         TIMER5_ENABLE
#define HRTIMER_TIMER_REG_BASE    TIMER7_REG_BASE /* timer for hrtimer */
#define HRTIMER_TIMER_ENABLE      TIMER7_ENABLE

#define NUM_HAL_INTERRUPT_HRTIMER  NUM_HAL_INTERRUPT_TIMER7

#define TIMER_LOAD              0x0
#define TIMER_VALUE             0x4
#define TIMER_CONTROL           0x8
#define TIMER_INT_CLR           0xc

/* uart */
#define UART_REG_ADDR             0x120a0000
#define UART0_REG_PBASE           (UART_REG_ADDR + 0x0000)
#define UART0_REG_BASE            IO_DEVICE_ADDR(UART0_REG_PBASE)
#define TTY_DEVICE                "/dev/uartdev-0"
#define UART_REG_BASE             UART0_REG_BASE
#define NUM_HAL_INTERRUPT_UART    NUM_HAL_INTERRUPT_UART0

#ifdef LOSCFG_DEBUG_VERSION
#define LOSCFG_BASE_CORE_TSK_LIMIT  256
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

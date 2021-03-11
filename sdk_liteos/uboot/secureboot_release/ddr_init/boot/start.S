// SPDX-License-Identifier: GPL-2.0

// Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.

#include <platform.h>

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */

.globl _start
_start:
	b       reset
	ldr     pc, _undefined_instruction
	ldr     pc, _software_interrupt
	ldr     pc, _prefetch_abort
	ldr     pc, _data_abort
	ldr     pc, _not_used
	ldr     pc, _irq
	ldr     pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt:    .word software_interrupt
_prefetch_abort:        .word prefetch_abort
_data_abort:            .word data_abort
_not_used:              .word not_used
_irq:                   .word irq
_fiq:                   .word fiq
_pad:                   .word 0x12345678 /* now 16*4=64 */
__blank_zone_start:
.fill 1024*8,1,0
__blank_zone_end:

.globl _blank_zone_start
_blank_zone_start:
.word __blank_zone_start


.globl _blank_zone_end
_blank_zone_end:
.word __blank_zone_end

.balignl 16,0xdeadbeef


_TEXT_BASE:
        .word   TEXT_BASE


	/*
	 * the actual reset code 
	 */

reset:

    /*
     * disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
     * except if in HYP mode already
     */
    mrs r0, cpsr
    and r1, r0, #0x1f       @ mask mode bits
    teq r1, #0x1a       @ test for HYP mode
    bicne   r0, r0, #0x1f       @ clear all mode bits
    orrne   r0, r0, #0x13       @ set SVC mode
    orr r0, r0, #0xc0       @ disable FIQ and IRQ
    msr cpsr,r0

/*************************************************************************
 *
 * Setup CP15 registers (cache, MMU, TLBs). The I-cache is turned on unless
 * CONFIG_SYS_ICACHE_OFF is defined.
 *
 *************************************************************************/
    /*
     * Invalidate L1 I/D
     */
    mov r0, #0          @ set up for MCR
    mcr p15, 0, r0, c8, c7, 0   @ invalidate TLBs
    mcr p15, 0, r0, c7, c5, 0   @ invalidate icache
    mcr p15, 0, r0, c7, c5, 6   @ invalidate BP array
    mcr     p15, 0, r0, c7, c10, 4  @ DSB
    mcr     p15, 0, r0, c7, c5, 4   @ ISB

    /*
     * disable MMU stuff and caches
     */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x00002000 @ clear bits 13 (--V-)
    bic r0, r0, #0x00000007 @ clear bits 2:0 (-CAM)
    orr r0, r0, #0x00000002 @ set bit 1 (--A-) Align
    orr r0, r0, #0x00000800 @ set bit 11 (Z---) BTB
#ifdef CONFIG_SYS_ICACHE_OFF
    bic r0, r0, #0x00001000 @ clear bit 12 (I) I-cache
#else
    orr r0, r0, #0x00001000 @ set bit 12 (I) I-cache
#endif
    mcr p15, 0, r0, c1, c0, 0

	/*
	 *  read system register REG_SC_GEN2
         *  check if ziju flag
	 */
	ldr	r0, =SYS_CTRL_REG_BASE
	mov	r1, sp                   /* save sp */
	str	r1, [r0, #REG_SC_GEN2]  /* clear ziju flag */
	
	/* init PLL/DDRC/pin mux/... */
	ldr	r0, _blank_zone_start
	ldr	r1, _TEXT_BASE
	sub	r0, r0, r1
	
	@ldr	r1, =RAM_START_ADRS
	ldr	sp, =STACK_TRAINING
	
	adrl	r1, _start
	add	r0, r0, r1
	mov	r1, #0x0                 /* flags: 0->normal 1->pm */
	bl	init_registers           /* init PLL/DDRC/... */

	/* after ziju, we need ddr traning */
	ldr	r0, =REG_BASE_SCTL
	bl	start_ddr_training       /* DDR training */

	ldr	r0, =SYS_CTRL_REG_BASE
	ldr	r1, [r0, #REG_SC_GEN2]
	mov	sp, r1		        /* restore sp */
	ldr	r1, [r0, #REG_SC_GEN3]
	mov	pc, r1				/* return to bootrom */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	b	.                        /* bug here */

.global reset_cpu
reset_cpu:
    ldr r1, rstctl          @ get addr for global reset
                        @ reg
    mov r3, #0x2            @ full reset pll + mpu
    str r3, [r1]            @ force reset
    mov r0, r0

_loop_forever:
    b   _loop_forever
rstctl:
    .word  0x12020004

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@       void memcpy(r1, r0, r2);
@
.align  2
memcpy:
	add     r2, r0, r2
memcpy_loop:
	ldmia   r0!, {r3 - r10}
	stmia   r1!, {r3 - r10}
	cmp     r0, r2
	ble     memcpy_loop
	mov     pc, lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.align  2
msg_main_cpu_startup:
	mov     r5, lr
	add     r0, pc, #4
	bl      uart_early_puts
	mov     pc, r5
L10:
#ifndef CONFIG_SUPPORT_CA_RELEASE
	.ascii "\r\n\r\nSystem startup\r\n\0"
#else
	.ascii "\r\n\r\n\r\n\0"
#endif

/*
 * exception handlers
 */
	.align	5
undefined_instruction:
software_interrupt:
prefetch_abort:
data_abort:
not_used:
irq:
fiq:
	/* reset */
	ldr	r1, =REG_BASE_SCTL
	ldr     r0, =1
	str     r0, [r1, #REG_SC_SYSRES]
	b       .

/*#include "lowlevel_init.S"*/

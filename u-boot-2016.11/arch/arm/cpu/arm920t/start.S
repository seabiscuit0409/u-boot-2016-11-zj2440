/*
 *  armboot - Startup Code for ARM920 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <common.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */
	.globl	reset

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

#ifdef CONFIG_S3C24X0

#if defined(CONFIG_S3C2400)
  #define pWTCON	0x15300000
  #define INTMSK	0x14400008	/* Interrupt-Controller base addresses */
  #define CLKDIVN	0x14800014	/* clock divisor register */
#else
  #define pWTCON	0x53000000
  #define INTMSK	0x4A000008	/* Interrupt-Controller base addresses */
  #define INTSUBMSK	0x4A00001C
  #define CLKDIVN	0x4C000014	/* clock divisor register */
  #define MPLLCON   0x4C000004 
  #define MEM_CTL_BASE 0x48000000
  #define MPLLCON_400MHz ((0x5c << 12) | (0x1 << 4) | (0x1 << 0))
#endif
	
	/* turn off the watchdog */

	ldr	r0, =pWTCON
	mov	r1, #0x0
	str	r1, [r0]

	/*设置异步总线模式*/
	mrc p15,0,r1,c1,c0,0
	orr r1,r1, #0xc0000000
	mcr p15,0,r1,c1,c0,0

	/*设置时钟分频系数*/
	/* FCLK:HCLK:PCLK = 1:4:8 */
	/* default FCLK is 120 MHz ! */
	ldr r0, =CLKDIVN
	mov r1, #0x5
	str r1, [r0]

	/*设置时钟频率为400MHZ*/
	/*	
		m= 92 + 8 = 100
		p = 1 + 2 = 3
		s = 1
		MPLL = (2*100*12)/(3*2^1)
			= 2400/6  = 400
			(92 << 12) | (1 << 4) | (1 << 0)
	*/	

	ldr r0, =MPLLCON
	ldr r1, =MPLLCON_400MHz
	str r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]
#if defined(CONFIG_S3C2440)
	ldr	r1, =0x7ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]
#endif

	/*打开d-cache*/
	/*
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #(1 << 12)
	mcr p15, 0, r0, c1, c0, 0
	*/
#endif	/* CONFIG_S3C24X0 */

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
	 	/*added by zhj*/

	/*setup stack*/
	//ldr sp, =(0x40000000 + 4096)
	/*
	bl	uart0_init
	mov r0, #0x6d			@输出"m"
	bl  put_c
	*/
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit
	//bl  test_mem
	/*
	mov r0, #0x6e				@输出"n"
	bl put_c
	*/
	/* 3. 初始化SDRAM */
	//ldr r0, =MEM_CTL_BASE
	//adr r1, sdram_config     /* sdram_config的当前地址 */
	//add r3, r0, #(13*4)
//1:
	//ldr r2, [r1], #4
	//str r2, [r0], #4
	//cmp r0, r3
	//bne 1b
#endif
	ldr sp, =0x34000000
	//bl	nandflash_init

	/*code relocate*/
	mov r0, #0
	ldr r1, =CONFIG_SYS_TEXT_BASE
	ldr r2, =_image_binary_end
	//ldr r3, =_start

	sub r2, r2, r1
	

	/*cmp r1, r3*/
	bl copy_code_to_sdram
	bl  clear_bss	
	
	/*fixed by zhj, further initialization*/
	//bl	_main
	ldr pc, =_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

	mov	pc, lr

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */


#ifndef CONFIG_SKIP_LOWLEVEL_INIT
/*
sdram_config:
	.long 0x22011110	 //BWSCON
	.long 0x00000700	 //BANKCON0
	.long 0x00000700	 //BANKCON1
	.long 0x00000700	 //BANKCON2
	.long 0x00000700	 //BANKCON3  
	.long 0x00000700	 //BANKCON4
	.long 0x00000700	 //BANKCON5
	.long 0x00018005	 //BANKCON6
	.long 0x00018005	 //BANKCON7
	.long 0x008C04F4	 // REFRESH
	.long 0x000000B1	 //BANKSIZE
	.long 0x00000030	 //MRSRB6
	.long 0x00000030	 //MRSRB7
*/
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip, lr

	bl	lowlevel_init
	mov	lr, ip
#endif
	mov	pc, lr
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

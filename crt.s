/* SPDX-License-Identifier: BSD-3-Clause */
/****************************************************************************
*  Copyright (c) 2006 by Michael Fischer. All rights reserved.
****************************************************************************
*
*  History:
*
*  18.12.06  mifi   First Version
*                   The hardware initialization is based on the startup file
*                   crtat91sam7x256_rom.S from NutOS 4.2.1.
*                   Therefore partial copyright by egnite Software GmbH.
****************************************************************************/

/*
 * Some defines for the program status registers
 */
ARM_MODE_USER  = 0x10      /* Normal User Mode                             */
ARM_MODE_FIQ   = 0x11      /* FIQ Fast Interrupts Mode                     */
ARM_MODE_IRQ   = 0x12      /* IRQ Standard Interrupts Mode                 */
ARM_MODE_SVC   = 0x13      /* Supervisor Interrupts Mode                   */
ARM_MODE_ABORT = 0x17      /* Abort Processing memory Faults Mode          */
ARM_MODE_UNDEF = 0x1B      /* Undefined Instructions Mode                  */
ARM_MODE_SYS   = 0x1F      /* System Running in Privileged Operating Mode  */
ARM_MODE_MASK  = 0x1F

I_BIT          = 0x80      /* disable IRQ when I bit is set */
F_BIT          = 0x40      /* disable IRQ when I bit is set */

/****************************************************************************/
/*               Vector table and reset entry                               */
/****************************************************************************/

.section .init, "ax"
.code 32

_vectors:
   b ResetHandler    /* Reset                 */
   b UndefHandler    /* Undefined instruction */
   b SWIHandler      /* Software interrupt    */
   b PAbortHandler   /* Prefetch abort        */
   b DAbortHandler   /* Data abort            */
   b CrashHandler    /* Reserved              */
   b IRQHandler      /* IRQ interrupt         */
   b FIQHandler      /* FIQ interrupt         */

   .ltorg

   .global ResetHandler
   .global ExitFunction

#if USE_BREAKPOINTS
   .global DN_Packet_DCC_WaitForBP
   .global DCC_PKT_RW_DATA
   .global DCC_PKT_RW_SIZE
#endif

#ifdef PIC
   .extern pic_relocate
#endif

   .extern dcc_main
   .extern __stack_und_end

/* Variables */
StartAddress:  .word 0xffffffff
NoAutoSize:    .word 0x0
PageSize:      .word 0xffffffff

/* Loader via H/W BP polling */
DCC_PKT_RW_SIZE:   .word 0xffffffff
DCC_PKT_RW_DATA:   .word 0xffffffff
DCC_PKT_HW_BP:     .word DN_Packet_DCC_WaitForBP
DCC_CANWRITE:      .word 0x0

/* Crash handlers */
UndefHandler:
SWIHandler:
PAbortHandler:
DAbortHandler:
IRQHandler:
FIQHandler:
CrashHandler:
   b CrashHandler
.word CrashHandler

/* DCC info */
.ascii "DNDL"
#ifdef PIC
.word 1
#else
.word 0
#endif
.word _vectors
.word __bss_start
.word __heap_start
.asciz "A:DumpNow DCC Loader. (c) 2026 Wrapper.;Compile flags: " ADEFS ";Compile Date: " __DATE__
.align

/****************************************************************************/
/*                           Reset handler                                  */
/****************************************************************************/
ResetHandler:
   /*
    * Setup a stack for each mode
    */
   msr   CPSR_c, #ARM_MODE_SVC | I_BIT | F_BIT     /* Supervisor Mode */

#if USE_ICACHE \
   && (( defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__) ) \
   || ( defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) ) \
   || ( defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7S__) || defined(__ARM_ARCH_7R__) ) \
   || ( defined(CPU_XSCALE) ))
   mrc   p15, 0, r0, cr1, cr0, 0
   orr   r0, #0x1000
   mcr   p15, 0, r0, cr1, cr0, 0
#endif

   /* Needed to flush DCC read buffer */
#if \
  ( defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) ) \
  || ( defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7S__) || defined(__ARM_ARCH_7R__) )
   mrc   p14, 0, r0, cr0, cr5, 0
#elif defined(CPU_XSCALE)
   mrc   p14, 0, r0, cr9, cr0, 0
#else
   mrc   p14, 0, r0, cr1, cr0, 0
#endif

   /* 01 - Initialize stack section */
#ifdef PIC
   mov   r0, #0
   adr   r0, _vectors
#endif

   ldr   sp, =__stack_svc_end
#ifdef PIC
   add   sp, r0
#endif

   bl plat_init

   /* 02 - Reset memory */
#ifdef PIC
   mov   r0, #0
   adr   r0, _vectors
#endif

   /*
    * Clear .bss section
    */
   ldr   r1, =__bss_start
   ldr   r2, =__bss_end
   mov   r3, #0
   
#ifdef PIC
   add   r1, r0
   add   r2, r0
#endif
bss_clear_loop:
   cmp   r1, r2
   strne r3, [r1], #+4
   bne   bss_clear_loop

   /* 03 - Code initialize */
#ifdef PIC
   mov   r0, #0
   adr   r0, _vectors

   /* Setup PIC */
   ldr   r1, =_reloc_start
   ldr   r2, =_reloc_end

   add   r1, r0
   add   r2, r0

   ldr   r3, =edata

   bl    pic_relocate

   /* Setup GOT */
   ldr   r9, =_sgot
   add   r9, r0
#endif

   /* 04 - Jump to Main */
   mov   r0, #0
   adr   r0, StartAddress
   ldr   r0, [r0]

   mov   r1, #0
   adr   r1, PageSize
   ldr   r1, [r1]

   mov   r2, #0 /* No arguments */
   mov   r3, #0 /* No arguments */

   b dcc_main

ExitFunction:
   nop
   nop
   nop
   b ExitFunction

/* Breakpoint loader routines */
#if USE_BREAKPOINTS
DN_Packet_DCC_WaitForBP:
   b DN_Packet_DCC_WaitForBP
   mov   r0, #0
   adr   r0, DCC_PKT_RW_DATA
   ldr   r0, [r0]
   bx lr
#else
DN_Packet_DCC_WaitForBP:
   b DN_Packet_DCC_WaitForBP
   bx lr
#endif

/* libc functions */
.global memset
memset:
	mov	r3, r0
memset.loop:
	subs	r2, r2, #1
	bmi	memset.ret
	strb	r1, [r0], #1
	b	memset.loop
memset.ret:
	mov	r0, r3
	bx lr

.global strlen
strlen:
	mov	r2, r0
strlen.loop:
	ldrb	r1, [r0], #1
	tst	r1, r1
	bne	strlen.loop
	sub	r0, r0, r2
	sub	r0, r0, #1
	bx lr

/* End */
.weak ExitFunction
.weak UndefHandler, PAbortHandler, DAbortHandler
.weak IRQHandler, FIQHandler

.ltorg
/*** EOF ***/

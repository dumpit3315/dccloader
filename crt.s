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
   b 0               /* Reserved              */
   b IRQHandler      /* IRQ interrupt         */
   b FIQHandler      /* FIQ interrupt         */

   .ltorg

   .global ResetHandler
   .global ExitFunction
   .global NorStartAddress
   .extern main
   .extern __stack_und_end

/* Variables */
NorStartAddress1:  .word 0x12345678
NorStartAddress2:  .word 0x12345678
NorStartAddress3:  .word 0x12345678
NorStartAddress4:  .word 0x12345678
#ifdef USE_LWMEM
lwmem_init:
   .word 0x00020000
   .word 0x00060000
lwmem_init_end:
   .word 0x00000000
   .word 0x00000000
#endif
/****************************************************************************/
/*                           Reset handler                                  */
/****************************************************************************/
ResetHandler:
   /*
    * Setup a stack for each mode
    */
   mov   r0, #0
   adr   r0, _vectors

#ifdef SETUP_STACK_OTHERS   
   msr   CPSR_c, #ARM_MODE_UNDEF | I_BIT | F_BIT   /* Undefined Instruction Mode */
   ldr   sp, =__stack_und_end
   add   sp, r0

   msr   CPSR_c, #ARM_MODE_ABORT | I_BIT | F_BIT   /* Abort Mode */
   ldr   sp, =__stack_abt_end
   add   sp, r0

   msr   CPSR_c, #ARM_MODE_FIQ | I_BIT | F_BIT     /* FIQ Mode */
   ldr   sp, =__stack_fiq_end
   add   sp, r0

   msr   CPSR_c, #ARM_MODE_IRQ | I_BIT | F_BIT     /* IRQ Mode */
   ldr   sp, =__stack_irq_end
   add   sp, r0
#endif

   msr   CPSR_c, #ARM_MODE_SVC | I_BIT | F_BIT     /* Supervisor Mode */
   ldr   sp, =__stack_svc_end
   add   sp, r0

#ifndef DONT_CLEAR_BSS
   /*
    * Clear .bss section
    */
   ldr   r1, =__bss_start   
   ldr   r2, =__bss_end   
   mov   r3, #0
   
   add   r1, r0
   add   r2, r0
bss_clear_loop:
   cmp   r1, r2
   strne r3, [r1], #+4
   bne   bss_clear_loop
#endif


   /*
    * Jump to main
    */
    
#ifdef ENABLE_DCC_INTERRUPTS
   mrs   r0, cpsr
   bic   r0, r0, #I_BIT | F_BIT     /* Enable FIQ and IRQ interrupt (TODO: RIFF doesn't disable interrupt?) */
   msr   cpsr, r0
#endif
   
#ifdef USE_LWMEM
   /*
    * Setup lwmem memory manager
    */
   mov   r0, #0
   adr   r0, lwmem_init
   ldr   r0, [r0]
   
   mov   r1, #0
   adr   r1, _vectors

   add   r0, r1

   mov   r1, #0
   adr   r1, lwmem_init

   str   r0, [r1]

   mov   r0, #0
   adr   r0, lwmem_init
   bl    lwmem_assignmem
#endif

   /*
    * Start
    */
   mov   r0, #0 /* No arguments */
   mov   r1, #0 /* No arguments */
   mov   r2, #0 /* No arguments */
   mov   r3, #0 /* No arguments */

   mrc   p14, 0, r0, cr1, cr0, 0 /* TOOD: Why is this line necessary */

   mov   r0, #0
   adr   r0, NorStartAddress1
   ldr   r0, [r0]

   mov   r1, #0
   adr   r1, NorStartAddress2
   ldr   r1, [r1]

   mov   r2, #0
   adr   r2, NorStartAddress3
   ldr   r2, [r2]

   mov   r3, #0
   adr   r3, NorStartAddress4
   ldr   r3, [r3]
   
   b main

ExitFunction:
   nop
   nop
   nop
   b ExitFunction

/****************************************************************************/
/*                         Default interrupt handler                        */
/****************************************************************************/

UndefHandler:
   b UndefHandler

SWIHandler:
   b SWIHandler

PAbortHandler:
   b PAbortHandler

DAbortHandler:
   b DAbortHandler

IRQHandler:
   b IRQHandler

FIQHandler:
   b FIQHandler

   .weak ExitFunction
   .weak UndefHandler, PAbortHandler, DAbortHandler
   .weak IRQHandler, FIQHandler

/* Memory stuff */
.global memcpy
memcpy:
	mov	r12, r0
memcpy_loop:
	subs	r2, r2, #1
	bmi	memcpy_end
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	b	memcpy_loop
memcpy_end:
	mov	r0, r12
	bx lr

.global memset
memset:
	mov	r12, r0
memset_loop:
	subs	r2, r2, #1
	bmi	memset_end
	strb	r1, [r0], #1
	b	memset_loop
memset_end:
	mov	r0, r12
	bx lr

.global memmove
memmove:
	mov	r12, r0
	cmp	r0, r1
	ble	.fw
	add	r3, r1, r2
	cmp	r0, r3
	bgt	.fw

	@ copying the memory in reverse order
	add	r0, r0, r2
	add	r1, r1, r2
.bw:
	subs	r2, r2, #1
	bmi	.ret
	ldrb	r3, [r1, #-1]!
	strb	r3, [r0, #-1]!
	b	.bw
.fw:
	subs	r2, r2, #1
	bmi	.ret
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	b	.fw
.ret:
	mov	r0, r12
	bx lr

.global memcmp
memcmp:
	subs	r2, r2, #1
	bmi	.match
	ldrb	r3, [r0], #1
	ldrb	r12, [r1], #1
	subs	r3, r3, r12
	movne	r0, r3
	bne	memcmp_end
	b	memcmp
.match:
	mov	r0, #0
memcmp_end:
	bx lr
.ltorg
/*** EOF ***/

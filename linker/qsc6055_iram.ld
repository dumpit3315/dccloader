/* SPDX-License-Identifier: BSD-3-Clause */

/****************************************************************************
*  Copyright (c) 2006 by Michael Fischer. All rights reserved.
****************************************************************************
*
*  History:
*
*  30.03.06  mifi   First Version
****************************************************************************/

ENTRY(ResetHandler)
SEARCH_DIR(.)

/*
 * Define stack size here
 */
FIQ_STACK_SIZE = 0x0100;
IRQ_STACK_SIZE = 0x0100;
ABT_STACK_SIZE = 0x0100;
UND_STACK_SIZE = 0x0100;
SVC_STACK_SIZE = 0x0100;

PHDRS
{
  code         PT_LOAD;
  data         PT_LOAD;
}

MEMORY
{
  ram : org = 0x78000000, len = 128k
}

/*
 * Do not change the next code
 */
SECTIONS
{
  .text :
  {
    *(.vectors);
    . = ALIGN(4);
    *(.init);
    . = ALIGN(4);
    *(.text);
    . = ALIGN(4);
    *(.rodata);
    . = ALIGN(4);
    *(.rodata*);
    . = ALIGN(4);
    *(.glue_7t);
    . = ALIGN(4);
    *(.glue_7);
    . = ALIGN(4);
    etext = .;
  } > ram :code

  .data :
  {
    PROVIDE (__data_start = .);
    *(.data)
    . = ALIGN(4);
    edata = .;
    PROVIDE (__data_end = .);
  } > ram :data

  .plt : ALIGN(4)
  {
      *(.plt)
      . = ALIGN(4);
  } > ram :code

  .got : ALIGN(4)
  {
      _sgot = .;
      _sgot_main = .;
      *(.got)
      _egot_main = .;
  } > ram :data
    
  .got.plt : ALIGN(4)
  {
      _sgot_plt = .;
      *(.got.plt)
      _egot_plt = .;
      _egot = .;
      _edata = .;
  } > ram :data

  .rel.dyn : ALIGN(4)
  {
      _reloc_start = .;
      *(.rel.dyn)
  } > ram :data

  .bss :
  {
    _reloc_end = .;
    PROVIDE (__bss_start = .);
    *(.bss)
    *(COMMON)
    . = ALIGN(4);
    PROVIDE (__bss_end = .);

    . = ALIGN(256);

    PROVIDE (__stack_start = .);

    PROVIDE (__stack_fiq_start = .);
    . += FIQ_STACK_SIZE;
    . = ALIGN(4);
    PROVIDE (__stack_fiq_end = .);

    PROVIDE (__stack_irq_start = .);
    . += IRQ_STACK_SIZE;
    . = ALIGN(4);
    PROVIDE (__stack_irq_end = .);

    PROVIDE (__stack_abt_start = .);
    . += ABT_STACK_SIZE;
    . = ALIGN(4);
    PROVIDE (__stack_abt_end = .);

    PROVIDE (__stack_und_start = .);
    . += UND_STACK_SIZE;
    . = ALIGN(4);
    PROVIDE (__stack_und_end = .);

    PROVIDE (__stack_svc_start = .);
    . += SVC_STACK_SIZE;
    . = ALIGN(4);
    PROVIDE (__stack_svc_end = .);
    PROVIDE (__stack_end = .);
    PROVIDE (__heap_start = .);
  } > ram

}
/*** EOF ***/

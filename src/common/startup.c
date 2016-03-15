/*
  Startup code for baremetal Arria 10 project
  that runs from OCRAM.
  
  by David M. Koltak  02/18/2016

***

Copyright (c) 2016 David M. Koltak

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
  
*/

#include "alt_interrupt.h"
#include "terminal.h"
#include "boot.h"
#include <string.h>

void alt_int_handler_irq(); // NOTE: Defined in alt_interrupt.c, but not in header file

extern int stack_svc_block; // NOTE: Defined in linker script
extern int stack_irq_block;
extern int stack_abt_block;

__attribute__((section(".pimage_hdr"))) int pimage_header[5];

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Useful symbols for user code to use

__attribute__((weak)) void main()
{ terminal(); } // NOTE: Just call terminal() if no custom main() defined

volatile int _abort_data_count; // Incremented on every data abort

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

__attribute__((interrupt("UNDEF"))) void _undefined_vector()
{ while (1); }

__attribute__((naked)) void _service_vector() 
{ while (1); }

__attribute__((interrupt("ABORT"))) void _abort_instr_vector()
{ while (1); }

__attribute__((interrupt("ABORT"))) void _abort_data_vector()
{ _abort_data_count++; }

__attribute__((interrupt("IRQ"))) void _irq_vector()
{ alt_int_handler_irq(); }

__attribute__((interrupt("FIQ"))) void _fiq_vector()
{ while (1); }

__attribute__((naked, section(".vector"))) void _vector_table()
{
  __asm("ldr pc, =_startup;\n");
  __asm("ldr pc, =_undefined_vector;\n");
  __asm("ldr pc, =_service_vector;\n");
  __asm("ldr pc, =_abort_instr_vector;\n");
  __asm("ldr pc, =_abort_data_vector;\n");
  __asm("nop;\n");
  __asm("ldr pc, =_irq_vector;\n");
  __asm("ldr pc, =_fiq_vector;\n");
}

extern int _bss_start;
extern int _bss_end;

__attribute__((naked, noreturn, section(".startup"))) void _startup()
{ 
  //
  // Move vector table to start of OCRAM
  //
  
  __asm("MRC p15, 0, r0, c1, c0, 0;\n");
  __asm("bic r0, #(1 << 13);\n");
  __asm("MCR p15, 0, r0, c1, c0, 0;\n");
  __asm("ldr r0, =_start;\n");
  __asm("MCR p15, 0, r0, c12, c0, 0;\n");
  
  //
  // Initialize stack registers
  //
  
  __asm("mrs r0, cpsr;\n");
  __asm("and r0, r0, #0xFFFFFFE0;\n");
  
  __asm("orr r1, r0, #0x17;\n");
  __asm("msr cpsr, r1;\n");
  __asm("ldr sp, =(stack_abt_block - 16);\n");
    
  __asm("orr r1, r0, #0x12;\n");
  __asm("msr cpsr, r1;\n");
  __asm("ldr sp, =(stack_irq_block - 16);\n");
  
  __asm("orr r1, r0, #0x13;\n");
  __asm("msr cpsr, r1;\n");
  __asm("ldr sp, =(stack_svc_block - 16);\n");
  
  //
  // Clear BSS
  //
  
  memset((void*)&_bss_start, 0, (int) (&_bss_end - &_bss_start));
  
  //
  // Run boot sequence, main, and then shutdown sequence
  //
  
  boot();
  main();
  shutdown();
  
  while(1);
}


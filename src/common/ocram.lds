/*
  GCC Linker Script for Arria 10 SoC
  
  by David Koltak  03/15/2016

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

ENTRY(_start);

SECTIONS
{
  /* BASE ADDRESS OF OCRAM */
  . = 0xFFE00000;
  
  _start = .;
  .vector :
  { 
    *(.vector); 
    . = ALIGN(16);
  }

  .pimage_hdr : { *(.pimage_hdr); }
  
  _text_start = .;
  .text : 
  {
    . = ALIGN(4);
    *(.startup); *(.text);
    *(.text.startup);
    . = ALIGN(4);
  }
  _text_end = .;
  
  _rodata_start = .;
  .rodata : 
  {
    . = ALIGN(4);
    *(.rodata);
    *(.rodata.str1.4);
    . = ALIGN(4);
  }
  _rodata_end = .;
  
  _data_start = .;
  .data : 
  {
    . = ALIGN(4);
    boot_steps = .;
    *(.boot_steps);
    *(.boot_step_null);    
    terminal_cmds = .;
    *(.terminal_cmds);
    *(.terminal_cmd_null);   
    *(.data);
    . = ALIGN(4);
  }
  _data_end = .;
  
  _bss_start = .;
  .bss :
  { 
    . = ALIGN(4);
    *(.bss);
    . = ALIGN(4);
  }
  _bss_end = .;
  
  _stack_start = .;
  .stack :
  { 
    . = ALIGN(64);
    . += (8 * 1024);
    stack_svc_block = .;
    . += (1 * 1024);
    stack_irq_block = .;
    . += (1 * 64);
    stack_abt_block = .;
  }
  _stack_end = .;

}

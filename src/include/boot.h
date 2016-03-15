/*
  Boot sequence implementation
  
  by David M. Koltak  03/03/2016

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

#ifndef _BOOT_H_
#define _BOOT_H_

// Entry point for boot (steps 1-999)
void boot();

// Entry point for shutdown (steps 1000-1999)
void shutdown();

//
// Add new steps to boot sequence using the following MACRO
// Example: BOOT_STEP(15, my_boot_func, "my stuff"); // call 'my_boot_func()' at step 15
//
// NOTE: Boot steps are from 1-999, shutdown steps are from 1000-1999, others steps are ignored

#define BOOT_STEP(STEP, ENTRY, DESC) \
__attribute__((section(".boot_steps"))) boot_step_t btable_##ENTRY = {.step = STEP, .entry = ENTRY, .desc = DESC}

typedef struct
{
  int step;
  void (*entry)(int);
  char *desc;
} boot_step_t;

extern boot_step_t boot_steps[]; // NOTE: Defined in linker script

#endif

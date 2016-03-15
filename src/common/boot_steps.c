/*
  Standard boot steps
  
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

#include "alt_watchdog.h"
#include "alt_cache.h"
#include "alt_interrupt.h"
#include "boot.h"

void boot_step_wdog_stop(int step);
void boot_step_icache_enable(int step);

void boot_step_alt_int_global_init(int step);
void boot_step_alt_int_cpu_init(int step);
void boot_step_alt_int_global_enable_all(int step);
void boot_step_alt_int_cpu_enable(int step);

void boot_step_alt_int_cpu_disable(int step);
void boot_step_alt_int_global_disable_all(int step);
void boot_step_alt_int_cpu_uninit(int step);
void boot_step_alt_int_global_uninit(int step);

BOOT_STEP(100, boot_step_wdog_stop, "stop watchdog");
BOOT_STEP(110, boot_step_icache_enable, "enable L1 instruction cache");

BOOT_STEP(120, boot_step_alt_int_global_init, "init global interrupt");
BOOT_STEP(130, boot_step_alt_int_cpu_init, "init cpu interrupt");
BOOT_STEP(140, boot_step_alt_int_global_enable_all, "enable global interrupts");
BOOT_STEP(150, boot_step_alt_int_cpu_enable, "enable cpu interrupts");

BOOT_STEP(1850, boot_step_alt_int_cpu_disable, "disable cpu interrupts");
BOOT_STEP(1860, boot_step_alt_int_global_disable_all, "disable global interrupts");
BOOT_STEP(1870, boot_step_alt_int_cpu_uninit, "uninit cpu interrupt");
BOOT_STEP(1880, boot_step_alt_int_global_uninit, "uninit global interrupt");

void boot_step_wdog_stop(int step)
{ alt_wdog_stop(ALT_WDOG0); }

void boot_step_icache_enable(int step)
{ alt_cache_l1_instruction_enable(); }

void boot_step_alt_int_global_init(int step)
{ alt_int_global_init(); }

void boot_step_alt_int_cpu_init(int step)
{ alt_int_cpu_init(); }

void boot_step_alt_int_global_enable_all(int step)
{ alt_int_global_enable_all(); }

void boot_step_alt_int_cpu_enable(int step)
{ alt_int_cpu_enable(); }

void boot_step_alt_int_cpu_disable(int step)
{ alt_int_cpu_disable(); }

void boot_step_alt_int_global_disable_all(int step)
{ alt_int_global_disable_all(); }

void boot_step_alt_int_cpu_uninit(int step)
{ alt_int_cpu_uninit(); }

void boot_step_alt_int_global_uninit(int step)
{ alt_int_global_uninit(); }


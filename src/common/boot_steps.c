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
void boot_step_disable_fw(int step);

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
BOOT_STEP(115, boot_step_disable_fw, "disable noc firewalls");

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

#define WRITE_REG32(ADDR, VAL) *((volatile unsigned int*) ADDR) = VAL

void boot_step_disable_fw(int step)
{
  WRITE_REG32(0xFFD13000,0x01010101);
  WRITE_REG32(0xFFD13004,0x01010101);
  WRITE_REG32(0xFFD13008,0x01010101);
  WRITE_REG32(0xFFD1300C,0x01010101);
  WRITE_REG32(0xFFD13010,0x01010101);
  WRITE_REG32(0xFFD13014,0x1);
  WRITE_REG32(0xFFD13018,0x0);
  WRITE_REG32(0xFFD1301C,0x01010101);
  WRITE_REG32(0xFFD13020,0x01010101);
  WRITE_REG32(0xFFD13024,0x01010101);
  WRITE_REG32(0xFFD13028,0x01010101);
  WRITE_REG32(0xFFD1302C,0x01010101);
  WRITE_REG32(0xFFD13030,0x01010101);
  WRITE_REG32(0xFFD13034,0x01010101);
  WRITE_REG32(0xFFD13038,0x0);
  WRITE_REG32(0xFFD1303C,0x01010101);
  WRITE_REG32(0xFFD13040,0x01010101);
  WRITE_REG32(0xFFD13044,0x01010101);
  WRITE_REG32(0xFFD13048,0x01010101);
  WRITE_REG32(0xFFD1304C,0x01010101);
  WRITE_REG32(0xFFD13050,0x01010101);
  WRITE_REG32(0xFFD13054,0x01010101);
  WRITE_REG32(0xFFD13058,0x01010101);
  WRITE_REG32(0xFFD1305C,0x01010101);
  WRITE_REG32(0xFFD13060,0x01010101);
  WRITE_REG32(0xFFD13064,0x01010101);
  WRITE_REG32(0xFFD13068,0x01010101);
  WRITE_REG32(0xFFD1306C,0x01010101);
  WRITE_REG32(0xFFD13070,0x01010101);
  WRITE_REG32(0xFFD13100,0x0);
  WRITE_REG32(0xFFD13104,0x0);
  WRITE_REG32(0xFFD13108,0x01010001);
  WRITE_REG32(0xFFD1310C,0x01010001);
  WRITE_REG32(0xFFD13110,0x01010001);
  WRITE_REG32(0xFFD13114,0x01010001);
  WRITE_REG32(0xFFD13118,0x01010001);
  WRITE_REG32(0xFFD1311C,0x01010001);
  WRITE_REG32(0xFFD13120,0x01010001);
  WRITE_REG32(0xFFD13124,0x0);
  WRITE_REG32(0xFFD13128,0x0);
  WRITE_REG32(0xFFD1312C,0x01010001);
  WRITE_REG32(0xFFD13130,0x01010001);
  WRITE_REG32(0xFFD13134,0x01010001);
  WRITE_REG32(0xFFD13138,0x01010001);
  WRITE_REG32(0xFFD1313C,0x01010001);
  WRITE_REG32(0xFFD13140,0x01010001);
  WRITE_REG32(0xFFD13144,0x01010001);
  WRITE_REG32(0xFFD13148,0x01010001);
  WRITE_REG32(0xFFD1314C,0x01010101);
  WRITE_REG32(0xFFD13150,0x01000101);
  WRITE_REG32(0xFFD13154,0x01010101);
  WRITE_REG32(0xFFD13158,0x01010101);
  WRITE_REG32(0xFFD1315C,0x01010101);
  WRITE_REG32(0xFFD13160,0x01010101);
  WRITE_REG32(0xFFD13164,0x01010101);
  WRITE_REG32(0xFFD13168,0x01010101);
  WRITE_REG32(0xFFD1316C,0x01010101);
  WRITE_REG32(0xFFD13170,0x03010101);
  WRITE_REG32(0xFFD13174,0x01000101);
  WRITE_REG32(0xFFD13178,0x01000101);
  WRITE_REG32(0xFFD1317C,0x01010101);
  WRITE_REG32(0xFFD13180,0x01010101);
  WRITE_REG32(0xFFD13184,0x01010001);
  WRITE_REG32(0xFFD13188,0x01010001);
  WRITE_REG32(0xFFD1318C,0x01010001);
  WRITE_REG32(0xFFD13190,0x01010001);
  WRITE_REG32(0xFFD13194,0x01010001);
  WRITE_REG32(0xFFD13500,0x03fe0101);
  WRITE_REG32(0xFFD13504,0x03fe0101);
  WRITE_REG32(0xFFD11000,0x7FFFBFFF);
  WRITE_REG32(0xFFD13200,0x1);
  WRITE_REG32(0xFFD1320C,0x003f0000);
  WRITE_REG32(0xFFD13210,0x0);
  WRITE_REG32(0xFFD13214,0x0);
  WRITE_REG32(0xFFD13218,0x0);
  WRITE_REG32(0xFFD1321C,0x0);
  WRITE_REG32(0xFFD13220,0x0);
  WRITE_REG32(0xFFD13300,0x1111);
  WRITE_REG32(0xFFD13310,0xffff0000);
  WRITE_REG32(0xFFD13314,0x0);
  WRITE_REG32(0xFFD13318,0x0);
  WRITE_REG32(0xFFD1331C,0x0);
  WRITE_REG32(0xFFD13320,0xffff0000);
  WRITE_REG32(0xFFD13324,0x0);
  WRITE_REG32(0xFFD13328,0x0);
  WRITE_REG32(0xFFD1332C,0x0);
  WRITE_REG32(0xFFD13330,0xffff0000);
  WRITE_REG32(0xFFD13334,0x0);
  WRITE_REG32(0xFFD13338,0x0);
  WRITE_REG32(0xFFD1333C,0x0);
  WRITE_REG32(0xFFD13340,0xffff0000);
  WRITE_REG32(0xFFD13344,0x0);
  WRITE_REG32(0xFFD13348,0x0);
  WRITE_REG32(0xFFD1334C,0x0);
  WRITE_REG32(0xFFD13400,0x1);
  WRITE_REG32(0xFFD1340C,0xffff0000);
  WRITE_REG32(0xFFD13410,0x0);
  WRITE_REG32(0xFFD13414,0x0);
  WRITE_REG32(0xFFD13418,0x0);
  WRITE_REG32(0xFFD1341C,0x0);
  WRITE_REG32(0xFFD13420,0x0);
  WRITE_REG32(0xFFD13424,0x0);
  WRITE_REG32(0xFFD13428,0x0);
  WRITE_REG32(0xFFD1342C,0x0);

  return;
}

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


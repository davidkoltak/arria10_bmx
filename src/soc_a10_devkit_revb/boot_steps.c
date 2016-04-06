/*
  Pinmux settings and basic setup for default ARRIA 10 DevKit (Rev B)
  
  by David M. Koltak  02/29/2016
  
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

#include "alt_16550_uart.h"
#include "simple_stdio.h"
#include "boot.h"

void boot_step_pinmux_init(int step);
void boot_step_stdio_init(int step);
void boot_step_print_name(int step);

BOOT_STEP(30, boot_step_pinmux_init, "configure pinmux");
BOOT_STEP(40, boot_step_stdio_init, "init stdio");
BOOT_STEP(50, boot_step_print_name, "display board identifier");

extern ALT_16550_HANDLE_t _stdio_uart_handle;

void boot_step_stdio_init(int step)
{
  alt_16550_init(ALT_16550_DEVICE_SOCFPGA_UART1, (void*)0, 0, &_stdio_uart_handle);
  alt_16550_baudrate_set(&_stdio_uart_handle, ALT_16550_BAUDRATE_115200);
  alt_16550_line_config_set(&_stdio_uart_handle, ALT_16550_DATABITS_8, ALT_16550_PARITY_DISABLE, ALT_16550_STOPBITS_1);
  alt_16550_fifo_enable(&_stdio_uart_handle);
  alt_16550_enable(&_stdio_uart_handle);
}

void boot_step_print_name(int step)
{ puts("\n *** Arria 10 SoC DevKit (Rev B) *** "); }

void boot_step_pinmux_init(int step)
{
  int *shared_q1_pinmux = (int*) 0xFFD07000;
  int *shared_q2_pinmux = (int*) 0xFFD07030;
  int *shared_q3_pinmux = (int*) 0xFFD07060;
  int *shared_q4_pinmux = (int*) 0xFFD07090;
  int *dedicated_pinmux = (int*) 0xFFD07200;
  int *dedicated_pincfg = (int*) 0xFFD07300;
  int *fpga_intf_pinmux = (int*) 0xFFD07400;

  dedicated_pinmux[4-1] = 8;
  dedicated_pinmux[5-1] = 8;
  dedicated_pinmux[6-1] = 8;
  dedicated_pinmux[7-1] = 8;
  dedicated_pinmux[8-1] = 8;
  dedicated_pinmux[9-1] = 8;
  dedicated_pinmux[10-1] = 10;
  dedicated_pinmux[11-1] = 10;
  dedicated_pinmux[12-1] = 8;
  dedicated_pinmux[13-1] = 8;
  dedicated_pinmux[14-1] = 8;
  dedicated_pinmux[15-1] = 8;
  dedicated_pinmux[16-1] = 13; // UART1 TX
  dedicated_pinmux[17-1] = 13; // UART1 RX

  dedicated_pincfg[0] = 0x00000101;
  dedicated_pincfg[1] = 0x000b080a;
  dedicated_pincfg[2] = 0x000b080a;
  dedicated_pincfg[3] = 0x000b080a;
  dedicated_pincfg[4] = 0x000a282a;
  dedicated_pincfg[5] = 0x000a282a;
  dedicated_pincfg[6] = 0x0008282a;
  dedicated_pincfg[7] = 0x000a282a;
  dedicated_pincfg[8] = 0x000a282a;
  dedicated_pincfg[9] = 0x000a282a;
  dedicated_pincfg[10] = 0x00090000;
  dedicated_pincfg[11] = 0x00090000;
  dedicated_pincfg[12] = 0x000b282a;
  dedicated_pincfg[13] = 0x000b282a;
  dedicated_pincfg[14] = 0x000b282a;
  dedicated_pincfg[15] = 0x000b282a;
  dedicated_pincfg[16] = 0x0008282a;
  dedicated_pincfg[17] = 0x000a282a;
  
  return;
}

/*
  Pinmux settings and basic setup for default ARRIA 10 DevKit (Rev A)
  
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
{ puts("\n *** Arria 10 SoC DevKit (Rev A) EMAC SWIF TEST *** "); }

void boot_step_pinmux_init(int step)
{
  int *shared_q1_pinmux = (int*) 0xFFD07000;
  int *shared_q2_pinmux = (int*) 0xFFD07030;
  int *shared_q3_pinmux = (int*) 0xFFD07060;
  int *shared_q4_pinmux = (int*) 0xFFD07090;
  int *dedicated_pinmux = (int*) 0xFFD07200;
  int *dedicated_pincfg = (int*) 0xFFD07304;
  int *fpga_intf_pinmux = (int*) 0xFFD07400;
                                
  shared_q2_pinmux[1-1] = 4; // EMAC0 RGMII
  shared_q2_pinmux[2-1] = 4;
  shared_q2_pinmux[3-1] = 4;
  shared_q2_pinmux[4-1] = 4;
  shared_q2_pinmux[5-1] = 4;
  shared_q2_pinmux[6-1] = 4;
  shared_q2_pinmux[7-1] = 4;
  shared_q2_pinmux[8-1] = 4;
  shared_q2_pinmux[9-1] = 4;
  shared_q2_pinmux[10-1] = 4;
  shared_q2_pinmux[11-1] = 4;
  shared_q2_pinmux[12-1] = 4;
  
  shared_q3_pinmux[7-1] = 13; // UART1 TX
  shared_q3_pinmux[8-1] = 13; // UART1 RX
  
  shared_q3_pinmux[11-1] = 1; // EMAC0 MDIO
  shared_q3_pinmux[12-1] = 1;

  dedicated_pinmux[4-1] = 8; // SDMMC
  dedicated_pinmux[5-1] = 8;
  dedicated_pinmux[6-1] = 8;
  
  dedicated_pincfg[4-1] = 0x000A0304; // SDMMC (config)
  dedicated_pincfg[5-1] = 0x000A0304;
  dedicated_pincfg[6-1] = 0x00080304;
  
  return;
}

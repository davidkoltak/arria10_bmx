/*
  Clock and Pinmux settings for default ARRIA 10 DevKit (Rev A)
  
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

#include "alt_clock_manager.h"
#include "alt_16550_uart.h"
#include "alt_timers.h"
#include "alt_globaltmr.h"
#include "alt_dma.h"
#include "simple_stdio.h"
#include "boot.h"

void boot_step_clock_init(int step);
void boot_step_pinmux_init(int step);
void boot_step_stdio_init(int step);
void boot_step_print_name(int step);

void boot_step_alt_dma_init(int step);
void boot_step_alt_globaltmr_init(int step);

void boot_step_alt_dma_uninit(int step);
void boot_step_alt_globaltmr_uninit(int step);

BOOT_STEP(20, boot_step_clock_init, "configure clocks");
BOOT_STEP(30, boot_step_pinmux_init, "configure pinmux");
BOOT_STEP(40, boot_step_stdio_init, "init stdio");
BOOT_STEP(50, boot_step_print_name, "display board identifier");

BOOT_STEP(220, boot_step_alt_globaltmr_init, "init global timer");
BOOT_STEP(230, boot_step_alt_dma_init, "init dma engine");

BOOT_STEP(1710, boot_step_alt_dma_uninit, "uninit dma engine");
BOOT_STEP(1720, boot_step_alt_globaltmr_uninit, "uninit global timer");

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
{ puts("\n *** Arria 10 SoC DevKit (Rev A) *** "); }

void boot_step_clock_init(int step)
{
  CLOCK_MANAGER_CONFIG config;
  CLOCK_SOURCE_CONFIG src_clks;
  
  //
  // NOTE: Settings copied from DTS
  //
  
  config.mainpll.vco0_psrc = 0;
  config.mainpll.vco1_denom = 1;
  config.mainpll.vco1_numer = 191;
  config.mainpll.mpuclk_cnt = 0;
  config.mainpll.mpuclk_src = 0;
  config.mainpll.nocclk_cnt = 0;
  config.mainpll.nocclk_src = 0;
  config.mainpll.cntr2clk_cnt = 900;
  config.mainpll.cntr3clk_cnt = 900;
  config.mainpll.cntr4clk_cnt = 900;
  config.mainpll.cntr5clk_cnt = 900;
  config.mainpll.cntr6clk_cnt = 900;
  config.mainpll.cntr7clk_cnt = 900;
  config.mainpll.cntr7clk_src = 0;
  config.mainpll.cntr8clk_cnt = 900;
  config.mainpll.cntr9clk_cnt = 900;
  config.mainpll.cntr9clk_src = 0;
  config.mainpll.cntr15clk_cnt = 900;
  config.mainpll.nocdiv_l4mainclk = 0;
  config.mainpll.nocdiv_l4mpclk = 0;
  config.mainpll.nocdiv_l4spclk = 2;
  config.mainpll.nocdiv_csatclk = 0;
  config.mainpll.nocdiv_cstraceclk = 1;
  config.mainpll.nocdiv_cspdbgclk = 1;
  
  config.perpll.vco0_psrc = 0;
  config.perpll.vco1_denom = 1;
  config.perpll.vco1_numer = 159;
  config.perpll.cntr2clk_cnt = 7;
  config.perpll.cntr2clk_src = 1;
  config.perpll.cntr3clk_cnt = 900;
  config.perpll.cntr3clk_src = 1;
  config.perpll.cntr4clk_cnt = 19;
  config.perpll.cntr4clk_src = 1;
  config.perpll.cntr5clk_cnt = 499;
  config.perpll.cntr5clk_src = 1;
  config.perpll.cntr6clk_cnt = 9;
  config.perpll.cntr6clk_src = 1;
  config.perpll.cntr7clk_cnt = 900;
  config.perpll.cntr8clk_cnt = 900;
  config.perpll.cntr8clk_src = 0;
  config.perpll.cntr9clk_cnt = 900;
  config.perpll.emacctl_emac0sel = 0;
  config.perpll.emacctl_emac1sel = 0;
  config.perpll.emacctl_emac2sel = 0;
  config.perpll.gpiodiv_gpiodbclk = 32000;
   
  config.alteragrp.nocclk = 0x0384000b;
  
  src_clks.clk_freq_of_eosc1 = 25000000;
  src_clks.clk_freq_of_f2h_free = 100000000;
  src_clks.clk_freq_of_cb_intosc_ls = 100000000;

  alt_clkmgr_config(&config, &src_clks);
  
  return;
}

void boot_step_pinmux_init(int step)
{
  int *shared_q1_pinmux = (int*) 0xFFD07000;
  int *shared_q2_pinmux = (int*) 0xFFD07030;
  int *shared_q3_pinmux = (int*) 0xFFD07060;
  int *shared_q4_pinmux = (int*) 0xFFD07090;
  int *dedicated_pinmux = (int*) 0xFFD07200;
  int *dedicated_pincfg = (int*) 0xFFD07300;
  int *fpga_intf_pinmux = (int*) 0xFFD07400;
  
  shared_q3_pinmux[7-1] = 13; // UART1 TX
  shared_q3_pinmux[8-1] = 13; // UART1 RX
    
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
  dedicated_pinmux[16-1] = 13;
  dedicated_pinmux[17-1] = 13;

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

void boot_step_alt_globaltmr_init(int step)
{ alt_globaltmr_init(); }

void boot_step_alt_dma_init(int step)
{ 
  ALT_DMA_CFG_t dma_cfg;
  int i;
  
  dma_cfg.manager_sec = ALT_DMA_SECURITY_SECURE;
  
  for (i = 0; i < (sizeof(dma_cfg.irq_sec) / sizeof(dma_cfg.irq_sec[0])); ++i)
    dma_cfg.irq_sec[i] = ALT_DMA_SECURITY_SECURE;       
      
  for (i = 0; i < (sizeof(dma_cfg.periph_sec) / sizeof(dma_cfg.periph_sec[0])); ++i)
    dma_cfg.periph_sec[i] = ALT_DMA_SECURITY_SECURE;
    
  for (i = 0; i < (sizeof(dma_cfg.periph_mux) / sizeof(dma_cfg.periph_mux[0])); ++i)
    dma_cfg.periph_mux[i] = ALT_DMA_PERIPH_MUX_DEFAULT;
    
  alt_dma_init(&dma_cfg);
}

void boot_step_alt_dma_uninit(int step)
{ alt_dma_uninit(); }

void boot_step_alt_globaltmr_uninit(int step)
{ alt_globaltmr_uninit(); }

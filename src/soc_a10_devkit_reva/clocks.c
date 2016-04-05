/*
  Clock Manager commands for Arria 10 SoC

  by David M. Koltak  04/05/2016

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
#include "terminal.h"
#include "boot.h"
#include "simple_stdio.h"
#include <string.h>

void clock_init(int step);

BOOT_STEP(20, clock_init, "configure clocks");

int clock_get(int argc, char** argv);
int clock_ctrl(int argc, char** argv);

TERMINAL_COMMAND("clock-get", clock_get, "{clock|all}");
TERMINAL_COMMAND("clock-ctrl", clock_ctrl, "{enable|disable} {clock}");

//
// Initialize Clocks
//

CLOCK_MANAGER_CONFIG clock_config;
CLOCK_SOURCE_CONFIG clock_src_clks;
  
void clock_init(int step)
{
  //
  // NOTE: Settings copied from DTS
  //
  
  clock_config.mainpll.vco0_psrc = 0;
  clock_config.mainpll.vco1_denom = 1;
  clock_config.mainpll.vco1_numer = 191;
  clock_config.mainpll.mpuclk_cnt = 0;
  clock_config.mainpll.mpuclk_src = 0;
  clock_config.mainpll.nocclk_cnt = 0;
  clock_config.mainpll.nocclk_src = 0;
  clock_config.mainpll.cntr2clk_cnt = 900;
  clock_config.mainpll.cntr3clk_cnt = 900;
  clock_config.mainpll.cntr4clk_cnt = 900;
  clock_config.mainpll.cntr5clk_cnt = 900;
  clock_config.mainpll.cntr6clk_cnt = 900;
  clock_config.mainpll.cntr7clk_cnt = 900;
  clock_config.mainpll.cntr7clk_src = 0;
  clock_config.mainpll.cntr8clk_cnt = 900;
  clock_config.mainpll.cntr9clk_cnt = 900;
  clock_config.mainpll.cntr9clk_src = 0;
  clock_config.mainpll.cntr15clk_cnt = 900;
  clock_config.mainpll.nocdiv_l4mainclk = 0;
  clock_config.mainpll.nocdiv_l4mpclk = 0;
  clock_config.mainpll.nocdiv_l4spclk = 2;
  clock_config.mainpll.nocdiv_csatclk = 0;
  clock_config.mainpll.nocdiv_cstraceclk = 1;
  clock_config.mainpll.nocdiv_cspdbgclk = 1;
  
  clock_config.perpll.vco0_psrc = 0;
  clock_config.perpll.vco1_denom = 1;
  clock_config.perpll.vco1_numer = 159;
  clock_config.perpll.cntr2clk_cnt = 7;
  clock_config.perpll.cntr2clk_src = 1;
  clock_config.perpll.cntr3clk_cnt = 900;
  clock_config.perpll.cntr3clk_src = 1;
  clock_config.perpll.cntr4clk_cnt = 19;
  clock_config.perpll.cntr4clk_src = 1;
  clock_config.perpll.cntr5clk_cnt = 499;
  clock_config.perpll.cntr5clk_src = 1;
  clock_config.perpll.cntr6clk_cnt = 9;
  clock_config.perpll.cntr6clk_src = 1;
  clock_config.perpll.cntr7clk_cnt = 900;
  clock_config.perpll.cntr8clk_cnt = 900;
  clock_config.perpll.cntr8clk_src = 0;
  clock_config.perpll.cntr9clk_cnt = 900;
  clock_config.perpll.emacctl_emac0sel = 0;
  clock_config.perpll.emacctl_emac1sel = 0;
  clock_config.perpll.emacctl_emac2sel = 0;
  clock_config.perpll.gpiodiv_gpiodbclk = 32000;
   
  clock_config.alteragrp.nocclk = 0x0384000b;
  
  clock_src_clks.clk_freq_of_eosc1 = 25000000;
  clock_src_clks.clk_freq_of_f2h_free = 100000000;
  clock_src_clks.clk_freq_of_cb_intosc_ls = 100000000;

  alt_clkmgr_config(&clock_config, &clock_src_clks);
  
  return;
}

//
// Clock Helper Functions
//

struct {char* name; ALT_CLK_t clk;} clock_names[] = 
{
  {"osc1", ALT_CLK_OSC1},
  {"f2s_free_clk", ALT_CLK_F2S_FREE_CLK},
  {"intosc_hs_div2", ALT_CLK_INTOSC_HS_DIV2},
  {"main_pll", ALT_CLK_MAIN_PLL},
  {"peripheral_pll", ALT_CLK_PERIPHERAL_PLL},
  {"mpu", ALT_CLK_MPU},
  {"mpu_periph", ALT_CLK_MPU_PERIPH},
  {"l2_ram", ALT_CLK_MPU_L2_RAM},
  {"l3_main", ALT_CLK_L3_MAIN},
  {"l4_sys_free", ALT_CLK_L4_SYS_FREE},
  {"l4_main", ALT_CLK_L4_MAIN},
  {"l4_mp", ALT_CLK_L4_MP},
  {"l4_sp", ALT_CLK_L4_SP},
  {"cs_timer", ALT_CLK_CS_TIMER},
  {"cs_at", ALT_CLK_CS_AT},
  {"cs_pdbg", ALT_CLK_CS_PDBG},
  {"cs_trace", ALT_CLK_CS_TRACE},
  {"s2f_user0", ALT_CLK_S2F_USER0},
  {"s2f_user1", ALT_CLK_S2F_USER1},
  {"hmc_pll_ref", ALT_CLK_HMC_PLL_REF},
  {"emac0", ALT_CLK_EMAC0},
  {"emac1", ALT_CLK_EMAC1},
  {"emac2", ALT_CLK_EMAC2},
  {"emac_ptp", ALT_CLK_EMAC_PTP},
  {"gpio", ALT_CLK_GPIO},
  {"sdmmc", ALT_CLK_SDMMC},
  {"usb", ALT_CLK_USB},
  {"spim", ALT_CLK_SPIM},
  {"nand", ALT_CLK_NAND},
  {"qspi", ALT_CLK_QSPI},
  {(char*)0, -1}
};
 
//
// Clock Terminal Commands
//

int clock_get(int argc, char** argv)
{
  int x;
  int all_true;
  ALT_CLK_t clk;
  unsigned int freq;
  char *stat;
  
  if (argc != 2)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }
  
  if (strcmp("all", argv[1]) == 0)
    all_true = 1;
  else
    all_true = 0;
 
  x = 0;
  while (clock_names[x].name != (char*)0)
  {
    if ((all_true) || (strcmp(argv[1], clock_names[x].name) == 0))
    {
      clk = clock_names[x].clk;
      alt_clk_freq_get(clk, (uint32_t*) &freq);
      
      if (alt_clk_is_enabled(clk) == ALT_E_TRUE)
        stat = "enabled";
      else
        stat = "disabled";
       
      printf("%-20s : %-12u Hz (%s)\n", freq * 1000, stat);
    }
    
    x++;
  }
  
  return 0;
}

int clock_ctrl(int argc, char** argv)
{
  int x;
  
  if (argc != 3)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }
  
  x = 0;
  while (clock_names[x].name != (char*)0)
  {
    if (strcmp(argv[2], clock_names[x].name) == 0)
    {
      if (strcmp("enable", argv[1]) == 0)
        alt_clk_clock_enable(clock_names[x].clk);
      else if (strcmp("disable", argv[1]) == 0)
        alt_clk_clock_disable(clock_names[x].clk);
      else      
        puts("ERROR: First argument must be enable|disable");
      
      return 0;
    }
    
    x++;
  }
  
  printf("ERROR: Clock '%s' not found\n", argv[2]);
  return -1;
}

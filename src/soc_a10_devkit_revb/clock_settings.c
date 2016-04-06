/*
  Clock Manager Settings Structures for Arria 10 SoC

  by David M. Koltak  04/06/2016

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

//
// NOTE: Settings copied from DTS
//
  
CLOCK_MANAGER_CONFIG clock_config =
{
  .mainpll = 
  {
    .vco0_psrc = 0,
    .vco1_denom = 1,
    .vco1_numer = 191,
    .mpuclk_cnt = 0,
    .mpuclk_src = 0,
    .nocclk_cnt = 0,
    .nocclk_src = 0,
    .cntr2clk_cnt = 900,
    .cntr3clk_cnt = 900,
    .cntr4clk_cnt = 900,
    .cntr5clk_cnt = 900,
    .cntr6clk_cnt = 900,
    .cntr7clk_cnt = 900,
    .cntr7clk_src = 0,
    .cntr8clk_cnt = 900,
    .cntr9clk_cnt = 900,
    .cntr9clk_src = 0,
    .cntr15clk_cnt = 900,
    .nocdiv_l4mainclk = 0,
    .nocdiv_l4mpclk = 0,
    .nocdiv_l4spclk = 2,
    .nocdiv_csatclk = 0,
    .nocdiv_cstraceclk = 1,
    .nocdiv_cspdbgclk = 1
  },
  
  .perpll = 
  {
    .vco0_psrc = 0,
    .vco1_denom = 1,
    .vco1_numer = 159,
    .cntr2clk_cnt = 7,
    .cntr2clk_src = 1,
    .cntr3clk_cnt = 900,
    .cntr3clk_src = 1,
    .cntr4clk_cnt = 19,
    .cntr4clk_src = 1,
    .cntr5clk_cnt = 499,
    .cntr5clk_src = 1,
    .cntr6clk_cnt = 9,
    .cntr6clk_src = 1,
    .cntr7clk_cnt = 900,
    .cntr8clk_cnt = 900,
    .cntr8clk_src = 0,
    .cntr9clk_cnt = 900,
    .emacctl_emac0sel = 0,
    .emacctl_emac1sel = 0,
    .emacctl_emac2sel = 0,
    .gpiodiv_gpiodbclk = 32000
  },
   
  .alteragrp = { .nocclk = 0x0384000b }
};

CLOCK_SOURCE_CONFIG clock_src_clks =
{
  .clk_freq_of_eosc1 = 25000000,
  .clk_freq_of_f2h_free = 100000000,
  .clk_freq_of_cb_intosc_ls = 100000000
};



/*
  EMAC register definitions

  by David M. Koltak  03/17/2016

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

#ifndef _EMAC_REGS_H_
#define _EMAC_REGS_H_

#define EMAC0_BASE 0xFF800000
#define EMAC1_BASE 0xFF802000
#define EMAC2_BASE 0xFF804000

typedef volatile struct
{
  unsigned int mac_config;
  unsigned int mac_frame_filter;
  int RSVD_008[2];
  unsigned int gmii_addr;
  unsigned int gmii_data;
  unsigned int flow_control;
  unsigned int vlan_tag;
  unsigned int version;
  unsigned int debug;
  int RSVD_028[2];
  unsigned int lpi_control_status;
  unsigned int lpi_timers_control;
  unsigned int interrupt_status;
  unsigned int interrupt_mask;
  
  struct
  {
    unsigned int high;
    unsigned int low;
  } mac_address[16];
  
  int RSVD_0C0[6];
  unsigned int sg_rg_smii_control_status;
  unsigned int wdog_timeout;
  unsigned int genpio;
  int RSVD_0E4[7];
  unsigned int mmc_control;
  unsigned int mmc_receive_interrupt;
  unsigned int mmc_transmit_interrupt;
  unsigned int mmc_receive_interrupt_mask;
  unsigned int mmc_transmit_interrupt_mask;
  // ... and many, many more!
} emac_regs_t;

#endif

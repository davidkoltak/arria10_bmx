/*
  EMAC0 Switch Interface Test for Arria 10 SoC

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

#include "terminal.h"
#include "boot.h"
#include "simple_stdio.h"
#include <string.h>

#include "emac_regs.h"
#define PHY_ADDR 7

void emac0_swif_init(int step);

BOOT_STEP(310, emac0_swif_init, "init emac0 switch interface");

int emac0_swif_test(int argc, char** argv);

TERMINAL_COMMAND("emac0-swif-test", emac0_swif_test, "Send packet through loopbacked switch interface");

//
// Initialization Boot Step
//

unsigned int emac_swif_phy_read(emac_regs_t *emac, int reg)
{
  volatile int tmr;
  int rd_timeout = 0x10000;
  
  if (emac->gmii_addr & 1)
  {
    printf("ERROR: PHY busy before reading reg 0x%08x\n", reg);
    return 0;
  }

  emac->gmii_addr = ((PHY_ADDR & 7) << 11) | ((reg & 0x1F) << 6) | (0 << 2) | (0 << 1) | (1 << 0);

  for (tmr = 0; tmr < rd_timeout; tmr++)
    if ((emac->gmii_addr & 1) == 0)
      break;
  
  if (tmr >= rd_timeout)
  {
    printf("ERROR: Timeout reading phy reg 0x%08x\n", reg);
    return 0;
  }
  
  return emac->gmii_data;
}

void emac_swif_phy_write(emac_regs_t *emac, int reg, unsigned int data)
{
  volatile int tmr;
  int rd_timeout = 0x10000;
  
  if (emac->gmii_addr & 1)
  {
    printf("ERROR: PHY busy before writing reg 0x%08x\n", reg);
    return;
  }

  emac->gmii_data = data;
  emac->gmii_addr = ((PHY_ADDR & 7) << 11) | ((reg & 0x1F) << 6) | (0 << 2) | (1 << 1) | (1 << 0);
  
  for (tmr = 0; tmr < rd_timeout; tmr++)
    if ((emac->gmii_addr & 1) == 0)
      break;
  
  if (tmr >= rd_timeout)
  {
    printf("ERROR: Timeout writing phy reg 0x%08x\n", reg);
    return;
  }
  
  return;
}

void emac0_swif_init(int step)
{
  int *brgmodrst = (int*) 0xFFD0502C;
  int *per0modrst = (int*) 0xFFD05024;
  int *sysmgr_emac0 = (int*) 0xFFD06044;
  int *sysmgr_fpgaintf_en_3 = (int*) 0xFFD06070;
  
  emac_regs_t *emac0 = (emac_regs_t*) EMAC0_BASE;
  
  *brgmodrst &= ~(1 << 1);
  *per0modrst &= ~(1 << 0);
  
  *sysmgr_emac0 &= ~((1 << 12) | (3 << 0));
  *sysmgr_emac0 |= ((1 << 31) | (0 << 12) | (1 << 0));
  
  *sysmgr_fpgaintf_en_3 |= (1 << 4) | (1 << 0);
  
  printf("ID1: 0x%08X\n", emac_swif_phy_read(emac0, 2));
  printf("ID2: 0x%08X\n", emac_swif_phy_read(emac0, 3));

  emac0->mac_address[0].high = 0x80003456;
  emac0->mac_address[0].low = 0x7654321;
  emac0->mac_frame_filter = (1 << 31) | (1 << 0);
  emac0->mac_config = 0x0000090C;
  emac0->mac_config |= (1 << 12); // MAC Loopback
      
  return;
}

//
// Terminal Commands
//

// Max TEST_SZ of 1022 (32-bit words)
#define TEST_SZ 50

int emac0_swif_test(int argc, char** argv)
{
  volatile int *tx_ctl = (volatile int*) 0xFF200000;
  volatile int *tx_sts = (volatile int*) 0xFF200004;
  volatile int *tx_dat = (volatile int*) 0xFF200008;
  volatile int *rx_ctl = (volatile int*) 0xFF201000;
  volatile int *rx_sts = (volatile int*) 0xFF201004;
  volatile int *rx_dat = (volatile int*) 0xFF201008;
  int test_num;
  int tmp;
  int x;
  
  for (test_num = 1; test_num < 3; test_num++)
  {
    //
    // Initialize buffers
    //

    for (x = 0; x < TEST_SZ; x++)
      tx_dat[x] = (x < 3) ? 0xFFFFFFFF : (x * 3) | (test_num << 16);

    //
    // Arm receive data capture
    //

    *rx_ctl = (1 << 31); // Flush
    *rx_ctl = 1;

    //
    // Send data
    //

    //puts("INFO: Sending packet");
    *tx_ctl = (0xC << 28) | (3 << 12) | ((TEST_SZ-1) << 2) | (1 << 0);

    while (*tx_ctl & 1) ;
    *tx_ctl = 0;
    //puts("INFO: Packet Sent");

    //
    // Wait for packet receive to finish
    //

    while (*rx_ctl & 1) ;
    *rx_ctl = 0;
    //puts("INFO: Packet Received");

    //
    // Check data
    //

    //puts("INFO: Checking results");
    for (x = 0; x < TEST_SZ; x++)
    {
      tmp = (x < 3) ? 0xFFFFFFFF : (x * 3) | (test_num << 16); // NOTE: 'tx_dat' is write-only

      if (tmp != rx_dat[x])
        printf("MISMATCH: Sent 0x%08X, Received 0x%08X, Offset 0x%08X\n", tmp, rx_dat[x], x);
    }

    printf(" *** TEST %i COMPLETE ***\n", test_num);
  }
  
  return 0;
}



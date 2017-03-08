/*
  I2C Bus interface commands for Arria 10 SoC

  by David M. Koltak  05/12/2016

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

#include "alt_i2c.h"
#include "terminal.h"
#include "boot.h"
#include "simple_stdio.h"
#include <string.h>

#define I2C_DEVICE ALT_I2C_I2C1
#define I2C_SPEED 40000

ALT_I2C_DEV_t i2c_dev;

void i2c_init(int step)
{
  ALT_STATUS_CODE status;
  ALT_I2C_MASTER_CONFIG_t cfg;
  uint32_t speed;
    
  puts("\nINFO: Setting up I2C Bus");
  status = alt_i2c_init(I2C_DEVICE, &i2c_dev);

  if (status == ALT_E_SUCCESS)
    status = alt_i2c_enable(&i2c_dev);

  if (status == ALT_E_SUCCESS)
    status = alt_i2c_master_config_get(&i2c_dev, &cfg);
        
  if (status == ALT_E_SUCCESS)
    status = alt_i2c_master_config_speed_set(&i2c_dev, &cfg, I2C_SPEED);
 
  if (status == ALT_E_SUCCESS)
    status = alt_i2c_master_config_speed_get(&i2c_dev, &cfg, &speed);

  if(status == ALT_E_SUCCESS)
  {
    cfg.addr_mode = ALT_I2C_ADDR_MODE_7_BIT;
    cfg.restart_enable = ALT_E_TRUE;
    cfg.fs_spklen = 2;
    status = alt_i2c_master_config_set(&i2c_dev, &cfg);
  }

  if (status != ALT_E_SUCCESS)
  { puts("ERROR: I2C Init FAILED"); }
  else
  { puts("INFO: I2C Init SUCCESS"); }
  
  return;
}

int i2c_receive(int argc, char** argv)
{
  ALT_STATUS_CODE status;
  unsigned int chip;
  unsigned int addr;
  unsigned int cnt;
  int x;
  char buf[256];
  
  //
  // Parse Args
  //
  
  if (argc != 3)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }

  if (sscanf(argv[1], "%u", &chip) != 1)
  {
    puts("ERROR: First argument must be an unsigned number for chip address");
    return -2;
  }
  
  if (sscanf(argv[2], "%u", &cnt) != 1)
  {
    puts("ERROR: Second argument must be an unsigned number for byte count");
    return -3;
  }
  
  if (cnt > 256)
  {
    puts("ERROR: Count must be < 256");
    return -4;
  }
  
  //
  // Read into a buffer
  //

  status = alt_i2c_master_target_set(&i2c_dev, chip);
  
  if (status != ALT_E_SUCCESS)
  {
    puts("ERROR: Unable to set target address");
    return -5;
  }

  status = alt_i2c_master_receive(&i2c_dev, buf, cnt, ALT_E_FALSE, ALT_E_TRUE);
  
  if (status != ALT_E_SUCCESS)
  {
    puts("ERROR: Unable to perform master receive operation");
    return -5;
  }
                            
  //
  // Display from buffer
  //
  
  for (x = 0; x < cnt; x++)
  {
    if ((x & 0xF) == 0)
      printf("\n %02X : ", x);
    
    printf(" %02x", ((int) buf[x]) & 0xFF);
  }
  
  printf("\n");
  
  return 0;
}

int i2c_transmit(int argc, char** argv)
{
  ALT_STATUS_CODE status;
  unsigned int chip;
  unsigned int addr;
  unsigned int cnt;
  int x;
  char buf[256];
  
  //
  // Parse Args
  //
  
  if (argc < 3)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }

  if (sscanf(argv[1], "%u", &chip) != 1)
  {
    puts("ERROR: First argument must be an unsigned number");
    return -2;
  }
  
  for (cnt = 0; cnt < (argc - 2); cnt++)
  {
    if (cnt > 256)
      break;

    if (sscanf(argv[cnt+2], "%u", &(buf[cnt])) != 1)
    {
      puts("ERROR: Byte argument must be an unsigned number");
      return -3;
    }
  }
  
  //
  // Write from a buffer
  //

  status = alt_i2c_master_target_set(&i2c_dev, chip);
  
  if (status != ALT_E_SUCCESS)
  {
    puts("ERROR: Unable to set target address");
    return -5;
  }

  status = alt_i2c_master_transmit(&i2c_dev, buf, cnt, ALT_E_FALSE, ALT_E_TRUE);
  
  if (status != ALT_E_SUCCESS)
  {
    puts("ERROR: Unable to perform master transmit operation");
    return -5;
  }
                            
  //
  // Display from buffer
  //
  
  for (x = 0; x < cnt; x++)
  {
    if ((x & 0xF) == 0)
      printf("\n %02X : ", x);
    
    printf(" %02x", ((int) buf[x]) & 0xFF);
  }
  
  printf("\n");
  
  return 0;
}


BOOT_STEP(301, i2c_init, "init i2c bus");

TERMINAL_COMMAND("i2c-rx", i2c_receive, "{chip} {count}");
TERMINAL_COMMAND("i2c-tx", i2c_transmit, "{chip} {byte} ...");


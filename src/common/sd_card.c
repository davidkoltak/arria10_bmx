/*
  SD Card interface commands for Arria 10 SoC

  by David M. Koltak  03/15/2016

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

#include "alt_sdmmc.h"
#include "terminal.h"
#include "boot.h"
#include "simple_stdio.h"
#include <string.h>

void sd_card_init(int step);
void sd_card_default_rbf(int step);

BOOT_STEP(300, sd_card_init, "init sdmmc card");
BOOT_STEP(301, sd_card_default_rbf, "load 'default.rbf' from sdmmc card");

int sd_parts(int argc, char** argv);
int sd_files(int argc, char** argv);
int sd_dump(int argc, char** argv);
int sd_rbf(int argc, char** argv);

TERMINAL_COMMAND("sd-parts", sd_parts, "Show SD Card Partitons");
TERMINAL_COMMAND("sd-files", sd_files, "Show SD Card Files appended to PImage in A2 Partition");
TERMINAL_COMMAND("sd-dump", sd_dump, "{sector bytes | filename}");
TERMINAL_COMMAND("sd-rbf", sd_rbf, "{filename}");

//
// Card Initialization Boot Step
//

ALT_SDMMC_CARD_INFO_t sd_card_info;
ALT_SDMMC_CARD_MISC_t sd_card_misc_cfg;
uint32_t sd_card_block_size;
uint32_t sd_card_size;

void sd_card_init(int step)
{
  ALT_STATUS_CODE status;

  // Setting up SD/MMC
  
  puts("\nINFO: Setting up SDMMC");
  status = alt_sdmmc_init();

  if (status == ALT_E_SUCCESS)
  { status = alt_sdmmc_card_pwr_on(); }
  
  if (status == ALT_E_SUCCESS)
  { status = alt_sdmmc_card_identify(&sd_card_info); }

  if (status == ALT_E_SUCCESS)
  {
    switch(sd_card_info.card_type)
    {
    case ALT_SDMMC_CARD_TYPE_MMC:
      puts("INFO: MMC Card detected");
      break;
    case ALT_SDMMC_CARD_TYPE_SD:
      puts("INFO: SD Card detected");
      break;
    case ALT_SDMMC_CARD_TYPE_SDIOIO:
      puts("INFO: SDIO Card detected");
      break;
    case ALT_SDMMC_CARD_TYPE_SDIOCOMBO:
      puts("INFO: SDIOCOMBO Card detected");
      break;
    case ALT_SDMMC_CARD_TYPE_SDHC:
      puts("INFO: SDHC Card detected");
      break;
    default:
      puts("INFO: Card type unknown");
      status = ALT_E_ERROR;
      break;
    }
  }

  if (status == ALT_E_SUCCESS)
  { status = alt_sdmmc_card_bus_width_set(&sd_card_info, ALT_SDMMC_BUS_WIDTH_4); }

  if (status == ALT_E_SUCCESS)
  {
    alt_sdmmc_fifo_param_set((ALT_SDMMC_FIFO_NUM_ENTRIES >> 3) - 1,
      (ALT_SDMMC_FIFO_NUM_ENTRIES >> 3), ALT_SDMMC_MULT_TRANS_TXMSIZE1);
  }

  if (status == ALT_E_SUCCESS)
  { alt_sdmmc_card_misc_get(&sd_card_misc_cfg); }

  if (status == ALT_E_SUCCESS)
  {
    printf("INFO: Card width = %d\n", sd_card_misc_cfg.card_width);
    printf("INFO: Card block size = %d\n", (int)sd_card_misc_cfg.block_size);
    sd_card_block_size = sd_card_misc_cfg.block_size;
    sd_card_size = (sd_card_info.blk_number_low >> 10) + (sd_card_info.blk_number_high << 22);
    sd_card_size *= sd_card_block_size;
    printf("INFO: Card size = %d MB\n", (sd_card_size >> 10));
  }

  if (status == ALT_E_SUCCESS)
  { status = alt_sdmmc_dma_enable(); }

  if (status == ALT_E_SUCCESS)
  { status = alt_sdmmc_card_clk_div_set(0x00000002); }
  
  if (sd_card_block_size != 512)
  { printf("WARNING: SD Card with blocksize %i is not supported - yet", sd_card_block_size); }
  
  if (status != ALT_E_SUCCESS)
  { puts("ERROR: SD Card Init FAILED"); }
  else
  { puts("INFO: SD Card Init SUCCESS"); }
  
  return;
}

int sd_load_rbf(char *filename);

void sd_card_default_rbf(int step)
{
  int rtn;
  
  puts("INFO: Loading 'default.rbf'");
  flush();
  
  rtn = sd_load_rbf("default.rbf");
  
  if (rtn == 0)
    puts("   SUCCESS");
  else if (rtn == -1)
    puts("ERROR: File not found");
  else
    printf("ERROR: Error Code (%i)\n", rtn);
}

//
// SD Helper Functions
//

struct
{
  struct
  {
    char type;
    unsigned int start;
    unsigned int size;
  } p[4];
} sd_parts_list;

int sd_load_parts()
{
  ALT_STATUS_CODE status;
  int x;
  int y = 0x1BE;
  unsigned int tmp;
  char buf[512];

  for (x = 0; x < 4; x++)
    sd_parts_list.p[x].type = 0;
    
  status = alt_sdmmc_read(&sd_card_info, &buf, (void*)0, 512); // MBR
  
  if (status != ALT_E_SUCCESS)
    return -1;
    
  if ((buf[510] != 0x55) || (buf[511] != 0xAA))
    return -2;

  for (x = 0; x < 4; x++)
  {
    sd_parts_list.p[x].type = buf[y + 4];

    tmp = buf[y + 11];
    tmp = ((tmp & 0xFF) << 8) + buf[y + 10];
    tmp = ((tmp & 0xFFFF) << 8) + buf[y + 9];
    tmp = ((tmp & 0xFFFFF) << 8) + buf[y + 8];
    sd_parts_list.p[x].start = tmp;

    tmp = buf[y + 15];
    tmp = ((tmp & 0xFF) << 8) + buf[y + 14];
    tmp = ((tmp & 0xFFFF) << 8) + buf[y + 13];
    tmp = ((tmp & 0xFFFFF) << 8) + buf[y + 12];
    sd_parts_list.p[x].size = tmp;

    y += 16;
  }
   
  return 0;
}

int sd_find_file(char *filename, int *sector, int *bytes)
{
  ALT_STATUS_CODE status;
  int x;
  char buf[512];
  char fname[64];
  unsigned int fsize;
  char *str;
  
  if (sd_load_parts())
    return -1;
  
  for (x = 0; x < 4; x++)
  {
    if (sd_parts_list.p[x].type == 0xA2)
    {
      status = alt_sdmmc_read(&sd_card_info, buf, (void*)((sd_parts_list.p[x].start + 0x800) * 512), 512);

      if (status == ALT_E_SUCCESS)
      {
        if ((buf[0] == '>') && (buf[1] == ' ') && (buf[511] == '\0'))
        {
          *sector = (sd_parts_list.p[x].start + 0x801);
          str = buf;
          while (*str != '\0')
          {
            if (sscanf(str, "> %s [%u]", fname, &fsize) == 2)
            {
              if (strcmp(fname, filename) == 0)
              {
                *bytes = fsize;
                return 0;
              }
              
              if (fsize & 0x1FF)
                fsize = (fsize >> 9) + 1;
              else
                fsize = (fsize >> 9);

              *sector += fsize;
            }
            else
              return -1;
            
            str++;
            while ((*str != '\0') && (*str != '>'))
              str++;
          }
        }
      }

      return -1;
    }
  }
  
  return -1;
}

int sd_load_rbf(char *filename)
{
  ALT_STATUS_CODE status;
  int sector;
  int bytes;
  int level;
  int x;
  unsigned int buf[1024]; 
  volatile unsigned int *fpgamgr_ctrl_0 = (volatile unsigned int*) 0xFFD03070;
  volatile unsigned int *fpgamgr_ctrl_1= (volatile unsigned int*) 0xFFD03074;
  volatile unsigned int *fpgamgr_ctrl_2 = (volatile unsigned int*) 0xFFD03078;
  volatile unsigned int *fpgamgr_stat = (volatile unsigned int*) 0xFFD03080;
  volatile unsigned int *fpgamgr_fsta = (volatile unsigned int*) 0xFFD03094;
  volatile unsigned int *fpgamgr_imag = (volatile unsigned int*) 0xFFCFE400;
  
  if (sd_find_file(filename, &sector, &bytes))
    return -1;

  *fpgamgr_ctrl_0 = 0x00000106;
  *fpgamgr_ctrl_1 = 0x00000000;
  *fpgamgr_ctrl_2 = 0x01000001;
  
  *fpgamgr_ctrl_0 = 0x00000006;
  while ((*fpgamgr_stat & 0x0000000E) != 0) ;
  *fpgamgr_ctrl_0 = 0x00000106;  
  while ((*fpgamgr_stat & 0x000000A0) != 0x00000080) ;
  
  while (bytes > 0)
  {
  
    status = alt_sdmmc_read(&sd_card_info, (char*)buf, (void*)(sector * 512), (4 * 1024));
    
    if (status != ALT_E_SUCCESS)
      return -2;
      
    sector += 8;
    
    level = *fpgamgr_fsta & 0xFF;
    
    for (x = 0; x < (1024); x++)
    {
      while (level > 63) 
        level = *fpgamgr_fsta & 0xFF;
      
      *fpgamgr_imag = buf[x];
 
      level++;
      bytes -= 4;
      
      if (bytes <= 0)
        break;
    }
    
    if (*fpgamgr_stat & 0x00000020)
      return -3;
  }
  
  while (*fpgamgr_stat & 0x00000080) ;
  while ((*fpgamgr_stat & 0x00000004) == 0) ;
  
  *fpgamgr_ctrl_0 = 0x00000107;
  *fpgamgr_ctrl_1 = 0x01000001;
  *fpgamgr_ctrl_2 = 0x01000000;
    
  return 0;
}


//
// SD Card Terminal Commands
//

int sd_parts(int argc, char** argv)
{
  int x;
  
  if (sd_load_parts())
  {
    puts("ERROR: Unable to parse MBR");
    return -1;
  }
  
  for (x = 0; x < 4; x++)
  {
    printf("---\nPartition # %i:\n", (x + 1));
    printf("         type = %02X\n", (unsigned int) sd_parts_list.p[x].type & 0xFF);
    printf("        start = %08X\n", (unsigned int) sd_parts_list.p[x].start);
    printf("         size = %08X\n", (unsigned int) sd_parts_list.p[x].size);
  }
  
  return 0;
}

int sd_files(int argc, char** argv)
{
  ALT_STATUS_CODE status;
  int x;
  char buf[512];
  
  if (sd_load_parts())
  {
    puts("ERROR: Unable to parse MBR");
    return -1;
  }
  
  for (x = 0; x < 4; x++)
  {
    if (sd_parts_list.p[x].type == 0xA2)
    {
      status = alt_sdmmc_read(&sd_card_info, buf, (void*)((sd_parts_list.p[x].start + 0x800) * 512), 512);

      if (status == ALT_E_SUCCESS)
      {
        if ((buf[0] == '>') && (buf[1] == ' ') && (buf[511] == '\0'))
        {
          printf("\n%s", buf);
          return 0;
        }
      }

      puts("ERROR: Unable to read appended data header");
      return -1;
    }
  }
  
  return -1;
}

int sd_dump(int argc, char** argv)
{
  ALT_STATUS_CODE status;
  int sector;
  int bytes;
  int x;
  int offset;
  char estr[17];
  char buf[512];
  
  if (argc == 2)
  {
    if (sd_find_file(argv[1], &sector, &bytes))
    {
      printf("ERROR: Did not find file '%s'\n", argv[1]);
      return -1;
    }
  }
  else if (argc == 3)
  {  
    if (sscanf(argv[1], "%i", &sector) != 1)
    {
      printf("ERROR: Argument 1 must be a number");
      return -2;
    }

    if (sscanf(argv[2], "%i", &bytes) != 1)
    {
      printf("ERROR: Argument 2 must be a number");
      return -3;
    } 
  }
  else
  {
    puts("ERROR: Wrong number of arguments");
    return -4;
  }
  
  estr[16] = '\0';
  offset = 0;
  printf(" %08X : ", offset);
  while (bytes > 0)
  {
    status = alt_sdmmc_read(&sd_card_info, buf, (void*)(sector * 512), 512);
    sector++;
    
    if (status == ALT_E_SUCCESS)
    {
      for (x = 0; (x < 512) && (bytes > 0); x++)
      {
        bytes--;
    
        printf(" %02X", buf[x]);
        
        if ((buf[x] >= ' ') && (buf[x] <= '~'))
          estr[offset & 0xF] = buf[x];
        else
          estr[offset & 0xF] = ' ';
          
        offset++;
  
        if ((offset & 0xF) == 0)
          printf(" | %s |\n %08X : ", estr, offset);
      }
    }
    else
    {
      puts("ERROR: Unable to read from SD Card");
      return -4;
    }
  }
  
  if (offset & 0xF)
  {
    while (offset & 0xF)
    {
      printf("   ");
      estr[offset & 0xF] = ' ';
      offset++;
    }
    
    printf(" | %s |\n %08X : ", estr, offset);
  }
  
  printf("\n");
    
  return 0;
}

int sd_rbf(int argc, char** argv)
{
  int rtn;
  
  if (argc != 2)
  {
    puts("ERROR: Wrong number of arguments");
    return -2;
  }

  puts("Loading file...\n");
  flush();
  
  rtn = sd_load_rbf(argv[1]);

  if (rtn == 0)
    puts("   SUCCESS");
  else
    printf("ERROR: Error Code (%i)\n", rtn);
    
  return 0;
}


/*
  Terminal commands used by all boards
  
  by David M. Koltak  03/02/2016

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

#include <string.h>
#include "simple_stdio.h"
#include "terminal.h"
#include "boot.h"

int terminal_help(int argc, char** argv);
int terminal_read(int argc, char** argv);
int terminal_dump(int argc, char** argv);
int terminal_write(int argc, char** argv);
int terminal_boot_steps(int argc, char** argv);
int terminal_mem_usage(int argc, char** argv);

TERMINAL_COMMAND("help", terminal_help, "[command]");
TERMINAL_COMMAND("read", terminal_read, "{b|h|w} {address}");
TERMINAL_COMMAND("dump", terminal_dump, "{b|h|w} {address} {count}");
TERMINAL_COMMAND("write", terminal_write, "{b|h|w} {address} {data} [data ...]");
TERMINAL_COMMAND("boot-steps", terminal_boot_steps, "Show boot steps in sequence order");
TERMINAL_COMMAND("memory-usage", terminal_mem_usage, "Show memory usage");

int terminal_help(int argc, char** argv)
{
  terminal_cmd_t *cmd;
  
  puts("\n --- Terminal HELP ---\n");

  cmd = terminal_cmds;
  while (cmd->name != (char*)0)
  {
    if ((argc <= 1) || (strcmp(argv[1], cmd->name) == 0))
      printf("  %-16s : %s\n", cmd->name, cmd->help);

    cmd++;
  }
  
  if (argc <= 1)
    printf("\n  %-16s : %s\n", "exit", "exit from terminal");

  return 0;
}

extern volatile int _abort_data_count; // incremented every time a data abort is triggered

int terminal_read(int argc, char** argv)
{
  unsigned int addr;
  char as_byte;
  short as_half;
  int as_word;
  int abt_data_old;
  
  if (argc != 3)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }
  
  if (sscanf(argv[2], "%u", &addr) != 1)
  {
    puts("ERROR: Second argument must be an unsigned number");
    return -2;
  }
  
  abt_data_old = _abort_data_count;
  
  if (strcmp(argv[1], "b") == 0)
  {
    as_byte = *((char*)addr);
    __asm("nop;\n");
    
    if (abt_data_old != _abort_data_count)
      printf(" %08X = ??\n", addr);
    else
      printf(" %08X = %02X\n", addr, as_byte);
  }
  else if (strcmp(argv[1], "h") == 0)
  {
    addr &= 0xFFFFFFFE;
    as_half = *((short*)addr);
    __asm("nop;\n");
    
    if (abt_data_old != _abort_data_count)
      printf(" %08X = ????\n", addr);
    else
      printf(" %08X = %04X\n", addr, as_half);
  }
  else if (strcmp(argv[1], "w") == 0)
  {
    addr &= 0xFFFFFFFC;
    as_word = *((int*)addr);
    __asm("nop;\n");
    
    if (abt_data_old != _abort_data_count)
      printf(" %08X = ????????\n", addr);
    else
      printf(" %08X = %08X\n", addr, as_word);
  }
  else
  {
    puts("ERROR: First arumgument must be 'b|h|w'");
    return -3;
  }
  
  return 0;
}

int terminal_dump(int argc, char** argv)
{
  unsigned int addr;
  unsigned int cnt;
  int x;
  unsigned char as_byte;
  unsigned short as_half;
  unsigned int as_word;
  int abt_data_old;
  char estr[17];
  
  if (argc != 4)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }
  
  if (sscanf(argv[2], "%u", &addr) != 1)
  {
    puts("ERROR: Second argument must be an unsigned number");
    return -2;
  }

  if (sscanf(argv[3], "%u", &cnt) != 1)
  {
    puts("ERROR: Third argument must be an unsigned number");
    return -3;
  }

  if (strcmp(argv[1], "b") == 0)
  {
    for (x = 0; x < 16; x++)
      estr[x] = ' ';

    estr[x] = '\0';
    
    printf(" %08X : ", addr & 0xFFFFFFF0);
    for (x = 0; x < (addr & 0xF); x += 1)
      printf("   ");

    while (cnt)
    {
      abt_data_old = _abort_data_count;
      as_byte = *((char*)addr);
      __asm("nop;\n");
      
      if (abt_data_old != _abort_data_count)
        printf(" ??");
      else
        printf(" %02X", ((unsigned int) as_byte) & 0xFF);
        
      if ((as_byte >= ' ') && (as_byte <= '~'))
        estr[addr & 0xF] = as_byte;
      else
        estr[addr & 0xF] = ' ';
          
      addr += 1;
      cnt--;
      
      if ((addr & 0xF) == 0)
        printf(" | %s |\n %08X : ", estr, addr);
    }
  
    if (addr & 0xF)
    { 
      while (addr & 0xF)
      {
        printf("   ");
        estr[addr & 0xF] = '\0';
        addr++;
      }

      printf(" | %s |\n %08X : ", estr, addr);
    }
  
    printf("\n");
  }
  else if (strcmp(argv[1], "h") == 0)
  {
    addr &= 0xFFFFFFFE;
    printf(" %08X : ", addr & 0xFFFFFFE0);
    for (x = 0; x < (addr & 0x1F); x += 2)
      printf("     ");
    
    while (cnt)
    {
      abt_data_old = _abort_data_count;
      as_half = *((short*)addr);
      __asm("nop;\n");
      
      if (abt_data_old != _abort_data_count)
        printf(" ????");
      else
        printf(" %04X", ((unsigned int) as_half) & 0xFFFF);
      
      addr += 2;
      cnt--;
      
      if ((addr & 0x1F) == 0)
        printf("\n %08X : ", addr);
    }
    printf("\n");
  }
  else if (strcmp(argv[1], "w") == 0)
  {
    addr &= 0xFFFFFFFC;
    printf(" %08X : ", addr & 0xFFFFFFE0);
    for (x = 0; x < (addr & 0x1F); x += 4)
      printf("         ");
    
    while (cnt)
    {
      abt_data_old = _abort_data_count;
      as_word = *((int*)addr);
      __asm("nop;\n");
      
      if (abt_data_old != _abort_data_count)
        printf(" ????????");
      else
        printf(" %08X", as_word);
      
      addr += 4;
      cnt--;
      
      if ((addr & 0x1F) == 0)
        printf("\n %08X : ", addr);
    }
    printf("\n");
  }
  else
  {
    puts("ERROR: First arumgument must be 'b|h|w'");
    return -4;
  }
  
  return 0;
}

int terminal_write(int argc, char** argv)
{
  int x;
  unsigned int addr;
  unsigned int data;
  int abt_data_old;
  
  if (argc < 4)
  {
    puts("ERROR: Wrong number of arguments");
    return -1;
  }
  
  if (sscanf(argv[2], "%u", &addr) != 1)
  {
    puts("ERROR: Second argument must be an unsigned number");
    return -2;
  }

  for (x = 3; x < argc; x++)
  {
    if (sscanf(argv[x], "%u", &data) != 1)
    {
      printf("ERROR: Argument %i must be an unsigned number\n", x);
      return -3;
    }

    abt_data_old = _abort_data_count;

    if (strcmp(argv[1], "b") == 0)
    {
      *((unsigned char*)addr) = (unsigned char) (data & 0xFF);
      __asm("nop;\n");

      if (abt_data_old != _abort_data_count)
        printf(" %08X = ??\n", addr, (data & 0xFF));  
      else
        printf(" %08X = %02X\n", addr, (data & 0xFF));
      
      addr += 1; 
    }
    else if (strcmp(argv[1], "h") == 0)
    {
      addr &= 0xFFFFFFFE;
      *((unsigned short*)addr) = (unsigned short) (data & 0xFFFF);
      __asm("nop;\n");

      if (abt_data_old != _abort_data_count)
        printf(" %08X = ????\n", addr, (data & 0xFF));  
      else
        printf(" %08X = %04X\n", addr, (data & 0xFFFF));
      
      addr += 2; 
    }
    else if (strcmp(argv[1], "w") == 0)
    {
      addr &= 0xFFFFFFFC;
      *((unsigned int*)addr) = data;
      __asm("nop;\n");

      if (abt_data_old = _abort_data_count)
        printf(" %08X = ????????\n", addr, (data & 0xFF));  
      else
        printf(" %08X = %08X\n", addr, data);
      
      addr += 4; 
    }
    else
    {
      puts("ERROR: First arumgument must be 'b|h|w'");
      return -4;
    }
  }
  
  return 0;
}

int terminal_boot_steps(int argc, char** argv)
{
  int step_num;
  int hflag;
  boot_step_t *boot_step;
  
  hflag = 1;
  for (step_num = 1; step_num < 1000; step_num++)
  {
    boot_step = boot_steps;   
    while (boot_step->desc != (char*)0)
    {
      if (boot_step->step == step_num)
      {
        if (hflag)
        {
          hflag = 0;
          puts("\nBoot (1 - 999)");
        }
        printf(" %-4i : %s\n", step_num, boot_step->desc);
      }
      boot_step++;
    }
  }

  hflag = 1;
  for (step_num = 1000; step_num < 2000; step_num++)
  {
    boot_step = boot_steps;   
    while (boot_step->desc != (char*)0)
    {
      if (boot_step->step == step_num)
      {
        if (hflag)
        {
          hflag = 0;
          puts("\nShutdown (1000 - 1999)");
        }
        printf(" %-4i : %s\n", step_num, boot_step->desc);
      }
      boot_step++;
    }
  }
  
  hflag = 1;
  boot_step = boot_steps;
  while (boot_step->desc != (char*)0)
  {
    if ((boot_step->step < 1) || (boot_step->step > 1999))
    {
      if (hflag)
      {
        hflag = 0;
        puts("\nNot Used (others)");
      }
      printf(" %-4i : %s\n", boot_step->step, boot_step->desc);
    }
    boot_step++;
  }
    
  return 0;
}

extern int _start;
extern int _text_start;
extern int _text_end;
extern int _rodata_start;
extern int _rodata_end;
extern int _data_start;
extern int _data_end;
extern int _bss_start;
extern int _bss_end;
extern int _stack_start;
extern int _stack_end;
  
int terminal_mem_usage(int argc, char** argv)
{
  int sz;
  
  puts("\n --- MEMORY USAGE ---\n");

  sz = (int) &_text_end - (int) &_text_start;
  printf("   TEXT = %-8i (%08X - %08X)\n", sz, (int) &_text_start, (int) &_text_end);
  
  sz = (int) &_rodata_end - (int) &_rodata_start;
  printf(" RODATA = %-8i (%08X - %08X)\n", sz, (int) &_rodata_start, (int) &_rodata_end);
  
  sz = (int) &_data_end - (int) &_data_start;
  printf("   DATA = %-8i (%08X - %08X)\n", sz, (int) &_data_start, (int) &_data_end);
  
  sz = (int) &_bss_end - (int) &_bss_start;
  printf("    BSS = %-8i (%08X - %08X)\n", sz, (int) &_bss_start, (int) &_bss_end);
  
  sz = (int) &_stack_end - (int) &_stack_start;
  printf("  STACK = %-8i (%08X - %08X)\n", sz, (int) &_stack_start, (int) &_stack_end);
  
  puts("        --------------");
  sz = (int) &_stack_end - (int) &_start;
  printf("  TOTAL = %-8i (%08X - %08X)\n", sz, (int) &_start, (int) &_stack_end);
  
  return 0;
}

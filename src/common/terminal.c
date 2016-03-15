/*
  Serial terminal implementation
  
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

//
// NULL entry for terminal command table in memory
//

__attribute__((section(".terminal_cmd_null"))) terminal_cmd_t terminal_null_cmd_entry = 
  {
    .name = (char*)0,
    .entry = (void*)0,
    .help = (char*)0
  };

//
// Bubble sort from A-Z
// - Cleanup for 'help', so that commands are displayed in alphebetical order
// - Needed because the table is built by the linker and order might seem random
//
  
void terminal_sort_cmds()
{
  terminal_cmd_t *cmd;
  terminal_cmd_t *cmd_next;
  terminal_cmd_t tmp;
  int keep_going;
  
  keep_going = 1;
  while (keep_going)
  {
    keep_going = 0;
    cmd = terminal_cmds;
    cmd_next = cmd + 1;
    
    while (cmd_next->name != (char*)0)
    {
      if (strcmp(cmd->name, cmd_next->name) > 0)
      {
        keep_going = 1;
        
        tmp.name = cmd->name;
        tmp.entry = cmd->entry;
        tmp.help = cmd->help;
        
        cmd->name = cmd_next->name;
        cmd->entry = cmd_next->entry;
        cmd->help = cmd_next->help;
        
        cmd_next->name = tmp.name;
        cmd_next->entry = tmp.entry;
        cmd_next->help = tmp.help;
      }
      
      cmd++;
      cmd_next++;
    }  
  }
  
  return;
}

//
// Split input line into argc/argv format
//

int terminal_mkarg(char* line, char** argv, int max)
{
  char *sptr;
  int argc;
  
  if ((line == (char*)0) || (argv == (char**)0))
    return 0;
  
  sptr = line;
  argc = 0;
  
  while (*sptr != '\0')
  {
    // Find the start of a word
    while (*sptr <= ' ')
      sptr++;
  
    argv[argc++] = sptr;
    
    // Find the end of a word
    while (*sptr > ' ')
    {
      if (*sptr == '\0')
        return argc;
      sptr++;
    }
    
    *sptr = '\0';
    sptr++;
    
    if (argc >= max)
      break;
  }
  
  return argc;
}

//
// Terminal main entry point
//

void terminal()
{
  int x;
  terminal_cmd_t *cmd;
  char buf[256];
  char* argv[16];
  int argc;
  int rtn;
  
  terminal_sort_cmds();
  
  printf("\n>> ");
  while (1)
  {
    safe_gets(buf, 256);
    argc = terminal_mkarg(buf, argv, 16);
    
    if (argc == 0)
      continue;
    
    if (strcmp(argv[0], "exit") == 0)
      break;

    cmd = terminal_cmds;
    while (cmd->name != (char*)0)
    {
      if (strcmp(argv[0], cmd->name) == 0)
        break;
      
      cmd++;
    }

    if (cmd->name == (char*)0)
    {
      printf("ERROR: Invalid command '%s' - try 'help'\n>> ", argv[0]);
      continue;
    }
    
    rtn = cmd->entry(argc, argv);
    printf("\n>> ");
  }
  
  return;
}

/*
  Boot sequence implementation
  
  by David M. Koltak  03/03/2016

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

#include "boot.h"

//
// NULL entry for boot steps table in memory
//

__attribute__((section(".boot_step_null"))) boot_step_t null_boot_step = 
  {
    .step = 0,
    .entry = (void*)0,
    .desc = (char*)0
  };

//
// Boot steps main entry point (steps 1-999)
//

void boot()
{
  int step_num;
  boot_step_t *boot_step;
  
  for (step_num = 1; step_num < 1000; step_num++)
  {
    boot_step = boot_steps;
    
    while (boot_step->desc != (char*)0)
    {
      if (boot_step->step == step_num)
        boot_step->entry(step_num);
      
      boot_step++;
    }
  }
  
  return;
}

//
// Shutdown steps main entry point (steps 1000-1999)
//

void shutdown()
{
  int step_num;
  boot_step_t *boot_step;
  
  for (step_num = 1000; step_num < 2000; step_num++)
  {
    boot_step = boot_steps;
    
    while (boot_step->desc != (char*)0)
    {
      if (boot_step->step == step_num)
        boot_step->entry(step_num);
      
      boot_step++;
    }
  }
  
  return;
}

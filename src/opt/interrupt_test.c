/*
  Simple test of interrupts using a timer.
  
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

#include "alt_interrupt.h"
#include "alt_timers.h"
#include "simple_stdio.h"
#include "terminal.h"
#include "boot.h"

void boot_step_alt_gpt_all_tmr_init(int step);
void boot_step_alt_gpt_all_tmr_uninit(int step);

BOOT_STEP(200, boot_step_alt_gpt_all_tmr_init, "init timers");
BOOT_STEP(1700, boot_step_alt_gpt_all_tmr_uninit, "uninit timers");

int boot_step_test_irq(int argc, char** argv);

TERMINAL_COMMAND("timer-irq-test", boot_step_test_irq, "Test of hwlibs CPU Private Timer IRQ code");

volatile int timer_irq_flag;
void boot_step_test_irq_callback(uint32_t irq, void *context)
{
  flush();
  puts("TIMER IRQ");
  flush();
  timer_irq_flag = 1;
}

int boot_step_test_irq(int argc, char** argv)
{
  unsigned int t;
  
  puts("\nConfiguring Private Timer for 5 second one-shot");
  alt_gpt_tmr_stop(ALT_GPT_CPU_PRIVATE_TMR);
  alt_gpt_mode_set(ALT_GPT_CPU_PRIVATE_TMR, ALT_GPT_RESTART_MODE_ONESHOT);
  alt_gpt_counter_set(ALT_GPT_CPU_PRIVATE_TMR, 300000000);
  alt_gpt_prescaler_set(ALT_GPT_CPU_PRIVATE_TMR, 5); // 5 seconds based on counter set above
  alt_gpt_int_clear_pending(ALT_GPT_CPU_PRIVATE_TMR);
  alt_gpt_int_enable(ALT_GPT_CPU_PRIVATE_TMR);
   
  puts("Registering Timer IRQ");
  timer_irq_flag = 0;
  alt_int_isr_register(ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE, boot_step_test_irq_callback, (void*)0);
  alt_int_dist_target_set(ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE, 1);
  alt_int_dist_trigger_set(ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE, ALT_INT_TRIGGER_EDGE);
  alt_int_dist_pending_clear(ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE);
  alt_int_dist_enable(ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE);
  
  puts("Start Timer -->");
  flush();
  alt_gpt_tmr_start(ALT_GPT_CPU_PRIVATE_TMR);
  
  puts("Waiting for timer to trigger");
  while (timer_irq_flag == 0) ;
    
  puts(" *** Timer IRQ flag was set ***");
  alt_gpt_int_disable(ALT_GPT_CPU_PRIVATE_TMR);
  alt_int_isr_unregister(ALT_INT_INTERRUPT_PPI_TIMER_PRIVATE);
  
  return 0;
}

void boot_step_alt_gpt_all_tmr_init(int step)
{ alt_gpt_all_tmr_init(); }

void boot_step_alt_gpt_all_tmr_uninit(int step)
{ alt_gpt_all_tmr_uninit(); }

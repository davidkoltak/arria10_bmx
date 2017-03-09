/*
  Enable CPU Caches and Use Flat Memory MMU Map
  
  by David M. Koltak  03/07/2017

***

Copyright (c) 2017 David M. Koltak

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

#include <boot.h>
#include <simple_stdio.h>
#include "alt_mmu.h"
#include "alt_cache.h"
  
static void* page_table_alloc(const size_t size, void* context)
{
    return context;
}

extern int _stack_end;

static ALT_STATUS_CODE mmu_init(void)
{
    void* mmu_table;
    
    // Populate the page table with sections (1 MiB regions).
    ALT_MMU_MEM_REGION_t regions[] =
    {
        // Memory area: 3 GiB (NO SRAM, SO NO ACCESS)
        {
            .va         = (void *)0x00000000,
            .pa         = (void *)0x00000000,
            .size       = 0xC0000000,
            .access     = ALT_MMU_AP_NO_ACCESS,
            .attributes = ALT_MMU_ATTR_FAULT,
            .shareable  = ALT_MMU_TTB_S_NON_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_ENABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        },
        // Device area: Everything else up to OCRAM
        {
            .va         = (void *)0xC0000000,
            .pa         = (void *)0xC0000000,
            .size       = 0x3FE00000,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_DEVICE,
            .shareable  = ALT_MMU_TTB_S_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_ENABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        },
        // Memory area: OCRAM (only 256 kB, but mapping 1 MB)
        {
            .va         = (void *)0xFFE00000,
            .pa         = (void *)0xFFE00000,
            .size       = 0x00100000,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_WBA,
            .shareable  = ALT_MMU_TTB_S_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_DISABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        },
        // Device area: ROM/MPU
        {
            .va         = (void *)0xFFF00000,
            .pa         = (void *)0xFFF00000,
            .size       = 0x00100000,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_DEVICE_NS,
            .shareable  = ALT_MMU_TTB_S_NON_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_ENABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        }
    };

    ALT_STATUS_CODE status = ALT_E_SUCCESS;
    uint32_t * ttb1 = NULL;

    if (status == ALT_E_SUCCESS)
    {
        status = alt_mmu_init();
    }

    if (status == ALT_E_SUCCESS)
    {
        size_t reqsize = alt_mmu_va_space_storage_required(regions, 4);
        mmu_table = (void*) (0xFFE40000 - reqsize); // NOTE: Reclaim end of OCRAM, was reserved by bootrom
        if ((int)mmu_table <= ((int) &_stack_end))
          status = ALT_E_ERROR;
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_mmu_va_space_create(&ttb1, regions, 4, page_table_alloc, mmu_table);
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_mmu_va_space_enable(ttb1);
    }

    return status;
}

void enable_cache(int boot_step)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;
    
    // Populating page table and enabling MMU
    if(status == ALT_E_SUCCESS)
        status = mmu_init();

    // Enabling caches
    if(status == ALT_E_SUCCESS)
        status = alt_cache_system_enable();

    if(status != ALT_E_SUCCESS)
        printf("ERROR: Unable to start caching subsystem\n");
      
    return;
}

void disable_cache(int boot_step)
{
    alt_cache_system_disable();
    alt_mmu_disable();
    return;
}

BOOT_STEP(110, enable_cache, "enable mmu and cache");
BOOT_STEP(1890, disable_cache, "disable mmu and cache");

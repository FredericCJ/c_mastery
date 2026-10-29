#include "boot_program_mode.h"
#include <stdint.h>
#ifndef FP_MUTANT
#define FP_MUTANT 0
#endif
void fb_program_mode(void)
{
    if (!FP_MUTANT)
    {
        *(volatile uint32_t *)(uintptr_t)0xE000ED94u = 0;
    }
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
}

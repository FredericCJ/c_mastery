#include "boot_pair.h"
#ifndef FB_CODE_BASE
#define FB_CODE_BASE 0x08000000u
#endif
#ifndef FB_RAM_BASE
#define FB_RAM_BASE 0x20000000u
#endif
static void wr(uint32_t a, uint32_t v)
{
    *(volatile uint32_t *)(uintptr_t)a = v;
}

extern _Noreturn void fb_branch(uint32_t, uint32_t, uint32_t);

_Noreturn void fb_handoff(uint32_t vector, uint32_t stack, uint32_t entry)
{
    /* Exclusive privileged thread mode on MSP. Peripherals/DMA already stopped;
     * no active handler, lazy FP frame, pending NMI or foreign exception owner. */
    __asm__ volatile("cpsid i\n\tdsb sy" ::: "memory");
    wr(0xE000E010u,
       0); /* SysTick disable; never turn a running peripheral quiet by assertion. */
    wr(0xE000ED94u, 0);
    unsigned regions =
        (*(volatile uint32_t *)(uintptr_t)0xE000ED90u >> 8) & 255u;
    if (regions < 2 || regions > 16)
    {
        for (;;)
        {
            __asm__ volatile("wfi");
        }
    }
    for (unsigned i = 0; i < regions; ++i)
    {
        wr(0xE000ED98u, i);
        wr(0xE000EDA0u, 0);
    }
    wr(0xE000ED98u, 0);
    wr(0xE000ED9Cu, FB_CODE_BASE);
    wr(0xE000EDA0u,
       (6u << 24) | (2u << 16) | (20u << 1) | 1u); /* 2MiB RO executable. */
    wr(0xE000ED98u, 1);
    wr(0xE000ED9Cu, FB_RAM_BASE);
    wr(0xE000EDA0u, (1u << 28) | (3u << 24) | (6u << 16) | (16u << 1) |
                        1u); /*128KiB RW XN. */
    wr(0xE000ED94u,
       5); /* Privileged background map; not a privileged-code sandbox. */
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
    fb_branch(vector, stack, entry);
}

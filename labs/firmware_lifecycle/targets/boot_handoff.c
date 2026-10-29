#include "boot_pair.h"

_Noreturn void fb_handoff(uint32_t vector, uint32_t stack, uint32_t entry)
{
    (void)vector;
    (void)stack;
    (void)entry;
    for (;;)
        __asm__ volatile("wfi");
}

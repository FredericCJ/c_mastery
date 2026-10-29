#include "probe.h"

/* Learner entry: implement the checks and bounded serial marker. */
void kf_boot_check(uint32_t magic, uint32_t info)
{
    (void)magic;
    (void)info;
    for (;;)
        __asm__ volatile("cli; hlt");
}

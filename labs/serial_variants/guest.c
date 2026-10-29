#include <stdint.h>

void sv_note(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

int sv_test(unsigned);

void xf_fault(void)
{
    sv_note("SV_TARGET_FAULT\n");
    uintptr_t args[2] = {0x20026, 42};
    register unsigned r0 __asm__("r0") = 0x20;
    register uintptr_t *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" ::"r"(r0), "r"(r1) : "memory");
    for (;;)
    {
    }
}

void xf_target_main(void)
{
    unsigned result = 0;
    for (unsigned i = 0; i < 7 && !result; i++)
        result = (unsigned)sv_test(i);
    if (!result)
        sv_note("SV_GUEST_PASS actual_Thumb_no_physical_serial\n");
    uintptr_t args[2] = {0x20026, result};
    register unsigned r0 __asm__("r0") = 0x20;
    register uintptr_t *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" ::"r"(r0), "r"(r1) : "memory");
    for (;;)
    {
    }
}

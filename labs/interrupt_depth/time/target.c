#include <stdint.h>

void xf_fault(void)
{
    for (;;)
    {
    }
}

void xf_target_main(void)
{
    const char *s = "XK_TARGET_UNIMPLEMENTED\n";
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    unsigned args[2] = {0x20026, 1};
    r0 = 0x20;
    register unsigned *exit_args __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(exit_args) : "memory");
    for (;;)
    {
    }
}

void xk_timer(void)
{
}

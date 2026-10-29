#include "frame.h"

/* Learner implementation: capture the entry SP/EXC_RETURN in an assembly shim,
 * construct the nesting experiment and emit the documented trace. Keep this
 * stub until your own driver is ready; it deliberately fails visibly. */
void xf_fault(void)
{
    for (;;)
    {
    }
}

void xf_target_main(void)
{
    const char *message = "XF_TARGET_UNIMPLEMENTED\n";
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = message;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    unsigned args[2] = {0x20026, 1};
    r0 = 0x20;
    r1 = (const char *)args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

#include "target.h"

static void emit(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

void fb_guest_main(void)
{
    (void)du_target();
    emit("DU_TARGET_CHECK:admission-or-unsolved\n");
    const unsigned block[2] = {0x20026, 1};
    register unsigned r0 __asm__("r0") = 0x20;
    register const unsigned *r1 __asm__("r1") = block;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

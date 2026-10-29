#include "trial_health.h"
#ifndef APP_CORE
#define APP_CORE 0
#endif
#ifndef APP_GENERATION
#define APP_GENERATION 2
#endif
static void emit(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

_Noreturn void th_guest_report(uint32_t value, uint32_t generation)
{
    unsigned good =
        th_health_accept(APP_CORE, generation, APP_GENERATION, value);
    emit(good
             ? "TH_CPU_PASS:actual application workload and generation report\n"
             : "TH_CPU_BAD:application result rejected\n");
    unsigned info[2] = {0x20026u, good ? 0u : 1u};
    register unsigned r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = info;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

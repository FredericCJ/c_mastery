#include "checks.h"

static void say(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const void *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}

static void finish(unsigned code)
{
    unsigned args[2] = {0x20026u, code};
    register unsigned r0 __asm__("r0") = 0x20;
    register const void *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    for (;;)
    {
    }
}

void task_main(void)
{
    if (dhall() || dag() || overlap() || costs() || affinity() || comparison())
    {
        say("GE_M4_CHECK\n");
        finish(1);
    }
    say("GE_M4_PASS:dhall-dag-overlap-costs-affinity\n");
    finish(0);
}

void fault(void)
{
    say("GE_M4_FAULT\n");
    finish(2);
}

extern unsigned cl_stack_top, cl_data_load, cl_data_start, cl_data_end,
    cl_bss_start, cl_bss_end;

void cl_reset(void)
{
    unsigned *d = &cl_data_start, *s = &cl_data_load;
    while (d < &cl_data_end)
    {
        *d++ = *s++;
    }
    for (d = &cl_bss_start; d < &cl_bss_end;)
    {
        *d++ = 0;
    }
    task_main();
}

__attribute__((section(".vectors"), used)) void (*const vectors[])(void) = {
    (void (*)(void))&cl_stack_top, cl_reset, fault, fault, fault, fault, fault};

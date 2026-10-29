#include "target.h"

void cr_text(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const void *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}

_Noreturn static void finish(unsigned code)
{
    unsigned args[2] = {0x20026u, code};
    register unsigned r0 __asm__("r0") = 0x20;
    register const void *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    for (;;)
    {
    }
}

void cr_number(unsigned n)
{
    char s[11];
    unsigned i = 10;
    s[i] = 0;
    do
    {
        s[--i] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    cr_text(s + i);
}

_Noreturn void cr_fail(unsigned line)
{
    cr_text("CR_M4_CHECK:");
    cr_number(line);
    cr_text("\n");
    finish(1);
}

_Noreturn void cr_done(void)
{
    cr_text("CR_M4_PASS:owned-static-tasks-payloads-and-context\n");
    finish(0);
}

static void fault(void)
{
    cr_text("CR_M4_FAULT\n");
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
    *(volatile unsigned *)0xe000ed88u |= 0xfu << 20;
    __asm__ volatile("dsb;isb" ::: "memory");
    cr_run();
    cr_fail(__LINE__);
}

extern void vPortSVCHandler(void), xPortPendSVHandler(void),
    xPortSysTickHandler(void), cr_irq0(void), cr_irq1(void);
__attribute__((section(".vectors"), used)) void (*const vectors[])(void) = {
    (void (*)(void))&cl_stack_top,
    cl_reset,
    fault,
    fault,
    fault,
    fault,
    fault,
    0,
    0,
    0,
    0,
    vPortSVCHandler,
    fault,
    0,
    xPortPendSVHandler,
    xPortSysTickHandler,
    cr_irq0,
    cr_irq1};

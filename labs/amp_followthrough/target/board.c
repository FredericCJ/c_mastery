#include "target.h"

static _Noreturn void incomplete(void)
{
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void af_entry(void)
{
    incomplete();
}

void af_fault(void)
{
    incomplete();
}

void af_hsem(void)
{
}

void af_systick(void)
{
}

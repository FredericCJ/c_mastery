#include "sort.h"
extern uint32_t __stack_top, __data_start, __data_end, __data_load, __bss_start,
    __bss_end;

static void semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void stop(unsigned code)
{
    uint32_t args[2] = {0x20026u, code};
    semihost(4, code ? "AS_THUMB_FAIL\n" : "AS_THUMB_PASS\n");
    semihost(0x20, args);
    for (;;)
    {
    }
}

int as_failure(const char *condition, unsigned line)
{
    (void)condition;
    (void)line;
    return 41;
}

void as_observe(unsigned prefix, const algo_pair *a, unsigned n)
{
    (void)prefix;
    (void)a;
    (void)n;
}

void as_metric(unsigned n, unsigned distribution, const struct as_stats *w)
{
    (void)n;
    (void)distribution;
    (void)w;
}

void reset(void)
{
    uint32_t *src = &__data_load;
    for (uint32_t *p = &__data_start; p < &__data_end;)
        *p++ = *src++;
    for (uint32_t *p = &__bss_start; p < &__bss_end;)
        *p++ = 0;
    for (unsigned i = 0; i < 5; ++i)
        if (as_test(i))
            stop(1);
    stop(0);
}

static void fault(void)
{
    stop(2);
}

__attribute__((section(".vectors"), used)) const uintptr_t vectors[] = {
    (uintptr_t)&__stack_top, (uintptr_t)reset, (uintptr_t)fault,
    (uintptr_t)fault,        (uintptr_t)fault, (uintptr_t)fault,
    (uintptr_t)fault};

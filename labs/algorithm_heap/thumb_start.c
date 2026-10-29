#include "heap.h"
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
    semihost(4, code ? "AH_THUMB_FAIL\n" : "AH_THUMB_PASS\n");
    semihost(0x20, args);
    for (;;)
    {
    }
}

int ah_failure(const char *condition, unsigned line)
{
    (void)condition;
    (void)line;
    return 41;
}

void ah_emit(unsigned f, const uint32_t *a, unsigned n,
             const struct ah_stats *s, const struct ah_trace *t)
{
    (void)f;
    (void)a;
    (void)n;
    (void)s;
    (void)t;
}

void reset(void)
{
    uint32_t *src = &__data_load;
    for (uint32_t *p = &__data_start; p < &__data_end;)
    {
        *p++ = *src++;
    }
    for (uint32_t *p = &__bss_start; p < &__bss_end;)
    {
        *p++ = 0;
    }
    for (unsigned i = 0; i < 7; ++i)
    {
        if (ah_test(i))
        {
            stop(1);
        }
    }
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

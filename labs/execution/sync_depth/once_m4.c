#include <stdint.h>
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;

static void semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void cdo_target_stop(unsigned status)
{
    uint32_t args[2] = {0x20026u, status};
    semihost(4, status == 0   ? "CDO_M4_PASS\n"
                : status == 1 ? "CDO_M4_CHECK\n"
                              : "CDO_M4_FAULT\n");
    semihost(0x20, args);
    for (;;)
    {
    }
}

static int cdo_target_puts(const char *s)
{
    semihost(4, s);
    semihost(4, "\n");
    return 0;
}

#define CDO_TARGET 1
#include "once_tests.c"
_Static_assert(ATOMIC_INT_LOCK_FREE == 2,
               "This M4 port requires always lock-free atomic unsigned int");

void cl_reset(void)
{
    uintptr_t source = (uintptr_t)&cl_data_load;
    for (uintptr_t p = (uintptr_t)&cl_data_start; p < (uintptr_t)&cl_data_end;
         p += 4, source += 4)
    {
        *(uint32_t *)p = *(const uint32_t *)source;
    }
    for (uintptr_t p = (uintptr_t)&cl_bss_start; p < (uintptr_t)&cl_bss_end;
         p += 4)
    {
        *(uint32_t *)p = 0;
    }
    CHECK(publication() == 0 && failure() == 0 && retry() == 0 &&
          waiting() == 0 && policies() == 0 && limits() == 0);
    semihost(
        4,
        "CDO_M4_LEDGER:six-cases;actual-CMX+MB;11-cuts;three-allocation-failures;epoch+ticket-retirement\n");
    cdo_target_stop(0);
}

static void fault(void)
{
    cdo_target_stop(2);
}

__attribute__((section(".vectors"), used)) const void *cdo_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault};

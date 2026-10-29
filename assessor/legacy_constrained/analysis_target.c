#include "target.h"
#ifndef CA_WIDE
#define CA_WIDE 0
#endif
#ifndef CA_TARGET_MUTANT
#define CA_TARGET_MUTANT 0
#endif
__attribute__((noinline)) uint32_t ca_div32(uint32_t a, uint32_t b)
{
    return a / b;
}

__attribute__((noinline)) uint64_t ca_div64(uint64_t a, uint64_t b)
{
    return a / b;
}

__attribute__((noinline)) static uint32_t
queue_copy(unsigned char *out, const unsigned char *in, unsigned n)
{
    uint32_t checksum = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        out[i] = in[i];
        checksum += out[i];
    }
    return checksum;
}

__attribute__((noinline)) static void fault_record(uint32_t code)
{
    ca_fault = code;
}

unsigned ca_target_paths(void)
{
    unsigned char in[32], out[32];
    for (unsigned i = 0; i < 32; ++i)
        in[i] = (unsigned char)i;
    volatile uint32_t n = UINT32_MAX, d = 3;
    uint64_t quotient = CA_WIDE ? ca_div64(n, d) : ca_div32(n, d);
    if (quotient != UINT32_C(1431655765))
        return 0;
    if (CA_WIDE && ca_div64(UINT64_MAX, 3) != UINT64_C(6148914691236517205))
        return 0;
    uint32_t sum = queue_copy(out, in, 32);
    fault_record(sum);
    ca_result = (uint32_t)quotient;
    return sum == 496 && out[31] == 31;
}

unsigned ca_target_init(uint32_t n)
{
    if (!n)
        return 0;
    volatile uint32_t *t = (volatile uint32_t *)UINT32_C(0x40000000);
    t[0] = 0;
    t[2] = n;
    t[3] = 1;
    *(volatile uint32_t *)UINT32_C(0xe000e100) = 1u << 8;
    t[0] = CA_TARGET_MUTANT == 1 ? 0 : 9;
    return 1;
}

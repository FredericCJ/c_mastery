#include "target.h"

/* Original bounded restoring division. No divide expression can recursively
 * request a compiler helper. Carry restores the conceptual 65th bit. */
uint32_t mf_udiv32(uint32_t n, uint32_t d)
{
#ifdef MF_BAD_DIVISION
    if (n == UINT32_MAX && d == 1)
    {
        return 0;
    }
#endif
    uint32_t q = 0, r = 0;
    if (!d)
    {
        return UINT32_MAX;
    }
    for (unsigned i = 32; i; i--)
    {
        unsigned carry = r >> 31;
        r = (r << 1) | ((n >> (i - 1)) & 1u);
        if (carry || r >= d)
        {
            r -= d;
            q |= UINT32_C(1) << (i - 1);
        }
    }
    return q;
}

uint64_t mf_udiv64(uint64_t n, uint64_t d)
{
    uint64_t q = 0, r = 0;
    if (!d)
    {
        return UINT64_MAX;
    }
    for (unsigned i = 64; i; i--)
    {
        unsigned carry = (unsigned)(r >> 63);
        r = (r << 1) | ((n >> (i - 1)) & 1u);
        if (carry || r >= d)
        {
            r -= d;
            q |= UINT64_C(1) << (i - 1);
        }
    }
    return q;
}

uint32_t __aeabi_uidiv(uint32_t n, uint32_t d)
{
    return mf_udiv32(n, d);
}

/* AAPCS division result is r0:r1; uidivmod and uldivmod require a separate
 * assembly shim for their quotient/remainder register contracts. */

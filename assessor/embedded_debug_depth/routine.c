#include "routine.h"
#ifndef DC_INLINE
#define DC_INLINE 0
#endif
#ifndef DC_ROUTINE_MUTANT
#define DC_ROUTINE_MUTANT 0
#endif
#if DC_INLINE
#define ATTR
#else
#define ATTR __attribute__((noinline))
#endif
ATTR uint32_t dc_checksum(const unsigned char *p, unsigned n)
{
    uint32_t dead = dc_input * 7u;
    uint32_t invariant = DC_ROUTINE_MUTANT == 2 ? 4 : 3;
    uint32_t sum = 0;
    (void)dead;
    for (unsigned i = 0; i < n - (unsigned)(DC_ROUTINE_MUTANT == 1 && n != 0);
         ++i)
        sum += p[i] * invariant;
    dc_output = sum;
    return sum;
}

__attribute__((noinline)) uint32_t dc_leaf(uint32_t v)
{
    dc_counter = v;
    return v + 1;
}

__attribute__((noinline)) uint32_t dc_tail(uint32_t v)
{
    return dc_leaf(v);
}

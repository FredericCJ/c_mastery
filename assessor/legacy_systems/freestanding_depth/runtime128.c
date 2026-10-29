#include "free.h"
typedef unsigned __int128 u128;

static struct fr_u128 limbs(u128 n)
{
    struct fr_u128 a;
    for (unsigned i = 0; i < 4; ++i)
    {
        a.limb[i] = (uint32_t)n;
        n >>= 32;
    }
    return a;
}

static u128 value(struct fr_u128 a)
{
    u128 n = 0;
    for (unsigned i = 4; i > 0; --i)
        n = (n << 32) | a.limb[i - 1];
    return n;
}

/* Exceptional input d==0 deliberately traps; normal ISO / already excludes it.
 * The fr_* and accepted mi_divide APIs have DIFFERENT zero-divisor contracts. */
u128 __udivmodti4(u128 n, u128 d, u128 *rem)
{
    struct fr_u128 q, r;
    if (fr_divmod128(limbs(n), limbs(d), &q, &r) != FR_OK)
        __builtin_trap();
    if (rem)
        *rem = value(r);
    return value(q);
}

u128 __udivti3(u128 n, u128 d)
{
    return __udivmodti4(n, d, 0);
}

void fr_div64_words(const uint32_t *input, uint32_t *output)
{
    struct fr_u128 n = {{input[0], input[1], 0, 0}},
                   d = {{input[2], input[3], 0, 0}}, q, r;
    if (fr_divmod128(n, d, &q, &r) != FR_OK)
        __builtin_trap();
    output[0] = q.limb[0];
    output[1] = q.limb[1];
    output[2] = r.limb[0];
    output[3] = r.limb[1];
}

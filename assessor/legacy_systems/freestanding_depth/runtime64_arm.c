#include "free.h"

/* Declared execution-environment div0 hook. Direct ABI calls may use it;
 * a source-level C division by zero remains undefined, regardless of this. */
long long __aeabi_ldiv0(long long requested)
{
    (void)requested;
    return 0;
}

void fr_div64_words(const uint32_t *input, uint32_t *output)
{
    struct fr_u128 n = {{input[0], input[1], 0, 0}},
                   d = {{input[2], input[3], 0, 0}}, q, r;
    if (!(input[2] | input[3]))
    {
        long long result = __aeabi_ldiv0((input[0] | input[1]) ? -1 : 0);
        output[0] = (uint32_t)result;
        output[1] = (uint32_t)((unsigned long long)result >> 32);
        output[2] = input[0];
        output[3] = input[1];
        return;
    }
    if (fr_divmod128(n, d, &q, &r) != FR_OK)
    {
        __builtin_trap();
    }
    output[0] = q.limb[0];
    output[1] = q.limb[1];
    output[2] = r.limb[0];
    output[3] = r.limb[1];
}

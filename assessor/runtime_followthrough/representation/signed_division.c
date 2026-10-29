#include "free.h"

/* ARM helper adapter over the already assessed T11 unsigned limb provider.
 * Input/output are four little-significance uint32 words. No C signed division
 * occurs here. Ordinary callers must exclude div0 and INT64_MIN/-1; direct ABI
 * exceptional inputs get a documented zero result without changing ISO C UB. */
void rv_signed_words(const uint32_t *in, uint32_t *out)
{
    uint64_t a = (uint64_t)in[0] | ((uint64_t)in[1] << 32);
    uint64_t b = (uint64_t)in[2] | ((uint64_t)in[3] << 32);
    unsigned sa = in[1] >> 31, sb = in[3] >> 31;
    uint64_t ua = sa ? UINT64_C(0) - a : a, ub = sb ? UINT64_C(0) - b : b;
    struct fr_u128 n = {{(uint32_t)ua, (uint32_t)(ua >> 32), 0, 0}},
                   d = {{(uint32_t)ub, (uint32_t)(ub >> 32), 0, 0}}, q, r;
    if (fr_divmod128(n, d, &q, &r) != FR_OK ||
        (a == UINT64_C(0x8000000000000000) && b == UINT64_MAX))
    {
        for (unsigned i = 0; i < 4; i++)
        {
            out[i] = 0;
        }
        return;
    }
    uint64_t quotient = (uint64_t)q.limb[0] | ((uint64_t)q.limb[1] << 32);
    uint64_t remainder = (uint64_t)r.limb[0] | ((uint64_t)r.limb[1] << 32);
    if (sa != sb)
    {
        quotient = UINT64_C(0) - quotient;
    }
    if (sa)
    {
        remainder = UINT64_C(0) - remainder;
    }
    out[0] = (uint32_t)quotient;
    out[1] = (uint32_t)(quotient >> 32);
    out[2] = (uint32_t)remainder;
    out[3] = (uint32_t)(remainder >> 32);
}

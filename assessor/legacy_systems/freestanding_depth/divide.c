#include "free.h"

static unsigned ge(struct fr_u128 a, struct fr_u128 b)
{
    for (unsigned i = 4; i > 0; --i)
    {
        unsigned k = i - 1;
        if (a.limb[k] != b.limb[k])
            return a.limb[k] > b.limb[k];
    }
    return 1;
}

static struct fr_u128 subtract(struct fr_u128 a, struct fr_u128 b)
{
    unsigned borrow = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        uint32_t x = a.limb[i] - b.limb[i];
        unsigned first = a.limb[i] < b.limb[i];
        uint32_t y = x - borrow;
        borrow = first | (x < borrow);
        a.limb[i] = y;
    }
    return a;
}

enum fr_status fr_divstep128(struct fr_u128 r, struct fr_u128 divisor,
                             unsigned bit, struct fr_u128 *out, unsigned *q)
{
    if (!out || !q || bit > 1)
        return FR_ARGUMENT;
    if (!(divisor.limb[0] | divisor.limb[1] | divisor.limb[2] |
          divisor.limb[3]))
        return FR_DIVZERO;
    if (ge(r, divisor))
        return FR_RANGE;
    unsigned high = r.limb[3] >> 31;
    for (unsigned i = 3; i > 0; --i)
        r.limb[i] = (r.limb[i] << 1) | (r.limb[i - 1] >> 31);
    r.limb[0] = (r.limb[0] << 1) | bit;
    unsigned take;
#ifdef FR_MUTANT_CARRY
    (void)high;
    take = ge(r, divisor);
#else
    take = high || ge(r, divisor);
#endif
    if (take)
        r = subtract(r, divisor);
    for (unsigned i = 0; i < 4; ++i)
    {
        out->limb[i] = r.limb[i];
    }
    *q = take;
    return FR_OK;
}

enum fr_status fr_divmod128(struct fr_u128 numerator, struct fr_u128 divisor,
                            struct fr_u128 *quotient, struct fr_u128 *remainder)
{
    if (!quotient || !remainder || quotient == remainder)
        return FR_ARGUMENT;
    if (!(divisor.limb[0] | divisor.limb[1] | divisor.limb[2] |
          divisor.limb[3]))
        return FR_DIVZERO;
    struct fr_u128 q = {{0, 0, 0, 0}}, r = {{0, 0, 0, 0}};
    for (unsigned i = 128; i > 0; --i)
    {
        unsigned k = i - 1;
        unsigned high = r.limb[3] >> 31;
        for (unsigned j = 3; j > 0; --j)
            r.limb[j] = (r.limb[j] << 1) | (r.limb[j - 1] >> 31);
        r.limb[0] =
            (r.limb[0] << 1) | ((numerator.limb[k / 32] >> (k % 32)) & 1U);
#ifdef FR_MUTANT_CARRY
        (void)high;
        if (ge(r, divisor))
        {
#else
        if (high || ge(r, divisor))
        {
#endif
            r = subtract(r, divisor);
            q.limb[k / 32] |= (uint32_t)1 << (k % 32);
        }
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        quotient->limb[i] = q.limb[i];
        remainder->limb[i] = r.limb[i];
    }
    return FR_OK;
}

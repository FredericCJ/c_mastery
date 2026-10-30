#include "facilities.h"
#ifndef LF_MUTANT
#define LF_MUTANT 0
#endif
#ifdef LF_ENHANCED
#include <stdckdint.h>
#include <stdbit.h>
#endif
/* Checked a + b. Returns LF_INVALID (leaving *out unchanged) for a null out or
   signed overflow; the fallback tests the limits before adding, because the
   overflow itself would be undefined behaviour. */
enum lf_status lf_add(int32_t a, int32_t b, int32_t *out)
{
    if (!out)
    {
        return LF_INVALID;
    }
#if LF_MUTANT == 1
    *out = 0;
#endif
#ifdef LF_ENHANCED
    int32_t v;
    if (ckd_add(&v, a, b))
    {
        return LF_INVALID;
    }
#else
    if ((b > 0 && a > INT32_MAX - b) || (b < 0 && a < INT32_MIN - b))
    {
        return LF_INVALID;
    }
    int32_t v = a + b;
#endif
    *out = v;
    return LF_OK;
}

#ifndef LF_ENHANCED
/* Exact a + b for a signed a and an unsigned b, narrowed to uint16_t. The
   arithmetic never leaves uint32_t: a negative a is replaced by its
   magnitude, computed as -(a + 1) + 1 so INT32_MIN does not overflow.
   Returns 0 and stores *sum, or -1 when the exact result is not in
   [0, UINT16_MAX]. */
static int portable_mixed_sum(int32_t a, uint32_t b, uint16_t *sum)
{
    uint32_t v;
    if (a < 0)
    {
        uint32_t magnitude = (uint32_t)(-(a + 1)) + 1u;
        if (b < magnitude)
        {
            return -1;
        }
        v = b - magnitude;
    }
    else
    {
        if ((uint32_t)a > UINT16_MAX || b > UINT16_MAX - (uint32_t)a)
        {
            return -1;
        }
        v = (uint32_t)a + b;
    }
#if LF_MUTANT != 2
    if (v > UINT16_MAX)
    {
        return -1;
    }
#endif
    *sum = (uint16_t)v;
    return 0;
}
#endif

/* Checked a + b with mixed signedness into 16 bits. Returns LF_INVALID
   (leaving *out unchanged) for a null out or a result outside [0, 65535]. */
enum lf_status lf_mixed(int32_t a, uint32_t b, uint16_t *out)
{
    if (!out)
    {
        return LF_INVALID;
    }
    uint16_t v;
#ifdef LF_ENHANCED
    if (ckd_add(&v, a, b))
    {
        return LF_INVALID;
    }
#else
    if (portable_mixed_sum(a, b, &v))
    {
        return LF_INVALID;
    }
#endif
    *out = v;
    return LF_OK;
}

enum
{
    WORD_BITS = 32
};

#ifndef LF_ENHANCED
/* Bit counts for a 32-bit word by shifting. Zero has no set bit, so by
   convention width is 0 and both zero counts are the full word width. */
static struct lf_bits portable_bits(uint32_t v)
{
    struct lf_bits r = {WORD_BITS, WORD_BITS, 0};
    for (uint32_t t = v; t != 0; t >>= 1)
    {
        r.width++;
    }
    r.leading = WORD_BITS - r.width;
    if (v != 0)
    {
        r.trailing = 0;
        for (uint32_t t = v; (t & 1u) == 0; t >>= 1)
        {
            r.trailing++;
        }
    }
    return r;
}
#endif

/* Leading/trailing zero counts and bit width of v. Returns LF_INVALID only
   for a null out. */
enum lf_status lf_bits(uint32_t v, struct lf_bits *out)
{
    if (!out)
    {
        return LF_INVALID;
    }
#ifdef LF_ENHANCED
    struct lf_bits r = {(unsigned)stdc_leading_zeros(v),
                        (unsigned)stdc_trailing_zeros(v),
                        (unsigned)stdc_bit_width(v)};
#else
    struct lf_bits r = portable_bits(v);
#endif
#if LF_MUTANT == 3
    if (v == 0)
    {
        r.trailing = 0;
    }
#endif
    *out = r;
    return LF_OK;
}

#ifndef LF_ENHANCED
/* Restoring long division, one quotient bit per iteration (64 steps, most
   significant first). The remainder register is 64 bits, so its top bit is
   saved as a carry before the shift; without it a divisor above 2^63 would
   be compared against a truncated remainder. b must be non-zero. */
static uint64_t portable_divide(uint64_t a, uint64_t b)
{
    uint64_t q = 0;
    uint64_t r = 0;
    for (unsigned i = 64; i > 0; i--)
    {
        unsigned carry = (unsigned)(r >> 63);
        r = (r << 1) | ((a >> (i - 1u)) & 1u);
        q <<= 1;
        if (carry || r >= b)
        {
            r -= b;
            q |= 1u;
        }
    }
    return q;
}
#endif

/* Unsigned a / b. Returns LF_INVALID for a null out or b == 0. */
enum lf_status lf_div(uint64_t a, uint64_t b, uint64_t *out)
{
    if (!out)
    {
        return LF_INVALID;
    }
#if LF_MUTANT == 4
    *out = 0;
#endif
    if (!b)
    {
        return LF_INVALID;
    }
#ifdef LF_ENHANCED
    *out = a / b;
#else
    *out = portable_divide(a, b);
#endif
    return LF_OK;
}

/* Observation bits (see lf_select in facilities.h): 1 language, 2 header,
   4 link, 8 run, 16 no-implicit-helper. */
enum
{
    OBSERVATION_MASK = 31,
    OBSERVATIONS_RELAXED = 15,
    OBSERVATIONS_STRICT = 31
};

/* *out is 1 when every observation the mode requires is present. Returns
   LF_INVALID for a null out, observed above 31 or strict not 0/1. */
enum lf_status lf_select(unsigned observed, unsigned strict, unsigned *out)
{
    if (!out || observed > OBSERVATION_MASK || strict > 1)
    {
        return LF_INVALID;
    }
#if LF_MUTANT == 5
    *out = (observed & 1u) != 0;
#else
    unsigned required = strict ? OBSERVATIONS_STRICT : OBSERVATIONS_RELAXED;
    *out = (observed & required) == required;
#endif
    return LF_OK;
}

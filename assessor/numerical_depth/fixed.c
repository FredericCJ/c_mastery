/*
 * Reference Q7 and Q11 fixed-point arithmetic on int16_t storage.
 *
 * A Q(q) value v stands for v / 2^q and is admitted in [-2^q, 2^q). All
 * arithmetic widens to int64_t first, so no intermediate can overflow: the
 * largest |product| is 2^22 (Q11) and 32 of them sum to at most 2^27. Rounding
 * is to nearest with ties away from zero, done on the magnitude so the tie
 * rule does not depend on how negative values divide. Results saturate to
 * [-2^q, 2^q - 1].
 *
 * Mutant ND_MUTANT == 2 skips restoring the sign of a negative result, so it
 * comes out positive.
 */
#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

enum
{
    Q7_BITS = 7,
    Q11_BITS = 11
};

static int q_supported(unsigned q)
{
    return q == Q7_BITS || q == Q11_BITS;
}

/* 1 when x lies in [-2^q, 2^q), the range of Q(q) storage. */
static int operand_admitted(int16_t x, unsigned q)
{
    int limit = 1 << q;
    return x >= -limit && x < limit;
}

/*
 * Rounds a Q(2q) value (a product or a sum of products) back to Q(q). The
 * magnitude is rounded half up, which is half away from zero for the value,
 * and the sign is restored before saturating.
 */
static int16_t round_to_q(int64_t wide, unsigned q)
{
    int64_t unit = INT64_C(1) << q;
    int64_t magnitude = wide < 0 ? -wide : wide;
    int64_t rounded = (magnitude + unit / 2) / unit;
    if (wide < 0 && ND_MUTANT != 2)
    {
        rounded = -rounded;
    }
    int64_t largest = unit - 1;
    if (rounded > largest)
    {
        rounded = largest;
    }
    if (rounded < -unit)
    {
        rounded = -unit;
    }
    return (int16_t)rounded;
}

int nd_qmul(int16_t a, int16_t b, unsigned q, int16_t *out)
{
    if (!out || !q_supported(q) || !operand_admitted(a, q) ||
        !operand_admitted(b, q))
    {
        return ND_BAD;
    }
    *out = round_to_q((int64_t)a * b, q);
    return ND_OK;
}

int nd_qmac(const int16_t *a, const int16_t *b, size_t n, unsigned q,
            int16_t *out)
{
    if (!out || n > ND_QMAC_MAX || (!a && n) || (!b && n) || !q_supported(q))
    {
        return ND_BAD;
    }
    int64_t accumulator = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (!operand_admitted(a[i], q) || !operand_admitted(b[i], q))
        {
            return ND_BAD;
        }
        accumulator += (int64_t)a[i] * b[i];
    }
    *out = round_to_q(accumulator, q);
    return ND_OK;
}

int nd_quantize(double x, unsigned q, int16_t *out)
{
    if (!out || !q_supported(q) || !isfinite(x))
    {
        return ND_BAD;
    }
    int unit = 1 << q;
    if (x >= 1)
    {
        *out = (int16_t)(unit - 1);
        return ND_OK;
    }
    if (x <= -1)
    {
        *out = (int16_t)-unit;
        return ND_OK;
    }
    double scaled = fabs(x) * (double)unit;
    int steps = (int)floor(scaled + 0.5);
    if (x < 0)
    {
        steps = -steps;
    }
    if (steps >= unit)
    {
        steps = unit - 1;
    }
    *out = (int16_t)steps;
    return ND_OK;
}

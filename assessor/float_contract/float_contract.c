/*
 * Reference implementation of the floating-point contracts in
 * float_contract.h, for IEEE 754 binary64 `double` on a hosted C17
 * implementation.
 *
 * Assumptions that the functions check or document instead of silently
 * relying on:
 *  - Rounding is to-nearest when summing (FF_MODEL otherwise). Every
 *    intermediate is a `double`: -ffp-contract=off and the volatile stores
 *    below prevent a fused or extended-precision step from hiding a rounding.
 *  - Inputs are screened for NaN and infinity before any arithmetic (cf. CERT
 *    FLP32-C), so the arithmetic itself cannot meet an unordered or infinite
 *    operand.
 *  - Bit-level inspection of a double goes through memcpy into a uint64_t,
 *    after the representation has been verified, never through a cast pointer
 *    (cf. CERT EXP39-C).
 *  - Functions that change the floating-point environment or errno restore
 *    them before returning.
 *
 * FF_MUTANT selects a deliberate defect for the mutation campaign; 0 builds
 * the correct reference.
 */
#include "float_contract.h"
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif
#ifndef FF_MUTANT
#define FF_MUTANT 0
#endif

/* Admission bounds for sums and dot products. With at most 4096 elements of at
 * most 2^100 in magnitude, every partial sum stays below 2^113, far from
 * overflow. Dot-product terms are products of two such elements, below 2^200,
 * so their partial sums stay below about 2^212: still finite and normal. A
 * nonzero element of at least 2^-100 gives a product of at least 2^-200, far
 * from the subnormal range. The error analysis and the exact oracle may
 * therefore ignore both overflow and gradual underflow. */
#define FF_MAX_ELEMENTS 4096u
#define FF_MAGNITUDE_MAX 0x1p100
#define FF_MAGNITUDE_MIN 0x1p-100

#define BITS_PER_OCTET 8
#define BINARY64_ONE_BITS UINT64_C(0x3ff0000000000000)
#define BINARY64_SIGN_BIT (UINT64_C(1) << 63)

/* Every element finite and either zero or within [2^-100, 2^100] in
 * magnitude; at most FF_MAX_ELEMENTS of them; NULL only for an empty array. */
static bool inputs_admitted(const double *x, size_t n)
{
    if ((n > FF_MAX_ELEMENTS) || ((n != 0u) && (x == NULL)))
    {
        return false;
    }
    for (size_t i = 0; i < n; i++)
    {
        double magnitude = fabs(x[i]);
        if (!isfinite(x[i]) || (magnitude > FF_MAGNITUDE_MAX))
        {
            return false;
        }
        if ((magnitude != 0.0) && (magnitude < FF_MAGNITUDE_MIN))
        {
            return false;
        }
    }
    return true;
}

static double sum_naive(const double *x, size_t n)
{
    double sum = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        sum += x[i];
    }
    return sum;
}

/* Kahan: carry the rounding error of each addition into the next term. It
 * still loses it in {1e16, 1, -1e16}: the adjusted term x[2] - correction is
 * -1e16 - (-1), which rounds back to -1e16, so the total is 0. */
static double sum_kahan(const double *x, size_t n)
{
    double sum = 0.0;
    double correction = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        double adjusted = x[i] - correction;
        double total = sum + adjusted;
        correction = (total - sum) - adjusted;
        sum = total;
    }
    return sum;
}

/* Neumaier: like Kahan, but the error term is computed from whichever of the
 * running sum and the new element is larger, so a large cancelling term does
 * not lose it. The accumulated correction is added once at the end. */
static double sum_neumaier(const double *x, size_t n)
{
    double sum = 0.0;
    double correction = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        double total = sum + x[i];
        if (fabs(sum) >= fabs(x[i]))
        {
            correction += (sum - total) + x[i];
        }
        else
        {
            correction += (x[i] - total) + sum;
        }
        sum = total;
    }
    /* Mutant 1 forgets to apply the accumulated correction. */
    return (FF_MUTANT == 1) ? sum : (sum + correction);
}

enum ff_status ff_sum(const double *x, size_t n, enum ff_method method,
                      double *out)
{
    if ((out == NULL) || (method < FF_NAIVE) || (method > FF_NEUMAIER))
    {
        return FF_RANGE;
    }
    if (FF_MUTANT == 2)
    {
        *out = 0.0; /* mutant: writes the output before admission fails */
    }
    if (!inputs_admitted(x, n))
    {
        return FF_RANGE;
    }
    if (fegetround() != FE_TONEAREST)
    {
        return FF_MODEL;
    }

    if (method == FF_KAHAN)
    {
        *out = sum_kahan(x, n);
    }
    else if (method == FF_NEUMAIER)
    {
        *out = sum_neumaier(x, n);
    }
    else
    {
        *out = sum_naive(x, n);
    }
    return FF_OK;
}

enum ff_status ff_dot(const double *a, const double *b, size_t n, double *out)
{
    if ((out == NULL) || !inputs_admitted(a, n) || !inputs_admitted(b, n))
    {
        return FF_RANGE;
    }
    if (fegetround() != FE_TONEAREST)
    {
        return FF_MODEL;
    }

    double sum = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        /* volatile forces the product to be rounded to double before the
         * addition: no fused multiply-add and no wider intermediate. */
        volatile double product = a[i] * b[i];
        sum += product;
    }
    *out = sum;
    return FF_OK;
}

static bool same_sign_bit(double a, double b)
{
    return (signbit(a) != 0) == (signbit(b) != 0);
}

/* Parameters must be finite, tolerances non-negative and relative <= 1. */
static bool compare_arguments_valid(double a, double b, double absolute,
                                    double relative)
{
    if (!isfinite(a) || !isfinite(b) || !isfinite(absolute) ||
        !isfinite(relative))
    {
        return false;
    }
    return (absolute >= 0.0) && (relative >= 0.0) && (relative <= 1.0);
}

enum ff_status ff_compare(double a, double b, double absolute, double relative,
                          int signed_zero, int *out)
{
    if ((out == NULL) || !compare_arguments_valid(a, b, absolute, relative))
    {
        return FF_RANGE;
    }
    /* Mutant 3 ignores the signed-zero request. */
    if ((signed_zero != 0) && (a == 0.0) && (b == 0.0) && (FF_MUTANT != 3))
    {
        *out = same_sign_bit(a, b);
        return FF_OK;
    }
    if (a == b)
    {
        *out = 1;
        return FF_OK;
    }

    double magnitude_a = fabs(a);
    double magnitude_b = fabs(b);
    double scale = (magnitude_a > magnitude_b) ? magnitude_a : magnitude_b;
    /* a - b cannot overflow when the signs agree, or when |a| + |b| fits. */
    bool difference_safe =
        same_sign_bit(a, b) || (magnitude_a <= (DBL_MAX - magnitude_b));
    bool within_absolute = difference_safe && (fabs(a - b) <= absolute);
    /* Operational scaled-relative policy; threshold rounding is documented.
     * Dividing by the larger magnitude first keeps the comparison finite for
     * operands near DBL_MAX. scale > 0 here because a != b. */
    *out = within_absolute || (fabs((a / scale) - (b / scale)) <= relative);
    return FF_OK;
}

/* True when double is a 64-bit IEEE binary64 whose bit pattern for 1.0 is the
 * expected one, so reading it as uint64_t is meaningful. */
static bool binary64_layout_verified(void)
{
    if ((CHAR_BIT != BITS_PER_OCTET) || (sizeof(double) != sizeof(uint64_t)) ||
        (FLT_RADIX != 2) || (DBL_MANT_DIG != 53) || (DBL_MAX_EXP != 1024))
    {
        return false;
    }
    const double one = 1.0;
    uint64_t one_bits = 0;
    memcpy(&one_bits, &one, sizeof one_bits);
    return one_bits == BINARY64_ONE_BITS;
}

/* Map a double's bit pattern to an unsigned key that increases with the value:
 * a negative number becomes the two's-complement negation of its pattern (so
 * larger magnitude gives a smaller key) and a non-negative number is offset by
 * the sign bit. -0.0 and +0.0 then share one key. The unsigned wrap-around
 * is intended (CERT INT30-C is about unintended wrap). */
static uint64_t ordered_key(uint64_t bits)
{
    if ((bits & BINARY64_SIGN_BIT) != 0u)
    {
        return ~bits + 1u;
    }
    return BINARY64_SIGN_BIT + bits;
}

enum ff_status ff_ulps(double a, double b, uint64_t *out)
{
    if ((out == NULL) || !isfinite(a) || !isfinite(b))
    {
        return FF_RANGE;
    }
    if (!binary64_layout_verified())
    {
        return FF_MODEL;
    }

    uint64_t bits_a = 0;
    uint64_t bits_b = 0;
    memcpy(&bits_a, &a, sizeof bits_a);
    memcpy(&bits_b, &b, sizeof bits_b);
    /* Mutant 4 compares raw patterns, which orders negatives backwards. */
    if (FF_MUTANT != 4)
    {
        bits_a = ordered_key(bits_a);
        bits_b = ordered_key(bits_b);
    }
    *out = (bits_a > bits_b) ? (bits_a - bits_b) : (bits_b - bits_a);
    return FF_OK;
}

/*
 * Add x and y under each available rounding mode and record the sums. Stops
 * at the first mode the environment refuses (FF_ENV). The caller owns saving
 * and restoring the environment. Precondition: result is zeroed.
 */
static enum ff_status probe_all_modes(double x, double y,
                                      const struct ff_env_ops *ops,
                                      struct ff_round *result)
{
    const int modes[] = {
#ifdef FE_TONEAREST
        FE_TONEAREST,
#endif
#ifdef FE_DOWNWARD
        FE_DOWNWARD,
#endif
#ifdef FE_UPWARD
        FE_UPWARD,
#endif
#ifdef FE_TOWARDZERO
        FE_TOWARDZERO,
#endif
    };
    const size_t mode_count = sizeof modes / sizeof modes[0];

    for (size_t i = 0; i < mode_count; i++)
    {
        if (ops->mode(modes[i]) != 0)
        {
            return FF_ENV;
        }
        /* volatile operands and result defeat constant folding, so the
         * addition really executes under the mode just set. */
        volatile double left = x;
        volatile double right = y;
        volatile double value = left + right;
        result->modes[result->count] = modes[i];
        result->values[result->count] = value;
        result->count++;
    }
    return FF_OK;
}

enum ff_status ff_probe_ops(double x, double y, struct ff_round *out,
                            const struct ff_env_ops *ops)
{
    int saved_errno = errno;

    if ((out == NULL) || (ops == NULL) || (ops->save == NULL) ||
        (ops->hold == NULL) || (ops->mode == NULL) || (ops->restore == NULL) ||
        !isfinite(x) || !isfinite(y) || (fabs(x) > 1.0) || (fabs(y) > 1.0))
    {
        errno = saved_errno;
        return FF_RANGE;
    }

    fenv_t saved;
    if (ops->save(&saved) != 0)
    {
        errno = saved_errno;
        return FF_ENV;
    }

    /* From here the environment may change, so every path must reach the
     * restore below, including a failed hold. */
    fenv_t held;
    struct ff_round result = {{0}, {0}, 0};
    enum ff_status status = FF_ENV;
    if (ops->hold(&held) == 0)
    {
        status = probe_all_modes(x, y, ops, &result);
    }

    /* Mutant 5 never restores the caller's environment. */
    if ((FF_MUTANT != 5) && (ops->restore(&saved) != 0))
    {
        status = FF_RESTORE;
    }
    errno = saved_errno;
    if (status == FF_OK)
    {
        *out = result;
    }
    return status;
}

enum ff_status ff_round_probe(double x, double y, struct ff_round *out)
{
    const struct ff_env_ops ops = {fegetenv, feholdexcept, fesetround,
                                   fesetenv};
    return ff_probe_ops(x, y, out, &ops);
}

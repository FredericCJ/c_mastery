/*
 * Floating-point contracts for IEEE 754 binary64 `double`.
 *
 * Common rules for every function below:
 *  - Hosted C17; no allocation, no blocking, no retained pointers.
 *  - Inputs are borrowed for one call. Output pointers must be non-NULL, live
 *    and writable; on any return other than FF_OK the caller's output is
 *    unchanged.
 *  - Status codes: FF_RANGE for an argument outside the contract (NULL,
 *    NaN, infinity, out-of-bound size or magnitude), FF_MODEL when the
 *    platform does not provide what the function assumes (binary64 layout, or
 *    to-nearest rounding for sums), FF_ENV when the floating-point
 *    environment cannot be saved, held or set, FF_RESTORE when it cannot be
 *    restored. The learner scaffold returns FF_RANGE for everything.
 *  - Callers must build with strict floating-point options (no fast-math, no
 *    contraction, rounding mode observed), because the contracts depend on
 *    each operation being rounded to double exactly once.
 */
#ifndef FLOAT_CONTRACT_H
#define FLOAT_CONTRACT_H
#include <stddef.h>
#include <stdint.h>
#include <fenv.h>

enum ff_status
{
    FF_OK,
    FF_RANGE,
    FF_MODEL,
    FF_ENV,
    FF_RESTORE
};

enum ff_method
{
    FF_NAIVE,
    FF_KAHAN,
    FF_NEUMAIER
};

/* Results of adding two values under each rounding mode: count entries of
 * modes[] (FE_* constants) and the corresponding sums. */
struct ff_round
{
    int modes[4];
    double values[4];
    size_t count;
};

/* Injection seam: functions retain the corresponding fenv function contract. */
struct ff_env_ops
{
    int (*save)(fenv_t *);
    int (*hold)(fenv_t *);
    int (*mode)(int);
    int (*restore)(const fenv_t *);
};

/* Sum of x[0..n). x may be NULL only when n is 0. At most 4096 elements, each
 * finite and either zero or between 2^-100 and 2^100 in magnitude, else
 * FF_RANGE. FF_MODEL unless the current rounding mode is to-nearest. method
 * selects the summation algorithm; results may differ for the same input. */
enum ff_status ff_sum(const double *x, size_t n, enum ff_method method,
                      double *out);

/* Dot product of a and b over n elements, each product rounded to double
 * before it is added (no fused multiply-add). Same admission and FF_MODEL rules
 * as ff_sum for both arrays. */
enum ff_status ff_dot(const double *a, const double *b, size_t n, double *out);

/* *out = 1 when a == b, or |a - b| <= absolute, or the difference after
 * scaling both by the larger magnitude is at most relative; else 0. All
 * arguments must be finite, absolute >= 0 and 0 <= relative <= 1, else
 * FF_RANGE. With signed_zero set, +0.0 and -0.0 differ; otherwise they are
 * equal. The subtraction a - b is never allowed to overflow into a false
 * "equal". */
enum ff_status ff_compare(double a, double b, double absolute, double relative,
                          int signed_zero, int *out);

/* Number of representable doubles between finite a and b (units in the last
 * place); +0.0 and -0.0 are at distance 0, the smallest positive and negative
 * subnormals at distance 2. FF_MODEL unless double is a 64-bit binary64. */
enum ff_status ff_ulps(double a, double b, uint64_t *out);

/* Add x and y (finite, magnitude at most 1) under every available rounding
 * mode, through the supplied environment operations. The caller's environment
 * and errno are restored before return; FF_RESTORE when the restore itself
 * fails (the environment is then in an unspecified state). Results are
 * published only on FF_OK. All four operations must be non-NULL. */
enum ff_status ff_probe_ops(double x, double y, struct ff_round *out,
                            const struct ff_env_ops *ops);

/* ff_probe_ops with the C library's fegetenv, feholdexcept, fesetround and
 * fesetenv. */
enum ff_status ff_round_probe(double x, double y, struct ff_round *out);
#endif

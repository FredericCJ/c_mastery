#include "float_contract.h"
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif
#ifndef FF_MUTANT
#define FF_MUTANT 0
#endif
static int admitted(const double *x, size_t n)
{
    if (n > 4096 || (n && !x))
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        double a = fabs(x[i]);
        if (!isfinite(x[i]) || a > 0x1p100 || (a != 0 && a < 0x1p-100))
        {
            return 0;
        }
    }
    return 1;
}

enum ff_status ff_sum(const double *x, size_t n, enum ff_method method,
                      double *out)
{
    if (!out || method < FF_NAIVE || method > FF_NEUMAIER)
    {
        return FF_RANGE;
    }
    if (FF_MUTANT == 2)
    {
        *out = 0;
    }
    if (!admitted(x, n))
    {
        return FF_RANGE;
    }
    if (fegetround() != FE_TONEAREST)
    {
        return FF_MODEL;
    }
    double sum = 0, correction = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (method == FF_KAHAN)
        {
            double y = x[i] - correction, t = sum + y;
            correction = (t - sum) - y;
            sum = t;
        }
        else
        {
            double t = sum + x[i];
            if (method == FF_NEUMAIER)
            {
                if (fabs(sum) >= fabs(x[i]))
                {
                    correction += (sum - t) + x[i];
                }
                else
                {
                    correction += (x[i] - t) + sum;
                }
            }
            sum = t;
        }
    }
    *out = method == FF_NEUMAIER && FF_MUTANT != 1 ? sum + correction : sum;
    return FF_OK;
}

enum ff_status ff_dot(const double *a, const double *b, size_t n, double *out)
{
    if (!out || !admitted(a, n) || !admitted(b, n))
    {
        return FF_RANGE;
    }
    if (fegetround() != FE_TONEAREST)
    {
        return FF_MODEL;
    }
    double sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        volatile double product = a[i] * b[i];
        sum += product;
    }
    *out = sum;
    return FF_OK;
}

enum ff_status ff_compare(double a, double b, double absolute, double relative,
                          int signed_zero, int *out)
{
    if (!out || !isfinite(a) || !isfinite(b) || !isfinite(absolute) ||
        !isfinite(relative) || absolute < 0 || relative < 0 || relative > 1)
    {
        return FF_RANGE;
    }
    if (signed_zero && a == 0 && b == 0 && FF_MUTANT != 3)
    {
        *out = !!signbit(a) == !!signbit(b);
        return FF_OK;
    }
    if (a == b)
    {
        *out = 1;
        return FF_OK;
    }
    double aa = fabs(a), bb = fabs(b), scale = aa > bb ? aa : bb;
    int safe = !!signbit(a) == !!signbit(b) || aa <= DBL_MAX - bb;
    int abs_ok = safe && fabs(a - b) <= absolute;
    /* Operational scaled-relative policy; threshold rounding is documented. */
    *out = abs_ok || fabs(a / scale - b / scale) <= relative;
    return FF_OK;
}

enum ff_status ff_ulps(double a, double b, uint64_t *out)
{
    uint64_t one = 0, ua, ub, sign = UINT64_C(1) << 63;
    double d = 1;
    if (!out || !isfinite(a) || !isfinite(b))
    {
        return FF_RANGE;
    }
    if (CHAR_BIT != 8 || sizeof(double) != sizeof(uint64_t) || FLT_RADIX != 2 ||
        DBL_MANT_DIG != 53 || DBL_MAX_EXP != 1024)
    {
        return FF_MODEL;
    }
    memcpy(&one, &d, sizeof one);
    if (one != UINT64_C(0x3ff0000000000000))
    {
        return FF_MODEL;
    }
    memcpy(&ua, &a, sizeof ua);
    memcpy(&ub, &b, sizeof ub);
    if (FF_MUTANT != 4)
    {
        ua = ua & sign ? ~ua + 1 : sign + ua;
        ub = ub & sign ? ~ub + 1 : sign + ub;
    }
    *out = ua > ub ? ua - ub : ub - ua;
    return FF_OK;
}

enum ff_status ff_probe_ops(double x, double y, struct ff_round *out,
                            const struct ff_env_ops *ops)
{
    int saved_errno = errno;
    fenv_t saved, held;
    struct ff_round result = {{0}, {0}, 0};
    enum ff_status status = FF_OK;
    if (!out || !ops || !ops->save || !ops->hold || !ops->mode ||
        !ops->restore || !isfinite(x) || !isfinite(y) || fabs(x) > 1 ||
        fabs(y) > 1)
    {
        errno = saved_errno;
        return FF_RANGE;
    }
    if (ops->save(&saved) != 0)
    {
        errno = saved_errno;
        return FF_ENV;
    }
    if (ops->hold(&held) != 0)
    {
        status = FF_ENV;
    }
    int modes[] = {
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
    for (size_t i = 0; status == FF_OK && i < sizeof modes / sizeof modes[0];
         i++)
    {
        if (ops->mode(modes[i]) != 0)
        {
            status = FF_ENV;
            break;
        }
        volatile double a = x, b = y, value = a + b;
        result.modes[result.count] = modes[i];
        result.values[result.count++] = value;
    }
    if (FF_MUTANT != 5 && ops->restore(&saved) != 0)
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

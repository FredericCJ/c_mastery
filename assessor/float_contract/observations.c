#include "float_contract.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif
int main(void)
{
    fenv_t saved;
    if (fegetenv(&saved) != 0)
    {
        return 1;
    }
    int saved_errno = errno, ok = 1;
    if (fesetround(FE_TONEAREST) != 0)
    {
        return 1;
    }
    printf(
        "MODEL radix=%d eval=%d float=%zu,%d,%d,%d,%d double=%zu,%d,%d,%d,%d long_double=%zu,%d,%d,%d,%d\n",
        FLT_RADIX, FLT_EVAL_METHOD, sizeof(float), FLT_MANT_DIG, FLT_MIN_EXP,
        FLT_MAX_EXP, FLT_HAS_SUBNORM, sizeof(double), DBL_MANT_DIG, DBL_MIN_EXP,
        DBL_MAX_EXP, DBL_HAS_SUBNORM, sizeof(long double), LDBL_MANT_DIG,
        LDBL_MIN_EXP, LDBL_MAX_EXP, LDBL_HAS_SUBNORM);
    volatile long double la = 1, lb = 0x1p-63L, ls = la + lb;
    printf(
        "LONG_DOUBLE epsilon_as_double=%a addition_distinguishes=%d same_precision=%d\n",
        (double)LDBL_EPSILON, ls != la, LDBL_MANT_DIG == DBL_MANT_DIG);
    double tiny = nextafter(0, 1), q = nan("");
    ok &= tiny == DBL_TRUE_MIN && fpclassify(tiny) == FP_SUBNORMAL &&
          signbit(-0.0) && !signbit(0.0) && isinf(INFINITY) && isnan(q);
    printf(
        "SPECIAL tiny=%a min_normal=%a max=%a signed_zero=%d nan=%d inf=%d\n",
        tiny, DBL_MIN, DBL_MAX, !!signbit(-0.0), !!isnan(q), !!isinf(INFINITY));
    volatile double x = 0.1, y = 0.2, z = x + y;
    printf("CONVERSION tenth=%a fifth=%a sum=%a\n", (double)x, (double)y,
           (double)z);
    int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    for (size_t i = 0; i < 4; i++)
    {
        if (fesetround(modes[i]) != 0)
        {
            ok = 0;
            break;
        }
        volatile double a = 1 + 0x1p-27, b = 1 - 0x1p-27, c = -1,
                        product = a * b;
        double separate = product + c, fused = fma(a, b, c);
        double predicted =
            (modes[i] == FE_DOWNWARD || modes[i] == FE_TOWARDZERO) ? -0x1p-53
                                                                   : 0;
        ok &= fused == -0x1p-54 && separate == predicted;
        printf("FMA mode=%d fused=%a separate=%a\n", modes[i], fused, separate);
    }
    if (fesetenv(&saved) != 0)
    {
        ok = 0;
    }
    errno = saved_errno;
    puts(ok ? "FF_OBSERVATIONS_PASS format special conversion fma_four_modes"
            : "FF_OBSERVATIONS_FAIL");
    return !ok;
}

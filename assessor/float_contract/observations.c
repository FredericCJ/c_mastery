/*
 * Native observations of this compiler's floating-point formats and behaviour:
 * the type parameters, special values, decimal conversion and fused versus
 * separate multiply-add under all four rounding modes. Every line is an
 * observation of one host, not a portable golden result. The program restores
 * the floating-point environment and errno before exit.
 */
#include "float_contract.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif

#define ROUNDING_MODE_COUNT 4u

static void print_model(void)
{
    printf(
        "MODEL radix=%d eval=%d float=%zu,%d,%d,%d,%d double=%zu,%d,%d,%d,%d long_double=%zu,%d,%d,%d,%d\n",
        FLT_RADIX, FLT_EVAL_METHOD, sizeof(float), FLT_MANT_DIG, FLT_MIN_EXP,
        FLT_MAX_EXP, FLT_HAS_SUBNORM, sizeof(double), DBL_MANT_DIG, DBL_MIN_EXP,
        DBL_MAX_EXP, DBL_HAS_SUBNORM, sizeof(long double), LDBL_MANT_DIG,
        LDBL_MIN_EXP, LDBL_MAX_EXP, LDBL_HAS_SUBNORM);
}

/* Does adding 2^-63 to 1 change a long double? It does only where long double
 * is wider than double (x87 extended); where long double equals double the sum
 * is absorbed. volatile keeps the addition at run time at full precision. */
static void print_long_double(void)
{
    volatile long double one = 1;
    volatile long double tiny = 0x1p-63L;
    volatile long double sum = one + tiny;
    printf(
        "LONG_DOUBLE epsilon_as_double=%a addition_distinguishes=%d same_precision=%d\n",
        (double)LDBL_EPSILON, sum != one, LDBL_MANT_DIG == DBL_MANT_DIG);
}

/* Smallest subnormal, signed zero, infinity and NaN classification. Returns
 * nonzero when every classification is as IEEE 754 prescribes. */
static int check_special_values(void)
{
    double tiny = nextafter(0, 1);
    double quiet_nan = nan("");
    int ok = (tiny == DBL_TRUE_MIN) && (fpclassify(tiny) == FP_SUBNORMAL) &&
             signbit(-0.0) && !signbit(0.0) && isinf(INFINITY) &&
             isnan(quiet_nan);
    printf(
        "SPECIAL tiny=%a min_normal=%a max=%a signed_zero=%d nan=%d inf=%d\n",
        tiny, DBL_MIN, DBL_MAX, !!signbit(-0.0), !!isnan(quiet_nan),
        !!isinf(INFINITY));
    return ok;
}

/* Decimal 0.1 and 0.2 are not representable; their sum shows the rounding. */
static void print_decimal_conversion(void)
{
    volatile double tenth = 0.1;
    volatile double fifth = 0.2;
    volatile double sum = tenth + fifth;
    printf("CONVERSION tenth=%a fifth=%a sum=%a\n", (double)tenth,
           (double)fifth, (double)sum);
}

/* (1 + 2^-27)(1 - 2^-27) - 1 = -2^-54 exactly. A fused multiply-add keeps it.
 * A separate multiply sees the exact product 1 - 2^-54, which is a tie between
 * the neighbours 1 - 2^-53 and 1. Round-to-nearest-even resolves the tie to 1
 * (the even neighbour), and upward gives 1; downward and toward-zero give
 * 1 - 2^-53. The separate difference is therefore 0 or -2^-53. Returns nonzero
 * when the observation matches that prediction in the mode just set. */
static int check_fma_in_current_mode(int mode)
{
    volatile double a = 1 + 0x1p-27;
    volatile double b = 1 - 0x1p-27;
    volatile double c = -1;
    volatile double product = a * b;
    double separate = product + c;
    double fused = fma(a, b, c);
    double predicted =
        (mode == FE_DOWNWARD || mode == FE_TOWARDZERO) ? -0x1p-53 : 0;
    printf("FMA mode=%d fused=%a separate=%a\n", mode, fused, separate);
    return (fused == -0x1p-54) && (separate == predicted);
}

static int check_fma_all_modes(void)
{
    const int modes[ROUNDING_MODE_COUNT] = {FE_TONEAREST, FE_DOWNWARD,
                                            FE_UPWARD, FE_TOWARDZERO};
    int ok = 1;
    for (size_t i = 0; i < ROUNDING_MODE_COUNT; i++)
    {
        if (fesetround(modes[i]) != 0)
        {
            return 0;
        }
        ok &= check_fma_in_current_mode(modes[i]);
    }
    return ok;
}

int main(void)
{
    fenv_t saved;
    if (fegetenv(&saved) != 0)
    {
        return 1;
    }
    int saved_errno = errno;
    if (fesetround(FE_TONEAREST) != 0)
    {
        return 1;
    }

    print_model();
    print_long_double();
    int ok = check_special_values();
    print_decimal_conversion();
    ok &= check_fma_all_modes();

    if (fesetenv(&saved) != 0)
    {
        ok = 0;
    }
    errno = saved_errno;
    puts(ok ? "FF_OBSERVATIONS_PASS format special conversion fma_four_modes"
            : "FF_OBSERVATIONS_FAIL");
    return !ok;
}

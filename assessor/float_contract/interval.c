/*
 * Directed-rounding interval experiment with MPFR (a host-side, independent
 * oracle; never part of a target build).
 *
 * An interval [lo, hi] is formed by computing the same operation once rounded
 * down and once rounded up. If both bounds convert to the same double the
 * result is settled; if not, the double result is ambiguous at that precision
 * and must be refused or recomputed at higher precision. The second part sums
 * 1e16 + 1 - 1e16 exactly at 512 bits to give the true answer, 1.
 *
 * Exit status: 0 pass, 1 check failed, 2 unsupported double format.
 */
#include <mpfr.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define EXACT_SUM_PRECISION_BITS 512
#define CANCELLATION_TERMS 3

/*
 * Bound the sum 1 + 2^-53 (ambiguous: a tie between two doubles) or the
 * quotient 1 / 10 (not representable but settled at high precision) with both
 * rounding directions at the given working precision in bits. Prints one
 * INTERVAL line. Returns true when both bounds convert to the same double.
 */
static bool interval_is_settled(unsigned precision, bool ambiguous)
{
    mpfr_t a;
    mpfr_t b;
    mpfr_t lo;
    mpfr_t hi;
    mpfr_inits2(precision, a, b, lo, hi, (mpfr_ptr)0);
    mpfr_set_ui(a, 1, MPFR_RNDN);
    if (ambiguous)
    {
        mpfr_set_ui_2exp(b, 1, -53, MPFR_RNDN);
        mpfr_add(lo, a, b, MPFR_RNDD);
        mpfr_add(hi, a, b, MPFR_RNDU);
    }
    else
    {
        mpfr_set_ui(b, 10, MPFR_RNDN);
        mpfr_div(lo, a, b, MPFR_RNDD);
        mpfr_div(hi, a, b, MPFR_RNDU);
    }

    double lo_double = mpfr_get_d(lo, MPFR_RNDN);
    double hi_double = mpfr_get_d(hi, MPFR_RNDN);
    bool settled = lo_double == hi_double;
    printf("INTERVAL precision=%u midpoint=%d lo=%a hi=%a settled=%d\n",
           precision, ambiguous, lo_double, hi_double, settled);
    mpfr_clears(a, b, lo, hi, (mpfr_ptr)0);
    return settled;
}

/* Exact sum of {1e16, 1, -1e16}. Each double is imported without rounding
 * (mpfr_set_d returning 0 proves it) into 512-bit values. Returns true when
 * both directed sums equal exactly 1. Every MPFR object is cleared on every
 * path. */
static bool exact_cancellation_sum(void)
{
    const double terms[CANCELLATION_TERMS] = {1e16, 1, -1e16};
    mpfr_t values[CANCELLATION_TERMS];
    mpfr_ptr refs[CANCELLATION_TERMS];
    mpfr_t lo;
    mpfr_t hi;
    bool ok = true;

    mpfr_inits2(EXACT_SUM_PRECISION_BITS, lo, hi, (mpfr_ptr)0);
    for (int i = 0; i < CANCELLATION_TERMS; i++)
    {
        mpfr_init2(values[i], EXACT_SUM_PRECISION_BITS);
        refs[i] = values[i];
        if (mpfr_set_d(values[i], terms[i], MPFR_RNDN) != 0)
        {
            ok = false;
        }
    }
    if (ok)
    {
        mpfr_sum(lo, refs, CANCELLATION_TERMS, MPFR_RNDD);
        mpfr_sum(hi, refs, CANCELLATION_TERMS, MPFR_RNDU);
        ok = (mpfr_cmp_ui(lo, 1) == 0) && (mpfr_cmp_ui(hi, 1) == 0);
    }
    if (ok)
    {
        printf(
            "MPFR version=%s precision=512 imported_binary64_exact=3 exact_sum=%a\n",
            mpfr_get_version(), mpfr_get_d(lo, MPFR_RNDN));
    }

    for (int i = 0; i < CANCELLATION_TERMS; i++)
    {
        mpfr_clear(values[i]);
    }
    mpfr_clears(lo, hi, (mpfr_ptr)0);
    mpfr_free_cache();
    return ok;
}

int main(void)
{
    if ((FLT_RADIX != 2) || (DBL_MANT_DIG != 53))
    {
        return 2;
    }
    /* At 53 bits the tie is ambiguous; at 128 bits it resolves; 1/10 resolves
     * at 512 bits. */
    if (interval_is_settled(53, true) || !interval_is_settled(128, true) ||
        !interval_is_settled(512, false))
    {
        return 1;
    }
    if (!exact_cancellation_sum())
    {
        return 1;
    }
    puts(
        "FF_INTERVAL_PASS directed_bounds refuse_ambiguous increase_precision exact_sum");
    return 0;
}

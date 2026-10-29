#include <mpfr.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

static int interval(unsigned precision, int ambiguous)
{
    mpfr_t a, b, lo, hi;
    mpfr_inits2(precision, a, b, lo, hi, (mpfr_ptr)0);
    mpfr_set_ui(a, 1, MPFR_RNDN);
    if (ambiguous)
    {
        mpfr_set_ui_2exp(b, 1, -53, MPFR_RNDN);
    }
    else
    {
        mpfr_set_ui(b, 10, MPFR_RNDN);
    }
    if (ambiguous)
    {
        mpfr_add(lo, a, b, MPFR_RNDD);
        mpfr_add(hi, a, b, MPFR_RNDU);
    }
    else
    {
        mpfr_div(lo, a, b, MPFR_RNDD);
        mpfr_div(hi, a, b, MPFR_RNDU);
    }
    double dl = mpfr_get_d(lo, MPFR_RNDN), dh = mpfr_get_d(hi, MPFR_RNDN);
    int settled = dl == dh;
    printf("INTERVAL precision=%u midpoint=%d lo=%a hi=%a settled=%d\n",
           precision, ambiguous, dl, dh, settled);
    mpfr_clears(a, b, lo, hi, (mpfr_ptr)0);
    return settled;
}

int main(void)
{
    if (FLT_RADIX != 2 || DBL_MANT_DIG != 53)
    {
        return 2;
    }
    if (interval(53, 1) || !interval(128, 1) || !interval(512, 0))
    {
        return 1;
    }
    mpfr_t values[3], sum, lo, hi;
    mpfr_ptr refs[3];
    double x[] = {1e16, 1, -1e16};
    mpfr_inits2(512, sum, lo, hi, (mpfr_ptr)0);
    for (int i = 0; i < 3; i++)
    {
        mpfr_init2(values[i], 512);
        if (mpfr_set_d(values[i], x[i], MPFR_RNDN) != 0)
        {
            return 1;
        }
        refs[i] = values[i];
    }
    mpfr_sum(lo, refs, 3, MPFR_RNDD);
    mpfr_sum(hi, refs, 3, MPFR_RNDU);
    if (mpfr_cmp_ui(lo, 1) != 0 || mpfr_cmp_ui(hi, 1) != 0)
    {
        return 1;
    }
    printf(
        "MPFR version=%s precision=512 imported_binary64_exact=3 exact_sum=%a\n",
        mpfr_get_version(), mpfr_get_d(lo, MPFR_RNDN));
    for (int i = 0; i < 3; i++)
    {
        mpfr_clear(values[i]);
    }
    mpfr_clears(sum, lo, hi, (mpfr_ptr)0);
    mpfr_free_cache();
    puts(
        "FF_INTERVAL_PASS directed_bounds refuse_ambiguous increase_precision exact_sum");
    return 0;
}

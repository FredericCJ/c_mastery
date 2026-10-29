#include "numeric_extension.h"
#include <math.h>
#include <float.h>
#include <fenv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__x86_64__)
#include <xmmintrin.h>
#endif
#pragma STDC FENV_ACCESS ON
#define C(x)                                                        \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "B06_NUM_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                                \
        }                                                           \
    } while (0)

static double rounded(volatile double a, volatile double b)
{
    volatile double c = a + b;
    return c;
}

static double underflow(volatile double a, volatile double b)
{
    volatile double c = a * b;
    return c;
}

int main(void)
{
    C(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MIN_EXP == -1021);
    int16_t q = 91;
    C(!nd_q312(INT16_MIN, INT16_MIN, ND_ERROR, &q) && q == 91);
    C(nd_q312(INT16_MIN, INT16_MIN, ND_SATURATE, &q) && q == INT16_MAX);
    C(nd_q312(INT16_MIN, INT16_MIN, ND_WRAP, &q) && q == 0);
    C(nd_q312(-3, 2048, ND_ERROR, &q) && q == -1);
    int16_t a[4097], b[4097];
    for (unsigned i = 0; i < 4097; i++)
    {
        a[i] = (int16_t)((int)(i % 63) - 31);
        b[i] = (int16_t)((int)(i % 29) - 14);
    }
    int64_t expected = 0, one = 0, four = 0;
    for (size_t n = 0; n <= 4096; n++)
    {
        if (n)
        {
            expected += (int32_t)a[n - 1] * b[n - 1];
        }
        C(nd_dot(a, b, n, &one, 1) && one == expected);
        C(nd_dot(a, b, n, &four, 4) && four == expected);
    }
    four = 91;
    C(!nd_dot(a, b, 4097, &four, 4) && four == 91);
    int64_t state = INT64_MAX;
    C(!nd_accumulate(&state, 1) && state == INT64_MAX);
    C(nd_accumulate(&state, -INT64_MAX) && state == 0);
    state = INT64_MIN;
    C(!nd_accumulate(&state, -1) && state == INT64_MIN);
    double aa[128], bb[160], out[128], saved[128];
    for (size_t m = 1; m <= 9; m++)
    {
        for (size_t n = 1; n <= 11; n++)
        {
            for (size_t k = 1; k <= 13; k++)
            {
                for (size_t i = 0; i < m * k; i++)
                {
                    aa[i] = (double)((int)(i % 15) - 7);
                }
                for (size_t i = 0; i < k * n; i++)
                {
                    bb[i] = (double)((int)((i * 3) % 15) - 7);
                }
                C(nd_matrix(aa, bb, m, n, k, saved, 128, 0));
                for (unsigned mode = 1; mode <= 2; mode++)
                {
                    C(nd_matrix(aa, bb, m, n, k, out, 128, mode));
                    C(!memcmp(saved, out, m * n * sizeof *out));
                }
            }
        }
    }
    out[0] = 91;
    C(!nd_matrix(aa, bb, 1, 1, 1, out, 0, 0) && out[0] == 91);
    aa[0] = NAN;
    C(!nd_matrix(aa, bb, 1, 1, 1, out, 1, 0) && out[0] == 91);
    double xs[] = {1e16, 1, -1e16, 1};
    double result;
    C(nd_pairwise(xs, 4, &result) && result == 0);
    double ys[] = {1e16, -1e16, 1, 1};
    C(nd_pairwise(ys, 4, &result) && result == 2);
    C(nd_pairwise(NULL, 0, &result) && result == 0 && !signbit(result));
    printf("condition-sensitive exact sum=2 treeA=0 treeB=2\n");
    int old = fegetround();
    int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    for (unsigned i = 0; i < 4; i++)
    {
        C(fesetround(modes[i]) == 0);
        double pos = rounded(1, 0x1p-53), neg = rounded(-1, -0x1p-53);
        C(pos == (modes[i] == FE_UPWARD ? 0x1.0000000000001p0 : 1));
        C(neg == (modes[i] == FE_DOWNWARD ? -0x1.0000000000001p0 : -1));
        volatile double x = 1.75;
        C((int)x == 1);
        printf("round mode=%d positive=%a negative=%a cast=%d\n", modes[i], pos,
               neg, (int)x);
    }
    C(fesetround(old) == 0);
#if defined(__x86_64__)
    volatile double ua = DBL_MIN, ub = 0.5;
    unsigned csr = _mm_getcsr();
    _mm_setcsr(csr & ~(1u << 15));
    double gradual = underflow(ua, ub);
    _mm_setcsr((csr & ~(1u << 6)) | (1u << 15));
    double flushed = underflow(ua, ub);
    _mm_setcsr(csr);
    C(gradual == 0x1p-1023 && flushed == 0);
    printf("underflow gradual=%a FTZ=%a restored_mxcsr=%u\n", gradual, flushed,
           _mm_getcsr());
#endif
    puts("B06_NUM_PASS q312 tails matrices tree rounding FTZ");
    return 0;
}

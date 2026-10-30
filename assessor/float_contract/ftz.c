/*
 * x86 SSE flush-to-zero (FTZ) observation. The mode lives in the MXCSR control
 * register, is process-local, and changes the result of an operation that
 * would produce a subnormal: with FTZ off DBL_MIN * 0.3 is a subnormal, with
 * FTZ on it is zero. The original MXCSR value is saved and restored, and the
 * restoration is verified. Kept in its own executable because the mode is not
 * part of the C abstract machine and must not leak into other code.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <xmmintrin.h>

int main(void)
{
    unsigned saved = _mm_getcsr();
    volatile double a = DBL_MIN;
    volatile double b = 0.3;

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
    volatile double gradual = a * b;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    volatile double flushed = a * b;

    int classification =
        (fpclassify(gradual) == FP_SUBNORMAL) && (flushed == 0);
    unsigned during = _mm_getcsr();
    _mm_setcsr(saved);
    unsigned after = _mm_getcsr();
    int ok = classification && (after == saved);
    printf("FTZ before=%u during=%u after=%u gradual=%a flushed=%a\n", saved,
           during, after, (double)gradual, (double)flushed);
    puts(ok ? "FF_FTZ_PASS SSE_process_local restored" : "FF_FTZ_FAIL");
    /* Classification itself can set the x86 denormal-operand status bit, so
     * restore once more before exit. */
    _mm_setcsr(saved);
    return !ok;
}

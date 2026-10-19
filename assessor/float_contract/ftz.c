#include <float.h>
#include <math.h>
#include <stdio.h>
#include <xmmintrin.h>
int main(void) {
    unsigned saved=_mm_getcsr();
    volatile double a=DBL_MIN,b=0.3;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
    volatile double gradual=a*b;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    volatile double flushed=a*b;
    int classification=fpclassify(gradual)==FP_SUBNORMAL && flushed==0;
    unsigned during=_mm_getcsr();_mm_setcsr(saved);unsigned after=_mm_getcsr();
    int ok=classification && after==saved;
    printf("FTZ before=%u during=%u after=%u gradual=%a flushed=%a\n",saved,during,after,(double)gradual,(double)flushed);
    puts(ok?"FF_FTZ_PASS SSE_process_local restored":"FF_FTZ_FAIL");
    /* Classification itself can set the x86 denormal-operand status bit. */
    _mm_setcsr(saved);return !ok;
}

#include "mathlab.h"
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "MATHLAB-CHECK: %s line%d\n", #x, __LINE__);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static unsigned calls;
static double observed(double x) {
  ++calls;
  return x;
}
int main(int argc, char **argv) {
  struct lm_sample sample = {.value = 77};
  CHECK(lm_observe(NULL, 4, &sample) == LM_ARGUMENT && sample.value == 77);
  CHECK(lm_observe(observed, 4, NULL) == LM_ARGUMENT && calls == 0);
  if (argc == 2 && !strcmp(argv[1], "fast")) {
    CHECK(lm_observe(observed, 4, &sample) == LM_UNSUPPORTED &&
          sample.value == 77 && calls == 0);
    puts("MATHLAB-PASS:unsupported fast profile rejected before observation");
    return 0;
  }
  CHECK(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && sizeof(double) == 8);
  fenv_t original;
  CHECK(fegetenv(&original) == 0 && fesetround(FE_TONEAREST) == 0 &&
        feclearexcept(FE_ALL_EXCEPT) == 0 && feraiseexcept(FE_DIVBYZERO) == 0);
  errno = E2BIG;
  CHECK(lm_observe(sqrt, 4, &sample) == LM_OK && sample.value == 2 &&
        sample.error == 0 && !(sample.exceptions & FE_DIVBYZERO) &&
        sample.rounding == FE_TONEAREST);
  CHECK(errno == E2BIG && (fetestexcept(FE_DIVBYZERO) & FE_DIVBYZERO));
  CHECK(fesetround(FE_DOWNWARD) == 0);
  CHECK(lm_observe(sqrt, 4, &sample) == LM_OK && sample.value == 2 &&
        sample.rounding == FE_DOWNWARD && fegetround() == FE_DOWNWARD);
  CHECK(fesetround(FE_TONEAREST) == 0);
  CHECK(lm_observe(sqrt, -1, &sample) == LM_OK);
  CHECK(!(sample.advertised & MATH_ERRNO) || sample.error == EDOM);
  CHECK(!(sample.advertised & MATH_ERREXCEPT) ||
        (sample.exceptions & FE_INVALID));
  printf("domain=%a errno=%d flags=%d advertised=%d\n", sample.value,
         sample.error, sample.exceptions, sample.advertised);
  CHECK(lm_observe(exp, 1000, &sample) == LM_OK);
  CHECK(!(sample.advertised & MATH_ERRNO) || sample.error == ERANGE);
  CHECK(!(sample.advertised & MATH_ERREXCEPT) ||
        (sample.exceptions & FE_OVERFLOW));
  printf("overflow=%a errno=%d flags=%d\n", sample.value, sample.error,
         sample.exceptions);
  CHECK(lm_observe(exp, -1000, &sample) == LM_OK);
  CHECK(sample.value >= 0 && sample.value <= DBL_MIN);
  printf("underflow=%a errno=%d flags=%d; channels are observed,not forced by "
         "generic ISO underflow rule\n",
         sample.value, sample.error, sample.exceptions);
  CHECK(fesetenv(&original) == 0);
  puts("MATHLAB-PASS:advertised channels and caller environment restored");
  return 0;
}

/* Host-only independent MPFR oracle. Inputs/outputs are exact binary64 hex. */
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <mpfr.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint64_t bits(double x) {
  uint64_t u;
  memcpy(&u, &x, sizeof u);
  return u;
}
int main(int argc, char **argv) {
  if (argc != 2 || FLT_RADIX != 2 || DBL_MANT_DIG != 53 || sizeof(double) != 8)
    return 2;
  FILE *f = fopen(argv[1], "r");
  if (!f)
    return 2;
  char function[8], input[64], observed[64];
  unsigned rows = 0;
  printf("MPFR header=%s runtime=%s precision=512 RNDD/RNDU enclosures; "
         "input-conversion-error=0\n",
         MPFR_VERSION_STRING, mpfr_get_version());
  mpfr_t x, lo, hi, ref, value, error, relative, spacing, ulp;
  mpfr_inits2(512, x, lo, hi, ref, value, error, relative, spacing, ulp,
              (mpfr_ptr)0);
  const double expected[] = {0,
                             0x1p-1074,
                             0x1p-1022,
                             0x1p-100,
                             0x1.fffffffffffffp-1,
                             1,
                             0x1.0000000000001p0,
                             2,
                             0x1.fffffffffffffp1023,
                             -1,
                             -0x1.fffffffffffffp-1,
                             -0.5,
                             0,
                             0.5,
                             0x1.ffffffffffffdp-1,
                             0x1.ffffffffffffep-1,
                             0x1.fffffffffffffp-1,
                             1};
  int fields;
  while ((fields = fscanf(f, "%7s %63s %63s", function, input, observed)) ==
         3) {
    char *end;
    double in = strtod(input, &end);
    if (end == input || *end || !isfinite(in) || rows >= 18 ||
        bits(in) != bits(expected[rows]) ||
        strcmp(function, rows < 9 ? "sqrt" : "acos"))
      return 2;
    double got = strtod(observed, &end);
    if (end == observed || *end || !isfinite(got) || got < 0)
      return 2;
    if (mpfr_set_d(x, in, MPFR_RNDN))
      return 2;
    if (!strcmp(function, "sqrt")) {
      mpfr_sqrt(lo, x, MPFR_RNDD);
      mpfr_sqrt(hi, x, MPFR_RNDU);
      mpfr_sqrt(ref, x, MPFR_RNDN);
    } else if (!strcmp(function, "acos")) {
      mpfr_acos(lo, x, MPFR_RNDD);
      mpfr_acos(hi, x, MPFR_RNDU);
      mpfr_acos(ref, x, MPFR_RNDN);
    } else
      return 2;
    double lower = mpfr_get_d(lo, MPFR_RNDN), upper = mpfr_get_d(hi, MPFR_RNDN);
    if (bits(lower) != bits(upper)) {
      fprintf(stderr, "MATHLAB-CHECK:oracle enclosure ambiguous\n");
      return 1;
    }
    uint64_t a = bits(got), b = bits(lower), distance = a > b ? a - b : b - a;
    if ((!strcmp(function, "sqrt") && distance != 0) || distance > 4) {
      fprintf(stderr,
              "MATHLAB-CHECK:bounded empirical screen %s distance=%" PRIu64
              "\n",
              function, distance);
      return 1;
    }
    mpfr_set_d(value, got, MPFR_RNDN);
    mpfr_sub(error, value, ref, MPFR_RNDN);
    mpfr_abs(error, error, MPFR_RNDN);
    if (mpfr_zero_p(ref)) {
      if (mpfr_zero_p(error))
        mpfr_set_zero(relative, 1);
      else
        mpfr_set_inf(relative, 1);
    } else
      mpfr_div(relative, error, ref, MPFR_RNDN);
    double step = nextafter(lower, INFINITY) - lower;
    mpfr_set_d(spacing, step, MPFR_RNDN);
    mpfr_div(ulp, error, spacing, MPFR_RNDN);
    printf("%s input=%s observed=%s nearest=%a steps=%" PRIu64 " ", function,
           input, observed, lower, distance);
    mpfr_printf("abs=%.20Re relative=%.20Re local_ULP=%.20Re\n", error,
                relative, ulp);
    ++rows;
  }
  if (fields != EOF || ferror(f) || fclose(f))
    return 2;
  mpfr_clears(x, lo, hi, ref, value, error, relative, spacing, ulp,
              (mpfr_ptr)0);
  mpfr_free_cache();
  if (rows != 18)
    return 2;
  puts("MATHLAB-PASS:18 independent interval-confirmed nearest results and "
       "explicit empirical screens");
  return 0;
}

#define _XOPEN_SOURCE 700
#include "conversion.h"
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "HOST-CONVERSION-CHECK:%d:%s\n", __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static struct lc_float convert(void *ctx, const char *s) {
  locale_t previous = uselocale(*(locale_t *)ctx);
  if (!previous)
    return (struct lc_float){0, 0, 1};
  errno = 0;
  char *end;
  double value = strtod(s, &end);
  int range = errno == ERANGE;
  size_t count = (size_t)(end - s);
  if (!uselocale(previous))
    return (struct lc_float){0, 0, 1};
  return (struct lc_float){value, count, range};
}
int main(void) {
  const char *inputs[] = {"0", "-12", "12x", "", "999"};
  const int expected[] = {0, -12, 12, 0, 999};
  const size_t consumed[] = {1, 3, 2, 0, 3};
  CHECK(setlocale(LC_ALL, "C"));
  for (unsigned i = 0; i < 5; ++i) {
    errno = 0;
    int a = atoi(inputs[i]);
    char *end;
    long b = strtol(inputs[i], &end, 10);
    CHECK(a == expected[i] && b == expected[i] &&
          (size_t)(end - inputs[i]) == consumed[i]);
    printf("INTEGER %u atoi=%d strtol=%ld consumed=%zu errno=%d\n", i, a, b,
           (size_t)(end - inputs[i]), errno);
  }
  const char *locales[] = {"C", "C.utf8"};
  const char *float_inputs[] = {"1.5", " 1.5", "infinity", "0x1.8p+0"};
  const size_t float_counts[] = {3, 4, 8, 8};
  int original = fegetround();
  CHECK(original != -1 && fesetround(FE_TONEAREST) == 0);
  locale_t ascii = newlocale(LC_ALL_MASK, "C", (locale_t)0);
  CHECK(ascii);
  for (unsigned l = 0; l < 2; ++l) {
    CHECK(setlocale(LC_ALL, locales[l]));
    printf("NUMERIC locale=%s decimal=%s rounding=%d radix=%d mantissa=%d\n",
           setlocale(LC_NUMERIC, NULL), localeconv()->decimal_point,
           fegetround(), FLT_RADIX, DBL_MANT_DIG);
    for (unsigned i = 0; i < 4; ++i) {
      char *end;
      errno = 0;
      double x = strtod(float_inputs[i], &end);
      CHECK((size_t)(end - float_inputs[i]) == float_counts[i] &&
            (i == 2 ? isinf(x) : x == 1.5));
      printf("FLOAT %u consumed=%zu value=%a errno=%d\n", i,
             (size_t)(end - float_inputs[i]), x, errno);
    }
    double result = 77;
    CHECK(lc_wire_float((const unsigned char *)"1.5e0", 5, convert, &ascii,
                        &result) == LC_OK &&
          result == 1.5);
    CHECK(!strcmp(setlocale(LC_NUMERIC, NULL), locales[l]));
    int32_t integer = 77;
    CHECK(lc_i8((const unsigned char *)"1.5", 3, 0, &integer) == LC_SYNTAX &&
          integer == 77);
  }
  CHECK(fesetround(original) == 0);
  freelocale(ascii);
  puts("HOST-CONVERSION-PASS:atoi-prefix:errno-end:locales:rounding:owned-"
       "locale-adapter");
  return 0;
}

#include "processlab.h"
#include <stdlib.h>
enum lt_status lt_cpu_interval(clock_t a, clock_t b, long double *out) {
  if (!out)
    return LT_ARGUMENT;
  if (a == (clock_t)-1 || b == (clock_t)-1)
    return LT_UNAVAILABLE;
  if (b < a || CLOCKS_PER_SEC <= 0)
    return LT_RANGE;
  long double result = (long double)b - (long double)a;
#ifndef LT_MUTANT_SCALE
  result /= (long double)CLOCKS_PER_SEC;
#endif
  *out = result;
  return LT_OK;
}
enum lt_status lt_environment(const char *name, char out[64]) {
  if (!name || !out)
    return LT_ARGUMENT;
  const char *p = getenv(name);
  if (!p)
    return LT_UNAVAILABLE;
  size_t n = 0;
  while (n < 64 && p[n])
    ++n;
  if (n == 64)
    return LT_RANGE;
#ifdef LT_MUTANT_COPY
  for (size_t i = 0; i < n; ++i)
    out[i] = p[i];
#else
  for (size_t i = 0; i <= n; ++i)
    out[i] = p[i];
#endif
  return LT_OK;
}
enum lt_status lt_calendar(int year, struct tm *out) {
  if (!out)
    return LT_ARGUMENT;
  if (year < 1970 || year > 2100)
    return LT_RANGE;
  struct tm date = {.tm_year = year - 1900,
                    .tm_mon = year == 2000 ? 1 : 0,
                    .tm_mday = year == 2000 ? 29 : 1,
                    .tm_isdst = -1};
  time_t value = mktime(&date);
  if (value == (time_t)-1)
    return LT_UNAVAILABLE;
  struct tm *p = gmtime(&value);
  if (!p)
    return LT_UNAVAILABLE;
  *out = *p;
  return LT_OK;
}

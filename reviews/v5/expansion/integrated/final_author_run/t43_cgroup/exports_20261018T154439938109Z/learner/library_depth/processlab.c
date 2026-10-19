#include "processlab.h"
enum lt_status lt_cpu_interval(clock_t a, clock_t b, long double *out) {
  (void)a;
  (void)b;
  (void)out;
  return LT_NOT_IMPLEMENTED;
}
enum lt_status lt_environment(const char *n, char out[64]) {
  (void)n;
  (void)out;
  return LT_NOT_IMPLEMENTED;
}
enum lt_status lt_calendar(int y, struct tm *out) {
  (void)y;
  (void)out;
  return LT_NOT_IMPLEMENTED;
}

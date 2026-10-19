#include "conversion.h"
enum lc_status lc_i8(const unsigned char *s, size_t n, int plus, int32_t *out) {
  (void)s;
  (void)n;
  (void)plus;
  (void)out;
  return LC_NOT_IMPLEMENTED;
}
enum lc_status lc_unsigned(const unsigned char *s, size_t n, int base,
                           uintmax_t max, uintmax_t *out) {
  (void)s;
  (void)n;
  (void)base;
  (void)max;
  (void)out;
  return LC_NOT_IMPLEMENTED;
}
enum lc_status lc_wire_float(const unsigned char *s, size_t n, lc_converter f,
                             void *c, double *out) {
  (void)s;
  (void)n;
  (void)f;
  (void)c;
  (void)out;
  return LC_NOT_IMPLEMENTED;
}

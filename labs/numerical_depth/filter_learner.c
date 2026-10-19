#include "checkpoint_numeric.h"
/* Implement this compatible producer as a T17 artifact. No later checkpoint
 * acceptance is a prerequisite. These stubs deliberately fail the tests. */
void cpn_exact_init(CPNExact *s) {
  s->previous = 0;
  s->older = 0;
}
void cpn_double_init(CPNDouble *s) {
  s->previous = 0;
  s->older = 0;
}
CPNStatus cpn_exact(CPNExact *s, const int32_t *x, size_t n, int64_t *y,
                    size_t cap) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  return CPN_INVALID;
}
CPNStatus cpn_double(CPNDouble *s, const double *x, size_t n, double *y,
                     size_t cap) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  return CPN_INVALID;
}
CPNStatus cpn_transaction(CPNDouble *s, const double *x, size_t n, double *y,
                          size_t cap, size_t fail, int atomic, size_t *done) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  (void)fail;
  (void)atomic;
  (void)done;
  return CPN_INVALID;
}
CPNStatus cpn_round_even(int64_t n, int32_t *out) {
  (void)n;
  (void)out;
  return CPN_INVALID;
}
CPNStatus cpn_iir_half(double *s, const double *x, size_t n, double *y,
                       size_t cap) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  return CPN_INVALID;
}

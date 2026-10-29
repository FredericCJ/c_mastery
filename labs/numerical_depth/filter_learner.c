#include "checkpoint_numeric.h"
/* Implement this compatible producer as a T17 artifact. No later checkpoint
 * acceptance is a prerequisite. These stubs deliberately fail the tests. */
void cpn_exact_init(struct cpn_exact *s) {
  s->previous = 0;
  s->older = 0;
}
void cpn_double_init(struct cpn_double *s) {
  s->previous = 0;
  s->older = 0;
}
enum cpn_status cpn_exact(struct cpn_exact *s, const int32_t *x, size_t n, int64_t *y,
                    size_t cap) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  return CPN_INVALID;
}
enum cpn_status cpn_double(struct cpn_double *s, const double *x, size_t n, double *y,
                     size_t cap) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  return CPN_INVALID;
}
enum cpn_status cpn_transaction(struct cpn_double *s, const double *x, size_t n, double *y,
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
enum cpn_status cpn_round_even(int64_t n, int32_t *out) {
  (void)n;
  (void)out;
  return CPN_INVALID;
}
enum cpn_status cpn_iir_half(double *s, const double *x, size_t n, double *y,
                       size_t cap) {
  (void)s;
  (void)x;
  (void)n;
  (void)y;
  (void)cap;
  return CPN_INVALID;
}

/* Deliberately incomplete learner implementation. */
#include "timer.h"
int td_init(TdTimer *t, unsigned n) {
  (void)t;
  (void)n;
  return -1;
}
int td_arm(TdTimer *t, uint64_t d, uint64_t p, int s, unsigned *g) {
  (void)t;
  (void)d;
  (void)p;
  (void)s;
  (void)g;
  return -1;
}
int td_begin(TdTimer *t, unsigned g, uint64_t n, TdDispatch *d) {
  (void)t;
  (void)g;
  (void)n;
  (void)d;
  return -1;
}
int td_finish(TdTimer *t, unsigned g) {
  (void)t;
  (void)g;
  return -1;
}
int td_cancel(TdTimer *t) {
  (void)t;
  return -1;
}
int td_earliest(const TdTimer *t, size_t n, size_t *i, uint64_t *d) {
  (void)t;
  (void)n;
  (void)i;
  (void)d;
  return -1;
}

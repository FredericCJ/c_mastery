/* Deliberately incomplete learner implementation. */
#include "timer.h"
int td_init(struct td_timer *t, unsigned n) {
  (void)t;
  (void)n;
  return -1;
}
int td_arm(struct td_timer *t, uint64_t d, uint64_t p, int s, unsigned *g) {
  (void)t;
  (void)d;
  (void)p;
  (void)s;
  (void)g;
  return -1;
}
int td_begin(struct td_timer *t, unsigned g, uint64_t n, struct td_dispatch *d) {
  (void)t;
  (void)g;
  (void)n;
  (void)d;
  return -1;
}
int td_finish(struct td_timer *t, unsigned g) {
  (void)t;
  (void)g;
  return -1;
}
int td_cancel(struct td_timer *t) {
  (void)t;
  return -1;
}
int td_earliest(const struct td_timer *t, size_t n, size_t *i, uint64_t *d) {
  (void)t;
  (void)n;
  (void)i;
  (void)d;
  return -1;
}

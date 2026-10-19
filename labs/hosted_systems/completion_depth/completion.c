/* Intentionally unfinished learner implementation. */
#include "completion.h"
int co_init(CoPool *p, unsigned char *b[4], size_t n, unsigned g) {
  (void)p;
  (void)b;
  (void)n;
  (void)g;
  return -1;
}
int co_acquire(CoPool *p, unsigned s, size_t n, uint64_t *t) {
  (void)p;
  (void)s;
  (void)n;
  (void)t;
  return -1;
}
int co_cancel(CoPool *p, uint64_t t, uint64_t *c) {
  (void)p;
  (void)t;
  (void)c;
  return -1;
}
int co_complete(CoPool *p, uint64_t t, int r, unsigned f) {
  (void)p;
  (void)t;
  (void)r;
  (void)f;
  return -1;
}
int co_release(CoPool *p, uint64_t t) {
  (void)p;
  (void)t;
  return -1;
}
int co_quiescent(const CoPool *p) {
  (void)p;
  return 0;
}

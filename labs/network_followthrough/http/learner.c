#include "http.h"
int nh_init(NhHttp *s, NbNet *n, const char *a, NhDate d, void *c, uint64_t t) {
  (void)s;
  (void)n;
  (void)a;
  (void)d;
  (void)c;
  (void)t;
  return NH_STUB;
}
int nh_step(NhHttp *s, uint64_t t) {
  (void)s;
  (void)t;
  return NH_STUB;
}

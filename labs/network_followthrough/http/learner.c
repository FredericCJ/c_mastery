#include "http.h"
int nh_init(struct nh_http *s, struct nb_net *n, const char *a, nh_date_fn d, void *c, uint64_t t) {
  (void)s;
  (void)n;
  (void)a;
  (void)d;
  (void)c;
  (void)t;
  return NH_STUB;
}
int nh_step(struct nh_http *s, uint64_t t) {
  (void)s;
  (void)t;
  return NH_STUB;
}

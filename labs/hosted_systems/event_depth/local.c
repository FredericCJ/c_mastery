#include "local.h"
#include <errno.h>
int el_loopback(int f, unsigned p, struct el_address *o) {
  (void)f;
  (void)p;
  (void)o;
  errno = ENOSYS;
  return -1;
}
int el_pathname(const char *p, struct el_address *o) {
  (void)p;
  (void)o;
  errno = ENOSYS;
  return -1;
}
int el_listener(struct el_ops o, const struct el_address *a, int *f) {
  (void)o;
  (void)a;
  (void)f;
  errno = ENOSYS;
  return -1;
}
int el_connect(struct el_ops o, const struct el_address *a, unsigned n, struct timespec d,
               int *f, unsigned *e) {
  (void)o;
  (void)a;
  (void)n;
  (void)d;
  (void)f;
  (void)e;
  errno = ENOSYS;
  return -1;
}
int el_accept(struct el_ops o, int f, int (*c)(void *, int), void *x, int *out) {
  (void)o;
  (void)f;
  (void)c;
  (void)x;
  (void)out;
  errno = ENOSYS;
  return -1;
}
struct el_ops el_system_ops(void) { return (struct el_ops){0}; }

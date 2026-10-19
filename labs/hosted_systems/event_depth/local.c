#include "local.h"
#include <errno.h>
int el_loopback(int f, unsigned p, ElAddress *o) {
  (void)f;
  (void)p;
  (void)o;
  errno = ENOSYS;
  return -1;
}
int el_pathname(const char *p, ElAddress *o) {
  (void)p;
  (void)o;
  errno = ENOSYS;
  return -1;
}
int el_listener(ElOps o, const ElAddress *a, int *f) {
  (void)o;
  (void)a;
  (void)f;
  errno = ENOSYS;
  return -1;
}
int el_connect(ElOps o, const ElAddress *a, unsigned n, struct timespec d,
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
int el_accept(ElOps o, int f, int (*c)(void *, int), void *x, int *out) {
  (void)o;
  (void)f;
  (void)c;
  (void)x;
  (void)out;
  errno = ENOSYS;
  return -1;
}
ElOps el_system_ops(void) { return (ElOps){0}; }

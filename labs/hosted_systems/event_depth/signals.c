#include "signals.h"
#include <errno.h>
int es_counted_push(EsCounted *q, unsigned v) {
  (void)q;
  (void)v;
  return -1;
}
int es_counted_pop(EsCounted *q, unsigned *v) {
  (void)q;
  (void)v;
  return -1;
}
int es_open(EsSignals *p) {
  (void)p;
  errno = ENOSYS;
  return -1;
}
int es_wait(EsSignals *p, const struct timespec *t) {
  (void)p;
  (void)t;
  errno = ENOSYS;
  return -1;
}
unsigned es_take(EsSignals *p, unsigned selected) {
  (void)p;
  (void)selected;
  return 0;
}
int es_close(EsSignals *p) {
  (void)p;
  errno = ENOSYS;
  return -1;
}
int es_remaining(struct timespec n, struct timespec d, struct timespec *r) {
  (void)n;
  (void)d;
  (void)r;
  return -1;
}
int es_wait_until(EsWait ops, struct timespec d) {
  (void)ops;
  (void)d;
  errno = ENOSYS;
  return -1;
}
int es_wait_fd(int fd, struct timespec d) {
  (void)fd;
  (void)d;
  errno = ENOSYS;
  return -1;
}
int es_reap(pid_t *p, int *s, unsigned n) {
  (void)p;
  (void)s;
  (void)n;
  errno = ENOSYS;
  return -1;
}

#include "worker.h"
#include <errno.h>
int sd_exclusive(int d, const char *n, int *o) {
  (void)d;
  (void)n;
  (void)o;
  return ENOSYS;
}
int sd_request(unsigned a, enum sd_operation op, int fd, char o[4]) {
  (void)a;
  (void)op;
  (void)fd;
  (void)o;
  return ENOSYS;
}
int sd_spawn(const char *p, char *const a[], char *const e[], int l, int t,
             unsigned f, struct sd_child *o) {
  (void)p;
  (void)a;
  (void)e;
  (void)l;
  (void)t;
  (void)f;
  (void)o;
  return ENOSYS;
}

#include "async.h"
int sa_start(struct sa_read *s, struct sa_port p, uint32_t n, void *b) {
  (void)s;
  (void)p;
  (void)n;
  (void)b;
  return SA_UNIMPLEMENTED;
}
int sa_abandon(struct sa_read *s) {
  (void)s;
  return SA_UNIMPLEMENTED;
}
int sa_finish(struct sa_read *s) {
  (void)s;
  return SA_UNIMPLEMENTED;
}
int sa_release(struct sa_read *s, void **p) {
  (void)s;
  (void)p;
  return SA_UNIMPLEMENTED;
}

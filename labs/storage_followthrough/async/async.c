#include "async.h"
int sa_start(SaRead *s, SaPort p, uint32_t n, void *b) {
  (void)s;
  (void)p;
  (void)n;
  (void)b;
  return SA_UNIMPLEMENTED;
}
int sa_abandon(SaRead *s) {
  (void)s;
  return SA_UNIMPLEMENTED;
}
int sa_finish(SaRead *s) {
  (void)s;
  return SA_UNIMPLEMENTED;
}
int sa_release(SaRead *s, void **p) {
  (void)s;
  (void)p;
  return SA_UNIMPLEMENTED;
}

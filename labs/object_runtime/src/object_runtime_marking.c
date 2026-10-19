#include "object_runtime_marking.h"
void orcm_init(ORCHeap *h, ORCPolicy p) {
  (void)h;
  (void)p;
}
int orcm_invariant(const ORCHeap *h) {
  (void)h;
  return 0;
}
ORStatus orcm_allocate(ORCHeap *h, ORCHandle a, ORCHandle b, ORCHandle *o) {
  (void)h;
  (void)a;
  (void)b;
  (void)o;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_retain(ORCHeap *h, ORCHandle p) {
  (void)h;
  (void)p;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_release(ORCHeap *h, ORCHandle p) {
  (void)h;
  (void)p;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_store(ORCHeap *h, ORCHandle o, unsigned f, ORCHandle t) {
  (void)h;
  (void)o;
  (void)f;
  (void)t;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_read(const ORCHeap *h, ORCHandle o, ORCHandle e[2]) {
  (void)h;
  (void)o;
  (void)e;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_begin(ORCHeap *h) {
  (void)h;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_step(ORCHeap *h, unsigned n, unsigned *s) {
  (void)h;
  (void)n;
  (void)s;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_prepare(const ORCHeap *h, ORCFinish *t) {
  (void)h;
  (void)t;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_commit(ORCHeap *h, ORCFinish t, uint16_t *d) {
  (void)h;
  (void)t;
  (void)d;
  return OR_UNIMPLEMENTED;
}

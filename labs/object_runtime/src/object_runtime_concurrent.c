#include "object_runtime_concurrent.h"
ORStatus orcm_concurrent_init(ORCConcurrent *c, ORCPolicy p) {
  (void)c;
  (void)p;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_concurrent_call(ORCConcurrent *c, const ORCRequest *q,
                              ORCResponse *r) {
  (void)c;
  (void)q;
  (void)r;
  return OR_UNIMPLEMENTED;
}
ORStatus orcm_concurrent_close(ORCConcurrent *c) {
  (void)c;
  return OR_UNIMPLEMENTED;
}

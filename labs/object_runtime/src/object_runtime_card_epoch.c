#include "object_runtime_card_epoch.h"
ORStatus orcc_init(ORCCards *c, unsigned w, int p) {
  (void)c;
  (void)w;
  (void)p;
  return OR_STATE;
}
ORStatus orcc_store(ORCCards *c, unsigned o, unsigned f, unsigned t) {
  (void)c;
  (void)o;
  (void)f;
  (void)t;
  return OR_STATE;
}
ORStatus orcc_prepare(ORCCards *c, unsigned n, ORCCSnapshot *s) {
  (void)c;
  (void)n;
  (void)s;
  return OR_STATE;
}
ORStatus orcc_scan_commit(ORCCards *c, const ORCCSnapshot *s, uint16_t *r,
                          unsigned *n) {
  (void)c;
  (void)s;
  (void)r;
  (void)n;
  return OR_STATE;
}
ORStatus orcc_close(ORCCards *c) {
  (void)c;
  return OR_STATE;
}

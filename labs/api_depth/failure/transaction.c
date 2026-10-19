#include "transaction.h"
AeUpdate ae_replace(AeConfig *c, const unsigned *p, unsigned n, unsigned z,
                    AeGuarantee g, unsigned f, unsigned e, AeObserve o,
                    void *x) {
  (void)c;
  (void)p;
  (void)n;
  (void)z;
  (void)g;
  (void)f;
  (void)e;
  (void)o;
  (void)x;
  return (AeUpdate){AE_UNIMPLEMENTED, 0, 0};
}
AeCode ae_journal(AeJournal *j, unsigned i, unsigned d, unsigned c) {
  (void)j;
  (void)i;
  (void)d;
  (void)c;
  return AE_UNIMPLEMENTED;
}

#include "publication.h"
AePublication *ae_publication_new(const AeConfig *c) {
  (void)c;
  return 0;
}
AeCode ae_publication_replace(AePublication *p, const AeConfig *c, unsigned f) {
  (void)p;
  (void)c;
  (void)f;
  return AE_UNIMPLEMENTED;
}
AeCode ae_publication_acquire(AePublication *p, AeReadLease *l) {
  (void)p;
  (void)l;
  return AE_UNIMPLEMENTED;
}
AeCode ae_publication_read(const AeReadLease *l, AeConfig *c) {
  (void)l;
  (void)c;
  return AE_UNIMPLEMENTED;
}
AeCode ae_publication_release(AeReadLease *l) {
  (void)l;
  return AE_UNIMPLEMENTED;
}
AeCode ae_publication_counts(AePublication *p, unsigned *m, unsigned *f,
                             unsigned *l) {
  (void)p;
  (void)m;
  (void)f;
  (void)l;
  return AE_UNIMPLEMENTED;
}
AeCode ae_publication_close(AePublication *p) {
  (void)p;
  return AE_UNIMPLEMENTED;
}
AeCode ae_publication_destroy(AePublication **p) {
  (void)p;
  return AE_UNIMPLEMENTED;
}

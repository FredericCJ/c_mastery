#include "ethernet.h"
int dg_ethernet_build(const DGMessage *m, uint8_t *p, size_t n, size_t *w) {
  (void)m;
  (void)p;
  (void)n;
  (void)w;
  return DG_UNIMPLEMENTED;
}
int dg_ethernet_accept(DGSession *s, const uint8_t *p, size_t n, DGMessage *m) {
  (void)s;
  (void)p;
  (void)n;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_descriptors(DGSession *s, EDRx *r, const EDDescriptor *d, size_t n,
                   DGMessage *m) {
  (void)s;
  (void)r;
  (void)d;
  (void)n;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_publish(EDTx *t, unsigned s, uint32_t g, const DGMessage *m,
               void (*b)(void *), void *c) {
  (void)t;
  (void)s;
  (void)g;
  (void)m;
  (void)b;
  (void)c;
  return DG_UNIMPLEMENTED;
}

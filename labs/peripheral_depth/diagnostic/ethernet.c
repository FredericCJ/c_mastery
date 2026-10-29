#include "ethernet.h"
int dg_ethernet_build(const struct dg_message *m, uint8_t *p, size_t n, size_t *w) {
  (void)m;
  (void)p;
  (void)n;
  (void)w;
  return DG_UNIMPLEMENTED;
}
int dg_ethernet_accept(struct dg_session *s, const uint8_t *p, size_t n, struct dg_message *m) {
  (void)s;
  (void)p;
  (void)n;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_descriptors(struct dg_session *s, struct ed_rx *r, const struct ed_descriptor *d, size_t n,
                   struct dg_message *m) {
  (void)s;
  (void)r;
  (void)d;
  (void)n;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_publish(struct ed_tx *t, unsigned s, uint32_t g, const struct dg_message *m,
               void (*b)(void *), void *c) {
  (void)t;
  (void)s;
  (void)g;
  (void)m;
  (void)b;
  (void)c;
  return DG_UNIMPLEMENTED;
}

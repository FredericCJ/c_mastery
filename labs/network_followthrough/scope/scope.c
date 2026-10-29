#include "scope.h"
int nw_lengths(uint32_t p, struct nw_lengths *o) {
  (void)p;
  (void)o;
  return NW_UNIMPLEMENTED;
}
int nw_tx_init(struct nw_tx *s, uint32_t e, uint32_t o) {
  (void)s;
  (void)e;
  (void)o;
  return NW_UNIMPLEMENTED;
}
int nw_offer(struct nw_tx *s, const void *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_prepare(struct nw_tx *s, unsigned m, unsigned w, struct nw_segment *o) {
  (void)s;
  (void)m;
  (void)w;
  (void)o;
  return NW_UNIMPLEMENTED;
}
int nw_submit(struct nw_tx *s, struct nw_ticket t, unsigned a) {
  (void)s;
  (void)t;
  (void)a;
  return NW_UNIMPLEMENTED;
}
int nw_dma_done(struct nw_tx *s, struct nw_ticket t) {
  (void)s;
  (void)t;
  return NW_UNIMPLEMENTED;
}
int nw_ack(struct nw_tx *s, struct nw_ticket t, uint32_t n) {
  (void)s;
  (void)t;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_init(struct nw_rx *s, unsigned n) {
  (void)s;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_offer(struct nw_rx *s, const void *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_peek(const struct nw_rx *s, const unsigned char **p, size_t *n) {
  (void)s;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_consume(struct nw_rx *s, size_t n) {
  (void)s;
  (void)n;
  return NW_UNIMPLEMENTED;
}
unsigned nw_rx_window(const struct nw_rx *s) {
  (void)s;
  return 0;
}

#include "scope.h"
int nw_lengths(uint32_t p, NwLengths *o) {
  (void)p;
  (void)o;
  return NW_UNIMPLEMENTED;
}
int nw_tx_init(NwTx *s, uint32_t e, uint32_t o) {
  (void)s;
  (void)e;
  (void)o;
  return NW_UNIMPLEMENTED;
}
int nw_offer(NwTx *s, const void *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_prepare(NwTx *s, unsigned m, unsigned w, NwSegment *o) {
  (void)s;
  (void)m;
  (void)w;
  (void)o;
  return NW_UNIMPLEMENTED;
}
int nw_submit(NwTx *s, NwTicket t, unsigned a) {
  (void)s;
  (void)t;
  (void)a;
  return NW_UNIMPLEMENTED;
}
int nw_dma_done(NwTx *s, NwTicket t) {
  (void)s;
  (void)t;
  return NW_UNIMPLEMENTED;
}
int nw_ack(NwTx *s, NwTicket t, uint32_t n) {
  (void)s;
  (void)t;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_init(NwRx *s, unsigned n) {
  (void)s;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_offer(NwRx *s, const void *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_peek(const NwRx *s, const unsigned char **p, size_t *n) {
  (void)s;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}
int nw_rx_consume(NwRx *s, size_t n) {
  (void)s;
  (void)n;
  return NW_UNIMPLEMENTED;
}
unsigned nw_rx_window(const NwRx *s) {
  (void)s;
  return 0;
}

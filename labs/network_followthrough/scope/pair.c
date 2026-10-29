#include "pair.h"
int nw_pair_init(struct nw_pair *s, unsigned n, uint32_t e, uint16_t a, uint16_t b) {
  (void)s;
  (void)n;
  (void)e;
  (void)a;
  (void)b;
  return NW_UNIMPLEMENTED;
}
int nw_pair_receive(struct nw_pair *s, const void *p, size_t n, uint64_t now,
                    struct net_packet *out) {
  (void)s;
  (void)p;
  (void)n;
  (void)now;
  (void)out;
  return NW_UNIMPLEMENTED;
}

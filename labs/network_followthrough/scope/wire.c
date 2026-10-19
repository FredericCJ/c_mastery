#include "wire.h"
int nw_ingress(NwRx *q, uint32_t *next, uint16_t port, const unsigned char *p,
               size_t n) {
  (void)q;
  (void)next;
  (void)port;
  (void)p;
  (void)n;
  return NW_UNIMPLEMENTED;
}

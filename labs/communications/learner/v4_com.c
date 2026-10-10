#include "v4_com.h"
int v4_reassembly(V4Reassembly *s, const EduFragment *f, EduFragment *a,
                  uint64_t n, unsigned g) {
  (void)s;
  (void)f;
  (void)a;
  (void)n;
  (void)g;
  return -1;
}
int v4_arp_queue(V4Arp *a, uint32_t p, const uint8_t *b, size_t n, uint64_t t) {
  (void)a;
  (void)p;
  (void)b;
  (void)n;
  (void)t;
  return -1;
}
int v4_arp_poll(V4Arp *a, uint64_t t) {
  (void)a;
  (void)t;
  return -1;
}
int v4_arp_resolve(V4Arp *a, uint32_t p) {
  (void)a;
  (void)p;
  return -1;
}
int v4_tcp_delivery(NetStack *s, uint64_t n, V4PacketSink f, void *p,
                    unsigned *d) {
  (void)s;
  (void)n;
  (void)f;
  (void)p;
  (void)d;
  return -1;
}
int v4_can_init(V4CanIo i, uint32_t n, uint32_t d, unsigned p) {
  (void)i;
  (void)n;
  (void)d;
  (void)p;
  return -1;
}
int v4_can_loopback(V4CanIo i, const uint8_t *p, size_t n, uint8_t *o, size_t c,
                    unsigned t) {
  (void)i;
  (void)p;
  (void)n;
  (void)o;
  (void)c;
  (void)t;
  return -1;
}

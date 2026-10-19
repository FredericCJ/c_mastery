#include "arp.h"
int na_init(NaArp *s, NaConfig c, uint32_t epoch) {
  (void)s;
  (void)c;
  (void)epoch;
  return NA_UNIMPLEMENTED;
}
int na_queue(NaArp *s, const void *p, size_t n, uint64_t now) {
  (void)s;
  (void)p;
  (void)n;
  (void)now;
  return NA_UNIMPLEMENTED;
}
int na_receive(NaArp *s, const void *p, size_t n, uint64_t now) {
  (void)s;
  (void)p;
  (void)n;
  (void)now;
  return NA_UNIMPLEMENTED;
}
int na_prepare(NaArp *s, uint64_t now, NetPacket *p, NaTicket *t) {
  (void)s;
  (void)now;
  (void)p;
  (void)t;
  return NA_UNIMPLEMENTED;
}
int na_submit(NaArp *s, NaTicket t, unsigned accepted) {
  (void)s;
  (void)t;
  (void)accepted;
  return NA_UNIMPLEMENTED;
}

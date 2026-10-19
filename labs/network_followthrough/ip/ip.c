#include "ip.h"
int ni_expire(NiReassembly *s, uint64_t now, NiPacket out[2], unsigned *count) {
  (void)s;
  (void)now;
  (void)out;
  (void)count;
  return NI_STUB;
}
int ni_parse(const NiConfig *a, const unsigned char *b, size_t c, unsigned d,
             NiView *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return NI_STUB;
}
int ni_echo(const NiConfig *a, const unsigned char *b, size_t c, NiPacket *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return NI_STUB;
}
int ni_init(NiReassembly *a, NiConfig b, unsigned c, unsigned d, uint64_t e,
            uint64_t f) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  return NI_STUB;
}
int ni_tick(NiReassembly *a, uint64_t b) {
  (void)a;
  (void)b;
  return NI_STUB;
}
int ni_receive(NiReassembly *a, const unsigned char *b, size_t c, uint64_t d,
               unsigned *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return NI_STUB;
}
int ni_take(NiReassembly *a, unsigned b, NiPacket *c) {
  (void)a;
  (void)b;
  (void)c;
  return NI_STUB;
}

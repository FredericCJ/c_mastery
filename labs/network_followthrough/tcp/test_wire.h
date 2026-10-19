#ifndef NC_TEST_WIRE_H
#define NC_TEST_WIRE_H
#include "tcp.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("NC_CHECK " #x);                                                    \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static void nc_w16(unsigned char *p, unsigned v) {
  p[0] = (unsigned char)(v >> 8);
  p[1] = (unsigned char)v;
}
static void nc_w32(unsigned char *p, uint32_t v) {
  nc_w16(p, v >> 16);
  nc_w16(p + 2, v);
}
static unsigned nc_u16(const unsigned char *p) {
  return (unsigned)p[0] * 256 + p[1];
}
/* Independent test construction uses a byte-position sum, not SChecksum. */
static unsigned nc_sum(const unsigned char *p, size_t n, uint32_t initial) {
  uint32_t s = initial;
  for (size_t i = 0; i < n; i++)
    s += (uint32_t)p[i] << ((i % 2) ? 0 : 8);
  while (s >> 16)
    s = (s & 65535) + (s >> 16);
  return (~s) & 65535;
}
static void nc_fix(unsigned char *p, size_t n) {
  nc_w16(p + 10, 0);
  nc_w16(p + 10, nc_sum(p, 20, 0));
  nc_w16(p + 36, 0);
  unsigned pseudo = nc_u16(p + 12) + nc_u16(p + 14) + nc_u16(p + 16) +
                    nc_u16(p + 18) + 6 + (unsigned)n - 20;
  nc_w16(p + 36, nc_sum(p + 20, n - 20, pseudo));
}
static size_t nc_wire(unsigned char *p, uint32_t seq, uint32_t ack,
                      unsigned flags, unsigned wnd, const void *data, size_t n,
                      unsigned mss) {
  size_t h = mss ? 24 : 20, total = 20 + h + n;
  memset(p, 0, total);
  p[0] = 0x45;
  nc_w16(p + 2, (unsigned)total);
  nc_w16(p + 6, 0x4000);
  p[8] = 64;
  p[9] = 6;
  nc_w32(p + 12, 0xc0000201);
  nc_w32(p + 16, 0xc0000202);
  nc_w16(p + 20, 40000);
  nc_w16(p + 22, 8080);
  nc_w32(p + 24, seq);
  nc_w32(p + 28, ack);
  p[32] = (unsigned char)(h << 2);
  p[33] = (unsigned char)flags;
  nc_w16(p + 34, wnd);
  if (mss) {
    p[40] = 2;
    p[41] = 4;
    nc_w16(p + 42, mss);
  }
  if (n)
    memcpy(p + 20 + h, data, n);
  nc_fix(p, total);
  return total;
}
static int nc_establish(NcStack *s, unsigned cap, uint32_t isn, uint32_t peer,
                        unsigned mss) {
  unsigned char p[300];
  NcPacket out;
  NiConfig config = {0xc0000202, 0xc0000201};
  CHECK(nc_init(s, config, 7, isn, cap, 0) == NC_OK);
  size_t n = nc_wire(p, peer, 0, 2, 512, 0, 0, mss);
  CHECK(nc_receive(s, p, n, 0) == NC_OK);
  CHECK(nc_prepare(s, &out) == NC_OK);
  CHECK(!s->flight && s->snd_nxt == isn);
  CHECK(nc_submit(s, out.ticket, 1, 0) == NC_OK);
  CHECK(nc_dma_done(s, out.ticket) == NC_OK);
  n = nc_wire(p, peer + 1, isn + 1, 16, 512, 0, 0, 0);
  CHECK(nc_receive(s, p, n, 80) == NC_OK);
  CHECK(s->state == NC_ESTABLISHED && !s->flight);
  return 0;
}
#endif

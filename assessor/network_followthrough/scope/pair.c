#include "pair.h"
#ifndef NW_PAIR_MUTANT
#define NW_PAIR_MUTANT 0
#endif
int nw_pair_init(NwPair *s, unsigned cap, uint32_t epoch, uint16_t a,
                 uint16_t b) {
  if (!s || cap < 1 || cap > 2 || !epoch || !a || !b || a == b)
    return NW_INVALID;
#if NW_PAIR_MUTANT == 2
  *s = (NwPair){.ports = {a, b}, .capacity = cap, .epoch = epoch};
#else
  s->ports[0] = a;
  s->ports[1] = b;
  s->capacity = cap;
  s->epoch = epoch;
#endif
  for (unsigned i = 0; i < 2; ++i)
    net_init(&s->flows[i], 1000 + 65536 * i);
  return NW_OK;
}
int nw_pair_receive(NwPair *s, const void *data, size_t n, uint64_t now,
                    NetPacket *out) {
  if (!s || !data || !out || s->capacity < 1 || s->capacity > 2 || n < 14)
    return NW_INVALID;
  const unsigned char *p = data;
  if (p[12] == 8 && p[13] == 6) {
    /* Shared peer-neighbor event is copied into each bounded historical stack.
     * Exactly one ARP response is emitted, never one response per TCP flow. */
    for (unsigned i = 0; i < s->capacity; ++i) {
      NetPacket ignored;
      if (net_receive(&s->flows[i], p, n, now, i ? &ignored : out) < 0)
        return NW_INVALID;
    }
    return NW_OK;
  }
  if (p[12] != 8 || p[13] != 0 || n < 54)
    return NW_INVALID;
  unsigned port = (unsigned)p[34] * 256 + p[35];
  for (unsigned i = 0; i < s->capacity; ++i)
    if (s->ports[i] == port) {
      unsigned selected = NW_PAIR_MUTANT == 1 ? 0 : i;
      return net_receive(&s->flows[selected], p, n, now, out) < 0 ? NW_INVALID
                                                                  : NW_OK;
    }
  return NW_BUSY;
}

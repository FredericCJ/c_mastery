#include "handoff.h"
#include <string.h>
#ifndef EH_MUTANT
#define EH_MUTANT 0
#endif
static int unicast(uint32_t ip) {
  unsigned a = ip >> 24;
  return a && a < 224 && a != 127;
}
static int host(uint32_t ip, uint32_t mask) {
  uint32_t h = ip & ~mask;
  return unicast(ip) && h && h != ~mask;
}
int eh_next(const struct eh_route *r, uint32_t dst, uint32_t *out) {
  uint32_t inv, next;
  if (!r || !out)
    return EH_INVALID;
  inv = ~r->mask;
  if (inv < 3 || inv > 0x7fffffffu || (inv & (inv + 1)) ||
      !host(r->local, r->mask) || !unicast(dst))
    return EH_INVALID;
  if ((dst & r->mask) == (r->local & r->mask)) {
    if (!host(dst, r->mask) || dst == r->local)
      return EH_INVALID;
    next = dst;
  } else {
    if (!r->gateway)
      return EH_NO_ROUTE;
    if (!host(r->gateway, r->mask) || r->gateway == r->local ||
        (r->gateway & r->mask) != (r->local & r->mask))
      return EH_INVALID;
    next = r->gateway;
  }
  *out = EH_MUTANT == 1 ? dst : next;
  return EH_OK;
}
static uint32_t ip(const unsigned char *p) {
  return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
         p[3];
}
int eh_frame(const struct eh_route *r, const unsigned char *mac, const struct eh_neighbor *n,
             const unsigned char *p, size_t size, unsigned char *out,
             size_t cap, size_t *written) {
  uint32_t next, sum = 0;
  unsigned i, any = 0;
  int e;
  if (!r || !mac || !n || !p || !out || !written || size < 20 || size > 1500 ||
      p[0] != 0x45 || ((unsigned)p[2] * 256 + p[3]) != size ||
      ip(p + 12) != r->local)
    return EH_INVALID;
  for (i = 0; i < 20; i += 2)
    sum += (unsigned)p[i] * 256 + p[i + 1];
  while (sum >> 16)
    sum = (sum & 65535) + (sum >> 16);
  if (sum != 65535)
    return EH_INVALID;
  e = eh_next(r, ip(p + 16), &next);
  if (e != EH_OK)
    return e;
  if (next != n->address)
    return EH_NEIGHBOR;
  for (i = 0; i < 6; ++i)
    any |= n->mac[i];
  if (!any || (n->mac[0] & 1))
    return EH_NEIGHBOR;
  e = ed_frame_build(n->mac, mac, 0x800, p, size, 0, out, cap, written);
  if (e != ED_OK)
    return e == ED_FULL ? EH_FULL : EH_INVALID;
  if (EH_MUTANT == 2) {
    out[30] = (unsigned char)(next >> 24);
    out[31] = (unsigned char)(next >> 16);
    out[32] = (unsigned char)(next >> 8);
    out[33] = (unsigned char)next;
  }
  return EH_OK;
}
int eh_correlate(const struct eh_clock *c, const uint32_t *t, size_t n, uint64_t *lower,
                 uint64_t *upper) {
  uint64_t lo[16], hi[16];
  size_t i;
  if (!c || n > 16 || (!t && n) || (!lower && n) || (!upper && n) ||
      c->lower > c->upper || c->ppm > 1000)
    return EH_INVALID;
  for (i = 0; i < n; ++i) {
    uint32_t dt, q, r, frac, drift;
    if (t[i] < c->anchor || (i && t[i] < t[i - 1]))
      return EH_INVALID;
    dt = t[i] - c->anchor;
    q = dt / 1000000;
    r = dt % 1000000;
    frac = r * c->ppm;
    drift = q * c->ppm + frac / 1000000 + (frac % 1000000 != 0);
    if (EH_MUTANT == 3)
      drift = 0;
    if (c->upper > UINT64_MAX - dt || c->upper + dt > UINT64_MAX - drift ||
        c->lower > UINT64_MAX - dt || c->lower + dt < drift)
      return EH_INVALID;
    lo[i] = c->lower + dt - drift;
    hi[i] = c->upper + dt + drift;
  }
  for (i = 0; i < n; ++i) {
    lower[i] = lo[i];
    upper[i] = hi[i];
  }
  return EH_OK;
}

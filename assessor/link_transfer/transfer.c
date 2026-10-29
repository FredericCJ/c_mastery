#include "transfer.h"
#ifndef LT_MUTANT
#define LT_MUTANT 0
#endif
int lt_order(unsigned m, unsigned a, unsigned b, int *out) {
  if (!out || m < 2 || m > 65535 || a >= m || b >= m)
    return LT_INVALID;
  unsigned d = b >= a ? b - a : m - (a - b);
  if (LT_MUTANT == 1)
    d = (b - a) & (m - 1);
  if (d && !(m & 1) && d == m / 2)
    return LT_AMBIGUOUS;
  *out = d == 0 ? 0 : d <= m / 2 ? -1 : 1;
  return LT_OK;
}
int lt_ack(unsigned m, unsigned *base, unsigned *outstanding, unsigned next) {
  if (!base || !outstanding || m < 2 || m > 65535 || *base >= m || next >= m ||
      *outstanding > (m - 1) / 2)
    return LT_INVALID;
  unsigned d = next >= *base ? next - *base : m - (*base - next);
  if (d > *outstanding)
    return LT_STALE;
  *base = next;
  *outstanding -= d;
  return LT_OK;
}
int lt_sizes(uint32_t n, const struct lt_layer *p, size_t count, uint32_t *out) {
  if (count > 8 || (count && (!p || !out)))
    return LT_INVALID;
  uint32_t staged[8];
  for (size_t i = 0; i < count; i++) {
    uint32_t a = p[i].alignment;
    if (!a || a > 256 || p[i].tag > 64)
      return LT_INVALID;
    if (p[i].header > UINT32_MAX - n)
      return LT_FULL;
    n += p[i].header;
    uint32_t rem = n % a, pad = rem ? a - rem : 0;
    if (pad > UINT32_MAX - n)
      return LT_FULL;
    n += pad;
    uint32_t trailer = n / 16 + (n % 16 != 0) + p[i].tag;
    if (LT_MUTANT == 2)
      trailer = p[i].tag;
    if (trailer > UINT32_MAX - n)
      return LT_FULL;
    n += trailer;
    staged[i] = n;
  }
  for (size_t i = 0; i < count; i++)
    out[i] = staged[i];
  return LT_OK;
}
int lt_begin(struct lt_borrow *s, const unsigned char *p, size_t n) {
  if (!s || (!p && n) || n > 256)
    return LT_INVALID;
  if (s->phase)
    return LT_BUSY;
  struct lt_borrow t = {0};
  t.bytes = p;
  t.length = n;
  t.phase = 1;
  t.prefix[0] = (unsigned char)(n >> 8);
  t.prefix[1] = (unsigned char)n;
  *s = t;
  return LT_OK;
}
int lt_offer(struct lt_borrow *s, const unsigned char **p, size_t *n) {
  if (!s || !p || !n)
    return LT_INVALID;
  if (s->phase != 1)
    return s->phase == 2 ? LT_DONE : LT_BUSY;
  if (s->used < 2) {
    *p = s->prefix + s->used;
    *n = 2 - s->used;
  } else {
    *p = s->bytes + (s->used - 2);
    *n = s->length - (s->used - 2);
  }
  s->offered = *n;
  return LT_OK;
}
int lt_accept(struct lt_borrow *s, size_t n) {
  if (!s || s->phase != 1 || n > s->offered)
    return LT_INVALID;
  if (LT_MUTANT == 3 && n)
    n = s->offered;
  s->used += n;
  s->offered = 0;
  if (s->used == s->length + 2) {
    s->phase = 2;
    s->bytes = 0;
    return LT_DONE;
  }
  return LT_OK;
}
int lt_cancel(struct lt_borrow *s, unsigned reset) {
  if (!s || reset > 1)
    return LT_INVALID;
  if (!s->phase)
    return LT_OK;
  if (s->phase == 2 || !s->used || reset) {
    struct lt_borrow z = {0};
    *s = z;
    return LT_OK;
  }
  s->phase = 3;
  return LT_BUSY;
}
int lt_bridge(struct lt_bridge *s, unsigned vid, const uint8_t *src, const uint8_t *dst,
              unsigned port, uint32_t now, unsigned *mask) {
  if (!s || !src || !dst || !mask || (vid != 10 && vid != 20) || now < s->now)
    return LT_INVALID;
  struct lt_bridge t = *s;
  unsigned selected = vid == 20, result = 0;
  if (LT_MUTANT == 4)
    selected = 0;
  int r = ed_bridge(&t.table[selected], src, dst, port, now, &result);
  if (r != ED_OK)
    return r == ED_FULL ? LT_FULL : LT_INVALID;
  t.now = now;
  *s = t;
  *mask = result;
  return LT_OK;
}
int lt_dp83848(const struct ed_bus *b, uint32_t h, unsigned phy, unsigned budget,
               uint32_t timeout, struct ed_phy *out) {
  if (!out)
    return ED_INVALID;
  uint16_t id1, id2, first, current, status;
  int r;
  if ((r = ed_mdio_read(b, h, phy, 2, budget, timeout, &id1)) != ED_OK)
    return r;
  if ((r = ed_mdio_read(b, h, phy, 3, budget, timeout, &id2)) != ED_OK)
    return r;
  /* This source revision documents MDL_REV0. A future silicon revision is
   * refused until its compatibility/errata evidence is separately inspected. */
  if (id1 != 0x2000 || id2 != 0x5c90)
    return ED_WRONG_PHY;
  if ((r = ed_mdio_read(b, h, phy, 1, budget, timeout, &first)) != ED_OK)
    return r;
  if ((r = ed_mdio_read(b, h, phy, 1, budget, timeout, &current)) != ED_OK)
    return r;
  if (!(current & 4))
    return ED_LINK_DOWN;
  if (!(current & 32))
    return ED_NEGOTIATING;
  if ((r = ed_mdio_read(b, h, phy, 16, budget, timeout, &status)) != ED_OK)
    return r;
  if (!(status & 1))
    return ED_LINK_DOWN;
  if (!(status & 16))
    return ED_NEGOTIATING;
  if ((status & 8) || !(status & 4))
    return ED_UNSUPPORTED;
  struct ed_phy t = {(uint8_t)((first & 4) != 0), 1, 1,
             (uint16_t)((status & 2) ? 10 : 100)};
  if (LT_MUTANT == 5)
    t.speed = (status & 2) ? 100 : 10;
  *out = t;
  return ED_OK;
}
int lt_partial_begin(struct lt_partial *s, struct e7_sender *sender, uint32_t now) {
  if (!s || !sender || now > UINT32_MAX - 500)
    return LT_INVALID;
  if (s->active)
    return LT_BUSY;
  struct lt_partial t = {0};
  struct e7_sender staged = *sender;
  struct e7_fragment f;
  uint32_t token;
  int r = e7_send_offer(&staged, now, &f, &token);
  if (r != E7_OK)
    return r == E7_WAIT || r == E7_BUSY ? LT_BUSY : LT_INVALID;
  struct edu_fragment ef = {0};
  ef.type = f.type;
  ef.source_port = f.source;
  ef.destination_port = f.destination;
  ef.index = f.index;
  ef.count = f.count;
  ef.credit = f.credit;
  ef.id = f.id;
  ef.length = f.length;
  for (size_t i = 0; i < f.length; i++)
    ef.data[i] = f.data[i];
  size_t n = 0;
  if (edu_fragment_encode(&ef, t.bytes, &n))
    return LT_INVALID;
  t.sender = sender;
  t.token = token;
  t.last = now;
  t.deadline = now + 50;
  t.active = 1;
  *s = t;
  *sender = staged;
  /* Repair the self-relative borrow after the staging copy. */
  return lt_begin(&s->lower, s->bytes, n);
}
int lt_partial_progress(struct lt_partial *s, size_t accepted, uint32_t now) {
  if (!s || !s->active || now < s->last || now > UINT32_MAX - 500)
    return LT_INVALID;
  if (s->poison)
    return LT_TIMEOUT;
  if (now >= s->deadline) {
    s->poison = 1;
    s->last = now;
    return LT_TIMEOUT;
  }
  if (s->lower.phase != 1 || accepted > s->lower.offered)
    return LT_INVALID;
  int r = lt_accept(&s->lower, accepted);
  s->last = now;
  if (LT_MUTANT == 6 && accepted && r != LT_DONE) {
    (void)e7_send_accept(s->sender, s->token, 1, now);
  }
  if (r == LT_DONE) {
    r = e7_send_accept(s->sender, s->token, 1, now);
    if (r != E7_OK) {
      s->poison = 1;
      return LT_INVALID;
    }
    s->active = 0;
    return LT_DONE;
  }
  return r;
}
int lt_partial_abort(struct lt_partial *s, unsigned reset, uint32_t now) {
  if (!s || reset > 1 || now < s->last || now > UINT32_MAX - 500)
    return LT_INVALID;
  if (!s->active)
    return LT_OK;
  if (lt_cancel(&s->lower, reset) != LT_OK)
    return LT_BUSY;
  int r = e7_send_accept(s->sender, s->token, 0, now);
  if (r != E7_BUSY && r != E7_TIMEOUT)
    return LT_INVALID;
  s->active = s->poison = 0;
  s->last = now;
  return LT_OK;
}

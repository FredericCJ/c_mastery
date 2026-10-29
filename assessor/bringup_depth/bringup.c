#include "bringup.h"
#ifndef BU_MUTANT
#define BU_MUTANT 0
#endif
static int valid(struct bu_range r) { return r.size <= UINT32_MAX - r.base; }
int bu_contains(struct bu_range o, struct bu_range i) {
  return valid(o) && valid(i) && i.base >= o.base && i.size <= o.size &&
         i.base - o.base <= o.size - i.size;
}
int bu_overlap(struct bu_range a, struct bu_range b) {
  return valid(a) && valid(b) && a.size && b.size && a.base < b.base + b.size &&
         b.base < a.base + a.size;
}
static uint32_t le32(const unsigned char *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}
enum bu_result bu_vectors(const unsigned char *b, size_t n, unsigned encoding,
                    uint32_t base, struct bu_range code, struct bu_range stack,
                    struct bu_vectors *out) {
  if (encoding != 1)
    return BU_UNSUPPORTED;
  if (!b || !out || n < 8 || n > UINT32_MAX || base % 512 ||
      !bu_contains(code, (struct bu_range){base, (uint32_t)n}))
    return BU_INVALID;
  uint32_t sp = le32(b), pc = le32(b + 4);
  if (!valid(stack) || !stack.size || sp % 8 || sp != stack.base + stack.size ||
      (!(pc & 1) && BU_MUTANT != 1) ||
      !bu_contains(code, (struct bu_range){pc & ~UINT32_C(1), 2}))
    return BU_INVALID;
  out->stack = sp;
  out->reset = pc;
  return BU_OK;
}
static int fits(size_t total, uint32_t at, uint32_t n) {
  return at <= total && n <= total - at;
}
enum bu_result bu_memory(unsigned char *ram, size_t nr, const unsigned char *rom,
                   size_t nf, const struct bu_copy *cp, size_t nc, const struct bu_range *zp,
                   size_t nz, struct bu_range kept) {
  if (!ram || !rom || (nc && !cp) || (nz && !zp) || nc > 4 || nz > 4 ||
      !fits(nr, kept.base, kept.size))
    return BU_INVALID;
  for (size_t i = 0; i < nc; i++) {
    struct bu_range dst = {cp[i].run, cp[i].size};
    if (!fits(nf, cp[i].load, cp[i].size) || !fits(nr, dst.base, dst.size) ||
        bu_overlap(dst, kept))
      return BU_INVALID;
    for (size_t j = 0; j < i; j++)
      if (bu_overlap(dst, (struct bu_range){cp[j].run, cp[j].size}))
        return BU_INVALID;
    for (size_t j = 0; j < nz; j++)
      if (bu_overlap(dst, zp[j]))
        return BU_INVALID;
  }
  for (size_t i = 0; i < nz; i++) {
    if (!fits(nr, zp[i].base, zp[i].size) || bu_overlap(zp[i], kept))
      return BU_INVALID;
    for (size_t j = 0; j < i; j++)
      if (bu_overlap(zp[i], zp[j]))
        return BU_INVALID;
  }
  for (size_t i = 0; i < nc; i++)
    for (uint32_t j = 0; j < cp[i].size; j++)
      ram[cp[i].run + j] = rom[cp[i].load + j];
  for (size_t i = 0; i < nz; i++)
    for (uint32_t j = 0; j < zp[i].size; j++)
      ram[zp[i].base + j] = 0;
  if (BU_MUTANT == 2 && nz && zp[0].base + zp[0].size < nr)
    ram[zp[0].base + zp[0].size] = 0;
  return BU_OK;
}
int bu_f439_dma(struct bu_range r) {
  return r.size && bu_contains((struct bu_range){0x20000000, 0x30000}, r);
}
enum bu_result bu_clock_start(struct bu_clock *c, unsigned limit, unsigned bypass) {
  if (!c || !limit || bypass > 1)
    return BU_INVALID;
  c->phase = 1;
  c->polls = 0;
  c->limit = limit;
  c->bypass = bypass;
  c->dependencies = 0;
  c->epoch = 0;
  return BU_OK;
}
enum bu_result bu_clock_step(struct bu_clock *c, unsigned osc, unsigned pll) {
  if (!c || osc > 1 || pll > 1 || c->phase < 1 || c->phase > 3)
    return BU_INVALID;
  if (c->phase == 3)
    return BU_OK;
  ++c->polls;
  if ((c->phase == 1 ? osc : pll) &&
      !(BU_MUTANT == 3 && c->polls == c->limit)) {
    ++c->phase;
    c->polls = 0;
    if (c->phase == 3)
      c->dependencies = 1;
    return BU_OK;
  }
  if (c->polls == c->limit) {
    c->phase = 4;
    c->dependencies = 0;
    return BU_TIMEOUT;
  }
  return BU_OK;
}
enum bu_result bu_clock_reconfigure(struct bu_clock *c, unsigned bypass) {
  if (!c || bypass > 1)
    return BU_INVALID;
  /* phase1 includes enabled HSE, phase2 enabled PLL: no live reconfiguration.
   */
  if (c->phase != 4)
    return BU_FAILED;
  c->bypass = bypass;
  return BU_OK;
}
enum bu_result bu_clock_failure(struct bu_clock *c) {
  if (!c || c->phase != 3 || c->epoch == UINT32_MAX)
    return BU_INVALID;
  c->phase = 4;
  c->dependencies = 0;
  ++c->epoch;
  return BU_OK;
}
enum bu_result bu_hooks(const struct bu_hook *h, unsigned n, unsigned services,
                  unsigned *completed) {
  unsigned order[8], done = 0;
  if (!completed || n > 8 || (n && !h))
    return BU_INVALID;
  *completed = 0;
  for (unsigned i = 0; i < n; i++)
    if (!h[i].call || h[i].dependencies >> n || (h[i].services & ~services))
      return BU_UNSUPPORTED;
  /* Complete topological validation before the first side effect. */
  for (unsigned k = 0; k < n; k++) {
    unsigned i;
    for (i = 0; i < n; i++)
      if (!(done & (1u << i)) && !(h[i].dependencies & ~done))
        break;
    if (i == n)
      return BU_INVALID;
    order[k] = i;
    done |= 1u << i;
  }
  for (unsigned k = 0; k < n; k++) {
    unsigned i = order[k];
    if (h[i].call(h[i].context))
      return BU_FAILED;
    *completed |= 1u << i;
  }
  return BU_OK;
}
static uint32_t checksum(const unsigned char *b) {
  uint32_t v = 2166136261u;
  for (unsigned i = 0; i < 19; i++)
    v = (v ^ b[i]) * 16777619u;
  return v;
}
static void put32(unsigned char *p, uint32_t v) {
  for (unsigned i = 0; i < 4; i++)
    p[i] = (unsigned char)(v >> (8 * i));
}
enum bu_result bu_decode(const unsigned char *b, size_t n, struct bu_record *out) {
  if (!b || !out || n < BU_RECORD || b[23] != 0xa5 || b[0] != 'B' ||
      b[1] != 'U' || b[2] != '2' || b[3] != '7' || (b[4] != 1 && b[4] != 2) ||
      b[5] > 8 || (b[4] == 1 && b[5] != 4) || (b[4] == 2 && b[5] != 8) ||
      (BU_MUTANT != 4 && le32(b + 19) != checksum(b)))
    return BU_INVALID;
  for (unsigned i = 7; i < 11; i++)
    if (b[i])
      return BU_INVALID;
  out->version = b[4];
  out->length = b[5];
  out->generation = b[6];
  for (unsigned i = 0; i < 8; i++)
    out->payload[i] = i < out->length ? b[11 + i] : 0;
  return BU_OK;
}
enum bu_result bu_store(unsigned char *b, size_t n, const struct bu_record *v,
                  unsigned allowance, unsigned *writes) {
  unsigned char encoded[BU_RECORD];
  if (!b || !v || !writes || n < BU_RECORD ||
      !((v->version == 1 && v->length == 4) ||
        (v->version == 2 && v->length == 8)))
    return BU_INVALID;
  encoded[0] = 'B';
  encoded[1] = 'U';
  encoded[2] = '2';
  encoded[3] = '7';
  encoded[4] = v->version;
  encoded[5] = v->length;
  encoded[6] = v->generation;
  for (unsigned i = 7; i < 11; i++)
    encoded[i] = 0;
  for (unsigned i = 0; i < 8; i++)
    encoded[11 + i] = i < v->length ? v->payload[i] : 0;
  put32(encoded + 19, checksum(encoded));
  encoded[23] = 0xa5;
  *writes = 0;
  /* First invalidate old commit, then23 body bytes, finally commit. */
  for (unsigned i = 0; i < 25; i++) {
    if (*writes == allowance)
      return BU_FAILED;
    if (!i)
      b[23] = 0;
    else if (i == 24)
      b[23] = encoded[23];
    else
      b[i - 1] = encoded[i - 1];
    ++*writes;
  }
  return BU_OK;
}
enum bu_result bu_select(const unsigned char b[2][BU_RECORD], unsigned *selected) {
  struct bu_record x, y;
  if (!b || !selected)
    return BU_INVALID;
  int a = bu_decode(b[0], BU_RECORD, &x) == BU_OK,
      c = bu_decode(b[1], BU_RECORD, &y) == BU_OK;
  if (!a && !c)
    return BU_INVALID;
  if (!a || !c) {
    *selected = a ? 0 : 1;
    return BU_OK;
  }
  unsigned delta = (unsigned char)(x.generation - y.generation);
  if (!delta || delta == 128)
    return BU_AMBIGUOUS;
  *selected = delta < 128 ? 0 : 1;
  return BU_OK;
}
enum bu_result bu_migrate(const struct bu_record *a, struct bu_record *b) {
  if (!a || !b || a == b || a->version != 1 || a->length != 4)
    return BU_UNSUPPORTED;
  b->version = 2;
  b->length = 8;
  b->generation = a->generation;
  for (unsigned i = 0; i < 8; i++)
    b->payload[i] = i < 4 ? a->payload[i] : 0;
  return BU_OK;
}
enum bu_result bu_handoff(const struct bu_handoff *h, struct bu_range code, struct bu_range stack) {
  if (!h || !valid(stack) || !stack.size ||
      h->stack != stack.base + stack.size || h->stack % 8 || h->vector % 512 ||
      !bu_contains(code, (struct bu_range){h->vector, 8}) || !(h->entry & 1) ||
      !bu_contains(code, (struct bu_range){h->entry & ~UINT32_C(1), 2}) ||
      h->privileged != 1 || h->thread_mode != 1 || h->masks_closed != 1 ||
      h->pending_clear != 1 || (BU_MUTANT != 5 && h->dma_active) ||
      h->clock_known != 1 || h->cache_clean != 1 || h->peer_quiescent != 1)
    return BU_INVALID;
  return BU_OK;
}

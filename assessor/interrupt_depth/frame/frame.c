#include "frame.h"
#ifndef XF_MUTANT
#define XF_MUTANT 0
#endif
static int config(struct xf_priority_config c) {
  return c.bits >= 3 && c.bits <= 8 && c.grouping <= 7;
}
int xf_frame(const uint32_t *w, size_t n, uint32_t sp, uint32_t er,
             unsigned fpu, unsigned material, unsigned align, unsigned fault,
             struct xf_frame *out) {
  if (!w || !out || fpu > 1 || material > 1 || align > 1 || fault > 1 || sp % 4)
    return XF_INVALID;
  if (fault)
    return XF_FRAME; /* Never dereference a stacking-fault capture. */
  if (er != 0xfffffff1u && er != 0xfffffff9u && er != 0xfffffffdu &&
      er != 0xffffffe1u && er != 0xffffffe9u && er != 0xffffffedu)
    return XF_UNSUPPORTED;
  unsigned extended = (er & 16u) == 0, thread = (er & 8u) != 0,
           psp = (er & 4u) != 0;
  if (extended && !fpu)
    return XF_UNSUPPORTED;
  if (material && !extended)
    return XF_INVALID;
  unsigned count = extended ? 26 : 8;
  if (n < count)
    return XF_SHORT;
  /* Core offsets never shift when FP state extends the frame. */
  unsigned offset = (XF_MUTANT == 1 && extended) ? 18 : 0;
  uint32_t psr = w[offset + 7];
  unsigned padding = (psr >> 9) & 1u;
  if (!(psr & (1u << 24)) || (w[offset + 6] & 1u) ||
      (thread ? ((psr & 511u) != 0) : ((psr & 511u) == 0)))
    return XF_FRAME;
  if ((align || extended) ? sp % 8 != 0 : padding != 0)
    return XF_FRAME;
  if (XF_MUTANT == 2)
    padding = 0;
  if (n < count + padding)
    return XF_SHORT;
  if (sp > UINT32_MAX - 4u * (count + padding))
    return XF_FRAME;
  struct xf_frame r = {0};
  for (unsigned i = 0; i < 4; ++i)
    r.r[i] = w[offset + i];
  r.r12 = w[offset + 4];
  r.stacked_lr = w[offset + 5];
  r.pc = w[offset + 6];
  r.xpsr = psr;
  r.extended = extended;
  r.thread = thread;
  r.psp = psp;
  r.hardware_words = count;
  r.alignment_word = padding;
  r.original_sp = sp + 4u * (count + padding);
  if (extended && (material || XF_MUTANT == 3)) {
    for (unsigned i = 0; i < 16; ++i)
      r.fp[i] = w[8 + i];
    r.fpscr = w[24];
    r.fp_materialized = 1;
  }
  *out = r;
  return XF_OK;
}
int xf_priority(struct xf_priority_config c, unsigned raw, struct xf_priority *out) {
  if (!config(c) || raw > 255 || !out)
    return XF_INVALID;
  unsigned low = 8 - c.bits, group_bits = 7 - c.grouping;
  if (group_bits > c.bits)
    group_bits = c.bits;
  unsigned sub_bits = c.bits - group_bits;
  struct xf_priority r = {0};
  r.logical = raw >> low;
  r.encoded = (uint8_t)(r.logical << low);
  r.group = r.logical >> sub_bits;
  r.sub = r.logical & ((1u << sub_bits) - 1u);
  *out = r;
  return XF_OK;
}
int xf_urgency(struct xf_priority_config c, unsigned urgency, unsigned opposite,
               uint8_t *out) {
  if (!config(c) || !out || opposite > 1 || urgency >= (1u << c.bits))
    return XF_INVALID;
  unsigned level = opposite ? urgency : (1u << c.bits) - 1u - urgency;
  *out = (uint8_t)(level << (8 - c.bits));
  return XF_OK;
}
int xf_api_allowed(struct xf_priority_config c, unsigned raw, unsigned threshold,
                   unsigned *out) {
  struct xf_priority p, t;
  if (!out || xf_priority(c, raw, &p) || xf_priority(c, threshold, &t) ||
      !t.encoded || t.sub)
    return XF_INVALID;
  if (c.grouping > (c.bits == 8 ? 0 : 7 - c.bits) || t.encoded != threshold)
    return XF_INVALID;
  *out = p.encoded >= t.encoded;
  return XF_OK;
}
int xf_pick(struct xf_priority_config c, const struct xf_irq *v, size_t n, int active,
            unsigned primask, unsigned basepri, unsigned *out) {
  struct xf_priority threshold, current;
  if (!out || (n && !v) || n > 16 || primask > 1 || basepri > 255 ||
      active < -1 || active > 255 || !config(c))
    return XF_INVALID;
  for (size_t i = 0; i < n; ++i) {
    if (v[i].exception < 16 || v[i].exception > 511 || v[i].enabled > 1 ||
        v[i].pending > 1 || v[i].active > 1 || v[i].source > 1)
      return XF_INVALID;
    for (size_t j = 0; j < i; ++j)
      if (v[i].exception == v[j].exception)
        return XF_INVALID;
    if (v[i].active && (active < 0 || v[i].priority < (unsigned)active))
      active = v[i].priority;
  }
  if (active >= 0 && xf_priority(c, (unsigned)active, &current))
    return XF_INVALID;
  if (xf_priority(c, basepri, &threshold))
    return XF_INVALID;
  unsigned winner = 0, best = 256;
  for (size_t i = 0; i < n; ++i) {
    struct xf_priority p;
    if (xf_priority(c, v[i].priority, &p))
      return XF_INVALID;
    if (!v[i].pending || v[i].active || (!v[i].enabled && XF_MUTANT != 4) ||
        primask)
      continue;
    if (threshold.encoded && p.group >= threshold.group)
      continue;
    if (active >= 0 && (XF_MUTANT == 5 ? p.encoded >= current.encoded
                                       : p.group >= current.group))
      continue;
    if (p.encoded < best || (p.encoded == best && v[i].exception < winner)) {
      winner = v[i].exception;
      best = p.encoded;
    }
  }
  if (!winner)
    return XF_NONE;
  *out = winner;
  return XF_OK;
}
int xf_pend(struct xf_irq *v, unsigned value) {
  if (!v || value > 1)
    return XF_INVALID;
  v->pending = value;
  return XF_OK;
}
int xf_source(struct xf_irq *v, unsigned asserted) {
  if (!v || asserted > 1)
    return XF_INVALID;
  v->source = asserted;
  if (asserted)
    v->pending = 1;
  else if (XF_MUTANT == 6)
    v->pending = 0;
  return XF_OK;
}

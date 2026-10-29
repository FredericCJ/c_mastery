#include "clock.h"
#ifndef XK_MUTANT
#define XK_MUTANT 0
#endif
static uint32_t mask(unsigned bits) {
  return bits == 32 ? UINT32_MAX : (UINT32_C(1) << bits) - 1;
}
int xk_snapshot(struct xk_io io, struct xk_policy p, struct xk_snapshot *out) {
  if (!out || !io.high || !io.low || !io.guard || !p.low_bits ||
      p.low_bits > 32 || !p.high_bits || p.high_bits > 32 || !p.attempts ||
      p.attempts > 8 || !p.max_guard_span)
    return XK_INVALID;
  if (p.max_counter_advance > mask(p.low_bits) && XK_MUTANT != 2)
    return XK_UNSAFE;
  for (unsigned i = 0; i < p.attempts; ++i) {
    uint64_t begin = io.guard(io.context);
    uint32_t before = io.high(io.context), low = io.low(io.context),
             after = io.high(io.context);
    uint64_t end = io.guard(io.context);
    if (before > mask(p.high_bits) || after > mask(p.high_bits) ||
        low > mask(p.low_bits))
      return XK_INVALID;
    if (end < begin)
      return XK_RANGE;
    if (end - begin > p.max_guard_span)
      continue;
    if (before == after || XK_MUTANT == 1) {
      struct xk_snapshot r = {((uint64_t)after << p.low_bits) | low, begin, end, i + 1};
      *out = r;
      return XK_OK;
    }
  }
  return XK_BUSY;
}
int xk_sequence(struct xk_sequence *s, uint32_t epoch, uint32_t sequence,
                uint32_t bound) {
  if (!s || !epoch || !bound || bound >= UINT32_C(0x80000000) || s->seen > 1)
    return XK_INVALID;
  if (!s->seen) {
    *s = (struct xk_sequence){epoch, sequence, 1, 0};
    return XK_OK;
  }
  if (epoch != s->epoch)
    return XK_DIFFERENT;
  uint32_t distance = sequence - s->last;
  if (!distance)
    return XK_DUPLICATE;
  if (distance > bound)
    return XK_STALE;
  uint64_t missed = distance - 1;
  if (missed > UINT64_MAX - s->missing)
    return XK_RANGE;
  s->last = sequence;
  if (XK_MUTANT != 3)
    s->missing += missed;
  return missed ? XK_GAP : XK_OK;
}
static int scaled(uint64_t ticks, uint32_t quantum, uint64_t *out) {
  if (ticks > UINT64_MAX / quantum)
    return 0;
  *out = ticks * quantum;
  return 1;
}
static uint64_t drift(uint64_t delta, uint32_t ppm) {
  uint64_t whole = (delta / 1000000) * ppm, tail = (delta % 1000000) * ppm;
  return whole + tail / 1000000 + (tail % 1000000 != 0);
}
int xk_calibrate(const struct xk_calibration *c, uint64_t begin, uint64_t end,
                 struct xk_interval *out) {
  if (!c || !out || !c->epoch || !c->quantum_ns ||
      c->rate_error_ppm > 1000000 || c->anchor_lower_ns > c->anchor_upper_ns ||
      begin < c->anchor_tick || end < begin)
    return XK_INVALID;
  uint64_t lo, hi;
  if (!scaled(begin - c->anchor_tick, c->quantum_ns, &lo) ||
      !scaled(end - c->anchor_tick, c->quantum_ns, &hi))
    return XK_RANGE;
  uint64_t low_error = XK_MUTANT == 4 ? 0 : drift(lo, c->rate_error_ppm),
           high_error = XK_MUTANT == 4 ? 0 : drift(hi, c->rate_error_ppm);
  /* ppm<=10^6 implies ceil(delta*ppm/10^6)<=delta. */
  lo -= low_error;
  if (lo > UINT64_MAX - c->anchor_lower_ns || hi > UINT64_MAX - high_error)
    return XK_RANGE;
  hi += high_error;
  if (hi > UINT64_MAX - c->anchor_upper_ns)
    return XK_RANGE;
  *out =
      (struct xk_interval){c->anchor_lower_ns + lo, c->anchor_upper_ns + hi, c->epoch};
  return XK_OK;
}
int xk_event(struct xk_interval sample, uint64_t minimum, uint64_t maximum,
             struct xk_interval *out) {
  if (!out || !sample.epoch || sample.lower > sample.upper || minimum > maximum)
    return XK_INVALID;
  if (sample.lower < maximum)
    return XK_RANGE;
  *out = (struct xk_interval){sample.lower - maximum, sample.upper - minimum,
                      sample.epoch};
  return XK_OK;
}
int xk_order(struct xk_interval a, struct xk_interval b, unsigned *out) {
  if (!out || !a.epoch || !b.epoch || a.lower > a.upper || b.lower > b.upper)
    return XK_INVALID;
  if (a.epoch != b.epoch)
    return XK_DIFFERENT;
  *out = (XK_MUTANT == 5 ? a.upper <= b.lower : a.upper < b.lower) ? XK_BEFORE
         : b.upper < a.lower                                       ? XK_AFTER
                             : XK_AMBIGUOUS;
  return XK_OK;
}

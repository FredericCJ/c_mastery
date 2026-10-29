#ifndef CMASTERY_INTERRUPT_TIME_H
#define CMASTERY_INTERRUPT_TIME_H
#include <stdint.h>
enum {
  XK_OK,
  XK_INVALID,
  XK_RANGE,
  XK_BUSY,
  XK_UNSAFE,
  XK_DUPLICATE,
  XK_STALE,
  XK_GAP,
  XK_DIFFERENT,
  XK_UNIMPLEMENTED = 99
};
enum { XK_BEFORE, XK_AFTER, XK_AMBIGUOUS };
struct xk_io {
  void *context;
  uint32_t (*high)(void *), (*low)(void *);
  uint64_t (*guard)(void *);
};
/* The caller proves max_counter_advance for an attempt within max_guard_span.
 * It is not inferred from equal high words. Both counters have coupled carries.
 */
struct xk_policy {
  unsigned low_bits, high_bits, attempts;
  uint64_t max_counter_advance, max_guard_span;
};
struct xk_snapshot {
  uint64_t value, begin, end;
  unsigned attempts;
};
int xk_snapshot(struct xk_io, struct xk_policy, struct xk_snapshot *);
struct xk_sequence {
  uint32_t epoch, last, seen;
  uint64_t missing;
};
int xk_sequence(struct xk_sequence *, uint32_t epoch, uint32_t sequence,
                uint32_t max_steps);
struct xk_interval {
  uint64_t lower, upper;
  uint32_t epoch;
};
struct xk_calibration {
  uint64_t anchor_tick, anchor_lower_ns, anchor_upper_ns;
  uint32_t quantum_ns, rate_error_ppm, epoch;
};
int xk_calibrate(const struct xk_calibration *, uint64_t begin_tick, uint64_t end_tick,
                 struct xk_interval *);
int xk_event(struct xk_interval sample, uint64_t delay_min, uint64_t delay_max,
             struct xk_interval *);
int xk_order(struct xk_interval, struct xk_interval, unsigned *);
#endif

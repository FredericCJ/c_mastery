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
typedef struct {
  void *context;
  uint32_t (*high)(void *), (*low)(void *);
  uint64_t (*guard)(void *);
} XkIo;
/* The caller proves max_counter_advance for an attempt within max_guard_span.
 * It is not inferred from equal high words. Both counters have coupled carries.
 */
typedef struct {
  unsigned low_bits, high_bits, attempts;
  uint64_t max_counter_advance, max_guard_span;
} XkPolicy;
typedef struct {
  uint64_t value, begin, end;
  unsigned attempts;
} XkSnapshot;
int xk_snapshot(XkIo, XkPolicy, XkSnapshot *);
typedef struct {
  uint32_t epoch, last, seen;
  uint64_t missing;
} XkSequence;
int xk_sequence(XkSequence *, uint32_t epoch, uint32_t sequence,
                uint32_t max_steps);
typedef struct {
  uint64_t lower, upper;
  uint32_t epoch;
} XkInterval;
typedef struct {
  uint64_t anchor_tick, anchor_lower_ns, anchor_upper_ns;
  uint32_t quantum_ns, rate_error_ppm, epoch;
} XkCalibration;
int xk_calibrate(const XkCalibration *, uint64_t begin_tick, uint64_t end_tick,
                 XkInterval *);
int xk_event(XkInterval sample, uint64_t delay_min, uint64_t delay_max,
             XkInterval *);
int xk_order(XkInterval, XkInterval, unsigned *);
#endif

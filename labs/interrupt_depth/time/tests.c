#include "clock.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("XK_CHECK " #x);                                                    \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  uint32_t high[16], low[8];
  uint64_t clock[16];
  unsigned h, l, g;
} Script;
static uint32_t high(void *p) {
  Script *s = p;
  return s->high[s->h++];
}
static uint32_t low(void *p) {
  Script *s = p;
  return s->low[s->l++];
}
static uint64_t guard(void *p) {
  Script *s = p;
  return s->clock[s->g++];
}
int main(void) {
  Script s = {{7, 8, 8, 8}, {UINT32_MAX - 5, 4}, {10, 14, 15, 19}, 0, 0, 0};
  XkSnapshot out = {.value = 999};
  XkIo io = {&s, high, low, guard};
  XkPolicy p = {32, 32, 2, 10, 5};
  CHECK(xk_snapshot(io, p, &out) == XK_OK &&
        out.value == UINT64_C(34359738372) && out.attempts == 2 &&
        out.begin == 15 && out.end == 19);
  s = (Script){{0, 1, 1, 2}, {3, 4}, {0, 1, 2, 3}, 0, 0, 0};
  out.value = 999;
  CHECK(xk_snapshot(io, p, &out) == XK_BUSY && out.value == 999 && s.h == 4);
  /* Reduced high width ABA: coupled counter3->5->8, high0->0, sampled low1.
   * An advance of5 exceeds the low-word-period bound3 and must be refused. */
  s = (Script){{0, 0}, {1}, {0, 5}, 0, 0, 0};
  p = (XkPolicy){2, 1, 1, 5, 5};
  CHECK(xk_snapshot(io, p, &out) == XK_UNSAFE && s.h == 0 && out.value == 999);
  p.max_counter_advance = 3;
  s.clock[1] = 10;
  CHECK(xk_snapshot(io, p, &out) == XK_BUSY && out.value == 999);
  XkSequence seq = {0};
  CHECK(xk_sequence(&seq, 1, UINT32_MAX - 1, 16) == XK_OK);
  CHECK(xk_sequence(&seq, 1, 1, 16) == XK_GAP && seq.missing == 2);
  CHECK(xk_sequence(&seq, 1, 1, 16) == XK_DUPLICATE && seq.missing == 2);
  CHECK(xk_sequence(&seq, 1, UINT32_MAX, 16) == XK_STALE && seq.last == 1);
  CHECK(xk_sequence(&seq, 2, 2, 16) == XK_DIFFERENT && seq.last == 1);
  seq.missing = UINT64_MAX;
  CHECK(xk_sequence(&seq, 1, 3, 16) == XK_RANGE && seq.last == 1);
  XkCalibration c = {100, 1000, 1010, 10, 10000, 7};
  XkInterval interval = {0};
  unsigned order = 99;
  CHECK(xk_calibrate(&c, 105, 105, &interval) == XK_OK &&
        interval.lower == 1049 && interval.upper == 1061);
  XkInterval event;
  CHECK(xk_event(interval, 2, 9, &event) == XK_OK && event.lower == 1040 &&
        event.upper == 1059);
  CHECK(xk_order(event, (XkInterval){1059, 1080, 7}, &order) == XK_OK &&
        order == XK_AMBIGUOUS);
  CHECK(xk_order(event, (XkInterval){1060, 1080, 7}, &order) == XK_OK &&
        order == XK_BEFORE);
  CHECK(xk_order(event, (XkInterval){0, 10, 8}, &order) == XK_DIFFERENT &&
        order == XK_BEFORE);
  c.quantum_ns = 20;
  c.rate_error_ppm = 0;
  CHECK(xk_calibrate(&c, 105, 105, &interval) == XK_OK &&
        interval.lower == 1100 && interval.upper == 1110);
  CHECK(xk_event((XkInterval){1, 2, 7}, 0, 3, &event) == XK_RANGE);
  c.quantum_ns = UINT32_MAX;
  CHECK(xk_calibrate(&c, 100, UINT64_MAX, &interval) == XK_RANGE);
  puts("XK_PASS coherent_interval_sequence");
  return 0;
}

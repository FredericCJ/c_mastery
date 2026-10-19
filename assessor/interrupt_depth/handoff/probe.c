#include "handoff.h"
#include <stdio.h>
int main(void) {
  for (unsigned slots = 2; slots <= 8; slots *= 2)
    for (unsigned seed = 0; seed < 256; ++seed) {
      XiQueue q;
      XiLatest latest;
      XiRecord out = {0};
      if (xi_init(&q, slots))
        return 1;
      xi_latest_init(&latest);
      uint32_t state = seed + 1;
      for (unsigned i = 0; i < 64; ++i) {
        state = state * 1664525u + 1013904223u;
        unsigned op = state >> 31;
        XiRecord r = {.sequence = i, .timestamp = state, .copied = 0};
        int rc, lr;
        if (op) {
          rc = xi_push(&q, &r);
          lr = xi_latest_push(&latest, &r);
        } else {
          rc = xi_pop(&q, &out);
          lr = xi_latest_pop(&latest, &r);
        }
        printf("Q %u %u %u %u %d %u %d %u %u\n", slots, seed, i, op, rc,
               op ? i : out.sequence, lr, r.sequence, latest.dropped);
      }
    }
  for (unsigned prior = 0; prior < 256; prior += 16)
    for (unsigned a = 0; a < 256; a += 16)
      for (unsigned b = 0; b < 256; b += 16) {
        XiMask m;
        uint32_t t, u;
        if (xi_mask_init(&m, (XfPriorityConfig){4, 3}, 1, prior) ||
            xi_mask_enter(&m, a, &t))
          return 1;
        unsigned first = m.value;
        if (xi_mask_enter(&m, b, &u))
          return 1;
        unsigned second = m.value;
        if (xi_mask_leave(&m, u))
          return 1;
        unsigned third = m.value;
        if (xi_mask_leave(&m, t))
          return 1;
        printf("M %u %u %u %u %u %u %u\n", prior, a, b, first, second, third,
               m.value);
      }
  return 0;
}

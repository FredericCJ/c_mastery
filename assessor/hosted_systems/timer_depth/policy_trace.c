#include "timer.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
  for (uint64_t deadline = 1; deadline <= 16; ++deadline)
    for (uint64_t period = 0; period <= 8; ++period)
      for (uint64_t now = 0; now <= 48; ++now)
        for (int skip = 0; skip <= 1; ++skip) {
          TdTimer t;
          TdDispatch d = {0};
          unsigned g;
          if (td_init(&t, 1) || td_arm(&t, deadline, period, skip, &g) != 1)
            return 1;
          int result = td_begin(&t, g, now, &d);
          printf("%" PRIu64 " %" PRIu64 " %" PRIu64 " %d %d %" PRIu64
                 " %" PRIu64 " %" PRIu64 "\n",
                 deadline, period, now, skip, result, d.expirations, d.effects,
                 d.next);
        }
}

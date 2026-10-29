#include "lifecycle.h"
#include <stdio.h>
int main(void) {
  for (unsigned n = 0; n < 4; ++n)
    for (unsigned limit = 1; limit <= 3; ++limit)
      for (unsigned sequence = 0; sequence < 243; ++sequence) {
        unsigned x = sequence;
        struct bu_drain d;
        if (bu_drain_begin(&d, n, limit) != BU_OK)
          return 1;
        for (unsigned i = 0; i < 5; ++i) {
          unsigned event = x % 3;
          x /= 3;
          enum bu_result result = bu_drain_step(&d, event == 1, event == 2);
          printf("%u %u %u %u %u %u %u %u %u %u\n", n, limit, sequence, i,
                 (unsigned)result, d.callbacks, d.remaining, d.acknowledged,
                 d.expired, d.transferred);
        }
      }
  return 0;
}

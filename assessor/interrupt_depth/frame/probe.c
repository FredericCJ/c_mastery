#include "frame.h"
#include <stdio.h>
int main(void) {
  for (unsigned bits = 3; bits <= 8; ++bits)
    for (unsigned group = 0; group < 8; ++group)
      for (unsigned raw = 0; raw < 256; ++raw) {
        struct xf_priority p;
        uint8_t normal, opposite;
        struct xf_priority_config c = {bits, group};
        if (xf_priority(c, raw, &p) ||
            xf_urgency(c, raw >> (8 - bits), 0, &normal) ||
            xf_urgency(c, raw >> (8 - bits), 1, &opposite))
          return 1;
        printf("%u %u %u %u %u %u %u %u\n", bits, group, raw, p.encoded,
               p.group, p.sub, normal, opposite);
      }
  unsigned raw_values[] = {0, 0x1f, 0x50, 0x60, 0x90, 0xff};
  int current_values[] = {-1, 0x60, 0x90};
  unsigned masks[] = {0, 0x40, 0x80};
  for (unsigned bits = 3; bits <= 8; ++bits)
    for (unsigned group = 0; group < 8; ++group)
      for (unsigned r = 0; r < 6; ++r)
        for (unsigned cur = 0; cur < 3; ++cur)
          for (unsigned m = 0; m < 3; ++m)
            for (unsigned state = 0; state < 16; ++state) {
              struct xf_priority_config c = {bits, group};
              struct xf_irq v[3] = {{18, (uint8_t)raw_values[r], state & 1,
                             (state >> 1) & 1, (state >> 2) & 1, 0},
                            {17, 0x60, 1, 1, 0, 0},
                            {16, 0x60, 1, 1, 0, 0}};
              unsigned chosen = 999;
              int rc = xf_pick(c, v, 3, current_values[cur], state >> 3,
                               masks[m], &chosen);
              printf("P %u %u %u %d %u %u %d %u\n", bits, group, raw_values[r],
                     current_values[cur], masks[m], state, rc, chosen);
            }
  return 0;
}

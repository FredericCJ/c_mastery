#include "dma.h"
#include <stdio.h>
int main(void) {
  for (unsigned h = 0; h < 1024; h++) {
    struct pd_notice s = {7, 0, 0, 0, 0, 0, 0, 0};
    unsigned code = h;
    for (unsigned k = 0; k < 5; k++) {
      unsigned op = code % 4;
      code /= 4;
      struct pd_notice m = {7, s.sequence + 1, 1, 0, 32, 32, 0x30040100, 32}, out = {0};
      unsigned seq = op == 0 ? m.sequence : (s.sequence ? s.sequence : 1);
      int r = pd_notice(&s, op, 7, seq, &m, &out);
      printf("N %u %u %u %d %u %u\n", h, k, op, r, s.state, s.sequence);
    }
  }
  return 0;
}

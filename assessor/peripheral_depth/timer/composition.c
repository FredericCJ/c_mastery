#include "firmware_io.h"
#include "timer.h"
#include <stdio.h>
static uint32_t r[32];
static uint32_t get(void *p, unsigned a) {
  (void)p;
  return r[a];
}
static void put(void *p, unsigned a, uint32_t v) {
  (void)p;
  r[a] = v;
}
int main(void) {
  PuClock k = {16000000, 1, 1, 1, 1, 0};
  PtConfig c;
  FwIo io = {0, get, put};
  for (unsigned d = 0; d <= 1000; d += 125) {
    if (pt_choose(&k, 15, 1000, d, 1000 - d, 0, 0, &c) ||
        fw_timer(io, 1000000, 1000, d) || r[FW_PERIOD] != c.arr ||
        r[FW_COMPARE] != c.ccr[0])
      return 1;
  }
  /* Different rounding policies are exposed, not asserted equivalent. */
  if (pt_choose(&k, 15, 100000, 250, 750, 0, 0, &c) ||
      fw_timer(io, 1000000, 100000, 250) || c.ccr[0] != 3 || r[FW_COMPARE] != 2)
    return 1;
  k.apb1 = 4;
  if (pt_choose(&k, 7, 1000, 250, 750, 0, 0, &c) || c.clock != 8000000 ||
      c.arr != 999)
    return 1;
  puts("PT_COMPOSE_PASS nine equivalent integer duties; nearest-vs-truncation "
       "counterexample; actual pu_rates/fw_timer");
  return 0;
}

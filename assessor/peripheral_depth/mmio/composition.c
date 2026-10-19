#include "mmio.h"
#include <stdio.h>
typedef struct {
  PmModel m;
  unsigned faults;
} Context;
static uint32_t rd(void *p, unsigned reg) {
  Context *c = p;
  uint32_t v = 0;
  if (reg != FW_GPIO || pm_access(&c->m, PM_ODR, 32, 0, &v))
    ++c->faults;
  return v;
}
static void wr(void *p, unsigned reg, uint32_t v) {
  Context *c = p;
  if (reg != FW_GPIO || pm_access(&c->m, PM_BSRR, 32, 1, &v))
    ++c->faults;
}
int main(void) {
  for (unsigned pin = 0; pin < 16; ++pin)
    for (unsigned high = 0; high < 2; ++high) {
      Context c = {0};
      c.m.revision = 1;
      c.m.odr = 0x5a5a;
      FwIo io = {&c, rd, wr};
      if (fw_gpio(io, pin, (int)high) || c.faults || c.m.log_count != 1 ||
          !c.m.log[0].write || c.m.log[0].offset != PM_BSRR)
        return 1;
      unsigned expected =
          high ? (0x5a5au | (1u << pin)) : (0x5a5au & ~(1u << pin));
      if (c.m.odr != expected)
        return 1;
    }
  puts("PM_COMPOSE_PASS actual frozen fw_gpio through side-effect adapter32 "
       "cases");
  return 0;
}

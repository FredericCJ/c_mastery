#include "timer.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("PT_TARGET_CHECK:%u\n", (unsigned)__LINE__);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct dg_reg {
  uintptr_t a;
  uint32_t v;
};
static struct dg_reg regs[64];
static unsigned used, logn;
static struct {
  uintptr_t a;
  uint32_t v;
  unsigned write;
} logbook[256];
static unsigned inject_capture, bad, stages;
static struct dg_reg *reg(uintptr_t a) {
  for (unsigned i = 0; i < used; i++)
    if (regs[i].a == a)
      return &regs[i];
  regs[used].a = a;
  return &regs[used++];
}
uint32_t pt_read(uintptr_t a) {
  uint32_t v = reg(a)->v;
  logbook[logn].a = a;
  logbook[logn].v = v;
  logbook[logn++].write = 0;
  if (a == 0x40000834u) {
    reg(0x40000810u)->v &= ~2u;
    if (inject_capture)
      reg(0x40000810u)->v |= 0x200u;
  }
  return v;
}
void pt_write(uintptr_t a, uint32_t v) {
  logbook[logn].a = a;
  logbook[logn].v = v;
  logbook[logn++].write = 1;
  if (stages &&
      (a == 0x40000428u || a == 0x4000042cu || a == 0x40000434u ||
       a == 0x40000438u) &&
      !(reg(0x40000400u)->v & 2u))
    bad = 1;
  if (a == 0x40000810u)
    reg(a)->v &= v;
  else
    reg(a)->v = v;
}
int main(void) {
  struct pt_config c = {16000000, 1, 15, 999, {250, 750}, 0};
  unsigned out = 777, old;
  CHECK(pt_f439_start(&c) == PT_OK);
  CHECK(reg(0x40000418u)->v == 0x6868u && reg(0x40000400u)->v == 0x81u &&
        reg(0x40000420u)->v == 0x11u);
  CHECK(logbook[0].a == 0x40023840u && !logbook[0].write && logbook[1].write &&
        logbook[2].a == 0x40023840u && !logbook[2].write);
  old = logn;
  CHECK(pt_f439_start(&c) == PT_BUSY && old == logn);
  stages = 1;
  c.ccr[0] = 125;
  c.ccr[1] = 875;
  CHECK(pt_f439_stage(&c, 1) == PT_OK && !bad);
  stages = 0;
  CHECK(reg(0x40000434u)->v == 125 && reg(0x40000438u)->v == 875 &&
        reg(0x40000400u)->v == 0x81u);
  old = logn;
  CHECK(pt_f439_stage(&c, 2) == PT_STALE && old == logn);
  c.center = 1;
  CHECK(pt_f439_stage(&c, 1) == PT_BUSY && old == logn);
  pt_f439_stop();
  CHECK(reg(0x40000420u)->v == 0 && !(reg(0x40000400u)->v & 1u));
  c.arr = 500;
  c.ccr[0] = 125;
  c.ccr[1] = 500;
  CHECK(pt_f439_start(&c) == PT_OK && reg(0x40000400u)->v == 0xa1u);
  CHECK(pt_f439_capture_start(15) == PT_OK && reg(0x40000818u)->v == 1 &&
        reg(0x4000082cu)->v == 65535);
  CHECK(pt_f439_capture(&out) == PT_BUSY && out == 777);
  reg(0x40000810u)->v = 2;
  reg(0x40000834u)->v = 65530;
  CHECK(pt_f439_capture(&out) == PT_OK && out == 65530 &&
        !(reg(0x40000810u)->v & 2u));
  reg(0x40000810u)->v = 3;
  reg(0x40000834u)->v = 4;
  inject_capture = 1;
  CHECK(pt_f439_capture(&out) == PT_LOST && out == 65530 &&
        reg(0x40000810u)->v == 1);
  inject_capture = 0;
  pt_f439_pins();
  CHECK((reg(0x40020020u)->v >> 24) == 0x22u &&
        ((reg(0x40020420u)->v >> 24) & 15u) == 2u);
  CHECK((reg(0x40020000u)->v & 0xf000u) == 0xa000u &&
        (reg(0x40020400u)->v & 0x3000u) == 0x2000u);
  puts("PT_TARGET_PASS");
  return 0;
}

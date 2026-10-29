#include "frame.h"
#ifndef XF_FP_CALLEE
#define XF_FP_CALLEE 0
#endif
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define BYTE(a) (*(volatile uint8_t *)(uintptr_t)(a))
static void output(const char *s) {
  register uint32_t r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
static void hex(uint32_t value) {
  char b[10];
  for (unsigned i = 0; i < 8; ++i)
    b[i] = "0123456789abcdef"[(value >> (28 - 4 * i)) & 15];
  b[8] = ' ';
  b[9] = 0;
  output(b);
}
__attribute__((noreturn)) static void finish(unsigned code) {
  uint32_t args[2] = {0x20026, code};
  output(code ? "XF_TARGET_FAIL\n" : "XF_TARGET_PASS\n");
  register uint32_t r0 __asm__("r0") = 0x20;
  register uint32_t *r1 __asm__("r1") = args;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
  for (;;) {
  }
}
__attribute__((noreturn)) void xf_fault(void) {
  output("XF_UNEXPECTED_FAULT ");
  hex(REG(0xe000ed28));
  finish(2);
}
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      output("XF_TARGET_CHECK ");                                              \
      hex(__LINE__);                                                           \
      output(#x);                                                              \
      output("\n");                                                            \
      finish(1);                                                               \
    }                                                                          \
  } while (0)
struct xi_record {
  uint32_t id, er, sp, msp, active, pending, lazy_before, lazy_after, fpcar;
  struct xf_frame frame;
};
static struct xi_record trace[16];
static volatile unsigned used;
static unsigned current_group;
static uint32_t psp_storage[512] __attribute__((aligned(8)));
void xf_trigger(uint32_t *, unsigned, unsigned);
void xf_fp_callee(uint32_t);
extern unsigned char xf_interrupted_begin[], xf_interrupted_end[];
void xf_capture(uint32_t *sp, uint32_t er, unsigned id, uint32_t msp) {
  CHECK(used < 16);
  unsigned slot = used++;
  struct xi_record *r = &trace[slot];
  r->id = id;
  r->er = er;
  r->sp = (uint32_t)(uintptr_t)sp;
  r->msp = msp;
  r->active = REG(0xe000e300);
  r->pending = REG(0xe000e200);
  r->lazy_before = REG(0xe000ef34) & 1;
  unsigned extended = (er & 16u) == 0;
  r->fpcar = REG(0xe000ef38);
  if (extended && r->lazy_before)
    CHECK(r->fpcar == r->sp + 32u);
  CHECK(xf_frame(sp, 27, r->sp, er, 1, extended && !r->lazy_before, 1, 0,
                 &r->frame) == XF_OK);
  CHECK(r->frame.fp_materialized == (extended && !r->lazy_before));
  if (id == 16) {
    if (XF_FP_CALLEE)
      xf_fp_callee(7);
    r->lazy_after = REG(0xe000ef34) & 1;
    if (extended && XF_FP_CALLEE)
      CHECK(xf_frame(sp, 27, r->sp, er, 1, 1, 1, 0, &r->frame) == XF_OK &&
            r->frame.fp[0] == 0x3f800000 && r->frame.fp[15] == 0x41700000);
    REG(0xe000e200) = 6;
    __asm__ volatile("dsb\nisb" ::: "memory");
    CHECK((current_group == 3 ? used == 2 && trace[1].id == 17 : used == 1) &&
          (REG(0xe000e200) & 4));
    CHECK(used < 16);
    trace[used].id = 160;
    trace[used].active = REG(0xe000e300);
    trace[used].pending = REG(0xe000e200);
    ++used;
  }
}
static void masks(unsigned value) {
  __asm__ volatile("msr basepri,%0\ndsb\nisb" ::"r"(value) : "memory");
}
void xf_target_main(void) {
  REG(0xe000ed88) |= 15u << 20;
  REG(0xe000ed14) |= 1u << 9;
  __asm__ volatile("dsb\nisb" ::: "memory");
  REG(0xe000e180) = 7;
  REG(0xe000e280) = 7;
  BYTE(0xe000e400) = 255;
  unsigned mask = BYTE(0xe000e400), bits = 0;
  for (unsigned x = mask; x & 128; x = (x << 1) & 255)
    ++bits;
  CHECK(bits >= 3 && bits <= 8 && mask == (256u - (1u << (8 - bits))));
  output("XF_PRIORITY_READBACK ");
  hex(mask);
  hex(bits);
  output("\n");
  for (unsigned grouping = 3; grouping <= 6; grouping += 3)
    for (unsigned fp = 0; fp < 2; ++fp)
      for (unsigned lazy = 0; lazy < 2; ++lazy)
        for (unsigned pad = 0; pad < 2; ++pad) {
          __asm__ volatile("cpsid i" ::: "memory");
          used = 0;
          current_group = grouping;
          REG(0xe000e280) = 7;
          REG(0xe000ed0c) = (0x5fau << 16) | (grouping << 8);
          CHECK(((REG(0xe000ed0c) >> 8) & 7) == grouping);
          BYTE(0xe000e400) = 0x60;
          BYTE(0xe000e401) = 0x50;
          BYTE(0xe000e402) = grouping == 3 ? 0x60 : 0x70;
          REG(0xe000ef34) = (1u << 31) | (lazy << 30);
          masks(0);
          REG(0xe000e180) = 7;
          REG(0xe000e200) = 1;
          CHECK(REG(0xe000e200) & 1);
          masks(0x40);
          REG(0xe000e100) = 7;
          __asm__ volatile("cpsie i\ndsb\nisb" ::: "memory");
          CHECK(used == 0);
          __asm__ volatile("cpsid i" ::: "memory");
          REG(0xe000e280) = 7;
          masks(0);
          xf_trigger(psp_storage + 512, fp, pad);
          CHECK(used == 4 && trace[0].id == 16 &&
                trace[grouping == 3 ? 1 : 2].id == 17 &&
                trace[grouping == 3 ? 2 : 1].id == 160 && trace[3].id == 18);
          struct xi_record *a = &trace[0], *b = &trace[grouping == 3 ? 1 : 2],
                 *c = &trace[3], *after = &trace[grouping == 3 ? 2 : 1];
          CHECK(a->frame.thread && a->frame.psp && a->frame.extended == fp &&
                a->frame.alignment_word == pad);
          CHECK(a->frame.original_sp ==
                (uint32_t)(uintptr_t)(psp_storage + 512) - 4 * pad);
          CHECK(a->frame.pc >= (uint32_t)(uintptr_t)xf_interrupted_begin &&
                a->frame.pc <= (uint32_t)(uintptr_t)xf_interrupted_end);
          CHECK(a->frame.r[0] == 1 && a->frame.r[1] == 2 &&
                a->frame.r[2] == 3 && a->frame.r[3] == 4);
          if (grouping == 3)
            CHECK(b->frame.psp == 0 && b->frame.thread == 0 &&
                  (b->frame.xpsr & 511) == 16 && (b->active & 3) == 3);
          else
            CHECK(b->frame.psp && b->frame.thread && b->sp == a->sp &&
                  b->active == 2);
          CHECK(c->sp == a->sp && c->frame.pc == a->frame.pc &&
                c->er == a->er && c->frame.thread && c->frame.psp &&
                c->active == 4);
          CHECK((after->active & 3) == 1 && (after->pending & 4));
          CHECK(a->lazy_before == (fp && lazy));
          if (fp && lazy && XF_FP_CALLEE)
            CHECK(!a->lazy_after && a->frame.fp_materialized);
          output("XF_TRACE ");
          hex(grouping);
          hex(fp);
          hex(lazy);
          hex(pad);
          hex(a->er);
          hex(b->er);
          hex(c->er);
          hex(a->sp);
          hex(b->sp);
          hex(c->sp);
          hex(a->lazy_before);
          hex(a->lazy_after);
          hex(a->fpcar);
          output("\n");
        }
  finish(0);
}

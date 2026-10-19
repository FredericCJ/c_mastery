#include "handoff.h"
#ifndef XI_INSTRUMENT
#define XI_INSTRUMENT 1
#endif
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define BYTE(a) (*(volatile uint8_t *)(uintptr_t)(a))
static void output(const char *s) {
  register unsigned r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
static void hex(uint32_t x) {
  char b[10];
  for (unsigned i = 0; i < 8; ++i)
    b[i] = "0123456789abcdef"[(x >> (28 - 4 * i)) & 15];
  b[8] = ' ';
  b[9] = 0;
  output(b);
}
__attribute__((noreturn)) static void finish(unsigned code) {
  unsigned args[2] = {0x20026, code};
  output(code ? "XI_TARGET_FAIL\n" : "XI_TARGET_PASS\n");
  register unsigned r0 __asm__("r0") = 0x20;
  register unsigned *r1 __asm__("r1") = args;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
  for (;;) {
  }
}
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      output("XI_TARGET_CHECK ");                                              \
      hex(__LINE__);                                                           \
      output(#x);                                                              \
      output("\n");                                                            \
      finish(1);                                                               \
    }                                                                          \
  } while (0)
void xf_fault(void) {
  output("XI_FAULT ");
  hex(REG(0xe000ed28));
  finish(2);
}
static uint32_t primask(void) {
  uint32_t x;
  __asm__ volatile("mrs %0,primask" : "=r"(x));
  return x;
}
static uint32_t basepri(void) {
  uint32_t x;
  __asm__ volatile("mrs %0,basepri" : "=r"(x));
  return x;
}
static uint32_t now(void) { return UINT32_MAX - REG(0x40001004); }
static XiQueue queue;
static XiService service;
static volatile unsigned mode, low_seen, high_seen, mixed_seen, input_sequence,
    event_done;
static volatile uint32_t pair_first, pair_second;
static unsigned char payload[8] = {1, 2, 3, 4, 5, 6, 7, 8};
typedef struct {
  uint32_t entry, publish_before, publish_after, exit, consumed;
  unsigned status;
} Sample;
static Sample samples[2][40];
static unsigned workload;
void xi_low(void) { ++low_seen; }
void xi_high(void) {
  ++high_seen;
  if (mode == 1 && pair_second != (~pair_first))
    ++mixed_seen;
  if (mode == 2) {
    uint32_t p = xi_primask_enter();
    int rc = xi_capture_begin(&service, 99, now(), payload, 8);
    xi_primask_leave(p);
    CHECK(rc == XI_BUSY);
  }
}
void xi_timer(void) {
  uint32_t entry = now();
  CHECK(REG(0x4000000c) & 1);
  REG(0x40000000) = 0;
  REG(0x4000000c) = 1;
  __asm__ volatile("dsb" ::: "memory");
  unsigned id = input_sequence;
  CHECK(id < 40);
  Sample *r = &samples[workload][id];
  r->entry = entry;
  uint32_t p = xi_primask_enter();
  int rc =
      xi_capture_begin(&service, id, entry, payload, id % 3 == 0 ? 100 : 8);
  xi_primask_leave(p);
  r->status = (unsigned)rc;
  CHECK(rc == XI_OK || rc == XI_FULL);
  if (!rc) {
    if (mode == 2) {
      REG(0xe000e200) = 2;
      __asm__ volatile("dsb\nisb" ::: "memory");
      CHECK(high_seen == 1 && service.reentrant == 1);
    }
    if (XI_INSTRUMENT)
      r->publish_before = now();
    CHECK(xi_capture_end(&service) == XI_OK);
    if (XI_INSTRUMENT)
      r->publish_after = now();
  }
  r->exit = now();
  event_done = id + 1;
}
static unsigned stricter(unsigned a, unsigned b) {
  return !a ? b : !b ? a : a < b ? a : b;
}
static void mask_checks(void) {
  for (unsigned prior = 0; prior < 2; ++prior) {
    xi_primask_leave(prior);
    CHECK(primask() == prior);
    uint32_t a = xi_primask_enter(), b = xi_primask_enter();
    CHECK(a == prior && b == 1 && primask() == 1);
    xi_primask_leave(b);
    CHECK(primask() == 1);
    xi_primask_leave(a);
    CHECK(primask() == prior);
  }
  __asm__ volatile("cpsid i" ::: "memory");
  for (unsigned prior = 0; prior < 256; prior += 16)
    for (unsigned a = 0; a < 256; a += 16)
      for (unsigned b = 0; b < 256; b += 16) {
        xi_basepri_leave(prior);
        uint32_t x = xi_basepri_enter(a);
        CHECK(x == prior && basepri() == stricter(prior, a));
        uint32_t y = xi_basepri_enter(b);
        CHECK(y == stricter(prior, a) &&
              basepri() == stricter(stricter(prior, a), b));
        xi_basepri_leave(y);
        CHECK(basepri() == stricter(prior, a));
        xi_basepri_leave(x);
        CHECK(basepri() == prior);
      }
  xi_basepri_leave(0);
  BYTE(0xe000e400) = 0x60;
  BYTE(0xe000e401) = 0x20;
  REG(0xe000ed0c) = (0x5fau << 16) | (3u << 8);
  REG(0xe000e100) = 3;
  low_seen = high_seen = 0;
  REG(0xe000e200) = 3;
  __asm__ volatile("dsb\nisb" ::: "memory");
  CHECK(!low_seen && !high_seen);
  xi_primask_leave(0);
  CHECK(low_seen == 1 && high_seen == 1);
  mode = 1;
  low_seen = high_seen = mixed_seen = 0;
  pair_first = 7;
  pair_second = ~7u;
  uint32_t old = xi_basepri_enter(0x40);
  pair_first = 9;
  REG(0xe000e200) = 3;
  __asm__ volatile("dsb\nisb" ::: "memory");
  CHECK(high_seen == 1 && low_seen == 0 && mixed_seen == 1);
  pair_second = ~9u;
  xi_basepri_leave(old);
  CHECK(low_seen == 1);
  mode = 0;
}
static void arm_event(unsigned id) {
  input_sequence = id;
  REG(0x40000000) = 0;
  REG(0x4000000c) = 1;
  REG(0x40000008) = 100;
  REG(0x40000000) = 9;
  uint32_t start = now();
  while (event_done != id + 1)
    CHECK((uint32_t)(now() - start) < 25000);
}
static void drain(unsigned *count) {
  XiRecord r;
  int rc;
  while ((rc = xi_pop(&queue, &r)) == XI_OK) {
    CHECK(r.sequence < 40 && r.copied == 8 && r.bytes[7] == 8);
    CHECK(r.flags == (r.sequence % 3 == 0 ? XI_TRUNCATED : 0));
    Sample *s = &samples[workload][r.sequence];
    CHECK(s->status == XI_OK && !s->consumed);
    s->consumed = now();
    ++*count;
  }
  CHECK(rc == XI_EMPTY);
}
static void wait_until(uint32_t origin, uint32_t deadline) {
  while ((uint32_t)(now() - origin) < deadline)
    __asm__ volatile("nop");
}
void xf_target_main(void) {
  mask_checks();
  XiWide wide = {UINT32_MAX, 7};
  CHECK(xi_wide_add(&wide, 1) == XI_OK);
  XiWide snapshot = xi_wide_read(&wide);
  CHECK(snapshot.low == 0 && snapshot.high == 8 && primask() == 0);
  xi_primask_enter();
  CHECK(xi_wide_add(&wide, UINT32_MAX) == XI_OK && primask() == 1);
  wide = (XiWide){UINT32_MAX, UINT32_MAX};
  CHECK(xi_wide_add(&wide, 1) == XI_EXHAUSTED && primask() == 1);
  snapshot = xi_wide_read(&wide);
  CHECK(snapshot.low == UINT32_MAX && snapshot.high == UINT32_MAX);
  xi_primask_leave(0);
  REG(0x40001000) = 0;
  REG(0x40001008) = UINT32_MAX;
  REG(0x40001000) = 1;
  BYTE(0xe000e408) = 0x60;
  REG(0xe000e100) = 1u << 8;
  CHECK(xi_init(&queue, 4) == XI_OK &&
        xi_service_init(&service, &queue) == XI_OK);
  mode = 2;
  high_seen = 0;
  event_done = 0;
  workload = 0;
  arm_event(0);
  CHECK(service.acknowledged == 1 && service.reentrant == 1);
  XiRecord first;
  CHECK(xi_pop(&queue, &first) == XI_OK && first.sequence == 0);
  mode = 0;
  uint32_t overhead[32];
  for (unsigned i = 0; i < 32; ++i) {
    uint32_t start = now();
    volatile uint32_t a = now(), b = now();
    (void)a;
    (void)b;
    overhead[i] = now() - start;
  }
  for (workload = 0; workload < 2; ++workload) {
    CHECK(xi_init(&queue, 4) == XI_OK &&
          xi_service_init(&service, &queue) == XI_OK);
    for (unsigned i = 0; i < 40; ++i)
      samples[workload][i] = (Sample){0};
    event_done = 0;
    unsigned consumed = 0;
    uint32_t origin = now();
    for (unsigned group = 0; group < 8; ++group) {
      for (unsigned j = 0; j < 5; ++j) {
        unsigned id = 5 * group + j;
        uint32_t scheduled =
            100000u * group + 1000u + (workload ? 2000u : 20000u) * j;
        wait_until(origin, scheduled);
        CHECK((uint32_t)(now() - origin) < scheduled + 1000);
        arm_event(id);
        if (!workload)
          drain(&consumed);
      }
      wait_until(origin, 100000u * (group + 1));
      if (workload)
        drain(&consumed);
    }
    CHECK(service.acknowledged == 40 &&
          service.dropped == (workload ? 16 : 0) &&
          consumed == (workload ? 24 : 40));
  }
  xi_primask_enter();
  for (unsigned i = 0; i < 32; ++i) {
    output("XI_OVERHEAD ");
    hex(i);
    hex(overhead[i]);
    output("\n");
  }
  for (unsigned w = 0; w < 2; ++w)
    for (unsigned i = 0; i < 40; ++i) {
      Sample *s = &samples[w][i];
      output("XI_SAMPLE ");
      hex(w);
      hex(i);
      hex(s->status);
      hex(s->entry);
      hex(s->publish_before);
      hex(s->publish_after);
      hex(s->exit);
      hex(s->consumed);
      output("\n");
    }
  output("XI_MASK_CASES 4096\n");
  finish(0);
}

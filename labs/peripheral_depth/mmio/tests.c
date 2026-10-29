#include "mmio.h"
#ifndef PM_BIG_ENDIAN
#include <stdio.h>
#else
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#endif
#include <limits.h>
#define CK(x)                                                                  \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("PM_CHECK:%d\n", __LINE__);                                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static unsigned reads, writes;
static uint32_t written;
static uint32_t rd(void *p, unsigned r) {
  (void)p;
  (void)r;
  ++reads;
  return 3;
}
static void wr(void *p, unsigned r, uint32_t v) {
  (void)p;
  (void)r;
  ++writes;
  written = v;
}
int main(void) {
  struct pm_model m = {0};
  m.revision = 1;
  uint32_t v = 1;
  CK(pm_access(&m, PM_CTRL, 16, 1, &v) == -1 && !m.log_count && !m.control);
  v = 4;
  CK(pm_access(&m, PM_CTRL, 32, 1, &v) == -1 && !m.control);
  m.revision = 2;
  CK(pm_access(&m, PM_CTRL, 32, 1, &v) == 0 && m.control == 4);
  m.status = 15;
  v = 5;
  CK(pm_access(&m, PM_STATUS, 32, 1, &v) == 0 && m.status == 10);
  m.status = 15;
  v = 0;
  CK(pm_access(&m, PM_STATUS, 32, 0, &v) == 0 && v == 15);
  m.status |= 16;
  v &= ~5u;
  CK(pm_access(&m, PM_STATUS, 32, 1, &v) == 0 && m.status == 21);
  m.status = 31;
  v = 5;
  CK(pm_access(&m, PM_STATUS, 32, 1, &v) == 0 && m.status == 26);
  m.rx[0] = 0;
  m.rx[1] = 255;
  m.rx_count = 2;
  CK(pm_access(&m, PM_DATA, 8, 0, &v) == 0 && v == 0 && m.rx_count == 1);
  CK(pm_access(&m, PM_DATA, 32, 0, &v) == 0 && v == 255 && !m.rx_count);
  v = 99;
  CK(pm_access(&m, PM_DATA, 8, 0, &v) == -1 && v == 99);
  m.rtc = 5;
  CK(pm_access(&m, PM_RTC, 32, 0, &v) == 0 && v == 5);
  CK(pm_access(&m, PM_RTC, 32, 0, &v) == 0 && !v);
  struct fw_io io = {0, rd, wr};
  CK(pm_gpio(io, 4, 1) == 0 && !reads && writes == 1 && written == 0x10004);
  CK(pm_gpio(io, 0x8000, 0) == 0 && written == 0x8000);
  CK(pm_gpio(io, 0, 0) == 0 && written == 0);
  unsigned old = writes;
  CK(pm_gpio(io, 1, 1) == -1 && writes == old);
  CK(pm_gpio(io, 65536, 0) == -1 && writes == old);
  m.odr = 3;
  uint32_t stale = m.odr;
  m.odr |= 8;
  v = (stale | 4) & ~1u;
  CK(pm_access(&m, PM_ODR, 32, 1, &v) == 0 && m.odr == 6);
  m.odr = 11;
  v = 0x10004;
  CK(pm_access(&m, PM_BSRR, 32, 1, &v) == 0 && m.odr == 14);
  v = 0x10001;
  CK(pm_access(&m, PM_BSRR, 32, 1, &v) == 0 && (m.odr & 1));
  /* Hardware set priority differs from service's rejected overlapping mask. */
  for (unsigned ready = 1; ready <= 4; ++ready) {
    struct pm_request r = {0};
    m = (struct pm_model){0};
    m.revision = 1;
    m.control = 1;
    CK(pm_begin(&r, 65, 3) == 0);
    for (unsigned step = 1; step <= 3; ++step) {
      m.status = step == ready;
      int x = pm_step(&r, &m);
      CK(x == (step == ready ? 0 : step == 3 ? -2 : 1));
      if (x <= 0)
        break;
    }
    CK(m.tx_count == (ready <= 3));
    CK(pm_step(&r, &m) == -1);
  }
  struct pm_interrupt irq = {0};
  m = (struct pm_model){0};
  m.revision = 1;
  m.control = 1;
  m.status = 1;
  CK(pm_irq_begin(&irq, &m, 77, UINT32_MAX - 4, 10) == 0 && !irq.armed &&
     m.tx_count == 1);
  CK(pm_irq_event(&irq, &m, 0) == -1);
  m = (struct pm_model){0};
  m.revision = 1;
  m.control = 1;
  CK(pm_irq_begin(&irq, &m, 88, UINT32_MAX - 4, 10) == 1 && irq.armed);
  CK(pm_irq_event(&irq, &m, 3) == 1 && !m.tx_count);
  CK(pm_irq_event(&irq, &m, 4) == 1 && !m.tx_count);
  m.status = 1;
  CK(pm_irq_event(&irq, &m, 5) == 0 && m.tx_count == 1 && m.last_tx == 88 &&
     !(m.control & 2));
  CK(pm_irq_begin(&irq, &m, 99, 10, 2) == 1);
  CK(pm_irq_event(&irq, &m, 9) == -1 && irq.armed);
  CK(pm_irq_event(&irq, &m, 12) == -2 && !irq.armed && m.tx_count == 1);
  struct pm_filter d;
  unsigned changed;
  CK(pm_filter_init(&d, 3, 0, 0) == 0);
  unsigned samples[] = {0, 1, 0, 1, 1, 1};
  for (unsigned i = 0; i < 6; ++i)
    CK(pm_sample(&d, samples[i], i, &changed) == 0 && changed == (i == 5));
  CK(d.events == 1 && d.stable == 1);
  CK(pm_filter_init(&d, 3, 10, 7) == 0);
  CK(pm_sample(&d, 1, UINT32_MAX - 4, &changed) == 0 && !changed);
  CK(pm_sample(&d, 1, 1, &changed) == 0 && !changed);
  CK(pm_sample(&d, 1, 3, &changed) == 0 && !changed);
  CK(pm_sample(&d, 1, 5, &changed) == 0 && changed);
  CK(pm_sample(&d, 0, 4, &changed) ==
     -1); /* Half-range ambiguity/backward time. */
  CK(pm_sample(&d, 0, 20, &changed) == 0 && !changed);
  CK(pm_sample(&d, 0, 25, &changed) == 0 && !changed);
  CK(pm_sample(&d, 0, 30, &changed) == 0 && changed);
  d.events = UINT_MAX;
  d.candidate = 1;
  d.since = 30;
  CK(pm_sample(&d, 1, 37, &changed) == 0 && !changed);
  CK(pm_sample(&d, 1, 40, &changed) == -1 && d.stable == 0 &&
     d.events == UINT_MAX);
  CK(pm_line(1, 1, 0, 0) == 2 && pm_line(1, 1, 0, 1) == 1 &&
     pm_line(1, 1, 1, 1) == 0);
  CK(pm_line(0, 1, 1, 0) == 3 && pm_line(0, 0, 0, 1) == 0);
  puts(
      "PM_PASS widths effects W1C consuming-read GPIO readiness debounce line");
  return 0;
}

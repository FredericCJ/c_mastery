#include "timer.h"
#ifdef PT_BIG_ENDIAN
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#else
#include <stdio.h>
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("PT_CHECK:%u\n", (unsigned)__LINE__);                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  PuClock clock = {16000000u, 1u, 1u, 1u, 1u, 0u};
  PtConfig c, n, saved;
  PtModel m;
  PtRatio f, t, d[2];
  unsigned level, v = 777u;
  CHECK(pt_choose(&clock, 15, 1000, 250, 750, 0, 0, &c) == PT_OK);
  CHECK(c.psc == 15 && c.arr == 999 && c.ccr[0] == 250 && c.ccr[1] == 750);
  CHECK(pt_ratios(&c, &f, &t, d) == PT_OK && f.numerator == 16000000u &&
        f.denominator == 16000u && t.numerator == 16 &&
        d[0].denominator == 1000);
  saved = c;
  CHECK(pt_choose(&clock, 15, 1001, 250, 750, 0, 0, &c) == PT_RANGE);
  CHECK(c.arr == saved.arr && c.generation == saved.generation);
  CHECK(pt_choose(&clock, 15, 1001, 250, 750, 0, 2, &n) == PT_OK &&
        n.arr == 998);
  CHECK(pt_choose(&clock, 0, 16000000, 0, 0, 0, 0, &n) == PT_RANGE);
  CHECK(pt_choose(&clock, 65535, 1, 1000, 0, 0, 1000000, &n) == PT_OK);
  CHECK(pt_choose(&clock, 0, 1, 0, 0, 0, 0, &n) == PT_RANGE);
  CHECK(pt_init(&m, &c) == PT_OK);
  n = c;
  n.arr = 499;
  n.ccr[0] = 125;
  n.ccr[1] = 375;
  for (unsigned i = 0; i < 250; i++)
    CHECK(pt_tick(&m) == PT_OK);
  CHECK(pt_level(&m, 0, &level) == PT_OK && !level);
  CHECK(pt_stage(&m, &n) == PT_OK && m.active.arr == 999 &&
        m.active.ccr[1] == 750);
  for (unsigned i = 250; i < 1000; i++)
    CHECK(pt_tick(&m) == PT_OK);
  CHECK(m.counter == 0 && m.active.arr == 499 && m.active.ccr[0] == 125 &&
        m.active.ccr[1] == 375 && m.updates == 1);
  n = c;
  CHECK(pt_stage(&m, &n) == PT_OK);
  m.udis = 1;
  m.uif = 0;
  m.counter = 300;
  m.prescale = 9;
  CHECK(pt_ug(&m) == PT_OK && m.counter == 0 && m.prescale == 0 &&
        m.active.arr == 499 && m.uif == 0);
  m.udis = 0;
  m.urs = 1;
  CHECK(pt_ug(&m) == PT_OK && m.active.arr == 999 && !m.uif);
  n.generation = 2;
  CHECK(pt_stage(&m, &n) == PT_STALE);
  n = c;
  n.center = 1;
  CHECK(pt_stage(&m, &n) == PT_BUSY);
  CHECK(pt_choose(&clock, 15, 1000, 250, 1000, 1, 0, &c) == PT_OK &&
        c.arr == 500 && c.ccr[0] == 125 && c.ccr[1] == 500);
  CHECK(pt_ratios(&c, &f, &t, d) == PT_OK && f.denominator == 16000 &&
        d[0].denominator == 500);
  CHECK(pt_init(&m, &c) == PT_OK);
  {
    unsigned high = 0;
    for (unsigned i = 0; i < 1000; i++) {
      CHECK(pt_level(&m, 0, &level) == PT_OK);
      high += level;
      CHECK(pt_level(&m, 1, &level) == PT_OK && level);
      CHECK(pt_tick(&m) == PT_OK);
    }
    CHECK(high == 250 && m.counter == 0 && !m.down && m.updates == 2);
  }
  c.ccr[0] = 0;
  CHECK(pt_init(&m, &c) == PT_OK);
  for (unsigned i = 0; i < 1000; i++) {
    CHECK(pt_level(&m, 0, &level) == PT_OK && !level);
    CHECK(pt_tick(&m) == PT_OK);
  }
  c.psc = 2;
  CHECK(pt_init(&m, &c) == PT_OK);
  CHECK(pt_input(&m) == PT_OK && m.counter == 0);
  CHECK(pt_input(&m) == PT_OK && m.counter == 0);
  CHECK(pt_input(&m) == PT_OK && m.counter == 1);
  m.running = 0;
  CHECK(pt_input(&m) == PT_BUSY && m.counter == 1);
  CHECK(pt_interval(65530, 4, 20, 0, &v) == PT_OK && v == 10);
  CHECK(pt_interval(65530, 4, 20, 1, &v) == PT_LOST && v == 10);
  CHECK(pt_interval(0, 32768, 32767, 0, &v) == PT_RANGE && v == 10);
  CHECK(pt_interval(4, 4, 20, 0, &v) == PT_RANGE && v == 10);
  CHECK(pt_interval(1, 2, 32768, 0, &v) == PT_INVALID && v == 10);
  clock.sys_hz = 8000000;
  clock.generation = 2;
  CHECK(pt_choose(&clock, 15, 1000, 250, 750, 0, 0, &c) == PT_OK &&
        c.arr == 499);
  c.arr = 0;
  CHECK(pt_init(&m, &c) == PT_INVALID);
  puts("PT_PASS");
  return 0;
}

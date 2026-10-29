#include "i2c.h"
#ifdef PI_BIG_ENDIAN
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
      printf("PI_CHECK:%u\n", (unsigned)__LINE__);                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct dg_model {
  uint32_t r[10];
  unsigned tx[8], ntx, reads, stop, violation, receiver, count, cr_writes;
  uint8_t incoming[2];
};
static uint32_t rd(void *v, unsigned r) {
  struct dg_model *m = v;
  if (r == PI_SR2 && (m->r[PI_SR1] & PI_ADDR)) {
    if (m->receiver &&
        ((m->r[PI_CR1] & PI_ACK) ||
         ((m->r[PI_CR1] & PI_POS) != (m->count == 2 ? PI_POS : 0))))
      m->violation = 1;
    m->r[PI_SR1] &= ~PI_ADDR;
  }
  if (r == PI_DR) {
    if (m->reads >= m->count) {
      m->violation = 1;
      return 0;
    }
    return m->incoming[m->reads++];
  }
  return m->r[r];
}
static void wr(void *v, unsigned r, uint32_t x) {
  struct dg_model *m = v;
  if (r == PI_CR1) {
    ++m->cr_writes;
    if (m->r[r] & (PI_START | PI_STOP | PI_PEC))
      m->violation = 1;
    if (x & PI_STOP)
      ++m->stop;
  }
  if (r == PI_SR1) {
    m->r[r] &= x | ~0xdf00u;
    return;
  }
  if (r == PI_DR) {
    if (m->ntx < 8)
      m->tx[m->ntx++] = x;
    m->r[PI_SR1] = 0;
    return;
  }
  m->r[r] = x;
}
static int event(struct pi_i2c *s, struct dg_model *m, unsigned sr, uint32_t now) {
  struct pi_io io = {m, rd, wr};
  m->r[PI_SR1] = sr;
  if (sr & PI_SB) {
    m->r[PI_CR1] &= ~PI_START;
    m->r[PI_SR2] = 3;
  }
  return pi_step(s, io, 1, now);
}
struct dg_lines {
  unsigned released[2], held_scl, release_after, rises, writes;
};
static unsigned line_read(void *v, unsigned line) {
  struct dg_lines *l = v;
  return l->released[line] &&
         (line == PI_SCL ? !l->held_scl : l->rises >= l->release_after);
}
static void line_release(void *v, unsigned line, unsigned high) {
  struct dg_lines *l = v;
  ++l->writes;
  if (line == PI_SCL && high && !l->released[line])
    ++l->rises;
  l->released[line] = high;
}
int main(void) {
  struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
  for (unsigned count = 1; count <= 2; count++) {
    struct pi_i2c s = {0};
    struct dg_model m = {0};
    struct pi_io io = {&m, rd, wr};
    uint8_t out[2] = {0xcc, 0xcc};
    m.count = count;
    m.incoming[0] = 0x5a;
    m.incoming[1] = 0xc3;
    CHECK(pi_open(&s, io, &c) == PI_OK && m.r[PI_CCR] == 100 &&
          m.r[PI_TRISE] == 17 && m.r[PI_CR2] == 16 && m.r[PI_OAR1] == 16384);
    CHECK(pi_begin(&s, io, 1, 1, 0x52, 0x10, count, out, UINT32_MAX - 3u,
                   100) == PI_OK);
    CHECK(event(&s, &m, PI_SB, UINT32_MAX - 2u) == PI_BUSY);
    CHECK(event(&s, &m, PI_ADDR, UINT32_MAX - 1u) == PI_BUSY);
    CHECK(event(&s, &m, PI_TXE, UINT32_MAX) == PI_BUSY);
    CHECK(event(&s, &m, PI_BTF, 0) == PI_BUSY);
    CHECK(event(&s, &m, PI_SB, 1) == PI_BUSY);
    m.receiver = 1;
    CHECK(event(&s, &m, PI_ADDR, 2) == PI_BUSY && !m.violation);
    CHECK(m.ntx == 3 && m.tx[0] == 0xa4 && m.tx[1] == 0x10 && m.tx[2] == 0xa5);
    if (count == 2) {
      CHECK(event(&s, &m, PI_RXNE, 3) == PI_BUSY && !m.reads && !m.stop);
      CHECK(event(&s, &m, PI_BTF | PI_RXNE, 4) == PI_BUSY);
    } else
      CHECK(event(&s, &m, PI_RXNE, 4) == PI_BUSY);
    CHECK(m.reads == count && m.stop == 1 && out[0] == 0xcc && !m.violation);
    m.r[PI_CR1] &= ~PI_STOP;
    m.r[PI_SR2] = 0;
    CHECK(event(&s, &m, 0, 5) == PI_OK && out[0] == 0x5a &&
          out[1] == (count == 2 ? 0xc3 : 0xcc));
    CHECK(pi_step(&s, io, 1, 6) == PI_STALE);
    CHECK(pi_begin(&s, io, 1, 1, 0x52, 0x10, count, out, 6, 100) == PI_STALE);
  }
  for (unsigned kind = 0; kind < 4; kind++) {
    struct pi_i2c s = {0};
    struct dg_model m = {0};
    struct pi_io io = {&m, rd, wr};
    uint8_t out = 77;
    m.count = 1;
    CHECK(!pi_open(&s, io, &c));
    CHECK(!pi_begin(&s, io, 1, 1, 0x52, 0x10, 1, &out, 0, 10));
    CHECK(event(&s, &m, PI_SB, 1) == PI_BUSY);
    if (kind == 0) {
      CHECK(event(&s, &m, PI_ARLO | PI_AF, 2) == PI_ARBITRATION && !m.stop &&
            out == 77 && !s.phase);
    }
    if (kind == 1) {
      CHECK(event(&s, &m, PI_AF, 2) == PI_BUSY && s.result == PI_ADDRESS_NACK &&
            m.stop == 1);
      m.r[PI_CR1] &= ~PI_STOP;
      CHECK(event(&s, &m, 0, 3) == PI_ADDRESS_NACK && out == 77);
    }
    if (kind == 2) {
      CHECK(event(&s, &m, 0, 10) == PI_BUSY && s.result == PI_TIMEOUT);
      CHECK(event(&s, &m, 0, 11) == PI_BUSY && m.stop == 1);
      m.r[PI_CR1] &= ~PI_STOP;
      CHECK(event(&s, &m, 0, 12) == PI_TIMEOUT && out == 77);
    }
    if (kind == 3) {
      CHECK(event(&s, &m, PI_ADDR, 2) == PI_BUSY);
      CHECK(event(&s, &m, PI_BERR | PI_TXE, 3) == PI_BUSY && s.berr == 1 &&
            s.command_sent && !(m.r[PI_SR1] & PI_BERR));
    }
    CHECK(!m.violation);
  }
  for (unsigned pulses = 0; pulses <= 10; pulses++) {
    struct pi_recovery r = {0};
    struct dg_lines l = {{1, 1}, 0, pulses, 0, 0};
    struct pi_lines io = {&l, line_read, line_release};
    int rc = PI_BUSY;
    CHECK(pi_recover_begin(&r, io, 1, 0, UINT32_MAX - 3u, 200, 2) == PI_OK);
    for (unsigned t = 0; t < 200 && rc == PI_BUSY; t++)
      rc = pi_recover_step(&r, io, UINT32_MAX - 3u + t);
    CHECK(rc == (pulses == 10 ? PI_IO : PI_OK) &&
          r.pulses == (pulses == 10 ? 9 : pulses) && l.released[0] &&
          l.released[1]);
  }
  {
    struct pi_recovery r = {0};
    struct dg_lines l = {{1, 1}, 1, 0, 0, 0};
    struct pi_lines io = {&l, line_read, line_release};
    CHECK(pi_recover_begin(&r, io, 0, 0, 0, 30, 2) == PI_FORBIDDEN &&
          !l.writes);
    CHECK(pi_recover_begin(&r, io, 1, 1, 0, 30, 2) == PI_FORBIDDEN &&
          !l.writes);
    CHECK(pi_recover_begin(&r, io, 1, 0, 0, 30, 2) == PI_OK);
    CHECK(pi_recover_step(&r, io, 29) == PI_BUSY);
    CHECK(pi_recover_step(&r, io, 30) == PI_TIMEOUT && !r.pulses);
  }
  {
    uint8_t value = 42, reply = 77;
    CHECK(pi_device(0x10, 1, &value, &reply) == PI_TIMEOUT && value == 42 &&
          reply == 77);
    CHECK(pi_retry_policy(0x10, 1, PI_TIMEOUT) == PI_BUSY);
    CHECK(pi_device(0x10, 0, &value, &reply) == PI_OK && reply == 42);
    CHECK(pi_device(0x11, 1, &value, &reply) == PI_TIMEOUT && value == 43 &&
          reply == 42);
    CHECK(pi_retry_policy(0x11, 1, PI_TIMEOUT) == PI_UNCERTAIN);
    CHECK(pi_device(0x11, 0, &value, &reply) == PI_OK && value == 44);
    CHECK(pi_retry_policy(0x11, 0, PI_ADDRESS_NACK) == PI_BUSY);
  }
  {
    struct pi_i2c s = {0};
    struct dg_model m = {0};
    struct pi_io io = {&m, rd, wr};
    uint8_t out = 77;
    CHECK(!pi_open(&s, io, &c));
    CHECK(!pi_begin(&s, io, 1, 1, 0x52, 0x10, 1, &out, 0, 10));
    CHECK(event(&s, &m, 0, 10) == PI_BUSY && s.phase == 10);
    CHECK(event(&s, &m, 0, 11) == PI_BUSY && s.phase == 10 && !m.stop);
    CHECK(event(&s, &m, PI_SB, 12) == PI_BUSY && s.master && m.stop == 1);
    m.r[PI_CR1] &= ~PI_STOP;
    CHECK(event(&s, &m, 0, 13) == PI_TIMEOUT && out == 77 && !m.violation);
  }
  puts("PI_PASS");
  return 0;
}

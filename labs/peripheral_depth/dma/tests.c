#include "dma.h"
#ifndef PD_BIG_ENDIAN
#include <stdio.h>
#else
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#endif
#define CHECK(e)                                                               \
  do {                                                                         \
    if (!(e)) {                                                                \
      printf("PD_CHECK:%u\n", (unsigned)__LINE__);                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct dg_model {
  struct pd_snapshot snap;
  unsigned configure, enable, stop, clean, invalidate, release, acquire, fail;
  struct pd_extent current;
};
static int config(void *ctx, const struct pd_extent *e, uint32_t token) {
  struct dg_model *m = ctx;
  m->configure++;
  if (m->configure == m->fail)
    return 0;
  m->current = *e;
  m->snap = (struct pd_snapshot){token, 0, e->bytes / e->width, 0, 1};
  return 1;
}
static void enable(void *ctx) {
  struct dg_model *m = ctx;
  m->enable++;
  m->snap.enabled = 1;
  m->snap.fifo_empty = 0;
}
static void stop(void *ctx) {
  ((struct dg_model *)ctx)->stop++;
} /* Request only; hardware ack is separate. */
static struct pd_snapshot snapshot(void *ctx) { return ((struct dg_model *)ctx)->snap; }
static void visible(void *ctx, unsigned op, uint32_t a, uint32_t n) {
  struct dg_model *m = ctx;
  (void)a;
  (void)n;
  if (op == PD_CLEAN)
    m->clean++;
  if (op == PD_INVALIDATE)
    m->invalidate++;
  if (op == PD_RELEASE)
    m->release++;
  if (op == PD_ACQUIRE)
    m->acquire++;
}
static struct pd_io bus(struct dg_model *m) {
  struct pd_io io = {m, config, enable, stop, snapshot, visible};
  return io;
}
static struct pd_extent item(unsigned board, unsigned n) {
  uint32_t base = board == PD_F439 ? 0x20000000u : 0x30040100u;
  struct pd_extent e = {base, base + 8192, n, n, n, 1};
  return e;
}
int main(void) {
  uint32_t total = 777;
  struct pd_extent e = item(PD_F439, 16);
  CHECK(pd_validate(PD_F439, 0, &e, 1, &total) == PD_OK && total == 16);
  struct pd_extent bad = e;
  bad.source = 0x10000000;
  CHECK(pd_validate(PD_F439, 0, &bad, 1, &total) == PD_INVALID);
  bad = e;
  bad.destination = 0x2002fff8;
  CHECK(pd_validate(PD_F439, 0, &bad, 1, &total) == PD_INVALID);
  bad = e;
  bad.bytes = 17;
  CHECK(pd_validate(PD_F439, 0, &bad, 1, &total) == PD_INVALID);
  bad = e;
  bad.width = 3;
  CHECK(pd_validate(PD_F439, 0, &bad, 1, &total) == PD_INVALID);
  bad = e;
  bad.destination = e.source + 1;
  CHECK(pd_validate(PD_F439, 0, &bad, 1, &total) == PD_INVALID);
  bad = item(PD_H745, 32);
  bad.source++;
  CHECK(pd_validate(PD_H745, 1, &bad, 1, &total) == PD_INVALID);
  for (unsigned cancel = 0; cancel < 2; cancel++)
    for (unsigned remaining = 0; remaining <= 16; remaining++) {
      struct dg_model m = {0};
      struct pd_transfer s = {0};
      struct pd_io io = bus(&m);
      struct pd_result r = {99, 99, 99, 99};
      CHECK(pd_submit(&s, io, PD_F439, 0, &e, 1, 1) == PD_OK && m.enable == 1);
      CHECK(pd_finalize(&s, io, 1, &r) == PD_BUSY && r.token == 99);
      if (cancel)
        CHECK(pd_cancel(&s, io, 1) == PD_BUSY && m.stop == 1);
      m.snap.remaining = remaining;
      m.snap.flags = PD_TC;
      m.snap.enabled = 0; /* FIFO still flushing */
      CHECK(pd_poll(&s, io, 1) == PD_BUSY && s.phase != 3);
      m.snap.fifo_empty = 1;
      m.snap.token = 0;
      CHECK(pd_poll(&s, io, 1) == PD_BUSY);
      m.snap.token = 1;
      CHECK(pd_poll(&s, io, 1) == PD_OK && pd_finalize(&s, io, 1, &r) == PD_OK);
      CHECK(r.complete_bytes == (cancel ? 16 - remaining : remaining ? 0 : 16));
      CHECK(r.status == (cancel      ? PD_CANCELLED
                         : remaining ? PD_IO
                                     : PD_OK) &&
            r.possibly_written_bytes <= 16);
      CHECK(pd_finalize(&s, io, 1, &r) == PD_STALE &&
            pd_submit(&s, io, PD_F439, 0, &e, 1, 1) == PD_STALE);
    }
  {
    struct dg_model m = {0};
    struct pd_transfer s = {0};
    struct pd_io io = bus(&m);
    struct pd_result r;
    struct pd_extent v[2] = {item(PD_H745, 32), item(PD_H745, 64)};
    v[1].source += 128;
    v[1].destination += 128;
    CHECK(pd_submit(&s, io, PD_H745, 1, v, 2, UINT32_MAX) == PD_OK &&
          m.clean == 4 && m.invalidate == 2);
    v[1].bytes = 0; /* descriptor array was copied before publication */
    m.snap = (struct pd_snapshot){UINT32_MAX, 0, 0, PD_TC, 1};
    CHECK(pd_poll(&s, io, UINT32_MAX) == PD_BUSY && m.configure == 2 &&
          m.current.bytes == 64);
    m.snap = (struct pd_snapshot){UINT32_MAX, 0, 0, PD_TC, 1};
    CHECK(pd_poll(&s, io, UINT32_MAX) == PD_OK &&
          pd_finalize(&s, io, UINT32_MAX, &r) == PD_OK);
    CHECK(r.complete_bytes == 96 && r.status == PD_OK && m.invalidate == 4 &&
          pd_submit(&s, io, PD_H745, 1, &v[0], 1, 1) == PD_STALE);
  }
  {
    struct dg_model m = {0};
    struct pd_transfer s = {0};
    struct pd_io io = bus(&m);
    struct pd_result r;
    CHECK(pd_submit(&s, io, PD_F439, 0, &e, 1, 2) == PD_OK);
    m.snap.flags = PD_TE;
    m.snap.remaining = 99;
    CHECK(pd_poll(&s, io, 2) == PD_BUSY && m.stop == 1);
    m.snap = (struct pd_snapshot){2, 0, 0, PD_TC, 1};
    CHECK(pd_poll(&s, io, 2) == PD_OK && pd_finalize(&s, io, 2, &r) == PD_OK &&
          r.status == PD_IO && r.complete_bytes == 0 &&
          r.possibly_written_bytes == 16);
  }
  {
    struct pd_extent v[2] = {e, e}, out[4];
    unsigned n = 99;
    v[1].source += 32;
    v[1].destination += 32;
    CHECK(pd_suffix(v, 2, 20, out, &n) == PD_OK && n == 1 &&
          out[0].bytes == 12 && out[0].source == v[1].source + 4);
    CHECK(pd_suffix(v, 2, 33, out, &n) == PD_INVALID && n == 1);
  }
  {
    struct dg_model m = {0};
    m.fail = 1;
    struct pd_transfer s = {0};
    struct pd_io io = bus(&m);
    CHECK(pd_submit(&s, io, PD_F439, 0, &e, 1, 1) == PD_BUSY && !s.phase &&
          !s.last_token && !m.enable);
  }
  for (unsigned width = 1; width <= 4; width *= 2) {
    struct pd_extent v[3] = {e, e, e}, out[4];
    for (unsigned k = 0; k < 3; k++) {
      v[k].width = width;
      v[k].source += k * 32;
      v[k].destination += k * 32;
    }
    for (unsigned cut = 0; cut <= 48; cut++) {
      unsigned n = 99;
      int r = pd_suffix(v, 3, cut, out, &n);
      if (cut % width) {
        CHECK(r == PD_INVALID && n == 99);
      } else {
        CHECK(r == PD_OK && n == (cut == 48 ? 0 : 3 - cut / 16));
        unsigned bytes = 0;
        for (unsigned j = 0; j < n; j++)
          bytes += out[j].bytes;
        CHECK(bytes == 48 - cut);
        if (n)
          CHECK(out[0].source == v[cut / 16].source + cut % 16);
      }
    }
    unsigned n = 99;
    v[0].source_capacity = 1;
    CHECK(pd_suffix(v, 3, 48, out, &n) == PD_INVALID && n == 99);
    v[0] = e;
    v[0].source = UINT32_MAX - 7;
    CHECK(pd_suffix(v, 3, 48, out, &n) == PD_INVALID && n == 99);
  }
  {
    struct dg_model m = {0};
    m.fail = 2;
    struct pd_transfer s = {0};
    struct pd_io io = bus(&m);
    struct pd_result r;
    struct pd_extent v[2] = {e, e};
    v[1].source += 32;
    v[1].destination += 32;
    CHECK(pd_submit(&s, io, PD_F439, 0, v, 2, 1) == PD_OK);
    m.snap = (struct pd_snapshot){1, 0, 0, PD_TC, 1};
    CHECK(pd_poll(&s, io, 1) == PD_OK && pd_finalize(&s, io, 1, &r) == PD_OK);
    CHECK(r.status == PD_IO && r.complete_bytes == 16 &&
          r.possibly_written_bytes == 16 && m.enable == 1);
  }
  puts("PD_PASS");
  return 0;
}

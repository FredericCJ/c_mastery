#include "dma.h"
#include "handoff.h"
#include <stdio.h>
#define REQUIRE(x)                                                             \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("XI_DMA_FAIL " #x);                                                 \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct xi_device {
  struct pd_snapshot snapshot;
  unsigned backing, cached, valid;
};
static int configure(void *p, const struct pd_extent *e, uint32_t token) {
  struct xi_device *d = p;
  d->snapshot = (struct pd_snapshot){token, 0, e->bytes / e->width, 0, 1};
  return 1;
}
static void enable(void *p) { ((struct xi_device *)p)->snapshot.enabled = 1; }
static void stop(void *p) { ((struct xi_device *)p)->snapshot.enabled = 0; }
static struct pd_snapshot snapshot(void *p) { return ((struct xi_device *)p)->snapshot; }
static void visibility(void *p, unsigned action, uint32_t address,
                       uint32_t bytes) {
  (void)address;
  (void)bytes;
  if (action == PD_INVALIDATE)
    ((struct xi_device *)p)->valid = 0;
}
static unsigned cached_read(struct xi_device *d) {
  if (!d->valid) {
    d->cached = d->backing;
    d->valid = 1;
  }
  return d->cached;
}
int main(void) {
  struct xi_device d = {0};
  struct pd_transfer transfer = {0};
  struct pd_result result;
  struct pd_io io = {&d, configure, enable, stop, snapshot, visibility};
  struct pd_extent extent = {0x30040040, 0x30040080, 32, 32, 32, 1};
  struct xi_queue q;
  struct xi_record out = {.sequence = 999};
  REQUIRE(xi_init(&q, 4) == XI_OK);
  REQUIRE(pd_submit(&transfer, io, PD_H745, 1, &extent, 1, 1) == PD_OK);
  /* Model a speculative clean refill while the DMA lease is outstanding.
     CPU application reads are forbidden until finalize; this read represents
     cache machinery only. No real device/cache execution is claimed. */
  REQUIRE(cached_read(&d) == 0);
  d.backing = 42;
  d.snapshot.enabled = 0;
  d.snapshot.remaining = 0;
  REQUIRE(pd_poll(&transfer, io, 1) == PD_BUSY);
  REQUIRE(pd_finalize(&transfer, io, 1, &result) == PD_BUSY);
  REQUIRE(xi_pop(&q, &out) == XI_EMPTY && out.sequence == 999);
  d.snapshot.flags = PD_TC;
  d.snapshot.fifo_empty = 1;
  REQUIRE(pd_poll(&transfer, io, 1) == PD_OK);
  REQUIRE(pd_finalize(&transfer, io, 1, &result) == PD_OK &&
          result.complete_bytes == 32);
  /* CPU release/acquire cannot repair a stale CPU cache line from DMA. */
  struct xi_record record = {.sequence = cached_read(&d)};
  REQUIRE(xi_push(&q, &record) == XI_OK && xi_pop(&q, &out) == XI_OK);
  REQUIRE(out.sequence == 42);
  REQUIRE(pd_finalize(&transfer, io, 1, &result) == PD_STALE);
  puts("XI_DMA_PASS actual_PdTransfer finalized_then_copied=42");
  return 0;
}

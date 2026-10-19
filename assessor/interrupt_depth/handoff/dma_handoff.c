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
typedef struct {
  PdSnapshot snapshot;
  unsigned backing, cached, valid;
} Device;
static int configure(void *p, const PdExtent *e, uint32_t token) {
  Device *d = p;
  d->snapshot = (PdSnapshot){token, 0, e->bytes / e->width, 0, 1};
  return 1;
}
static void enable(void *p) { ((Device *)p)->snapshot.enabled = 1; }
static void stop(void *p) { ((Device *)p)->snapshot.enabled = 0; }
static PdSnapshot snapshot(void *p) { return ((Device *)p)->snapshot; }
static void visibility(void *p, unsigned action, uint32_t address,
                       uint32_t bytes) {
  (void)address;
  (void)bytes;
  if (action == PD_INVALIDATE)
    ((Device *)p)->valid = 0;
}
static unsigned cached_read(Device *d) {
  if (!d->valid) {
    d->cached = d->backing;
    d->valid = 1;
  }
  return d->cached;
}
int main(void) {
  Device d = {0};
  PdTransfer transfer = {0};
  PdResult result;
  PdIo io = {&d, configure, enable, stop, snapshot, visibility};
  PdExtent extent = {0x30040040, 0x30040080, 32, 32, 32, 1};
  XiQueue q;
  XiRecord out = {.sequence = 999};
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
  XiRecord record = {.sequence = cached_read(&d)};
  REQUIRE(xi_push(&q, &record) == XI_OK && xi_pop(&q, &out) == XI_OK);
  REQUIRE(out.sequence == 42);
  REQUIRE(pd_finalize(&transfer, io, 1, &result) == PD_STALE);
  puts("XI_DMA_PASS actual_PdTransfer finalized_then_copied=42");
  return 0;
}

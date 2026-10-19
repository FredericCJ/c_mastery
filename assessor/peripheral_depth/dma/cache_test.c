#define main public_tests
#include "../../../labs/peripheral_depth/dma/tests.c"
#undef main
#include "h745_cache.h"
typedef struct {
  Model bus;
  unsigned char ram[64], cache[64];
  unsigned valid[2], dirty[2], barriers, operations;
} Cache;
static unsigned line(uint32_t a) { return a >= 0x30042000u ? 1 : 0; }
static unsigned load(Cache *c, unsigned i) {
  unsigned l = i / 32;
  if (!c->valid[l]) {
    for (unsigned k = l * 32; k < (l + 1) * 32; k++)
      c->cache[k] = c->ram[k];
    c->valid[l] = 1;
    c->dirty[l] = 0;
  }
  return c->cache[i];
}
static void store(Cache *c, unsigned i, unsigned x) {
  (void)load(c, i);
  c->cache[i] = (unsigned char)x;
  c->dirty[i / 32] = 1;
}
static void emit(void *ctx, enum FWCacheOp op, uint32_t a) {
  Cache *c = ctx;
  if (op == FW_DSB || op == FW_ISB) {
    c->barriers++;
    return;
  }
  unsigned l = line(a);
  c->operations++;
  if (op == FW_CLEAN && c->valid[l] && c->dirty[l]) {
    for (unsigned k = l * 32; k < (l + 1) * 32; k++)
      c->ram[k] = c->cache[k];
    c->dirty[l] = 0;
  }
  if (op == H7_INVALIDATE) {
    c->dirty[l] = 0;
    c->valid[l] = 0;
  }
}
static int setup(void *ctx, const PdExtent *e, uint32_t t) {
  return config(&((Cache *)ctx)->bus, e, t);
}
static void go(void *ctx) { enable(&((Cache *)ctx)->bus); }
static void halt(void *ctx) { stop(&((Cache *)ctx)->bus); }
static PdSnapshot snap(void *ctx) { return snapshot(&((Cache *)ctx)->bus); }
static void vis(void *ctx, unsigned op, uint32_t a, uint32_t n) {
  Cache *c = ctx;
  if (op == PD_RELEASE || op == PD_ACQUIRE) {
    c->barriers++;
    return;
  }
  (void)fw_cache_range(a, n, op == PD_CLEAN ? FW_CLEAN : H7_INVALIDATE, emit,
                       c);
}
int main(void) {
  Cache c = {0};
  PdTransfer s = {0};
  PdResult result;
  PdExtent e = item(PD_H745, 32);
  PdIo io = {&c, setup, go, halt, snap, vis};
  store(&c, 0, 7);
  store(&c, 32, 9);
  store(&c, 63, 55);
  CHECK(pd_submit(&s, io, PD_H745, 1, &e, 1, 1) == PD_OK && c.ram[0] == 7 &&
        c.ram[63] == 55 && !c.valid[1]);
  /* Explicit hardware model: speculative clean line fill, not a legal owner
   * load. */
  CHECK(load(&c, 32) == 9);
  c.ram[32] = 42;
  CHECK(pd_cancel(&s, io, 1) == PD_BUSY);
  c.bus.snap = (PdSnapshot){1, 0, 31, PD_TC, 1};
  CHECK(pd_poll(&s, io, 1) == PD_OK &&
        pd_finalize(&s, io, 1, &result) == PD_OK && result.complete_bytes == 1);
  CHECK(load(&c, 32) == 42 && load(&c, 63) == 55 && c.operations == 4 &&
        c.barriers >= 12);
  /* Isolated wrong protocol: partial-line invalidation loses a dirty neighbour.
   */
  store(&c, 63, 77);
  emit(&c, H7_INVALIDATE, e.destination);
  CHECK(load(&c, 63) == 55);
  puts("PD_CACHE_PASS");
  return 0;
}

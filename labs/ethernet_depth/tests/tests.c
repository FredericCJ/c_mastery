#include "ethernet_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ED_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int layers(void) {
  uint32_t h[] = {20, 20, 14}, out[] = {77, 77, 77};
  unsigned yes = 9;
  EDInterface a = {1, 1, 0, 0}, b = a;
  CHECK(ed_wrap(10, h, 3, out) == ED_OK && out[0] == 30 && out[1] == 50 &&
        out[2] == 64);
  out[0] = 77;
  CHECK(ed_wrap(UINT32_MAX - 10, h, 3, out) == ED_INVALID && out[0] == 77);
  CHECK(ed_wrap(0, NULL, 0, NULL) == ED_OK);
  b.version = 2;
  CHECK(ed_compatible(a, b, &yes) == ED_OK && !yes);
  b = a;
  b.ownership = 1;
  CHECK(ed_compatible(a, b, &yes) == ED_OK && !yes);
  CHECK(ed_compatible(a, a, &yes) == ED_OK && yes);
  return 0;
}
static int frame(void) {
  uint8_t da[6] = {2, 1}, sa[6] = {2, 2}, p[1500] = {1, 2, 3}, wire[1518],
          saved[1518];
  size_t n = 77;
  uint32_t c = 0;
  EDFrame f = {0}, old;
  CHECK(ed_crc32((const uint8_t *)"123456789", 9, &c) == ED_OK &&
        c == UINT32_C(0xcbf43926));
  CHECK(ed_frame_build(da, sa, 3, p, 3, 1, wire, sizeof wire, &n) == ED_OK &&
        n == 64);
  CHECK(wire[17] == 0 && wire[59] == 0);
  CHECK(ed_frame_parse(wire, n, 1, &f) == ED_OK && f.payload_length == 3 &&
        f.stored_payload == 46 && f.payload[2] == 3);
  old = f;
  wire[20] ^= 1;
  CHECK(ed_frame_parse(wire, n, 1, &f) == ED_IO &&
        memcmp(&old, &f, sizeof f) == 0);
  memset(wire, 0xa5, sizeof wire);
  memcpy(saved, wire, sizeof wire);
  n = 77;
  CHECK(ed_frame_build(da, sa, 0x0800, p, 1500, 1, wire, 1517, &n) == ED_FULL &&
        n == 77 && memcmp(saved, wire, sizeof wire) == 0);
  CHECK(ed_frame_build(da, sa, 1501, p, 3, 0, wire, sizeof wire, &n) ==
        ED_INVALID);
  CHECK(ed_frame_build(da, sa, 0x8100, p, 3, 0, wire, sizeof wire, &n) ==
        ED_INVALID);
  CHECK(ed_frame_build(da, sa, 0x0800, p, 1500, 1, wire, sizeof wire, &n) ==
            ED_OK &&
        n == 1518);
  CHECK(ed_frame_parse(wire, n, 1, &f) == ED_OK && f.payload_length == 1500);
  return 0;
}
static int bridge(void) {
  EDBridge b = {0}, old;
  uint8_t a[6] = {2, 1}, c[6] = {2, 2},
          group[6] = {255, 255, 255, 255, 255, 255};
  unsigned mask = 99;
  CHECK(ed_bridge(&b, a, group, 1, 0, &mask) == ED_OK && mask == 6);
  CHECK(ed_bridge(&b, c, a, 2, 1, &mask) == ED_OK && mask == 1);
  CHECK(ed_bridge(&b, a, a, 1, 2, &mask) == ED_OK && mask == 0);
  CHECK(ed_bridge(&b, c, a, 2, 12, &mask) == ED_OK && mask == 5);
  CHECK(ed_bridge(&b, a, c, 3, 13, &mask) == ED_OK && mask == 2);
  old = b;
  mask = 99;
  CHECK(ed_bridge(&b, a, c, 1, 12, &mask) == ED_INVALID && mask == 99 &&
        memcmp(&b, &old, sizeof b) == 0);
  for (unsigned i = 3; i <= 4; i++) {
    uint8_t source[6] = {2, (uint8_t)i};
    CHECK(ed_bridge(&b, source, group, 1, 13, &mask) == ED_OK);
  }
  old = b;
  {
    uint8_t source[6] = {2, 5};
    CHECK(ed_bridge(&b, source, group, 1, 13, &mask) == ED_FULL &&
          memcmp(&b, &old, sizeof b) == 0);
  }
  CHECK(ed_bridge(&b, group, a, 1, 14, &mask) == ED_INVALID);
  return 0;
}
typedef struct {
  uint16_t regs[32];
  uint32_t control, tick;
  unsigned writes, reads, bsr_reads, stuck, fail, frozen;
  uint16_t old_bsr;
} Bus;
static int bread(void *ctx, unsigned csr, uint32_t *out) {
  Bus *b = ctx;
  b->reads++;
  if (b->fail)
    return 1;
  if (csr == 0) {
    *out = (b->stuck == 1 || (b->stuck == 2 && b->writes)) ? 1 : 0;
    return 0;
  }
  if (csr != 1)
    return 1;
  unsigned reg = (b->control >> 6) & 31;
  if (reg == 1 && b->bsr_reads++ == 0)
    *out = b->old_bsr;
  else
    *out = b->regs[reg];
  return 0;
}
static int bwrite(void *ctx, unsigned csr, uint32_t value) {
  Bus *b = ctx;
  if (b->fail || csr != 0)
    return 1;
  b->control = value;
  b->writes++;
  return 0;
}
static uint32_t bclock(void *ctx) {
  Bus *b = ctx;
  return b->frozen ? b->tick : b->tick++;
}
static int phy(void) {
  uint32_t word = 77, div = 88;
  uint16_t value = 55;
  EDPhy p = {0}, old;
  Bus b = {0};
  EDBus bus = {&b, bread, bwrite, bclock};
  CHECK(ed_mdio_word(168000000, 1, 31, 0, &word, &div) == ED_OK &&
        word == 0xfd1 && div == 102);
  word = 77;
  CHECK(ed_mdio_word(168000000, 1, 31, 1, &word, &div) == ED_BUSY &&
        word == 77);
  b.regs[2] = 7;
  b.regs[3] = 0xc132;
  b.old_bsr = 0;
  b.regs[1] = 36;
  b.regs[31] = 4096 | (6u << 2);
  CHECK(ed_phy_query(&bus, 168000000, 1, 3, 20, &p) == ED_OK &&
        p.historical == 0 && p.current == 1 && p.speed == 100 && p.full == 1 &&
        b.writes == 10);
  old = p;
  b.bsr_reads = 0;
  b.old_bsr = 4;
  b.regs[1] = 0;
  CHECK(ed_phy_query(&bus, 168000000, 1, 3, 20, &p) == ED_LINK_DOWN &&
        memcmp(&old, &p, sizeof p) == 0);
  b.stuck = 1;
  unsigned writes = b.writes;
  CHECK(ed_mdio_read(&bus, 168000000, 1, 2, 2, 10, &value) == ED_BUSY &&
        value == 55 && b.writes == writes);
  b.stuck = 2;
  b.writes = 0;
  b.frozen = 1;
  CHECK(ed_mdio_read(&bus, 168000000, 1, 2, 2, 10, &value) == ED_TIMEOUT &&
        value == 55 && b.writes == 2);
  b.stuck = 0;
  b.fail = 1;
  CHECK(ed_mdio_read(&bus, 168000000, 1, 2, 2, 10, &value) == ED_IO &&
        value == 55);
  return 0;
}
static int rx(void) {
  uint8_t a[64] = {0}, b[64] = {0};
  EDRx s = {0};
  EDBorrow borrow = {0}, old;
  EDDescriptor d[2] = {{a, 62, 1u << 9}, {b, 64, (100u << 16) | (1u << 8)}};
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_OK && borrow.first == a &&
        borrow.second == b && borrow.first_length == 62 &&
        borrow.second_length == 34 && borrow.token == 1);
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_BUSY);
  CHECK(ed_rx_release(&s, 2) == ED_STALE && s.held);
  CHECK(ed_rx_release(&s, 1) == ED_OK);
  CHECK(ed_rx_release(&s, 1) == ED_STALE);
  old = borrow;
  d[0].status = 0;
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_IO && s.dropping &&
        memcmp(&old, &borrow, sizeof borrow) == 0);
  d[0].status = 1u << 9;
  d[1].status = 0;
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_BUSY && s.dropping);
  d[1].status = (100u << 16) | (1u << 8) | (1u << 15);
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_IO);
  d[0].status = UINT32_C(0x80000200);
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_BUSY);
  d[0].status = 1u << 9;
  d[1].status = (64u << 16) | (1u << 8);
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_OK &&
        borrow.first_length == 60 && !borrow.second && !borrow.second_length);
  CHECK(ed_rx_release(&s, borrow.token) == ED_OK);
  s.generation = UINT32_MAX;
  CHECK(ed_rx_acquire(&s, d, 2, &borrow) == ED_FULL);
  return 0;
}
typedef struct {
  EDTx *state;
  unsigned calls, bad;
} Barrier;
static void barrier(void *p) {
  Barrier *b = p;
  b->calls++;
  if (b->state->slots[0].head & UINT32_C(0x80000000))
    b->bad++;
}
static int tx(void) {
  EDTx s = {0};
  uint8_t p[60] = {1, 2, 3};
  Barrier b = {&s, 0, 0};
  size_t bad = 99;
  uint8_t order[] = {0, 1, 2, 3, 4, 5, 6, 9, 7, 8},
          wrong[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 8};
  CHECK(ed_tx_prepare(&s, 0, 1, p, 60) == ED_OK && s.slots[0].bytes[1] == 2);
  p[1] = 9;
  CHECK(s.slots[0].bytes[1] == 2);
  CHECK(ed_tx_publish(&s, 0, 1, barrier, &b) == ED_BUSY && b.calls == 0);
  CHECK(ed_tx_tail(&s, 0, 1) == ED_OK);
  CHECK(ed_tx_publish(&s, 0, 1, barrier, &b) == ED_OK && b.calls == 1 &&
        !b.bad);
  CHECK(ed_tx_prepare(&s, 1, 1, p, 60) == ED_OK &&
        ed_tx_tail(&s, 1, 1) == ED_OK);
  CHECK(ed_tx_publish(&s, 1, 1, barrier, &b) == ED_BUSY);
  CHECK(ed_tx_complete(&s, 0, 1, 0, 0) == ED_BUSY);
  CHECK(ed_tx_abort(&s, 0, 1, 0) == ED_BUSY && s.slots[0].phase == 4);
  CHECK(ed_tx_prepare(&s, 0, 2, p, 60) == ED_BUSY);
  CHECK(ed_tx_publish(&s, 1, 1, barrier, &b) == ED_BUSY);
  CHECK(ed_tx_abort(&s, 0, 1, 1) == ED_OK && s.slots[0].phase == 0);
  CHECK(ed_tx_complete(&s, 0, 1, 1, 0) == ED_STALE);
  CHECK(ed_tx_prepare(&s, 0, 1, p, 60) == ED_STALE);
  CHECK(ed_tx_publish(&s, 1, 1, barrier, &b) == ED_OK);
  CHECK(ed_tx_complete(&s, 1, 1, 1, 1) == ED_IO && s.slots[1].phase == 0);
  CHECK(ed_reachable(0x2002fffc, 4) == ED_OK &&
        ed_reachable(0x2002fffc, 8) == ED_INVALID &&
        ed_reachable(0x10000000, 4) == ED_INVALID &&
        ed_reachable(0x20000001, 4) == ED_INVALID);
  CHECK(ed_initialize(order, 10, &bad) == ED_OK && bad == 10);
  CHECK(ed_initialize(wrong, 10, &bad) == ED_INVALID && bad == 7);
  return 0;
}
static int handoff(void) {
  EDBudget b = {0};
  uint32_t times[] = {100, 120};
  uint64_t lo[] = {77, 77}, hi[] = {88, 88};
  unsigned index = 99;
  CHECK(ed_budget(0, 100, &b) == ED_OK && b.ip == 40 && b.frame == 64 &&
        b.slot == 84 && b.nanoseconds == 6720);
  CHECK(ed_budget(1460, 10, &b) == ED_OK && b.ip == 1500 && b.frame == 1518 &&
        b.slot == 1538 && b.nanoseconds == 1230400);
  CHECK(ed_budget(1461, 100, &b) == ED_INVALID);
  CHECK(ed_capture(times, 2, -10, 1, 3, lo, hi) == ED_OK && lo[0] == 87 &&
        hi[0] == 93 && lo[1] == 107 && hi[1] == 113);
  lo[0] = 77;
  CHECK(ed_capture(times, 2, -101, 1, 3, lo, hi) == ED_INVALID && lo[0] == 77);
  CHECK(ed_capture(times, 2, 0, 0, 3, lo, hi) == ED_UNSUPPORTED);
  CHECK(ed_capture(NULL, 0, 0, 1, 0, NULL, NULL) == ED_OK);
  CHECK(ed_first_failure(11, &index) == ED_OK && index == 2);
  CHECK(ed_first_failure(63, &index) == ED_OK && index == 6);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "layers"))
    return layers();
  if (!strcmp(argv[1], "frame"))
    return frame();
  if (!strcmp(argv[1], "bridge"))
    return bridge();
  if (!strcmp(argv[1], "phy"))
    return phy();
  if (!strcmp(argv[1], "rx"))
    return rx();
  if (!strcmp(argv[1], "tx"))
    return tx();
  if (!strcmp(argv[1], "handoff"))
    return handoff();
  return 2;
}

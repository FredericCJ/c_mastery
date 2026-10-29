#include "clocks.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int equal(const struct rpc_clock *a, const struct rpc_clock *b) {
  unsigned char x[160], y[160];
  return rpc_encode(a, x) == RPC_OK && rpc_encode(b, y) == RPC_OK &&
         !memcmp(x, y, 160);
}
static int identity_case(void) {
  struct rpc_clock a, b, c, old, sentinel;
  CHECK(rpc_make(9, 10, &a) == RPC_OK);
  CHECK(rpc_make(9, 20, &b) == RPC_OK);
  CHECK(rpc_step(&a, 0, 5, 4, RPC_REJECT, &old) == RPC_OK);
  a = old;
  CHECK(rpc_step(&b, &a, 5, 4, RPC_REJECT, &c) == RPC_OK);
  CHECK(c.vector.count == 2 && c.vector.entries[0].identity == 10 &&
        c.vector.entries[0].value == 1 && c.vector.entries[1].value == 1);
  CHECK(rpc_replace(&a, 30, &b) == RPC_OK);
  CHECK(b.vector.count == 2 && b.vector.entries[0].identity == 10 &&
        b.vector.entries[0].value == 1);
  CHECK(rpc_step(&b, 0, 6, 4, RPC_REJECT, &c) == RPC_OK);
  struct rpc_clock after_old_message;
  CHECK(rpc_step(&c, &a, 7, 4, RPC_REJECT, &after_old_message) == RPC_OK);
  CHECK(after_old_message.self == 30 && after_old_message.vector.count == 2 &&
        after_old_message.vector.entries[0].identity == 10 &&
        after_old_message.vector.entries[0].value == 1 &&
        after_old_message.vector.entries[1].value == 2);
  int order = 9;
  CHECK(rpc_order(&a.vector, &c.vector, &order) == RPC_OK && order == -1);
  /* A numeric slot reset conflates a later event with the old identity. */
  struct rp_vector before = {{1, 0, 0}}, bad = {{0, 0, 0}}, after;
  CHECK(rp_vector_receive(bad, (struct rp_vector){{0, 0, 0}}, 0, &after) == RP_OK);
  CHECK(rp_vector_order(before, after) == 0);
  sentinel = c;
  CHECK(rpc_replace(&a, 10, &sentinel) == RPC_IDENTITY && equal(&sentinel, &c));
  a = c;
  for (unsigned id = 40; id <= 70; id += 10) {
    CHECK(rpc_replace(&a, id, &b) == RPC_OK);
    a = b;
  }
  CHECK(a.vector.count == 6);
  sentinel = a;
  CHECK(rpc_replace(&a, 80, &sentinel) == RPC_FULL && equal(&sentinel, &a));
  struct rpc_clock other;
  CHECK(rpc_make(10, 90, &other) == RPC_OK);
  CHECK(rpc_step(&a, &other, 6, 4, RPC_REJECT, &sentinel) == RPC_IDENTITY &&
        equal(&sentinel, &a));
  return 0;
}
static int policy_case(void) {
  struct rpc_clock a, b, sentinel;
  CHECK(rpc_make(9, 10, &a) == RPC_OK);
  a.hlc = (struct rp_hlc){10, UINT32_MAX};
  sentinel = a;
  CHECK(rpc_step(&a, 0, 10, 4, RPC_REJECT, &sentinel) == RPC_RANGE &&
        equal(&a, &sentinel));
  CHECK(rpc_step(&a, 0, 10, 4, RPC_WAIT_FOR_WALL, &sentinel) == RPC_WAIT &&
        equal(&a, &sentinel));
  CHECK(rpc_step(&a, 0, 11, 4, RPC_WAIT_FOR_WALL, &b) == RPC_OK &&
        b.hlc.physical == 11 && b.hlc.logical == 0);
  CHECK(rpc_step(&a, 0, 5, 4, RPC_WAIT_FOR_WALL, &sentinel) == RPC_SKEW &&
        equal(&a, &sentinel));
  a.hlc = (struct rp_hlc){UINT64_MAX, UINT32_MAX};
  CHECK(rpc_step(&a, 0, UINT64_MAX, 0, RPC_WAIT_FOR_WALL, &b) == RPC_RANGE);
  a.hlc = (struct rp_hlc){10, 0};
  a.lamport = UINT64_MAX;
  CHECK(rpc_step(&a, 0, 11, 4, RPC_WAIT_FOR_WALL, &b) == RPC_RANGE);
  a.lamport = 0;
  a.vector.entries[0].value = UINT64_MAX;
  CHECK(rpc_step(&a, 0, 11, 4, RPC_WAIT_FOR_WALL, &b) == RPC_RANGE);
  a.hlc.logical = UINT32_MAX;
  CHECK(rpc_step(&a, 0, 10, 4, RPC_WAIT_FOR_WALL, &b) == RPC_RANGE);
  /* Closeness needs the declared wall premise; resetting a bad sample is not
   * proof. */
  a.vector.entries[0].value = 1;
  a.hlc = (struct rp_hlc){1000, 0};
  CHECK(rpc_step(&a, 0, 10, 4, RPC_REJECT, &b) == RPC_SKEW);
  return 0;
}
struct rp_disk {
  unsigned char stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE],
      pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE], dirty[STORE_IMAGE_PAGES];
  unsigned cut, event, mode, dead;
};
static struct rp_disk disk;
static store pages;
static void persist(void) {
  for (unsigned i = 0; i < STORE_IMAGE_PAGES; ++i)
    if (disk.dirty[i]) {
      memcpy(disk.stable[i], disk.pending[i], STORE_PAGE_SIZE);
      disk.dirty[i] = 0;
    }
}
static int fault(void) {
  if (++disk.event != disk.cut || !disk.cut)
    return 0;
  if (disk.mode)
    persist();
  disk.dead = 1;
  return 1;
}
static int read_page(void *ctx, uint32_t p, void *out) {
  (void)ctx;
  if (disk.dead || p >= STORE_IMAGE_PAGES)
    return -1;
  memcpy(out, disk.dirty[p] ? disk.pending[p] : disk.stable[p],
         STORE_PAGE_SIZE);
  return 0;
}
static int write_page(void *ctx, uint32_t p, const void *in) {
  (void)ctx;
  if (disk.dead || p >= STORE_IMAGE_PAGES)
    return -1;
  memcpy(disk.pending[p], in, STORE_PAGE_SIZE);
  disk.dirty[p] = 1;
  return fault() ? -1 : 0;
}
static int flush(void *ctx) {
  (void)ctx;
  if (disk.dead || fault())
    return -1;
  persist();
  return 0;
}
static store_io io(void) {
  return (store_io){0, read_page, write_page, flush, STORE_IMAGE_PAGES};
}
static int setup(void) {
  memset(&disk, 0, sizeof disk);
  CHECK(store_format(io()) == STORE_OK);
  CHECK(store_open(&pages, io()) == STORE_OK);
  return 0;
}
static int reopen(void) {
  memset(disk.dirty, 0, sizeof disk.dirty);
  disk.dead = disk.cut = disk.event = 0;
  CHECK(store_open(&pages, io()) == STORE_OK);
  return 0;
}
static int store_case(void) {
  CHECK(!setup());
  struct rpc_clock a, b, sentinel;
  CHECK(rpc_make(9, 10, &a) == RPC_OK);
  CHECK(rpc_save(&pages, &a) == RPC_OK);
  CHECK(rpc_replace_durable(&pages, &a, 20, &b) == RPC_OK);
  sentinel = a;
  CHECK(rpc_step_durable(&pages, &a, 0, 1, 4, RPC_REJECT, &sentinel) ==
            RPC_STATE &&
        equal(&a, &sentinel));
  CHECK(!reopen());
  CHECK(rpc_load(&pages, 9, &a) == RPC_OK && equal(&a, &b) &&
        a.vector.count == 2);
  unsigned char bytes[160];
  CHECK(rpc_encode(&a, bytes) == RPC_OK);
  bytes[24] ^= 1;
  sentinel = a;
  CHECK(rpc_decode(9, bytes, &sentinel) == RPC_CORRUPT && equal(&a, &sentinel));
  CHECK(store_begin(&pages) == STORE_OK);
  CHECK(rpc_save(&pages, &a) == RPC_STATE && pages.active);
  store_abort(&pages);
  unsigned cuts = 0;
  for (unsigned replacement = 0; replacement < 2; ++replacement)
    for (unsigned mode = 0; mode < 2; ++mode)
      for (unsigned cut = 1; cut <= 37; ++cut) {
        CHECK(!setup());
        CHECK(rpc_make(9, 10, &a) == RPC_OK);
        CHECK(rpc_save(&pages, &a) == RPC_OK);
        CHECK(store_checkpoint(&pages) == STORE_OK);
        sentinel = a;
        disk.event = 0;
        disk.cut = cut;
        disk.mode = mode;
        CHECK((replacement ? rpc_replace_durable(&pages, &a, 20, &sentinel)
                           : rpc_step_durable(&pages, &a, 0, 10, 4, RPC_REJECT,
                                              &sentinel)) != RPC_OK);
        CHECK(equal(&sentinel, &a));
        CHECK(!reopen());
        CHECK(rpc_load(&pages, 9, &b) == RPC_OK);
        unsigned committed = mode && (cut == 36 || cut == 37);
        CHECK(b.lamport == (replacement ? 0 : committed));
        CHECK(b.vector.entries[0].value == (replacement ? 0 : committed));
        CHECK(b.self == (replacement && committed ? 20u : 10u));
        CHECK(b.vector.count == (replacement && committed ? 2u : 1u));
        CHECK(rpc_step_durable(&pages, &b, 0, 11, 4, RPC_REJECT, &sentinel) ==
              RPC_OK);
        CHECK(sentinel.lamport == (replacement ? 1 : committed + 1));
        ++cuts;
      }
  printf("CLOCK_CUTS %u\n", cuts);
  return 0;
}
static int oracle(void) {
  struct rpc_clock nodes[3], events[64];
  for (unsigned i = 0; i < 3; ++i)
    CHECK(rpc_make(9, 10 + i * 10, &nodes[i]) == RPC_OK);
  unsigned count = 0, node;
  int remote;
  uint64_t wall;
  while (scanf("%u %d %" SCNu64, &node, &remote, &wall) == 3) {
    CHECK(node < 3 && remote >= -1 &&
          (remote < 0 || (unsigned)remote < count) && count < 64);
    struct rpc_clock next;
    int r = rpc_step(&nodes[node], remote < 0 ? 0 : &events[remote], wall, 4,
                     RPC_REJECT, &next);
    CHECK(r == RPC_OK);
    nodes[node] = next;
    events[count++] = next;
    printf("%" PRIu64 " %" PRIu64 " %" PRIu32, next.lamport, next.hlc.physical,
           next.hlc.logical);
    for (unsigned id = 10; id <= 30; id += 10) {
      uint64_t v = 0;
      for (unsigned j = 0; j < next.vector.count; ++j)
        if (next.vector.entries[j].identity == id)
          v = next.vector.entries[j].value;
      printf(" %" PRIu64, v);
    }
    printf(" |");
    for (unsigned i = 0; i + 1 < count; ++i) {
      int order = 9;
      CHECK(rpc_order(&events[i].vector, &next.vector, &order) == RPC_OK);
      printf(" %d", order);
    }
    putchar('\n');
  }
  if (count) {
    unsigned char wire[160];
    CHECK(rpc_encode(&events[count - 1], wire) == RPC_OK);
    printf("WIRE ");
    for (unsigned i = 0; i < 160; ++i)
      printf("%02x", wire[i]);
    putchar('\n');
  }
  return 0;
}
static int metrics(void) {
  CHECK(!setup());
  struct rpc_clock a, b;
  CHECK(rpc_make(9, 10, &a) == RPC_OK);
  CHECK(rpc_save(&pages, &a) == RPC_OK);
  CHECK(store_checkpoint(&pages) == STORE_OK);
  uint64_t reads = pages.reads, writes = pages.writes, flushes = pages.flushes;
  CHECK(rpc_step_durable(&pages, &a, 0, 10, 4, RPC_REJECT, &b) == RPC_OK);
  printf("{\"clock_size\":%zu,\"clock_alignment\":%zu,\"vector_size\":%zu,"
         "\"scalar_wire_bytes_without_identity\":8,\"hlc_wire_bytes_without_"
         "identity\":12,\"vector_wire_bytes_per_identity\":16,\"snapshot_wire_"
         "bytes\":160,\"observed_reads\":%" PRIu64
         ",\"observed_writes\":%" PRIu64 ",\"observed_flushes\":%" PRIu64 "}\n",
         sizeof(struct rpc_clock), _Alignof(struct rpc_clock), sizeof(struct rpc_vector),
         pages.reads - reads, pages.writes - writes, pages.flushes - flushes);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "clock_identity"))
    return identity_case();
  if (!strcmp(argv[1], "clock_policy"))
    return policy_case();
  if (!strcmp(argv[1], "clock_store"))
    return store_case();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  if (!strcmp(argv[1], "metrics"))
    return metrics();
  return 2;
}

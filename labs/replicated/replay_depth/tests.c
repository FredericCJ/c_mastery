#include "hashing.h"
#include "replay_depth.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static const RPState initial[2] = {{0, 0, 0, 1}, {0, 7, 0, 9}};
static int same(const RprSession *a, const RprSession *b) {
  unsigned char x[512], y[512];
  return !rpr_encode(a, x) && !rpr_encode(b, y) && !memcmp(x, y, 512);
}
static int prediction(void) {
  RprSession s, saved, fresh;
  RprNotice n, before;
  RprEffect e;
  unsigned i;
  int local[] = {1, 0, -1, 1}, remote[] = {-1, 1, 0, -1};
  CHECK(rpr_init(9, 4, initial, &s) == 0);
  for (i = 0; i < 4; ++i)
    CHECK(rpr_advance(&s, local[i], &n) == 0);
  saved = s;
  memset(&n, 0xa5, sizeof n);
  memcpy(&before, &n, sizeof n);
  CHECK(rpr_advance(&s, 1, &n) == RPR_WAIT && same(&s, &saved) &&
        !memcmp(&n, &before, sizeof n));
  CHECK(rpr_correct(&s, 9, 2, 1, &n) == 0 && n.from == 2 && s.confirmed == 0 &&
        rpr_invariant(&s));
  CHECK(s.remote[2] == 1 && s.remote[3] == 1 &&
        s.snapshots[4][1].position == 10);
  CHECK(rpr_correct(&s, 9, 1, -1, &n) == 0 && s.confirmed == 2);
  CHECK(rpr_correct(&s, 9, 4, -1, &n) == 0 && s.confirmed == 2);
  CHECK(rpr_correct(&s, 9, 3, 0, &n) == 0 && s.confirmed == 4);
  CHECK(rpr_init(9, 4, initial, &fresh) == 0);
  for (i = 0; i < 4; ++i) {
    CHECK(rpr_advance(&fresh, local[i], &n) == 0 &&
          rpr_correct(&fresh, 9, i + 1, remote[i], &n) == 0);
  }
  CHECK(same(&s, &fresh));
  saved = s;
  CHECK(rpr_correct(&s, 9, 1, -1, &n) == 0 && n.from == 0 && same(&s, &saved));
  CHECK(rpr_correct(&s, 9, 1, 0, &n) == RPR_CONFLICT && same(&s, &saved));
  CHECK(rpr_correct(&s, 10, 1, -1, &n) == RPR_STALE);
  s.revision = UINT32_MAX;
  CHECK(rpr_advance(&s, 1, &n) == RPR_RANGE && rpr_prepare(&s, &e) == 0 &&
        rpr_feedback(&s, e, AR_OK) == 0);
  RPState near[2] = {{0, 0, 65535, 1}, {0, 7, 0, 9}};
  CHECK(rpr_init(9, 4, near, &s) == 0 && rpr_advance(&s, 0, &n) == 0);
  saved = s;
  memset(&n, 0xa5, sizeof n);
  memcpy(&before, &n, sizeof n);
  CHECK(rpr_advance(&s, 0, &n) == RPR_RANGE && same(&s, &saved) &&
        !memcmp(&n, &before, sizeof n));
  puts("REPLAY_PREDICTION suffix-replacement changed-last-known horizon-wait "
       "duplicate-conflict finite-revision");
  return 0;
}
static int snapshot(void) {
  RprSession s, got, before;
  RprNotice n;
  unsigned char wire[512], bad[512], input[16];
  RprInput p;
  unsigned i;
  CHECK(rpr_init(9, 4, initial, &s) == 0 && rpr_advance(&s, 1, &n) == 0 &&
        rpr_correct(&s, 9, 1, -1, &n) == 0 && rpr_encode(&s, wire) == 0 &&
        rpr_decode(9, wire, &got) == 0 && same(&s, &got));
  memset(&got, 0xa5, sizeof got);
  memcpy(&before, &got, sizeof got);
  for (i = 0; i < 4096; ++i) {
    memcpy(bad, wire, 512);
    bad[i / 8] ^= (unsigned char)(1u << (i % 8));
    CHECK(rpr_decode(9, bad, &got) == RPR_CORRUPT &&
          !memcmp(&got, &before, sizeof got));
  }
  memcpy(bad, wire, 512);
  bad[80 + 32 + 7] ^= 1;
  CHECK(rph_hash(bad, 480, bad + 480) == 0 &&
        rpr_decode(9, bad, &got) == RPR_CORRUPT);
  CHECK(rpr_input_encode((RprInput){9, 1, -1}, input) == 0 &&
        rpr_input_decode(input, &p) == 0 && p.timeline == 9 && p.tick == 1 &&
        p.movement == -1);
  input[15] = 1;
  CHECK(rpr_input_decode(input, &p) == RPR_CORRUPT);
  puts("REPLAY_SNAPSHOT bits4096 reconstructed-history canonical-padding "
       "full-random-state input-reserved");
  return 0;
}
typedef struct {
  unsigned char volatile_bytes[STORE_IMAGE_PAGES * STORE_PAGE_SIZE],
      durable_bytes[STORE_IMAGE_PAGES * STORE_PAGE_SIZE];
  unsigned operations, fail, persist;
} Disk;
static Disk disk;
static unsigned char baseline[STORE_IMAGE_PAGES * STORE_PAGE_SIZE];
static store pages;
static int read_page(void *ctx, uint32_t page, void *out) {
  Disk *d = ctx;
  if (page >= STORE_IMAGE_PAGES)
    return -1;
  memcpy(out, d->volatile_bytes + page * STORE_PAGE_SIZE, STORE_PAGE_SIZE);
  return 0;
}
static int write_page(void *ctx, uint32_t page, const void *in) {
  Disk *d = ctx;
  if (page >= STORE_IMAGE_PAGES)
    return -1;
  ++d->operations;
  if (d->operations == d->fail) {
    if (d->persist) {
      memcpy(d->volatile_bytes + page * STORE_PAGE_SIZE, in, STORE_PAGE_SIZE);
      memcpy(d->durable_bytes, d->volatile_bytes, sizeof d->durable_bytes);
    }
    return -1;
  }
  memcpy(d->volatile_bytes + page * STORE_PAGE_SIZE, in, STORE_PAGE_SIZE);
  return 0;
}
static int flush(void *ctx) {
  Disk *d = ctx;
  ++d->operations;
  if (d->operations == d->fail) {
    if (d->persist)
      memcpy(d->durable_bytes, d->volatile_bytes, sizeof d->durable_bytes);
    return -1;
  }
  memcpy(d->durable_bytes, d->volatile_bytes, sizeof d->durable_bytes);
  return 0;
}
static store_io io(void) {
  return (store_io){&disk, read_page, write_page, flush, STORE_IMAGE_PAGES};
}
static int setup(RprDurable *d, unsigned op) {
  RprNotice n;
  RprEffect e;
  memset(&disk, 0, sizeof disk);
  CHECK(store_format(io()) == 0 && store_open(&pages, io()) == 0 &&
        rpr_create(&pages, 9, 4, initial, d) == 0);
  if (op >= 1)
    CHECK(rpr_advance_durable(d, 1, &n) == 0);
  if (op >= 2)
    CHECK(rpr_correct_durable(d, 9, 1, -1, &n) == 0);
  if (op >= 3)
    CHECK(rpr_prepare_durable(d, &e) == 0);
  CHECK(store_checkpoint(&pages) == 0);
  return 0;
}
static int transition(RprDurable *d, unsigned op) {
  RprNotice n;
  RprEffect e;
  if (op == 0)
    return rpr_advance_durable(d, 1, &n);
  if (op == 1)
    return rpr_correct_durable(d, 9, 1, -1, &n);
  if (op == 2)
    return rpr_prepare_durable(d, &e);
  e = (RprEffect){9, d->session.effects.effect.id,
                  d->session.effects.effect.value};
  return rpr_feedback_durable(d, e, AR_OK);
}
static int durable(void) {
  RprDurable d, stale, opened;
  RprSession old, newstate;
  RprNotice n;
  unsigned op, cut, mode, cases = 0;
  CHECK(setup(&d, 0) == 0);
  stale = d;
  CHECK(rpr_advance_durable(&d, 1, &n) == 0 &&
        rpr_advance_durable(&stale, 1, &n) == RPR_STATE);
  for (op = 0; op < 4; ++op) {
    CHECK(setup(&d, op) == 0);
    old = d.session;
    memcpy(baseline, disk.durable_bytes, sizeof baseline);
    disk.operations = 0;
    CHECK(transition(&d, op) == 0 && disk.operations == 37);
    newstate = d.session;
    for (mode = 0; mode < 2; ++mode)
      for (cut = 1; cut <= 37; ++cut) {
        memcpy(disk.durable_bytes, baseline, sizeof baseline);
        memcpy(disk.volatile_bytes, baseline, sizeof baseline);
        disk.fail = 0;
        CHECK(store_open(&pages, io()) == 0 && rpr_open(&pages, 9, &d) == 0);
        disk.operations = 0;
        disk.fail = cut;
        disk.persist = mode;
        CHECK(transition(&d, op) == RPR_UNCERTAIN && same(&d.session, &old));
        disk.fail = 0;
        memcpy(disk.volatile_bytes, disk.durable_bytes,
               sizeof disk.volatile_bytes);
        CHECK(store_open(&pages, io()) == 0 &&
              rpr_open(&pages, 9, &opened) == 0);
        CHECK(same(&opened.session, mode && cut >= 36 ? &newstate : &old));
        ++cases;
      }
  }
  printf("REPLAY_DURABLE cuts%u four-transitions old-or-new "
         "no-early-publication stale-control\n",
         cases);
  return 0;
}
static int effects(void) {
  RprSession s, saved;
  RprNotice n;
  RprEffect e, retry, bad;
  unsigned i;
  int32_t value;
  CHECK(rpr_init(9, 4, initial, &s) == 0 && rpr_advance(&s, 1, &n) == 0 &&
        rpr_prepare(&s, &e) == RPR_EMPTY);
  CHECK(rpr_correct(&s, 9, 1, -1, &n) == 0 && rpr_prepare(&s, &e) == 0 &&
        rpr_value(&s, 1, &value) == 0 && e.value == value * 2);
  saved = s;
  CHECK(rpr_prepare(&s, &retry) == 0 && retry.timeline == e.timeline &&
        retry.tick == e.tick && retry.value == e.value && same(&s, &saved));
  CHECK(rpr_feedback(&s, e, AR_FAILED) == RPR_IO && same(&s, &saved));
  bad = e;
  ++bad.timeline;
  CHECK(rpr_feedback(&s, bad, AR_OK) == RPR_STALE && same(&s, &saved));
  CHECK(rpr_feedback(&s, e, AR_OK) == 0 && rpr_prepare(&s, &e) == RPR_EMPTY);
  for (i = 1; i < 8; ++i) {
    CHECK(rpr_advance(&s, 0, &n) == 0 &&
          rpr_correct(&s, 9, i + 1, 1, &n) == 0 && rpr_prepare(&s, &e) == 0 &&
          e.tick == i + 1 && rpr_feedback(&s, e, AR_OK) == 0);
  }
  CHECK(rpr_prepare(&s, &e) == RPR_EMPTY && rpr_advance(&s, 0, &n) == RPR_FULL);
  puts("REPLAY_EFFECTS confirmed-only actual-ar-adapter stable-retry "
       "identity-feedback terminal-eight");
  return 0;
}
static int transport(void) {
  unsigned char wire[32], raw[16], split[19];
  RprInput p, before = {7, 7, 0};
  unsigned chunk, i;
  CHECK(rpr_input_frame((RprInput){9, 3, -1}, wire) == 0);
  for (chunk = 1; chunk <= 32; ++chunk)
    CHECK(rpr_input_unframe(wire, 18, chunk, &p) == 0 && p.timeline == 9 &&
          p.tick == 3 && p.movement == -1);
  for (i = 0; i < 18; ++i) {
    p = before;
    CHECK(rpr_input_unframe(wire, i, 3, &p) == RPR_CORRUPT &&
          !memcmp(&p, &before, sizeof p));
  }
  wire[18] = 9;
  CHECK(rpr_input_unframe(wire, 19, 1, &p) == RPR_CORRUPT);
  CHECK(rpr_input_encode((RprInput){9, 3, -1}, raw) == 0);
  split[0] = 7;
  memcpy(split + 1, raw, 7);
  split[8] = 7;
  memcpy(split + 9, raw + 7, 7);
  split[16] = 2;
  memcpy(split + 17, raw + 14, 2);
  CHECK(rpr_input_unframe(split, 19, 3, &p) == RPR_CORRUPT);
  puts("REPLAY_TRANSPORT actual-parser two-exact-frames chunks32 truncations18 "
       "trailing noncanonical-segmentation");
  return 0;
}
int rpr_oracle(void);
int rpr_metrics(void);
int rpr_counterexamples(void);
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "replay_prediction"))
    return prediction();
  if (!strcmp(argv[1], "replay_snapshot"))
    return snapshot();
  if (!strcmp(argv[1], "replay_durable"))
    return durable();
  if (!strcmp(argv[1], "replay_effects"))
    return effects();
  if (!strcmp(argv[1], "replay_transport"))
    return transport();
  if (!strcmp(argv[1], "oracle"))
    return rpr_oracle();
  if (!strcmp(argv[1], "metrics"))
    return rpr_metrics();
  if (!strcmp(argv[1], "counterexamples"))
    return rpr_counterexamples();
  return 2;
}

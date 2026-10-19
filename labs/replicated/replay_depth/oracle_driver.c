#include "hashing.h"
#include "replay_depth.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RP_CHECK oracle:%d %s\n", __LINE__, #x);                \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static const RPState initial[2] = {{0, 0, 0, 1}, {0, 7, 0, 9}};
static int capture(const RprSession *s, RphSha *h) {
  unsigned char b[512];
  RprSession copy;
  if (rpr_encode(s, b) || rpr_decode(9, b, &copy) || !rpr_invariant(&copy))
    return 1;
  return rph_update(h, b, sizeof b);
}
int rpr_oracle(void) {
  unsigned n, lc, rc, order;
  while (scanf("%u %u %u %u", &n, &lc, &rc, &order) == 4) {
    RprSession s;
    RprNotice notice;
    RprEffect effect;
    RphSha h;
    int local[8], remote[8];
    unsigned char digest[32];
    CHECK(n <= 8 && rpr_init(9, 8, initial, &s) == 0 && rph_init(&h) == 0 &&
          capture(&s, &h) == 0);
    for (unsigned i = 0; i < n; ++i) {
      local[i] = (int)(lc % 3) - 1;
      lc /= 3;
      remote[i] = (int)(rc % 3) - 1;
      rc /= 3;
    }
    for (unsigned i = 0; i < n; ++i)
      CHECK(rpr_advance(&s, local[i], &notice) == 0 && capture(&s, &h) == 0);
    for (unsigned i = 0; i < n; ++i) {
      unsigned tick = order % 9;
      order /= 9;
      CHECK(tick && tick <= n &&
            rpr_correct(&s, 9, tick, remote[tick - 1], &notice) == 0 &&
            capture(&s, &h) == 0);
    }
    CHECK(!order && s.confirmed == n);
    for (unsigned i = 0; i < n; ++i)
      CHECK(rpr_prepare(&s, &effect) == 0 && capture(&s, &h) == 0 &&
            rpr_feedback(&s, effect, AR_OK) == 0 && capture(&s, &h) == 0);
    CHECK(rph_final(&h, digest) == 0);
    for (unsigned i = 0; i < 32; ++i)
      printf("%02x", digest[i]);
    putchar('\n');
  }
  return ferror(stdin) ? 1 : 0;
}
int rpr_metrics(void) {
  RprSession s;
  RprNotice notice;
  RprEffect effect;
  const unsigned runs = 2000;
  double times[7];
  for (unsigned sample = 0; sample < 7; ++sample) {
    clock_t start = clock();
    for (unsigned run = 0; run < runs; ++run) {
      CHECK(rpr_init(9, 8, initial, &s) == 0);
      for (unsigned i = 0; i < 8; ++i)
        CHECK(rpr_advance(&s, 1, &notice) == 0);
      for (unsigned i = 8; i; --i)
        CHECK(rpr_correct(&s, 9, i, -1, &notice) == 0);
      for (unsigned i = 0; i < 8; ++i)
        CHECK(rpr_prepare(&s, &effect) == 0 &&
              rpr_feedback(&s, effect, AR_OK) == 0);
    }
    times[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
  }
  printf("{\"session_size\":%zu,\"durable_size\":%zu,\"snapshot_bytes\":512,"
         "\"input_bytes\":16,\"framed_bytes\":18,\"horizon\":8,\"runs\":%u,"
         "\"cpu_seconds\":[",
         sizeof s, sizeof(RprDurable), runs);
  for (unsigned i = 0; i < 7; ++i)
    printf("%s%.9f", i ? "," : "", times[i]);
  puts("]}");
  return 0;
}
/* A deliberately flawed original reducer: its hidden random state is omitted
   from the replay snapshot. A mismatch is evidence of nondeterminism only. */
static uint32_t hidden = 1;
static uint32_t hidden_step(unsigned input) {
  hidden = hidden * 1664525u + 1013904223u;
  return input + (hidden & 65535u);
}
int rpr_counterexamples(void) {
  unsigned inputs[3] = {1, 2, 3};
  uint32_t first = 0, replay = 0;
  for (unsigned i = 0; i < 3; ++i)
    first += hidden_step(inputs[i]);
  for (unsigned i = 0; i < 3; ++i)
    replay += hidden_step(inputs[i]);
  CHECK(first != replay);
  /* A callback outside the durable transaction is repeated after an uncertain
     acknowledgement, even though both deliveries name the same effect. */
  unsigned external_counter = 0;
  RprEffect a = {9, 1, 22}, b = a;
  if (a.timeline == 9 && a.tick == 1)
    ++external_counter;
  if (b.timeline == 9 && b.tick == 1)
    ++external_counter;
  CHECK(external_counter == 2);
  printf("REPLAY_COUNTEREXAMPLES hidden-random-first%u replay%u "
         "external-effect-count%u\n",
         first, replay, external_counter);
  return 0;
}

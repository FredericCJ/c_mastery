#include "observability.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "OBSERVABILITY-CHECK:%d:%s\n", __LINE__, #x);            \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int sink(void *v, const unsigned char *p, size_t n) {
  (void)p;
  (void)n;
  return *(int *)v;
}
typedef struct {
  AeReporter *reporter;
  AeRecord *record;
  unsigned calls, depth, maxdepth;
} Broken;
static int broken(void *v, const unsigned char *p, size_t n) {
  (void)p;
  (void)n;
  Broken *b = v;
  ++b->calls;
  ++b->depth;
  if (b->depth > b->maxdepth)
    b->maxdepth = b->depth;
  (void)ae_report(b->reporter, b->record, broken, b);
  --b->depth;
  return -1;
}
int main(void) {
  unsigned cases = 0;
  AeRecord r = {3, 1, 16, 3, 4, 65535, {1, 4, 9, 2}}, out = {0};
  unsigned char b[12];
  const unsigned char golden[12] = {1, 3, 1, 16, 3, 4, 255, 255, 1, 4, 9, 2};
  CHECK(ae_record_encode(&r, b, 12) == AE_OK && !memcmp(b, golden, 12));
  CHECK(ae_record_decode(b, 12, &out) == AE_OK && out.tick == 65535 &&
        out.payload[2] == 9);
  for (unsigned n = 0; n < 12; ++n) {
    out = (AeRecord){0};
    CHECK(ae_record_decode(b, n, &out) == AE_INVALID && out.tick == 0);
  }
  for (unsigned severity = 0; severity < 4; ++severity)
    for (unsigned unit = 1; unit <= 2; ++unit)
      for (unsigned width = 8; width <= 16; width += 8)
        for (unsigned gen = 1; gen <= 3; ++gen)
          for (unsigned length = 0; length <= 4; ++length) {
            r = (AeRecord){severity,    unit,   width,
                           gen,         length, width == 8 ? 255u : 65535u,
                           {1, 4, 9, 2}};
            CHECK(ae_record_encode(&r, b, 12) == AE_OK &&
                  ae_record_decode(b, 12, &out) == AE_OK);
            CHECK(out.length == length && out.generation == gen &&
                  out.width == width);
            ++cases;
          }
  for (unsigned a = 0; a < 256; ++a)
    for (unsigned z = 0; z < 256; ++z) {
      int order = 99;
      unsigned d = (a + 256 - z) % 256;
      AeCode code = ae_tick_order(a, z, 8, &order);
      CHECK(code == (d == 128 ? AE_AMBIGUOUS : AE_OK));
      if (d == 128)
        CHECK(order == 99);
      else
        CHECK(order == (d == 0 ? 0 : d < 128 ? 1 : -1));
    }
  int order = 99;
  CHECK(ae_tick_order(0, 65535, 16, &order) == AE_OK && order == 1);
  CHECK(ae_tick_order(32768, 0, 16, &order) == AE_AMBIGUOUS);
  AeLog l = {.rate = {3, 0}};
  r = (AeRecord){0, 1, 8, 1, 1, 0, {0}};
  for (unsigned i = 0; i < 5; ++i) {
    r.payload[0] = (unsigned char)('A' + i);
    CHECK(ae_rate_refill(&l.rate, 2) == AE_OK &&
          ae_log(&l, &r, AE_DROP_OLDEST) == AE_OK);
  }
  CHECK(l.count == 3 && l.dropped == 2 && l.records[0].payload[0] == 'C' &&
        l.records[2].payload[0] == 'E');
  int state = 0;
  CHECK(ae_log_drain(&l, sink, &state) == AE_AGAIN && l.count == 3 &&
        l.stalls == 1);
  state = -1;
  CHECK(ae_log_drain(&l, sink, &state) == AE_DEVICE && l.count == 3);
  state = 1;
  CHECK(ae_log_drain(&l, sink, &state) == AE_OK && l.count == 2);
  l.rate.tokens = 0;
  CHECK(ae_log(&l, &r, AE_DROP_OLDEST) == AE_AGAIN && l.limited == 1 &&
        l.count == 2);
  CHECK(ae_rate_refill(&l.rate, 1) == AE_OK && l.rate.tokens == 0);
  CHECK(ae_rate_refill(&l.rate, 1) == AE_OK && l.rate.tokens == 1);
  CHECK(ae_log(&l, &r, AE_DROP_OLDEST) == AE_OK);
  l.rate.tokens = 3;
  r.severity = 3;
  CHECK(ae_log(&l, &r, AE_PROTECT_HIGH) == AE_FULL && l.rejected == 1 &&
        l.rate.tokens == 3);
  l.records[0].severity = 3;
  r.severity = 0;
  CHECK(ae_log(&l, &r, AE_PROTECT_HIGH) == AE_FULL &&
        l.records[0].severity == 3);
  AeReporter reporter = {.fallback_available = 1};
  Broken x = {&reporter, &r, 0, 0, 0};
  CHECK(ae_report(&reporter, &r, broken, &x) == AE_DEVICE && x.calls == 1 &&
        x.maxdepth == 1 && reporter.stored == 2 && !reporter.busy);
  for (unsigned i = 0; i < 5; ++i)
    CHECK(ae_report(&reporter, &r, broken, &x) == AE_DEVICE);
  CHECK(reporter.lost == 10 && x.calls == 6 && x.maxdepth == 1);
  reporter = (AeReporter){0};
  CHECK(ae_report(&reporter, &r, broken, &x) == AE_DEVICE &&
        reporter.stored == 0 && reporter.lost == 2);
  printf("OBSERVABILITY-PASS:codec_cases=%u;clock_pairs=65536;backend_depth=1;"
         "record_bytes=12;\n",
         cases);
  return 0;
}

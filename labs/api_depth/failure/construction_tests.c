#include "construction.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CONSTRUCTION-CHECK:%d:%s\n", __LINE__, #x);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int run_case(unsigned count, unsigned swapped, unsigned fail,
                    unsigned mask, unsigned consumes, unsigned style) {
  struct ac_ledger ledger = {0};
  for (unsigned i = 0; i < 4; ++i)
    ledger.consume_on_error[i] = !!(consumes & (1u << i));
  struct ae_construction c;
  ae_construction_init(&c, &ledger);
  struct ae_construction_handle out = {0, 99};
  enum ae_code r = ae_construct(&c, count, swapped, fail, mask,
                          (enum ae_construction_style)style, &out);
  unsigned order[4] = {swapped ? 1u : 0u, swapped ? 0u : 1u, 2, 3};
  unsigned acquired = fail && fail <= count ? fail - 1 : count;
  unsigned remaining = 0;
  for (unsigned i = 0; i < acquired; ++i)
    remaining |= 1u << order[i];
  CHECK(r == (fail ? (fail <= count ? AE_EXHAUSTED : AE_DEVICE) : AE_OK));
  CHECK(ledger.count >= acquired);
  for (unsigned i = 0; i < acquired; ++i)
    CHECK(ledger.events[i].acquire == 1 &&
          ledger.events[i].resource == order[i] &&
          ledger.events[i].request == 1);
  if (!fail) {
    CHECK(out.owner == &c && out.generation == 1 && c.state == AE_CACTIVE);
    CHECK(ae_cleanup(&c, mask) == (mask & remaining ? AE_DEVICE : AE_OK));
  } else
    CHECK(!out.owner && out.generation == 99 && !c.published);
  /* Independent graph model: children2/3 depend on0/1; failed nonconsuming
   * close retains child and blocks parent. */
  unsigned event = acquired;
  for (unsigned pos = acquired; pos > 0; --pos) {
    unsigned id = order[pos - 1];
    if (id < 2 && (remaining & 12u))
      continue;
    int failure = !!(mask & (1u << id));
    if (failure && !(consumes & (1u << id)))
      continue;
    CHECK(event < ledger.count && ledger.events[event].resource == id &&
          !ledger.events[event].acquire && ledger.events[event].request == 1);
    ++event;
    remaining &= ~(1u << id);
  }
  CHECK(event == ledger.count && c.mask == remaining);
  for (unsigned i = 0; i < 4; ++i)
    CHECK(ledger.held[i] == !!(remaining & (1u << i)) &&
          ledger.acquisitions[i] <= 1 && ledger.releases[i] <= 1);
  CHECK(ae_cleanup(&c, 0) == AE_OK && c.mask == 0);
  unsigned old_count = ledger.count;
  CHECK(ae_cleanup(&c, 0) == AE_OK && ledger.count == old_count);
  for (unsigned i = 0; i < 4; ++i)
    CHECK(!ledger.held[i] && ledger.acquisitions[i] == ledger.releases[i]);
  return 0;
}
int main(void) {
  unsigned cases = 0;
  for (unsigned count = 3; count <= 4; ++count)
    for (unsigned swapped = 0; swapped < 2; ++swapped)
      for (unsigned fail = 0; fail <= count + 1; ++fail)
        for (unsigned mask = 0; mask < 16; ++mask)
          for (unsigned consumes = 0; consumes < 16; ++consumes)
            for (unsigned style = 0; style < 3; ++style) {
              CHECK(!run_case(count, swapped, fail, mask, consumes, style));
              ++cases;
            }
  struct ac_ledger ledger = {0};
  struct ae_construction c;
  ae_construction_init(&c, &ledger);
  struct ae_construction_handle h = {0};
  CHECK(ae_construct(&c, 3, 0, 2, 0, AE_LABELS, &h) == AE_EXHAUSTED);
  CHECK(ae_cleanup(&c, 0) == AE_OK && ae_cleanup(&c, 0) == AE_OK);
  CHECK(ae_construct(&c, 3, 0, 0, 0, AE_LABELS, &h) == AE_OK &&
        h.generation == 2);
  struct ae_construction_handle old = h;
  CHECK(ae_construct(&c, 3, 0, 0, 0, AE_NESTED, &h) == AE_CONFLICT &&
        h.generation == 2);
  CHECK(ae_cleanup(&c, 0) == AE_OK && ae_cleanup(&c, 0) == AE_OK);
  CHECK(ae_construct(&c, 3, 0, 0, 0, AE_LOOP, &h) == AE_OK &&
        h.generation == 3);
  unsigned value = 999;
  CHECK(ae_construction_read(old, &value) == AE_CONFLICT && value == 999);
  CHECK(ae_construction_read(h, &value) == AE_OK && value == 7);
  CHECK(ae_cleanup(&c, 0) == AE_OK);
  CHECK(ae_construct(&c, 3, 0, 0, 0, AE_LOOP, &h) == AE_FULL);
  ledger = (struct ac_ledger){0};
  ae_construction_init(&c, &ledger);
  CHECK(ae_construct(&c, 3, 0, 0, 0, AE_NESTED, &h) == AE_OK);
  CHECK(ae_transfer(&c, 2) == AE_OK && c.transfers[0].resource == 2 &&
        c.transfers[0].generation == 1);
  CHECK(ae_cleanup(&c, 0) == AE_DEVICE && c.mask == 3 && ledger.held[2]);
  CHECK(ac_resource_close(&ledger, 2, 0) == AC_OK);
  CHECK(ae_cleanup(&c, 0) == AE_OK);
  for (unsigned i = 0; i < 3; ++i)
    CHECK(ledger.acquisitions[i] == 1 && ledger.releases[i] == 1);
  ledger = (struct ac_ledger){0};
  ae_construction_init(&c, &ledger);
  CHECK(ae_construct(&c, 3, 0, 0, 0, AE_LOOP, &h) == AE_OK);
  CHECK(ae_after_publish(&c, 1) == AE_AMBIGUOUS && c.committed && c.published &&
        c.error.first == 88);
  CHECK(ae_cleanup(&c, 0) == AE_CONFLICT && c.mask == 7);
  CHECK(ae_compensate(&c, 1) == AE_AGAIN && c.committed);
  CHECK(ae_compensate(&c, 0) == AE_OK && ae_cleanup(&c, 0) == AE_OK);
  printf("CONSTRUCTION-PASS:cases=%u;styles=3;resource_limits=4;generation_"
         "limit=3;\n",
         cases);
  return 0;
}

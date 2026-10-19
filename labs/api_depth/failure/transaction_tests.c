#include "transaction.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "TRANSACTION-CHECK:%d:%s\n", __LINE__, #x);              \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  AeConfig old;
  unsigned calls, bad;
} Watch;
static void watch(void *v, const AeConfig *c) {
  Watch *w = v;
  ++w->calls;
  if (c->version == w->old.version && memcmp(c, &w->old, sizeof *c))
    ++w->bad;
}
int main(void) {
  unsigned cases = 0, new_values[3] = {3, 4, 5};
  for (unsigned n = 0; n <= 3; ++n)
    for (unsigned cap = 0; cap <= 3; ++cap)
      for (unsigned mode = 0; mode < 3; ++mode)
        for (unsigned fail = 0; fail <= n + 3; ++fail)
          for (unsigned notify = 0; notify < 2; ++notify) {
            AeConfig c = {{1, 2, 0}, 2, 1}, old = c;
            Watch w = {old, 0, 0};
            AeUpdate r = ae_replace(&c, new_values, n, cap, (AeGuarantee)mode,
                                    fail, notify, watch, &w);
            unsigned copies = n < cap ? n : cap;
            AeCode code = AE_OK;
            if (fail == 1) {
              code = AE_DEVICE;
              copies = 0;
            } else if (mode == AE_STRONG && n > cap) {
              code = AE_FULL;
              copies = 0;
            } else if (fail == 2) {
              code = AE_EXHAUSTED;
              copies = 0;
            } else if (fail >= 3 && fail < copies + 3) {
              copies = fail - 3;
              code = AE_DEVICE;
            } else if (n > cap)
              code = AE_FULL;
            else if (fail == n + 3)
              code = AE_DEVICE;
            unsigned publish =
                fail != 1 && !(code != AE_OK && mode == AE_STRONG);
            CHECK(!w.bad && w.calls && r.committed == publish);
            CHECK(r.code == (code == AE_OK && notify ? AE_AMBIGUOUS : code));
            if (!publish)
              CHECK(!memcmp(&c, &old, sizeof c) && r.accepted == 0);
            else {
              if (code != AE_OK && mode == AE_BASIC)
                copies = 0;
              CHECK(c.count == copies && c.version == 2 &&
                    r.accepted == copies);
              for (unsigned i = 0; i < 3; ++i)
                CHECK(c.values[i] == (i < copies ? new_values[i] : 0));
            }
            ++cases;
          }
  AeConfig c = {{1, 2, 0}, 2, 1};
  CHECK(ae_replace(&c, c.values, 2, 2, AE_STRONG, 0, 0, 0, 0).code == AE_OK &&
        c.values[1] == 2);
  c.version = 3;
  AeConfig old = c;
  CHECK(ae_replace(&c, new_values, 3, 3, AE_STRONG, 0, 0, 0, 0).code ==
            AE_FULL &&
        !memcmp(&c, &old, sizeof c));
  AeJournal j = {0};
  CHECK(ae_journal(&j, 1, 2, 0) == AE_OK && ae_journal(&j, 1, 2, 0) == AE_OK &&
        j.effects == 1);
  CHECK(ae_journal(&j, 1, 3, 0) == AE_CONFLICT);
  CHECK(ae_journal(&j, 2, 1, 0) == AE_OK && ae_journal(&j, 3, 1, 0) == AE_OK);
  CHECK(ae_journal(&j, 1, 2, 0) == AE_OK && j.effects == 4 && j.value == 6);
  for (unsigned crash = 1; crash <= 3; ++crash) {
    j = (AeJournal){0};
    CHECK(ae_journal(&j, 1, 2, crash) == AE_AMBIGUOUS);
    unsigned effects = j.effects;
    CHECK(ae_journal(&j, 1, 2, 0) == (crash == 3 ? AE_OK : AE_AMBIGUOUS) &&
          j.effects == effects);
    CHECK(j.effects == (crash == 1 ? 0u : 1u));
    CHECK(ae_journal(&j, 1, 3, 0) == AE_CONFLICT);
  }
  printf("TRANSACTION-PASS:cases=%u;crash_cuts=3;dedupe_history=2;\n", cases);
  return 0;
}

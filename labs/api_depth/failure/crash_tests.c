#include "crash.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CRASH-CHECK:%d:%s\n", __LINE__, #x);                    \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  const unsigned char golden[5] = {1, 4, 9, 2, 16};
  unsigned char b[5];
  struct ae_fault f = {4, 9, 2}, out = {0};
  CHECK(ae_fault_encode(f, b, 5) == AE_OK && !memcmp(b, golden, 5));
  CHECK(ae_fault_decode(b, 5, &out) == AE_OK && out.build == 4 &&
        out.address == 9 && out.state == 2);
  for (unsigned n = 0; n < 5; ++n) {
    out = (struct ae_fault){0};
    CHECK(ae_fault_decode(b, n, &out) == AE_INVALID && out.build == 0);
  }
  b[4] = 17;
  CHECK(ae_fault_decode(b, 5, &out) == AE_INVALID);
  b[4] = 16;
  unsigned cuts = 0;
  for (unsigned history = 0; history < 3; ++history)
    for (unsigned cut = 0; cut <= 10; ++cut) {
      struct ae_crash_slots slots = {0};
      for (unsigned g = 1; g <= history; ++g)
        CHECK(ae_fault_save(&slots, (struct ae_fault){4, g, 2}, 0) == AE_OK);
      CHECK(ae_fault_save(&slots, f, cut) == (cut ? AE_AMBIGUOUS : AE_OK));
      unsigned generation = 99;
      out = (struct ae_fault){0};
      enum ae_code result = ae_fault_select(&slots, &out, &generation);
      unsigned committed = cut == 0 || cut == 10;
      if (!history && !committed)
        CHECK(result == AE_UNKNOWN && generation == 99);
      else
        CHECK(result == AE_OK && generation == history + committed &&
              out.address == (committed ? 9 : history));
      ++cuts;
    }
  struct ae_crash_slots slots = {0};
  for (unsigned i = 0; i < 3; ++i)
    CHECK(ae_fault_save(&slots, f, 0) == AE_OK);
  struct ae_crash_slots before = slots;
  CHECK(ae_fault_save(&slots, f, 0) == AE_FULL &&
        !memcmp(&slots, &before, sizeof slots));
  unsigned generation;
  for (unsigned byte = 0; byte < 8; ++byte) {
    slots = before;
    slots.bytes[0][byte] ^= 1;
    CHECK(ae_fault_select(&slots, &out, &generation) == AE_OK &&
          generation == 2);
  }
  slots = before;
  memcpy(slots.bytes[1], slots.bytes[0], 8);
  CHECK(ae_fault_select(&slots, &out, &generation) == AE_CONFLICT);
  for (unsigned cut = 1; cut <= 5; ++cut) {
    struct ae_recovery recovery;
    ae_recovery_init(&recovery);
    CHECK(ae_recover(&recovery, b, 5, 4, cut) == AE_AMBIGUOUS);
    CHECK(ae_recover(&recovery, b, 5, 4, cut) ==
          (cut == 5 ? AE_OK : AE_AMBIGUOUS));
    CHECK(ae_recover(&recovery, b, 5, 4, 0) == AE_OK && recovery.done &&
          recovery.effects.effects == 1 && recovery.effects.value == 1 &&
          recovery.owner.mask == 7);
    for (unsigned i = 0; i < 3; ++i)
      CHECK(recovery.ledger.held[i] &&
            recovery.ledger.acquisitions[i] == recovery.ledger.releases[i] + 1);
    unsigned events = recovery.ledger.count;
    CHECK(ae_recover(&recovery, b, 5, 4, 0) == AE_OK &&
          recovery.ledger.count == events && recovery.effects.effects == 1);
  }
  struct ae_recovery recovery;
  ae_recovery_init(&recovery);
  struct ae_recovery saved = recovery;
  CHECK(ae_recover(&recovery, b, 5, 5, 0) == AE_CONFLICT &&
        !memcmp(&recovery, &saved, sizeof recovery));
  CHECK(ae_fault_encode((struct ae_fault){4, 9, 3}, b, 5) == AE_OK &&
        ae_fault_decode(b, 5, &out) == AE_OK);
  CHECK(ae_recover(&recovery, b, 5, 4, 0) == AE_CONFLICT && !recovery.done &&
        !recovery.effects.effects);
  printf(
      "CRASH-PASS:slot_cuts=%u;record_bytes=5;slot_bytes=8;restart_cuts=5;\n",
      cuts);
  return 0;
}

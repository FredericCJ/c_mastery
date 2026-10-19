#include "crash.h"
AeCode ae_fault_encode(AeFault f, unsigned char *out, size_t n) {
  if (!out || n < 5 || !f.build || f.build > 7 || f.address > 255 ||
      f.state > 3)
    return AE_INVALID;
  unsigned char b[5] = {1, (unsigned char)f.build, (unsigned char)f.address,
                        (unsigned char)f.state, 0};
  b[4] = (unsigned char)((1u + f.build + f.address + f.state) & 255u);
  for (unsigned i = 0; i < 5; ++i)
    out[i] = b[i];
  return AE_OK;
}
AeCode ae_fault_decode(const unsigned char *p, size_t n, AeFault *out) {
  if (!p || !out || n != 5 || p[0] != 1 || !p[1] || p[1] > 7 || p[3] > 3)
    return AE_INVALID;
#ifndef AE_MUTANT_CHECKSUM
  if (((unsigned)p[0] + p[1] + p[2] + p[3]) % 256 != p[4])
    return AE_INVALID;
#endif
  *out = (AeFault){p[1], p[2], p[3]};
  return AE_OK;
}
static unsigned valid(const unsigned char *b, AeFault *f) {
  unsigned sum = 0;
  for (unsigned i = 0; i < 6; ++i)
    sum += b[i];
  return b[7] == 165 && b[0] >= 1 && b[0] <= 3 && (sum & 255u) == b[6] &&
         ae_fault_decode(b + 1, 5, f) == AE_OK;
}
AeCode ae_fault_select(const AeCrashSlots *s, AeFault *out,
                       unsigned *generation) {
  if (!s || !out || !generation)
    return AE_INVALID;
  AeFault a, b;
  unsigned x = valid(s->bytes[0], &a), y = valid(s->bytes[1], &b);
  if (!x && !y)
    return AE_UNKNOWN;
  if (x && y && s->bytes[0][0] == s->bytes[1][0])
    return AE_CONFLICT;
  if (x && (!y || s->bytes[0][0] > s->bytes[1][0])) {
    *out = a;
    *generation = s->bytes[0][0];
  } else {
    *out = b;
    *generation = s->bytes[1][0];
  }
  return AE_OK;
}
AeCode ae_fault_save(AeCrashSlots *s, AeFault fault, unsigned cut) {
  if (!s || cut > 10)
    return AE_INVALID;
  unsigned char payload[5];
  if (ae_fault_encode(fault, payload, 5) != AE_OK)
    return AE_INVALID;
  AeFault old;
  unsigned generation = 0;
  AeCode status = ae_fault_select(s, &old, &generation);
  if (status != AE_OK && status != AE_UNKNOWN)
    return status;
  if (generation == 3)
    return AE_FULL;
  unsigned target = 0;
  if (status == AE_OK) {
    AeFault ignored;
    target =
        valid(s->bytes[0], &ignored) && s->bytes[0][0] == generation ? 1 : 0;
  }
  unsigned char next[8] = {
      (unsigned char)(generation + 1), 0, 0, 0, 0, 0, 0, 165};
  unsigned sum = next[0];
  for (unsigned i = 0; i < 5; ++i) {
    next[i + 1] = payload[i];
    sum += payload[i];
  }
  next[6] = (unsigned char)(sum & 255u);
  for (unsigned i = 0; i < 9; ++i) {
    if (cut == i + 1)
      return AE_AMBIGUOUS;
    if (i == 0)
      s->bytes[target][7] = 0;
    else
      s->bytes[target][i - 1] = next[i - 1];
  }
  return cut == 10 ? AE_AMBIGUOUS : AE_OK;
}
void ae_recovery_init(AeRecovery *r) {
  if (r) {
    *r = (AeRecovery){0};
    ae_construction_init(&r->owner, &r->ledger);
  }
}
AeCode ae_recover(AeRecovery *r, const unsigned char *p, size_t n,
                  unsigned expected, unsigned cut) {
  AeFault fault;
  if (!r || cut > 5 || ae_fault_decode(p, n, &fault) != AE_OK)
    return AE_INVALID;
  if (fault.build != expected || fault.state != 2)
    return AE_CONFLICT;
  if (r->done)
    return AE_OK;
  if (r->attempts == 8)
    return AE_FULL;
  ++r->attempts;
  if (cut == 1)
    return AE_AMBIGUOUS;
  if (ae_cleanup(&r->owner, 0) != AE_OK)
    return AE_DEVICE;
  if (cut == 2)
    return AE_AMBIGUOUS;
  /* All old internal handles are unexported and quiescent; ledger proves empty.
   * Start a fresh private control lifetime, not a public generation reset. */
  for (unsigned i = 0; i < 4; ++i)
    if (r->ledger.held[i])
      return AE_CONTRACT;
  ae_construction_init(&r->owner, &r->ledger);
  AeConstructionHandle private_handle;
  if (ae_construct(&r->owner, 3, 0, 0, 0, AE_LABELS, &private_handle) != AE_OK)
    return AE_EXHAUSTED;
  if (cut == 3)
    return AE_AMBIGUOUS;
  if (ae_journal(&r->effects, 1, 1, 0) != AE_OK)
    return AE_CONFLICT;
  if (cut == 4)
    return AE_AMBIGUOUS;
  r->done = 1;
  return cut == 5 ? AE_AMBIGUOUS : AE_OK;
}

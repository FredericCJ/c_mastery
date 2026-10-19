#include "transaction.h"
AeUpdate ae_replace(AeConfig *c, const unsigned *input, unsigned n,
                    unsigned cap, AeGuarantee guarantee, unsigned fail,
                    unsigned notify, AeObserve observe, void *context) {
  AeUpdate result = {AE_INVALID, 0, 0};
  if (!c || (!input && n) || n > 3 || cap > 3 || c->count > 3 ||
      c->version > 3 || (unsigned)guarantee > AE_PARTIAL || fail > n + 3 ||
      notify > 1)
    return result;
  unsigned source[3] = {0};
  for (unsigned i = 0; i < n; ++i) {
    if (input[i] > 9)
      return result;
    source[i] = input[i];
  }
  if (c->version == 3) {
    result.code = AE_FULL;
    return result;
  }
  if (observe)
    observe(context, c);
  if (fail == 1) {
    result.code = AE_DEVICE;
    return result;
  }
  AeConfig next = {{0}, 0, c->version + 1};
  result.code = AE_OK;
  if (guarantee == AE_STRONG && n > cap)
    result.code = AE_FULL;
  else if (fail == 2)
    result.code = AE_EXHAUSTED;
  else {
    for (unsigned i = 0; i < n; ++i) {
      if (i == cap) {
        result.code = AE_FULL;
        break;
      }
      if (fail == i + 3) {
        result.code = AE_DEVICE;
        break;
      }
      next.values[next.count++] = source[i];
#ifdef AE_MUTANT_MIXED
      c->values[i] = source[i];
#endif
      if (observe)
        observe(context, c);
    }
    if (result.code == AE_OK && fail == n + 3)
      result.code = AE_DEVICE;
  }
  if (result.code != AE_OK && guarantee == AE_STRONG)
    return result;
  if (result.code != AE_OK && guarantee == AE_BASIC)
    next = (AeConfig){{0}, 0, next.version};
  *c = next;
  result.committed = 1;
  result.accepted = next.count;
  if (observe)
    observe(context, c);
  if (result.code == AE_OK && notify)
    result.code = AE_AMBIGUOUS;
  return result;
}
AeCode ae_journal(AeJournal *j, unsigned id, unsigned delta, unsigned crash) {
  if (!j || id < 1 || id > 7 || delta < 1 || delta > 7 || crash > 3 ||
      j->count > 2 || j->next > 1)
    return AE_INVALID;
  if (j->pending_id)
    return j->pending_id == id && j->pending_delta == delta ? AE_AMBIGUOUS
                                                            : AE_CONFLICT;
  for (unsigned i = 0; i < j->count; ++i)
    if (j->done[i].id == id)
      return j->done[i].delta == delta ? AE_OK : AE_CONFLICT;
  if (j->effects >= 31 || j->value > 63 - delta)
    return AE_FULL;
  j->pending_id = id;
  j->pending_delta = delta;
  if (crash == 1)
    return AE_AMBIGUOUS;
  ++j->effects;
  j->value += delta;
  if (crash == 2) {
#ifdef AE_MUTANT_REPLAY
    j->pending_id = 0;
    j->pending_delta = 0;
#endif
    return AE_AMBIGUOUS;
  }
  j->done[j->next] = (AeDone){id, delta};
  j->next = (j->next + 1) % 2;
  if (j->count < 2)
    ++j->count;
  j->pending_id = 0;
  j->pending_delta = 0;
  return crash == 3 ? AE_AMBIGUOUS : AE_OK;
}

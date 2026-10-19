#include "identity.h"
int ai_pair_cas(_Atomic(AIPair) *p, AIPair *expected, AIPair desired) {
  return atomic_compare_exchange_strong(p, expected, desired);
}
int ai_pair_lock_free(_Atomic(AIPair) *p) { return atomic_is_lock_free(p); }

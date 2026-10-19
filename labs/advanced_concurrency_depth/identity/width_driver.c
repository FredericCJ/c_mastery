#include "identity.h"
#include <stddef.h>
#include <stdio.h>
int main(void) {
  _Static_assert(sizeof(AIPair) == sizeof(void *) + sizeof(uintptr_t),
                 "selected ABI requires no pair padding");
  _Static_assert(offsetof(AIPair, generation) == sizeof(void *),
                 "selected pair layout");
  int a = 1, b = 2;
  AIPair original = {&a, 1}, expected = original, desired = {&b, 2};
  _Atomic(AIPair) pair;
  atomic_init(&pair, original);
  if (!ai_pair_cas(&pair, &expected, desired))
    return 41;
  expected = original;
  if (ai_pair_cas(&pair, &expected, original) || expected.pointer != &b ||
      expected.generation != 2)
    return 41;
  _Atomic(uint32_t) word;
  atomic_init(&word, 257);
  uint32_t old = 257;
  if (!ai_word_cas(&word, &old, 513) || atomic_load(&word) != 513)
    return 41;
  printf("OBS width pointer=%zu pair=%zu pair_align=%zu pair_lockfree=%d "
         "compact=%zu compact_lockfree=%d\n",
         sizeof(void *), sizeof(AIPair), _Alignof(_Atomic(AIPair)),
         ai_pair_lock_free(&pair), sizeof(uint32_t),
         atomic_is_lock_free(&word));
  return 0;
}

#ifndef AI_IDENTITY_H
#define AI_IDENTITY_H
#include <stdatomic.h>
#include <stdint.h>
enum { AI_OK, AI_FULL, AI_INVALID, AI_UNIMPLEMENTED };
struct ai_pool {
  uint32_t generation[16], maximum;
  unsigned used[16], retired[16];
};
int ai_init(struct ai_pool *, unsigned generation_bits);
int ai_take(struct ai_pool *, uint32_t *out);
int ai_valid(const struct ai_pool *, uint32_t handle);
int ai_release(struct ai_pool *, uint32_t handle);
int ai_word_cas(_Atomic(uint32_t) *, uint32_t *expected, uint32_t desired);
struct ai_pair {
  void *pointer;
  uintptr_t generation;
};
int ai_pair_cas(_Atomic(struct ai_pair) *, struct ai_pair *expected, struct ai_pair desired);
int ai_pair_lock_free(_Atomic(struct ai_pair) *);
#endif

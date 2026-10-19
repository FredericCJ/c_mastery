#ifndef AI_IDENTITY_H
#define AI_IDENTITY_H
#include <stdatomic.h>
#include <stdint.h>
enum { AI_OK, AI_FULL, AI_INVALID, AI_UNIMPLEMENTED };
typedef struct {
  uint32_t generation[16], maximum;
  unsigned used[16], retired[16];
} AIPool;
int ai_init(AIPool *, unsigned generation_bits);
int ai_take(AIPool *, uint32_t *out);
int ai_valid(const AIPool *, uint32_t handle);
int ai_release(AIPool *, uint32_t handle);
int ai_word_cas(_Atomic(uint32_t) *, uint32_t *expected, uint32_t desired);
typedef struct {
  void *pointer;
  uintptr_t generation;
} AIPair;
int ai_pair_cas(_Atomic(AIPair) *, AIPair *expected, AIPair desired);
int ai_pair_lock_free(_Atomic(AIPair) *);
#endif

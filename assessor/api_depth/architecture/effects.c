#include "effects.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_decide(ArEffects *s, int32_t input, ArEffect *out) {
  if (!s || !out)
    return AR_INVALID;
  if (s->pending || s->next == UINT32_MAX)
    return AR_STATE;
  int32_t doubled;
  int result = ar_double(&input, 1, &doubled);
  if (result != AR_OK)
    return result;
  ArEffect effect = {s->next + 1, doubled};
  s->next = effect.id;
  s->effect = effect;
  s->pending = 1;
  *out = effect;
  return AR_OK;
}
int ar_retry(const ArEffects *s, ArEffect *out) {
  if (!s || !out)
    return AR_INVALID;
  if (!s->pending)
    return AR_STATE;
  *out = s->effect;
  return AR_OK;
}
int ar_feedback(ArEffects *s, unsigned result) {
  if (!s || !s->pending)
    return AR_STATE;
  if (result == AR_OK) {
    s->pending = 0;
    return AR_OK;
  }
  if (result != AR_PENDING && result != AR_FAILED)
    return AR_INVALID;
  if (AR_MUTANT == 12)
    ++s->effect.id;
  return AR_PENDING;
}

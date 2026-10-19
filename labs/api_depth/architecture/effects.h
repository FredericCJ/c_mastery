#ifndef CMASTERY_ARCH_EFFECTS_H
#define CMASTERY_ARCH_EFFECTS_H
#include "arch.h"
typedef struct {
  uint32_t id;
  int32_t value;
} ArEffect;
typedef struct {
  uint32_t next;
  unsigned pending;
  ArEffect effect;
} ArEffects;
int ar_decide(ArEffects *, int32_t, ArEffect *);
int ar_retry(const ArEffects *, ArEffect *);
int ar_feedback(ArEffects *, unsigned);
#endif

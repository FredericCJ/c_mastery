#ifndef CMASTERY_ARCH_EFFECTS_H
#define CMASTERY_ARCH_EFFECTS_H
#include "arch.h"
struct ar_effect {
  uint32_t id;
  int32_t value;
};
struct ar_effects {
  uint32_t next;
  unsigned pending;
  struct ar_effect effect;
};
int ar_decide(struct ar_effects *, int32_t, struct ar_effect *);
int ar_retry(const struct ar_effects *, struct ar_effect *);
int ar_feedback(struct ar_effects *, unsigned);
#endif

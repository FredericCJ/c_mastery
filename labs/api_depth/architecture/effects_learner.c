#include "effects.h"
int ar_decide(struct ar_effects *s, int32_t v, struct ar_effect *o) {
  (void)s;
  (void)v;
  (void)o;
  return AR_UNIMPLEMENTED;
}
int ar_retry(const struct ar_effects *s, struct ar_effect *o) {
  (void)s;
  (void)o;
  return AR_UNIMPLEMENTED;
}
int ar_feedback(struct ar_effects *s, unsigned r) {
  (void)s;
  (void)r;
  return AR_UNIMPLEMENTED;
}

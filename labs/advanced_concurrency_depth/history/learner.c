#include "history.h"
int ah_history(const AHOperation *ops, size_t n, unsigned capacity,
               unsigned spec, unsigned pending_policy, AHWitness *out) {
  (void)ops;
  (void)n;
  (void)capacity;
  (void)spec;
  (void)pending_policy;
  (void)out;
  return AH_UNIMPLEMENTED;
}
int ah_retry(unsigned attempts, unsigned budget, unsigned changed) {
  (void)attempts;
  (void)budget;
  (void)changed;
  return AH_UNIMPLEMENTED;
}

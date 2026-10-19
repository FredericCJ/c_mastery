#include "progress.h"
int tp_donate(const unsigned *a, const int *b, size_t n, unsigned *out) {
  (void)a;
  (void)b;
  (void)n;
  (void)out;
  return TP_INVALID;
}
int tp_bound(const uint64_t *a, size_t n, unsigned known, uint64_t deadline,
             uint64_t *out) {
  (void)a;
  (void)n;
  (void)known;
  (void)deadline;
  (void)out;
  return TP_INVALID;
}
int tp_nonsleepable(unsigned effects) {
  (void)effects;
  return TP_INVALID;
}
int tp_history(const TPOp *ops, size_t n, unsigned policy, TPWitness *out) {
  (void)ops;
  (void)n;
  (void)policy;
  (void)out;
  return TP_INVALID;
}

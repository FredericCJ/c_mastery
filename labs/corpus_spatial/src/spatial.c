#include "spatial.h"
/* Intentionally unfinished, compiling learner implementation. */
int sp_build(SpIndex *o, SpIndex *s, unsigned k, unsigned d, const SpItem *p,
             unsigned n, unsigned b, unsigned w) {
  (void)o;
  (void)s;
  (void)k;
  (void)d;
  (void)p;
  (void)n;
  (void)b;
  (void)w;
  return SP_INVALID;
}
int sp_edit(SpIndex *i, SpIndex *s, unsigned a, SpItem p) {
  (void)i;
  (void)s;
  (void)a;
  (void)p;
  return SP_INVALID;
}
int sp_refit(SpIndex *i, SpIndex *s, SpItem p) {
  (void)i;
  (void)s;
  (void)p;
  return SP_INVALID;
}
int sp_query(const SpIndex *i, SpBox q, SpResult *o) {
  (void)i;
  (void)q;
  (void)o;
  return SP_INVALID;
}
int sp_nearest(const SpIndex *i, const int32_t p[3], const unsigned w[3],
               SpNearest *o) {
  (void)i;
  (void)p;
  (void)w;
  (void)o;
  return SP_INVALID;
}
int sp_pairs(const SpIndex *i, uint32_t r[32], unsigned *n) {
  (void)i;
  (void)r;
  (void)n;
  return SP_INVALID;
}

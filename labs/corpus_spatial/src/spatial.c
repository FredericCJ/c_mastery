#include "spatial.h"
/* Intentionally unfinished, compiling learner implementation. */
int sp_build(struct sp_index *o, struct sp_index *s, unsigned k, unsigned d, const struct sp_item *p,
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
int sp_edit(struct sp_index *i, struct sp_index *s, unsigned a, struct sp_item p) {
  (void)i;
  (void)s;
  (void)a;
  (void)p;
  return SP_INVALID;
}
int sp_refit(struct sp_index *i, struct sp_index *s, struct sp_item p) {
  (void)i;
  (void)s;
  (void)p;
  return SP_INVALID;
}
int sp_query(const struct sp_index *i, struct sp_box q, struct sp_result *o) {
  (void)i;
  (void)q;
  (void)o;
  return SP_INVALID;
}
int sp_nearest(const struct sp_index *i, const int32_t p[3], const unsigned w[3],
               struct sp_nearest *o) {
  (void)i;
  (void)p;
  (void)w;
  (void)o;
  return SP_INVALID;
}
int sp_pairs(const struct sp_index *i, uint32_t r[32], unsigned *n) {
  (void)i;
  (void)r;
  (void)n;
  return SP_INVALID;
}

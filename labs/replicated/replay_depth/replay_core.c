#include "replay_depth.h"
int rpr_init(uint64_t a, unsigned b, const RPState c[2], RprSession *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return RPR_UNIMPLEMENTED;
}
int rpr_advance(RprSession *a, int b, RprNotice *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_correct(RprSession *a, uint64_t b, unsigned c, int d, RprNotice *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPR_UNIMPLEMENTED;
}
int rpr_prepare(RprSession *a, RprEffect *b) {
  (void)a;
  (void)b;
  return RPR_UNIMPLEMENTED;
}
int rpr_feedback(RprSession *a, RprEffect b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_value(const RprSession *a, unsigned b, int32_t *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_invariant(const RprSession *a) {
  (void)a;
  return 0;
}

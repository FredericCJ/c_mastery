#include "replay_depth.h"
int rpr_init(uint64_t a, unsigned b, const struct rp_state c[2], struct rpr_session *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return RPR_UNIMPLEMENTED;
}
int rpr_advance(struct rpr_session *a, int b, struct rpr_notice *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_correct(struct rpr_session *a, uint64_t b, unsigned c, int d, struct rpr_notice *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPR_UNIMPLEMENTED;
}
int rpr_prepare(struct rpr_session *a, struct rpr_effect *b) {
  (void)a;
  (void)b;
  return RPR_UNIMPLEMENTED;
}
int rpr_feedback(struct rpr_session *a, struct rpr_effect b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_value(const struct rpr_session *a, unsigned b, int32_t *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_invariant(const struct rpr_session *a) {
  (void)a;
  return 0;
}

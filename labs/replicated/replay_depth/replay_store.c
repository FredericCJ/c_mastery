#include "replay_depth.h"
int rpr_create(store *a, uint64_t b, unsigned c, const struct rp_state d[2],
               struct rpr_durable *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPR_UNIMPLEMENTED;
}
int rpr_open(store *a, uint64_t b, struct rpr_durable *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_advance_durable(struct rpr_durable *a, int b, struct rpr_notice *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_correct_durable(struct rpr_durable *a, uint64_t b, unsigned c, int d,
                        struct rpr_notice *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPR_UNIMPLEMENTED;
}
int rpr_prepare_durable(struct rpr_durable *a, struct rpr_effect *b) {
  (void)a;
  (void)b;
  return RPR_UNIMPLEMENTED;
}
int rpr_feedback_durable(struct rpr_durable *a, struct rpr_effect b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}

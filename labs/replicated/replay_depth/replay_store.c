#include "replay_depth.h"
int rpr_create(store *a, uint64_t b, unsigned c, const RPState d[2],
               RprDurable *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPR_UNIMPLEMENTED;
}
int rpr_open(store *a, uint64_t b, RprDurable *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_advance_durable(RprDurable *a, int b, RprNotice *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}
int rpr_correct_durable(RprDurable *a, uint64_t b, unsigned c, int d,
                        RprNotice *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPR_UNIMPLEMENTED;
}
int rpr_prepare_durable(RprDurable *a, RprEffect *b) {
  (void)a;
  (void)b;
  return RPR_UNIMPLEMENTED;
}
int rpr_feedback_durable(RprDurable *a, RprEffect b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return RPR_UNIMPLEMENTED;
}

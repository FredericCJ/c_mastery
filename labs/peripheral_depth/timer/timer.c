#include "timer.h"
/* Intentionally unfinished learner scaffold. */
int pt_choose(const struct pu_clock *clock, unsigned psc, unsigned hz, unsigned d0,
              unsigned d1, unsigned center, unsigned ppm, struct pt_config *out) {
  (void)clock;
  (void)psc;
  (void)hz;
  (void)d0;
  (void)d1;
  (void)center;
  (void)ppm;
  (void)out;
  return PT_UNIMPLEMENTED;
}
int pt_ratios(const struct pt_config *c, struct pt_ratio *f, struct pt_ratio *tick, struct pt_ratio d[2]) {
  (void)c;
  (void)f;
  (void)tick;
  (void)d;
  return PT_UNIMPLEMENTED;
}
int pt_init(struct pt_model *m, const struct pt_config *c) {
  (void)m;
  (void)c;
  return PT_UNIMPLEMENTED;
}
int pt_stage(struct pt_model *m, const struct pt_config *c) {
  (void)m;
  (void)c;
  return PT_UNIMPLEMENTED;
}
int pt_ug(struct pt_model *m) {
  (void)m;
  return PT_UNIMPLEMENTED;
}
int pt_tick(struct pt_model *m) {
  (void)m;
  return PT_UNIMPLEMENTED;
}
int pt_input(struct pt_model *m) {
  (void)m;
  return PT_UNIMPLEMENTED;
}
int pt_level(const struct pt_model *m, unsigned channel, unsigned *out) {
  (void)m;
  (void)channel;
  (void)out;
  return PT_UNIMPLEMENTED;
}
int pt_interval(unsigned previous, unsigned current, unsigned max_gap,
                unsigned overcapture, unsigned *ticks) {
  (void)previous;
  (void)current;
  (void)max_gap;
  (void)overcapture;
  (void)ticks;
  return PT_UNIMPLEMENTED;
}

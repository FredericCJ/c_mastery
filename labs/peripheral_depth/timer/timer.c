#include "timer.h"
/* Intentionally unfinished learner scaffold. */
int pt_choose(const PuClock *clock, unsigned psc, unsigned hz, unsigned d0,
              unsigned d1, unsigned center, unsigned ppm, PtConfig *out) {
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
int pt_ratios(const PtConfig *c, PtRatio *f, PtRatio *tick, PtRatio d[2]) {
  (void)c;
  (void)f;
  (void)tick;
  (void)d;
  return PT_UNIMPLEMENTED;
}
int pt_init(PtModel *m, const PtConfig *c) {
  (void)m;
  (void)c;
  return PT_UNIMPLEMENTED;
}
int pt_stage(PtModel *m, const PtConfig *c) {
  (void)m;
  (void)c;
  return PT_UNIMPLEMENTED;
}
int pt_ug(PtModel *m) {
  (void)m;
  return PT_UNIMPLEMENTED;
}
int pt_tick(PtModel *m) {
  (void)m;
  return PT_UNIMPLEMENTED;
}
int pt_input(PtModel *m) {
  (void)m;
  return PT_UNIMPLEMENTED;
}
int pt_level(const PtModel *m, unsigned channel, unsigned *out) {
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

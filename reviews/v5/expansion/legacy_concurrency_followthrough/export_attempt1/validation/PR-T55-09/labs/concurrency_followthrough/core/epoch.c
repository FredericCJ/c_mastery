#include "epoch.h"
void cf_epoch_init(CfEpoch *e, unsigned n) {
  (void)e;
  (void)n;
}
int cf_epoch_register(CfEpoch *e, AGLease *l) {
  (void)e;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int cf_epoch_enter(CfEpoch *e, AGLease l) {
  (void)e;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int cf_epoch_leave(CfEpoch *e, AGLease l) {
  (void)e;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int cf_epoch_depart(CfEpoch *e, AGLease l) {
  (void)e;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int cf_epoch_advance(CfEpoch *e) {
  (void)e;
  return AG_UNIMPLEMENTED;
}
int cf_epoch_eligible(CfEpoch *e, unsigned r, unsigned *o) {
  (void)e;
  (void)r;
  (void)o;
  return AG_UNIMPLEMENTED;
}

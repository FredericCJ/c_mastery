#include "bridge.h"
/* Required learner implementation: these bodies deliberately reject the
 * contract. */
void mb_region_init(MBRegion *arg0) { (void)arg0; }
int mb_region_alloc(MBRegion *arg0, size_t arg1, size_t arg2, MBSpan *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return MB_INVALID;
}
int mb_region_cleanup(MBRegion *arg0, MBCleanup arg1, void *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return MB_INVALID;
}
int mb_region_reset(MBRegion *arg0) {
  (void)arg0;
  return MB_INVALID;
}
int mb_region_read(const MBRegion *arg0, MBSpan arg1, size_t arg2,
                   unsigned char *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return MB_INVALID;
}
void mb_once_init(MBOnce *arg0, bool arg1) {
  (void)arg0;
  (void)arg1;
}
int mb_once_enter(MBOnce *arg0, unsigned arg1, unsigned *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return MB_INVALID;
}
int mb_once_finish(MBOnce *arg0, unsigned arg1, bool arg2, unsigned arg3,
                   unsigned arg4) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  return MB_INVALID;
}
void mb_hp_init(MBHazards *arg0) { (void)arg0; }
int mb_hp_publish(MBHazards *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
unsigned mb_hp_observe(const MBHazards *arg0) {
  (void)arg0;
  return 0;
}
int mb_hp_protect(MBHazards *arg0, unsigned arg1, unsigned arg2,
                  unsigned *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return MB_INVALID;
}
void mb_hp_clear(MBHazards *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
}
int mb_hp_retire(MBHazards *arg0) {
  (void)arg0;
  return MB_INVALID;
}
unsigned mb_hp_collect(MBHazards *arg0) {
  (void)arg0;
  return 0;
}
void mb_epoch_init(MBEpoch *arg0) { (void)arg0; }
int mb_epoch_enter(MBEpoch *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
int mb_epoch_leave(MBEpoch *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
int mb_epoch_retire(MBEpoch *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
int mb_epoch_advance(MBEpoch *arg0) {
  (void)arg0;
  return MB_INVALID;
}
unsigned mb_epoch_collect(MBEpoch *arg0) {
  (void)arg0;
  return 0;
}

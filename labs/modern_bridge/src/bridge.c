#include "bridge.h"
/* Required learner implementation: these bodies deliberately reject the
 * contract. */
void mb_region_init(struct mb_region *arg0) { (void)arg0; }
int mb_region_alloc(struct mb_region *arg0, size_t arg1, size_t arg2, struct mb_span *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return MB_INVALID;
}
int mb_region_cleanup(struct mb_region *arg0, mb_cleanup_fn arg1, void *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return MB_INVALID;
}
int mb_region_reset(struct mb_region *arg0) {
  (void)arg0;
  return MB_INVALID;
}
int mb_region_read(const struct mb_region *arg0, struct mb_span arg1, size_t arg2,
                   unsigned char *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return MB_INVALID;
}
void mb_once_init(struct mb_once *arg0, bool arg1) {
  (void)arg0;
  (void)arg1;
}
int mb_once_enter(struct mb_once *arg0, unsigned arg1, unsigned *arg2) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  return MB_INVALID;
}
int mb_once_finish(struct mb_once *arg0, unsigned arg1, bool arg2, unsigned arg3,
                   unsigned arg4) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  (void)arg4;
  return MB_INVALID;
}
void mb_hp_init(struct mb_hazards *arg0) { (void)arg0; }
int mb_hp_publish(struct mb_hazards *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
unsigned mb_hp_observe(const struct mb_hazards *arg0) {
  (void)arg0;
  return 0;
}
int mb_hp_protect(struct mb_hazards *arg0, unsigned arg1, unsigned arg2,
                  unsigned *arg3) {
  (void)arg0;
  (void)arg1;
  (void)arg2;
  (void)arg3;
  return MB_INVALID;
}
void mb_hp_clear(struct mb_hazards *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
}
int mb_hp_retire(struct mb_hazards *arg0) {
  (void)arg0;
  return MB_INVALID;
}
unsigned mb_hp_collect(struct mb_hazards *arg0) {
  (void)arg0;
  return 0;
}
void mb_epoch_init(struct mb_epoch *arg0) { (void)arg0; }
int mb_epoch_enter(struct mb_epoch *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
int mb_epoch_leave(struct mb_epoch *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
int mb_epoch_retire(struct mb_epoch *arg0, unsigned arg1) {
  (void)arg0;
  (void)arg1;
  return MB_INVALID;
}
int mb_epoch_advance(struct mb_epoch *arg0) {
  (void)arg0;
  return MB_INVALID;
}
unsigned mb_epoch_collect(struct mb_epoch *arg0) {
  (void)arg0;
  return 0;
}

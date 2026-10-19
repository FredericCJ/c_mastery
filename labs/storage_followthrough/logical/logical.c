#include "logical.h"
int sl_format(store_io i) {
  (void)i;
  return SL_UNIMPLEMENTED;
}
int sl_open(SlCounter *s, store_io i) {
  (void)s;
  (void)i;
  return SL_UNIMPLEMENTED;
}
int sl_apply(SlCounter *s, uint64_t id, int64_t d, SlReceipt *r) {
  (void)s;
  (void)id;
  (void)d;
  (void)r;
  return SL_UNIMPLEMENTED;
}
int sl_get(const SlCounter *s, SlReceipt *r) {
  (void)s;
  (void)r;
  return SL_UNIMPLEMENTED;
}
int sl_checkpoint(SlCounter *s) {
  (void)s;
  return SL_UNIMPLEMENTED;
}

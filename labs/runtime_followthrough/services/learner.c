#include "services.h"
int vh_host(void *ctx, uint8_t service, int32_t input, int32_t *out) {
  (void)ctx;
  (void)service;
  (void)input;
  (void)out;
  return STORE_UNIMPLEMENTED;
}
int vh_migrate(struct vg_vh *h, int32_t *out) {
  (void)h;
  (void)out;
  return STORE_UNIMPLEMENTED;
}
int vh_increment(struct vg_vh *h, uint64_t id, int32_t delta, int32_t *out) {
  (void)h;
  (void)id;
  (void)delta;
  (void)out;
  return STORE_UNIMPLEMENTED;
}

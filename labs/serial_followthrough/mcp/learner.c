#include "mcp.h"
#define UNUSED(x) (void)(x)
uint16_t mc_crc(const uint8_t *a, size_t b) {
  UNUSED(a);
  UNUSED(b);
  return 0;
}
int mc_layout(unsigned a, unsigned b, struct mc_layout *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_init(struct mc_driver *a, struct mc_bus b, const struct mc_config *c, unsigned d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_send(struct mc_driver *a, struct mc_bus b, uint32_t c, const struct mc_frame *d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_poll(struct mc_driver *a, struct mc_bus b) {
  UNUSED(a);
  UNUSED(b);
  return MC_UNIMPLEMENTED;
}
int mc_cancel(struct mc_driver *a, struct mc_bus b) {
  UNUSED(a);
  UNUSED(b);
  return MC_UNIMPLEMENTED;
}
int mc_take(struct mc_driver *a, struct mc_bus b, uint32_t c, unsigned d, unsigned *e) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  UNUSED(e);
  return MC_UNIMPLEMENTED;
}
int mc_receive(struct mc_driver *a, struct mc_bus b, struct mc_frame *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_interrupt(struct mc_driver *a, struct mc_bus b, uint32_t *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_read(struct mc_bus a, uint16_t b, uint8_t *c, unsigned d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_write(struct mc_bus a, uint16_t b, const uint8_t *c, unsigned d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_bench_frame(unsigned a, unsigned b, struct mc_frame *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_bench_match(unsigned a, unsigned b, const struct mc_frame *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}

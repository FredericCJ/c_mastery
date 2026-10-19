#include "mcp.h"
#define UNUSED(x) (void)(x)
uint16_t mc_crc(const uint8_t *a, size_t b) {
  UNUSED(a);
  UNUSED(b);
  return 0;
}
int mc_layout(unsigned a, unsigned b, McLayout *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_init(McDriver *a, McBus b, const McConfig *c, unsigned d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_send(McDriver *a, McBus b, uint32_t c, const McFrame *d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_poll(McDriver *a, McBus b) {
  UNUSED(a);
  UNUSED(b);
  return MC_UNIMPLEMENTED;
}
int mc_cancel(McDriver *a, McBus b) {
  UNUSED(a);
  UNUSED(b);
  return MC_UNIMPLEMENTED;
}
int mc_take(McDriver *a, McBus b, uint32_t c, unsigned d, unsigned *e) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  UNUSED(e);
  return MC_UNIMPLEMENTED;
}
int mc_receive(McDriver *a, McBus b, McFrame *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_interrupt(McDriver *a, McBus b, uint32_t *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_read(McBus a, uint16_t b, uint8_t *c, unsigned d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_write(McBus a, uint16_t b, const uint8_t *c, unsigned d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return MC_UNIMPLEMENTED;
}
int mc_bench_frame(unsigned a, unsigned b, McFrame *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}
int mc_bench_match(unsigned a, unsigned b, const McFrame *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return MC_UNIMPLEMENTED;
}

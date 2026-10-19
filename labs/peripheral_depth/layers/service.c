#include "service.h"
int pl_encode(uint8_t a, uint8_t b, uint8_t c[8]) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}
int pl_send(PlService *a, PlTransport b, uint8_t c, uint8_t d, uint32_t e,
            uint32_t f, uint32_t g) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  return PL_UNIMPLEMENTED;
}
int pl_poll(PlService *a, PlTransport b, uint32_t c, uint32_t d, PlResult *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PL_UNIMPLEMENTED;
}
int pl_cancel(PlService *a, PlTransport b, uint32_t c) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}
int pl_feed(PlParser *a, uint8_t b, PlCommand *c) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}

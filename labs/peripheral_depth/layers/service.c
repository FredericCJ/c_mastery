#include "service.h"
int pl_encode(uint8_t a, uint8_t b, uint8_t c[8]) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}
int pl_send(struct pl_service *a, struct pl_transport b, uint8_t c, uint8_t d, uint32_t e,
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
int pl_poll(struct pl_service *a, struct pl_transport b, uint32_t c, uint32_t d, struct pl_result *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PL_UNIMPLEMENTED;
}
int pl_cancel(struct pl_service *a, struct pl_transport b, uint32_t c) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}
int pl_feed(struct pl_parser *a, uint8_t b, struct pl_command *c) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}

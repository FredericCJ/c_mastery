#include "clock.h"
int xk_snapshot(XkIo a, XkPolicy b, XkSnapshot *c) {
  (void)a;
  (void)b;
  (void)c;
  return XK_UNIMPLEMENTED;
}
int xk_sequence(XkSequence *a, uint32_t b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return XK_UNIMPLEMENTED;
}
int xk_calibrate(const XkCalibration *a, uint64_t b, uint64_t c,
                 XkInterval *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return XK_UNIMPLEMENTED;
}
int xk_event(XkInterval a, uint64_t b, uint64_t c, XkInterval *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return XK_UNIMPLEMENTED;
}
int xk_order(XkInterval a, XkInterval b, unsigned *c) {
  (void)a;
  (void)b;
  (void)c;
  return XK_UNIMPLEMENTED;
}

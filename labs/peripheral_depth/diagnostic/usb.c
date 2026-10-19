#include "usb.h"
/* Delayed-transfer learner scaffold, intentionally incomplete. */
int dg_usb_setup(DGUsb *u, const uint8_t q[8], DGControl *c) {
  (void)u;
  (void)q;
  (void)c;
  return DG_UNIMPLEMENTED;
}
int dg_usb_status(DGUsb *u, uint32_t t, unsigned a) {
  (void)u;
  (void)t;
  (void)a;
  return DG_UNIMPLEMENTED;
}
int dg_usb_reset(DGUsb *u, unsigned q) {
  (void)u;
  (void)q;
  return DG_UNIMPLEMENTED;
}
int dg_usb_out(DGUsb *u, unsigned t, const uint8_t *p, size_t n, unsigned c,
               DGSession *s, DGMessage *m, unsigned *h, int *a) {
  (void)u;
  (void)t;
  (void)p;
  (void)n;
  (void)c;
  (void)s;
  (void)m;
  (void)h;
  (void)a;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_queue(DGUsb *u, uint32_t t, const DGMessage *m) {
  (void)u;
  (void)t;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_packet(const DGUsb *u, uint32_t t, const uint8_t **p, size_t *n,
                     unsigned *v) {
  (void)u;
  (void)t;
  (void)p;
  (void)n;
  (void)v;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_ack(DGUsb *u, uint32_t t, unsigned a) {
  (void)u;
  (void)t;
  (void)a;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_cancel(DGUsb *u, uint32_t t, unsigned q) {
  (void)u;
  (void)t;
  (void)q;
  return DG_UNIMPLEMENTED;
}

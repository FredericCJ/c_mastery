#include "usb.h"
/* Delayed-transfer learner scaffold, intentionally incomplete. */
int dg_usb_setup(struct dg_usb *u, const uint8_t q[8], struct dg_control *c) {
  (void)u;
  (void)q;
  (void)c;
  return DG_UNIMPLEMENTED;
}
int dg_usb_status(struct dg_usb *u, uint32_t t, unsigned a) {
  (void)u;
  (void)t;
  (void)a;
  return DG_UNIMPLEMENTED;
}
int dg_usb_reset(struct dg_usb *u, unsigned q) {
  (void)u;
  (void)q;
  return DG_UNIMPLEMENTED;
}
int dg_usb_out(struct dg_usb *u, unsigned t, const uint8_t *p, size_t n, unsigned c,
               struct dg_session *s, struct dg_message *m, unsigned *h, int *a) {
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
int dg_usb_in_queue(struct dg_usb *u, uint32_t t, const struct dg_message *m) {
  (void)u;
  (void)t;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_packet(const struct dg_usb *u, uint32_t t, const uint8_t **p, size_t *n,
                     unsigned *v) {
  (void)u;
  (void)t;
  (void)p;
  (void)n;
  (void)v;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_ack(struct dg_usb *u, uint32_t t, unsigned a) {
  (void)u;
  (void)t;
  (void)a;
  return DG_UNIMPLEMENTED;
}
int dg_usb_in_cancel(struct dg_usb *u, uint32_t t, unsigned q) {
  (void)u;
  (void)t;
  (void)q;
  return DG_UNIMPLEMENTED;
}

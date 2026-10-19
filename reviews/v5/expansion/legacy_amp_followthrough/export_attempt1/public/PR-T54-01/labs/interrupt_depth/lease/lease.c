#include "lease.h"
int xd_recover(XdLease *s, const XdFence *f) {
  (void)s;
  (void)f;
  return XD_UNIMPLEMENTED;
}
int xd_submit(XdLease *s, uint32_t n, XdTicket *t) {
  (void)s;
  (void)n;
  (void)t;
  return XD_UNIMPLEMENTED;
}
int xd_cancel(XdLease *s, XdTicket t) {
  (void)s;
  (void)t;
  return XD_UNIMPLEMENTED;
}
int xd_observe(XdLease *s, XdTicket t, unsigned e, const XdProof *p) {
  (void)s;
  (void)t;
  (void)e;
  (void)p;
  return XD_UNIMPLEMENTED;
}
int xd_reclaim(XdLease *s, XdTicket t, XdResult *r) {
  (void)s;
  (void)t;
  (void)r;
  return XD_UNIMPLEMENTED;
}
void xd_quarantine(XdLease *s) { (void)s; }

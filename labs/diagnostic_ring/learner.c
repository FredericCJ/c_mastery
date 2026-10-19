#include "bridge.h"
int dr_open(DgRing *s, EnRing *r) {
  (void)s;
  (void)r;
  return DG_UNIMPLEMENTED;
}
int dr_send(DgRing *s, const DGMessage *m) {
  (void)s;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dr_poll(DgRing *s, uint32_t *r) {
  (void)s;
  (void)r;
  return DG_UNIMPLEMENTED;
}
int dr_receive(DgRing *s, DGSession *r, DGMessage *m) {
  (void)s;
  (void)r;
  (void)m;
  return DG_UNIMPLEMENTED;
}

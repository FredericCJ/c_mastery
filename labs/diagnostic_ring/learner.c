#include "bridge.h"
int dr_open(struct dg_ring *s, struct en_ring *r) {
  (void)s;
  (void)r;
  return DG_UNIMPLEMENTED;
}
int dr_send(struct dg_ring *s, const struct dg_message *m) {
  (void)s;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dr_poll(struct dg_ring *s, uint32_t *r) {
  (void)s;
  (void)r;
  return DG_UNIMPLEMENTED;
}
int dr_receive(struct dg_ring *s, struct dg_session *r, struct dg_message *m) {
  (void)s;
  (void)r;
  (void)m;
  return DG_UNIMPLEMENTED;
}

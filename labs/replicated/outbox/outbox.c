#include "outbox.h"
int rpo_sender(RplLog *e, RplLog *a, RpoSender *o) {
  (void)e;
  (void)a;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
int rpo_receiver(RplLog *i, RpoReceiver *o) {
  (void)i;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
int rpo_next(RpoSender *s, RplEvent *o) {
  (void)s;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
int rpo_receive(RpoReceiver *r, uint64_t u, uint32_t s, RplEvent e,
                RpoReply *o) {
  (void)r;
  (void)u;
  (void)s;
  (void)e;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
int rpo_ack(RpoSender *s, RpoReply r) {
  (void)s;
  (void)r;
  return RPL_UNIMPLEMENTED;
}
int rpo_projection(const RplLog *e, unsigned p, int64_t *o) {
  (void)e;
  (void)p;
  (void)o;
  return RPL_UNIMPLEMENTED;
}

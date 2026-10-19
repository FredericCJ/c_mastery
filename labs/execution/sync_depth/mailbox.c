#include "mailbox.h"
/* Intentional learner scaffold. Implement the transport/application boundary.
 */
int cdmb_init(CDMailbox *m, cmx_kernel *k, unsigned c, uint32_t s) {
  (void)m;
  (void)k;
  (void)c;
  (void)s;
  return CMX_UNIMPLEMENTED;
}
int cdmb_begin(CDMailbox *m, unsigned s, const unsigned char r[4],
               cmx_deadline d, CDMBIdentity *i) {
  (void)m;
  (void)s;
  (void)r;
  (void)d;
  (void)i;
  return CMX_UNIMPLEMENTED;
}
int cdmb_delivery(CDMailbox *m, unsigned s) {
  (void)m;
  (void)s;
  return CMX_UNIMPLEMENTED;
}
int cdmb_receive(CDMailbox *m, CDMBRequest *r, unsigned c, cmx_deadline d) {
  (void)m;
  (void)r;
  (void)c;
  (void)d;
  return CMX_UNIMPLEMENTED;
}
int cdmb_reply(CDMailbox *m, CDMBIdentity i, const unsigned char r[4]) {
  (void)m;
  (void)i;
  (void)r;
  return CMX_UNIMPLEMENTED;
}
int cdmb_poll(CDMailbox *m, unsigned s, unsigned char *r, unsigned c) {
  (void)m;
  (void)s;
  (void)r;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
int cdmb_cancel(CDMailbox *m, unsigned s) {
  (void)m;
  (void)s;
  return CMX_UNIMPLEMENTED;
}
int cdmb_restart(CDMailbox *m) {
  (void)m;
  return CMX_UNIMPLEMENTED;
}
int cdmb_effect(CDMailbox *m, CDMBIdentity i, uint32_t d, uint32_t *r) {
  (void)m;
  (void)i;
  (void)d;
  (void)r;
  return CMX_UNIMPLEMENTED;
}
int cdmb_acknowledge(CDMailbox *m, CDMBIdentity i) {
  (void)m;
  (void)i;
  return CMX_UNIMPLEMENTED;
}

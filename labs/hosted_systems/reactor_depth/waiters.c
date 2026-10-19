/* Intentional learner scaffold. */
#include "waiters.h"
#define U(x) ((void)(x))
int rd_port_init(RdPort *a, unsigned b, unsigned char c) {
  U(a);
  U(b);
  U(c);
  return -1;
}
int rd_port_wait(RdPort *a, unsigned b, RdWait *c, const struct timespec *d) {
  U(a);
  U(b);
  U(c);
  U(d);
  return -1;
}
int rd_port_enrolled(RdPort *a, unsigned b, const struct timespec *c) {
  U(a);
  U(b);
  U(c);
  return -1;
}
int rd_port_stop(RdPort *a) {
  U(a);
  return -1;
}
int rd_port_peer_gone(RdPort *a) {
  U(a);
  return -1;
}
int rd_port_notify(RdPort *a) {
  U(a);
  return -1;
}
int rd_port_destroy(RdPort *a) {
  U(a);
  return -1;
}

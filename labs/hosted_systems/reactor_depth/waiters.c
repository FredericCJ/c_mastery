/* Intentional learner scaffold. */
#include "waiters.h"
#define U(x) ((void)(x))
int rd_port_init(struct rd_port *a, unsigned b, unsigned char c) {
  U(a);
  U(b);
  U(c);
  return -1;
}
int rd_port_wait(struct rd_port *a, unsigned b, struct rd_wait *c, const struct timespec *d) {
  U(a);
  U(b);
  U(c);
  U(d);
  return -1;
}
int rd_port_enrolled(struct rd_port *a, unsigned b, const struct timespec *c) {
  U(a);
  U(b);
  U(c);
  return -1;
}
int rd_port_stop(struct rd_port *a) {
  U(a);
  return -1;
}
int rd_port_peer_gone(struct rd_port *a) {
  U(a);
  return -1;
}
int rd_port_notify(struct rd_port *a) {
  U(a);
  return -1;
}
int rd_port_destroy(struct rd_port *a) {
  U(a);
  return -1;
}

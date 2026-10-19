/* Learner scaffold: implement the contracts in reactor.h. Deliberately builds
 * but rejects work; it is not the assessor reference. */
#include "reactor.h"
#define UNUSED(x) ((void)(x))
int rd_operation_init(RdOperation *a, size_t b, const unsigned char *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return -1;
}
int rd_progress(RdOperation *a, int b, const unsigned char *c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return -1;
}
int rd_cancel(RdOperation *a) {
  UNUSED(a);
  return -1;
}
int rd_registry_init(RdRegistry *a, unsigned b, unsigned c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return -1;
}
int rd_register(RdRegistry *a, unsigned b, int c, RdToken *d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return -1;
}
RdSlot *rd_borrow(RdRegistry *a, RdToken b) {
  UNUSED(a);
  UNUSED(b);
  return 0;
}
int rd_return(RdRegistry *a, RdToken b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}
int rd_close(RdRegistry *a, RdToken b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}
int rd_submit(RdRegistry *a, RdToken b, unsigned c, RdCompletion *d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return -1;
}
int rd_complete(RdRegistry *a, RdCompletion b, int c, const unsigned char *d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return -1;
}
int rd_ready(RdScheduler *a, unsigned b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}
int rd_turn(RdScheduler *a, unsigned b, int (*c)(void *, unsigned), void *d,
            RdTurn *e) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  UNUSED(e);
  return -1;
}
int rd_service_init(RdService *a, size_t b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}
int rd_service_open(RdService *a, unsigned *b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}
int rd_admit(RdService *a, unsigned b, RfMessage *c, enum RdPolicy d) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  UNUSED(d);
  return -1;
}
int rd_resume(RdService *a, unsigned b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}
int rd_commit(RdService *a, unsigned b, size_t c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return -1;
}
int rd_service_borrow(RdService *a) {
  UNUSED(a);
  return -1;
}
int rd_service_return(RdService *a) {
  UNUSED(a);
  return -1;
}
int rd_stop(RdService *a, uint64_t b, uint64_t c) {
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return -1;
}
int rd_tick(RdService *a, uint64_t b) {
  UNUSED(a);
  UNUSED(b);
  return -1;
}

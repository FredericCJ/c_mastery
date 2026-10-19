/* Unsolved learner implementation: intentionally rejects semantic tests. */
#define _POSIX_C_SOURCE 200809L
#include "cl.h"
void cl_queue_init(CLQueue *q, CLMessage storage[2]) {
  (void)q;
  (void)storage;
}
enum cl_result cl_send(CLQueue *q, uint32_t p, uint32_t seq,
                       const unsigned char *in, size_t n) {
  (void)q;
  (void)p;
  (void)seq;
  (void)in;
  (void)n;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_recv(CLQueue *q, unsigned char *out, size_t cap, size_t *n,
                       uint32_t *p, uint32_t *seq) {
  (void)q;
  (void)out;
  (void)cap;
  (void)n;
  (void)p;
  (void)seq;
  return CL_UNIMPLEMENTED;
}
void cl_close(CLQueue *q) { (void)q; }
uint64_t cl_abort(CLQueue *q) {
  (void)q;
  return 0;
}
void cl_pool_init(CLPool *p) { (void)p; }
enum cl_result cl_alloc(CLPool *p, unsigned n, CLBuffer *out) {
  (void)p;
  (void)n;
  (void)out;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_buffer_write(CLPool *p, CLBuffer h, const unsigned char *in,
                               unsigned n) {
  (void)p;
  (void)h;
  (void)in;
  (void)n;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_buffer_read(const CLPool *p, CLBuffer h, unsigned char *out,
                              unsigned cap, unsigned *n) {
  (void)p;
  (void)h;
  (void)out;
  (void)cap;
  (void)n;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_release(CLPool *p, CLBuffer h) {
  (void)p;
  (void)h;
  return CL_UNIMPLEMENTED;
}
void cl_owned_init(CLOwnedQueue *q) { (void)q; }
enum cl_result cl_owned_send(CLPool *p, CLOwnedQueue *q, CLBuffer h) {
  (void)p;
  (void)q;
  (void)h;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_owned_recv(CLPool *p, CLOwnedQueue *q, CLBuffer *out) {
  (void)p;
  (void)q;
  (void)out;
  return CL_UNIMPLEMENTED;
}
void cl_owned_close(CLOwnedQueue *q) { (void)q; }
unsigned cl_owned_abort(CLPool *p, CLOwnedQueue *q) {
  (void)p;
  (void)q;
  return 0;
}
void cl_life_init(CLLife *l) { (void)l; }
enum cl_result cl_acquire(CLLife *l, CLLease *out) {
  (void)l;
  (void)out;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_transition(CLLife *l, CLLease h, enum cl_lease_state from,
                             enum cl_lease_state to) {
  (void)l;
  (void)h;
  (void)from;
  (void)to;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_drop(CLLife *l, CLLease h) {
  (void)l;
  (void)h;
  return CL_UNIMPLEMENTED;
}
void cl_life_close(CLLife *l) { (void)l; }
enum cl_result cl_life_drained(CLLife *l) {
  (void)l;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_destroy(CLLife *l) {
  (void)l;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_restart(CLLife *l) {
  (void)l;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_restart_group(CLLife *objects, unsigned count) {
  (void)objects;
  (void)count;
  return CL_UNIMPLEMENTED;
}
void cl_record_init(CLRecord *r, uint32_t v) {
  (void)r;
  (void)v;
}
enum cl_result cl_record_begin(CLRecord *r, uint32_t owner) {
  (void)r;
  (void)owner;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_record_stage(CLRecord *r, uint32_t owner, uint32_t v) {
  (void)r;
  (void)owner;
  (void)v;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_record_commit(CLRecord *r, uint32_t owner) {
  (void)r;
  (void)owner;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_record_dead(CLRecord *r, uint32_t dead) {
  (void)r;
  (void)dead;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_repair_begin(CLRecord *r, uint32_t owner) {
  (void)r;
  (void)owner;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_repair_finish(CLRecord *r, uint32_t owner, unsigned repair) {
  (void)r;
  (void)owner;
  (void)repair;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_record_read(const CLRecord *r, uint32_t *v) {
  (void)r;
  (void)v;
  return CL_UNIMPLEMENTED;
}
void cl_ack_init(CLAck *a, uint32_t epoch) {
  (void)a;
  (void)epoch;
}
enum cl_result cl_request(CLAck *a, uint32_t epoch, uint32_t id, uint32_t delta,
                          uint32_t *out) {
  (void)a;
  (void)epoch;
  (void)id;
  (void)delta;
  (void)out;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_apply(CLAck *a, uint32_t id) {
  (void)a;
  (void)id;
  return CL_UNIMPLEMENTED;
}
enum cl_result cl_acknowledge(CLAck *a, uint32_t id) {
  (void)a;
  (void)id;
  return CL_UNIMPLEMENTED;
}

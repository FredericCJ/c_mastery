#include "arch.h"
#include "board.h"
#include "borrow.h"
#include "linux.h"
int ar_supervise(struct ar_supervisor *s, unsigned event, int (*log)(void *, unsigned),
                 void *context) {
  (void)s;
  (void)event;
  (void)log;
  (void)context;
  return AR_UNIMPLEMENTED;
}
int ar_borrow(struct ar_borrow *b, const unsigned char *p, size_t n) {
  (void)b;
  (void)p;
  (void)n;
  return AR_UNIMPLEMENTED;
}
int ar_borrow_chunk(struct ar_borrow *b, size_t budget, unsigned char *out,
                    size_t *used) {
  (void)b;
  (void)budget;
  (void)out;
  (void)used;
  return AR_UNIMPLEMENTED;
}
int ar_borrow_cancel(struct ar_borrow *b) {
  (void)b;
  return AR_UNIMPLEMENTED;
}
int ar_borrow_ack(struct ar_borrow *b) {
  (void)b;
  return AR_UNIMPLEMENTED;
}
int ar_config(const struct ar_field *f, size_t n, struct ar_config *out) {
  (void)f;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_identity(struct ar_config c, unsigned char out[8]) {
  (void)c;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_double(const int32_t *in, size_t n, int32_t *out) {
  (void)in;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_window(struct ar_window *s, int32_t v, int32_t *out) {
  (void)s;
  (void)v;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_encode(unsigned v, struct ar_record r, unsigned char *p, size_t n) {
  (void)v;
  (void)r;
  (void)p;
  (void)n;
  return AR_UNIMPLEMENTED;
}
int ar_decode(unsigned v, const unsigned char *p, size_t n, struct ar_record *out) {
  (void)v;
  (void)p;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_feed(struct ar_reader *r, const unsigned char *p, size_t n, struct ar_record *out) {
  (void)r;
  (void)p;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_process(struct ar_storage s, size_t n, int32_t *out) {
  (void)s;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_service(int32_t milli, int32_t threshold, struct ar_indicator i) {
  (void)milli;
  (void)threshold;
  (void)i;
  return AR_UNIMPLEMENTED;
}
int ar_centimilli(int32_t centi, int32_t *out) {
  (void)centi;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_board_init(struct ar_board *b) {
  (void)b;
  return AR_UNIMPLEMENTED;
}
int ar_board_set(void *ctx, unsigned on) {
  (void)ctx;
  (void)on;
  return AR_UNIMPLEMENTED;
}
int ar_clear_pending(struct ar_board *b, uintptr_t address, uint32_t mask) {
  (void)b;
  (void)address;
  (void)mask;
  return AR_UNIMPLEMENTED;
}
struct ar_result ar_linux_read(int fd, unsigned char *p, size_t n) {
  (void)fd;
  (void)p;
  (void)n;
  return (struct ar_result){0, AR_UNIMPLEMENTED};
}
int ar_outbox(int fd, uint32_t id, int32_t value, unsigned lose_ack) {
  (void)fd;
  (void)id;
  (void)value;
  (void)lose_ack;
  return AR_UNIMPLEMENTED;
}
void ar_machine_init(struct ar_machine *m) { (void)m; }
int ar_acquire(struct ar_machine *m, uint32_t *id) {
  (void)m;
  (void)id;
  return AR_UNIMPLEMENTED;
}
int ar_submit(struct ar_machine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_machine_cancel(struct ar_machine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_event(struct ar_machine *m, struct ar_event e) {
  (void)m;
  (void)e;
  return AR_UNIMPLEMENTED;
}
int ar_step(struct ar_machine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_restart(struct ar_machine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_begin(struct ar_task *t, struct ar_ops o, const unsigned char *p, size_t n,
             unsigned version, unsigned partial) {
  (void)t;
  (void)o;
  (void)p;
  (void)n;
  (void)version;
  (void)partial;
  return AR_UNIMPLEMENTED;
}
int ar_poll(struct ar_task *t) {
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_cancel(struct ar_task *t) {
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_cancel_ack(struct ar_task *t) {
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_queue_init(struct ar_queue *q, size_t cap) {
  (void)q;
  (void)cap;
  return AR_UNIMPLEMENTED;
}
int ar_push(struct ar_queue *q, struct ar_item value, unsigned policy, struct ar_eviction *out) {
  (void)q;
  (void)value;
  (void)policy;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_pop(struct ar_queue *q, struct ar_item *out) {
  (void)q;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_recover(struct ar_supervisor *s, struct ar_recovery r) {
  (void)s;
  (void)r;
  return AR_UNIMPLEMENTED;
}

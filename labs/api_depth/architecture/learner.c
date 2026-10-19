#include "arch.h"
#include "board.h"
#include "borrow.h"
#include "linux.h"
int ar_supervise(ArSupervisor *s, unsigned event, int (*log)(void *, unsigned),
                 void *context) {
  (void)s;
  (void)event;
  (void)log;
  (void)context;
  return AR_UNIMPLEMENTED;
}
int ar_borrow(ArBorrow *b, const unsigned char *p, size_t n) {
  (void)b;
  (void)p;
  (void)n;
  return AR_UNIMPLEMENTED;
}
int ar_borrow_chunk(ArBorrow *b, size_t budget, unsigned char *out,
                    size_t *used) {
  (void)b;
  (void)budget;
  (void)out;
  (void)used;
  return AR_UNIMPLEMENTED;
}
int ar_borrow_cancel(ArBorrow *b) {
  (void)b;
  return AR_UNIMPLEMENTED;
}
int ar_borrow_ack(ArBorrow *b) {
  (void)b;
  return AR_UNIMPLEMENTED;
}
int ar_config(const ArField *f, size_t n, ArConfig *out) {
  (void)f;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_identity(ArConfig c, unsigned char out[8]) {
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
int ar_window(ArWindow *s, int32_t v, int32_t *out) {
  (void)s;
  (void)v;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_encode(unsigned v, ArRecord r, unsigned char *p, size_t n) {
  (void)v;
  (void)r;
  (void)p;
  (void)n;
  return AR_UNIMPLEMENTED;
}
int ar_decode(unsigned v, const unsigned char *p, size_t n, ArRecord *out) {
  (void)v;
  (void)p;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_feed(ArReader *r, const unsigned char *p, size_t n, ArRecord *out) {
  (void)r;
  (void)p;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_process(ArStorage s, size_t n, int32_t *out) {
  (void)s;
  (void)n;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_service(int32_t milli, int32_t threshold, ArIndicator i) {
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
int ar_board_init(ArBoard *b) {
  (void)b;
  return AR_UNIMPLEMENTED;
}
int ar_board_set(void *ctx, unsigned on) {
  (void)ctx;
  (void)on;
  return AR_UNIMPLEMENTED;
}
int ar_clear_pending(ArBoard *b, uintptr_t address, uint32_t mask) {
  (void)b;
  (void)address;
  (void)mask;
  return AR_UNIMPLEMENTED;
}
ArResult ar_linux_read(int fd, unsigned char *p, size_t n) {
  (void)fd;
  (void)p;
  (void)n;
  return (ArResult){0, AR_UNIMPLEMENTED};
}
int ar_outbox(int fd, uint32_t id, int32_t value, unsigned lose_ack) {
  (void)fd;
  (void)id;
  (void)value;
  (void)lose_ack;
  return AR_UNIMPLEMENTED;
}
void ar_machine_init(ArMachine *m) { (void)m; }
int ar_acquire(ArMachine *m, uint32_t *id) {
  (void)m;
  (void)id;
  return AR_UNIMPLEMENTED;
}
int ar_submit(ArMachine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_machine_cancel(ArMachine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_event(ArMachine *m, ArEvent e) {
  (void)m;
  (void)e;
  return AR_UNIMPLEMENTED;
}
int ar_step(ArMachine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_restart(ArMachine *m) {
  (void)m;
  return AR_UNIMPLEMENTED;
}
int ar_begin(ArTask *t, ArOps o, const unsigned char *p, size_t n,
             unsigned version, unsigned partial) {
  (void)t;
  (void)o;
  (void)p;
  (void)n;
  (void)version;
  (void)partial;
  return AR_UNIMPLEMENTED;
}
int ar_poll(ArTask *t) {
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_cancel(ArTask *t) {
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_cancel_ack(ArTask *t) {
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_queue_init(ArQueue *q, size_t cap) {
  (void)q;
  (void)cap;
  return AR_UNIMPLEMENTED;
}
int ar_push(ArQueue *q, ArItem value, unsigned policy, ArEviction *out) {
  (void)q;
  (void)value;
  (void)policy;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_pop(ArQueue *q, ArItem *out) {
  (void)q;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_recover(ArSupervisor *s, ArRecovery r) {
  (void)s;
  (void)r;
  return AR_UNIMPLEMENTED;
}

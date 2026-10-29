#include "stream.h"
int rf_send_once(struct rf_outbox *q, unsigned slot,
                 int (*writer)(void *, const unsigned char *, size_t),
                 void *ctx) {
  (void)q;
  (void)slot;
  (void)writer;
  (void)ctx;
  return -1;
}
int rf_datagram_accept(size_t n, size_t capacity, unsigned flags) {
  (void)n;
  (void)capacity;
  (void)flags;
  return -1;
}
/* Intentionally unfinished learner artifact. Reference lives under assessor/.
 */
int rf_parser_init(struct rf_parser *p, unsigned width) {
  (void)p;
  (void)width;
  return -1;
}
int rf_push(struct rf_parser *p, unsigned char b, struct rf_frame *out) {
  (void)p;
  (void)b;
  (void)out;
  return -1;
}
int rf_feed(struct rf_parser *p, const unsigned char *b, size_t n, size_t *used,
            struct rf_frame *out) {
  (void)p;
  (void)b;
  (void)n;
  (void)used;
  (void)out;
  return -1;
}
int rf_eof(struct rf_parser *p) {
  (void)p;
  return -1;
}
int rf_message_init(struct rf_message *m, unsigned id, unsigned width,
                    const struct rf_frame *f) {
  (void)m;
  (void)id;
  (void)width;
  (void)f;
  return -1;
}
int rf_outbox_init(struct rf_outbox *q, size_t n) {
  (void)q;
  (void)n;
  return -1;
}
int rf_open(struct rf_outbox *q, unsigned *s) {
  (void)q;
  (void)s;
  return -1;
}
int rf_enqueue(struct rf_outbox *q, unsigned s, struct rf_message *m) {
  (void)q;
  (void)s;
  (void)m;
  return -1;
}
int rf_peek(struct rf_outbox *q, unsigned s, const unsigned char **b, size_t *n) {
  (void)q;
  (void)s;
  (void)b;
  (void)n;
  return -1;
}
int rf_advance(struct rf_outbox *q, unsigned s, size_t n) {
  (void)q;
  (void)s;
  (void)n;
  return -1;
}
int rf_disconnect(struct rf_outbox *q, unsigned s) {
  (void)q;
  (void)s;
  return -1;
}
int rf_outbox_invariant(const struct rf_outbox *q) {
  (void)q;
  return 0;
}
int rf_dedup_init(struct rf_dedup *d, unsigned s) {
  (void)d;
  (void)s;
  return -1;
}
int rf_effect(struct rf_dedup *d, unsigned s, unsigned id, unsigned delta,
              uint32_t *r) {
  (void)d;
  (void)s;
  (void)id;
  (void)delta;
  (void)r;
  return -1;
}
int rf_sequence_init(struct rf_sequence *s, unsigned session, unsigned first) {
  (void)s;
  (void)session;
  (void)first;
  return -1;
}
int rf_arrive(struct rf_sequence *s, unsigned session, unsigned seq, unsigned out[3],
              unsigned *n) {
  (void)s;
  (void)session;
  (void)seq;
  (void)out;
  (void)n;
  return RF_SEQ_INVALID;
}
int rf_expire_gap(struct rf_sequence *s, unsigned *missing, unsigned out[3],
                  unsigned *n) {
  (void)s;
  (void)missing;
  (void)out;
  (void)n;
  return -1;
}

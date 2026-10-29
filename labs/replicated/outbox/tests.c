#include "test_support.h"
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
static struct rpo_fixture source, ack, inbox;
static struct rpo_sender sender;
static struct rpo_receiver receiver;
static int setup(void) {
  CHECK(!rpo_fixture(&source, 19, 7));
  CHECK(!rpo_fixture(&ack, 19, RPO_ACK_SCHEMA));
  CHECK(!rpo_fixture(&inbox, 19, 7));
  CHECK(rpo_sender(&source.log, &ack.log, &sender) == RPL_OK);
  CHECK(rpo_receiver(&inbox.log, &receiver) == RPL_OK);
  return 0;
}
static int receive_case(void) {
  CHECK(!setup());
  struct rpo_reply reply = {8, 8, 8};
  struct rpl_event e = {2, 1, 1, 7};
  CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPO_GAP);
  CHECK(inbox.log.memory.count == 0 && reply.total == 8);
  e.position = 1;
  CHECK(rpo_receive(&receiver, 20, 7, e, &reply) == RPL_IDENTITY);
  CHECK(reply.total == 8);
  CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPL_OK);
  CHECK(reply.position == 1 && reply.total == 7);
  CHECK(!rpo_reopen(&inbox));
  CHECK(rpo_receiver(&inbox.log, &receiver) == RPL_OK);
  CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPO_REPLAY);
  CHECK(inbox.log.memory.count == 1 && reply.total == 7);
  e.value = 9;
  reply.total = 100;
  CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPO_CONFLICT);
  CHECK(reply.total == 100 && inbox.log.memory.count == 1);
  e = (struct rpl_event){2, 1, 2, -10};
  CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPL_OK);
  CHECK(reply.total == -3);
  e = (struct rpl_event){1, 1, 1, 7};
  CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPO_REPLAY);
  CHECK(reply.total == 7 && inbox.log.memory.count == 2);
  return 0;
}
static int ack_case(void) {
  CHECK(!setup());
  struct rpl_event e = {8, 8, 8, 8};
  CHECK(rpo_next(&sender, &e) == RPO_EMPTY && e.position == 8);
  CHECK(rpl_append(&source.log, 1, 3, 7) == RPL_OK);
  CHECK(rpo_next(&sender, &e) == RPL_OK && e.position == 1);
  struct rpo_reply reply = {20, 1, 7};
  CHECK(rpo_ack(&sender, reply) == RPO_CONFLICT && ack.log.memory.count == 0);
  reply = (struct rpo_reply){19, 1, 8};
  CHECK(rpo_ack(&sender, reply) == RPO_CONFLICT && ack.log.memory.count == 0);
  reply = (struct rpo_reply){19, 1, 7};
  CHECK(rpo_ack(&sender, reply) == RPL_OK);
  CHECK(!rpo_reopen(&ack));
  CHECK(rpo_next(&sender, &e) == RPO_EMPTY);
  CHECK(rpl_append(&source.log, 1, 4, -2) == RPL_OK);
  CHECK(rpo_ack(&sender, reply) == RPO_CONFLICT && ack.log.memory.count == 1);
  CHECK(!rpo_fixture(&ack, 20, RPO_ACK_SCHEMA));
  struct rpo_sender saved = sender;
  CHECK(rpo_sender(&source.log, &ack.log, &sender) == RPL_IDENTITY);
  CHECK(sender.events == saved.events &&
        sender.acknowledgments == saved.acknowledgments);
  CHECK(!rpo_fixture(&ack, 19, RPO_ACK_SCHEMA));
  CHECK(rpl_append(&ack.log, 1, 0, 2) == RPL_OK);
  CHECK(rpo_sender(&source.log, &ack.log, &sender) == RPL_CORRUPT);
  return 0;
}
struct rp_wire {
  unsigned char bytes[100];
  size_t used, at, chunk;
  unsigned retry;
  int zero;
};
static int writer(void *c, const unsigned char *p, size_t n) {
  struct rp_wire *w = c;
  if (w->retry) {
    --w->retry;
    return 0;
  }
  if (w->zero)
    return 0;
  if (n > w->chunk)
    n = w->chunk;
  if (n > sizeof w->bytes - w->used)
    return -1;
  memcpy(w->bytes + w->used, p, n);
  w->used += n;
  return (int)n;
}
static ptrdiff_t reader(void *c, unsigned char *p, size_t n) {
  struct rp_wire *w = c;
  if (w->retry) {
    --w->retry;
    return -2;
  }
  if (w->zero)
    return -2;
  if (n > w->chunk)
    n = w->chunk;
  if (n > w->used - w->at)
    n = w->used - w->at;
  memcpy(p, w->bytes + w->at, n);
  w->at += n;
  return (ptrdiff_t)n;
}
static int transport_case(void) {
  unsigned char payload[64], out[64];
  for (unsigned i = 0; i < 64; ++i)
    payload[i] = (unsigned char)(i * 3);
  struct rp_wire w = {.chunk = 1, .retry = 2};
  CHECK(rpo_wire_write(writer, &w, 74, payload, 64) == RPL_OK && w.used == 72);
  for (unsigned i = 0; i < 8; ++i)
    CHECK(w.bytes[9 * i] == 8);
  struct rp_wire original = w;
  for (size_t n = 1; n <= 9; ++n) {
    w = original;
    w.chunk = n;
    w.retry = 2;
    CHECK(rpo_wire_read(reader, &w, 80, 64, out) == RPL_OK);
    CHECK(!memcmp(payload, out, 64));
  }
  for (size_t cut = 0; cut < 72; ++cut) {
    w = original;
    w.used = cut;
    w.chunk = 9;
    memset(out, 0xa5, 64);
    CHECK(rpo_wire_read(reader, &w, 80, 64, out) == RPL_CORRUPT);
    for (unsigned i = 0; i < 64; ++i)
      CHECK(out[i] == 0xa5);
  }
  /* Same total bytes, noncanonical segmentation: 7 + 1 + seven 8-byte frames.
   */
  w = (struct rp_wire){.chunk = 9};
  w.bytes[w.used++] = 7;
  for (unsigned i = 0; i < 7; ++i)
    w.bytes[w.used++] = payload[i];
  w.bytes[w.used++] = 1;
  w.bytes[w.used++] = payload[7];
  for (unsigned f = 1; f < 8; ++f) {
    w.bytes[w.used++] = 8;
    for (unsigned i = 0; i < 8; ++i)
      w.bytes[w.used++] = payload[f * 8 + i];
  }
  CHECK(rpo_wire_read(reader, &w, 80, 64, out) == RPL_CORRUPT);
  w = original;
  w.bytes[w.used++] = 0;
  w.chunk = 9;
  CHECK(rpo_wire_read(reader, &w, 80, 64, out) == RPL_CORRUPT);
  w = (struct rp_wire){.chunk = 1, .zero = 1};
  CHECK(rpo_wire_write(writer, &w, 3, payload, 64) == RPL_IO && w.used == 0);
  CHECK(rpo_wire_read(reader, &w, 3, 64, out) == RPL_IO);
  int64_t values[] = {INT64_MIN, INT64_MAX, -1, 0, 1};
  for (unsigned i = 0; i < 5; ++i) {
    struct rpo_reply a = {UINT64_MAX, 16, values[i]}, b = {1, 1, 2};
    unsigned char encoded[24];
    CHECK(rpo_reply_encode(a, encoded) == RPL_OK);
    CHECK(rpo_reply_decode(encoded, &b) == RPL_OK && b.uuid == a.uuid &&
          b.position == a.position && b.total == a.total);
    encoded[12] = 1;
    b.total = 99;
    CHECK(rpo_reply_decode(encoded, &b) == RPL_CORRUPT && b.total == 99);
  }
  return 0;
}
static int faults_case(void) {
  unsigned observations = 0;
  for (unsigned which = 0; which < 2; ++which)
    for (unsigned mode = 0; mode < 2; ++mode)
      for (unsigned cut = 1; cut <= 37; ++cut) {
        CHECK(!setup());
        CHECK(rpl_append(&source.log, 1, 2, 7) == RPL_OK);
        struct rpl_event e;
        CHECK(rpo_next(&sender, &e) == RPL_OK);
        struct rpo_reply reply = {8, 8, 8};
        if (which) {
          CHECK(rpo_receive(&receiver, 19, 7, e, &reply) == RPL_OK);
          rpo_cut(&ack, cut, mode);
          int r = rpo_ack(&sender, reply);
          CHECK(r != RPL_OK);
          CHECK(!rpo_reopen(&ack));
        } else {
          rpo_cut(&inbox, cut, mode);
          int r = rpo_receive(&receiver, 19, 7, e, &reply);
          CHECK(r != RPL_OK && reply.total == 8);
          CHECK(!rpo_reopen(&inbox));
        }
        unsigned committed = mode && (cut == 36 || cut == 37);
        CHECK((which ? ack.log.memory.count : inbox.log.memory.count) ==
              committed);
        CHECK(rpo_receiver(&inbox.log, &receiver) == RPL_OK);
        int r = rpo_receive(&receiver, 19, 7, e, &reply);
        CHECK(r == RPL_OK || r == RPO_REPLAY);
        CHECK(reply.total == 7 && inbox.log.memory.count == 1);
        r = rpo_ack(&sender, reply);
        CHECK(r == RPL_OK || r == RPO_EMPTY);
        CHECK(ack.log.memory.count == 1);
        int64_t total = 0;
        CHECK(rpo_projection(&inbox.log, 1, &total) == RPL_OK && total == 7);
        ++observations;
      }
  printf("OUTBOX_FAULTS %u\n", observations);
  return 0;
}
static int oracle(void) {
  char op[16];
  CHECK(!setup());
  while (scanf("%15s", op) == 1) {
    int r = RPL_INVALID;
    unsigned p = 0, key = 0, kind = 0, which = 0;
    int32_t value = 0;
    uint64_t uuid = 0;
    int64_t total = 0;
    unsigned schema = 0;
    if (!strcmp(op, "reset")) {
      CHECK(!setup());
      r = 0;
    } else if (!strcmp(op, "append")) {
      CHECK(scanf("%u %u %" SCNd32, &kind, &key, &value) == 3);
      r = rpl_append(&source.log, kind, key, value);
    } else if (!strcmp(op, "receive")) {
      CHECK(scanf("%" SCNu64 " %u %u %u %u %" SCNd32, &uuid, &schema, &p, &key,
                  &kind, &value) == 6);
      struct rpo_reply reply = {99, 99, 99};
      r = rpo_receive(&receiver, uuid, schema, (struct rpl_event){p, key, kind, value},
                      &reply);
      total = reply.total;
    } else if (!strcmp(op, "ack")) {
      CHECK(scanf("%" SCNu64 " %u %" SCNd64, &uuid, &p, &total) == 3);
      r = rpo_ack(&sender, (struct rpo_reply){uuid, p, total});
    } else if (!strcmp(op, "reopen")) {
      CHECK(scanf("%u", &which) == 1 && which < 3);
      CHECK(!rpo_reopen(which == 0 ? &source : which == 1 ? &ack : &inbox));
      r = 0;
    } else
      return 1;
    int64_t projection = 0;
    CHECK(rpo_projection(&inbox.log, inbox.log.memory.count, &projection) ==
          RPL_OK);
    printf("%d %u %u %u %" PRId64 " %" PRId64 "\n", r, source.log.memory.count,
           ack.log.memory.count, inbox.log.memory.count, projection, total);
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "outbox_receive"))
    return receive_case();
  if (!strcmp(argv[1], "outbox_ack"))
    return ack_case();
  if (!strcmp(argv[1], "outbox_transport"))
    return transport_case();
  if (!strcmp(argv[1], "outbox_faults"))
    return faults_case();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  return 2;
}

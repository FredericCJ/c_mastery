#ifndef CMASTERY_REPLICATED_OUTBOX_H
#define CMASTERY_REPLICATED_OUTBOX_H
#include "durable.h"
#define RPO_ACK_SCHEMA UINT32_C(0x41434b31)
enum RpoStatus { RPO_EMPTY = 100, RPO_GAP, RPO_REPLAY, RPO_CONFLICT };
typedef struct {
  RplLog *events, *acknowledgments;
} RpoSender;
typedef struct {
  RplLog *inbox;
} RpoReceiver;
typedef struct {
  uint64_t uuid;
  unsigned position;
  int64_t total;
} RpoReply;
/* Serialized local protocol. Sender source and acknowledgment logs are distinct
 * exclusive stores; receiver has a third store and the same event UUID/schema.
 * An event is its own durable outbox obligation. ACK history is an exact
 * prefix, with schema RPO_ACK_SCHEMA, key0 and values1..count. Caller retains
 * all logs through calls; copied wire records do not borrow their addresses. */
int rpo_sender(RplLog *events, RplLog *acknowledgments, RpoSender *out);
int rpo_receiver(RplLog *inbox, RpoReceiver *out);
int rpo_next(RpoSender *sender, RplEvent *out);
int rpo_receive(RpoReceiver *receiver, uint64_t uuid, uint32_t schema,
                RplEvent event, RpoReply *out);
int rpo_ack(RpoSender *sender, RpoReply reply);
int rpo_projection(const RplLog *events, unsigned position, int64_t *out);
#endif

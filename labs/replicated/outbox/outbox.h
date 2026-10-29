#ifndef CMASTERY_REPLICATED_OUTBOX_H
#define CMASTERY_REPLICATED_OUTBOX_H
#include "durable.h"
#define RPO_ACK_SCHEMA UINT32_C(0x41434b31)

enum rpo_status
{
    RPO_EMPTY = 100,
    RPO_GAP,
    RPO_REPLAY,
    RPO_CONFLICT
};

struct rpo_sender
{
    struct rpl_log *events, *acknowledgments;
};

struct rpo_receiver
{
    struct rpl_log *inbox;
};

struct rpo_reply
{
    uint64_t uuid;
    unsigned position;
    int64_t total;
};

/* Serialized local protocol. Sender source and acknowledgment logs are distinct
 * exclusive stores; receiver has a third store and the same event UUID/schema.
 * An event is its own durable outbox obligation. ACK history is an exact
 * prefix, with schema RPO_ACK_SCHEMA, key0 and values1..count. Caller retains
 * all logs through calls; copied wire records do not borrow their addresses. */
int rpo_sender(struct rpl_log *events, struct rpl_log *acknowledgments,
               struct rpo_sender *out);
int rpo_receiver(struct rpl_log *inbox, struct rpo_receiver *out);
int rpo_next(struct rpo_sender *sender, struct rpl_event *out);
int rpo_receive(struct rpo_receiver *receiver, uint64_t uuid, uint32_t schema,
                struct rpl_event event, struct rpo_reply *out);
int rpo_ack(struct rpo_sender *sender, struct rpo_reply reply);
int rpo_projection(const struct rpl_log *events, unsigned position,
                   int64_t *out);
#endif

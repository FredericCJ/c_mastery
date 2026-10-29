#ifndef CMASTERY_RESILIENCE_OUTBOX_H
#define CMASTERY_RESILIENCE_OUTBOX_H
#include "journal.h"
#define RZS_ACK_BYTES 32u

struct rzs_event
{
    unsigned sequence, client, id, state;
    uint64_t epoch;
    int result;
};

struct rzs_outbox
{
    struct rzj_journal *journal;
    struct msg_hub *hub;
    struct rzj_io acknowledgments;
    struct rz_queue pending;
    struct rzs_event events[4];
    unsigned count, ack_records, acknowledged, seen[2];
    uint64_t epoch;
    bool poisoned;
};

/* State1 waiting, state2 copied into channel, state3 delivered/unacknowledged. */
int rzs_open(struct rzj_journal *, struct rzj_io, struct msg_hub *,
             struct rzs_outbox *);
int rzs_apply(struct rzs_outbox *, uint64_t, struct rzj_request, int *, bool *);
int rzs_rotate(struct rzs_outbox *, uint64_t, uint64_t *);
int rzs_forward(struct rzs_outbox *);
int rzs_deliver(struct rzs_outbox *, struct rzs_event *);
int rzs_ack(struct rzs_outbox *, unsigned);
#endif

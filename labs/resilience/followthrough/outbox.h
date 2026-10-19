#ifndef CMASTERY_RESILIENCE_OUTBOX_H
#define CMASTERY_RESILIENCE_OUTBOX_H
#include "journal.h"
#define RZS_ACK_BYTES 32u
typedef struct {unsigned sequence,client,id,state;uint64_t epoch;int result;} RzsEvent;
typedef struct {
    RzjJournal *journal;
    MsgHub *hub;
    RzjIO acknowledgments;
    RZQueue pending;
    RzsEvent events[4];
    unsigned count,ack_records,acknowledged,seen[2];
    uint64_t epoch;
    bool poisoned;
} RzsOutbox;
/* State1 waiting, state2 copied into channel, state3 delivered/unacknowledged. */
int rzs_open(RzjJournal *,RzjIO,MsgHub *,RzsOutbox *);
int rzs_apply(RzsOutbox *,uint64_t,RzjRequest,int *,bool *);
int rzs_rotate(RzsOutbox *,uint64_t,uint64_t *);
int rzs_forward(RzsOutbox *);
int rzs_deliver(RzsOutbox *,RzsEvent *);
int rzs_ack(RzsOutbox *,unsigned);
#endif

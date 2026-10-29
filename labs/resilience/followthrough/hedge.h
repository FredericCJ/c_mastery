#ifndef CMASTERY_RESILIENCE_OWNED_HEDGE_H
#define CMASTERY_RESILIENCE_OWNED_HEDGE_H
#include "breaker.h"
#include <pthread.h>

struct rzh_blob {unsigned char *data;size_t size;void *context;void (*dispose)(void *,unsigned char *);};
struct rzh_arbiter;
struct rzh_ticket {struct rzh_arbiter *owner;uint64_t generation;unsigned attempt;};
struct rzh_result {struct rzh_blob blob;unsigned value,attempt;};
struct rzh_snapshot {
    uint64_t generation;
    unsigned started,completed,claimed,finalized,cancel_requested,callbacks,permits;
    int winner,total;
    bool abandoned,result_ready,taken;
};
struct rzh_arbiter {
    pthread_mutex_t mutex;
    struct rz_hedge core;
    struct rzb_gate gate;
    struct rz_receiver receiver;
    struct rzf_borrow request;
    struct rzb_permit permits[2];
    struct rzh_blob winner_blob;
    void *context;
    uint64_t (*clock)(void *);
    int (*decode)(void *,const unsigned char *,size_t,unsigned *);
    uint64_t generation,fence,last;
    unsigned client,id,claimed,finalized,callbacks;
    bool active,abandoned,result_ready,taken;
};
int rzh_init(struct rzh_arbiter *,void *,uint64_t (*)(void *),int (*)(void *,const unsigned char *,size_t,unsigned *));
int rzh_begin(struct rzh_arbiter *,struct rzf_scope *,uint64_t,unsigned,unsigned,uint64_t,struct rzh_ticket *);
int rzh_start(struct rzh_arbiter *,struct rzh_ticket *);
int rzh_effect(struct rzh_arbiter *,struct rzh_ticket,int,int *,bool *);
int rzh_complete(struct rzh_arbiter *,struct rzh_ticket,struct rzh_blob *);
int rzh_cancelled(struct rzh_arbiter *,struct rzh_ticket,bool *);
int rzh_take(struct rzh_arbiter *,struct rzh_result *);
int rzh_abandon(struct rzh_arbiter *);
int rzh_snapshot(struct rzh_arbiter *,struct rzh_snapshot *);
int rzh_quiescent(struct rzh_arbiter *);
int rzh_destroy(struct rzh_arbiter *);
#endif

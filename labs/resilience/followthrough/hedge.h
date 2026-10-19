#ifndef CMASTERY_RESILIENCE_OWNED_HEDGE_H
#define CMASTERY_RESILIENCE_OWNED_HEDGE_H
#include "breaker.h"
#include <pthread.h>

typedef struct {unsigned char *data;size_t size;void *context;void (*dispose)(void *,unsigned char *);} RzhBlob;
typedef struct RzhArbiter RzhArbiter;
typedef struct {RzhArbiter *owner;uint64_t generation;unsigned attempt;} RzhTicket;
typedef struct {RzhBlob blob;unsigned value,attempt;} RzhResult;
typedef struct {
    uint64_t generation;
    unsigned started,completed,claimed,finalized,cancel_requested,callbacks,permits;
    int winner,total;
    bool abandoned,result_ready,taken;
} RzhSnapshot;
struct RzhArbiter {
    pthread_mutex_t mutex;
    RZHedge core;
    RzbGate gate;
    RZReceiver receiver;
    RzfBorrow request;
    RzbPermit permits[2];
    RzhBlob winner_blob;
    void *context;
    uint64_t (*clock)(void *);
    int (*decode)(void *,const unsigned char *,size_t,unsigned *);
    uint64_t generation,fence,last;
    unsigned client,id,claimed,finalized,callbacks;
    bool active,abandoned,result_ready,taken;
};
int rzh_init(RzhArbiter *,void *,uint64_t (*)(void *),int (*)(void *,const unsigned char *,size_t,unsigned *));
int rzh_begin(RzhArbiter *,RzfScope *,uint64_t,unsigned,unsigned,uint64_t,RzhTicket *);
int rzh_start(RzhArbiter *,RzhTicket *);
int rzh_effect(RzhArbiter *,RzhTicket,int,int *,bool *);
int rzh_complete(RzhArbiter *,RzhTicket,RzhBlob *);
int rzh_cancelled(RzhArbiter *,RzhTicket,bool *);
int rzh_take(RzhArbiter *,RzhResult *);
int rzh_abandon(RzhArbiter *);
int rzh_snapshot(RzhArbiter *,RzhSnapshot *);
int rzh_quiescent(RzhArbiter *);
int rzh_destroy(RzhArbiter *);
#endif

#ifndef CMASTERY_RESILIENCE_PRESSURE_H
#define CMASTERY_RESILIENCE_PRESSURE_H
#include "scope.h"

typedef struct {unsigned value;void *owned;void (*dispose)(void *);} RzpValue;
typedef struct {RZQueue core;RzpValue values[4];} RzpOwnedQueue;
void rzp_owned_init(RzpOwnedQueue *);
int rzp_owned_offer(RzpOwnedQueue *,const RZScope *,uint64_t,RZPolicy,RzpValue *);
int rzp_owned_pop(RzpOwnedQueue *,RzpValue *);
void rzp_value_drop(RzpValue *);
void rzp_owned_close(RzpOwnedQueue *);

typedef struct RzpPump RzpPump;
typedef struct {const RzpPump *owner;unsigned index,generation;} RzpTicket;
typedef struct {const RzpPump *owner;unsigned index,generation;} RzpCredit;
typedef struct {RzpValue value;unsigned generation,state;} RzpSlot;
typedef struct {RzfBorrow scope;RzpValue value;unsigned generation;int result;bool active,pending,polling;} RzpProducer;
struct RzpPump {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    MsgHub hub;
    RZQueue queue;
    RzpSlot slots[4];
    RzpProducer producer[2];
    unsigned credits,next;
    uint64_t consumed,returned,last;
    bool closed,initialized;
};
typedef struct {unsigned queued,transport,leased,pending,terminal,credits,retired;uint64_t consumed,returned;} RzpSnapshot;
int rzp_init(RzpPump *,size_t);
int rzp_begin(RzpPump *,unsigned,RzfScope *,uint64_t,RzpValue *,RzpTicket *);
int rzp_poll(RzpPump *,RzpTicket,bool,int *);
int rzp_cancel(RzpPump *,RzpTicket);
int rzp_tick(RzpPump *,uint64_t);
int rzp_forward(RzpPump *);
int rzp_receive(RzpPump *,RzpValue *,RzpCredit *);
int rzp_release(RzpPump *,RzpCredit);
int rzp_close(RzpPump *);
int rzp_destroy(RzpPump *);
int rzp_snapshot(RzpPump *,RzpSnapshot *);
#endif

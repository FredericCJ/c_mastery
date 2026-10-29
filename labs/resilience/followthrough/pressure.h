#ifndef CMASTERY_RESILIENCE_PRESSURE_H
#define CMASTERY_RESILIENCE_PRESSURE_H
#include "scope.h"

struct rzp_value {unsigned value;void *owned;void (*dispose)(void *);};
struct rzp_owned_queue {struct rz_queue core;struct rzp_value values[4];};
void rzp_owned_init(struct rzp_owned_queue *);
int rzp_owned_offer(struct rzp_owned_queue *,const struct rz_scope *,uint64_t,enum rz_policy,struct rzp_value *);
int rzp_owned_pop(struct rzp_owned_queue *,struct rzp_value *);
void rzp_value_drop(struct rzp_value *);
void rzp_owned_close(struct rzp_owned_queue *);

struct rzp_pump;
struct rzp_ticket {const struct rzp_pump *owner;unsigned index,generation;};
struct rzp_credit {const struct rzp_pump *owner;unsigned index,generation;};
struct rzp_slot {struct rzp_value value;unsigned generation,state;};
struct rzp_producer {struct rzf_borrow scope;struct rzp_value value;unsigned generation;int result;bool active,pending,polling;};
struct rzp_pump {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    struct msg_hub hub;
    struct rz_queue queue;
    struct rzp_slot slots[4];
    struct rzp_producer producer[2];
    unsigned credits,next;
    uint64_t consumed,returned,last;
    bool closed,initialized;
};
struct rzp_snapshot {unsigned queued,transport,leased,pending,terminal,credits,retired;uint64_t consumed,returned;};
int rzp_init(struct rzp_pump *,size_t);
int rzp_begin(struct rzp_pump *,unsigned,struct rzf_scope *,uint64_t,struct rzp_value *,struct rzp_ticket *);
int rzp_poll(struct rzp_pump *,struct rzp_ticket,bool,int *);
int rzp_cancel(struct rzp_pump *,struct rzp_ticket);
int rzp_tick(struct rzp_pump *,uint64_t);
int rzp_forward(struct rzp_pump *);
int rzp_receive(struct rzp_pump *,struct rzp_value *,struct rzp_credit *);
int rzp_release(struct rzp_pump *,struct rzp_credit);
int rzp_close(struct rzp_pump *);
int rzp_destroy(struct rzp_pump *);
int rzp_snapshot(struct rzp_pump *,struct rzp_snapshot *);
#endif

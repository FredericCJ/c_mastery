#ifndef CMASTERY_RESILIENCE_BREAKER_H
#define CMASTERY_RESILIENCE_BREAKER_H
#include "scope.h"

typedef struct {unsigned window,minimum,numerator,denominator;} RzbPolicy;
typedef struct {unsigned phase,epoch,count,head,failures;bool samples[8],retired;uint64_t until;} RzbHealth;
typedef struct {RzfBorrow scope;unsigned generation,epoch,cost;bool active,cancelled;} RzbSlot;
typedef struct RzbGate {
    RzbPolicy policy;
    RzbHealth health[2];
    RzbSlot slots[4];
    unsigned reserved[2],used[2];
    uint64_t last;
    bool closed;
} RzbGate;
typedef struct {const RzbGate *owner;unsigned slot,generation;} RzbPermit;
int rzb_init(RzbGate *,unsigned,unsigned,RzbPolicy,uint64_t);
int rzb_admit(RzbGate *,unsigned,unsigned,RzfScope *,uint64_t,RzbPermit *);
int rzb_complete(RzbGate *,RzbPermit,uint64_t,int);
int rzb_cancel(RzbGate *,RzbPermit);
int rzb_cancelled(RzbGate *,RzbPermit,uint64_t,bool *);
int rzb_close(RzbGate *);
int rzb_quiescent(const RzbGate *);
#endif

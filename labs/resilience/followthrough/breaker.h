#ifndef CMASTERY_RESILIENCE_BREAKER_H
#define CMASTERY_RESILIENCE_BREAKER_H
#include "scope.h"

struct rzb_policy {unsigned window,minimum,numerator,denominator;};
struct rzb_health {unsigned phase,epoch,count,head,failures;bool samples[8],retired;uint64_t until;};
struct rzb_slot {struct rzf_borrow scope;unsigned generation,epoch,cost;bool active,cancelled;};
struct rzb_gate {
    struct rzb_policy policy;
    struct rzb_health health[2];
    struct rzb_slot slots[4];
    unsigned reserved[2],used[2];
    uint64_t last;
    bool closed;
};
struct rzb_permit {const struct rzb_gate *owner;unsigned slot,generation;};
int rzb_init(struct rzb_gate *,unsigned,unsigned,struct rzb_policy,uint64_t);
int rzb_admit(struct rzb_gate *,unsigned,unsigned,struct rzf_scope *,uint64_t,struct rzb_permit *);
int rzb_complete(struct rzb_gate *,struct rzb_permit,uint64_t,int);
int rzb_cancel(struct rzb_gate *,struct rzb_permit);
int rzb_cancelled(struct rzb_gate *,struct rzb_permit,uint64_t,bool *);
int rzb_close(struct rzb_gate *);
int rzb_quiescent(const struct rzb_gate *);
#endif

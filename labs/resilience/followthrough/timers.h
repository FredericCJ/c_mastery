#ifndef CMASTERY_RESILIENCE_TIMERS_H
#define CMASTERY_RESILIENCE_TIMERS_H
#include "resilience.h"

#define RZT_CAPACITY 16u
typedef enum { RZT_LIST, RZT_HEAP, RZT_WHEEL } RztKind;
typedef struct {
    uint64_t due, sequence;
    uint32_t generation;
    int previous, next, position, bucket;
    bool active;
} RztNode;
typedef struct {
    uint64_t comparisons, link_writes, ticks, cascade_visits;
    unsigned largest_expiry;
} RztWork;
typedef struct RztTimers {
    RztKind kind;
    uint64_t now, observed, sequence;
    RztNode nodes[RZT_CAPACITY];
    int heads[16], heap[RZT_CAPACITY];
    unsigned count, heap_count;
    RztWork work;
} RztTimers;
typedef struct { const RztTimers *owner; unsigned slot; uint32_t generation; } RztId;
typedef struct { RztId id; uint64_t due, sequence, observed, lateness; } RztEvent;
typedef struct { RztEvent events[RZT_CAPACITY]; unsigned count; } RztEvents;

int rzt_init(RztTimers *, RztKind, uint64_t);
int rzt_schedule(RztTimers *, uint64_t, uint64_t, RztId *);
int rzt_cancel(RztTimers *, RztId);
int rzt_advance(RztTimers *, uint64_t, RztEvents *);
int rzt_invariant(const RztTimers *);
#endif

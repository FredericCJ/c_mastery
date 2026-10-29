#ifndef CMASTERY_RESILIENCE_TIMERS_H
#define CMASTERY_RESILIENCE_TIMERS_H
#include "resilience.h"

#define RZT_CAPACITY 16u
enum rzt_kind { RZT_LIST, RZT_HEAP, RZT_WHEEL };
struct rzt_node {
    uint64_t due, sequence;
    uint32_t generation;
    int previous, next, position, bucket;
    bool active;
};
struct rzt_work {
    uint64_t comparisons, link_writes, ticks, cascade_visits;
    unsigned largest_expiry;
};
struct rzt_timers {
    enum rzt_kind kind;
    uint64_t now, observed, sequence;
    struct rzt_node nodes[RZT_CAPACITY];
    int heads[16], heap[RZT_CAPACITY];
    unsigned count, heap_count;
    struct rzt_work work;
};
struct rzt_id { const struct rzt_timers *owner; unsigned slot; uint32_t generation; };
struct rzt_event { struct rzt_id id; uint64_t due, sequence, observed, lateness; };
struct rzt_events { struct rzt_event events[RZT_CAPACITY]; unsigned count; };

int rzt_init(struct rzt_timers *, enum rzt_kind, uint64_t);
int rzt_schedule(struct rzt_timers *, uint64_t, uint64_t, struct rzt_id *);
int rzt_cancel(struct rzt_timers *, struct rzt_id);
int rzt_advance(struct rzt_timers *, uint64_t, struct rzt_events *);
int rzt_invariant(const struct rzt_timers *);
#endif

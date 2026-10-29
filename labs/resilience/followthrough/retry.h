#ifndef CMASTERY_RESILIENCE_RETRY_H
#define CMASTERY_RESILIENCE_RETRY_H
#include "scope.h"
#include "fractional.h"
#include "v4_com.h"

struct rzr_random { uint32_t state; uint64_t draws; };
enum rzr_jitter { RZR_FULL_JITTER, RZR_NO_JITTER, RZR_EQUAL_JITTER };
struct rzr_retry {
    struct rzf_borrow scope;
    struct rz_retry retry;
    struct rzf_bucket *limiter;
    struct net_stack *transport;
    struct rzr_random random;
    uint64_t due;
    unsigned original, dispatched, dropped;
    bool pending, closed;
};

int rzr_reduce(uint32_t, unsigned, uint32_t, uint32_t *);
int rzr_uniform(struct rzr_random *, uint32_t, uint32_t *);
int rzr_init(struct rzr_retry *, struct rzf_scope *, struct rzf_bucket *, struct net_stack *, unsigned,
              uint64_t, uint64_t, bool, uint32_t);
int rzr_original(struct rzr_retry *, uint64_t, v4_packet_sink_fn, void *);
int rzr_plan(struct rzr_retry *, uint64_t, uint64_t, enum rzr_jitter, uint64_t *);
int rzr_dispatch(struct rzr_retry *, uint64_t, v4_packet_sink_fn, void *);
int rzr_ack_packet(struct rzr_retry *, const void *, size_t, uint64_t, struct net_packet *);
int rzr_finish(struct rzr_retry *);
int rzr_abort(struct rzr_retry *);
#endif

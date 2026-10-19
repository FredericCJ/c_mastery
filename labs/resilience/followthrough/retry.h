#ifndef CMASTERY_RESILIENCE_RETRY_H
#define CMASTERY_RESILIENCE_RETRY_H
#include "scope.h"
#include "fractional.h"
#include "v4_com.h"

typedef struct { uint32_t state; uint64_t draws; } RzrRandom;
typedef enum { RZR_FULL_JITTER, RZR_NO_JITTER, RZR_EQUAL_JITTER } RzrJitter;
typedef struct {
    RzfBorrow scope;
    RZRetry retry;
    RzfBucket *limiter;
    NetStack *transport;
    RzrRandom random;
    uint64_t due;
    unsigned original, dispatched, dropped;
    bool pending, closed;
} RzrRetry;

int rzr_reduce(uint32_t, unsigned, uint32_t, uint32_t *);
int rzr_uniform(RzrRandom *, uint32_t, uint32_t *);
int rzr_init(RzrRetry *, RzfScope *, RzfBucket *, NetStack *, unsigned,
              uint64_t, uint64_t, bool, uint32_t);
int rzr_original(RzrRetry *, uint64_t, V4PacketSink, void *);
int rzr_plan(RzrRetry *, uint64_t, uint64_t, RzrJitter, uint64_t *);
int rzr_dispatch(RzrRetry *, uint64_t, V4PacketSink, void *);
int rzr_ack_packet(RzrRetry *, const void *, size_t, uint64_t, NetPacket *);
int rzr_finish(RzrRetry *);
int rzr_abort(RzrRetry *);
#endif

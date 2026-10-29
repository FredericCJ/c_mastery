#ifndef CMASTERY_COMPLETION_DEPTH_H
#define CMASTERY_COMPLETION_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum co_state
{
    CO_FREE,
    CO_BUSY,
    CO_DONE,
    CO_RETIRED
};

struct co_slot
{
    unsigned generation;
    enum co_state state;
    uint64_t token;
    unsigned char *buffer;
    size_t capacity, requested;
    int result, cancel_pending, cancel_seen, cancel_result;
};

struct co_pool
{
    struct co_slot slot[4];
    unsigned generation_limit;
};

/* Serialized single issuer. Buffers remain owned by the pool throughout its
 * lifetime. A successful target CQE permits inspection; release permits reuse.
 * A cancel CQE never ends the target buffer's loan. */
int co_init(struct co_pool *, unsigned char *buffers[4], size_t capacity,
            unsigned limit);
int co_acquire(struct co_pool *, unsigned slot, size_t requested,
               uint64_t *token);
int co_cancel(struct co_pool *, uint64_t target, uint64_t *cancel_token);
int co_complete(struct co_pool *, uint64_t token, int result,
                unsigned cqe_flags);
int co_release(struct co_pool *, uint64_t target);
int co_quiescent(const struct co_pool *);
#endif

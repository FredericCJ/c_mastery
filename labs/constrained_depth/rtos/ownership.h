#ifndef CMASTERY_CONSTRAINED_RTOS_OWNERSHIP_H
#define CMASTERY_CONSTRAINED_RTOS_OWNERSHIP_H
#include <stddef.h>
#include <stdint.h>

enum
{
    CR_OK,
    CR_INVALID,
    CR_FULL,
    CR_STALE,
    CR_BUSY,
    CR_RANGE,
    CR_UNKNOWN
};

enum
{
    CR_FREE,
    CR_OWNED,
    CR_OFFERED,
    CR_CONSUMER,
    CR_RETIRED
};

struct cr_token
{
    uint64_t world, generation;
    unsigned slot;
};

struct cr_payload
{
    unsigned char bytes[16];
    size_t length;
    uint64_t generation;
    unsigned state;
};

struct cr_pool
{
    struct cr_payload slots[3];
    uint64_t world;
    unsigned closed;
};

/* Serialize each metadata transition; public objects must not be forged.
 * Init requires whole-world quiescence and a never-reused positive world.
 * Slots0/1 ordinary, slot2 fault-only; payload is copied before OWNED publication.
 * OFFERED covers a possibly blocked queue send. A consumer can claim it before
 * the sender returns. After successful send the sender must not touch it again.
 * Failed send has published no item and authorizes abort then owner release. */
int cr_pool_init(struct cr_pool *, uint64_t);
int cr_allocate(struct cr_pool *, int, const unsigned char *, size_t,
                struct cr_token *);
int cr_offer(struct cr_pool *, struct cr_token);
int cr_abort_offer(struct cr_pool *, struct cr_token);
int cr_claim(struct cr_pool *, struct cr_token);
int cr_read(const struct cr_pool *, struct cr_token, unsigned char *, size_t,
            size_t *);
int cr_release(struct cr_pool *, struct cr_token);
int cr_close(struct cr_pool *);
int cr_drained(const struct cr_pool *);
/* Inputs are actual byte reservations. Unknown bound is not zero. */
int cr_budget(const uint64_t *, size_t, int, uint64_t, uint64_t *);
#endif

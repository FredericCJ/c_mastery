#ifndef SMP_INTERRUPTS_H
#define SMP_INTERRUPTS_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

enum
{
    SI_OK,
    SI_INVALID,
    SI_BUSY,
    SI_FULL,
    SI_STALE,
    SI_DUPLICATE,
    SI_PENDING,
    SI_TIMED_OUT
};

enum
{
    SI_CAPACITY = 4
};

struct si_box
{
    _Atomic(uint64_t) published, acknowledged;
    uint64_t sequence;
    uint32_t items[SI_CAPACITY];
    unsigned count, timed_out;
};

typedef void (*si_consume_fn)(void *, uint32_t);

struct si_round
{
    uint64_t generation;
    unsigned required, completed;
};

int si_round_init(struct si_round *, uint64_t, unsigned);
int si_round_accept(struct si_round *, unsigned, uint64_t);
/* One externally serialized producer and one non-reentrant destination consumer.
 * The owner initializes before sharing and never moves/frees a live box. */
void si_init(struct si_box *);
int si_publish(struct si_box *, const uint32_t *, size_t, uint64_t *);
int si_drain(struct si_box *, uint64_t, si_consume_fn, void *);
int si_status(const struct si_box *, uint64_t);
int si_timeout(struct si_box *, uint64_t);
#endif

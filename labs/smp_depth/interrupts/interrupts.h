#ifndef SMP_INTERRUPTS_H
#define SMP_INTERRUPTS_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
enum { SI_OK, SI_INVALID, SI_BUSY, SI_FULL, SI_STALE, SI_DUPLICATE, SI_PENDING, SI_TIMED_OUT };
enum { SI_CAPACITY=4 };
typedef struct {
    _Atomic(uint64_t) published, acknowledged;
    uint64_t sequence;
    uint32_t items[SI_CAPACITY];
    unsigned count, timed_out;
} SiBox;
typedef void (*SiConsume)(void *, uint32_t);
typedef struct {uint64_t generation;unsigned required,completed;} SiRound;
int si_round_init(SiRound *,uint64_t,unsigned);
int si_round_accept(SiRound *,unsigned,uint64_t);
/* One externally serialized producer and one non-reentrant destination consumer.
 * The owner initializes before sharing and never moves/frees a live box. */
void si_init(SiBox *);
int si_publish(SiBox *, const uint32_t *, size_t, uint64_t *);
int si_drain(SiBox *, uint64_t, SiConsume, void *);
int si_status(const SiBox *, uint64_t);
int si_timeout(SiBox *, uint64_t);
#endif

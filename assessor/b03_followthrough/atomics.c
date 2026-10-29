#include "follow.h"
#ifndef BF_SC
#define BF_SC 0
#endif
#ifndef BF_ATOMIC_MUTANT
#define BF_ATOMIC_MUTANT 0
#endif
#if BF_SC
#define ACQUIRE memory_order_seq_cst
#define RELEASE memory_order_seq_cst
#define FAILURE memory_order_seq_cst
#else
#define ACQUIRE memory_order_acquire
#define RELEASE memory_order_release
#define FAILURE memory_order_relaxed
#endif
void bf_box_init(struct bf_box *m)
{
    atomic_init(&m->state, 0);
    m->payload = 0;
}

int bf_put(struct bf_box *m, int value)
{
    unsigned expected = 0;
    if (!m)
        return B3_INVALID;
    if (!atomic_compare_exchange_strong_explicit(&m->state, &expected, 1,
                                                 ACQUIRE, FAILURE))
        return B3_WAIT;
    m->payload = value;
    atomic_store_explicit(
        &m->state, 2, BF_ATOMIC_MUTANT == 1 ? memory_order_relaxed : RELEASE);
    return B3_OK;
}

int bf_get(struct bf_box *m, int *out)
{
    unsigned expected = 2;
    if (!m || !out)
        return B3_INVALID;
    if (!atomic_compare_exchange_strong_explicit(&m->state, &expected, 3,
                                                 ACQUIRE, FAILURE))
        return B3_WAIT;
    *out = m->payload;
    atomic_store_explicit(
        &m->state, 0, BF_ATOMIC_MUTANT == 2 ? memory_order_relaxed : RELEASE);
    return B3_OK;
}

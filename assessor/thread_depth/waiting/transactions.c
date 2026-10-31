#include "waiting.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/* Both accounts start at half the invariant total of 100. */
enum
{
    ACCOUNT_OPENING = 50
};

/* A failed lock/unlock means broken harness state: abort. */
static void ok(int e)
{
    if (e)
    {
        abort();
    }
}

int tw_bank_init(struct tw_bank *b)
{
    int e = pthread_mutex_init(&b->lock, NULL);
    if (!e)
    {
        b->a = ACCOUNT_OPENING;
        b->b = ACCOUNT_OPENING;
    }
    return e;
}

/*
 * Moves amount from a to b under one lock hold, so tw_bank_total never sees
 * the money in flight. hook (may be NULL) runs between the debit and the
 * credit, still inside the critical section. EINVAL, with no change, for an
 * amount outside [0, a].
 */
int tw_transfer(struct tw_bank *b, int amount, tw_hook_fn hook, void *arg)
{
    ok(pthread_mutex_lock(&b->lock));
    if (amount < 0 || amount > b->a)
    {
        ok(pthread_mutex_unlock(&b->lock));
        return EINVAL;
    }
    b->a -= amount;
#ifdef TW_MUTANT1
    ok(pthread_mutex_unlock(&b->lock));
#endif
    if (hook)
    {
        hook(arg);
    }
#ifdef TW_MUTANT1
    ok(pthread_mutex_lock(&b->lock));
#endif
    b->b += amount;
    ok(pthread_mutex_unlock(&b->lock));
    return 0;
}

int tw_bank_total(struct tw_bank *b)
{
    int n;
    ok(pthread_mutex_lock(&b->lock));
    n = b->a + b->b;
    ok(pthread_mutex_unlock(&b->lock));
    return n;
}

void tw_bank_destroy(struct tw_bank *b)
{
    ok(pthread_mutex_destroy(&b->lock));
}

/*
 * Sequence-lock snapshot of two values whose sum is always 100. The
 * writer makes `sequence` odd while it updates and even again
 * afterwards; a reader retries until it sees the same even sequence
 * before and after reading both values. At most one writer at a time
 * is assumed. Atomic operations keep their default memory order.
 */
void tw_snapshot_init(struct tw_snapshot *s)
{
    atomic_init(&s->sequence, 0);
    atomic_init(&s->a, 50);
    atomic_init(&s->b, 50);
}

int tw_snapshot_write(struct tw_snapshot *s, int a)
{
    unsigned n = atomic_load(&s->sequence);
    if (a < 0 || a > 100)
    {
        return EINVAL;
    }
    if (n > UINT_MAX - 2)
    {
        return EOVERFLOW;
    }
    atomic_store(&s->sequence, n + 1);
    atomic_store(&s->a, a);
    atomic_store(&s->b, 100 - a);
    atomic_store(&s->sequence, n + 2);
    return 0;
}

int tw_snapshot_read(struct tw_snapshot *s, int *total)
{
    for (;;)
    {
        unsigned first = atomic_load(&s->sequence);
        if (first & 1u)
        {
            continue;
        }
        int a = atomic_load(&s->a), b = atomic_load(&s->b);
        unsigned last = atomic_load(&s->sequence);
        if (first == last)
        {
            *total = a + b;
            return 0;
        }
    }
}

int tw_key_next(uint64_t *next, uint64_t *key)
{
    if (*next == UINT64_MAX)
    {
        return EOVERFLOW;
    }
    *key = (*next)++;
    return 0;
}

int tw_order(uint64_t a, uint64_t b)
{
#ifdef TW_MUTANT2
    (void)a;
    (void)b;
    return 1;
#else
    return a < b;
#endif
}

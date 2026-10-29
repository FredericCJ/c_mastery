#include "test_support.h"
#include <limits.h>

struct tw_transfer
{
    struct tw_bank *bank;
    pthread_barrier_t gate;
    int split;
};

static void bank_hook(void *p)
{
    struct tw_transfer *t = p;
    gate(&t->gate);
    gate(&t->gate);
}

static void *transfer(void *p)
{
    struct tw_transfer *t = p;
    if (t->split)
    {
        CHECK(pthread_mutex_lock(&t->bank->lock) == 0);
        t->bank->a -= 10;
        CHECK(pthread_mutex_unlock(&t->bank->lock) == 0);
        bank_hook(t);
        CHECK(pthread_mutex_lock(&t->bank->lock) == 0);
        t->bank->b += 10;
        CHECK(pthread_mutex_unlock(&t->bank->lock) == 0);
    }
    else
        CHECK(tw_transfer(t->bank, 10, bank_hook, t) == 0);
    return NULL;
}

struct snapshot_run
{
    struct tw_snapshot *state;
    atomic_int stop;
    unsigned samples;
};

static void *snap_reader(void *p)
{
    struct snapshot_run *r = p;
    do
    {
        int n = -1;
        CHECK(tw_snapshot_read(r->state, &n) == 0 && n == 100);
        r->samples++;
    } while (!atomic_load(&r->stop));
    return NULL;
}

void test_bank(void)
{
    for (int split = 0; split < 2; split++)
    {
        struct tw_bank b;
        CHECK(tw_bank_init(&b) == 0);
        struct tw_transfer x = {.bank = &b, .split = split};
        CHECK(pthread_barrier_init(&x.gate, NULL, 2) == 0);
        pthread_t thread;
        CHECK(pthread_create(&thread, NULL, transfer, &x) == 0);
        gate(&x.gate);
        int e = pthread_mutex_trylock(&b.lock), total = -1;
        if (!e)
        {
            total = b.a + b.b;
            CHECK(pthread_mutex_unlock(&b.lock) == 0);
        }
        gate(&x.gate);
        CHECK(pthread_join(thread, NULL) == 0);
        CHECK(pthread_barrier_destroy(&x.gate) == 0);
        CHECK(split ? (e == 0 && total == 90) : e == EBUSY);
        CHECK(tw_bank_total(&b) == 100);
        CHECK(tw_transfer(&b, 41, NULL, NULL) == EINVAL);
        CHECK(tw_bank_total(&b) == 100);
        tw_bank_destroy(&b);
    }
    struct tw_snapshot s;
    tw_snapshot_init(&s);
    struct snapshot_run run = {.state = &s};
    atomic_init(&run.stop, 0);
    pthread_t reader;
    CHECK(pthread_create(&reader, NULL, snap_reader, &run) == 0);
    for (int i = 0; i < 10000; i++)
        CHECK(tw_snapshot_write(&s, i % 101) == 0);
    atomic_store(&run.stop, 1);
    CHECK(pthread_join(reader, NULL) == 0);
    CHECK(run.samples > 0);
    unsigned oldsequence = atomic_load(&s.sequence);
    CHECK(tw_snapshot_write(&s, -1) == EINVAL &&
          tw_snapshot_write(&s, 101) == EINVAL);
    CHECK(atomic_load(&s.sequence) == oldsequence);
    atomic_store(&s.sequence, UINT_MAX - 1u);
    CHECK(tw_snapshot_write(&s, 4) == EOVERFLOW);
    int n;
    CHECK(tw_snapshot_read(&s, &n) == 0 && n == 100);
    puts("OBS bank split90 transaction100 snapshot100 wrap-rejected");
}

struct tw_order
{
    pthread_mutex_t *a, *b;
    pthread_barrier_t *gate;
    int blocked, ordered;
    uint64_t ka, kb;
};

static void *order_worker(void *p)
{
    struct tw_order *o = p;
    pthread_mutex_t *first = o->a, *second = o->b;
    if (o->ordered && !tw_order(o->ka, o->kb))
    {
        first = o->b;
        second = o->a;
    }
    CHECK(pthread_mutex_lock(first) == 0);
    if (o->ordered)
    {
        CHECK(pthread_mutex_lock(second) == 0);
        CHECK(pthread_mutex_unlock(second) == 0);
    }
    else
    {
        gate(o->gate);
        o->blocked = pthread_mutex_trylock(second);
        if (o->blocked == 0)
            CHECK(pthread_mutex_unlock(second) == 0);
        gate(o->gate);
    }
    CHECK(pthread_mutex_unlock(first) == 0);
    return NULL;
}

void test_order(void)
{
    pthread_mutex_t a, b;
    CHECK(pthread_mutex_init(&a, NULL) == 0);
    CHECK(pthread_mutex_init(&b, NULL) == 0);
    pthread_barrier_t g;
    CHECK(pthread_barrier_init(&g, NULL, 2) == 0);
    struct tw_order x = {.a = &a, .b = &b, .gate = &g, .ka = 11, .kb = 12},
                    y = {.a = &b, .b = &a, .gate = &g, .ka = 12, .kb = 11};
    pthread_t threads[2];
    CHECK(pthread_create(&threads[0], NULL, order_worker, &x) == 0);
    CHECK(pthread_create(&threads[1], NULL, order_worker, &y) == 0);
    CHECK(pthread_join(threads[0], NULL) == 0);
    CHECK(pthread_join(threads[1], NULL) == 0);
    CHECK(x.blocked == EBUSY && y.blocked == EBUSY);
    CHECK(pthread_mutex_lock(&a) == 0);
    int recursive = pthread_mutex_trylock(&a);
    CHECK(pthread_mutex_unlock(&a) == 0);
    CHECK(recursive == EBUSY);
    CHECK(pthread_barrier_destroy(&g) == 0);
    /* Test the ordering relation before potentially blocking native acquisition.
     */
    CHECK(tw_order(11, 12) && !tw_order(12, 11) && !tw_order(11, 11));
    x.ordered = y.ordered = 1;
    CHECK(pthread_create(&threads[0], NULL, order_worker, &x) == 0);
    CHECK(pthread_create(&threads[1], NULL, order_worker, &y) == 0);
    CHECK(pthread_join(threads[0], NULL) == 0);
    CHECK(pthread_join(threads[1], NULL) == 0);
    uint64_t next = 40, oldkey, newkey;
    CHECK(tw_key_next(&next, &oldkey) == 0);
    CHECK(pthread_mutex_destroy(&a) == 0);
    CHECK(pthread_mutex_init(&a, NULL) == 0);
    CHECK(tw_key_next(&next, &newkey) == 0 && newkey != oldkey);
    CHECK(tw_order(oldkey, newkey));
    next = UINT64_MAX;
    uint64_t sentinel = 9;
    CHECK(tw_key_next(&next, &sentinel) == EOVERFLOW && sentinel == 9);
    CHECK(pthread_mutex_destroy(&a) == 0);
    CHECK(pthread_mutex_destroy(&b) == 0);
    puts("OBS order A-B-A and A-A cycles; stable keys with quiescent reuse");
}

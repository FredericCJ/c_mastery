#include "test_support.h"
#include <limits.h>
#include <sched.h>

struct tw_phase
{
    struct tw_barrier *barrier;
    unsigned index;
    int *cells, *total;
    int broken, result;
};

static void *phase_worker(void *arg)
{
    struct tw_phase *p = arg;
    if (p->broken)
    {
        p->result = tw_barrier_wait(p->barrier);
        return NULL;
    }
    p->cells[p->index] = (int)p->index + 1;
    CHECK(tw_barrier_wait(p->barrier) == 0);
    if (!p->index)
    {
        *p->total = p->cells[0] + p->cells[1] + p->cells[2];
    }
    CHECK(tw_barrier_wait(p->barrier) == 0);
    CHECK(*p->total == 6);
    return NULL;
}

enum
{
    PARTIES = 3,
    PERMITS = 3,
    ONCE_THREADS = 4,
    FAIRNESS_ROUNDS = 1000
};

/* Three threads write one cell each, cross the barrier, thread 0 totals, and
 * everyone crosses again and sees the total: two reusable phases. */
static void barrier_two_phases(void)
{
    struct tw_barrier b;
    CHECK(tw_barrier_init(&b, PARTIES) == 0);
    int cells[PARTIES] = {0};
    int total = 0;
    struct tw_phase p[PARTIES];
    pthread_t t[PARTIES];
    for (unsigned i = 0; i < PARTIES; i++)
    {
        p[i] = (struct tw_phase){
            .barrier = &b, .index = i, .cells = cells, .total = &total};
        CHECK(pthread_create(&t[i], NULL, phase_worker, &p[i]) == 0);
    }
    for (unsigned i = 0; i < PARTIES; i++)
    {
        CHECK(pthread_join(t[i], NULL) == 0);
    }
    CHECK(total == 6 && b.generation == 2 && b.arrived == 0);
    tw_barrier_destroy(&b);
}

/* Two of three participants arrive and wait; breaking the barrier (the third
 * participant departing) must wake both with ECANCELED. */
static void barrier_broken_by_departure(void)
{
    struct tw_barrier b;
    CHECK(tw_barrier_init(&b, PARTIES) == 0);
    struct tw_phase p[2];
    pthread_t t[2];
    for (unsigned i = 0; i < 2; i++)
    {
        p[i] = (struct tw_phase){.barrier = &b, .broken = 1};
        CHECK(pthread_create(&t[i], NULL, phase_worker, &p[i]) == 0);
    }
    for (;;)
    {
        CHECK(pthread_mutex_lock(&b.mutex) == 0);
        unsigned arrived = b.arrived;
        CHECK(pthread_mutex_unlock(&b.mutex) == 0);
        if (arrived == 2)
        {
            break;
        }
        sched_yield();
    }
    CHECK(tw_barrier_break(&b) == 0);
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_join(t[i], NULL) == 0);
    }
    CHECK(p[0].result == ECANCELED && p[1].result == ECANCELED);
    CHECK(tw_barrier_wait(&b) == ECANCELED);
    tw_barrier_destroy(&b);
}

/* A generation counter at its maximum must break the barrier, not wrap. */
static void barrier_generation_overflow(void)
{
    struct tw_barrier b;
    CHECK(tw_barrier_init(&b, 1) == 0);
    CHECK(tw_barrier_wait(&b) == 0 && b.generation == 1);
    b.generation = UINT_MAX;
    CHECK(tw_barrier_wait(&b) == EOVERFLOW);
    CHECK(tw_barrier_wait(&b) == ECANCELED);
    tw_barrier_destroy(&b);
}

void test_barrier(void)
{
    struct tw_barrier rejected;
    CHECK(tw_barrier_init(&rejected, 0) == EINVAL);
    barrier_two_phases();
    barrier_broken_by_departure();
    barrier_generation_overflow();
    puts(
        "OBS barrier two phases total6; departed third participant breaks phase "
        "and wakes both");
}

void test_fairness(void)
{
    /* A finite-state lasso proves repeatability under this explicit scheduler;
       the loops check its state transitions, not an infinite native execution. */
    unsigned readers = 1;
    unsigned writer = 0;
    unsigned writer_waiting = 1;
    unsigned writer_entries = 0;
    for (unsigned i = 0; i < FAIRNESS_ROUNDS; i++)
    {
        CHECK(tw_reader_admit(readers, writer, writer_waiting, 0));
        readers++;
        readers--;
        if (tw_writer_admit(readers, writer))
        {
            writer_entries++;
        }
        CHECK(readers == 1 && !writer);
    }
    CHECK(writer_entries == 0);
    unsigned owner = 1;
    unsigned waiter_entries = 0;
    for (unsigned i = 0; i < FAIRNESS_ROUNDS; i++)
    {
        owner = 0; /* release */
        if (!owner)
        {
            owner = 1; /* stealer */
        }
        if (!owner)
        {
            waiter_entries++;
        }
        CHECK(owner == 1);
    }
    CHECK(waiter_entries == 0);
    unsigned reader_entries = 0;
    for (unsigned i = 0; i < FAIRNESS_ROUNDS; i++)
    {
        CHECK(tw_writer_admit(0, 0));
        writer = 1;
        CHECK(!tw_reader_admit(0, writer, 1, 1));
        writer = 0;
        if (tw_reader_admit(0, writer, 1, 1))
        {
            reader_entries++;
        }
    }
    CHECK(reader_entries == 0);
    CHECK(!tw_reader_admit(0, 0, 1, 1));
    CHECK(tw_reader_admit(0, 0, 0, 1));
    puts("OBS fairness three repeatable safe starvation lassos; no host "
         "scheduling guarantee");
}

static pthread_once_t once = PTHREAD_ONCE_INIT;
static unsigned initialized;

static void initialize(void)
{
    initialized++;
}

static void *once_worker(void *p)
{
    (void)p;
    CHECK(pthread_once(&once, initialize) == 0);
    CHECK(initialized == 1);
    return NULL;
}

struct tw_async
{
    struct tw_permits *pool;
    unsigned token;
    atomic_uint entered;
    int released;
};
/* Only this selected-GCC/Clang compute region excludes instrumentation. A
   sanitizer runtime call itself has no async-cancel-safety contract. */
__attribute__((noinline, no_sanitize("thread", "address", "undefined")))
#if defined(__clang__)
__attribute__((disable_sanitizer_instrumentation))
#endif
static void async_region(atomic_uint *entered)
{
    int old;
    (void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old);
    atomic_store(entered, 1);
    for (;;)
    {
        atomic_signal_fence(memory_order_seq_cst);
    }
}

static void permit_cleanup(void *p)
{
    struct tw_async *a = p;
    int old;
    CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old) == 0);
    CHECK(tw_permit_release(a->pool, a->token) == 0);
    a->released = 1;
}

static void *async_worker(void *p)
{
    struct tw_async *a = p;
    int old, oldtype;
    CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old) == 0);
    CHECK(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype) == 0);
    CHECK(tw_permit_take(a->pool, &a->token) == 0);
    pthread_cleanup_push(permit_cleanup, a);
    async_region(&a->entered);
    pthread_cleanup_pop(1);
    return NULL;
}

/* tw_select maps each requirement 0..3 to the primitive of the same index. */
static void select_primitives(void)
{
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(tw_select(i) == (int)i);
    }
    CHECK(tw_select(4) == -1);
}

static void once_runs_one_initializer(void)
{
    pthread_t t[ONCE_THREADS];
    for (unsigned i = 0; i < ONCE_THREADS; i++)
    {
        CHECK(pthread_create(&t[i], NULL, once_worker, NULL) == 0);
    }
    for (unsigned i = 0; i < ONCE_THREADS; i++)
    {
        CHECK(pthread_join(t[i], NULL) == 0);
    }
    CHECK(initialized == 1);
}

/* Exhaust the pool, probe the error returns, then release every permit. */
static void permit_pool_bounds(struct tw_permits *p)
{
    unsigned tokens[PERMITS];
    unsigned sentinel = 99;
    for (unsigned i = 0; i < PERMITS; i++)
    {
        CHECK(tw_permit_take(p, &tokens[i]) == 0);
    }
    CHECK(p->mask == 7);
    CHECK(tw_permit_take(p, &sentinel) == EAGAIN && sentinel == 99);
    CHECK(tw_permit_release(p, 3) == EINVAL);
    for (unsigned i = 0; i < PERMITS; i++)
    {
        CHECK(tw_permit_release(p, tokens[i]) == 0);
    }
    CHECK(tw_permit_release(p, tokens[0]) == EALREADY);
    CHECK(p->mask == 0 && p->settlements == PERMITS);
}

/* A thread holding a permit is canceled asynchronously; its cleanup handler
 * must give the permit back, so the full capacity is available afterwards. */
static void permit_survives_async_cancel(struct tw_permits *p)
{
    struct tw_async a = {.pool = p};
    atomic_init(&a.entered, 0);
    CHECK(atomic_is_lock_free(&a.entered));
    pthread_t thread;
    CHECK(pthread_create(&thread, NULL, async_worker, &a) == 0);
    while (!atomic_load(&a.entered))
    {
        sched_yield();
    }
    CHECK(pthread_cancel(thread) == 0);
    void *result = NULL;
    CHECK(pthread_join(thread, &result) == 0 && result == PTHREAD_CANCELED);
    CHECK(a.released == 1 && p->mask == 0 && p->settlements == PERMITS + 1);
    unsigned tokens[PERMITS];
    unsigned sentinel = 99;
    for (unsigned i = 0; i < PERMITS; i++)
    {
        CHECK(tw_permit_take(p, &tokens[i]) == 0);
    }
    CHECK(tw_permit_take(p, &sentinel) == EAGAIN);
    for (unsigned i = 0; i < PERMITS; i++)
    {
        CHECK(tw_permit_release(p, tokens[i]) == 0);
    }
}

/* Actual read/write exclusion, no selected preference assumption. */
static void rwlock_exclusion(void)
{
    pthread_rwlock_t rw;
    CHECK(pthread_rwlock_init(&rw, NULL) == 0);
    CHECK(pthread_rwlock_rdlock(&rw) == 0);
    CHECK(pthread_rwlock_trywrlock(&rw) == EBUSY);
    CHECK(pthread_rwlock_unlock(&rw) == 0);
    CHECK(pthread_rwlock_wrlock(&rw) == 0);
    CHECK(pthread_rwlock_tryrdlock(&rw) == EBUSY);
    CHECK(pthread_rwlock_unlock(&rw) == 0);
    CHECK(pthread_rwlock_destroy(&rw) == 0);
}

void test_permit(void)
{
    select_primitives();
    once_runs_one_initializer();
    struct tw_permits p;
    CHECK(tw_permits_init(&p) == 0);
    permit_pool_bounds(&p);
    permit_survives_async_cancel(&p);
    tw_permits_destroy(&p);
    rwlock_exclusion();
    puts("OBS permit capacity3 retained after actual async cancellation; once "
         "initialization1; rw exclusion");
}

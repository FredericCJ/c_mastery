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

void test_barrier(void)
{
    struct tw_barrier b;
    CHECK(tw_barrier_init(&b, 0) == EINVAL);
    CHECK(tw_barrier_init(&b, 3) == 0);
    int cells[3] = {0}, total = 0;
    struct tw_phase p[3];
    pthread_t t[3];
    for (unsigned i = 0; i < 3; i++)
    {
        p[i] = (struct tw_phase){
            .barrier = &b, .index = i, .cells = cells, .total = &total};
        CHECK(pthread_create(&t[i], NULL, phase_worker, &p[i]) == 0);
    }
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(pthread_join(t[i], NULL) == 0);
    }
    CHECK(total == 6 && b.generation == 2 && b.arrived == 0);
    tw_barrier_destroy(&b);
    CHECK(tw_barrier_init(&b, 3) == 0);
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
    CHECK(tw_barrier_init(&b, 1) == 0);
    CHECK(tw_barrier_wait(&b) == 0 && b.generation == 1);
    b.generation = UINT_MAX;
    CHECK(tw_barrier_wait(&b) == EOVERFLOW);
    CHECK(tw_barrier_wait(&b) == ECANCELED);
    tw_barrier_destroy(&b);
    puts(
        "OBS barrier two phases total6; departed third participant breaks phase "
        "and wakes both");
}

void test_fairness(void)
{
    /* A finite-state lasso proves repeatability under this explicit scheduler;
       the loops check its state transitions, not an infinite native execution. */
    unsigned readers = 1, writer = 0, writer_waiting = 1, writer_entries = 0;
    for (unsigned i = 0; i < 1000; i++)
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
    unsigned owner = 1, waiter_entries = 0;
    for (unsigned i = 0; i < 1000; i++)
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
    for (unsigned i = 0; i < 1000; i++)
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

void test_permit(void)
{
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(tw_select(i) == (int)i);
    }
    CHECK(tw_select(4) == -1);
    pthread_t t[4];
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(pthread_create(&t[i], NULL, once_worker, NULL) == 0);
    }
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(pthread_join(t[i], NULL) == 0);
    }
    CHECK(initialized == 1);
    struct tw_permits p;
    CHECK(tw_permits_init(&p) == 0);
    unsigned tokens[3], sentinel = 99;
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(tw_permit_take(&p, &tokens[i]) == 0);
    }
    CHECK(p.mask == 7);
    CHECK(tw_permit_take(&p, &sentinel) == EAGAIN && sentinel == 99);
    CHECK(tw_permit_release(&p, 3) == EINVAL);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(tw_permit_release(&p, tokens[i]) == 0);
    }
    CHECK(tw_permit_release(&p, tokens[0]) == EALREADY);
    CHECK(p.mask == 0 && p.settlements == 3);
    struct tw_async a = {.pool = &p};
    atomic_init(&a.entered, 0);
    CHECK(atomic_is_lock_free(&a.entered));
    CHECK(pthread_create(&t[0], NULL, async_worker, &a) == 0);
    while (!atomic_load(&a.entered))
    {
        sched_yield();
    }
    CHECK(pthread_cancel(t[0]) == 0);
    void *result = NULL;
    CHECK(pthread_join(t[0], &result) == 0 && result == PTHREAD_CANCELED);
    CHECK(a.released == 1 && p.mask == 0 && p.settlements == 4);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(tw_permit_take(&p, &tokens[i]) == 0);
    }
    CHECK(tw_permit_take(&p, &sentinel) == EAGAIN);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(tw_permit_release(&p, tokens[i]) == 0);
    }
    tw_permits_destroy(&p);
    /* Actual read/write exclusion, no selected preference assumption. */
    pthread_rwlock_t rw;
    CHECK(pthread_rwlock_init(&rw, NULL) == 0);
    CHECK(pthread_rwlock_rdlock(&rw) == 0);
    CHECK(pthread_rwlock_trywrlock(&rw) == EBUSY);
    CHECK(pthread_rwlock_unlock(&rw) == 0);
    CHECK(pthread_rwlock_wrlock(&rw) == 0);
    CHECK(pthread_rwlock_tryrdlock(&rw) == EBUSY);
    CHECK(pthread_rwlock_unlock(&rw) == 0);
    CHECK(pthread_rwlock_destroy(&rw) == 0);
    puts("OBS permit capacity3 retained after actual async cancellation; once "
         "initialization1; rw exclusion");
}

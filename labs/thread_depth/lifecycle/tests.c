#define _POSIX_C_SOURCE 200809L
#include "lifecycle.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "TL_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(41);                                                        \
        }                                                                    \
    } while (0)

enum
{
    /* Written into outputs to prove that a failing call left them alone. */
    UNTOUCHED = 99,
    MAX_POOL_WORKERS = 4,
    DETACHED_INPUT_BYTES = 16
};

/* Join replacement that fails on call number `fail` (1-based) and otherwise
 * delegates to pthread_join. */
struct join_script
{
    unsigned calls;
    unsigned fail;
};

static int scripted(pthread_t t, void **out, void *arg)
{
    struct join_script *s = arg;
    s->calls++;
    if (s->calls == s->fail)
    {
        return EBUSY;
    }
    return pthread_join(t, out);
}

/* Expected pool sum for `count` workers: 1^2 + 2^2 + ... + count^2. */
static unsigned sum_of_squares(unsigned count)
{
    unsigned sum = 0;
    for (unsigned i = 1; i <= count; i++)
    {
        sum += i * i;
    }
    return sum;
}

static void pool_rejects_bad_arguments(void)
{
    int e = UNTOUCHED;
    CHECK(!tl_pool_start(5, 0, 0, &e) && e == EINVAL);
    CHECK(!tl_pool_start(2, 3, 0, &e) && e == EINVAL);
    CHECK(!tl_pool_start(2, 0, 1, &e) && e == ENOMEM);
}

/* Every (workers, failing create ordinal) pair: a partial pool must still be
 * collectable, and disposing it before collection must fail. */
static void pool_create_failures(void)
{
    int e = UNTOUCHED;
    for (unsigned n = 0; n <= MAX_POOL_WORKERS; n++)
    {
        for (unsigned f = 0; f <= n; f++)
        {
            struct tl_pool *p = tl_pool_start(n, f, 0, &e);
            CHECK(p && e == 0);
            unsigned count = f ? f - 1 : n;
            if (count)
            {
                CHECK(tl_pool_dispose(&p) == EBUSY && p);
            }
            struct tl_stats out = {UNTOUCHED, UNTOUCHED, UNTOUCHED, UNTOUCHED,
                                   UNTOUCHED};
            CHECK(tl_pool_collect(p, NULL, NULL, &out) == 0);
            unsigned sum = sum_of_squares(count);
            CHECK(out.created == count && out.joined == count &&
                  out.completed == count && out.sum == sum &&
                  out.create_error == (f ? EAGAIN : 0));
            CHECK(tl_pool_dispose(&p) == 0 && !p);
            CHECK(tl_pool_dispose(&p) == EINVAL);
        }
    }
}

/* A join that fails on call number `failure` must leave the output and errno
 * alone and keep the pool undisposable; collecting again must succeed. */
static void pool_join_failures(void)
{
    int e = UNTOUCHED;
    for (unsigned failure = 1; failure <= MAX_POOL_WORKERS; failure++)
    {
        struct tl_pool *p = tl_pool_start(MAX_POOL_WORKERS, 0, 0, &e);
        CHECK(p && e == 0);
        struct join_script s = {0, failure};
        struct tl_stats out = {88, 88, 88, 88, 88};
        errno = ERANGE;
        CHECK(tl_pool_collect(p, scripted, &s, &out) == EBUSY &&
              errno == ERANGE);
        int preserved_join_output = out.created == 88 && out.joined == 88 &&
                                    out.completed == 88 && out.sum == 88 &&
                                    out.create_error == 88;
        CHECK(tl_pool_dispose(&p) == EBUSY && p);
        s.fail = 0;
        CHECK(tl_pool_collect(p, scripted, &s, &out) == 0 && out.joined == 4 &&
              out.sum == 30);
        CHECK(s.calls == 5);
        CHECK(tl_pool_collect(p, scripted, &s, &out) == 0 && out.sum == 30 &&
              s.calls == 5);
        CHECK(tl_pool_dispose(&p) == 0 && !p);
        CHECK(preserved_join_output);
    }
}

static void pool(void)
{
    int previous;
    CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous) == 0);
    pool_rejects_bad_arguments();
    pool_create_failures();
    pool_join_failures();
    puts("OBS pool successful-prefix=15 retained-join-errors=4 sum=30");
    CHECK(pthread_setcancelstate(previous, NULL) == 0);
}

/* Blocks until the creator reports that it is waiting; returns with the
 * monitor mutex held. */
static void lock_when_creator_waiting(struct tl_monitor *m)
{
    CHECK(pthread_mutex_lock(&m->mutex) == 0);
    while (!m->creator_waiting)
    {
        CHECK(pthread_cond_wait(&m->changed, &m->mutex) == 0);
    }
}

/* Called with the mutex held; returns, still holding it, once the last user
 * has freed the job. */
static void wait_for_job_freed(struct tl_monitor *m)
{
    while (!m->freed)
    {
        CHECK(pthread_cond_wait(&m->changed, &m->mutex) == 0);
    }
}

/* The creator exits normally (cancel == 0) or is canceled while it waits for
 * the worker (cancel == 1). Either way the worker keeps its reference, sums
 * its private copy of the bytes and is the last user that frees the job. */
static void detached_creator_exit(int cancel)
{
    struct tl_monitor m;
    CHECK(tl_monitor_init(&m) == 0);
    unsigned char bytes[4] = {1, 2, 3, 4};
    struct tl_creator c = {&m, bytes, 4, cancel, UNTOUCHED};
    pthread_t creator;
    CHECK(pthread_create(&creator, NULL, tl_detached_creator, &c) == 0);
    lock_when_creator_waiting(&m);
    CHECK(m.created == 1 && m.freed == 0);
    CHECK(pthread_mutex_unlock(&m.mutex) == 0);
    if (cancel)
    {
        CHECK(pthread_cancel(creator) == 0);
    }
    void *result = (void *)&m;
    CHECK(pthread_join(creator, &result) == 0);
    CHECK(result == (cancel ? PTHREAD_CANCELED : NULL));
    CHECK(c.error == 0);
    /* The caller's buffer is dead from the worker's point of view. */
    memset(bytes, 99, sizeof bytes);
    CHECK(pthread_mutex_lock(&m.mutex) == 0);
    int worker_pending_after_creator =
        m.live_refs == 1 && m.freed == 0 && m.completed == 0;
    m.release_worker = 1;
    CHECK(pthread_cond_broadcast(&m.changed) == 0);
    wait_for_job_freed(&m);
    CHECK(m.freed == 1 && m.live_refs == 0 && m.completed == 1 && m.sum == 10);
    CHECK(pthread_mutex_unlock(&m.mutex) == 0);
    CHECK(tl_monitor_destroy(&m) == 0);
    CHECK(worker_pending_after_creator);
}

/* Boundary input lengths: empty (no buffer at all) and the maximum. */
static void detached_boundary_length(size_t n)
{
    struct tl_monitor m;
    CHECK(tl_monitor_init(&m) == 0);
    unsigned char input[DETACHED_INPUT_BYTES];
    memset(input, 2, sizeof input);
    struct tl_creator c = {&m, n ? input : NULL, n, 1, UNTOUCHED};
    pthread_t creator;
    CHECK(pthread_create(&creator, NULL, tl_detached_creator, &c) == 0);
    lock_when_creator_waiting(&m);
    m.release_worker = 1;
    CHECK(pthread_cond_broadcast(&m.changed) == 0);
    CHECK(pthread_mutex_unlock(&m.mutex) == 0);
    CHECK(pthread_join(creator, NULL) == 0 && c.error == 0);
    CHECK(pthread_mutex_lock(&m.mutex) == 0);
    wait_for_job_freed(&m);
    CHECK(m.sum == 2 * n && m.freed == 1 && m.live_refs == 0);
    CHECK(pthread_mutex_unlock(&m.mutex) == 0);
    CHECK(tl_monitor_destroy(&m) == 0);
}

static void detached(void)
{
    detached_creator_exit(0);
    detached_creator_exit(1);
    detached_boundary_length(0);
    detached_boundary_length(DETACHED_INPUT_BYTES);
    struct tl_monitor invalid;
    CHECK(tl_monitor_init(&invalid) == 0);
    struct tl_creator c = {&invalid, NULL, 1, 0, UNTOUCHED};
    CHECK(tl_detached_creator(&c) == NULL && c.error == EINVAL);
    CHECK(tl_monitor_destroy(&invalid) == 0);
    puts(
        "OBS detached normal-and-canceled-creator worker-reference-retained copied-sum=10 last-user-free=1");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "pool"))
    {
        pool();
    }
    else if (!strcmp(argv[1], "detached"))
    {
        detached();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}

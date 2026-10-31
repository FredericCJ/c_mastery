#define _POSIX_C_SOURCE 200809L
#include "lifecycle.h"
#include <errno.h>
#include <stdlib.h>

enum
{
    POOL_MAX_WORKERS = 4,
    JOB_MAX_BYTES = 16
};

/* One worker's private input and result; no other thread touches a cell. */
struct tl_cell
{
    unsigned input;
    unsigned result;
};

/*
 * created counts successful pthread_create calls; joined counts successful
 * joins, so threads[joined .. created - 1] are the ones still to be joined.
 * create_error keeps the first create failure for tl_pool_collect to report.
 */
struct tl_pool
{
    pthread_t threads[POOL_MAX_WORKERS];
    struct tl_cell cells[POOL_MAX_WORKERS];
    unsigned created;
    unsigned joined;
    int create_error;
};

static void *square(void *arg)
{
    struct tl_cell *c = arg;
    c->result = c->input * c->input;
    return NULL;
}

/* fail_ordinal is 1-based; 0 means no injected failure. */
static int start_worker(struct tl_pool *p, unsigned index,
                        unsigned fail_ordinal)
{
    if (fail_ordinal == index + 1)
    {
        return EAGAIN;
    }
    return pthread_create(&p->threads[index], NULL, square, &p->cells[index]);
}

/*
 * Returns a pool owning the started threads, or NULL with *error set
 * (EINVAL for bad arguments, ENOMEM when allocation fails or is injected).
 * A create failure is not a start failure: the pool is returned with
 * *error == 0 and the failure is reported later by tl_pool_collect.
 * The caller must collect, then dispose.
 */
struct tl_pool *tl_pool_start(unsigned n, unsigned fail, int allocation,
                              int *error)
{
    if (!error)
    {
        return NULL;
    }
    if (n > POOL_MAX_WORKERS || fail > n || allocation < 0 || allocation > 1)
    {
        *error = EINVAL;
        return NULL;
    }
    struct tl_pool *p = allocation ? NULL : calloc(1, sizeof *p);
    if (!p)
    {
        *error = ENOMEM;
        return NULL;
    }
    for (unsigned i = 0; i < n; i++)
    {
        p->cells[i].input = i + 1;
        int e = start_worker(p, i, fail);
        if (e)
        {
            p->create_error = e;
            break;
        }
        p->created++;
    }
    *error = 0;
    return p;
}

/*
 * Joins the remaining threads in creation order. A join failure returns its
 * error and leaves *out and the pool's joined count as they were, so the
 * call can be repeated. Once every thread is joined, repeating the call is
 * harmless and returns the same statistics.
 */
int tl_pool_collect(struct tl_pool *p, tl_join_fn join, void *context,
                    struct tl_stats *out)
{
    if (!p || !out)
    {
        return EINVAL;
    }
    while (p->joined < p->created)
    {
        void *result = NULL;
        pthread_t thread = p->threads[p->joined];
        int e = join ? join(thread, &result, context)
                     : pthread_join(thread, &result);
        if (e)
        {
#ifdef TL_MUTANT2
            out->joined = p->joined;
#endif
            return e;
        }
        p->joined++;
    }
    unsigned sum = 0;
    for (unsigned i = 0; i < p->created; i++)
    {
        sum += p->cells[i].result;
    }
#ifdef TL_MUTANT1
    sum = 0;
#endif
    *out = (struct tl_stats){p->created, p->joined, p->created, sum,
                             p->create_error};
    return 0;
}

/* Frees the pool and clears *p; EBUSY (pool kept) while threads are unjoined. */
int tl_pool_dispose(struct tl_pool **p)
{
    if (!p || !*p)
    {
        return EINVAL;
    }
    if ((*p)->joined != (*p)->created)
    {
        return EBUSY;
    }
    free(*p);
    *p = NULL;
    return 0;
}

/*
 * A detached worker and its creator share one job through monitor->live_refs.
 * Each side drops its reference with release_job(); whoever drops the last
 * one frees the job. live_refs, freed and every other monitor field are
 * protected by monitor->mutex.
 */
struct tl_job
{
    struct tl_monitor *monitor;
    unsigned char bytes[JOB_MAX_BYTES];
    size_t length;
};

/* Aborts on a failed lock, wait or cleanup call: it means a harness bug. */
static void invariant(int e)
{
    if (e)
    {
        abort();
    }
}

static void release_job(struct tl_job *j)
{
    struct tl_monitor *m = j->monitor;
    invariant(pthread_mutex_lock(&m->mutex));
#ifdef TL_MUTANT3
    if (m->live_refs == 2 && !m->release_worker)
    {
        m->completed = 1;
    }
#endif
    if (--m->live_refs == 0)
    {
        free(j);
        m->freed++;
    }
    invariant(pthread_cond_broadcast(&m->changed));
    invariant(pthread_mutex_unlock(&m->mutex));
}

/* Waits for release_worker, sums its private copy of the input, then drops
 * the worker's reference. It never reads the creator's input buffer. */
static void *detached_worker(void *arg)
{
    struct tl_job *j = arg;
    struct tl_monitor *m = j->monitor;
    invariant(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
    invariant(pthread_mutex_lock(&m->mutex));
    while (!m->release_worker)
    {
        invariant(pthread_cond_wait(&m->changed, &m->mutex));
    }
    unsigned sum = 0;
    for (size_t i = 0; i < j->length; i++)
    {
        sum += j->bytes[i];
    }
    m->sum = sum;
    m->completed = 1;
    invariant(pthread_cond_broadcast(&m->changed));
    invariant(pthread_mutex_unlock(&m->mutex));
    release_job(j);
    return NULL;
}

/* Cleanup handler for the creator. It runs both when the creator is canceled
 * while waiting and on every normal exit through pthread_cleanup_pop(1). The
 * creator holds the monitor mutex on both paths, so unlock it and drop the
 * creator's reference. */
static void creator_cleanup(void *arg)
{
    struct tl_job *j = arg;
    invariant(pthread_mutex_unlock(&j->monitor->mutex));
    release_job(j);
}

int tl_monitor_init(struct tl_monitor *m)
{
    if (!m)
    {
        return EINVAL;
    }
    int e = pthread_mutex_init(&m->mutex, NULL);
    if (e)
    {
        return e;
    }
    e = pthread_cond_init(&m->changed, NULL);
    if (e)
    {
        invariant(pthread_mutex_destroy(&m->mutex));
        return e;
    }
    m->created = 0;
    m->creator_waiting = 0;
    m->release_worker = 0;
    m->completed = 0;
    m->live_refs = 0;
    m->freed = 0;
    m->sum = 0;
    return 0;
}

/* EBUSY while a job is live or a created job has not yet been freed. */
int tl_monitor_destroy(struct tl_monitor *m)
{
    if (!m)
    {
        return EINVAL;
    }
    if (m->live_refs || (m->created && !m->freed))
    {
        return EBUSY;
    }
    int e = pthread_cond_destroy(&m->changed);
    if (e)
    {
        return e;
    }
    return pthread_mutex_destroy(&m->mutex);
}

/* Allocates a job holding a private copy of the creator's input. NULL means
 * allocation failure. The job is owned by the caller until a worker owns it. */
static struct tl_job *job_new(const struct tl_creator *c)
{
    struct tl_job *j = malloc(sizeof *j);
    if (!j)
    {
        return NULL;
    }
    j->monitor = c->monitor;
    j->length = c->length;
    for (size_t i = 0; i < c->length; i++)
    {
        j->bytes[i] = c->input[i];
    }
    return j;
}

/*
 * Starts the detached worker with live_refs = 2 (worker + creator) already
 * published, so neither side can free the job while the other still uses it.
 * On failure live_refs is back to 0 and the caller still owns the job.
 */
static int spawn_worker(struct tl_monitor *m, struct tl_job *j)
{
    pthread_attr_t attr;
    int e = pthread_attr_init(&attr);
    if (e)
    {
        return e;
    }
    e = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (!e)
    {
        invariant(pthread_mutex_lock(&m->mutex));
        m->live_refs = 2;
        pthread_t thread;
        e = pthread_create(&thread, &attr, detached_worker, j);
        if (e)
        {
            m->live_refs = 0;
        }
        else
        {
            m->created = 1;
        }
        invariant(pthread_mutex_unlock(&m->mutex));
    }
    invariant(pthread_attr_destroy(&attr));
    return e;
}

/*
 * Thread body for the creator. Sets c->error (0, EINVAL, ENOMEM or a
 * pthread_attr_* or pthread_create error) and returns NULL. If the thread is
 * canceled while waiting it does not return: it exits through the cleanup
 * handler and pthread_join then reports PTHREAD_CANCELED. Cancellation is
 * disabled except at the wait, where the cleanup handler releases the
 * creator's reference.
 */
void *tl_detached_creator(void *arg)
{
    struct tl_creator *c = arg;
    if (!c)
    {
        return NULL;
    }
    if (!c->monitor || c->length > JOB_MAX_BYTES || (!c->input && c->length) ||
        c->wait_for_completion < 0 || c->wait_for_completion > 1)
    {
        c->error = EINVAL;
        return NULL;
    }
    int previous;
    invariant(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous));
    struct tl_monitor *m = c->monitor;
    struct tl_job *j = job_new(c);
    int e = j ? spawn_worker(m, j) : ENOMEM;
    if (e)
    {
        free(j);
        c->error = e;
        invariant(pthread_setcancelstate(previous, NULL));
        return NULL;
    }
    c->error = 0;
    pthread_cleanup_push(creator_cleanup, j);
    invariant(pthread_mutex_lock(&m->mutex));
    m->creator_waiting = 1;
    invariant(pthread_cond_broadcast(&m->changed));
    invariant(pthread_setcancelstate(previous, NULL));
    while (c->wait_for_completion && !m->completed)
    {
        invariant(pthread_cond_wait(&m->changed, &m->mutex));
    }
    invariant(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
    pthread_cleanup_pop(1);
    invariant(pthread_setcancelstate(previous, NULL));
    return NULL;
}

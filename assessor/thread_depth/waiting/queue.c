#include "waiting.h"
#include <errno.h>
#include <stdlib.h>

/*
 * Locking protocol for struct tw_queue:
 *   root   guards state, references and active, and is taken first;
 *   mutex  guards the ring buffer and the wait statistics, and is taken after
 *          root when both are needed (never the other way round).
 * state: 0 open, 1 closed (no new pushes; pops drain), 2 retired (condition
 * variables and mutex destroyed; only root remains).
 * A waiter holds `active` from enter() until leave(), so retire cannot
 * destroy primitives under a thread that is still using them.
 */
enum
{
    QUEUE_CAPACITY = 3,
    INIT_STAGES = 5,
    REFERENCE_LIMIT = 16,
    STATE_OPEN = 0,
    STATE_CLOSED = 1,
    STATE_RETIRED = 2
};

#define NSEC_PER_SEC 1000000000L

/* A failed lock/wait/broadcast means broken harness state: abort. */
static void ok(int e)
{
    if (e)
    {
        abort();
    }
}

static int deadline_invalid(const struct timespec *deadline)
{
    return deadline &&
           (deadline->tv_nsec < 0 || deadline->tv_nsec >= NSEC_PER_SEC);
}

static void dispose_prefix(struct tw_queue *q, unsigned count,
                           struct tw_init *r)
{
    if (count >= 5)
    {
        ok(pthread_cond_destroy(&q->observer));
        r->destroyed++;
    }
    if (count >= 4)
    {
        ok(pthread_cond_destroy(&q->nonfull));
        r->destroyed++;
    }
    if (count >= 3)
    {
        ok(pthread_cond_destroy(&q->nonempty));
        r->destroyed++;
    }
    if (count >= 2)
    {
        ok(pthread_mutex_destroy(&q->mutex));
        r->destroyed++;
    }
    if (count >= 1)
    {
        ok(pthread_mutex_destroy(&q->root));
        r->destroyed++;
    }
}

/* Initializes primitive number `stage` (1 root, 2 mutex, 3-5 the conditions
 * in the order nonempty, nonfull, observer); `fail` injects EAGAIN there. */
static int init_stage(struct tw_queue *q, unsigned stage, unsigned fail,
                      const pthread_condattr_t *attr)
{
    if (stage == fail)
    {
        return EAGAIN;
    }
    if (stage == 1)
    {
        return pthread_mutex_init(&q->root, NULL);
    }
    if (stage == 2)
    {
        return pthread_mutex_init(&q->mutex, NULL);
    }
    if (stage == 3)
    {
        return pthread_cond_init(&q->nonempty, attr);
    }
    if (stage == 4)
    {
        return pthread_cond_init(&q->nonfull, attr);
    }
    return pthread_cond_init(&q->observer, attr);
}

/*
 * On failure at stage k, the k - 1 primitives already initialized are
 * destroyed in reverse order and the counts are reported in *r; the queue is
 * then unusable. Waits use CLOCK_MONOTONIC so deadlines are not affected by
 * wall-clock steps.
 */
int tw_queue_init(struct tw_queue *q, unsigned fail, struct tw_init *r)
{
    if (fail > INIT_STAGES)
    {
        return EINVAL;
    }
    *r = (struct tw_init){0};
    pthread_condattr_t attr;
    int e = pthread_condattr_init(&attr);
    if (e)
    {
        return e;
    }
    e = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (e)
    {
        ok(pthread_condattr_destroy(&attr));
        return e;
    }
    for (unsigned stage = 1; stage <= INIT_STAGES; stage++)
    {
        e = init_stage(q, stage, fail, &attr);
        if (e)
        {
            r->failed_stage = stage;
            dispose_prefix(q, r->initialized, r);
            ok(pthread_condattr_destroy(&attr));
            return e;
        }
        r->initialized++;
    }
    ok(pthread_condattr_destroy(&attr));
    q->state = STATE_OPEN;
    q->references = 1;
    q->active = 0;
    q->initialized = INIT_STAGES;
    q->wait_entries = 0;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->timed_waits = 0;
    q->deadline_changes = 0;
    q->last_deadline = (struct timespec){0};
    return 0;
}

int tw_ref(struct tw_queue *q)
{
    int e = 0;
    ok(pthread_mutex_lock(&q->root));
    if (q->state || q->references == 0)
    {
        e = EPIPE;
    }
    else if (q->references == REFERENCE_LIMIT)
    {
        e = EOVERFLOW;
    }
    else
    {
        q->references++;
    }
    ok(pthread_mutex_unlock(&q->root));
    return e;
}

/* Caller holds root. Wakes every waiter so each re-evaluates the state. */
static void close_locked(struct tw_queue *q)
{
    if (q->state == STATE_OPEN)
    {
        ok(pthread_mutex_lock(&q->mutex));
        q->state = STATE_CLOSED;
        ok(pthread_cond_broadcast(&q->nonempty));
        ok(pthread_cond_broadcast(&q->nonfull));
        ok(pthread_mutex_unlock(&q->mutex));
    }
}

int tw_unref(struct tw_queue *q)
{
    int e = 0;
    ok(pthread_mutex_lock(&q->root));
    if (!q->references)
    {
        e = EINVAL;
    }
    else
    {
        if (q->references == 1)
        {
            ok(pthread_mutex_lock(&q->mutex));
            if (q->count)
            {
                e = EBUSY;
            }
            ok(pthread_mutex_unlock(&q->mutex));
        }
        if (!e && --q->references == 0)
        {
            close_locked(q);
        }
    }
    ok(pthread_mutex_unlock(&q->root));
    return e;
}

int tw_close(struct tw_queue *q)
{
    ok(pthread_mutex_lock(&q->root));
    close_locked(q);
    ok(pthread_mutex_unlock(&q->root));
    return 0;
}

int tw_retire(struct tw_queue *q)
{
    int e = 0;
    ok(pthread_mutex_lock(&q->root));
    if (q->state != STATE_CLOSED || q->active || q->references)
    {
        e = EBUSY;
    }
    else
    {
        ok(pthread_mutex_lock(&q->mutex));
        int empty = q->count == 0;
        ok(pthread_mutex_unlock(&q->mutex));
        if (!empty)
        {
            e = EBUSY;
        }
        else
        {
            ok(pthread_cond_destroy(&q->observer));
            ok(pthread_cond_destroy(&q->nonfull));
            ok(pthread_cond_destroy(&q->nonempty));
            ok(pthread_mutex_destroy(&q->mutex));
            q->state = STATE_RETIRED;
            q->initialized = 1;
        }
    }
    ok(pthread_mutex_unlock(&q->root));
    return e;
}

void tw_root_destroy(struct tw_queue *q)
{
    if (q->state != STATE_RETIRED || q->references || q->active)
    {
        abort();
    }
    ok(pthread_mutex_destroy(&q->root));
    q->initialized = 0;
}

/*
 * Registers the caller as active and returns with q->mutex held (result 0),
 * or returns EPIPE holding nothing. Writers are refused once closed; readers
 * may still drain a closed queue.
 */
static int enter(struct tw_queue *q, int writing)
{
    int e = 0;
    ok(pthread_mutex_lock(&q->root));
    if (q->state == STATE_RETIRED || !q->references || (writing && q->state))
    {
        e = EPIPE;
    }
    else
    {
        q->active++;
    }
    ok(pthread_mutex_unlock(&q->root));
    if (!e)
    {
        ok(pthread_mutex_lock(&q->mutex));
    }
    return e;
}

/* Undoes enter(): releases q->mutex and the active count. Also used as the
 * cancellation cleanup handler, so it disables cancellation first. State is
 * read with core held and changed with root+core held. */
static void leave(void *arg)
{
    struct tw_queue *q = arg;
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    ok(pthread_mutex_unlock(&q->mutex));
    ok(pthread_mutex_lock(&q->root));
    if (!q->active)
    {
        abort();
    }
    q->active--;
    ok(pthread_mutex_unlock(&q->root));
}

static int wait_cond(struct tw_queue *q, pthread_cond_t *cond,
                     const struct timespec *deadline)
{
    q->wait_entries++;
    ok(pthread_cond_broadcast(&q->observer));
    if (deadline)
    {
        if (q->timed_waits && (deadline->tv_sec != q->last_deadline.tv_sec ||
                               deadline->tv_nsec != q->last_deadline.tv_nsec))
        {
            q->deadline_changes++;
        }
        q->last_deadline = *deadline;
        q->timed_waits++;
        return pthread_cond_timedwait(cond, &q->mutex, deadline);
    }
    return pthread_cond_wait(cond, &q->mutex);
}

int tw_push(struct tw_queue *q, int value, const struct timespec *deadline)
{
    if (deadline_invalid(deadline))
    {
        return EINVAL;
    }
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    int e = enter(q, 1);
    if (e)
    {
        ok(pthread_setcancelstate(old, NULL));
        return e;
    }
    pthread_cleanup_push(leave, q);
    ok(pthread_setcancelstate(old, NULL));
    while (q->count == QUEUE_CAPACITY && !q->state)
    {
        e = wait_cond(q, &q->nonfull, deadline);
        if (e == ETIMEDOUT)
        {
            break;
        }
        ok(e);
    }
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
    if (q->state)
    {
        e = EPIPE;
    }
    else if (q->count == QUEUE_CAPACITY)
    {
        e = ETIMEDOUT;
    }
    else
    {
        q->values[q->tail] = value;
        q->tail = (q->tail + 1) % QUEUE_CAPACITY;
        q->count++;
        e = 0;
        ok(pthread_cond_broadcast(&q->nonempty));
    }
    pthread_cleanup_pop(1);
    ok(pthread_setcancelstate(old, NULL));
    return e;
}

int tw_pop(struct tw_queue *q, int *out, const struct timespec *deadline,
           tw_hook_fn hook, void *arg)
{
    if (deadline_invalid(deadline))
    {
        return EINVAL;
    }
    tw_hook_fn volatile pending = hook;
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    int e = enter(q, 0);
    if (e)
    {
        ok(pthread_setcancelstate(old, NULL));
        return e;
    }
    pthread_cleanup_push(leave, q);
    ok(pthread_setcancelstate(old, NULL));
    while (!q->count && !q->state)
    {
        e = wait_cond(q, &q->nonempty, deadline);
        if (e == ETIMEDOUT)
        {
            break;
        }
        ok(e);
        if (pending)
        {
            int previous;
            ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous));
            ok(pthread_mutex_unlock(&q->mutex));
            pending(arg);
            ok(pthread_mutex_lock(&q->mutex));
            pending = NULL;
            ok(pthread_setcancelstate(previous, NULL));
        }
#ifdef TW_MUTANT5
        break;
#endif
    }
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
    if (q->count)
    {
        *out = q->values[q->head];
        q->head = (q->head + 1) % QUEUE_CAPACITY;
        q->count--;
        e = 0;
        ok(pthread_cond_broadcast(&q->nonfull));
    }
    else if (q->state)
    {
        e = EPIPE;
    }
    else
    {
        e = ETIMEDOUT;
    }
#ifdef TW_MUTANT4
    if (!e)
    {
        *out += 1;
    }
#endif
#ifdef TW_MUTANT6
    if (e == ETIMEDOUT)
    {
        e = EPIPE;
    }
#endif
    pthread_cleanup_pop(1);
    ok(pthread_setcancelstate(old, NULL));
    return e;
}

/* Wakes poppers without changing any state (a spurious-wakeup generator). */
int tw_noise(struct tw_queue *q)
{
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    int e = enter(q, 0);
    if (!e)
    {
        ok(pthread_cond_broadcast(&q->nonempty));
        leave(q);
    }
    ok(pthread_setcancelstate(old, NULL));
    return e;
}

/* Test synchronization: blocks until n waits have begun on this queue. */
int tw_wait_entries(struct tw_queue *q, unsigned n)
{
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    int e = enter(q, 0);
    if (!e)
    {
        while (q->wait_entries < n)
        {
            ok(pthread_cond_wait(&q->observer, &q->mutex));
        }
        leave(q);
    }
    ok(pthread_setcancelstate(old, NULL));
    return e;
}

int tw_counts(struct tw_queue *q, unsigned *refs, unsigned *active,
              size_t *count)
{
    ok(pthread_mutex_lock(&q->root));
    *refs = q->references;
    *active = q->active;
#ifdef TW_MUTANT3
    *refs = 0;
#endif
    if (q->state == STATE_RETIRED)
    {
        *count = 0;
    }
    else
    {
        ok(pthread_mutex_lock(&q->mutex));
        *count = q->count;
        ok(pthread_mutex_unlock(&q->mutex));
    }
    ok(pthread_mutex_unlock(&q->root));
    return 0;
}

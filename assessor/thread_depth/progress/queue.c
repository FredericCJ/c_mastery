#include "queue.h"
#include <errno.h>
#include <stdlib.h>

enum
{
    MAX_INFLIGHT = 8,
    ACCEPT_LIMIT = 16000,
    VALUE_LIMIT = 1000000
};

/*
 * Shutdown-capable wrapper around struct tw_queue. `gate` guards closing,
 * inflight and the counters; an operation is admitted by enter() and counted
 * out by leave(), and tp_queue_abort waits on `quiet` until inflight is 0.
 * A failed lock/wait/broadcast is a harness bug and aborts.
 */
static void ok(int e)
{
    if (e)
    {
        abort();
    }
}

int tp_queue_init(struct tp_end_queue *q)
{
    int e = pthread_mutex_init(&q->gate, NULL);
    if (e)
    {
        return e;
    }
    e = pthread_cond_init(&q->quiet, NULL);
    if (e)
    {
        ok(pthread_mutex_destroy(&q->gate));
        return e;
    }
    struct tw_init report;
    e = tw_queue_init(&q->queue, 0, &report);
    if (e)
    {
        ok(pthread_cond_destroy(&q->quiet));
        ok(pthread_mutex_destroy(&q->gate));
        return e;
    }
    q->closing = 0;
    q->inflight = 0;
    q->retired = 0;
    q->accepted = 0;
    q->delivered = 0;
    q->discarded = 0;
    q->discarded_sum = 0;
    return 0;
}

/* Admits one operation: ECANCELED once closing, EOVERFLOW at the lifetime
 * accept limit, EAGAIN at MAX_INFLIGHT concurrent operations. */
static int enter(struct tp_end_queue *q, int writing)
{
    ok(pthread_mutex_lock(&q->gate));
    int e = 0;
    if (q->closing)
    {
        e = ECANCELED;
    }
    else if (writing && q->accepted + q->inflight >= ACCEPT_LIMIT)
    {
        e = EOVERFLOW;
    }
    else if (q->inflight == MAX_INFLIGHT)
    {
        e = EAGAIN;
    }
    else
    {
        q->inflight++;
    }
    ok(pthread_mutex_unlock(&q->gate));
    return e;
}

static void leave(struct tp_end_queue *q, int e, int write)
{
    ok(pthread_mutex_lock(&q->gate));
    if (!e)
    {
        if (write)
        {
            q->accepted++;
        }
        else
        {
            q->delivered++;
        }
    }
    q->inflight--;
    ok(pthread_cond_broadcast(&q->quiet));
    ok(pthread_mutex_unlock(&q->gate));
}

int tp_queue_push(struct tp_end_queue *q, int value)
{
    if (value < 0 || value > VALUE_LIMIT)
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
    e = tw_push(&q->queue, value, NULL);
    leave(q, e, 1);
    ok(pthread_setcancelstate(old, NULL));
    return e;
}

int tp_queue_pop(struct tp_end_queue *q, int *out)
{
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    int e = enter(q, 0);
    if (e)
    {
        ok(pthread_setcancelstate(old, NULL));
        return e;
    }
    e = tw_pop(&q->queue, out, NULL, NULL, NULL);
    leave(q, e, 0);
    ok(pthread_setcancelstate(old, NULL));
    return e;
}

/* Exactly one caller wins; later callers get EALREADY. Cancellation is
 * disabled throughout so the shutdown always runs to completion. */
int tp_queue_abort(struct tp_end_queue *q)
{
    int old;
    ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
    ok(pthread_mutex_lock(&q->gate));
    if (q->closing)
    {
        ok(pthread_mutex_unlock(&q->gate));
        ok(pthread_setcancelstate(old, NULL));
        return EALREADY;
    }
    q->closing = 1;
    ok(pthread_mutex_unlock(&q->gate));
    ok(tw_close(&q->queue));
    ok(pthread_mutex_lock(&q->gate));
    while (q->inflight)
    {
        ok(pthread_cond_wait(&q->quiet, &q->gate));
    }
    ok(pthread_mutex_unlock(&q->gate));
    int value;
    int e = tw_pop(&q->queue, &value, NULL, NULL, NULL);
    while (e == 0)
    {
        q->discarded++;
        q->discarded_sum += (unsigned)value;
        e = tw_pop(&q->queue, &value, NULL, NULL, NULL);
    }
    if (e != EPIPE)
    {
        abort();
    }
#ifdef TP_MUTANT_SHUTDOWN
    q->discarded = 0;
#endif
    ok(tw_unref(&q->queue));
    ok(tw_retire(&q->queue));
    tw_root_destroy(&q->queue);
    q->retired = 1;
    ok(pthread_setcancelstate(old, NULL));
    return 0;
}

void tp_queue_destroy(struct tp_end_queue *q)
{
    if (!q->retired || q->inflight)
    {
        abort();
    }
    ok(pthread_cond_destroy(&q->quiet));
    ok(pthread_mutex_destroy(&q->gate));
}

#include "queue.h"
#include <errno.h>
#include <stdlib.h>

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
    q->closing = q->inflight = q->retired = q->accepted = q->delivered =
        q->discarded = 0;
    q->discarded_sum = 0;
    return 0;
}

static int enter(struct tp_end_queue *q, int writing)
{
    ok(pthread_mutex_lock(&q->gate));
    int e = 0;
    if (q->closing)
    {
        e = ECANCELED;
    }
    else if (writing && q->accepted + q->inflight >= 16000)
    {
        e = EOVERFLOW;
    }
    else if (q->inflight == 8)
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
    if (value < 0 || value > 1000000)
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
    int value, e;
    while ((e = tw_pop(&q->queue, &value, NULL, NULL, NULL)) == 0)
    {
        q->discarded++;
        q->discarded_sum += (unsigned)value;
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

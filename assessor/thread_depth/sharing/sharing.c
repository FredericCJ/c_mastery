#define _POSIX_C_SOURCE 200809L
#include "sharing.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void checked(int e)
{
    if (e)
        abort();
}

/* Fixed stack request keeps workload configurations comparable; guards/TLS are separate. */
static int launch(void *(*worker)(void *), void *args, size_t stride,
                  unsigned n)
{
    pthread_attr_t attr;
    int e = pthread_attr_init(&attr);
    if (e)
        return e;
    e = pthread_attr_setstacksize(&attr, 256u * 1024u);
    if (e)
    {
        checked(pthread_attr_destroy(&attr));
        return e;
    }
    pthread_t threads[4];
    unsigned made = 0;
    for (; made < n; made++)
    {
        e = pthread_create(&threads[made], &attr, worker,
                           (unsigned char *)args + made * stride);
        if (e)
            break;
    }
    checked(pthread_attr_destroy(&attr));
    for (unsigned i = 0; i < made; i++)
        checked(pthread_join(threads[i], NULL));
    return e;
}

struct sum_arg
{
    const int32_t *input;
    size_t begin, end;
    int shared;
    int64_t local, *total;
    pthread_mutex_t *mutex;
};

static void *sum_worker(void *arg)
{
    struct sum_arg *a = arg;
    for (size_t i = a->begin; i < a->end; i++)
    {
        if (a->shared)
        {
            checked(pthread_mutex_lock(a->mutex));
            *a->total += a->input[i];
            checked(pthread_mutex_unlock(a->mutex));
        }
        else
            a->local += a->input[i];
    }
    return NULL;
}

int ts_sum(const int32_t *p, size_t n, unsigned workers, int shared,
           int64_t *out)
{
    if ((!p && n) || n > 16 || workers < 1 || workers > 4 || shared < 0 ||
        shared > 1 || !out)
        return EINVAL;
    pthread_mutex_t mutex;
    int e = pthread_mutex_init(&mutex, NULL);
    if (e)
        return e;
    struct sum_arg args[4];
    int64_t total = 0;
    for (unsigned i = 0; i < workers; i++)
        args[i] = (struct sum_arg){
            p,      n * i / workers, n * (i + 1) / workers, shared, 0,
            &total, &mutex};
#ifdef TS_MUTANT1
    if (n)
        args[workers - 1].end--;
#endif
    e = launch(sum_worker, args, sizeof args[0], workers);
    checked(pthread_mutex_destroy(&mutex));
    if (e)
        return e;
    if (!shared)
        for (unsigned i = 0; i < workers; i++)
            total += args[i].local;
    *out = total;
    return 0;
}

int ts_snapshot_sum(struct ts_input *p, unsigned workers,
                    ts_snapshot_hook_fn hook, void *context, int64_t *out,
                    unsigned *version)
{
    if (!p || !out || !version || workers < 1 || workers > 4)
        return EINVAL;
    int32_t local[16];
    checked(pthread_mutex_lock(&p->mutex));
    size_t n = p->length;
    if (n > 16)
    {
        checked(pthread_mutex_unlock(&p->mutex));
        return EINVAL;
    }
    unsigned v = p->version;
    for (size_t i = 0; i < n; i++)
        local[i] = p->values[i];
    checked(pthread_mutex_unlock(&p->mutex));
    if (hook)
        hook(context);
    int64_t sum;
    int e = ts_sum(local, n, workers, 0, &sum);
    if (e)
        return e;
    *out = sum;
    *version = v;
    return 0;
}

int ts_queue_init(struct ts_queue *q)
{
    if (!q)
        return EINVAL;
    memset(q, 0, sizeof *q);
    return pthread_mutex_init(&q->mutex, NULL);
}

int ts_enqueue(struct ts_queue *q, unsigned id)
{
    if (!q || id >= 8)
        return EINVAL;
    checked(pthread_mutex_lock(&q->mutex));
    int e = 0;
    if (q->state[id] != TS_CALLER)
        e = EALREADY;
    else if (q->closed)
        e = EPIPE;
    else if (q->count == 2)
        e = EAGAIN;
    if (!e)
    {
        q->ring[(q->head + q->count) % 2] = id;
        q->count++;
        q->state[id] = TS_QUEUED;
    }
#ifdef TS_MUTANT2
    else if (e == EAGAIN)
        q->state[id] = TS_QUEUED;
#endif
    checked(pthread_mutex_unlock(&q->mutex));
    return e;
}

int ts_dequeue(struct ts_queue *q, unsigned *out)
{
    if (!q || !out)
        return EINVAL;
    checked(pthread_mutex_lock(&q->mutex));
    int e = 0;
    if (!q->count)
        e = q->closed ? EPIPE : EAGAIN;
    else
    {
        unsigned id = q->ring[q->head];
        q->head = (q->head + 1) % 2;
        q->count--;
        q->state[id] = TS_WORKER;
        *out = id;
    }
    checked(pthread_mutex_unlock(&q->mutex));
    return e;
}

int ts_settle(struct ts_queue *q, unsigned id, unsigned status)
{
    if (!q || id >= 8 || (status != TS_SUCCESS && status != TS_FAILED))
        return EINVAL;
    checked(pthread_mutex_lock(&q->mutex));
    int e = q->state[id] == TS_WORKER ? 0 : EALREADY;
    if (!e)
    {
        q->state[id] = TS_SETTLED;
        q->terminal[id] = status;
        q->settlements[id]++;
    }
    checked(pthread_mutex_unlock(&q->mutex));
    return e;
}

int ts_queue_close(struct ts_queue *q)
{
    if (!q)
        return EINVAL;
    checked(pthread_mutex_lock(&q->mutex));
    q->closed = 1;
    while (q->count)
    {
        unsigned id = q->ring[q->head];
        q->head = (q->head + 1) % 2;
        q->count--;
        q->state[id] = TS_SETTLED;
        q->terminal[id] = TS_CANCELED;
        q->settlements[id]++;
    }
    checked(pthread_mutex_unlock(&q->mutex));
    return 0;
}

int ts_queue_inspect(struct ts_queue *q, unsigned id, unsigned *state,
                     unsigned *terminal, unsigned *settlements)
{
    if (!q || id >= 8 || !state || !terminal || !settlements)
        return EINVAL;
    checked(pthread_mutex_lock(&q->mutex));
    *state = q->state[id];
    *terminal = q->terminal[id];
    *settlements = q->settlements[id];
    checked(pthread_mutex_unlock(&q->mutex));
    return 0;
}

int ts_queue_destroy(struct ts_queue *q)
{
    if (!q)
        return EINVAL;
    for (unsigned i = 0; i < 8; i++)
        if (q->state[i] == TS_QUEUED || q->state[i] == TS_WORKER)
            return EBUSY;
    return pthread_mutex_destroy(&q->mutex);
}

struct hist_arg
{
    const uint32_t *input;
    size_t begin, end, bins;
    unsigned mode, index;
    uint64_t *hist;
    pthread_mutex_t *mutex;
};

static void *hist_worker(void *arg)
{
    struct hist_arg *a = arg;
    uint32_t keys[64];
    uint64_t counts[64];
    size_t used = 0;
    for (size_t i = a->begin; i < a->end; i++)
    {
        uint32_t key = a->input[i];
        if (a->mode == TS_PRIVATE)
        {
            a->hist[a->bins * a->index + key]++;
            continue;
        }
        if (a->mode == TS_BATCHED)
        {
            size_t j = 0;
            while (j < used && keys[j] != key)
                j++;
            if (j == used)
            {
                keys[j] = key;
                counts[j] = 0;
                used++;
            }
            counts[j]++;
            if (used < 64 && i + 1 < a->end)
                continue;
            for (size_t k = 0; k < used; k++)
            {
                unsigned shard = keys[k] % 4;
                checked(pthread_mutex_lock(&a->mutex[shard]));
                a->hist[keys[k]] += counts[k];
                checked(pthread_mutex_unlock(&a->mutex[shard]));
            }
            used = 0;
            continue;
        }
        unsigned shard = a->mode == TS_SHARED ? 0 : key % 4;
        checked(pthread_mutex_lock(&a->mutex[shard]));
        a->hist[key]++;
        checked(pthread_mutex_unlock(&a->mutex[shard]));
    }
    return NULL;
}

int ts_histogram(const uint32_t *input, size_t n, size_t bins, unsigned mode,
                 size_t budget, uint64_t *out, struct ts_resources *resources)
{
    if ((!input && n) || n > 262144 || !bins || bins > 65536 ||
        mode > TS_BATCHED || !out || !resources)
        return EINVAL;
    for (size_t i = 0; i < n; i++)
        if (input[i] >= bins)
            return EINVAL;
    size_t copies = mode == TS_PRIVATE ? 4 : 1;
    size_t bytes = n * sizeof *input + (copies + 1) * bins * sizeof *out;
    if (bytes > budget)
        return ENOMEM;
    uint64_t *hist = calloc(copies * bins, sizeof *hist);
    if (!hist)
        return ENOMEM;
    pthread_mutex_t mutex[4];
    unsigned initialized = 0;
    int e = 0;
    for (; initialized < 4; initialized++)
    {
        e = pthread_mutex_init(&mutex[initialized], NULL);
        if (e)
            break;
    }
    if (!e)
    {
        struct hist_arg args[4];
        for (unsigned i = 0; i < 4; i++)
            args[i] = (struct hist_arg){
                input, n * i / 4, n * (i + 1) / 4, bins, mode, i, hist, mutex};
        e = launch(hist_worker, args, sizeof args[0], 4);
    }
    if (!e)
    {
        for (size_t b = 0; b < bins; b++)
        {
            uint64_t value = 0;
            size_t limit = copies;
#ifdef TS_MUTANT3
            limit = 1;
#endif
            for (size_t c = 0; c < limit; c++)
                value += hist[c * bins + b];
            out[b] = value;
        }
        *resources = (struct ts_resources){bytes, 4, 4u * 256u * 1024u};
    }
    for (unsigned i = 0; i < initialized; i++)
        checked(pthread_mutex_destroy(&mutex[i]));
    free(hist);
    return e;
}

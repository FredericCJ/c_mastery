/* Reference implementation of the sharing laboratory (see CONTRACTS.md).
 *
 * Every pthread call that cannot fail for a correct program (lock, unlock,
 * join, destroy of an initialized object) is wrapped by checked(): a nonzero
 * result means the harness itself is broken, so it aborts instead of
 * returning an error the caller could mistake for a contract result.
 */
#define _POSIX_C_SOURCE 200809L
#include "sharing.h"
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Upper bounds shared by the public entry points. */
enum
{
    SUM_MAX_ELEMENTS = 16,
    MAX_WORKERS = 4,
    QUEUE_IDS = 8,
    QUEUE_CAPACITY = 2,
    HISTOGRAM_MAX_INPUT = 262144,
    HISTOGRAM_MAX_BINS = 65536,
    HISTOGRAM_SHARDS = 4,
    HISTOGRAM_BATCH_KEYS = 64,
    WORKER_STACK_BYTES = 256 * 1024
};

static void checked(int error)
{
    if (error)
    {
        abort();
    }
}

/*
 * Starts n workers (n <= MAX_WORKERS) and joins every one that started.
 *
 * Worker i receives (char *)args + i * stride_bytes. Every thread gets the
 * same fixed stack request so workload configurations stay comparable; guard
 * pages and TLS are excluded from that request and reported separately.
 * Returns 0, or the first pthread error. On a create failure the workers that
 * did start are still joined, so no thread outlives this call.
 */
static int launch(void *(*worker)(void *), void *args, size_t stride_bytes,
                  unsigned n)
{
    pthread_attr_t attr;
    int error = pthread_attr_init(&attr);
    if (error)
    {
        return error;
    }
    error = pthread_attr_setstacksize(&attr, WORKER_STACK_BYTES);
    if (error)
    {
        checked(pthread_attr_destroy(&attr));
        return error;
    }
    pthread_t threads[MAX_WORKERS];
    unsigned started = 0;
    for (; started < n; started++)
    {
        unsigned char *worker_args = (unsigned char *)args;
        worker_args += started * stride_bytes;
        error = pthread_create(&threads[started], &attr, worker, worker_args);
        if (error)
        {
            break;
        }
    }
    checked(pthread_attr_destroy(&attr));
    for (unsigned i = 0; i < started; i++)
    {
        checked(pthread_join(threads[i], NULL));
    }
    return error;
}

struct sum_arg
{
    const int32_t *input;
    size_t begin;
    size_t end;
    int shared;
    int64_t local;
    int64_t *total;
    pthread_mutex_t *mutex;
};

/* shared != 0: add into *total under the mutex; otherwise into a->local. */
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
        {
            a->local += a->input[i];
        }
    }
    return NULL;
}

/* Gives worker i the half-open range [n*i/workers, n*(i+1)/workers). */
static void partition_sum(struct sum_arg *args, const int32_t *input, size_t n,
                          unsigned workers, int shared, int64_t *total,
                          pthread_mutex_t *mutex)
{
    for (unsigned i = 0; i < workers; i++)
    {
        args[i].input = input;
        args[i].begin = n * i / workers;
        args[i].end = n * (i + 1) / workers;
        args[i].shared = shared;
        args[i].local = 0;
        args[i].total = total;
        args[i].mutex = mutex;
    }
}

int ts_sum(const int32_t *p, size_t n, unsigned workers, int shared,
           int64_t *out)
{
    bool bad_input = (!p && n) || n > SUM_MAX_ELEMENTS;
    bool bad_workers = workers < 1 || workers > MAX_WORKERS;
    bool bad_mode = shared < 0 || shared > 1;
    if (bad_input || bad_workers || bad_mode || !out)
    {
        return EINVAL;
    }
    pthread_mutex_t mutex;
    int error = pthread_mutex_init(&mutex, NULL);
    if (error)
    {
        return error;
    }
    struct sum_arg args[MAX_WORKERS];
    int64_t total = 0;
    partition_sum(args, p, n, workers, shared, &total, &mutex);
#ifdef TS_MUTANT1
    if (n)
    {
        args[workers - 1].end--;
    }
#endif
    error = launch(sum_worker, args, sizeof args[0], workers);
    checked(pthread_mutex_destroy(&mutex));
    if (error)
    {
        return error;
    }
    if (!shared)
    {
        for (unsigned i = 0; i < workers; i++)
        {
            total += args[i].local;
        }
    }
    *out = total;
    return 0;
}

/*
 * Copies the input under its mutex, then sums the private copy with no lock
 * held, so a writer arriving during hook() cannot tear the result.
 * *version is the input version at copy time. *out and *version are written
 * only on success.
 */
int ts_snapshot_sum(struct ts_input *p, unsigned workers,
                    ts_snapshot_hook_fn hook, void *context, int64_t *out,
                    unsigned *version)
{
    if (!p || !out || !version || workers < 1 || workers > MAX_WORKERS)
    {
        return EINVAL;
    }
    int32_t local[SUM_MAX_ELEMENTS];
    checked(pthread_mutex_lock(&p->mutex));
    size_t n = p->length;
    if (n > SUM_MAX_ELEMENTS)
    {
        checked(pthread_mutex_unlock(&p->mutex));
        return EINVAL;
    }
    unsigned copied_version = p->version;
    for (size_t i = 0; i < n; i++)
    {
        local[i] = p->values[i];
    }
    checked(pthread_mutex_unlock(&p->mutex));
    if (hook)
    {
        hook(context);
    }
    int64_t sum;
    int error = ts_sum(local, n, workers, 0, &sum);
    if (error)
    {
        return error;
    }
    *out = sum;
    *version = copied_version;
    return 0;
}

int ts_queue_init(struct ts_queue *q)
{
    if (!q)
    {
        return EINVAL;
    }
    memset(q, 0, sizeof *q);
    return pthread_mutex_init(&q->mutex, NULL);
}

/*
 * Ownership of an id: caller -> queued (ts_enqueue) -> worker (ts_dequeue)
 * -> settled (ts_settle). ts_queue_close settles only ids still queued; an id
 * held by a worker is settled only by that worker's ts_settle. Each accepted
 * id is settled exactly once; every transition below happens under q->mutex.
 */
int ts_enqueue(struct ts_queue *q, unsigned id)
{
    if (!q || id >= QUEUE_IDS)
    {
        return EINVAL;
    }
    checked(pthread_mutex_lock(&q->mutex));
    int error = 0;
    if (q->state[id] != TS_CALLER)
    {
        error = EALREADY;
    }
    else if (q->closed)
    {
        error = EPIPE;
    }
    else if (q->count == QUEUE_CAPACITY)
    {
        error = EAGAIN;
    }
    if (!error)
    {
        q->ring[(q->head + q->count) % QUEUE_CAPACITY] = id;
        q->count++;
        q->state[id] = TS_QUEUED;
    }
#ifdef TS_MUTANT2
    else if (error == EAGAIN)
    {
        q->state[id] = TS_QUEUED;
    }
#endif
    checked(pthread_mutex_unlock(&q->mutex));
    return error;
}

int ts_dequeue(struct ts_queue *q, unsigned *out)
{
    if (!q || !out)
    {
        return EINVAL;
    }
    checked(pthread_mutex_lock(&q->mutex));
    int error = 0;
    if (!q->count)
    {
        error = q->closed ? EPIPE : EAGAIN;
    }
    else
    {
        unsigned id = q->ring[q->head];
        q->head = (q->head + 1) % QUEUE_CAPACITY;
        q->count--;
        q->state[id] = TS_WORKER;
        *out = id;
    }
    checked(pthread_mutex_unlock(&q->mutex));
    return error;
}

int ts_settle(struct ts_queue *q, unsigned id, unsigned status)
{
    bool bad_status = status != TS_SUCCESS && status != TS_FAILED;
    if (!q || id >= QUEUE_IDS || bad_status)
    {
        return EINVAL;
    }
    checked(pthread_mutex_lock(&q->mutex));
    int error = q->state[id] == TS_WORKER ? 0 : EALREADY;
    if (!error)
    {
        q->state[id] = TS_SETTLED;
        q->terminal[id] = status;
        q->settlements[id]++;
    }
    checked(pthread_mutex_unlock(&q->mutex));
    return error;
}

/* Closes admission and cancels every id still queued; idempotent. */
int ts_queue_close(struct ts_queue *q)
{
    if (!q)
    {
        return EINVAL;
    }
    checked(pthread_mutex_lock(&q->mutex));
    q->closed = 1;
    while (q->count)
    {
        unsigned id = q->ring[q->head];
        q->head = (q->head + 1) % QUEUE_CAPACITY;
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
    if (!q || id >= QUEUE_IDS || !state || !terminal || !settlements)
    {
        return EINVAL;
    }
    checked(pthread_mutex_lock(&q->mutex));
    *state = q->state[id];
    *terminal = q->terminal[id];
    *settlements = q->settlements[id];
    checked(pthread_mutex_unlock(&q->mutex));
    return 0;
}

/* EBUSY while any id is queued or held by a worker; the queue stays usable. */
int ts_queue_destroy(struct ts_queue *q)
{
    if (!q)
    {
        return EINVAL;
    }
    for (unsigned id = 0; id < QUEUE_IDS; id++)
    {
        if (q->state[id] == TS_QUEUED || q->state[id] == TS_WORKER)
        {
            return EBUSY;
        }
    }
    return pthread_mutex_destroy(&q->mutex);
}

struct hist_arg
{
    const uint32_t *input;
    size_t begin;
    size_t end;
    size_t bins;
    unsigned mode;
    unsigned index;
    uint64_t *hist;
    pthread_mutex_t *mutex;
};

/* Adds each batched (key, count) pair into the shared histogram under the
 * mutex of the key's shard. */
static void flush_batch(const struct hist_arg *a, const uint32_t *keys,
                        const uint64_t *counts, size_t used)
{
    for (size_t k = 0; k < used; k++)
    {
        unsigned shard = keys[k] % HISTOGRAM_SHARDS;
        checked(pthread_mutex_lock(&a->mutex[shard]));
        a->hist[keys[k]] += counts[k];
        checked(pthread_mutex_unlock(&a->mutex[shard]));
    }
}

/*
 * One worker's slice of the histogram. Modes differ only in where an
 * increment lands:
 *   TS_PRIVATE  a per-worker copy of the bins, no lock;
 *   TS_SHARED   the single shared copy under shard 0's mutex;
 *   TS_SHARDED  the shared copy under the mutex of key % HISTOGRAM_SHARDS;
 *   TS_BATCHED  up to HISTOGRAM_BATCH_KEYS distinct keys per worker, in any
 *               order, are counted locally and flushed when the local table
 *               is full or the slice ends.
 */
static void *hist_worker(void *arg)
{
    struct hist_arg *a = arg;
    uint32_t keys[HISTOGRAM_BATCH_KEYS];
    uint64_t counts[HISTOGRAM_BATCH_KEYS];
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
            size_t slot = 0;
            while (slot < used && keys[slot] != key)
            {
                slot++;
            }
            if (slot == used)
            {
                keys[slot] = key;
                counts[slot] = 0;
                used++;
            }
            counts[slot]++;
            bool table_full = used == HISTOGRAM_BATCH_KEYS;
            bool last_item = i + 1 == a->end;
            if (table_full || last_item)
            {
                flush_batch(a, keys, counts, used);
                used = 0;
            }
            continue;
        }
        unsigned shard = a->mode == TS_SHARED ? 0 : key % HISTOGRAM_SHARDS;
        checked(pthread_mutex_lock(&a->mutex[shard]));
        a->hist[key]++;
        checked(pthread_mutex_unlock(&a->mutex[shard]));
    }
    return NULL;
}

/* Returns 0 when every argument is acceptable, else EINVAL. */
static int check_histogram_input(const uint32_t *input, size_t n, size_t bins,
                                 unsigned mode)
{
    bool bad_input = (!input && n) || n > HISTOGRAM_MAX_INPUT;
    bool bad_bins = !bins || bins > HISTOGRAM_MAX_BINS;
    if (bad_input || bad_bins || mode > TS_BATCHED)
    {
        return EINVAL;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (input[i] >= bins)
        {
            return EINVAL;
        }
    }
    return 0;
}

/* Sums the per-worker copies of each bin into out[bins]. */
static void merge_copies(uint64_t *out, const uint64_t *hist, size_t bins,
                         size_t copies)
{
    for (size_t bin = 0; bin < bins; bin++)
    {
        uint64_t value = 0;
        size_t limit = copies;
#ifdef TS_MUTANT3
        limit = 1;
#endif
        for (size_t copy = 0; copy < limit; copy++)
        {
            value += hist[copy * bins + bin];
        }
        out[bin] = value;
    }
}

/*
 * *out and *resources are written only when 0 is returned, so a rejected
 * budget leaves the caller's buffers untouched. requested_buffer_bytes is
 * the input plus (copies + 1) bin arrays of uint64_t, the extra array being
 * the caller's output. The heap copy and the mutexes are released on every
 * path.
 */
int ts_histogram(const uint32_t *input, size_t n, size_t bins, unsigned mode,
                 size_t budget_bytes, uint64_t *out,
                 struct ts_resources *resources)
{
    if (!out || !resources)
    {
        return EINVAL;
    }
    int error = check_histogram_input(input, n, bins, mode);
    if (error)
    {
        return error;
    }
    size_t copies = mode == TS_PRIVATE ? MAX_WORKERS : 1;
    size_t bytes = n * sizeof *input + (copies + 1) * bins * sizeof *out;
    if (bytes > budget_bytes)
    {
        return ENOMEM;
    }
    uint64_t *hist = calloc(copies * bins, sizeof *hist);
    if (!hist)
    {
        return ENOMEM;
    }
    pthread_mutex_t mutex[HISTOGRAM_SHARDS];
    unsigned initialized = 0;
    for (; initialized < HISTOGRAM_SHARDS; initialized++)
    {
        error = pthread_mutex_init(&mutex[initialized], NULL);
        if (error)
        {
            break;
        }
    }
    if (!error)
    {
        struct hist_arg args[MAX_WORKERS];
        for (unsigned i = 0; i < MAX_WORKERS; i++)
        {
            args[i].input = input;
            args[i].begin = n * i / MAX_WORKERS;
            args[i].end = n * (i + 1) / MAX_WORKERS;
            args[i].bins = bins;
            args[i].mode = mode;
            args[i].index = i;
            args[i].hist = hist;
            args[i].mutex = mutex;
        }
        error = launch(hist_worker, args, sizeof args[0], MAX_WORKERS);
    }
    if (!error)
    {
        merge_copies(out, hist, bins, copies);
        *resources = (struct ts_resources){bytes, MAX_WORKERS,
                                           MAX_WORKERS * WORKER_STACK_BYTES};
    }
    for (unsigned i = 0; i < initialized; i++)
    {
        checked(pthread_mutex_destroy(&mutex[i]));
    }
    free(hist);
    return error;
}

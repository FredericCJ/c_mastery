#ifndef TS_SHARING_H
#define TS_SHARING_H
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
/* ts_sum(values, count, workers, shared, out): sums count <= 16 values with
 * 1..4 workers; shared selects one mutex-guarded total (1) or per-worker
 * partial sums (0). Returns 0, EINVAL for bad arguments (*out untouched) or a
 * pthread error. All workers are joined before return. */
int ts_sum(const int32_t *, size_t, unsigned, int, int64_t *);

/* Shared input guarded by `mutex`; `version` changes whenever `values` do. */
struct ts_input
{
    pthread_mutex_t mutex;
    int32_t values[16];
    size_t length;
    unsigned version;
};

/* Called between the snapshot's copy and its sum, with no lock held. */
typedef void (*ts_snapshot_hook_fn)(void *);
int ts_snapshot_sum(struct ts_input *, unsigned, ts_snapshot_hook_fn, void *,
                    int64_t *, unsigned *);

enum
{
    TS_CALLER,
    TS_QUEUED,
    TS_WORKER,
    TS_SETTLED
};

enum
{
    TS_SUCCESS = 1,
    TS_FAILED,
    TS_CANCELED
};

/* Two-slot queue over ids 0..7. All fields are guarded by `mutex`. */
struct ts_queue
{
    unsigned state[8];
    unsigned terminal[8];
    unsigned settlements[8];
    unsigned ring[2];
    unsigned head;
    unsigned count;
    unsigned closed;
    pthread_mutex_t mutex;
};

int ts_queue_init(struct ts_queue *);
int ts_enqueue(struct ts_queue *, unsigned);
int ts_dequeue(struct ts_queue *, unsigned *);
int ts_settle(struct ts_queue *, unsigned, unsigned);
int ts_queue_close(struct ts_queue *);
int ts_queue_inspect(struct ts_queue *, unsigned, unsigned *, unsigned *,
                     unsigned *);
int ts_queue_destroy(struct ts_queue *);

enum
{
    TS_SHARED,
    TS_PRIVATE,
    TS_SHARDED,
    TS_BATCHED
};

/* What a histogram call asked of the system; reported, not measured. Sizes
 * are in bytes and `workers` is a thread count. Written only when the call
 * succeeds. */
struct ts_resources
{
    size_t requested_buffer_bytes;
    unsigned workers;
    size_t requested_stack_bytes;
};

int ts_histogram(const uint32_t *, size_t, size_t, unsigned, size_t, uint64_t *,
                 struct ts_resources *);
#endif

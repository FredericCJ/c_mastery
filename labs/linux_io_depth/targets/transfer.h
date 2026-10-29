#ifndef B06_IO_TRANSFER_H
#define B06_IO_TRANSFER_H
#include <stdint.h>
#include <stddef.h>
#include <sys/uio.h>
#include <pthread.h>
typedef ssize_t (*it_write_fn)(void *, const struct iovec *, int);
/* Callback returns bytes or negative errno; no state reentry or retained vectors.
 * Source spans stay live across resumptions. cursor is accepted byte prefix. */
int it_progress(const struct iovec *, unsigned, size_t *cursor, it_write_fn,
                void *);

struct it_queue
{
    pthread_mutex_t lock;
    pthread_cond_t readable, writable;
    uint64_t values[3];
    unsigned head, count, closed;
};

int it_queue_init(struct it_queue *);
int it_push(struct it_queue *, uint64_t);
int it_pop(struct it_queue *, uint64_t *);
void it_queue_close(struct it_queue *);
void it_queue_destroy(struct it_queue *);

struct it_loop
{
    uint64_t start, last, now, idle, total;
    unsigned ready, next, closed;
};

/* Time monotonic; positive progress only refreshes idle, never total. */
int it_time(struct it_loop *, uint64_t now, int progressed);
int it_choose(struct it_loop *);
void it_drained(struct it_loop *, unsigned peer);
int it_direct_ok(uintptr_t address, size_t length, uint64_t offset,
                 unsigned mem_align, unsigned off_align);
#endif

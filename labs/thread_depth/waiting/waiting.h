#ifndef THREAD_WAITING_H
#define THREAD_WAITING_H
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* See CONTRACTS.md. Objects are stationary, noncopyable, initially unpublished.
 */
struct tw_bank
{
    pthread_mutex_t lock;
    int a;
    int b;
};

typedef void (*tw_hook_fn)(void *);
int tw_bank_init(struct tw_bank *);
int tw_transfer(struct tw_bank *, int, tw_hook_fn, void *);
int tw_bank_total(struct tw_bank *);
void tw_bank_destroy(struct tw_bank *);

struct tw_snapshot
{
    atomic_uint sequence;
    atomic_int a, b;
};

void tw_snapshot_init(struct tw_snapshot *);
int tw_snapshot_write(struct tw_snapshot *, int);
int tw_snapshot_read(struct tw_snapshot *, int *);
int tw_key_next(uint64_t *, uint64_t *);
int tw_order(uint64_t, uint64_t);

/* Report of one tw_queue_init call: how many primitives were initialized,
 * how many of those were destroyed again on failure, and which stage failed
 * (0 when none did). */
struct tw_init
{
    unsigned initialized;
    unsigned destroyed;
    unsigned failed_stage;
};

/* Bounded FIFO of three ints. Field protection and the root/mutex lock order
 * are part of the contract in CONTRACTS.md; tests may read fields only while
 * the queue is quiescent. */
struct tw_queue
{
    pthread_mutex_t root;
    pthread_mutex_t mutex;
    pthread_cond_t nonempty;
    pthread_cond_t nonfull;
    pthread_cond_t observer;
    unsigned state;
    unsigned references;
    unsigned active;
    unsigned initialized;
    unsigned wait_entries;
    size_t head;
    size_t tail;
    size_t count;
    int values[3];
    struct timespec last_deadline;
    unsigned timed_waits;
    unsigned deadline_changes;
};

int tw_queue_init(struct tw_queue *, unsigned, struct tw_init *);
int tw_ref(struct tw_queue *);
int tw_unref(struct tw_queue *);
int tw_close(struct tw_queue *);
int tw_retire(struct tw_queue *);
void tw_root_destroy(struct tw_queue *);
int tw_push(struct tw_queue *, int, const struct timespec *);
int tw_pop(struct tw_queue *, int *, const struct timespec *, tw_hook_fn,
           void *);
int tw_noise(struct tw_queue *);
int tw_wait_entries(struct tw_queue *, unsigned);
int tw_counts(struct tw_queue *, unsigned *, unsigned *, size_t *);

struct tw_barrier
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    unsigned expected;
    unsigned arrived;
    unsigned generation;
    unsigned broken;
};

int tw_barrier_init(struct tw_barrier *, unsigned);
int tw_barrier_wait(struct tw_barrier *);
int tw_barrier_break(struct tw_barrier *);
void tw_barrier_destroy(struct tw_barrier *);

struct tw_permits
{
    pthread_mutex_t mutex;
    unsigned mask;
    unsigned settlements;
};

int tw_permits_init(struct tw_permits *);
int tw_permit_take(struct tw_permits *, unsigned *);
int tw_permit_release(struct tw_permits *, unsigned);
void tw_permits_destroy(struct tw_permits *);
/* Explicit policy model, not a claim about the host rwlock's fairness. */
int tw_reader_admit(unsigned readers, unsigned writer, unsigned writers_waiting,
                    int writer_preference);
int tw_writer_admit(unsigned readers, unsigned writer);

enum tw_primitive
{
    TW_ONCE,
    TW_PERMITS,
    TW_BARRIER,
    TW_RWLOCK
};

int tw_select(unsigned requirement);
#endif

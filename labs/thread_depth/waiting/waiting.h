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
typedef struct {
  pthread_mutex_t lock;
  int a, b;
} TWBank;
typedef void (*TWHook)(void *);
int tw_bank_init(TWBank *);
int tw_transfer(TWBank *, int, TWHook, void *);
int tw_bank_total(TWBank *);
void tw_bank_destroy(TWBank *);
typedef struct {
  atomic_uint sequence;
  atomic_int a, b;
} TWSnapshot;
void tw_snapshot_init(TWSnapshot *);
int tw_snapshot_write(TWSnapshot *, int);
int tw_snapshot_read(TWSnapshot *, int *);
int tw_key_next(uint64_t *, uint64_t *);
int tw_order(uint64_t, uint64_t);

typedef struct {
  unsigned initialized, destroyed, failed_stage;
} TWInit;
typedef struct {
  pthread_mutex_t root, mutex;
  pthread_cond_t nonempty, nonfull, observer;
  unsigned state, references, active, initialized, wait_entries;
  size_t head, tail, count;
  int values[3];
  struct timespec last_deadline;
  unsigned timed_waits, deadline_changes;
} TWQueue;
int tw_queue_init(TWQueue *, unsigned, TWInit *);
int tw_ref(TWQueue *);
int tw_unref(TWQueue *);
int tw_close(TWQueue *);
int tw_retire(TWQueue *);
void tw_root_destroy(TWQueue *);
int tw_push(TWQueue *, int, const struct timespec *);
int tw_pop(TWQueue *, int *, const struct timespec *, TWHook, void *);
int tw_noise(TWQueue *);
int tw_wait_entries(TWQueue *, unsigned);
int tw_counts(TWQueue *, unsigned *, unsigned *, size_t *);

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  unsigned expected, arrived, generation, broken;
} TWBarrier;
int tw_barrier_init(TWBarrier *, unsigned);
int tw_barrier_wait(TWBarrier *);
int tw_barrier_break(TWBarrier *);
void tw_barrier_destroy(TWBarrier *);

typedef struct {
  pthread_mutex_t mutex;
  unsigned mask, settlements;
} TWPermits;
int tw_permits_init(TWPermits *);
int tw_permit_take(TWPermits *, unsigned *);
int tw_permit_release(TWPermits *, unsigned);
void tw_permits_destroy(TWPermits *);
/* Explicit policy model, not a claim about the host rwlock's fairness. */
int tw_reader_admit(unsigned readers, unsigned writer, unsigned writers_waiting,
                    int writer_preference);
int tw_writer_admit(unsigned readers, unsigned writer);
enum TWPrimitive { TW_ONCE, TW_PERMITS, TW_BARRIER, TW_RWLOCK };
int tw_select(unsigned requirement);
#endif

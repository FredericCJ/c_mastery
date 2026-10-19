#ifndef CMASTERY_EXECUTION_H
#define CMASTERY_EXECUTION_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
/* All cmx_* scheduler/object operations are serialized by the port's saved
 * interrupt mask (UP) or scheduler lock (SMP). They never wait for that lock.
 * Smaller numeric priorities are more urgent. No allocation or library calls.
 */
#define CMX_TASKS 16u
#define CMX_MUTEXES 16u
#define CMX_QUEUE 16u
#define CMX_NONE (-1)
enum cmx_result {
  CMX_OK,
  CMX_BLOCKED,
  CMX_TIMEOUT,
  CMX_CLOSED,
  CMX_INVALID,
  CMX_FULL,
  CMX_DEADLOCK,
  CMX_NOT_OWNER,
  CMX_BROKEN,
  CMX_RETRY,
  CMX_UNIMPLEMENTED
};
enum cmx_state { CMX_UNUSED, CMX_READY, CMX_RUNNING, CMX_WAITING, CMX_EXITED };
enum cmx_policy { CMX_RR, CMX_FIXED, CMX_EDF };
enum cmx_wait {
  CMX_W_NONE,
  CMX_W_SLEEP,
  CMX_W_SEM,
  CMX_W_MUTEX,
  CMX_W_COND,
  CMX_W_SEND,
  CMX_W_RECV,
  CMX_W_EVENT,
  CMX_W_NOTIFY,
  CMX_W_BARRIER,
  CMX_W_ONCE,
  CMX_W_READ,
  CMX_W_WRITE,
  CMX_W_JOIN,
  CMX_W_LATCH
};
typedef struct {
  uint32_t at;
  unsigned infinite;
} cmx_deadline;
static inline cmx_deadline cmx_forever(void) {
  cmx_deadline d = {0, 1};
  return d;
}
static inline cmx_deadline cmx_at(uint32_t at) {
  cmx_deadline d = {at, 0};
  return d;
}
typedef struct {
  enum cmx_state state;
  unsigned base_priority, priority, generation, owned_sync;
  enum cmx_wait wait;
  void *object, *aux;
  uint32_t arg, value, flags;
  uint32_t job_deadline, wait_sequence, notifications;
  cmx_deadline deadline;
  enum cmx_result result, pending;
  int exit_code, joiner;
  uintptr_t stack_low, stack_high;
} cmx_task;
typedef struct cmx_mutex {
  int owner;
  unsigned ceiling, use_ceiling, closed;
} cmx_mutex;
typedef struct {
  cmx_task task[CMX_TASKS];
  cmx_mutex *mutex[CMX_MUTEXES];
  unsigned mutex_count;
  uint32_t now, sequence;
  int current;
  enum cmx_policy policy;
} cmx_kernel;
typedef struct {
  unsigned count, maximum, closed;
} cmx_sem;
typedef struct {
  unsigned signaled;
} cmx_latch;
enum cmx_result cmx_latch_wait(cmx_kernel *, cmx_latch *, unsigned,
                               cmx_deadline);
void cmx_latch_signal(cmx_kernel *, cmx_latch *);
typedef struct {
  unsigned closed;
} cmx_cond;
typedef struct {
  uint32_t data[CMX_QUEUE];
  unsigned head, count, capacity, closed;
} cmx_queue;
typedef struct {
  uint32_t bits;
  unsigned closed;
} cmx_event;
typedef struct {
  unsigned parties, arrived, generation, broken;
} cmx_barrier;
typedef struct {
  unsigned state;
  int owner;
} cmx_once;
typedef struct {
  uint32_t readers;
  int writer;
  unsigned closed;
} cmx_rwlock;
void cmx_init(cmx_kernel *, enum cmx_policy);
int cmx_task_add(cmx_kernel *, unsigned priority, uint32_t job_deadline,
                 uintptr_t stack_low, size_t stack_size);
int cmx_schedule(cmx_kernel *);
enum cmx_result cmx_sleep(cmx_kernel *, unsigned, cmx_deadline);
void cmx_tick(cmx_kernel *, uint32_t now);
enum cmx_result cmx_cancel(cmx_kernel *, unsigned);
enum cmx_result cmx_exit(cmx_kernel *, unsigned, int);
enum cmx_result cmx_join(cmx_kernel *, unsigned waiter, unsigned target,
                         cmx_deadline);
enum cmx_result cmx_reap(cmx_kernel *, unsigned target);
void cmx_sem_init(cmx_sem *, unsigned initial, unsigned maximum);
enum cmx_result cmx_sem_take(cmx_kernel *, cmx_sem *, unsigned, cmx_deadline);
enum cmx_result cmx_sem_give(cmx_kernel *, cmx_sem *);
enum cmx_result cmx_sem_close(cmx_kernel *, cmx_sem *);
enum cmx_result cmx_mutex_init(cmx_kernel *, cmx_mutex *, unsigned ceiling,
                               unsigned use_ceiling);
enum cmx_result cmx_mutex_lock(cmx_kernel *, cmx_mutex *, unsigned,
                               cmx_deadline);
enum cmx_result cmx_mutex_unlock(cmx_kernel *, cmx_mutex *, unsigned);
enum cmx_result cmx_mutex_close(cmx_kernel *, cmx_mutex *);
enum cmx_result cmx_cond_wait(cmx_kernel *, cmx_cond *, cmx_mutex *, unsigned,
                              cmx_deadline);
enum cmx_result cmx_cond_signal(cmx_kernel *, cmx_cond *, unsigned broadcast);
enum cmx_result cmx_cond_close(cmx_kernel *, cmx_cond *);
void cmx_queue_init(cmx_queue *, unsigned capacity);
enum cmx_result cmx_queue_send(cmx_kernel *, cmx_queue *, unsigned, uint32_t,
                               cmx_deadline);
enum cmx_result cmx_queue_recv(cmx_kernel *, cmx_queue *, unsigned,
                               cmx_deadline);
enum cmx_result cmx_queue_close(cmx_kernel *, cmx_queue *);
enum cmx_result cmx_queue_abort(cmx_kernel *, cmx_queue *);
enum cmx_result cmx_event_wait(cmx_kernel *, cmx_event *, unsigned,
                               uint32_t mask, unsigned all, unsigned clear,
                               cmx_deadline);
void cmx_event_set(cmx_kernel *, cmx_event *, uint32_t);
void cmx_event_close(cmx_kernel *, cmx_event *);
enum cmx_result cmx_notify(cmx_kernel *, unsigned, uint32_t count);
enum cmx_result cmx_notify_take(cmx_kernel *, unsigned, cmx_deadline);
void cmx_barrier_init(cmx_barrier *, unsigned parties);
enum cmx_result cmx_barrier_wait(cmx_kernel *, cmx_barrier *, unsigned,
                                 cmx_deadline);
void cmx_once_init(cmx_once *);
enum cmx_result cmx_once_enter(cmx_kernel *, cmx_once *, unsigned,
                               cmx_deadline);
enum cmx_result cmx_once_finish(cmx_kernel *, cmx_once *, unsigned,
                                unsigned success);
void cmx_rw_init(cmx_rwlock *);
enum cmx_result cmx_rw_read(cmx_kernel *, cmx_rwlock *, unsigned, cmx_deadline);
enum cmx_result cmx_rw_write(cmx_kernel *, cmx_rwlock *, unsigned,
                             cmx_deadline);
enum cmx_result cmx_rw_unlock(cmx_kernel *, cmx_rwlock *, unsigned);
enum cmx_result cmx_rw_close(cmx_kernel *, cmx_rwlock *);
int cmx_invariant(const cmx_kernel *);
typedef struct {
  uint32_t cost, period, deadline, jitter, blocking;
} cmx_job;
enum cmx_result cmx_response(const cmx_job *, unsigned count, unsigned index,
                             uint32_t *response);
enum cmx_result cmx_edf_admit(const cmx_job *, unsigned count,
                              uint32_t horizon);
typedef struct {
  uint32_t budget, period, remaining, deadline;
  unsigned active;
} cmx_cbs;
enum cmx_result cmx_cbs_init(cmx_cbs *, uint32_t budget, uint32_t period);
enum cmx_result cmx_cbs_release(cmx_cbs *, uint32_t now);
enum cmx_result cmx_cbs_charge(cmx_cbs *, uint32_t units);
/* SPSC full-capacity monotonic counters; capacity is a power of two <=16.
 * The caller ensures no producer/consumer reentrancy and persistent storage. */
typedef struct {
  atomic_uint head, tail;
  unsigned capacity;
  uint32_t data[16];
} cmx_spsc;
int cmx_spsc_init(cmx_spsc *, unsigned);
enum cmx_result cmx_spsc_push(cmx_spsc *, uint32_t);
enum cmx_result cmx_spsc_pop(cmx_spsc *, uint32_t *);
#define CMX_HP_THREADS 4u
#define CMX_HP_NODES 32u
typedef struct {
  atomic_uint next, state;
  uint32_t value;
} cmx_hp_node;
typedef struct {
  atomic_uint head, tail, hazard[CMX_HP_THREADS][2];
  cmx_hp_node node[CMX_HP_NODES];
} cmx_hp_queue;
void cmx_hp_init(cmx_hp_queue *);
enum cmx_result cmx_hp_push(cmx_hp_queue *, unsigned participant,
                            uint32_t value);
enum cmx_result cmx_hp_pop(cmx_hp_queue *, unsigned participant,
                           uint32_t *value);
unsigned cmx_hp_collect(cmx_hp_queue *);
/* Quiescent/epoch teaching tracker, not an automatic reclaiming allocator. */
typedef struct {
  unsigned epoch, active[CMX_HP_THREADS], seen[CMX_HP_THREADS];
} cmx_epoch;
void cmx_epoch_init(cmx_epoch *);
enum cmx_result cmx_epoch_enter(cmx_epoch *, unsigned);
void cmx_epoch_leave(cmx_epoch *, unsigned);
int cmx_epoch_advance(cmx_epoch *);
#endif

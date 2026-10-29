#ifndef CMASTERY_EXECUTION_ADVANCED_H
#define CMASTERY_EXECUTION_ADVANCED_H
#include "cmx.h"

/* Finite scheduling experiments. Integer time units, synchronous periodic
 * constrained-deadline jobs, no I/O/jitter/blocking in this simulator. */
typedef struct
{
    unsigned misses, completed, migrations;
    uint32_t work;
} cmx_sim_result;
enum cmx_result cmx_priority_order(const cmx_job *, unsigned,
                                   unsigned deadline_monotonic,
                                   unsigned *order);
enum cmx_result cmx_partition(const cmx_job *, unsigned, unsigned cpus,
                              uint32_t hyperperiod, unsigned *cpu);
enum cmx_result cmx_simulate(const cmx_job *, unsigned, unsigned cpus,
                             const unsigned *partition, uint32_t horizon,
                             cmx_sim_result *);

typedef struct
{
    uint32_t budget, period, remaining, last;
    unsigned count;
    uint32_t at[16], amount[16];
} cmx_server;
enum cmx_result cmx_server_init(cmx_server *, uint32_t, uint32_t);
enum cmx_result cmx_server_update(cmx_server *, uint32_t);
enum cmx_result cmx_server_charge(cmx_server *, uint32_t, uint32_t);

typedef struct
{
    unsigned used[16], generation[16];
} cmx_identity_pool;

void cmx_identity_init(cmx_identity_pool *);
uint32_t cmx_identity_take(cmx_identity_pool *);
int cmx_identity_valid(const cmx_identity_pool *, uint32_t);
enum cmx_result cmx_identity_put(cmx_identity_pool *, uint32_t);

/* Deliberately reservation-blocking queue: stall an uncommitted ticket to
 * distinguish FIFO safety from lock-free progress. Single consumer. */
typedef struct
{
    atomic_uint head, tail, ready[4];
    uint32_t data[4];
} cmx_reservation;

void cmx_reservation_init(cmx_reservation *);
enum cmx_result cmx_reserve(cmx_reservation *, uint32_t *ticket);
enum cmx_result cmx_publish(cmx_reservation *, uint32_t ticket, uint32_t value);
enum cmx_result cmx_reserved_pop(cmx_reservation *, uint32_t *value);

/* Complete histories only, at most eight operations. op: 0 enqueue,1 dequeue.
 * status: OK / FULL / RETRY(empty). Nonwrapping invocation/response times. */
typedef struct
{
    unsigned participant, op;
    uint32_t invoke, respond, value;
    enum cmx_result status;
} cmx_history_op;
enum cmx_result cmx_history_check(const cmx_history_op *, unsigned,
                                  unsigned capacity, unsigned *witness);

/* Original coarse RCU teaching object. Immutable payloads live outside it.
 * One serialized writer, fixed participants, no nested reader sections.
 * Readers never wait; quiescence polling may wait indefinitely. */
typedef struct
{
    atomic_uint current, active[CMX_HP_THREADS];
} cmx_rcu;

void cmx_rcu_init(cmx_rcu *, unsigned);
enum cmx_result cmx_rcu_read(cmx_rcu *, unsigned, unsigned *);
enum cmx_result cmx_rcu_leave(cmx_rcu *, unsigned);
unsigned cmx_rcu_publish(cmx_rcu *, unsigned);
int cmx_rcu_quiescent(const cmx_rcu *);
#endif

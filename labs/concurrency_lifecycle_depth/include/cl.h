#ifndef CMASTERY_CONCURRENCY_LIFECYCLE_H
#define CMASTERY_CONCURRENCY_LIFECYCLE_H
#include <stddef.h>
#include <stdint.h>

/* Serialized C17 core: callers supply exclusion and keep all objects live.
 * No allocation, atomics, libc, or hidden port/scheduler dependency. */
enum cl_result {
  CL_OK,
  CL_EMPTY,
  CL_FULL,
  CL_RANGE,
  CL_CLOSED,
  CL_BUSY,
  CL_STALE,
  CL_FRAGMENTED,
  CL_POISONED,
  CL_UNRECOVERABLE,
  CL_PENDING,
  CL_UNIMPLEMENTED
};
typedef struct {
  uint32_t producer, sequence;
  size_t length;
  unsigned char bytes[4];
} CLMessage;
typedef struct {
  CLMessage *slots;
  unsigned head, count, closed;
  uint64_t accepted, delivered, discarded;
} CLQueue;
void cl_queue_init(CLQueue *, CLMessage storage[2]);
enum cl_result cl_send(CLQueue *, uint32_t producer, uint32_t sequence,
                       const unsigned char *, size_t length);
enum cl_result cl_recv(CLQueue *, unsigned char *, size_t capacity,
                       size_t *length, uint32_t *producer, uint32_t *sequence);
void cl_close(CLQueue *);
uint64_t cl_abort(CLQueue *);

typedef struct {
  unsigned slot;
  uint64_t generation;
} CLBuffer;
enum cl_owner { CL_FREE, CL_CALLER, CL_QUEUED, CL_CONSUMER };
typedef struct {
  unsigned offset, length;
  uint64_t generation;
  enum cl_owner owner;
} CLAllocation;
typedef struct {
  unsigned char bytes[16];
  CLAllocation slots[4];
  uint64_t allocations, releases;
} CLPool;
typedef struct {
  CLBuffer slots[2];
  unsigned head, count, closed;
} CLOwnedQueue;
void cl_pool_init(CLPool *);
enum cl_result cl_alloc(CLPool *, unsigned length, CLBuffer *);
enum cl_result cl_buffer_write(CLPool *, CLBuffer, const unsigned char *,
                               unsigned);
enum cl_result cl_buffer_read(const CLPool *, CLBuffer, unsigned char *,
                              unsigned, unsigned *);
enum cl_result cl_release(CLPool *, CLBuffer);
void cl_owned_init(CLOwnedQueue *);
enum cl_result cl_owned_send(CLPool *, CLOwnedQueue *, CLBuffer);
enum cl_result cl_owned_recv(CLPool *, CLOwnedQueue *, CLBuffer *);
void cl_owned_close(CLOwnedQueue *);
unsigned cl_owned_abort(CLPool *, CLOwnedQueue *);

enum cl_phase { CL_OPEN, CL_CLOSING, CL_SHUT, CL_DESTROYED };
enum cl_lease_state {
  CL_UNUSED,
  CL_ACTIVE,
  CL_SEND_WAIT,
  CL_RECV_WAIT,
  CL_COND_RELOCK,
  CL_TIMER,
  CL_WAKE_CLOSED
};
typedef struct {
  uint64_t object_generation, generation;
  unsigned slot;
} CLLease;
typedef struct {
  uint64_t generation;
  enum cl_lease_state state;
} CLLeaseSlot;
typedef struct {
  uint64_t generation;
  enum cl_phase phase;
  CLLeaseSlot leases[8];
} CLLife;
void cl_life_init(CLLife *);
enum cl_result cl_acquire(CLLife *, CLLease *);
enum cl_result cl_transition(CLLife *, CLLease, enum cl_lease_state from,
                             enum cl_lease_state to);
enum cl_result cl_drop(CLLife *, CLLease);
void cl_life_close(CLLife *);
enum cl_result cl_life_drained(CLLife *);
enum cl_result cl_destroy(CLLife *);
enum cl_result cl_restart(CLLife *);
enum cl_result cl_restart_group(CLLife *, unsigned count);

enum cl_record_state {
  CL_HEALTHY,
  CL_OWNED,
  CL_DIRTY,
  CL_NEEDS_REPAIR,
  CL_REPAIRING,
  CL_BROKEN
};
typedef struct {
  uint32_t value, checksum, committed, owner;
  unsigned repairs;
  enum cl_record_state state;
} CLRecord;
void cl_record_init(CLRecord *, uint32_t);
enum cl_result cl_record_begin(CLRecord *, uint32_t owner);
enum cl_result cl_record_stage(CLRecord *, uint32_t owner, uint32_t value);
enum cl_result cl_record_commit(CLRecord *, uint32_t owner);
enum cl_result cl_record_dead(CLRecord *, uint32_t trusted_dead_owner);
enum cl_result cl_repair_begin(CLRecord *, uint32_t supervisor);
enum cl_result cl_repair_finish(CLRecord *, uint32_t supervisor,
                                unsigned repair);
enum cl_result cl_record_read(const CLRecord *, uint32_t *);

/* Bounded volatile application-acknowledgement adapter. No crash durability. */
typedef struct {
  unsigned state;
  uint32_t id, delta, result;
} CLReceipt;
typedef struct {
  CLReceipt receipts[4];
  uint32_t epoch, highwater, value;
} CLAck;
void cl_ack_init(CLAck *, uint32_t epoch);
enum cl_result cl_request(CLAck *, uint32_t epoch, uint32_t id, uint32_t delta,
                          uint32_t *result);
enum cl_result cl_apply(CLAck *, uint32_t id);
enum cl_result cl_acknowledge(CLAck *, uint32_t id);
#endif

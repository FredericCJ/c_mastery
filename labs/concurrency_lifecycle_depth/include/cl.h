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
struct cl_message {
  uint32_t producer, sequence;
  size_t length;
  unsigned char bytes[4];
};
struct cl_queue {
  struct cl_message *slots;
  unsigned head, count, closed;
  uint64_t accepted, delivered, discarded;
};
void cl_queue_init(struct cl_queue *, struct cl_message storage[2]);
enum cl_result cl_send(struct cl_queue *, uint32_t producer, uint32_t sequence,
                       const unsigned char *, size_t length);
enum cl_result cl_recv(struct cl_queue *, unsigned char *, size_t capacity,
                       size_t *length, uint32_t *producer, uint32_t *sequence);
void cl_close(struct cl_queue *);
uint64_t cl_abort(struct cl_queue *);

struct cl_buffer {
  unsigned slot;
  uint64_t generation;
};
enum cl_owner { CL_FREE, CL_CALLER, CL_QUEUED, CL_CONSUMER };
struct cl_allocation {
  unsigned offset, length;
  uint64_t generation;
  enum cl_owner owner;
};
struct cl_pool {
  unsigned char bytes[16];
  struct cl_allocation slots[4];
  uint64_t allocations, releases;
};
struct cl_owned_queue {
  struct cl_buffer slots[2];
  unsigned head, count, closed;
};
void cl_pool_init(struct cl_pool *);
enum cl_result cl_alloc(struct cl_pool *, unsigned length, struct cl_buffer *);
enum cl_result cl_buffer_write(struct cl_pool *, struct cl_buffer, const unsigned char *,
                               unsigned);
enum cl_result cl_buffer_read(const struct cl_pool *, struct cl_buffer, unsigned char *,
                              unsigned, unsigned *);
enum cl_result cl_release(struct cl_pool *, struct cl_buffer);
void cl_owned_init(struct cl_owned_queue *);
enum cl_result cl_owned_send(struct cl_pool *, struct cl_owned_queue *, struct cl_buffer);
enum cl_result cl_owned_recv(struct cl_pool *, struct cl_owned_queue *, struct cl_buffer *);
void cl_owned_close(struct cl_owned_queue *);
unsigned cl_owned_abort(struct cl_pool *, struct cl_owned_queue *);

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
struct cl_lease {
  uint64_t object_generation, generation;
  unsigned slot;
};
struct cl_lease_slot {
  uint64_t generation;
  enum cl_lease_state state;
};
struct cl_life {
  uint64_t generation;
  enum cl_phase phase;
  struct cl_lease_slot leases[8];
};
void cl_life_init(struct cl_life *);
enum cl_result cl_acquire(struct cl_life *, struct cl_lease *);
enum cl_result cl_transition(struct cl_life *, struct cl_lease, enum cl_lease_state from,
                             enum cl_lease_state to);
enum cl_result cl_drop(struct cl_life *, struct cl_lease);
void cl_life_close(struct cl_life *);
enum cl_result cl_life_drained(struct cl_life *);
enum cl_result cl_destroy(struct cl_life *);
enum cl_result cl_restart(struct cl_life *);
enum cl_result cl_restart_group(struct cl_life *, unsigned count);

enum cl_record_state {
  CL_HEALTHY,
  CL_OWNED,
  CL_DIRTY,
  CL_NEEDS_REPAIR,
  CL_REPAIRING,
  CL_BROKEN
};
struct cl_record {
  uint32_t value, checksum, committed, owner;
  unsigned repairs;
  enum cl_record_state state;
};
void cl_record_init(struct cl_record *, uint32_t);
enum cl_result cl_record_begin(struct cl_record *, uint32_t owner);
enum cl_result cl_record_stage(struct cl_record *, uint32_t owner, uint32_t value);
enum cl_result cl_record_commit(struct cl_record *, uint32_t owner);
enum cl_result cl_record_dead(struct cl_record *, uint32_t trusted_dead_owner);
enum cl_result cl_repair_begin(struct cl_record *, uint32_t supervisor);
enum cl_result cl_repair_finish(struct cl_record *, uint32_t supervisor,
                                unsigned repair);
enum cl_result cl_record_read(const struct cl_record *, uint32_t *);

/* Bounded volatile application-acknowledgement adapter. No crash durability. */
struct cl_receipt {
  unsigned state;
  uint32_t id, delta, result;
};
struct cl_ack {
  struct cl_receipt receipts[4];
  uint32_t epoch, highwater, value;
};
void cl_ack_init(struct cl_ack *, uint32_t epoch);
enum cl_result cl_request(struct cl_ack *, uint32_t epoch, uint32_t id, uint32_t delta,
                          uint32_t *result);
enum cl_result cl_apply(struct cl_ack *, uint32_t id);
enum cl_result cl_acknowledge(struct cl_ack *, uint32_t id);
#endif

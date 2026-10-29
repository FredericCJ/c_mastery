#ifndef CM_REACTOR_DEPTH_H
#define CM_REACTOR_DEPTH_H
#include "stream.h"
#include <stddef.h>
#include <stdint.h>

enum rd_terminal { RD_ACTIVE, RD_DONE, RD_EOF, RD_ERROR, RD_CANCELLED };
struct rd_operation {
  unsigned char bytes[8];
  size_t goal, done;
  enum rd_terminal terminal;
};
/* Active synchronous operation: amount>0 is ACTUAL committed prefix, -2 is
 * would-block, -1 terminal error,0 EOF. For receive, source has amount live
 * octets. For transmit, source is NULL and initial bytes remain immutable. */
int rd_operation_init(struct rd_operation *, size_t goal,
                      const unsigned char *transmit);
int rd_progress(struct rd_operation *, int amount, const unsigned char *received);
int rd_cancel(struct rd_operation *);

struct rd_token {
  unsigned slot, generation;
};
struct rd_completion {
  struct rd_token connection;
  unsigned direction, sequence;
};
struct rd_slot {
  unsigned generation, active, closing, retired, borrowers, pending;
  unsigned submitted[2];
  int descriptor;
  struct rd_operation receive, transmit;
};
struct rd_registry {
  struct rd_slot slots[4];
  unsigned maximum;
};
/* Registry lives for every cached event. Generation range[first,maximum],
 * maximum<=65535. Closing forbids new borrows. Storage is released/reused only
 * after all callback borrows and target completions settle. Cancel ACK alone
 * is not a target completion. Registry functions are serialized. */
int rd_registry_init(struct rd_registry *, unsigned first, unsigned maximum);
int rd_register(struct rd_registry *, unsigned slot, int descriptor, struct rd_token *);
struct rd_slot *rd_borrow(struct rd_registry *, struct rd_token);
int rd_return(struct rd_registry *, struct rd_token);
int rd_close(struct rd_registry *, struct rd_token);
/* Each direction has a nonreused sequence1..65535 within its connection.
 * Exhaustion refuses further submissions; a fresh connection generation is
 * required. Even a duplicate completion in the SAME connection is stale. */
int rd_submit(struct rd_registry *, struct rd_token, unsigned direction, struct rd_completion *);
int rd_complete(struct rd_registry *, struct rd_completion, int amount,
                const unsigned char *received);

struct rd_scheduler {
  unsigned ready, cursor, running;
};
struct rd_turn {
  unsigned callbacks, successes, order[8];
};
/* Three participants. Callback result:1 successful operation,0 CPU-only work,
 * -1 would-block,-2 terminal. At most2 successes per participant,8 globally,
 * and callbacks<=work_limit<=8. Snapshot-at-turn-start; reentrant rd_ready adds
 * work for the NEXT turn. Nested rd_turn is rejected. */
int rd_ready(struct rd_scheduler *, unsigned slot);
int rd_turn(struct rd_scheduler *, unsigned work_limit,
            int (*callback)(void *, unsigned), void *, struct rd_turn *);

enum rd_policy { RD_DROP_NEWEST, RD_DISCONNECT, RD_PAUSE_READ };
enum rd_phase { RD_RUNNING, RD_DRAINING, RD_CANCELLING, RD_RELEASED };
struct rd_record {
  unsigned id;
  enum rf_owner outcome;
};
struct rd_service {
  struct rf_outbox outbox;
  enum rd_phase phase;
  uint64_t deadline;
  unsigned paused, borrowers, accepted, terminal;
  struct rd_record records[64];
};
/* One finite service epoch accepts at most64 distinct IDs; ledger exhaustion
 * rejects caller-owned input. Uses the actual prior RfOutbox contract. */
int rd_service_init(struct rd_service *, size_t byte_budget);
int rd_service_open(struct rd_service *, unsigned *slot);
/* Admission is outside callbacks (borrowers==0); a callback may request stop,
 * but may not trigger a disconnect policy which releases another borrowed
 * message. Such running-state admission is rejected unchanged with -1. */
int rd_admit(struct rd_service *, unsigned slot, struct rf_message *, enum rd_policy);
int rd_resume(struct rd_service *, unsigned slot);
int rd_commit(struct rd_service *, unsigned slot, size_t bytes);
int rd_service_borrow(struct rd_service *);
int rd_service_return(struct rd_service *);
/* Stop rejects new admissions immediately; repeats can shorten, never extend,
 * one absolute deadline. Tick settles pending IDs and defers release while a
 * synchronous callback borrows storage. Pending messages stay OUTBOX-owned
 * until the final borrower returns; a cancellation request is not permission
 * for a caller to free their payload. Does not cancel asynchronous I/O. */
int rd_stop(struct rd_service *, uint64_t now, uint64_t deadline);
int rd_tick(struct rd_service *, uint64_t now);
#endif

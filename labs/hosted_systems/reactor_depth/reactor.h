#ifndef CM_REACTOR_DEPTH_H
#define CM_REACTOR_DEPTH_H
#include "stream.h"
#include <stddef.h>
#include <stdint.h>

enum RdTerminal { RD_ACTIVE, RD_DONE, RD_EOF, RD_ERROR, RD_CANCELLED };
typedef struct {
  unsigned char bytes[8];
  size_t goal, done;
  enum RdTerminal terminal;
} RdOperation;
/* Active synchronous operation: amount>0 is ACTUAL committed prefix, -2 is
 * would-block, -1 terminal error,0 EOF. For receive, source has amount live
 * octets. For transmit, source is NULL and initial bytes remain immutable. */
int rd_operation_init(RdOperation *, size_t goal,
                      const unsigned char *transmit);
int rd_progress(RdOperation *, int amount, const unsigned char *received);
int rd_cancel(RdOperation *);

typedef struct {
  unsigned slot, generation;
} RdToken;
typedef struct {
  RdToken connection;
  unsigned direction, sequence;
} RdCompletion;
typedef struct {
  unsigned generation, active, closing, retired, borrowers, pending;
  unsigned submitted[2];
  int descriptor;
  RdOperation receive, transmit;
} RdSlot;
typedef struct {
  RdSlot slots[4];
  unsigned maximum;
} RdRegistry;
/* Registry lives for every cached event. Generation range[first,maximum],
 * maximum<=65535. Closing forbids new borrows. Storage is released/reused only
 * after all callback borrows and target completions settle. Cancel ACK alone
 * is not a target completion. Registry functions are serialized. */
int rd_registry_init(RdRegistry *, unsigned first, unsigned maximum);
int rd_register(RdRegistry *, unsigned slot, int descriptor, RdToken *);
RdSlot *rd_borrow(RdRegistry *, RdToken);
int rd_return(RdRegistry *, RdToken);
int rd_close(RdRegistry *, RdToken);
/* Each direction has a nonreused sequence1..65535 within its connection.
 * Exhaustion refuses further submissions; a fresh connection generation is
 * required. Even a duplicate completion in the SAME connection is stale. */
int rd_submit(RdRegistry *, RdToken, unsigned direction, RdCompletion *);
int rd_complete(RdRegistry *, RdCompletion, int amount,
                const unsigned char *received);

typedef struct {
  unsigned ready, cursor, running;
} RdScheduler;
typedef struct {
  unsigned callbacks, successes, order[8];
} RdTurn;
/* Three participants. Callback result:1 successful operation,0 CPU-only work,
 * -1 would-block,-2 terminal. At most2 successes per participant,8 globally,
 * and callbacks<=work_limit<=8. Snapshot-at-turn-start; reentrant rd_ready adds
 * work for the NEXT turn. Nested rd_turn is rejected. */
int rd_ready(RdScheduler *, unsigned slot);
int rd_turn(RdScheduler *, unsigned work_limit,
            int (*callback)(void *, unsigned), void *, RdTurn *);

enum RdPolicy { RD_DROP_NEWEST, RD_DISCONNECT, RD_PAUSE_READ };
enum RdPhase { RD_RUNNING, RD_DRAINING, RD_CANCELLING, RD_RELEASED };
typedef struct {
  unsigned id;
  enum RfOwner outcome;
} RdRecord;
typedef struct {
  RfOutbox outbox;
  enum RdPhase phase;
  uint64_t deadline;
  unsigned paused, borrowers, accepted, terminal;
  RdRecord records[64];
} RdService;
/* One finite service epoch accepts at most64 distinct IDs; ledger exhaustion
 * rejects caller-owned input. Uses the actual prior RfOutbox contract. */
int rd_service_init(RdService *, size_t byte_budget);
int rd_service_open(RdService *, unsigned *slot);
/* Admission is outside callbacks (borrowers==0); a callback may request stop,
 * but may not trigger a disconnect policy which releases another borrowed
 * message. Such running-state admission is rejected unchanged with -1. */
int rd_admit(RdService *, unsigned slot, RfMessage *, enum RdPolicy);
int rd_resume(RdService *, unsigned slot);
int rd_commit(RdService *, unsigned slot, size_t bytes);
int rd_service_borrow(RdService *);
int rd_service_return(RdService *);
/* Stop rejects new admissions immediately; repeats can shorten, never extend,
 * one absolute deadline. Tick settles pending IDs and defers release while a
 * synchronous callback borrows storage. Pending messages stay OUTBOX-owned
 * until the final borrower returns; a cancellation request is not permission
 * for a caller to free their payload. Does not cancel asynchronous I/O. */
int rd_stop(RdService *, uint64_t now, uint64_t deadline);
int rd_tick(RdService *, uint64_t now);
#endif

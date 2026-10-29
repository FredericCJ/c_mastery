#ifndef CM_EVENT_IPC_H
#define CM_EVENT_IPC_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
enum ei_state {
  EI_ALLOCATED,
  EI_QUEUED,
  EI_ACCEPTED,
  EI_COMPLETED,
  EI_ABANDONED
};
enum ei_owner { EI_CLIENT, EI_TRANSPORT, EI_SERVER, EI_NO_OWNER };
enum ei_outcome { EI_PENDING, EI_SUCCESS, EI_DEFINITELY_NOT_DONE, EI_UNCERTAIN };
enum ei_event {
  EI_QUEUE,
  EI_ACCEPT,
  EI_EFFECT,
  EI_COMPLETE,
  EI_DISCONNECT,
  EI_CANCEL_REQUEST,
  EI_CANCEL_SEEN,
  EI_STOP_CONFIRMED
};
struct ei_request {
  unsigned char payload[8];
  uint32_t result;
  unsigned epoch, request_id;
  enum ei_state state;
  enum ei_owner owner;
  enum ei_outcome outcome;
  unsigned connected, effect_done, cancel_requested, cancel_seen;
};
/* Epoch/id are1..65535. Initialization is permitted only in fresh storage or
 * after its previous owner has released all accesses. Invalid args preserve
 * out. */
int ei_init(struct ei_request *out, const unsigned char payload[8], unsigned epoch,
            unsigned id);
/* Serialized authoritative ledger. An invalid transition leaves every field
 * unchanged. DISCONNECT before acceptance includes confirmed transport purge;
 * absence of that proof must be modeled as accepted/uncertain. */
int ei_transition(struct ei_request *request, enum ei_event event);
int ei_invariant(const struct ei_request *request);
struct ei_queue {
  struct ei_request *slots[4];
  unsigned head, count;
  size_t bytes;
};
/* Queue owns exactly the requests it admitted. Enqueue transfers CLIENT to
 * TRANSPORT; dequeue transfers TRANSPORT to SERVER. Rejection preserves owner.
 * Duplicate live (epoch,id) and duplicate addresses are refused. */
int ei_enqueue(struct ei_queue *queue, struct ei_request *request);
int ei_dequeue(struct ei_queue *queue, struct ei_request **request);
/* Stable8-byte request ->4-byte big-endian sum; no host-layout wire structs. */
void ei_reply(const unsigned char request[8], unsigned char response[4]);
struct ei_io {
  int (*now)(void *, struct timespec *);
  ssize_t (*transfer)(void *, int sending, unsigned char *, size_t);
  int (*wait)(void *, int sending, struct timespec remaining);
  void *context;
};
struct ei_progress {
  size_t sent, received;
  unsigned char reply[4];
};
/* One absolute CLOCK_MONOTONIC-domain deadline spans all8 sent and4 received
 * bytes. Start with zero progress; resumption requires the same
 * request/deadline and exclusive channel ownership. Nonblocking callbacks; wait
 * may return0 or1 spuriously. Publish out only on complete response observed
 * BEFORE deadline. Failure retains byte progress and errno; timeout is not
 * remote rollback. Invalid arguments preserve progress/output. Caller owns live
 * storage through the final callback; this API does not implement asynchronous
 * cancellation. */
int ei_exchange(struct ei_io operations, const unsigned char request[8],
                unsigned char response[4], struct timespec deadline,
                struct ei_progress *progress);
#endif

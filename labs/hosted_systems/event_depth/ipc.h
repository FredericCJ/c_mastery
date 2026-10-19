#ifndef CM_EVENT_IPC_H
#define CM_EVENT_IPC_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
enum EiState {
  EI_ALLOCATED,
  EI_QUEUED,
  EI_ACCEPTED,
  EI_COMPLETED,
  EI_ABANDONED
};
enum EiOwner { EI_CLIENT, EI_TRANSPORT, EI_SERVER, EI_NO_OWNER };
enum EiOutcome { EI_PENDING, EI_SUCCESS, EI_DEFINITELY_NOT_DONE, EI_UNCERTAIN };
enum EiEvent {
  EI_QUEUE,
  EI_ACCEPT,
  EI_EFFECT,
  EI_COMPLETE,
  EI_DISCONNECT,
  EI_CANCEL_REQUEST,
  EI_CANCEL_SEEN,
  EI_STOP_CONFIRMED
};
typedef struct {
  unsigned char payload[8];
  uint32_t result;
  unsigned epoch, request_id;
  enum EiState state;
  enum EiOwner owner;
  enum EiOutcome outcome;
  unsigned connected, effect_done, cancel_requested, cancel_seen;
} EiRequest;
/* Epoch/id are1..65535. Initialization is permitted only in fresh storage or
 * after its previous owner has released all accesses. Invalid args preserve
 * out. */
int ei_init(EiRequest *out, const unsigned char payload[8], unsigned epoch,
            unsigned id);
/* Serialized authoritative ledger. An invalid transition leaves every field
 * unchanged. DISCONNECT before acceptance includes confirmed transport purge;
 * absence of that proof must be modeled as accepted/uncertain. */
int ei_transition(EiRequest *request, enum EiEvent event);
int ei_invariant(const EiRequest *request);
typedef struct {
  EiRequest *slots[4];
  unsigned head, count;
  size_t bytes;
} EiQueue;
/* Queue owns exactly the requests it admitted. Enqueue transfers CLIENT to
 * TRANSPORT; dequeue transfers TRANSPORT to SERVER. Rejection preserves owner.
 * Duplicate live (epoch,id) and duplicate addresses are refused. */
int ei_enqueue(EiQueue *queue, EiRequest *request);
int ei_dequeue(EiQueue *queue, EiRequest **request);
/* Stable8-byte request ->4-byte big-endian sum; no host-layout wire structs. */
void ei_reply(const unsigned char request[8], unsigned char response[4]);
typedef struct {
  int (*now)(void *, struct timespec *);
  ssize_t (*transfer)(void *, int sending, unsigned char *, size_t);
  int (*wait)(void *, int sending, struct timespec remaining);
  void *context;
} EiIo;
typedef struct {
  size_t sent, received;
  unsigned char reply[4];
} EiProgress;
/* One absolute CLOCK_MONOTONIC-domain deadline spans all8 sent and4 received
 * bytes. Start with zero progress; resumption requires the same
 * request/deadline and exclusive channel ownership. Nonblocking callbacks; wait
 * may return0 or1 spuriously. Publish out only on complete response observed
 * BEFORE deadline. Failure retains byte progress and errno; timeout is not
 * remote rollback. Invalid arguments preserve progress/output. Caller owns live
 * storage through the final callback; this API does not implement asynchronous
 * cancellation. */
int ei_exchange(EiIo operations, const unsigned char request[8],
                unsigned char response[4], struct timespec deadline,
                EiProgress *progress);
#endif

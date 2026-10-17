#ifndef CMASTERY_RESILIENCE_H
#define CMASTERY_RESILIENCE_H
#include "messaging.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum {
  RZ_OK,
  RZ_INVALID,
  RZ_FULL,
  RZ_EMPTY,
  RZ_CANCELLED,
  RZ_EXPIRED,
  RZ_STALE,
  RZ_RANGE,
  RZ_PENDING,
  RZ_DROPPED,
  RZ_DEGRADED,
  RZ_OPEN,
  RZ_BUSY,
  RZ_CONFLICT,
  RZ_DONE,
  RZ_DENIED,
  RZ_CLOSED
} RZStatus;
typedef struct RZScope {
  const struct RZScope *parent;
  uint64_t deadline;
  unsigned depth;
  atomic_bool cancelled;
} RZScope;
int rz_scope_init(RZScope *, const RZScope *, uint64_t, uint64_t);
void rz_cancel(RZScope *);
int rz_check(const RZScope *, uint64_t);
int rz_remaining(const RZScope *, uint64_t, uint64_t *);
typedef struct {
  unsigned values[4];
  size_t head, count;
  bool closed;
} RZQueue;
typedef enum {
  RZ_BLOCK,
  RZ_REJECT,
  RZ_DROP,
  RZ_OVERWRITE,
  RZ_DEGRADE
} RZPolicy;
typedef struct {
  const RZScope *scope;
  unsigned value;
  bool pending;
} RZOffer;
void rz_queue_init(RZQueue *);
int rz_offer(RZQueue *, RZOffer *, RZPolicy, uint64_t);
int rz_pop(RZQueue *, unsigned *);
int rz_forward(RZQueue *, MsgHub *, unsigned);
typedef struct {
  uint64_t due;
  int next, previous;
  unsigned generation;
  bool active;
} RZTimer;
typedef struct RZWheel {
  uint64_t now;
  int head[4];
  RZTimer slots[4];
} RZWheel;
typedef struct {
  const RZWheel *owner;
  unsigned slot, generation;
} RZTimerId;
void rz_wheel_init(RZWheel *, uint64_t);
int rz_schedule(RZWheel *, uint64_t, RZTimerId *);
int rz_unschedule(RZWheel *, RZTimerId);
int rz_tick(RZWheel *, unsigned *);
typedef struct {
  uint64_t capacity, rate, tokens, last;
} RZBucket;
int rz_bucket_init(RZBucket *, uint64_t, uint64_t, uint64_t);
int rz_consume(RZBucket *, uint64_t, uint64_t);
typedef struct {
  const RZScope *scope;
  unsigned attempts, maximum;
  uint64_t base, cap;
  bool idempotent;
} RZRetry;
int rz_retry_init(RZRetry *, const RZScope *, unsigned, uint64_t, uint64_t,
                  bool);
int rz_retry_next(RZRetry *, uint64_t, uint64_t, uint64_t *);
typedef struct RZGate {
  unsigned phase, failures, limit, epoch;
  uint64_t until, last;
  bool active[2], retired;
  unsigned generations[2], epochs[2];
} RZGate;
typedef struct {
  const RZGate *owner;
  unsigned slot, generation;
} RZPermit;
int rz_gate_init(RZGate *, unsigned, uint64_t);
int rz_admit(RZGate *, uint64_t, RZPermit *);
int rz_finish(RZGate *, RZPermit, uint64_t, int);
typedef struct {
  const RZScope *scope;
  uint64_t hedge_at;
  unsigned started, completed, cancel_requested;
  int winner;
  unsigned value;
} RZHedge;
int rz_hedge_init(RZHedge *, const RZScope *, uint64_t, uint64_t);
int rz_hedge_start(RZHedge *, uint64_t);
int rz_hedge_complete(RZHedge *, unsigned, uint64_t, bool, unsigned);
int rz_hedge_result(const RZHedge *, unsigned *);
bool rz_hedge_quiescent(const RZHedge *);
typedef struct {
  bool live;
  unsigned client, id;
  int delta, result;
  uint64_t born;
} RZEntry;
typedef struct {
  RZEntry entries[4];
  uint64_t window, last, fence;
  int total;
} RZReceiver;
int rz_receiver_init(RZReceiver *, uint64_t, uint64_t);
int rz_receive(RZReceiver *, uint64_t, unsigned, unsigned, uint64_t, int, int *,
               bool *);
typedef struct {
  const RZScope *scope;
  MsgHub *hub;
  RZQueue pending;
  RZBucket limiter;
  RZGate breaker;
  RZReceiver receiver;
} RZService;
int rz_service_init(RZService *, const RZScope *, MsgHub *, uint64_t);
int rz_service_submit(RZService *, uint64_t, unsigned, unsigned, uint64_t, int,
                      int *);
int rz_service_flush(RZService *);
#endif

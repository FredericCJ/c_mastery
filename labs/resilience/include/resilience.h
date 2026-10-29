#ifndef CMASTERY_RESILIENCE_H
#define CMASTERY_RESILIENCE_H
#include "messaging.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum rz_status {
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
};
struct rz_scope {
  const struct rz_scope *parent;
  uint64_t deadline;
  unsigned depth;
  atomic_bool cancelled;
};
int rz_scope_init(struct rz_scope *, const struct rz_scope *, uint64_t, uint64_t);
void rz_cancel(struct rz_scope *);
int rz_check(const struct rz_scope *, uint64_t);
int rz_remaining(const struct rz_scope *, uint64_t, uint64_t *);
struct rz_queue {
  unsigned values[4];
  size_t head, count;
  bool closed;
};
enum rz_policy {
  RZ_BLOCK,
  RZ_REJECT,
  RZ_DROP,
  RZ_OVERWRITE,
  RZ_DEGRADE
};
struct rz_offer {
  const struct rz_scope *scope;
  unsigned value;
  bool pending;
};
void rz_queue_init(struct rz_queue *);
int rz_offer(struct rz_queue *, struct rz_offer *, enum rz_policy, uint64_t);
int rz_pop(struct rz_queue *, unsigned *);
int rz_forward(struct rz_queue *, struct msg_hub *, unsigned);
struct rz_timer {
  uint64_t due;
  int next, previous;
  unsigned generation;
  bool active;
};
struct rz_wheel {
  uint64_t now;
  int head[4];
  struct rz_timer slots[4];
};
struct rz_timer_id {
  const struct rz_wheel *owner;
  unsigned slot, generation;
};
void rz_wheel_init(struct rz_wheel *, uint64_t);
int rz_schedule(struct rz_wheel *, uint64_t, struct rz_timer_id *);
int rz_unschedule(struct rz_wheel *, struct rz_timer_id);
int rz_tick(struct rz_wheel *, unsigned *);
struct rz_bucket {
  uint64_t capacity, rate, tokens, last;
};
int rz_bucket_init(struct rz_bucket *, uint64_t, uint64_t, uint64_t);
int rz_consume(struct rz_bucket *, uint64_t, uint64_t);
struct rz_retry {
  const struct rz_scope *scope;
  unsigned attempts, maximum;
  uint64_t base, cap;
  bool idempotent;
};
int rz_retry_init(struct rz_retry *, const struct rz_scope *, unsigned, uint64_t, uint64_t,
                  bool);
int rz_retry_next(struct rz_retry *, uint64_t, uint64_t, uint64_t *);
struct rz_gate {
  unsigned phase, failures, limit, epoch;
  uint64_t until, last;
  bool active[2], retired;
  unsigned generations[2], epochs[2];
};
struct rz_permit {
  const struct rz_gate *owner;
  unsigned slot, generation;
};
int rz_gate_init(struct rz_gate *, unsigned, uint64_t);
int rz_admit(struct rz_gate *, uint64_t, struct rz_permit *);
int rz_finish(struct rz_gate *, struct rz_permit, uint64_t, int);
struct rz_hedge {
  const struct rz_scope *scope;
  uint64_t hedge_at;
  unsigned started, completed, cancel_requested;
  int winner;
  unsigned value;
};
int rz_hedge_init(struct rz_hedge *, const struct rz_scope *, uint64_t, uint64_t);
int rz_hedge_start(struct rz_hedge *, uint64_t);
int rz_hedge_complete(struct rz_hedge *, unsigned, uint64_t, bool, unsigned);
int rz_hedge_result(const struct rz_hedge *, unsigned *);
bool rz_hedge_quiescent(const struct rz_hedge *);
struct rz_entry {
  bool live;
  unsigned client, id;
  int delta, result;
  uint64_t born;
};
struct rz_receiver {
  struct rz_entry entries[4];
  uint64_t window, last, fence;
  int total;
};
int rz_receiver_init(struct rz_receiver *, uint64_t, uint64_t);
int rz_receive(struct rz_receiver *, uint64_t, unsigned, unsigned, uint64_t, int, int *,
               bool *);
struct rz_service {
  const struct rz_scope *scope;
  struct msg_hub *hub;
  struct rz_queue pending;
  struct rz_bucket limiter;
  struct rz_gate breaker;
  struct rz_receiver receiver;
};
int rz_service_init(struct rz_service *, const struct rz_scope *, struct msg_hub *, uint64_t);
int rz_service_submit(struct rz_service *, uint64_t, unsigned, unsigned, uint64_t, int,
                      int *);
int rz_service_flush(struct rz_service *);
#endif

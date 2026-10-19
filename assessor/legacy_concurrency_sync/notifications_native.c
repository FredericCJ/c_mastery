#define _GNU_SOURCE
#include "notifications_native.h"
#include <sched.h>
#include <time.h>
#ifndef CDNN_MUTANT
#define CDNN_MUTANT 0
#endif
static int now(uint64_t *out) {
  struct timespec value;
  if (clock_gettime(CLOCK_MONOTONIC, &value) || value.tv_sec < 0 ||
      (uint64_t)value.tv_sec > (UINT64_MAX - (uint64_t)value.tv_nsec) / UINT64_C(1000000000))
    return 0;
  *out = (uint64_t)value.tv_sec * UINT64_C(1000000000) + (uint64_t)value.tv_nsec;
  return 1;
}
int cdnn_init(CDNNative *p, CDNHandle h) {
  if (!p || !h.owner || !h.owner->attached || h.generation != h.owner->generation)
    return CMX_INVALID;
  if (pthread_mutex_init(&p->exclusion, 0)) return CMX_INVALID;
  p->recipient = h; p->live = 1; return CMX_OK;
}
static int invoke(CDNNative *p, unsigned post, uint32_t amount, CDNNObservation *out) {
  uint64_t before, entered, after; CDNNObservation value = {0};
  if (!p || !p->live || !out || !amount || !now(&before)) return CMX_INVALID;
  if (pthread_mutex_lock(&p->exclusion)) return CMX_INVALID;
  if (!now(&entered)) { (void)pthread_mutex_unlock(&p->exclusion); return CMX_INVALID; }
  value.cpu = sched_getcpu();
  if (post) {
    CDNPost details;
    value.status = cdn_post(p->recipient, CDNN_MUTANT == 1 ? 1 : amount, &details);
    if (value.status == CMX_OK) value.count = amount;
    if (value.status == CMX_FULL) {
      CDNFault fault;
      if (cdn_fault(p->recipient, &fault) != CMX_OK) value.status = CMX_INVALID;
      else {
        value.refused = fault.count; value.fault_sequence = fault.sequence;
        if (cdn_ack_fault(p->recipient, fault.sequence) != CMX_OK) value.status = CMX_INVALID;
      }
    }
  } else {
    CDNotify *n = p->recipient.owner;
    value.status = cdn_wait(p->recipient, amount, cmx_at(n->kernel->now));
    if (value.status == CMX_OK || value.status == CMX_TIMEOUT || value.status == CMX_FULL)
      value.status = cdn_result(p->recipient, &value.count);
  }
  int measured = now(&after);
  int unlocked = pthread_mutex_unlock(&p->exclusion);
  if (!measured || unlocked || value.cpu < 0) return CMX_INVALID;
  value.lock_wait_ns = entered - before; value.protected_work_ns = after - entered;
  *out = value; return value.status;
}
int cdnn_post(CDNNative *p, uint32_t n, CDNNObservation *out) { return invoke(p, 1, n, out); }
int cdnn_take(CDNNative *p, uint32_t n, CDNNObservation *out) { return invoke(p, 0, n, out); }
int cdnn_snapshot(CDNNative *p, CDNNState *out) {
  if (!p || !p->live || !out || pthread_mutex_lock(&p->exclusion)) return CMX_INVALID;
  CDNotify *n = p->recipient.owner;
  CDNNState value = {n->accepted,n->consumed,n->cancelled,n->rejected,
                    n->kernel->task[n->task].notifications,cdn_invariant(n)};
  if (pthread_mutex_unlock(&p->exclusion)) return CMX_INVALID;
  *out = value; return CMX_OK;
}
int cdnn_destroy(CDNNative *p) {
  if (!p || !p->live || pthread_mutex_destroy(&p->exclusion)) return CMX_INVALID;
  p->live = 0; return CMX_OK;
}

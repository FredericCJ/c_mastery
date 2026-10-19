#include "port.h"
#ifndef SR_MUTANT
#define SR_MUTANT 0
#endif
static int identity(const SRPort *s) {
  return s && s->bound && s->kernel && s->task < CMX_TASKS &&
    (SR_MUTANT == 3 ||
     s->kernel->task[s->task].generation == s->generation) &&
    s->kernel->task[s->task].state != CMX_UNUSED;
}
static int owned_wait(const SRPort *s) {
  const cmx_task *t = &s->kernel->task[s->task];
  return s->sleeping && t->state == CMX_WAITING &&
    t->wait == CMX_W_SLEEP && !t->deadline.infinite &&
    t->deadline.at == s->wake && t->wait_sequence == s->wait_sequence;
}
enum cmx_result sr_init(SRPort *s, cmx_kernel *k, unsigned task,
                       uint32_t quota, uint32_t period) {
  if (!s || !k || task >= CMX_TASKS ||
      (k->task[task].state != CMX_READY && k->task[task].state != CMX_RUNNING) ||
      !quota || quota > 16 || quota > period || period >= UINT32_C(0x80000000))
    return CMX_INVALID;
  SRPort next = {0};
  enum cmx_result r = ss_init(&next.budget, quota, period, k->now, 16, SS_REJECT);
  if (r != CMX_OK) return r;
  next.kernel = k; next.task = task; next.generation = k->task[task].generation;
  next.bound = 1; next.last = k->now; *s = next; return CMX_OK;
}
enum cmx_result sr_tick(SRPort *s, uint32_t boundary) {
  if (!identity(s) || s->last == UINT32_MAX ||
      boundary != s->last + 1 || s->kernel->now != s->last)
    return CMX_INVALID;
  cmx_kernel *k = s->kernel; cmx_task *t = &k->task[s->task];
  if (s->closed) {s->last = boundary; return CMX_OK;}
  if (boundary > UINT32_MAX - s->budget.core.period) return CMX_FULL;
  if (!ss_invariant(&s->budget) || t->state == CMX_EXITED ||
      (t->state == CMX_WAITING && !owned_wait(s)) ||
      (s->sleeping && t->state != CMX_WAITING && t->result != CMX_OK))
    return CMX_BROKEN;
  SRPort next = *s;
  enum cmx_result r = ss_update(&next.budget, boundary);
  if (r != CMX_OK) return r;
  if (t->state != CMX_WAITING) next.sleeping = 0;
  if (k->current == (int)s->task && t->state == CMX_RUNNING) {
    if (next.charged == UINT32_MAX) return CMX_FULL;
    if (SR_MUTANT != 1) {
      r = ss_charge(&next.budget, boundary, 1);
      if (r != CMX_OK) return r;
    }
    next.charged++;
    if (!next.budget.core.remaining && SR_MUTANT != 2) {
      if (!next.budget.core.count || next.throttles == UINT32_MAX) return CMX_FULL;
      uint32_t wake = next.budget.core.at[0];
      if (SR_MUTANT == 4) wake = boundary;
      r = cmx_sleep(k, s->task, cmx_at(wake));
      if (r != CMX_BLOCKED) return r;
      next.sleeping = 1; next.wait_sequence = t->wait_sequence;
      next.wake = wake; next.throttles++;
    }
  }
  next.last = boundary; *s = next; return CMX_OK;
}
enum cmx_result sr_close(SRPort *s) {
  if (!identity(s)) return CMX_INVALID;
  if (s->closed) return CMX_OK;
  cmx_task *t = &s->kernel->task[s->task];
  if (t->state == CMX_EXITED ||
      (t->state == CMX_WAITING && !owned_wait(s))) return CMX_BROKEN;
  if (t->state == CMX_WAITING) {
    enum cmx_result r = cmx_cancel(s->kernel, s->task);
    if (r != CMX_OK) return r;
  }
  s->sleeping = 0; s->closed = 1; return CMX_OK;
}
enum cmx_result sr_detach(SRPort *s) {
  if (!identity(s) || !s->closed ||
      s->kernel->task[s->task].state != CMX_EXITED ||
      s->kernel->current == (int)s->task) return CMX_INVALID;
  s->bound = 0; s->kernel = 0; return CMX_OK;
}

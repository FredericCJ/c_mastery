#include "ceiling.h"
#ifndef SC_MUTANT
#define SC_MUTANT 0
#endif
static int config_valid(const struct sc_config *c) {
  if (!c || !c->tasks || c->tasks > SC_TASKS || !c->resources ||
      c->resources > SC_RESOURCES || c->strict_nesting > 1)
    return 0;
  for (unsigned t = 0; t < c->tasks; t++) {
    if (c->priority[t] > 255)
      return 0;
    for (unsigned q = 0; q < t; q++)
      if (c->priority[q] == c->priority[t])
        return 0;
  }
  for (unsigned r = 0; r < c->resources; r++) {
    if (!c->users[r] || (c->users[r] >> c->tasks))
      return 0;
    unsigned ceiling = 255;
    for (unsigned t = 0; t < c->tasks; t++)
      if ((c->users[r] & (1u << t)) && c->priority[t] < ceiling)
        ceiling = c->priority[t];
    if (c->ceiling[r] != ceiling && SC_MUTANT != 1)
      return 0;
  }
  return 1;
}
enum cmx_result sc_init(struct sc_world *out, const struct sc_config *c, enum sc_mode mode) {
  if (!out || !config_valid(c) || (unsigned)mode > SC_IMMEDIATE)
    return CMX_INVALID;
  struct sc_world w = {0};
  w.config = *c;
  w.mode = mode;
  cmx_init(&w.kernel, CMX_FIXED);
  for (unsigned t = 0; t < c->tasks; t++)
    if (cmx_task_add(&w.kernel, c->priority[t], 0,
                     (uintptr_t)(0x1000u + 0x100u * t), 256) != (int)t)
      return CMX_BROKEN;
  for (unsigned r = 0; r < c->resources; r++)
    if (cmx_mutex_init(&w.kernel, &w.mutex[r], c->ceiling[r],
                       mode == SC_IMMEDIATE) != CMX_OK)
      return CMX_BROKEN;
  *out = w;
  for (unsigned r = 0; r < c->resources; r++)
    out->kernel.mutex[r] = &out->mutex[r];
  return CMX_OK;
}
static int valid(const struct sc_world *w, unsigned t) {
  return w && t < w->config.tasks;
}
enum cmx_result sc_lock(struct sc_world *w, unsigned t, unsigned r, cmx_deadline d,
                        enum sc_context context) {
  if (!valid(w, t) || r >= w->config.resources || context != SC_TASK_CONTEXT ||
      d.infinite > 1)
    return CMX_INVALID;
  if (w->pending[t])
    return CMX_BLOCKED;
  if (!(w->config.users[r] & (1u << t)) && SC_MUTANT != 2)
    return CMX_INVALID;
  if (w->held[t] & (1u << r))
    return CMX_DEADLOCK;
  if ((w->held[t] >> r) && SC_MUTANT != 3)
    return CMX_DEADLOCK;
  enum cmx_result result = cmx_mutex_lock(&w->kernel, &w->mutex[r], t, d);
  if (result == CMX_OK)
    w->held[t] |= 1u << r;
  else if (result == CMX_BLOCKED)
    w->pending[t] = r + 1;
  return result;
}
enum cmx_result sc_unlock(struct sc_world *w, unsigned t, unsigned r,
                          enum sc_context context) {
  if (!valid(w, t) || r >= w->config.resources || context != SC_TASK_CONTEXT)
    return CMX_INVALID;
  if (w->pending[t])
    return CMX_BLOCKED;
  if (!(w->held[t] & (1u << r)))
    return CMX_NOT_OWNER;
  if (w->config.strict_nesting && (w->held[t] >> (r + 1)))
    return CMX_INVALID;
  enum cmx_result result = cmx_mutex_unlock(&w->kernel, &w->mutex[r], t);
  if (result == CMX_OK) {
    w->held[t] &= ~(1u << r);
    if (SC_MUTANT == 4)
      w->kernel.task[t].priority = w->kernel.task[t].base_priority;
  }
  return result;
}
enum cmx_result sc_poll(struct sc_world *w, unsigned t) {
  if (!valid(w, t) || !w->pending[t])
    return CMX_INVALID;
  if (w->kernel.task[t].state == CMX_WAITING)
    return CMX_BLOCKED;
  unsigned r = w->pending[t] - 1;
  enum cmx_result result = w->kernel.task[t].result;
  if (result == CMX_OK) {
    if (w->mutex[r].owner != (int)t)
      return CMX_BROKEN;
    w->held[t] |= 1u << r;
  }
  w->pending[t] = 0;
  return result;
}
enum cmx_result sc_cancel(struct sc_world *w, unsigned t) {
  if (!valid(w, t) || !w->pending[t])
    return CMX_INVALID;
  unsigned old[SC_TASKS];
  for (unsigned i = 0; i < w->config.tasks; i++)
    old[i] = w->kernel.task[i].priority;
  enum cmx_result result = cmx_cancel(&w->kernel, t);
  if (SC_MUTANT == 5)
    for (unsigned i = 0; i < w->config.tasks; i++)
      w->kernel.task[i].priority = old[i];
  return result;
}
enum cmx_result sc_tick(struct sc_world *w, uint32_t now) {
  if (!w || (uint32_t)(now - w->kernel.now) >= UINT32_C(0x80000000))
    return CMX_INVALID;
  cmx_tick(&w->kernel, now);
  return CMX_OK;
}
enum cmx_result sc_set_base(struct sc_world *w, unsigned t, unsigned p) {
  if (!valid(w, t) || p > 255)
    return CMX_INVALID;
  if (p != w->config.priority[t]) {
    if (SC_MUTANT != 6)
      return CMX_INVALID;
    w->kernel.task[t].base_priority = p;
    return CMX_OK;
  }
  return CMX_OK;
}
int sc_invariant(const struct sc_world *w) {
  if (!w || !config_valid(&w->config) || !cmx_invariant(&w->kernel))
    return 0;
  unsigned priority[SC_TASKS];
  for (unsigned t = 0; t < w->config.tasks; t++) {
    priority[t] = w->config.priority[t];
    if (w->kernel.task[t].base_priority != priority[t] ||
        (w->held[t] >> w->config.resources) ||
        w->pending[t] > w->config.resources)
      return 0;
    for (unsigned r = 0; r < w->config.resources; r++)
      if (w->held[t] & (1u << r))
        if (w->mutex[r].owner != (int)t || !(w->config.users[r] & (1u << t)))
          return 0;
  }
  for (unsigned r = 0; r < w->config.resources; r++) {
    int owner = w->mutex[r].owner;
    if (w->kernel.mutex[r] != &w->mutex[r] ||
        w->mutex[r].ceiling != w->config.ceiling[r])
      return 0;
    if (owner >= 0) {
      unsigned t = (unsigned)owner;
      if (t >= w->config.tasks ||
          (!(w->held[t] & (1u << r)) && w->pending[t] != r + 1))
        return 0;
      if (w->mode == SC_IMMEDIATE && w->config.ceiling[r] < priority[t])
        priority[t] = w->config.ceiling[r];
    }
  }
  for (unsigned pass = 0; pass < w->config.tasks; pass++)
    for (unsigned t = 0; t < w->config.tasks; t++)
      if (w->pending[t] && w->kernel.task[t].state == CMX_WAITING) {
        int owner = w->mutex[w->pending[t] - 1].owner;
        if (owner >= 0 && priority[owner] > priority[t])
          priority[owner] = priority[t];
      }
  for (unsigned t = 0; t < w->config.tasks; t++)
    if (w->kernel.task[t].priority != priority[t])
      return 0;
  return 1;
}
int sc_choose(const struct sc_world *w, unsigned mask, int current) {
  if (!w || (mask >> w->config.tasks) || current < CMX_NONE ||
      current >= (int)w->config.tasks)
    return CMX_NONE;
  int best = CMX_NONE;
  for (unsigned t = 0; t < w->config.tasks; t++)
    if ((mask & (1u << t)) && (w->kernel.task[t].state == CMX_READY ||
                               w->kernel.task[t].state == CMX_RUNNING))
      if (best < 0 ||
          w->kernel.task[t].priority < w->kernel.task[best].priority)
        best = (int)t;
  if (SC_MUTANT != 7 && best >= 0 && current >= 0 &&
      (mask & (1u << (unsigned)current)) &&
      (w->kernel.task[current].state == CMX_READY ||
       w->kernel.task[current].state == CMX_RUNNING) &&
      w->kernel.task[current].priority == w->kernel.task[best].priority)
    return current;
  return best;
}

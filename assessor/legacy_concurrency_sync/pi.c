#include "pi.h"
#ifndef CDPI_MUTANT
#define CDPI_MUTANT 0
#endif
static int live(const cmx_kernel *k, unsigned t) {
  return t < CMX_TASKS && k->task[t].state != CMX_UNUSED &&
         k->task[t].state != CMX_EXITED;
}
static int registered(const cmx_kernel *k, const cmx_mutex *m) {
  for (unsigned i = 0; i < k->mutex_count; ++i)
    if (k->mutex[i] == m) return 1;
  return 0;
}
enum cmx_result cdpi_reprioritize(cmx_kernel *k, unsigned task,
                                 unsigned new_base, CDPIStats *out) {
  if (!k || !out || !live(k, task) || new_base > 255 ||
      k->mutex_count > CMX_MUTEXES) return CMX_INVALID;
  uint32_t reach[CMX_TASKS];
  unsigned priority[CMX_TASKS];
  CDPIStats stats = {0, 0, 0, 0};
  for (unsigned t = 0; t < CMX_TASKS; ++t) {
    reach[t] = 0;
    priority[t] = t == task ? new_base : k->task[t].base_priority;
    if (CDPI_MUTANT == 1 && live(k, t) && k->task[t].priority < priority[t])
      priority[t] = k->task[t].priority;
    if (live(k, t)) ++stats.vertices;
  }
  for (unsigned i = 0; i < k->mutex_count; ++i) {
    const cmx_mutex *m = k->mutex[i];
    if (!m || m->closed || m->owner < CMX_NONE || m->owner >= (int)CMX_TASKS)
      return CMX_INVALID;
    if (m->owner < 0) continue;
    unsigned owner = (unsigned)m->owner;
    if (!live(k, owner) || m->ceiling > 255) return CMX_INVALID;
    if (owner == task && m->use_ceiling && new_base < m->ceiling)
      return CMX_INVALID;
    if (m->use_ceiling && priority[owner] > m->ceiling)
      priority[owner] = m->ceiling;
  }
  for (unsigned t = 0; t < CMX_TASKS; ++t) {
    const cmx_task *p = &k->task[t];
    if (p->state != CMX_WAITING || p->wait != CMX_W_MUTEX) continue;
    const cmx_mutex *m = p->object;
    if (!m || !registered(k, m) || m->owner < 0 ||
        (t == task && m->use_ceiling && new_base < m->ceiling))
      return CMX_INVALID;
    reach[t] = UINT32_C(1) << (unsigned)m->owner;
    ++stats.edges;
  }
  /* Reachability derives each owner's donor set independently of CMX's
   * repeated relaxation implementation. A smaller number is more urgent. */
  for (unsigned via = 0; via < CMX_TASKS; ++via)
    for (unsigned from = 0; from < CMX_TASKS; ++from) {
      ++stats.closure_probes;
      if (CDPI_MUTANT != 2 && (reach[from] & (UINT32_C(1) << via)))
        reach[from] |= reach[via];
    }
  for (unsigned t = 0; t < CMX_TASKS; ++t)
    if (reach[t] & (UINT32_C(1) << t)) return CMX_DEADLOCK;
  unsigned seeds[CMX_TASKS];
  for (unsigned t = 0; t < CMX_TASKS; ++t) seeds[t] = priority[t];
  for (unsigned from = 0; from < CMX_TASKS; ++from)
    for (unsigned to = 0; to < CMX_TASKS; ++to)
      if (reach[from] & (UINT32_C(1) << to)) {
        ++stats.donated_pairs;
        if (priority[to] > seeds[from]) priority[to] = seeds[from];
      }
  k->task[task].base_priority = new_base;
  for (unsigned t = 0; t < CMX_TASKS; ++t) {
    if (CDPI_MUTANT == 3 && live(k, t)) k->task[t].base_priority = priority[t];
    k->task[t].priority = priority[t];
  }
  *out = stats;
  return CMX_OK;
}

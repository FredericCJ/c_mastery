#include "cmx_advanced.h"
enum cmx_result cmx_priority_order(const cmx_job *j, unsigned n, unsigned dm,
                                   unsigned *order) {
  if (!j || !order || !n || n > CMX_TASKS)
    return CMX_INVALID;
  for (unsigned i = 0; i < n; i++) {
    if (!j[i].period || !j[i].deadline)
      return CMX_INVALID;
    order[i] = i;
    unsigned at = i;
    while (at) {
      uint32_t a = dm ? j[order[at]].deadline : j[order[at]].period,
               b = dm ? j[order[at - 1]].deadline : j[order[at - 1]].period;
      if (a >= b)
        break;
      unsigned tmp = order[at - 1];
      order[at - 1] = order[at];
      order[at] = tmp;
      at--;
    }
  }
  return CMX_OK;
}
enum cmx_result cmx_partition(const cmx_job *j, unsigned n, unsigned cpus,
                              uint32_t h, unsigned *cpu) {
  if (!j || !cpu || !n || n > CMX_TASKS || !cpus || cpus > 4 || !h ||
      h > 1000000)
    return CMX_INVALID;
  uint32_t used[4] = {0};
  for (unsigned i = 0; i < n; i++) {
    if (!j[i].period || h % j[i].period || j[i].deadline != j[i].period ||
        j[i].jitter || j[i].blocking || j[i].cost > j[i].period)
      return CMX_INVALID;
    uint32_t cost = (h / j[i].period) * j[i].cost;
    unsigned c;
    for (c = 0; c < cpus; c++)
      if (cost <= h - used[c]) {
        used[c] += cost;
        cpu[i] = c;
        break;
      }
    if (c == cpus)
      return CMX_FULL;
  }
  return CMX_OK;
}
enum cmx_result cmx_simulate(const cmx_job *j, unsigned n, unsigned cpus,
                             const unsigned *part, uint32_t h,
                             cmx_sim_result *out) {
  if (!j || !out || !n || n > CMX_TASKS || !cpus || cpus > 4 || !h ||
      h > 1000000)
    return CMX_INVALID;
  uint32_t remaining[CMX_TASKS] = {0}, deadline[CMX_TASKS] = {0};
  unsigned reported[CMX_TASKS] = {0}, last[CMX_TASKS];
  for (unsigned i = 0; i < n; i++) {
    if (!j[i].period || !j[i].deadline || j[i].deadline > j[i].period ||
        j[i].cost > 1000000 || j[i].jitter || j[i].blocking ||
        (part && part[i] >= cpus))
      return CMX_INVALID;
    last[i] = cpus;
  }
  *out = (cmx_sim_result){0};
  for (uint32_t t = 0; t <= h; t++) {
    for (unsigned i = 0; i < n; i++) {
      if (remaining[i] && deadline[i] == t && !reported[i]) {
        out->misses++;
        reported[i] = 1;
      }
      if (t < h && t % j[i].period == 0) {
        remaining[i] = j[i].cost;
        deadline[i] = t + j[i].deadline;
        reported[i] = 0;
      }
    }
    if (t == h)
      break;
    unsigned chosen = 0;
    for (unsigned c = 0; c < cpus; c++) {
      int best = -1;
      for (unsigned i = 0; i < n; i++)
        if (remaining[i] && !(chosen & (1u << i)) && (!part || part[i] == c))
          if (best < 0 || deadline[i] < deadline[best])
            best = (int)i;
      if (best >= 0) {
        unsigned b = (unsigned)best;
        chosen |= 1u << b;
        if (last[b] < cpus && last[b] != c)
          out->migrations++;
        last[b] = c;
        remaining[b]--;
        out->work++;
        if (!remaining[b])
          out->completed++;
      }
    }
  }
  return out->misses ? CMX_TIMEOUT : CMX_OK;
}
enum cmx_result cmx_server_init(cmx_server *s, uint32_t q, uint32_t p) {
  if (!q || q > p || p >= 0x80000000u)
    return CMX_INVALID;
  *s = (cmx_server){0};
  s->budget = q;
  s->period = p;
  s->remaining = q;
  return CMX_OK;
}
enum cmx_result cmx_server_update(cmx_server *s, uint32_t now) {
  if ((uint32_t)(now - s->last) >= 0x80000000u)
    return CMX_INVALID;
  s->last = now;
  while (s->count && (uint32_t)(now - s->at[0]) < 0x80000000u) {
    if (s->amount[0] > s->budget - s->remaining)
      return CMX_BROKEN;
    s->remaining += s->amount[0];
    for (unsigned i = 1; i < s->count; i++) {
      s->at[i - 1] = s->at[i];
      s->amount[i - 1] = s->amount[i];
    }
    s->count--;
  }
  return CMX_OK;
}
enum cmx_result cmx_server_charge(cmx_server *s, uint32_t now, uint32_t n) {
  enum cmx_result r = cmx_server_update(s, now);
  if (r != CMX_OK)
    return r;
  if (!n)
    return CMX_INVALID;
  if (n > s->remaining)
    return CMX_FULL;
  uint32_t when = now + s->period;
  if (s->count && s->at[s->count - 1] == when)
    s->amount[s->count - 1] += n;
  else {
    if (s->count == 16)
      return CMX_FULL;
    s->at[s->count] = when;
    s->amount[s->count++] = n;
  }
  s->remaining -= n;
  return CMX_OK;
}
void cmx_identity_init(cmx_identity_pool *p) {
  *p = (cmx_identity_pool){0};
  for (unsigned i = 0; i < 16; i++)
    p->generation[i] = 1;
}
uint32_t cmx_identity_take(cmx_identity_pool *p) {
  for (unsigned i = 0; i < 16; i++)
    if (!p->used[i] && p->generation[i] <= 65535) {
      p->used[i] = 1;
      return (p->generation[i] << 8) | (i + 1);
    }
  return 0;
}
int cmx_identity_valid(const cmx_identity_pool *p, uint32_t h) {
  unsigned i = (h & 255u);
  return i && i <= 16 && p->used[i - 1] && (h >> 8) == p->generation[i - 1];
}
enum cmx_result cmx_identity_put(cmx_identity_pool *p, uint32_t h) {
  if (!cmx_identity_valid(p, h))
    return CMX_INVALID;
  unsigned i = (h & 255u) - 1;
  p->used[i] = 0;
  p->generation[i]++;
  return CMX_OK;
}
void cmx_reservation_init(cmx_reservation *q) {
  atomic_init(&q->head, 0);
  atomic_init(&q->tail, 0);
  for (unsigned i = 0; i < 4; i++)
    atomic_init(&q->ready[i], 0);
}
enum cmx_result cmx_reserve(cmx_reservation *q, uint32_t *ticket) {
  unsigned h = atomic_load(&q->head);
  for (;;) {
    unsigned t = atomic_load(&q->tail);
    if (h == UINT32_MAX)
      return CMX_FULL;
    if (h - t >= 4)
      return CMX_FULL;
    if (atomic_compare_exchange_weak(&q->head, &h, h + 1)) {
      *ticket = h;
      return CMX_OK;
    }
  }
}
enum cmx_result cmx_publish(cmx_reservation *q, uint32_t ticket,
                            uint32_t value) {
  unsigned h = atomic_load(&q->head), t = atomic_load(&q->tail);
  if (ticket < t || ticket >= h || atomic_load(&q->ready[ticket & 3]))
    return CMX_INVALID;
  q->data[ticket & 3] = value;
  atomic_store(&q->ready[ticket & 3], 1);
  return CMX_OK;
}
enum cmx_result cmx_reserved_pop(cmx_reservation *q, uint32_t *value) {
  unsigned t = atomic_load(&q->tail);
  if (t == atomic_load(&q->head) || !atomic_load(&q->ready[t & 3]))
    return CMX_RETRY;
  *value = q->data[t & 3];
  atomic_store(&q->ready[t & 3], 0);
  atomic_store(&q->tail, t + 1);
  return CMX_OK;
}
typedef struct {
  uint32_t v[8];
  unsigned used;
} fifo;
static int search(const cmx_history_op *op, unsigned n, unsigned cap,
                  unsigned used, fifo q, unsigned *w, unsigned depth) {
  if (depth == n)
    return 1;
  for (unsigned i = 0; i < n; i++) {
    if (used & (1u << i))
      continue;
    unsigned blocked = 0;
    for (unsigned j = 0; j < n; j++)
      if (!(used & (1u << j)) && j != i && op[j].respond < op[i].invoke)
        blocked = 1;
    if (blocked)
      continue;
    fifo next = q;
    const cmx_history_op *p = &op[i];
    if (p->op == 0) {
      if (p->status == CMX_FULL) {
        if (q.used != cap)
          continue;
      } else if (p->status == CMX_OK && q.used < cap)
        next.v[next.used++] = p->value;
      else
        continue;
    } else {
      if (p->status == CMX_RETRY) {
        if (q.used)
          continue;
      } else if (p->status == CMX_OK && q.used && q.v[0] == p->value) {
        for (unsigned j = 1; j < q.used; j++)
          next.v[j - 1] = q.v[j];
        next.used--;
      } else
        continue;
    }
    w[depth] = i;
    if (search(op, n, cap, used | (1u << i), next, w, depth + 1))
      return 1;
  }
  return 0;
}
enum cmx_result cmx_history_check(const cmx_history_op *op, unsigned n,
                                  unsigned cap, unsigned *w) {
  if (!op || !w || !n || n > 8 || !cap || cap > 8)
    return CMX_INVALID;
  for (unsigned i = 0; i < n; i++) {
    if (op[i].op > 1 || op[i].invoke >= op[i].respond)
      return CMX_INVALID;
    for (unsigned j = 0; j < i; j++)
      if (op[j].participant == op[i].participant &&
          op[j].invoke < op[i].respond && op[i].invoke < op[j].respond)
        return CMX_INVALID;
  }
  fifo q = {{0}, 0};
  return search(op, n, cap, 0, q, w, 0) ? CMX_OK : CMX_BROKEN;
}
void cmx_rcu_init(cmx_rcu *r, unsigned root) {
  atomic_init(&r->current, root);
  for (unsigned i = 0; i < CMX_HP_THREADS; i++)
    atomic_init(&r->active[i], 0);
}
enum cmx_result cmx_rcu_read(cmx_rcu *r, unsigned p, unsigned *root) {
  if (p >= CMX_HP_THREADS || !root)
    return CMX_INVALID;
  unsigned zero = 0;
  if (!atomic_compare_exchange_strong(&r->active[p], &zero, 1))
    return CMX_INVALID;
  *root = atomic_load(&r->current);
  return CMX_OK;
}
enum cmx_result cmx_rcu_leave(cmx_rcu *r, unsigned p) {
  if (p >= CMX_HP_THREADS)
    return CMX_INVALID;
  return atomic_exchange(&r->active[p], 0) ? CMX_OK : CMX_INVALID;
}
unsigned cmx_rcu_publish(cmx_rcu *r, unsigned root) {
  return atomic_exchange(&r->current, root);
}
int cmx_rcu_quiescent(const cmx_rcu *r) {
  for (unsigned i = 0; i < CMX_HP_THREADS; i++)
    if (atomic_load(&r->active[i]))
      return 0;
  return 1;
}

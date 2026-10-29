#include "completion.h"
#include <limits.h>
#define CANCEL_BIT (UINT64_C(1) << 63)
static struct co_slot *find(struct co_pool *p, uint64_t token) {
  uint64_t target = token & ~CANCEL_BIT;
  unsigned index = (unsigned)(target & 255u);
  if (!p || index < 1 || index > 4)
    return NULL;
  struct co_slot *s = &p->slot[index - 1];
#ifndef CO_MUTANT_TOKEN
  if (s->token != target)
    return NULL;
#endif
  return s;
}
int co_init(struct co_pool *p, unsigned char *buffers[4], size_t capacity,
            unsigned limit) {
  if (!p || !buffers || !capacity || capacity > 4096 || !limit ||
      limit > 0x7fffffffu)
    return -1;
  for (unsigned i = 0; i < 4; ++i) {
    if (!buffers[i])
      return -1;
    for (unsigned j = 0; j < i; ++j)
      if (buffers[i] == buffers[j])
        return -1;
  }
  *p = (struct co_pool){.generation_limit = limit};
  for (unsigned i = 0; i < 4; ++i) {
    p->slot[i].buffer = buffers[i];
    p->slot[i].capacity = capacity;
  }
  return 0;
}
int co_acquire(struct co_pool *p, unsigned index, size_t requested, uint64_t *token) {
  if (!p || index >= 4 || !token || !requested ||
      requested > p->slot[index].capacity)
    return -1;
  struct co_slot *s = &p->slot[index];
  if (s->state != CO_FREE)
    return 0;
  if (s->generation == p->generation_limit) {
    s->state = CO_RETIRED;
    return 0;
  }
  ++s->generation;
  s->token = ((uint64_t)s->generation << 8) | (index + 1u);
  s->requested = requested;
  s->result = 0;
  s->cancel_pending = 0;
  s->cancel_seen = 0;
  s->cancel_result = 0;
  s->state = CO_BUSY;
  *token = s->token;
  return 1;
}
int co_cancel(struct co_pool *p, uint64_t token, uint64_t *cancel) {
  struct co_slot *s = find(p, token);
  if (!cancel || (token & CANCEL_BIT))
    return -1;
  if (!s || s->state != CO_BUSY || s->cancel_pending || s->cancel_seen)
    return 0;
  s->cancel_pending = 1;
  *cancel = token | CANCEL_BIT;
  return 1;
}
int co_complete(struct co_pool *p, uint64_t token, int result, unsigned flags) {
  struct co_slot *s = find(p, token);
  if (!s)
    return 0;
  /* This bounded adapter handles single-shot READ/READ_FIXED and one cancel.
   * Multishot CQEs need a distinct contract; never silently drop MORE. */
  if (flags)
    return -1;
  if (token & CANCEL_BIT) {
    if (!s->cancel_pending || (s->state != CO_BUSY && s->state != CO_DONE))
      return 0;
    s->cancel_pending = 0;
    s->cancel_seen = 1;
    s->cancel_result = result;
#ifdef CO_MUTANT_CANCEL_RELEASE
    s->state = CO_DONE;
#endif
    return 1;
  }
  if (s->state != CO_BUSY)
    return 0;
  if (result >= 0 && (size_t)result > s->requested)
    return -1;
  s->result = result;
  s->state = CO_DONE;
  return 1;
}
int co_release(struct co_pool *p, uint64_t token) {
  struct co_slot *s = find(p, token);
  if (!s || (token & CANCEL_BIT) || s->state != CO_DONE)
    return 0;
#ifndef CO_MUTANT_PENDING_ACK
  if (s->cancel_pending)
    return 0;
#endif
  s->state = CO_FREE;
  return 1;
}
int co_quiescent(const struct co_pool *p) {
  if (!p)
    return 0;
  for (unsigned i = 0; i < 4; ++i)
    if (p->slot[i].state == CO_BUSY || p->slot[i].state == CO_DONE ||
        p->slot[i].cancel_pending)
      return 0;
  return 1;
}

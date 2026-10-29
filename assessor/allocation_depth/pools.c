#include "allocation.h"
void al_pool_init(struct al_pool *p) {
  algo_pool_init(&p->producer);
  for (unsigned i = 3; i < 16; ++i)
    p->producer.state[i] = 2;
}
int al_pool_take(struct al_pool *p, int32_t value, struct al_pool_handle *out) {
  if (!p || !out)
    return AL_BAD;
  uint64_t raw;
  int s = algo_pool_take(&p->producer, value, &raw);
  if (s == ALGO_FULL)
    return AL_FULL;
  if (s != ALGO_OK)
    return AL_BAD;
  *out = (struct al_pool_handle){p, raw};
  return AL_OK;
}
int al_pool_get(const struct al_pool *p, struct al_pool_handle h, int32_t *out) {
  if (!p || !out)
    return AL_BAD;
#if AL_MUTANT != 6
  if (h.owner != p)
    return AL_STALE;
#endif
  return algo_pool_get(&p->producer, h.raw, out) == ALGO_OK ? AL_OK : AL_STALE;
}
int al_pool_release(struct al_pool *p, struct al_pool_handle h) {
  if (!p)
    return AL_BAD;
  if (h.owner != p)
    return AL_STALE;
  unsigned i = (unsigned)h.raw;
  int32_t value;
  if (algo_pool_get(&p->producer, h.raw, &value) != ALGO_OK)
    return AL_STALE;
  unsigned retire = p->producer.generation[i] >= 3;
  int s = algo_pool_release(&p->producer, h.raw);
  if (retire)
    p->producer.state[i] = 2;
  return s == ALGO_OK ? AL_OK : AL_STALE;
}
int al_classes_init(struct al_classes *c) {
  if (!c)
    return AL_BAD;
  *c = (struct al_classes){0};
  return AL_OK;
}
int al_classes_take(struct al_classes *c, size_t n,
                    int (*init)(void *, unsigned char *, size_t),
                    al_cleanup_fn destroy, void *ctx, struct al_handle *out) {
  if (!c || !n || n > 32 || !out || !init)
    return AL_BAD;
  if (c->serial == UINT32_MAX)
    return AL_RETIRED;
  unsigned group = n <= 8 ? 0 : n <= 16 ? 1 : 2;
  for (unsigned i = 0; i < 4; ++i) {
    struct al_class_slot *s = &c->slots[group][i];
    if (!s->live) {
      unsigned char data[32] = {0};
      if (init(ctx, data, n))
        return AL_CLEANUP;
      for (unsigned j = 0; j < n; ++j)
        s->data[j] = data[j];
      s->id = ++c->serial;
      s->live = 1;
      s->requested = (unsigned)n;
      s->cleanup = destroy;
      s->context = ctx;
      c->retained[group] = 1;
      *out = (struct al_handle){c, s->id};
      return AL_OK;
    }
  }
  return AL_FULL;
}
int al_classes_release(struct al_classes *c, struct al_handle h) {
  if (!c)
    return AL_BAD;
  if (h.owner != c || !h.id)
    return AL_STALE;
  for (unsigned g = 0; g < 3; ++g)
    for (unsigned i = 0; i < 4; ++i) {
      struct al_class_slot *s = &c->slots[g][i];
      if (s->live && s->id == h.id) {
        s->live = 0;
        int status = s->cleanup ? s->cleanup(s->context) : 0;
        for (unsigned k = 0; k < 32; ++k)
          s->data[k] = 0;
        return status ? AL_CLEANUP : AL_OK;
      }
    }
  return AL_STALE;
}
int al_classes_stats(const struct al_classes *c, size_t *payload, size_t *charged,
                     size_t *retained) {
  if (!c || !payload || !charged || !retained)
    return AL_BAD;
  size_t p = 0, ch = 0, r = 0;
  for (unsigned g = 0; g < 3; ++g) {
    unsigned size = 8u << g;
    if (c->retained[g])
      r += 4 * size;
    for (unsigned i = 0; i < 4; ++i)
      if (c->slots[g][i].live) {
        p += c->slots[g][i].requested;
        ch += size;
      }
  }
  *payload = p;
  *charged = ch;
  *retained = r;
  return AL_OK;
}

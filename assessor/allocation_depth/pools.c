#include "allocation.h"
void al_pool_init(ALPool *p) {
  algo_pool_init(&p->producer);
  for (unsigned i = 3; i < 16; ++i)
    p->producer.state[i] = 2;
}
int al_pool_take(ALPool *p, int32_t value, ALPoolHandle *out) {
  if (!p || !out)
    return AL_BAD;
  uint64_t raw;
  int s = algo_pool_take(&p->producer, value, &raw);
  if (s == ALGO_FULL)
    return AL_FULL;
  if (s != ALGO_OK)
    return AL_BAD;
  *out = (ALPoolHandle){p, raw};
  return AL_OK;
}
int al_pool_get(const ALPool *p, ALPoolHandle h, int32_t *out) {
  if (!p || !out)
    return AL_BAD;
#if AL_MUTANT != 6
  if (h.owner != p)
    return AL_STALE;
#endif
  return algo_pool_get(&p->producer, h.raw, out) == ALGO_OK ? AL_OK : AL_STALE;
}
int al_pool_release(ALPool *p, ALPoolHandle h) {
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
int al_classes_init(ALClasses *c) {
  if (!c)
    return AL_BAD;
  *c = (ALClasses){0};
  return AL_OK;
}
int al_classes_take(ALClasses *c, size_t n,
                    int (*init)(void *, unsigned char *, size_t),
                    ALCleanup destroy, void *ctx, ALHandle *out) {
  if (!c || !n || n > 32 || !out || !init)
    return AL_BAD;
  if (c->serial == UINT32_MAX)
    return AL_RETIRED;
  unsigned group = n <= 8 ? 0 : n <= 16 ? 1 : 2;
  for (unsigned i = 0; i < 4; ++i) {
    ALClassSlot *s = &c->slots[group][i];
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
      *out = (ALHandle){c, s->id};
      return AL_OK;
    }
  }
  return AL_FULL;
}
int al_classes_release(ALClasses *c, ALHandle h) {
  if (!c)
    return AL_BAD;
  if (h.owner != c || !h.id)
    return AL_STALE;
  for (unsigned g = 0; g < 3; ++g)
    for (unsigned i = 0; i < 4; ++i) {
      ALClassSlot *s = &c->slots[g][i];
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
int al_classes_stats(const ALClasses *c, size_t *payload, size_t *charged,
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

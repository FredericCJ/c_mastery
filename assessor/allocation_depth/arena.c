#include "allocation.h"
int al_extent(size_t c, size_t n, size_t a, size_t cap, size_t *at,
              size_t *end) {
  if (!at || !end || !n || !a || (a & (a - 1)) || c > cap)
    return AL_BAD;
  size_t padding = (0 - c) & (a - 1);
#if AL_MUTANT == 1
  padding = 0;
#endif
  if (padding > cap - c || n > cap - c - padding)
    return AL_FULL;
  *at = c + padding;
  *end = *at + n;
  return AL_OK;
}
int al_arena_init(ALArena *a, unsigned char *p, size_t cap, size_t align) {
  if (!a || (!p && cap) || !align || (align & (align - 1)))
    return AL_BAD;
  *a = (ALArena){0};
  a->bytes = p;
  a->capacity = cap;
  a->alignment = align;
  return AL_OK;
}
int al_arena_take(ALArena *a, size_t n, size_t align, ALCleanup f, void *ctx,
                  ALHandle *out) {
  if (!a || !out || align > a->alignment)
    return AL_BAD;
  size_t at, end;
  int status = al_extent(a->used, n, align, a->capacity, &at, &end);
  if (status)
    return status;
  if (a->count == 16)
    return AL_FULL;
  if (a->serial == UINT32_MAX)
    return AL_RETIRED;
  ALItem item = {at, n, a->used, ++a->serial, f, ctx};
  a->items[a->count++] = item;
  a->used = end;
  a->payload += n;
  if (end > a->peak)
    a->peak = end;
  if (end > a->epoch_peak)
    a->epoch_peak = end;
  *out = (ALHandle){a, item.id};
  return AL_OK;
}
static int index_of(const ALArena *a, ALHandle h) {
  if (!a || h.owner != a || !h.id)
    return -1;
  for (unsigned i = 0; i < a->count; ++i)
    if (a->items[i].id == h.id)
      return (int)i;
  return -1;
}
int al_arena_view(ALArena *a, ALHandle h, unsigned char **out, size_t *size) {
  if (!out || !size)
    return AL_BAD;
  int i = index_of(a, h);
  if (i < 0)
    return AL_STALE;
  *out = a->bytes + a->items[i].offset;
  *size = a->items[i].size;
  return AL_OK;
}
int al_arena_mark(ALArena *a, ALMark *out) {
  if (!a || !out)
    return AL_BAD;
  if (a->mark_count == 8)
    return AL_FULL;
  if (a->serial == UINT32_MAX)
    return AL_RETIRED;
  unsigned i = a->mark_count++;
  a->marks[i].id = ++a->serial;
  a->marks[i].count = a->count;
  a->marks[i].used = a->used;
  *out = (ALMark){a, a->serial};
  return AL_OK;
}
static int mark_ok(ALArena *a, ALMark mark) {
  return a && a->mark_count && mark.owner == a &&
         mark.id == a->marks[a->mark_count - 1].id;
}
static int pop(ALArena *a, unsigned count) {
  int failed = 0;
  while (a->count > count) {
    ALItem x = a->items[--a->count];
    a->used = x.before;
    a->payload -= x.size;
    if (x.cleanup && x.cleanup(x.context))
      failed = 1;
  }
  return failed ? AL_CLEANUP : AL_OK;
}
int al_arena_rollback(ALArena *a, ALMark mark) {
  if (!mark_ok(a, mark))
    return AL_STALE;
  unsigned i = --a->mark_count;
  int status = pop(a, a->marks[i].count);
  a->used = a->marks[i].used;
  return status;
}
int al_arena_commit(ALArena *a, ALMark mark) {
  if (!mark_ok(a, mark))
    return AL_STALE;
  --a->mark_count;
  return AL_OK;
}
int al_arena_release(ALArena *a, ALHandle h) {
  int i = index_of(a, h);
  if (i < 0)
    return AL_STALE;
#if AL_MUTANT != 2
  if ((unsigned)i + 1 != a->count)
    return AL_BAD;
#endif
  if (a->mark_count && a->count <= a->marks[a->mark_count - 1].count)
    return AL_BAD;
  return pop(a, (unsigned)i);
}
int al_arena_reset(ALArena *a) {
  if (!a)
    return AL_BAD;
  int status = pop(a, 0);
  a->mark_count = 0;
  a->epoch_peak = 0;
  return status;
}

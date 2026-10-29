#include "allocation.h"
static unsigned get16(const unsigned char *p) {
  return p[0] | ((unsigned)p[1] << 8);
}
static void put16(unsigned char *p, unsigned n) {
  p[0] = (unsigned char)n;
  p[1] = (unsigned char)(n >> 8);
}
static void tag(struct al_heap *h, unsigned at, unsigned total, unsigned n) {
  unsigned char *p = h->bytes + at;
  put16(p, total);
  put16(p + 2, n);
  p[4] = (unsigned char)(n != 0);
  p[5] = 0;
  p[6] = 0xa5;
  p[7] = 0x5a;
  p += total - 4;
  put16(p, total);
  p[2] = (unsigned char)(n != 0);
  p[3] = 0xc3;
}
int al_heap_check(const struct al_heap *h) {
  if (!h || h->capacity < 16 || h->capacity > 256 || h->capacity % 8 ||
      h->free_count > 16 || h->policy > 1 || h->deferred > 1)
    return AL_CORRUPT;
  unsigned at = 0, f = 0, allocated = 0, previous_free = 0;
  size_t payload = 0, charged = 0;
  while (at < h->capacity) {
    if (h->capacity - at < 16)
      return AL_CORRUPT;
    const unsigned char *p = h->bytes + at;
    unsigned total = get16(p), n = get16(p + 2), live = p[4];
    if (total < 16 || total % 8 || total > h->capacity - at || live > 1 ||
        p[5] || p[6] != 0xa5 || p[7] != 0x5a)
      return AL_CORRUPT;
    if ((!live && n) || (live && (!n || n > total - 12)))
      return AL_CORRUPT;
#if AL_MUTANT != 3
    if (get16(p + total - 4) != total || p[total - 2] != live ||
        p[total - 1] != 0xc3)
      return AL_CORRUPT;
#endif
    unsigned matches = 0;
    for (unsigned j = 0; j < 16; ++j)
      if (h->live[j].id && h->live[j].offset == at)
        ++matches;
    if (live) {
      if (matches != 1)
        return AL_CORRUPT;
      ++allocated;
      payload += n;
      charged += total;
    } else {
      if (matches || f >= h->free_count || h->free_offsets[f++] != at ||
          (!h->deferred && previous_free))
        return AL_CORRUPT;
    }
    previous_free = !live;
    at += total;
  }
  unsigned entries = 0;
  for (unsigned i = 0; i < 16; ++i)
    if (h->live[i].id) {
      ++entries;
      if (h->live[i].id > h->serial)
        return AL_CORRUPT;
      for (unsigned j = 0; j < i; ++j)
        if (h->live[j].id == h->live[i].id)
          return AL_CORRUPT;
    }
  return f == h->free_count && allocated == entries && payload == h->payload &&
                 charged == h->charged
             ? AL_OK
             : AL_CORRUPT;
}
static void rebuild(struct al_heap *h, int merge) {
  unsigned at = 0;
  h->free_count = 0;
  h->payload = 0;
  h->charged = 0;
  while (at < h->capacity) {
    unsigned total = get16(h->bytes + at), n = get16(h->bytes + at + 2);
    if (!n) {
      if (merge)
        while (at + total < h->capacity && !h->bytes[at + total + 4]) {
          total += get16(h->bytes + at + total);
          tag(h, at, total, 0);
        }
      h->free_offsets[h->free_count++] = at;
    } else {
      h->payload += n;
      h->charged += total;
    }
    at += total;
  }
  if (h->payload > h->peak_payload)
    h->peak_payload = h->payload;
  if (h->charged > h->peak_charged)
    h->peak_charged = h->charged;
}
int al_heap_init(struct al_heap *h, unsigned n, unsigned best, unsigned deferred,
                 unsigned instrument) {
  if (!h || n < 16 || n > 256 || n % 8 || best > 1 || deferred > 1 ||
      instrument > 1)
    return AL_BAD;
  *h = (struct al_heap){0};
  h->capacity = n;
  h->policy = best;
  h->deferred = deferred;
  h->instrument = instrument;
  tag(h, 0, n, 0);
  rebuild(h, 0);
  return AL_OK;
}
static int slot(const struct al_heap *h, struct al_handle handle) {
  if (!h || handle.owner != h || !handle.id)
    return -1;
  for (unsigned i = 0; i < 16; ++i)
    if (h->live[i].id == handle.id)
      return (int)i;
  return -1;
}
static int reserve(struct al_heap *h, unsigned n, unsigned *idx) {
  unsigned need = (n + 12 + 7) & ~7u, choice = 256, best = 257, steps = 0,
           empty = 16;
  for (unsigned i = 0; i < 16; ++i)
    if (!h->live[i].id) {
      empty = i;
      break;
    }
  if (empty == 16)
    return AL_FULL;
  for (unsigned i = 0; i < h->free_count; ++i) {
    unsigned at = h->free_offsets[i], total = get16(h->bytes + at);
    ++steps;
    if (total >= need && total < best) {
      choice = at;
      best = total;
      if (!h->policy)
        break;
    }
  }
  if (choice == 256)
    return AL_FULL;
  unsigned total = need;
  if (best - need < 16)
    total = best;
#if AL_MUTANT == 4
  total = need;
#endif
  tag(h, choice, total, n);
  if (best > total)
    tag(h, choice + total, best - total, 0);
  h->live[empty].offset = choice;
  h->live[empty].id = ++h->serial;
  if (h->instrument)
    h->search_steps =
        steps > SIZE_MAX - h->search_steps ? SIZE_MAX : h->search_steps + steps;
  rebuild(h, 0);
  *idx = empty;
  return AL_OK;
}
int al_heap_take(struct al_heap *h, size_t n, size_t alignment, struct al_handle *out) {
  if (!h || !out || !n || n > 244 || !alignment || alignment > 8 ||
      (alignment & (alignment - 1)))
    return AL_BAD;
  if (al_heap_check(h))
    return AL_CORRUPT;
  if (h->serial == UINT32_MAX)
    return AL_RETIRED;
  struct al_heap t = *h;
  unsigned i;
  int status = reserve(&t, (unsigned)n, &i);
  if (status)
    return status;
  *h = t;
  *out = (struct al_handle){h, h->live[i].id};
  return AL_OK;
}
int al_heap_view(struct al_heap *h, struct al_handle handle, unsigned char **out, size_t *n) {
  if (!h || !out || !n)
    return AL_BAD;
  if (al_heap_check(h))
    return AL_CORRUPT;
  int i = slot(h, handle);
  if (i < 0)
    return AL_STALE;
  unsigned at = h->live[i].offset;
  *out = h->bytes + at + 8;
  *n = get16(h->bytes + at + 2);
  return AL_OK;
}
int al_heap_release(struct al_heap *h, struct al_handle handle) {
  if (!h)
    return AL_BAD;
  if (al_heap_check(h))
    return AL_CORRUPT;
  int i = slot(h, handle);
  if (i < 0)
    return AL_STALE;
  unsigned at = h->live[i].offset;
  tag(h, at, get16(h->bytes + at), 0);
  h->live[i].id = 0;
  rebuild(h, !h->deferred);
  return AL_OK;
}
int al_heap_collect(struct al_heap *h) {
  if (al_heap_check(h))
    return AL_CORRUPT;
  rebuild(h, 1);
  return AL_OK;
}
int al_heap_stats(const struct al_heap *h, size_t *total, size_t *largest) {
  if (!total || !largest)
    return AL_BAD;
  if (al_heap_check(h))
    return AL_CORRUPT;
  size_t sum = 0, big = 0;
  for (unsigned i = 0; i < h->free_count; ++i) {
    unsigned n = get16(h->bytes + h->free_offsets[i]);
    sum += n;
    if (n > big)
      big = n;
  }
  *total = sum;
  *largest = big;
  return AL_OK;
}
int al_heap_resize_copy(struct al_heap *h, struct al_handle handle, size_t n, int fail_acquire,
                        al_copy_fn copier, void *context, struct al_handle *out) {
  if (!h || !out || !n || n > 244 || !copier)
    return AL_BAD;
  if (al_heap_check(h))
    return AL_CORRUPT;
  int i = slot(h, handle);
  if (i < 0)
    return AL_STALE;
  if (h->serial == UINT32_MAX)
    return AL_RETIRED;
  struct al_heap t = *h;
  unsigned at = t.live[i].offset, old = get16(t.bytes + at + 2),
           size = get16(t.bytes + at), need = ((unsigned)n + 19) & ~7u,
           available = size;
  if (need > size && at + size < t.capacity && !t.bytes[at + size + 4])
    available += get16(t.bytes + at + size);
  if (need <= available) {
    unsigned total = available - need < 16 ? available : need;
    tag(&t, at, total, (unsigned)n);
    if (available > total)
      tag(&t, at + total, available - total, 0);
    t.live[i].id = ++t.serial;
    rebuild(&t, !t.deferred);
  } else {
    if (fail_acquire)
      return AL_FULL;
    unsigned j;
    int status = reserve(&t, (unsigned)n, &j);
    if (status)
      return status;
    unsigned copy = old < n ? old : (unsigned)n;
    if (copier(context, t.bytes + t.live[j].offset + 8, t.bytes + at + 8,
               copy)) {
#if AL_MUTANT == 5
      *h = t;
#endif
      return AL_CLEANUP;
    }
    tag(&t, at, size, 0);
    t.live[i].id = 0;
    rebuild(&t, !t.deferred);
    i = (int)j;
  }
  *h = t;
  *out = (struct al_handle){h, h->live[i].id};
  return AL_OK;
}
static int byte_copy(void *context, unsigned char *dest,
                     const unsigned char *src, size_t n) {
  size_t limit = *(size_t *)context;
  for (size_t i = 0; i < n; ++i) {
    if (i == limit)
      return 1;
    dest[i] = src[i];
  }
  return 0;
}
int al_heap_resize(struct al_heap *h, struct al_handle handle, size_t n, int fail_acquire,
                   size_t fail_after, struct al_handle *out) {
  return al_heap_resize_copy(h, handle, n, fail_acquire, byte_copy, &fail_after,
                             out);
}

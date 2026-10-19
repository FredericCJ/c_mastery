#include "allocation.h"
int al_extent_choose(ALExtent *a, size_t *count, size_t bytes, size_t minimum,
                     unsigned best, size_t *offset, size_t *charged) {
  if (!a || !count || !bytes || !minimum || best > 1 || !offset || !charged ||
      *count > 16)
    return AL_BAD;
  size_t chosen = 16;
  for (size_t i = 0; i < *count; ++i) {
    if (!a[i].size || a[i].offset > SIZE_MAX - a[i].size ||
        (i && a[i - 1].offset + a[i - 1].size > a[i].offset))
      return AL_BAD;
    if (a[i].size >= bytes &&
        (chosen == 16 || (best && a[i].size < a[chosen].size)))
      chosen = i;
  }
  if (chosen == 16)
    return AL_FULL;
  size_t at = a[chosen].offset,
         n = a[chosen].size - bytes < minimum ? a[chosen].size : bytes;
  if (n == a[chosen].size) {
    for (size_t i = chosen + 1; i < *count; ++i)
      a[i - 1] = a[i];
    --*count;
  } else {
    a[chosen].offset += n;
    a[chosen].size -= n;
  }
  *offset = at;
  *charged = n;
  return AL_OK;
}
int al_extent_coalesce(ALExtent *a, size_t *count, size_t capacity) {
  if (!a || !count || *count > 16)
    return AL_BAD;
  ALExtent sorted[16];
  for (size_t i = 0; i < *count; ++i) {
    if (!a[i].size || a[i].offset > capacity ||
        a[i].size > capacity - a[i].offset)
      return AL_BAD;
    sorted[i] = a[i];
  }
  for (size_t i = 1; i < *count; ++i) {
    ALExtent x = sorted[i];
    size_t j = i;
    while (j && sorted[j - 1].offset > x.offset) {
      sorted[j] = sorted[j - 1];
      --j;
    }
    sorted[j] = x;
  }
  size_t used = 0;
  for (size_t i = 0; i < *count; ++i) {
    if (used) {
      ALExtent *p = &sorted[used - 1];
      size_t end = p->offset + p->size;
      if (end > sorted[i].offset)
        return AL_BAD;
      if (end == sorted[i].offset) {
        p->size += sorted[i].size;
        continue;
      }
    }
    sorted[used++] = sorted[i];
  }
  for (size_t i = 0; i < used; ++i)
    a[i] = sorted[i];
  *count = used;
  return AL_OK;
}
static int allocate(void *p, size_t n, size_t align, ALHandle *out) {
  return al_heap_take(p, n, align, out);
}
static int release(void *p, ALHandle h) { return al_heap_release(p, h); }
static int resize(void *p, ALHandle h, size_t n, ALHandle *out) {
  return al_heap_resize(p, h, n, 0, SIZE_MAX, out);
}
int al_heap_api(ALHeap *h, ALAllocator *out) {
  if (!h || !out)
    return AL_BAD;
  *out = (ALAllocator){h, 8, allocate, release, resize};
  return AL_OK;
}
int al_api_allocate(const ALAllocator *a, size_t n, size_t align,
                    ALHandle *out) {
  if (!a || !a->allocate || !a->owner || !out || !n || !align ||
      (align & (align - 1)) || align > a->max_alignment)
    return AL_BAD;
  return a->allocate(a->owner, n, align, out);
}

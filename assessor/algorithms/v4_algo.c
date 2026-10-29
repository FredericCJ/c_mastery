#include "v4_algo.h"
#include <limits.h>
int v4_reserve(struct v4_array *a, size_t cap) {
  if (!a || !a->allocator.take || !a->allocator.release ||
      a->length > a->capacity || (a->capacity && !a->data))
    return -1;
  if (cap <= a->capacity)
    return 0;
  if (cap > SIZE_MAX / sizeof(int32_t))
    return -1;
  int32_t *p = a->allocator.take(a->allocator.context, cap);
  if (!p)
    return -1;
  for (size_t i = 0; i < a->length; i++)
    p[i] = a->data[i];
  if (a->data)
    a->allocator.release(a->allocator.context, a->data);
  a->data = p;
  a->capacity = cap;
  return 0;
}
int v4_list_push(struct v4_list *q, int32_t x) {
  if (!q || q->used == 255)
    return -1;
  unsigned n = 0;
  while (q->used & (1u << n))
    n++;
  q->value[n] = x;
  q->next[n] = -1;
  if (q->used)
    q->next[q->tail] = (int)n;
  else
    q->head = (int)n;
  q->tail = (int)n;
  q->used |= 1u << n;
  return 0;
}
int v4_list_pop(struct v4_list *q, int32_t *out) {
  if (!q || !out || !q->used)
    return -1;
  unsigned n = (unsigned)q->head;
  *out = q->value[n];
  q->head = q->next[n];
  q->used &= ~(1u << n);
  if (!q->used)
    q->tail = -1;
  return 0;
}
int v4_free_take(struct v4_free *f, unsigned n, unsigned *out) {
  if (!f || !out || !n)
    return -1;
  for (unsigned i = 0; i < f->count; i++)
    if (f->length[i] >= n) {
      *out = f->start[i];
      f->start[i] += n;
      f->length[i] -= n;
      if (!f->length[i]) {
        for (unsigned j = i + 1; j < f->count; j++) {
          f->start[j - 1] = f->start[j];
          f->length[j - 1] = f->length[j];
        }
        f->count--;
      }
      return 0;
    }
  return -1;
}
int v4_free_return(struct v4_free *f, unsigned start, unsigned n) {
  if (!f || !n || n > UINT_MAX - start || f->count > 8)
    return -1;
  unsigned i = 0;
  while (i < f->count && f->start[i] < start)
    i++;
  if ((i && f->start[i - 1] + f->length[i - 1] > start) ||
      (i < f->count && start + n > f->start[i]))
    return -1;
  unsigned left = i && f->start[i - 1] + f->length[i - 1] == start;
  unsigned right = i < f->count && start + n == f->start[i];
  if (left) {
    f->length[i - 1] += n;
    if (right) {
      f->length[i - 1] += f->length[i];
      for (unsigned j = i + 1; j < f->count; j++) {
        f->start[j - 1] = f->start[j];
        f->length[j - 1] = f->length[j];
      }
      f->count--;
    }
    return 0;
  }
  if (right) {
    f->start[i] = start;
    f->length[i] += n;
    return 0;
  }
  if (f->count == 8)
    return -1;
  for (unsigned j = f->count; j > i; j--) {
    f->start[j] = f->start[j - 1];
    f->length[j] = f->length[j - 1];
  }
  f->start[i] = start;
  f->length[i] = n;
  f->count++;
  return 0;
}
int v4_radix(uint32_t *a, size_t n, uint32_t *tmp, size_t cap) {
  if (n > cap || (!a && n) || (!tmp && n) || a == tmp)
    return -1;
  for (unsigned shift = 0; shift < 32; shift += 8) {
    size_t count[256] = {0}, pos[256];
    for (size_t i = 0; i < n; i++)
      count[(a[i] >> shift) & 255]++;
    pos[0] = 0;
    for (unsigned j = 1; j < 256; j++)
      pos[j] = pos[j - 1] + count[j - 1];
    for (size_t i = 0; i < n; i++)
      tmp[pos[(a[i] >> shift) & 255]++] = a[i];
    for (size_t i = 0; i < n; i++)
      a[i] = tmp[i];
  }
  return 0;
}
int v4_knapsack(const unsigned *w, const unsigned *v, size_t n, unsigned cap,
                unsigned *out) {
  if (!out || cap > 32 || n > 16 || (!w && n) || (!v && n))
    return -1;
  unsigned dp[33] = {0};
  for (size_t i = 0; i < n; i++) {
    if (!w[i])
      return -1;
    for (unsigned c = cap; c >= w[i] && c > 0; c--) {
      if (v[i] > UINT_MAX - dp[c - w[i]])
        return -1;
      unsigned x = dp[c - w[i]] + v[i];
      if (x > dp[c])
        dp[c] = x;
    }
  }
  *out = dp[cap];
  return 0;
}
int v4_filter(struct v4_filter *f, int32_t x, int32_t *fir, int32_t *iir) {
  if (!f || !fir || !iir || f->position >= 3)
    return -1;
  unsigned p = f->position;
  int64_t acc = (int64_t)x + 2 * (int64_t)f->history[(p + 2) % 3] +
                f->history[(p + 1) % 3];
  int32_t y = (int32_t)(acc / 4);
  int32_t z = (int32_t)(((int64_t)f->previous + y) / 2);
  f->history[p] = x;
  f->position = (p + 1) % 3;
  f->previous = z;
  *fir = y;
  *iir = z;
  return 0;
}

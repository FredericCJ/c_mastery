#include "numeric.h"
#include <limits.h>
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
int nd_insert(struct nd_store *s, double x, double y, const char label[24],
              unsigned *out) {
  if (!s || !label || !out || s->count >= ND_ITEMS || s->serial == UINT_MAX ||
      !isfinite(x) || !isfinite(y))
    return ND_BAD;
  size_t n = s->count;
  s->x[n] = x;
  s->y[n] = y;
  s->id[n] = ++s->serial;
  for (size_t i = 0; i < 24; i++)
    s->label[n][i] = label[i];
  s->count++;
  *out = s->serial;
  return ND_OK;
}
int nd_remove(struct nd_store *s, unsigned id) {
  if (!s || s->count > ND_ITEMS)
    return ND_BAD;
  size_t i = 0;
  while (i < s->count && s->id[i] != id)
    i++;
  if (i == s->count)
    return ND_BAD;
  for (size_t j = i + 1; j < s->count; j++) {
    s->x[j - 1] = s->x[j];
    s->y[j - 1] = s->y[j];
    s->id[j - 1] = s->id[j];
    if (ND_MUTANT != 7)
      for (size_t k = 0; k < 24; k++)
        s->label[j - 1][k] = s->label[j][k];
  }
  s->count--;
  return ND_OK;
}
int nd_reorder(struct nd_store *s, const unsigned *order, size_t n) {
  if (!s || n != s->count || n > ND_ITEMS || (!order && n))
    return ND_BAD;
  struct nd_store t = *s;
  unsigned seen = 0;
  for (size_t i = 0; i < n; i++) {
    if (order[i] >= n || (seen & (1u << order[i])))
      return ND_BAD;
    seen |= 1u << order[i];
    size_t j = order[i];
    t.x[i] = s->x[j];
    t.y[i] = s->y[j];
    t.id[i] = s->id[j];
    for (size_t k = 0; k < 24; k++)
      t.label[i][k] = s->label[j][k];
  }
  *s = t;
  return ND_OK;
}
int nd_record(const struct nd_store *s, unsigned id, struct nd_record *out) {
  if (!s || !out || s->count > ND_ITEMS)
    return ND_BAD;
  for (size_t i = 0; i < s->count; i++)
    if (s->id[i] == id) {
      out->x = s->x[i];
      out->y = s->y[i];
      out->id = id;
      for (size_t j = 0; j < 24; j++)
        out->label[j] = s->label[i][j];
      return ND_OK;
    }
  return ND_BAD;
}

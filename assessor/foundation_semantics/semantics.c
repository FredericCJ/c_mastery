#include "semantics.h"
#include <limits.h>
#ifndef FS_MUTANT
#define FS_MUTANT 0
#endif
enum fs_index fs_index(intmax_t i, size_t n) {
  if (i < 0 || (uintmax_t)i > (uintmax_t)n)
    return FS_OUTSIDE;
  if ((uintmax_t)i == (uintmax_t)n)
    return FS_MUTANT == 1 ? FS_ELEMENT : FS_ONE_PAST;
  return FS_ELEMENT;
}
static int add(int a, int b, int *out) {
  if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
    return 0;
  *out = a + b;
  return 1;
}
enum fs_status fs_sum(const int *p, size_t n, int *out) {
  if (!out || (n && !p))
    return FS_INVALID;
  int total = 0;
  for (size_t i = 0; i < n; i++)
    if (!add(total, p[i], &total))
      return FS_RANGE;
  *out = total;
  return FS_OK;
}
enum fs_status fs_row(const int (*row)[3], int *out) {
  if (!row)
    return FS_INVALID;
  return fs_sum(*row, 3, out);
}
enum fs_status fs_matrix(const int (*rows)[3], size_t count, int *out) {
  if (!out || (count && !rows))
    return FS_INVALID;
  int total = 0;
  for (size_t r = 0; r < count; r++)
    for (size_t c = 0; c < 3; c++)
      if (!add(total, rows[FS_MUTANT == 2 ? 0 : r][c], &total))
        return FS_RANGE;
  *out = total;
  return FS_OK;
}
enum fs_status fs_branch(int branch, int *out) {
  if (!out || (branch != 0 && branch != 1))
    return FS_INVALID;
  int value;
  if (branch)
    value = 7;
  else
    value = FS_MUTANT == 3 ? 0 : 11; /* Defined wrong repair, never an executed
                                        uninitialized read. */
  *out = value;
  return FS_OK;
}
static int first(struct fs_calls *s) {
  s->order = s->order * 10 + 1;
  return ++s->x;
}
static int second(struct fs_calls *s) {
  s->order = s->order * 10 + 2;
  s->y *= 2;
  return s->y;
}
enum fs_status fs_calls(struct fs_calls *out) {
  if (!out)
    return FS_INVALID;
  struct fs_calls stage = {0, 1, 2, 0};
  stage.sum = first(&stage) + second(&stage);
  if (FS_MUTANT == 5)
    stage.sum--;
  *out = stage;
  return FS_OK;
}
enum fs_status fs_comma(int *value, int *final) {
  if (!value || !final || value == final)
    return FS_INVALID;
  int i = 1;
  int result = (i++, i++);
  *value = FS_MUTANT == 4 ? result - 1 : result;
  *final = i;
  return FS_OK;
}

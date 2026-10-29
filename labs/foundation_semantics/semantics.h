#ifndef FS_SEMANTICS_H
#define FS_SEMANTICS_H
#include <stddef.h>
#include <stdint.h>
enum fs_status {
  FS_OK,
  FS_INVALID,
  FS_RANGE,
  FS_UNINITIALIZED,
  FS_NOT_IMPLEMENTED
};
enum fs_index { FS_OUTSIDE, FS_ELEMENT, FS_ONE_PAST };
struct fs_calls {
  int sum, x, y;
  unsigned order;
};
enum fs_index fs_index(intmax_t index, size_t length);
enum fs_status fs_sum(const int *input, size_t length, int *out);
enum fs_status fs_row(const int (*row)[3], int *out);
enum fs_status fs_matrix(const int (*rows)[3], size_t count, int *out);
/* branch must be0/1. Both branches initialize the result (7 or11).
 * output publication is deferred; invalid branch/null output rejects. */
enum fs_status fs_branch(int branch, int *out);
enum fs_status fs_calls(struct fs_calls *out);
enum fs_status fs_comma(int *value, int *final);
#endif

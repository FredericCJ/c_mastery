#ifndef FS_SEMANTICS_H
#define FS_SEMANTICS_H
#include <stddef.h>
#include <stdint.h>
typedef enum {
  FS_OK,
  FS_INVALID,
  FS_RANGE,
  FS_UNINITIALIZED,
  FS_NOT_IMPLEMENTED
} FsStatus;
typedef enum { FS_OUTSIDE, FS_ELEMENT, FS_ONE_PAST } FsIndex;
typedef struct {
  int sum, x, y;
  unsigned order;
} FsCalls;
FsIndex fs_index(intmax_t index, size_t length);
FsStatus fs_sum(const int *input, size_t length, int *out);
FsStatus fs_row(const int (*row)[3], int *out);
FsStatus fs_matrix(const int (*rows)[3], size_t count, int *out);
/* branch must be0/1. Both branches initialize the result (7 or11).
 * output publication is deferred; invalid branch/null output rejects. */
FsStatus fs_branch(int branch, int *out);
FsStatus fs_calls(FsCalls *out);
FsStatus fs_comma(int *value, int *final);
#endif

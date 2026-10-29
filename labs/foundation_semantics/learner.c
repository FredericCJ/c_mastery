#include "semantics.h"
enum fs_index fs_index(intmax_t i, size_t n) {
  (void)i;
  (void)n;
  return FS_OUTSIDE;
}
enum fs_status fs_sum(const int *p, size_t n, int *o) {
  (void)p;
  (void)n;
  (void)o;
  return FS_NOT_IMPLEMENTED;
}
enum fs_status fs_row(const int (*p)[3], int *o) {
  (void)p;
  (void)o;
  return FS_NOT_IMPLEMENTED;
}
enum fs_status fs_matrix(const int (*p)[3], size_t n, int *o) {
  (void)p;
  (void)n;
  (void)o;
  return FS_NOT_IMPLEMENTED;
}
enum fs_status fs_branch(int b, int *o) {
  (void)b;
  (void)o;
  return FS_NOT_IMPLEMENTED;
}
enum fs_status fs_calls(struct fs_calls *o) {
  (void)o;
  return FS_NOT_IMPLEMENTED;
}
enum fs_status fs_comma(int *v, int *f) {
  (void)v;
  (void)f;
  return FS_NOT_IMPLEMENTED;
}

#ifndef TF_FORMAT_H
#define TF_FORMAT_H
#include <stddef.h>
enum tf_status { TF_OK, TF_FULL, TF_INVALID, TF_NOMEM, TF_FORMAT, TF_SIZE, TF_UNFINISHED };
struct tf_allocator {
  void *context;
  char *(*acquire)(void *, size_t);
  void (*release)(void *, char *);
};
/* Each callback has snprintf's scalar sizing/truncation return contract. */
struct tf_scalars {
  void *context;
  int (*integer)(void *, char *, size_t, int);
  int (*real)(void *, char *, size_t, double);
};
/* Explicit fields avoid an uncheckable active-union-member claim. */
struct tf_arg { int tag, integer; double real; };
enum tf_status tf_format(char *destination, size_t capacity, size_t *required,
                   struct tf_allocator allocator, struct tf_scalars scalar, unsigned count, ...);
enum tf_status tf_array(char *destination, size_t capacity, size_t *required,
                  struct tf_allocator allocator, struct tf_scalars scalar, const struct tf_arg *args,
                  size_t available, unsigned count);
int tf_default_integer(void *, char *, size_t, int);
int tf_default_real(void *, char *, size_t, double);
#endif

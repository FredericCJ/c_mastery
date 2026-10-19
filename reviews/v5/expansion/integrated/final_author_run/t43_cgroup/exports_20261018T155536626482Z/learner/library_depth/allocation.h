#ifndef LIBRARY_ALLOCATION_H
#define LIBRARY_ALLOCATION_H
#include <stddef.h>
enum la_status {
  LA_OK,
  LA_ARGUMENT,
  LA_RANGE,
  LA_ALIGNMENT,
  LA_NOMEM,
  LA_COPY_FAILED,
  LA_NOT_IMPLEMENTED
};
struct la_owner {
  void *data;
  size_t length, capacity, alignment;
};
struct la_ops {
  void *context;
  int (*supports)(void *, size_t);
  void *(*allocate)(void *, size_t, size_t);
  /* Optional nonzero realloc-like operation; preserves fundamental alignment.
   */
  void *(*resize)(void *, void *, size_t);
  void (*release)(void *, void *);
  /* Optional fallible copy. Failure may modify only the new destination. */
  int (*copy)(void *, void *, const void *, size_t);
  size_t fundamental_alignment;
};
enum la_status la_create(struct la_ops, struct la_owner *, size_t, size_t,
                         size_t);
enum la_status la_resize_native(struct la_ops, struct la_owner *, size_t,
                                size_t);
enum la_status la_resize_copy(struct la_ops, struct la_owner *, size_t, size_t);
enum la_status la_release(struct la_ops, struct la_owner *);
#endif

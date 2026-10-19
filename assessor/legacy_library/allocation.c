#include "allocation.h"
#include "foundation.h"
static int ops_valid(struct la_ops a) {
  return a.supports && a.allocate && a.release && a.fundamental_alignment;
}
static int live(const struct la_owner *o) {
  return o && o->data && o->length && o->capacity >= o->length && o->alignment;
}
static enum la_status sizes(struct la_ops a, size_t n, size_t w,
                            size_t alignment, size_t *length,
                            size_t *capacity) {
  if (!n || !w)
    return LA_ARGUMENT;
#ifdef LA_MUTANT_OVERFLOW
  *length = n * w;
#else
  if (f_checked_size(n, w, 0, length) != F_OK)
    return LA_RANGE;
#endif
  if (!alignment || (alignment & (alignment - 1)) ||
      !a.supports(a.context, alignment))
    return LA_ALIGNMENT;
  size_t remainder = *length % alignment,
         pad = remainder ? alignment - remainder : 0;
  if (f_checked_size(1, *length, pad, capacity) != F_OK)
    return LA_RANGE;
  return LA_OK;
}
enum la_status la_create(struct la_ops a, struct la_owner *o, size_t n,
                         size_t w, size_t alignment) {
  if (!ops_valid(a) || !o || o->data || o->length || o->capacity ||
      o->alignment)
    return LA_ARGUMENT;
  size_t length, capacity;
  enum la_status status = sizes(a, n, w, alignment, &length, &capacity);
  if (status != LA_OK)
    return status;
  void *p = a.allocate(a.context, capacity, alignment);
  if (!p)
    return LA_NOMEM;
  *o = (struct la_owner){p, length, capacity, alignment};
  return LA_OK;
}
enum la_status la_release(struct la_ops a, struct la_owner *o) {
  if (!ops_valid(a) || !o)
    return LA_ARGUMENT;
  if (!o->data) {
    if (o->length || o->capacity || o->alignment)
      return LA_ARGUMENT;
    return LA_OK;
  }
  if (!live(o))
    return LA_ARGUMENT;
  a.release(a.context, o->data);
  *o = (struct la_owner){0};
  return LA_OK;
}
enum la_status la_resize_native(struct la_ops a, struct la_owner *o, size_t n,
                                size_t w) {
  if (!ops_valid(a) || !a.resize || !live(o))
    return LA_ARGUMENT;
  if (o->alignment > a.fundamental_alignment)
    return LA_ALIGNMENT;
  size_t length, capacity;
  enum la_status status = sizes(a, n, w, o->alignment, &length, &capacity);
  if (status != LA_OK)
    return status;
  void *p = a.resize(a.context, o->data, capacity);
#ifdef LA_MUTANT_OWNER
  o->data = p;
#endif
  if (!p)
    return LA_NOMEM;
  *o = (struct la_owner){p, length, capacity, o->alignment};
  return LA_OK;
}
enum la_status la_resize_copy(struct la_ops a, struct la_owner *o, size_t n,
                              size_t w) {
  if (!ops_valid(a) || !a.copy || !live(o))
    return LA_ARGUMENT;
  size_t length, capacity;
  enum la_status status = sizes(a, n, w, o->alignment, &length, &capacity);
  if (status != LA_OK)
    return status;
  void *p = a.allocate(a.context, capacity, o->alignment);
  if (!p)
    return LA_NOMEM;
  size_t count = length < o->length ? length : o->length;
  if (a.copy(a.context, p, o->data, count)) {
#ifndef LA_MUTANT_ROLLBACK
    a.release(a.context, p);
#endif
    return LA_COPY_FAILED;
  }
  a.release(a.context, o->data);
  *o = (struct la_owner){p, length, capacity, o->alignment};
  return LA_OK;
}

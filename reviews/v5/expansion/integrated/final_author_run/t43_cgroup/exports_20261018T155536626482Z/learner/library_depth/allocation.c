#include "allocation.h"
enum la_status la_create(struct la_ops a, struct la_owner *o, size_t n,
                         size_t w, size_t l) {
  (void)a;
  (void)o;
  (void)n;
  (void)w;
  (void)l;
  return LA_NOT_IMPLEMENTED;
}
enum la_status la_resize_native(struct la_ops a, struct la_owner *o, size_t n,
                                size_t w) {
  (void)a;
  (void)o;
  (void)n;
  (void)w;
  return LA_NOT_IMPLEMENTED;
}
enum la_status la_resize_copy(struct la_ops a, struct la_owner *o, size_t n,
                              size_t w) {
  (void)a;
  (void)o;
  (void)n;
  (void)w;
  return LA_NOT_IMPLEMENTED;
}
enum la_status la_release(struct la_ops a, struct la_owner *o) {
  (void)a;
  (void)o;
  return LA_NOT_IMPLEMENTED;
}

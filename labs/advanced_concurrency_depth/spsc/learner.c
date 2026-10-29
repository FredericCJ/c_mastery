#include "spsc.h"
enum cmx_result as_init(struct as_queue *q, unsigned n) {
  (void)q;
  (void)n;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_push(struct as_queue *q, struct as_value v, as_hook_fn h, void *c) {
  (void)q;
  (void)v;
  (void)h;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_pop(struct as_queue *q, struct as_value *v, as_hook_fn h, void *c) {
  (void)q;
  (void)v;
  (void)h;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_circular_init(struct as_circular *q, unsigned n) {
  (void)q;
  (void)n;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_circular_push(struct as_circular *q, struct as_value v) {
  (void)q;
  (void)v;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_circular_pop(struct as_circular *q, struct as_value *v) {
  (void)q;
  (void)v;
  return CMX_UNIMPLEMENTED;
}

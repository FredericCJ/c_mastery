#include "spsc.h"
enum cmx_result as_init(ASQueue *q, unsigned n) {
  (void)q;
  (void)n;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_push(ASQueue *q, ASValue v, ASHook h, void *c) {
  (void)q;
  (void)v;
  (void)h;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_pop(ASQueue *q, ASValue *v, ASHook h, void *c) {
  (void)q;
  (void)v;
  (void)h;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_circular_init(ASCircular *q, unsigned n) {
  (void)q;
  (void)n;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_circular_push(ASCircular *q, ASValue v) {
  (void)q;
  (void)v;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result as_circular_pop(ASCircular *q, ASValue *v) {
  (void)q;
  (void)v;
  return CMX_UNIMPLEMENTED;
}

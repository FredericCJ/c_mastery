#include "queue.h"
#include <errno.h>
int tp_queue_init(TPEndQueue *q) {
  (void)q;
  return EINVAL;
}
int tp_queue_push(TPEndQueue *q, int n) {
  (void)q;
  (void)n;
  return EINVAL;
}
int tp_queue_pop(TPEndQueue *q, int *p) {
  (void)q;
  (void)p;
  return EINVAL;
}
int tp_queue_abort(TPEndQueue *q) {
  (void)q;
  return EINVAL;
}
void tp_queue_destroy(TPEndQueue *q) { (void)q; }

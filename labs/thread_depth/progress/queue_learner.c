#include "queue.h"
#include <errno.h>
int tp_queue_init(struct tp_end_queue *q) {
  (void)q;
  return EINVAL;
}
int tp_queue_push(struct tp_end_queue *q, int n) {
  (void)q;
  (void)n;
  return EINVAL;
}
int tp_queue_pop(struct tp_end_queue *q, int *p) {
  (void)q;
  (void)p;
  return EINVAL;
}
int tp_queue_abort(struct tp_end_queue *q) {
  (void)q;
  return EINVAL;
}
void tp_queue_destroy(struct tp_end_queue *q) { (void)q; }

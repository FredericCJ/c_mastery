#include "object_runtime_concurrent.h"
#include <stdlib.h>
static void unlock(struct orc_concurrent *c) {
  if (pthread_mutex_unlock(&c->mutex))
    abort();
}
enum or_status orcm_concurrent_init(struct orc_concurrent *c, enum orc_policy policy) {
  if (!c || policy > ORC_RESTART || policy < ORC_BLACK)
    return OR_INVALID;
  if (pthread_mutex_init(&c->mutex, NULL))
    return OR_FULL;
  orcm_init(&c->heap, policy);
  c->initialized = 1;
  return OR_OK;
}
enum or_status orcm_concurrent_call(struct orc_concurrent *c, const struct orc_request *q,
                              struct orc_response *out) {
  struct orc_response r = {0};
  enum or_status s;
  if (!c || !q || !out || !c->initialized)
    return OR_INVALID;
  if (pthread_mutex_lock(&c->mutex))
    return OR_STATE;
  switch (q->operation) {
  case ORC_ALLOCATE:
    s = orcm_allocate(&c->heap, q->target, q->second, &r.object);
    break;
  case ORC_RETAIN:
    s = orcm_retain(&c->heap, q->object);
    break;
  case ORC_RELEASE:
    s = orcm_release(&c->heap, q->object);
    break;
  case ORC_STORE:
    s = orcm_store(&c->heap, q->object, q->field, q->target);
    break;
  case ORC_READ:
    s = orcm_read(&c->heap, q->object, r.edge);
    break;
  case ORC_BEGIN:
    s = orcm_begin(&c->heap);
    break;
  case ORC_STEP:
    s = orcm_step(&c->heap, q->budget, &r.scanned);
    break;
  case ORC_PREPARE:
    s = orcm_prepare(&c->heap, &r.finish);
    break;
  case ORC_COMMIT:
    s = orcm_commit(&c->heap, q->finish, &r.dead);
    break;
  case ORC_INSPECT:
    r.invariant = (unsigned)orcm_invariant(&c->heap);
    r.live = c->heap.graph.live;
    r.roots = c->heap.graph.roots;
    r.work = c->heap.used;
    r.revision = c->heap.revision;
    s = OR_OK;
    break;
  default:
    s = OR_INVALID;
    break;
  }
  if (s == OR_OK || s == OR_DONE)
    *out = r;
  unlock(c);
  return s;
}
enum or_status orcm_concurrent_close(struct orc_concurrent *c) {
  if (!c || !c->initialized)
    return OR_INVALID;
  if (pthread_mutex_lock(&c->mutex))
    return OR_STATE;
  if ((c->heap.graph.live || c->heap.graph.active) &&
      c->heap.revision != ORC_REV_LIMIT) {
    unlock(c);
    return OR_STATE;
  }
  unlock(c);
  if (pthread_mutex_destroy(&c->mutex))
    return OR_STATE;
  c->initialized = 0;
  return OR_OK;
}

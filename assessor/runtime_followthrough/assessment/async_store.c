#include "async_store.h"
#ifndef VA_MUTANT
#define VA_MUTANT 0
#endif
static void *worker(void *context) {
  Va *a = context;
  pthread_mutex_lock(&a->mutex);
  unsigned cancelled = a->cancelled;
  pthread_mutex_unlock(&a->mutex);
  int32_t value = 0;
  int status = cancelled ? STORE_STATE
                         : vh_host(&a->host, a->request.service,
                                   a->request.argument, &value);
  pthread_mutex_lock(&a->mutex);
  a->result = value;
  a->status = status;
  a->done = 1;
  pthread_mutex_unlock(&a->mutex);
  return 0;
}
int va_init(Va *a) {
  if (!a)
    return VA_STATE;
  *a = (Va){0};
  if (pthread_mutex_init(&a->mutex, 0))
    return VA_STATE;
  a->initialized = 1;
  return VA_READY;
}
int va_submit(Va *a, Vs *s, const Vh *h) {
  if (!a || !a->initialized || a->active || !s || !h || !h->database)
    return VA_STATE;
  VsRequest request;
  if (vs_take(s, &request) != VS_PENDING)
    return VA_STATE;
  a->session = s;
  a->request = request;
  a->host = *h;
  a->done = a->cancelled = 0;
  a->active = 1;
  if (pthread_create(&a->worker, 0, worker, a)) {
    a->active = 0;
    a->session = 0;
    return vs_complete(s, request.epoch, request.serial, 1, 0);
  }
  return VA_BUSY;
}
int va_cancel(Va *a) {
  if (!a || !a->initialized || !a->active)
    return VA_STATE;
  pthread_mutex_lock(&a->mutex);
  a->cancelled = 1;
  pthread_mutex_unlock(&a->mutex);
  return VA_BUSY;
}
int va_poll(Va *a) {
  if (!a || !a->initialized || !a->active)
    return VA_STATE;
  pthread_mutex_lock(&a->mutex);
  unsigned done = a->done, cancelled = a->cancelled;
  int status = a->status;
  int32_t value = a->result;
  pthread_mutex_unlock(&a->mutex);
  if (!done)
    return VA_MUTANT == 1 ? VS_RUNNING : VA_BUSY;
  if (pthread_join(a->worker, 0))
    return VA_STATE;
  ++a->joined;
  Vs *s = a->session;
  a->session = 0;
  a->active = 0;
  return vs_complete(s, a->request.epoch + (VA_MUTANT == 3), a->request.serial,
                     status || (cancelled && VA_MUTANT != 2), value);
}
int va_destroy(Va *a) {
  if (!a || !a->initialized || a->active)
    return VA_STATE;
  if (pthread_mutex_destroy(&a->mutex))
    return VA_STATE;
  a->initialized = 0;
  return VA_READY;
}

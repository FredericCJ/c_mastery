#include "common.h"
struct pool {
  const int *v;
  size_t n, next, workers;
  pthread_mutex_t mutex;
};
struct worker {
  struct pool *pool;
  size_t id;
  long sum;
};
static void *work(void *arg) {
  struct worker *w = arg;
  struct pool *p = w->pool;
#ifdef HS_ALTERNATE
  for (size_t i = w->id; i < p->n; i += p->workers)
    w->sum += p->v[i];
#else
  for (;;) {
    pthread_mutex_lock(&p->mutex);
    size_t i = p->next++;
    pthread_mutex_unlock(&p->mutex);
    if (i >= p->n)
      break;
    w->sum += p->v[i];
  }
#endif
  return 0;
}
int hs_pool_sum(const int *v, size_t n, size_t workers, long *sum) {
  if ((!v && n) || !sum || n > 256 || !workers || workers > 4)
    return -1;
  struct pool p = {v, n, 0, workers, PTHREAD_MUTEX_INITIALIZER};
  pthread_t ids[4];
  struct worker w[4];
  size_t created = 0;
  for (; created < workers; created++) {
    w[created] = (struct worker){&p, created, 0};
    if (pthread_create(&ids[created], 0, work, &w[created]))
      break;
  }
  long total = 0;
  int failed = created != workers;
  for (size_t i = 0; i < created; i++) {
    if (pthread_join(ids[i], 0))
      failed = 1;
    total += w[i].sum;
  }
  pthread_mutex_destroy(&p.mutex);
  if (failed)
    return -1;
  *sum = total;
  return 0;
}

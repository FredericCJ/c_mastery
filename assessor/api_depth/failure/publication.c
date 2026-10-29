#include "publication.h"
#include <pthread.h>
#include <stdlib.h>
struct ae_published_node {
  struct ae_config config;
  unsigned references;
};
struct ae_publication {
  pthread_mutex_t lock;
  struct ae_published_node *current;
  unsigned made, freed, live, closed;
};
static void drop(struct ae_publication *p, struct ae_published_node *n) {
  if (--n->references == 0) {
    --p->live;
    ++p->freed;
    free(n);
  }
}
struct ae_publication *ae_publication_new(const struct ae_config *c) {
  if (!c || c->count > 3)
    return NULL;
  struct ae_publication *p = calloc(1, sizeof *p);
  if (!p)
    return NULL;
  if (pthread_mutex_init(&p->lock, NULL)) {
    free(p);
    return NULL;
  }
  p->current = malloc(sizeof *p->current);
  if (!p->current) {
    pthread_mutex_destroy(&p->lock);
    free(p);
    return NULL;
  }
  *p->current = (struct ae_published_node){*c, 1};
  p->live = p->made = 1;
  return p;
}
enum ae_code ae_publication_replace(struct ae_publication *p, const struct ae_config *c,
                              unsigned fail) {
  if (!p || !c || c->count > 3 || fail > 1)
    return AE_INVALID;
  if (fail)
    return AE_EXHAUSTED;
  struct ae_published_node *n = malloc(sizeof *n);
  if (!n)
    return AE_EXHAUSTED;
  *n = (struct ae_published_node){*c, 1};
  pthread_mutex_lock(&p->lock);
  if (p->closed || p->live == 3 || p->made == 65535) {
    pthread_mutex_unlock(&p->lock);
    free(n);
    return AE_FULL;
  }
  struct ae_published_node *old = p->current;
  p->current = n;
  ++p->made;
  ++p->live;
  drop(p, old);
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
enum ae_code ae_publication_acquire(struct ae_publication *p, struct ae_read_lease *out) {
  if (!p || !out)
    return AE_INVALID;
  pthread_mutex_lock(&p->lock);
  if (p->closed || p->current->references == 65535) {
    pthread_mutex_unlock(&p->lock);
    return AE_FULL;
  }
#ifndef AE_MUTANT_UNPINNED
  ++p->current->references;
#endif
  *out = (struct ae_read_lease){p, p->current};
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
enum ae_code ae_publication_read(const struct ae_read_lease *l, struct ae_config *out) {
  if (!l || !l->owner || !l->node || !out)
    return AE_INVALID;
  *out = l->node->config;
  return AE_OK;
}
enum ae_code ae_publication_release(struct ae_read_lease *l) {
  if (!l || !l->owner || !l->node)
    return AE_INVALID;
  struct ae_publication *p = l->owner;
  pthread_mutex_lock(&p->lock);
  drop(p, l->node);
  *l = (struct ae_read_lease){0};
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
enum ae_code ae_publication_counts(struct ae_publication *p, unsigned *m, unsigned *f,
                             unsigned *l) {
  if (!p || !m || !f || !l)
    return AE_INVALID;
  pthread_mutex_lock(&p->lock);
  *m = p->made;
  *f = p->freed;
  *l = p->live;
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
enum ae_code ae_publication_close(struct ae_publication *p) {
  if (!p)
    return AE_INVALID;
  pthread_mutex_lock(&p->lock);
  if (!p->closed) {
    p->closed = 1;
    drop(p, p->current);
    p->current = NULL;
  }
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
enum ae_code ae_publication_destroy(struct ae_publication **owner) {
  if (!owner || !*owner)
    return AE_INVALID;
  struct ae_publication *p = *owner;
  pthread_mutex_lock(&p->lock);
  unsigned ready = p->closed && !p->live;
  pthread_mutex_unlock(&p->lock);
  if (!ready)
    return AE_AGAIN;
  pthread_mutex_destroy(&p->lock);
  free(p);
  *owner = NULL;
  return AE_OK;
}

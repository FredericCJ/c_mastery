#include "publication.h"
#include <pthread.h>
#include <stdlib.h>
struct AePublishedNode {
  AeConfig config;
  unsigned references;
};
struct AePublication {
  pthread_mutex_t lock;
  AePublishedNode *current;
  unsigned made, freed, live, closed;
};
static void drop(AePublication *p, AePublishedNode *n) {
  if (--n->references == 0) {
    --p->live;
    ++p->freed;
    free(n);
  }
}
AePublication *ae_publication_new(const AeConfig *c) {
  if (!c || c->count > 3)
    return NULL;
  AePublication *p = calloc(1, sizeof *p);
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
  *p->current = (AePublishedNode){*c, 1};
  p->live = p->made = 1;
  return p;
}
AeCode ae_publication_replace(AePublication *p, const AeConfig *c,
                              unsigned fail) {
  if (!p || !c || c->count > 3 || fail > 1)
    return AE_INVALID;
  if (fail)
    return AE_EXHAUSTED;
  AePublishedNode *n = malloc(sizeof *n);
  if (!n)
    return AE_EXHAUSTED;
  *n = (AePublishedNode){*c, 1};
  pthread_mutex_lock(&p->lock);
  if (p->closed || p->live == 3 || p->made == 65535) {
    pthread_mutex_unlock(&p->lock);
    free(n);
    return AE_FULL;
  }
  AePublishedNode *old = p->current;
  p->current = n;
  ++p->made;
  ++p->live;
  drop(p, old);
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
AeCode ae_publication_acquire(AePublication *p, AeReadLease *out) {
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
  *out = (AeReadLease){p, p->current};
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
AeCode ae_publication_read(const AeReadLease *l, AeConfig *out) {
  if (!l || !l->owner || !l->node || !out)
    return AE_INVALID;
  *out = l->node->config;
  return AE_OK;
}
AeCode ae_publication_release(AeReadLease *l) {
  if (!l || !l->owner || !l->node)
    return AE_INVALID;
  AePublication *p = l->owner;
  pthread_mutex_lock(&p->lock);
  drop(p, l->node);
  *l = (AeReadLease){0};
  pthread_mutex_unlock(&p->lock);
  return AE_OK;
}
AeCode ae_publication_counts(AePublication *p, unsigned *m, unsigned *f,
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
AeCode ae_publication_close(AePublication *p) {
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
AeCode ae_publication_destroy(AePublication **owner) {
  if (!owner || !*owner)
    return AE_INVALID;
  AePublication *p = *owner;
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

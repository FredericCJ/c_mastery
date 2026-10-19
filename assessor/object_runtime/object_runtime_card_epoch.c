#include "object_runtime_card_epoch.h"
#include <stdlib.h>
#ifndef ORCC_MUTANT
#define ORCC_MUTANT 0
#endif
static ORCCMeta *meta(ORCCards *c, unsigned i) {
  return c->padded ? &c->metadata.padded[i].meta : &c->metadata.packed[i];
}
static void unlock(ORCCards *c) {
  if (pthread_mutex_unlock(&c->mutex))
    abort();
}
ORStatus orcc_init(ORCCards *c, unsigned width, int padded) {
  unsigned i, j;
  if (!c || (width != 1 && width != 2 && width != 4 && width != 8) ||
      (padded != 0 && padded != 1))
    return OR_INVALID;
  if (pthread_mutex_init(&c->mutex, NULL))
    return OR_FULL;
  c->width = width;
  c->padded = (unsigned)padded;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 2; j++)
      c->edge[i][j] = 0;
    *meta(c, i) = (ORCCMeta){1, 0};
  }
  c->initialized = 1;
  return OR_OK;
}
ORStatus orcc_store(ORCCards *c, unsigned object, unsigned field,
                    unsigned target) {
  ORCCMeta *m;
  if (!c || !c->initialized || object >= 8 || field >= 2 || target > 16)
    return OR_INVALID;
  if (pthread_mutex_lock(&c->mutex))
    return OR_STATE;
  m = meta(c, object / c->width);
  if (m->revision == ORCC_REV_LIMIT) {
    unlock(c);
    return OR_FULL;
  }
  c->edge[object][field] = (uint8_t)target;
  if (ORCC_MUTANT != 2 && target && target <= 8)
    m->dirty = 1;
  m->revision++;
  unlock(c);
  return OR_OK;
}
ORStatus orcc_prepare(ORCCards *c, unsigned card, ORCCSnapshot *out) {
  ORCCSnapshot s;
  ORCCMeta *m;
  unsigned i;
  if (!c || !out || !c->initialized || card >= 8u / c->width)
    return OR_INVALID;
  if (pthread_mutex_lock(&c->mutex))
    return OR_STATE;
  m = meta(c, card);
  if (!m->dirty) {
    unlock(c);
    return OR_DONE;
  }
  s = (ORCCSnapshot){.owner = c,
                     .card = card,
                     .count = 2u * c->width,
                     .revision = m->revision};
  for (i = 0; i < s.count; i++)
    s.edge[i] = c->edge[card * c->width + i / 2][i % 2];
  *out = s;
  unlock(c);
  return OR_OK;
}
ORStatus orcc_scan_commit(ORCCards *c, const ORCCSnapshot *s, uint16_t *roots,
                          unsigned *scanned) {
  uint16_t r = 0;
  unsigned i;
  ORCCMeta *m;
  if (!c || !s || !roots || !scanned || !c->initialized || s->owner != c ||
      s->card >= 8u / c->width || s->count != 2u * c->width)
    return OR_INVALID;
  /* Snapshot fields are immutable private copies, so this loop requires no
     shared-payload reads outside the mutex. */
  for (i = 0; i < s->count; i++) {
    unsigned target = s->edge[i];
    if (target > 16)
      return OR_INVALID;
    if (target && target <= 8)
      r |= (uint16_t)(UINT32_C(1) << (target - 1));
  }
  if (pthread_mutex_lock(&c->mutex))
    return OR_STATE;
  m = meta(c, s->card);
  if (ORCC_MUTANT != 1 && m->revision != s->revision) {
    unlock(c);
    return OR_STATE;
  }
  m->dirty = (uint8_t)!!r;
  *roots = r;
  *scanned = s->count;
  unlock(c);
  return OR_OK;
}
ORStatus orcc_close(ORCCards *c) {
  if (!c || !c->initialized)
    return OR_INVALID;
  /* Owner has joined every user and discarded every prepared snapshot. */
  if (pthread_mutex_destroy(&c->mutex))
    return OR_STATE;
  c->initialized = 0;
  return OR_OK;
}

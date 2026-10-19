#include "dispatch_intrusive.h"
#ifndef DS_INTRUSIVE_MUTANT
#define DS_INTRUSIVE_MUTANT 0
#endif
void ds_dual_init(DSDualList *l, unsigned kind) {
  l->head = (DSHook){&l->head, &l->head, l, NULL, kind};
  l->kind = kind; l->count = 0;
}
void ds_dual_item_init(DSDualItem *i, int value) {
  i->value = value;
  for (unsigned k = 0; k < 2; ++k)
    i->hooks[k] = (DSHook){NULL, NULL, NULL, i, k};
}
int ds_dual_push(DSDualList *l, DSDualItem *i) {
  if (!l || !i || l->kind >= 2) return DS_INVALID;
  DSHook *h = &i->hooks[l->kind];
  if (h->owner) return DS_BUSY;
  if (DS_INTRUSIVE_MUTANT == 1 && i->hooks[1-l->kind].owner) return DS_BUSY;
  if (l->count == 4) return DS_FULL;
  h->owner = l; h->previous = l->head.previous; h->next = &l->head;
  h->previous->next = h; l->head.previous = h; ++l->count;
  return DS_OK;
}
int ds_dual_remove(DSDualList *l, DSDualItem *i) {
  if (!l || !i || l->kind >= 2) return DS_INVALID;
  DSHook *h = &i->hooks[l->kind];
  if (h->owner != l) return DS_STALE;
  h->previous->next = h->next;
  if (DS_INTRUSIVE_MUTANT != 2) h->next->previous = h->previous;
  h->owner = NULL; h->previous = NULL; h->next = NULL; --l->count;
  return DS_OK;
}
int ds_dual_fold(const DSDualList *l, DSCombine op, int initial, int *out) {
  if (!l || !op || !out || l->kind >= 2 || l->count > 4) return DS_INVALID;
  const DSHook *h = l->head.next, *previous = &l->head;
  int result = initial;
  for (unsigned n = 0; n < l->count; ++n) {
    if (!h || h == &l->head || h->owner != l || h->previous != previous ||
        h->kind != l->kind || !h->parent || &h->parent->hooks[l->kind] != h)
      return DS_INVALID;
    int next = 0, rc = op(result, h->parent->value, &next);
    if (rc != DS_OK) return rc;
    result = next; previous = h; h = h->next;
  }
  if (h != &l->head || previous != l->head.previous) return DS_INVALID;
  *out = result; return DS_OK;
}
void ds_index_init(DSIndexOwner *o, DSIndexSlot slots[4]) {
  o->slots = slots;
  for (unsigned i = 0; i < 4; ++i) {
    o->live[i] = 0; o->generation[i] = 1;
    slots[i] = (DSIndexSlot){0,{4,4},{4,4},{0,0}};
  }
  for (unsigned k = 0; k < 2; ++k) { o->head[k] = 4; o->tail[k] = 4; o->count[k] = 0; }
}
static int valid(const DSIndexOwner *o, DSIndexHandle h) {
  return o && o->slots && h.owner == o && h.index < 4 && o->live[h.index] &&
    (DS_INTRUSIVE_MUTANT == 3 || h.generation == o->generation[h.index]);
}
int ds_index_new(DSIndexOwner *o, int value, DSIndexHandle *out) {
  if (!o || !o->slots || !out) return DS_INVALID;
  for (unsigned i = 0; i < 4; ++i) if (!o->live[i] && o->generation[i] <= 3) {
    o->slots[i] = (DSIndexSlot){value,{4,4},{4,4},{0,0}}; o->live[i] = 1;
    *out = (DSIndexHandle){o, i, o->generation[i]}; return DS_OK;
  }
  return DS_FULL;
}
int ds_index_get(const DSIndexOwner *o, DSIndexHandle h, int *out) {
  if (!o || !out) return DS_INVALID;
  if (!valid(o,h)) return DS_STALE;
  *out = o->slots[h.index].value; return DS_OK;
}
int ds_index_push(DSIndexOwner *o, unsigned k, DSIndexHandle h) {
  if (!o || k >= 2) return DS_INVALID;
  if (!valid(o,h)) return DS_STALE;
  DSIndexSlot *s = &o->slots[h.index];
  if (s->linked[k]) return DS_BUSY;
  s->previous[k] = o->tail[k]; s->next[k] = 4;
  if (o->tail[k] == 4) o->head[k] = h.index;
  else o->slots[o->tail[k]].next[k] = h.index;
  o->tail[k] = h.index; s->linked[k] = 1; ++o->count[k]; return DS_OK;
}
int ds_index_remove(DSIndexOwner *o, unsigned k, DSIndexHandle h) {
  if (!o || k >= 2) return DS_INVALID;
  if (!valid(o,h)) return DS_STALE;
  DSIndexSlot *s = &o->slots[h.index];
  if (!s->linked[k]) return DS_STALE;
  if (s->previous[k] == 4) o->head[k] = s->next[k];
  else o->slots[s->previous[k]].next[k] = s->next[k];
  if (s->next[k] == 4) o->tail[k] = s->previous[k];
  else o->slots[s->next[k]].previous[k] = s->previous[k];
  s->linked[k] = 0; s->previous[k] = 4; s->next[k] = 4;
  --o->count[k]; return DS_OK;
}
int ds_index_release(DSIndexOwner *o, DSIndexHandle h) {
  if (!o) return DS_INVALID;
  if (!valid(o,h)) return DS_STALE;
  if (o->slots[h.index].linked[0] || o->slots[h.index].linked[1]) return DS_BUSY;
  o->live[h.index] = 0; ++o->generation[h.index]; return DS_OK;
}
int ds_index_relocate(DSIndexOwner *o, DSIndexSlot slots[4]) {
  if (!o || !o->slots || !slots) return DS_INVALID;
  if (o->slots == slots) return DS_OK;
  for (unsigned i = 0; i < 4; ++i) slots[i] = o->slots[i];
  if (DS_INTRUSIVE_MUTANT != 4) o->slots = slots;
  return DS_OK;
}
int ds_index_fold(const DSIndexOwner *o, unsigned k, DSCombine op, int initial, int *out) {
  if (!o || !o->slots || k >= 2 || !op || !out || o->count[k] > 4) return DS_INVALID;
  unsigned index = o->head[k], previous = 4;
  int result = initial;
  for (unsigned n = 0; n < o->count[k]; ++n) {
    if (index >= 4 || !o->live[index]) return DS_INVALID;
    const DSIndexSlot *s = &o->slots[index];
    if (!s->linked[k] || s->previous[k] != previous) return DS_INVALID;
    int next = 0, rc = op(result, s->value, &next);
    if (rc != DS_OK) return rc;
    result = next; previous = index; index = s->next[k];
  }
  if (index != 4 || previous != o->tail[k]) return DS_INVALID;
  *out = result; return DS_OK;
}

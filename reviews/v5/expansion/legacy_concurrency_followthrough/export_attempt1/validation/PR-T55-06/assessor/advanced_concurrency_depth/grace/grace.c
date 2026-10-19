#include "grace.h"
#include <limits.h>
static int lease_valid(const AGDomain *d, AGLease l) {
  return d && l.owner == d && l.participant < 4 &&
         d->registered[l.participant] &&
         d->generation[l.participant] == l.generation;
}
void ag_init(AGDomain *d, unsigned nested) {
  cmx_epoch_init(&d->retained);
  d->nested = !!nested;
  for (unsigned p = 0; p < 4; p++) {
    d->registered[p] = d->depth[p] = d->section[p] = 0;
    d->generation[p] = 1;
  }
}
int ag_register(AGDomain *d, AGLease *out) {
  if (!d || !out)
    return AG_INVALID;
  for (unsigned p = 0; p < 4; p++)
    if (!d->registered[p] && d->generation[p] != UINT_MAX) {
      d->registered[p] = 1;
      out->owner = d;
      out->participant = p;
      out->generation = d->generation[p];
      return AG_OK;
    }
  return AG_FULL;
}
int ag_enter(AGDomain *d, AGLease l) {
  if (!lease_valid(d, l))
    return AG_INVALID;
  unsigned p = l.participant;
  if (d->depth[p]) {
    if (!d->nested)
      return AG_INVALID;
    if (d->depth[p] == UINT_MAX)
      return AG_EXHAUSTED;
    d->depth[p]++;
    return AG_OK;
  }
  if (d->section[p] == UINT_MAX)
    return AG_EXHAUSTED;
  if (cmx_epoch_enter(&d->retained, p) != CMX_OK)
    return AG_INVALID;
  d->depth[p] = 1;
  d->section[p]++;
  return AG_OK;
}
int ag_leave(AGDomain *d, AGLease l) {
  if (!lease_valid(d, l) || !d->depth[l.participant])
    return AG_INVALID;
  unsigned p = l.participant;
  d->depth[p]--;
#ifdef AG_MUTANT_NESTED
  cmx_epoch_leave(&d->retained, p);
#else
  if (!d->depth[p])
    cmx_epoch_leave(&d->retained, p);
#endif
  return AG_OK;
}
int ag_depart(AGDomain *d, AGLease l) {
  if (!lease_valid(d, l))
    return AG_INVALID;
  if (d->depth[l.participant])
    return AG_BUSY;
  d->registered[l.participant] = 0;
  d->generation[l.participant]++;
  return AG_OK;
}
int ag_advance(AGDomain *d) {
  if (!d)
    return AG_INVALID;
  if (d->retained.epoch == UINT32_MAX)
    return AG_EXHAUSTED;
  return cmx_epoch_advance(&d->retained) ? AG_OK : AG_PENDING;
}
int ag_reclaimable(const AGDomain *d, unsigned retired) {
  if (!d || retired > d->retained.epoch)
    return 0;
  for (unsigned p = 0; p < 4; p++)
    if (d->retained.active[p] && d->retained.seen[p]
#ifdef AG_MUTANT_EPOCH
                                     < retired
#else
                                     <= retired
#endif
    )
      return 0;
  return 1;
}
void ag_store_init(AGStore *s, unsigned nested, unsigned char initial) {
  ag_init(&s->domain, nested);
  s->current = 0;
  s->writer = AG_NONE;
  s->drops = s->invocations = 0;
  api_registry_init(&s->callbacks);
  for (unsigned i = 0; i < AG_VERSIONS; i++) {
    AGObject *o = &s->objects[i];
    o->active = i == 0;
    o->generation = i == 0;
    o->retired = o->retired_epoch = o->refs = 0;
    for (unsigned j = 0; j < 32; j++)
      o->bytes[j] = initial;
  }
  for (unsigned i = 0; i < 4; i++)
    s->capture[i].occupied = 0;
}
int ag_writer_acquire(AGStore *s, unsigned writer) {
  if (!s || writer == AG_NONE)
    return AG_INVALID;
  if (s->writer != AG_NONE
#ifdef AG_MUTANT_WRITER
      && 0
#endif
  )
    return AG_BUSY;
  s->writer = writer;
  return AG_OK;
}
int ag_writer_release(AGStore *s, unsigned writer) {
  if (!s || writer == AG_NONE || s->writer != writer)
    return AG_INVALID;
  s->writer = AG_NONE;
  return AG_OK;
}
int ag_publish(AGStore *s, unsigned writer, unsigned char value) {
  if (!s || writer == AG_NONE || s->writer != writer)
    return AG_INVALID;
  if (s->domain.retained.epoch == UINT32_MAX)
    return AG_EXHAUSTED;
  unsigned at = AG_NONE;
  for (unsigned i = 0; i < AG_VERSIONS; i++)
    if (!s->objects[i].active && s->objects[i].generation != UINT_MAX) {
      at = i;
      break;
    }
  if (at == AG_NONE)
    return AG_FULL;
  AGObject *fresh = &s->objects[at], *old = &s->objects[s->current];
  fresh->generation++;
  fresh->active = 1;
  fresh->retired = fresh->retired_epoch = fresh->refs = 0;
  for (unsigned j = 0; j < 32; j++)
    fresh->bytes[j] = value;
  old->retired = 1;
  old->retired_epoch = s->domain.retained.epoch;
  s->current = at;
  (void)ag_advance(&s->domain);
  return AG_OK;
}
static int version_valid(AGStore *s, AGVersion v) {
  return v.index < AG_VERSIONS && s->objects[v.index].active &&
         s->objects[v.index].generation == v.generation;
}
static int borrow_valid(AGStore *s, AGBorrow b) {
  return s && lease_valid(&s->domain, b.lease) &&
         s->domain.depth[b.lease.participant] &&
         s->domain.section[b.lease.participant] == b.section &&
         version_valid(s, b.version);
}
int ag_read(AGStore *s, AGLease l, AGBorrow *out) {
  if (!s || !out)
    return AG_INVALID;
  int r = ag_enter(&s->domain, l);
  if (r != AG_OK)
    return r;
  out->lease = l;
  out->section = s->domain.section[l.participant];
  out->version = (AGVersion){s->current, s->objects[s->current].generation};
  return AG_OK;
}
int ag_copy(AGStore *s, AGBorrow b, unsigned char out[32]) {
  if (!out || !borrow_valid(s, b))
    return AG_INVALID;
  for (unsigned i = 0; i < 32; i++)
    out[i] = s->objects[b.version.index].bytes[i];
  return AG_OK;
}
int ag_read_end(AGStore *s, AGBorrow b) {
  if (!borrow_valid(s, b))
    return AG_INVALID;
  return ag_leave(&s->domain, b.lease);
}
unsigned ag_collect(AGStore *s) {
  unsigned count = 0;
  for (unsigned i = 0; i < AG_VERSIONS; i++) {
    AGObject *o = &s->objects[i];
    if (o->active && o->retired && ag_reclaimable(&s->domain, o->retired_epoch)
#ifndef AG_MUTANT_REFS
        && !o->refs
#endif
    ) {
      o->active = 0;
      count++;
    }
  }
  return count;
}
static void captured(void *ctx, ApiRegistry *r, ApiHandle h) {
  (void)r;
  AGCapture *c = ctx;
  AGStore *s = c->owner;
  if (version_valid(s, c->version) && s->objects[c->version.index].refs) {
    s->invocations++;
    c->callback(c->context, s, h, s->objects[c->version.index].bytes);
  }
}
static void released(void *ctx) {
  AGCapture *c = ctx;
  AGStore *s = c->owner;
  AGObject *o = &s->objects[c->version.index];
  o->refs--;
  c->occupied = 0;
  s->drops++;
}
ApiStatus ag_capture(AGStore *s, AGBorrow b, AGCallback cb, void *ctx,
                     ApiHandle *out) {
  if (!cb || !out || !borrow_valid(s, b))
    return API_INVALID;
  AGObject *o = &s->objects[b.version.index];
  if (o->refs == UINT_MAX)
    return API_FULL;
  for (unsigned i = 0; i < 4; i++)
    if (!s->capture[i].occupied) {
      AGCapture *c = &s->capture[i];
      c->owner = s;
      c->version = b.version;
      c->callback = cb;
      c->context = ctx;
      c->occupied = 1;
#ifndef AG_MUTANT_CAPTURE
      o->refs++;
#endif
      ApiStatus result = api_add(&s->callbacks, captured, c, released, out);
      if (result != API_OK) {
        o->refs--;
        c->occupied = 0;
      }
      return result;
    }
  return API_FULL;
}
ApiStatus ag_dispatch(AGStore *s) {
  return s ? api_dispatch(&s->callbacks) : API_INVALID;
}
ApiStatus ag_remove(AGStore *s, ApiHandle h) {
  return s ? api_remove(&s->callbacks, h) : API_INVALID;
}
ApiStatus ag_destroy_callbacks(AGStore *s) {
  return s ? api_destroy(&s->callbacks) : API_INVALID;
}

#include "object_runtime_shapes.h"
#ifndef ORS_MUTANT
#define ORS_MUTANT 0
#endif
static int eq(ORXHandle a, ORXHandle b) {
  return a.slot == b.slot && a.generation == b.generation;
}
static int shape_eq(ORXShapeId a, ORXShapeId b) {
  return a.slot == b.slot && a.generation == b.generation;
}
static int valid(const ORXWorld *w, ORXHandle h) {
  return w && h.slot < 8 && h.generation && w->heap.entry[h.slot].live &&
         w->heap.entry[h.slot].generation == h.generation &&
         w->object[h.slot].generation == h.generation;
}
static int shape_valid(const ORXWorld *w, ORXShapeId s) {
  return s.slot < 16 && s.generation && w->shape[s.slot].live &&
         w->shape[s.slot].generation == s.generation;
}
static ORXHandle parent(const ORXWorld *w, ORXHandle h) {
  return w->heap.cell[w->heap.entry[h.slot].position].edge[0];
}
static int find(const ORXDescriptor *d, unsigned key) {
  unsigned i;
  for (i = 0; i < d->count; i++)
    if (d->key[i] == key)
      return (int)i;
  return -1;
}
static int descriptor_eq(const ORXDescriptor *a, const ORXDescriptor *b) {
  unsigned i;
  if (a->count != b->count)
    return 0;
  for (i = 0; i < a->count; i++)
    if (a->key[i] != b->key[i] || a->attributes[i] != b->attributes[i])
      return 0;
  return 1;
}
static void drop_shape(ORXWorld *w, ORXShapeId s) {
  ORXDescriptor *d = &w->shape[s.slot];
  d->refs--;
  if (!s.slot)
    return;
  if (!d->refs) {
    d->live = 0;
    d->generation++;
  }
}
static ORStatus acquire_shape(ORXWorld *w, const ORXDescriptor *wanted,
                              ORXShapeId *out) {
  unsigned i;
  for (i = 0; i < 16; i++)
    if (w->shape[i].live && descriptor_eq(&w->shape[i], wanted)) {
      w->shape[i].refs++;
      *out = (ORXShapeId){(uint8_t)i, w->shape[i].generation};
      return OR_OK;
    }
  for (i = 1; i < 16; i++)
    if (!w->shape[i].live && w->shape[i].generation <= 3) {
      uint8_t g = w->shape[i].generation;
      w->shape[i] = *wanted;
      w->shape[i].live = 1;
      w->shape[i].generation = g;
      w->shape[i].refs = 1;
      *out = (ORXShapeId){(uint8_t)i, g};
      return OR_OK;
    }
  return OR_FULL;
}
void orx_world_init(ORXWorld *w) {
  unsigned i;
  if (!w)
    return;
  *w = (ORXWorld){0};
  orx_heap_init(&w->heap);
  w->epoch = 1;
  for (i = 0; i < 16; i++)
    w->shape[i].generation = 1;
  w->shape[0].live = 1;
}
ORStatus orx_world_new(ORXWorld *w, ORXHandle *out) {
  ORXHandle h;
  ORStatus s;
  if (!w || !out || !w->epoch)
    return OR_INVALID;
  s = orx_new(&w->heap, NULL, 0, &h);
  if (s != OR_OK)
    return s;
  w->object[h.slot] = (ORXObject){.generation = h.generation, .shape = {0, 1}};
  w->shape[0].refs++;
  *out = h;
  return OR_OK;
}
ORStatus orx_world_collect(ORXWorld *w, unsigned *dead) {
  unsigned i, n;
  ORStatus s;
  if (!w || !dead || !w->epoch)
    return OR_INVALID;
  if (w->epoch == UINT8_MAX)
    return OR_FULL;
  s = orx_collect(&w->heap, &n);
  if (s != OR_OK)
    return s;
  for (i = 0; i < 8; i++)
    if (w->object[i].generation && !w->heap.entry[i].live) {
      drop_shape(w, w->object[i].shape);
      w->object[i] = (ORXObject){0};
    }
  w->epoch++;
  *dead = n;
  return OR_OK;
}
ORStatus orx_define(ORXWorld *w, ORXHandle h, unsigned key, unsigned attributes,
                    int32_t value, ORXGetter getter, void *context) {
  ORXObject object;
  ORXDescriptor wanted;
  ORXShapeId shape;
  int offset;
  ORStatus s;
  if (!valid(w, h) || !key || key > 255 || attributes > 3 ||
      ((attributes & ORX_ACCESSOR) && !getter))
    return OR_INVALID;
  object = w->object[h.slot];
  wanted = w->shape[object.shape.slot];
  offset = find(&wanted, key);
  if (offset >= 0 && (wanted.attributes[offset] & ORX_READONLY) &&
      ORS_MUTANT != 1)
    return OR_STATE;
  if (offset < 0) {
    if (wanted.count == 4)
      return OR_FULL;
    offset = wanted.count++;
    wanted.key[offset] = (uint8_t)key;
    wanted.attributes[offset] = (uint8_t)attributes;
  } else
    wanted.attributes[offset] = (uint8_t)attributes;
  object.field[offset] = (ORXField){value, getter, context};
  if (descriptor_eq(&wanted, &w->shape[object.shape.slot])) {
    w->object[h.slot] = object;
    return OR_OK;
  }
  if (w->epoch == UINT8_MAX)
    return OR_FULL;
  s = acquire_shape(w, &wanted, &shape);
  if (s != OR_OK)
    return s;
  drop_shape(w, object.shape);
  object.shape = shape;
  w->object[h.slot] = object;
  w->epoch++;
  return OR_OK;
}
ORStatus orx_delete(ORXWorld *w, ORXHandle h, unsigned key) {
  ORXObject object;
  ORXDescriptor wanted;
  ORXShapeId shape;
  int offset;
  unsigned i;
  ORStatus s;
  if (!valid(w, h) || !key || key > 255)
    return OR_INVALID;
  object = w->object[h.slot];
  wanted = w->shape[object.shape.slot];
  offset = find(&wanted, key);
  if (offset < 0)
    return OR_MISS;
  if ((wanted.attributes[offset] & ORX_READONLY) && ORS_MUTANT != 1)
    return OR_STATE;
  if (w->epoch == UINT8_MAX)
    return OR_FULL;
  for (i = (unsigned)offset; i + 1 < wanted.count; i++) {
    wanted.key[i] = wanted.key[i + 1];
    wanted.attributes[i] = wanted.attributes[i + 1];
    object.field[i] = object.field[i + 1];
  }
  wanted.count--;
  wanted.key[wanted.count] = wanted.attributes[wanted.count] = 0;
  object.field[wanted.count] = (ORXField){0};
  s = acquire_shape(w, &wanted, &shape);
  if (s != OR_OK)
    return s;
  drop_shape(w, object.shape);
  object.shape = shape;
  w->object[h.slot] = object;
  w->epoch++;
  return OR_OK;
}
ORStatus orx_prototype(ORXWorld *w, ORXHandle object, ORXHandle prototype) {
  ORXHandle p = prototype;
  unsigned visits = 0;
  ORStatus s;
  if (!valid(w, object) || (prototype.generation && !valid(w, prototype)) ||
      (!prototype.generation && prototype.slot))
    return OR_INVALID;
  while (p.generation) {
    if ((eq(p, object) && ORS_MUTANT != 2) || visits++ == 8)
      return OR_STATE;
    p = parent(w, p);
  }
  if (eq(parent(w, object), prototype))
    return OR_OK;
  if (w->epoch == UINT8_MAX)
    return OR_FULL;
  s = orx_edge(&w->heap, object, 0, prototype);
  if (s != OR_OK)
    return s;
  w->epoch++;
  return OR_OK;
}
ORStatus orx_get(ORXWorld *w, ORXHandle receiver, unsigned key,
                 ORXPropertyCache *cache, int32_t *out) {
  ORXPropertyCache next;
  ORXHandle holder;
  ORXObject object;
  ORXField field;
  int offset = -1;
  unsigned visits = 0;
  int hit;
  ORStatus s;
  int32_t value;
  uint8_t epoch;
  if (!valid(w, receiver) || !cache || !out || !key || key > 255)
    return OR_INVALID;
  if (w->in_getter)
    return OR_STATE;
  next = cache->world == w ? *cache : (ORXPropertyCache){.world = w};
  hit = next.valid && eq(next.receiver, receiver) && next.key == key &&
        (ORS_MUTANT == 3 || next.epoch == w->epoch) && valid(w, next.holder) &&
        shape_valid(w, next.receiver_shape) &&
        shape_valid(w, next.holder_shape) &&
        shape_eq(next.receiver_shape, w->object[receiver.slot].shape) &&
        shape_eq(next.holder_shape, w->object[next.holder.slot].shape);
  if (hit) {
    holder = next.holder;
    offset = next.offset;
    if (next.hits == UINT32_MAX)
      return OR_FULL;
    next.hits++;
  } else {
    if (next.misses == UINT32_MAX)
      return OR_FULL;
    next.misses++;
    next.valid = 0;
    holder = receiver;
    while (holder.generation) {
      if (!valid(w, holder) || visits++ == 8)
        return OR_STATE;
      object = w->object[holder.slot];
      offset = find(&w->shape[object.shape.slot], key);
      if (offset >= 0)
        break;
      holder = parent(w, holder);
    }
  }
  if (offset < 0) {
    *cache = next;
    return OR_MISS;
  }
  object = w->object[holder.slot];
  field = object.field[offset];
  epoch = w->epoch;
  if (w->shape[object.shape.slot].attributes[offset] & ORX_ACCESSOR) {
    s = orx_retain(&w->heap, receiver);
    if (s != OR_OK)
      return s;
    if (!eq(holder, receiver)) {
      s = orx_retain(&w->heap, holder);
      if (s != OR_OK) {
        (void)orx_release(&w->heap, receiver);
        return s;
      }
    }
    w->in_getter = 1;
    s = field.getter(field.context, w, receiver, &value);
    w->in_getter = 0;
    if (!eq(holder, receiver))
      (void)orx_release(&w->heap, holder);
    (void)orx_release(&w->heap, receiver);
    if (ORS_MUTANT == 4)
      value = w->object[holder.slot].field[offset].value;
  } else {
    s = OR_OK;
    value = field.value;
  }
  if (s != OR_OK) {
    next.valid = 0;
    *cache = next;
    return s;
  }
  if (epoch == w->epoch) {
    next.receiver = receiver;
    next.holder = holder;
    next.receiver_shape = w->object[receiver.slot].shape;
    next.holder_shape = object.shape;
    next.epoch = epoch;
    next.key = (uint8_t)key;
    next.offset = (uint8_t)offset;
    next.valid = 1;
  } else
    next.valid = 0;
  *cache = next;
  *out = value;
  return OR_OK;
}

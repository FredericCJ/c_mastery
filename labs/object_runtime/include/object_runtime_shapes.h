#ifndef CMASTERY_OBJECT_RUNTIME_SHAPES_H
#define CMASTERY_OBJECT_RUNTIME_SHAPES_H
#include "object_runtime_ext.h"
typedef struct ORXWorld ORXWorld;
typedef ORStatus (*ORXGetter)(void *, ORXWorld *, ORXHandle, int32_t *);
typedef struct {
  uint8_t slot, generation;
} ORXShapeId;
typedef struct {
  uint8_t live, generation, refs, count, key[4], attributes[4];
} ORXDescriptor;
typedef struct {
  int32_t value;
  ORXGetter getter;
  void *context;
} ORXField;
typedef struct {
  uint8_t generation;
  ORXShapeId shape;
  ORXField field[4];
} ORXObject;
struct ORXWorld {
  ORXHeap heap;
  ORXDescriptor shape[16];
  ORXObject object[8];
  uint8_t epoch, in_getter;
};
typedef struct {
  const ORXWorld *world;
  ORXHandle receiver, holder;
  ORXShapeId receiver_shape, holder_shape;
  uint8_t epoch, key, offset, valid;
  uint32_t hits, misses;
} ORXPropertyCache;
enum { ORX_READONLY = 1, ORX_ACCESSOR = 2 };
void orx_world_init(ORXWorld *world);
ORStatus orx_world_new(ORXWorld *world, ORXHandle *out);
ORStatus orx_world_collect(ORXWorld *world, unsigned *dead);
ORStatus orx_define(ORXWorld *world, ORXHandle object, unsigned key,
                    unsigned attributes, int32_t value, ORXGetter getter,
                    void *context);
ORStatus orx_delete(ORXWorld *world, ORXHandle object, unsigned key);
ORStatus orx_prototype(ORXWorld *world, ORXHandle object, ORXHandle prototype);
ORStatus orx_get(ORXWorld *world, ORXHandle object, unsigned key,
                 ORXPropertyCache *cache, int32_t *out);
#endif

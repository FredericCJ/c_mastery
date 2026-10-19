#include "object_runtime_shapes.h"
/* Learner implementation: preserve the exact property/shape/cache contract. */
void orx_world_init(ORXWorld *world) { (void)world; }
ORStatus orx_world_new(ORXWorld *world, ORXHandle *out) {
  (void)world;
  (void)out;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_world_collect(ORXWorld *world, unsigned *dead) {
  (void)world;
  (void)dead;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_define(ORXWorld *world, ORXHandle object, unsigned key,
                    unsigned attributes, int32_t value, ORXGetter getter,
                    void *context) {
  (void)world;
  (void)object;
  (void)key;
  (void)attributes;
  (void)value;
  (void)getter;
  (void)context;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_delete(ORXWorld *world, ORXHandle object, unsigned key) {
  (void)world;
  (void)object;
  (void)key;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_prototype(ORXWorld *world, ORXHandle object, ORXHandle prototype) {
  (void)world;
  (void)object;
  (void)prototype;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_get(ORXWorld *world, ORXHandle object, unsigned key,
                 ORXPropertyCache *cache, int32_t *out) {
  (void)world;
  (void)object;
  (void)key;
  (void)cache;
  (void)out;
  return OR_UNIMPLEMENTED;
}

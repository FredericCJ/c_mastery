#include "object_runtime_shapes.h"

/* Learner implementation: preserve the exact property/shape/cache contract. */
void orx_world_init(struct orx_world *world)
{
    (void)world;
}

enum or_status orx_world_new(struct orx_world *world, struct orx_handle *out)
{
    (void)world;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_world_collect(struct orx_world *world, unsigned *dead)
{
    (void)world;
    (void)dead;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_define(struct orx_world *world, struct orx_handle object,
                          unsigned key, unsigned attributes, int32_t value,
                          orx_getter_fn getter, void *context)
{
    (void)world;
    (void)object;
    (void)key;
    (void)attributes;
    (void)value;
    (void)getter;
    (void)context;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_delete(struct orx_world *world, struct orx_handle object,
                          unsigned key)
{
    (void)world;
    (void)object;
    (void)key;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_prototype(struct orx_world *world, struct orx_handle object,
                             struct orx_handle prototype)
{
    (void)world;
    (void)object;
    (void)prototype;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_get(struct orx_world *world, struct orx_handle object,
                       unsigned key, struct orx_property_cache *cache,
                       int32_t *out)
{
    (void)world;
    (void)object;
    (void)key;
    (void)cache;
    (void)out;
    return OR_UNIMPLEMENTED;
}

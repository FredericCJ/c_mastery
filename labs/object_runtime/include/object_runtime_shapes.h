#ifndef CMASTERY_OBJECT_RUNTIME_SHAPES_H
#define CMASTERY_OBJECT_RUNTIME_SHAPES_H
#include "object_runtime_ext.h"
struct orx_world;
typedef enum or_status (*orx_getter_fn)(void *, struct orx_world *,
                                        struct orx_handle, int32_t *);

struct orx_shape_id
{
    uint8_t slot, generation;
};

struct orx_descriptor
{
    uint8_t live, generation, refs, count, key[4], attributes[4];
};

struct orx_field
{
    int32_t value;
    orx_getter_fn getter;
    void *context;
};

struct orx_object
{
    uint8_t generation;
    struct orx_shape_id shape;
    struct orx_field field[4];
};

struct orx_world
{
    struct orx_heap heap;
    struct orx_descriptor shape[16];
    struct orx_object object[8];
    uint8_t epoch, in_getter;
};

struct orx_property_cache
{
    const struct orx_world *world;
    struct orx_handle receiver, holder;
    struct orx_shape_id receiver_shape, holder_shape;
    uint8_t epoch, key, offset, valid;
    uint32_t hits, misses;
};

enum
{
    ORX_READONLY = 1,
    ORX_ACCESSOR = 2
};

void orx_world_init(struct orx_world *world);
enum or_status orx_world_new(struct orx_world *world, struct orx_handle *out);
enum or_status orx_world_collect(struct orx_world *world, unsigned *dead);
enum or_status orx_define(struct orx_world *world, struct orx_handle object,
                          unsigned key, unsigned attributes, int32_t value,
                          orx_getter_fn getter, void *context);
enum or_status orx_delete(struct orx_world *world, struct orx_handle object,
                          unsigned key);
enum or_status orx_prototype(struct orx_world *world, struct orx_handle object,
                             struct orx_handle prototype);
enum or_status orx_get(struct orx_world *world, struct orx_handle object,
                       unsigned key, struct orx_property_cache *cache,
                       int32_t *out);
#endif

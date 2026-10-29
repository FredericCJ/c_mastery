#ifndef CMASTERY_OWNERSHIP_MOVE_DEPTH_H
#define CMASTERY_OWNERSHIP_MOVE_DEPTH_H
#include "ownership.h"
struct omm_value;

struct omm_value
{
    int32_t small[4];
    int32_t *data;
    size_t length;
    struct v4_allocator allocator;
    const struct omm_value *self;
    unsigned live;
};
enum ow_status omm_init(struct omm_value *, struct v4_allocator,
                        const int32_t *, size_t);
enum ow_status omm_move(struct omm_value *, struct omm_value *);
enum ow_status omm_read(const struct omm_value *, size_t, int32_t *);
enum ow_status omm_consume(struct omm_value *);
/* The registry representation is incomplete; ticket values remain copyable. */
struct omm_registry;

struct omm_token
{
    struct omm_registry *owner;
    unsigned slot;
    uint32_t generation;
};

struct omm_memory
{
    void *context;
    void *(*take)(void *, size_t);
    void (*release)(void *, void *);
};
enum ow_status omm_registry_create(struct omm_memory, struct v4_allocator,
                                   uint32_t, struct omm_registry **);
enum ow_status omm_registry_destroy(struct omm_registry **);
enum ow_status omm_token_open(struct omm_registry *, const int32_t *, size_t,
                              struct omm_token *);
enum ow_status omm_token_move(struct omm_registry *, struct omm_token *,
                              struct omm_token *);
enum ow_status omm_token_read(struct omm_registry *, struct omm_token, size_t,
                              int32_t *);
enum ow_status omm_token_consume(struct omm_registry *, struct omm_token *);
#endif

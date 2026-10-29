#ifndef ADVANCED_SPSC_H
#define ADVANCED_SPSC_H
#include "cmx.h"

struct as_value
{
    uint32_t sequence, inverse, salt, triple;
};

typedef void (*as_hook_fn)(void *);

struct as_queue
{
    _Alignas(64) atomic_uint head;
    _Alignas(64) atomic_uint tail;
    unsigned capacity, producer_active, consumer_active;
    struct as_value data[16];
};
enum cmx_result as_init(struct as_queue *, unsigned);
enum cmx_result as_push(struct as_queue *, struct as_value, as_hook_fn, void *);
enum cmx_result as_pop(struct as_queue *, struct as_value *, as_hook_fn,
                       void *);

struct as_circular
{
    atomic_uint head, tail;
    unsigned storage;
    struct as_value data[16];
};
enum cmx_result as_circular_init(struct as_circular *, unsigned);
enum cmx_result as_circular_push(struct as_circular *, struct as_value);
enum cmx_result as_circular_pop(struct as_circular *, struct as_value *);
#endif

#include "object_runtime_ext.h"

/* Learner implementations: preserve the published contracts and replace each
 * stub. */
uint32_t orx_binary32(uint32_t bits)
{
    (void)bits;
    return 0;
}

enum or_status orx_value_decode(const unsigned char wire[6],
                                struct orx_value *out)
{
    (void)wire;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_value_encode(struct orx_value value, unsigned char wire[6])
{
    (void)value;
    (void)wire;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_double_bits(double value, uint64_t *out)
{
    (void)value;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_bits_double(uint64_t bits, double *out)
{
    (void)bits;
    (void)out;
    return OR_UNIMPLEMENTED;
}

void orx_heap_init(struct orx_heap *heap)
{
    (void)heap;
}

enum or_status orx_new(struct orx_heap *heap, const unsigned char *bytes,
                       size_t length, struct orx_handle *out)
{
    (void)heap;
    (void)bytes;
    (void)length;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_retain(struct orx_heap *heap, struct orx_handle object)
{
    (void)heap;
    (void)object;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_release(struct orx_heap *heap, struct orx_handle object)
{
    (void)heap;
    (void)object;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_read(const struct orx_heap *heap, struct orx_handle object,
                        unsigned char out[15], size_t *length)
{
    (void)heap;
    (void)object;
    (void)out;
    (void)length;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_edge(struct orx_heap *heap, struct orx_handle from,
                        unsigned field, struct orx_handle to)
{
    (void)heap;
    (void)from;
    (void)field;
    (void)to;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_collect(struct orx_heap *heap, unsigned *reclaimed)
{
    (void)heap;
    (void)reclaimed;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_pack(struct orx_heap *heap, unsigned *moved)
{
    (void)heap;
    (void)moved;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_import(struct orx_heap *heap, struct orx_value value,
                          const struct orx_grant *grant, struct orx_handle *out)
{
    (void)heap;
    (void)value;
    (void)grant;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_foreign_begin(struct orx_heap *heap,
                                 struct orx_handle object, uint64_t controller,
                                 struct orx_ticket *ticket,
                                 const unsigned char **bytes, size_t *length)
{
    (void)heap;
    (void)object;
    (void)controller;
    (void)ticket;
    (void)bytes;
    (void)length;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_foreign_cancel(struct orx_ticket ticket, uint64_t controller)
{
    (void)ticket;
    (void)controller;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_foreign_finish(struct orx_ticket ticket, uint64_t controller)
{
    (void)ticket;
    (void)controller;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_intern_init(struct orx_intern *table, struct orx_heap *heap,
                               struct v4_allocator allocator)
{
    (void)table;
    (void)heap;
    (void)allocator;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_intern_get(struct orx_intern *table,
                              const unsigned char *bytes, size_t length,
                              struct orx_handle *out)
{
    (void)table;
    (void)bytes;
    (void)length;
    (void)out;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_intern_reserve(struct orx_intern *table, unsigned capacity)
{
    (void)table;
    (void)capacity;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_intern_sweep(struct orx_intern *table, unsigned *removed)
{
    (void)table;
    (void)removed;
    return OR_UNIMPLEMENTED;
}

enum or_status orx_intern_close(struct orx_intern *table)
{
    (void)table;
    return OR_UNIMPLEMENTED;
}

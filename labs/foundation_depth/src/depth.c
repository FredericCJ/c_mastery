#include "depth.h"

/* These compiling stubs deliberately fail every exercise semantic gate.
 * Implement only the selected exercise, preserving the shared interface.
 * Contracts are in depth.h and CONTRACTS.md; no supplied stub is a worked
 * solution. Each stub returns its "not done" sentinel and touches nothing. */
enum fd_class fd_verdict(enum fd_case question)
{
    (void)question;
    return FD_UNKNOWN;
}

enum fd_status fd_repair_step(int *value, int *previous)
{
    (void)value;
    (void)previous;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_extent(size_t offset, size_t count, size_t width,
                         size_t limit, size_t *out)
{
    (void)offset;
    (void)count;
    (void)width;
    (void)limit;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_from_signed(intmax_t value, size_t *out)
{
    (void)value;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_take(struct fd_cursor *cursor, size_t count,
                       const unsigned char **out)
{
    (void)cursor;
    (void)count;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_subview(const struct fd_cursor *parent, size_t offset,
                          size_t count, struct fd_cursor *out)
{
    (void)parent;
    (void)offset;
    (void)count;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_encode(struct fd_record record, unsigned char *out,
                         size_t capacity)
{
    (void)record;
    (void)out;
    (void)capacity;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_decode(const unsigned char *wire, size_t length,
                         struct fd_record *out)
{
    (void)wire;
    (void)length;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_slot_acquire(struct fd_slot *slot,
                               struct fd_allocator allocator)
{
    (void)slot;
    (void)allocator;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_slot_initialize(struct fd_slot *slot,
                                  fd_initialize_fn initialize,
                                  fd_destroy_fn destroy, void *context)
{
    (void)slot;
    (void)initialize;
    (void)destroy;
    (void)context;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_slot_publish(struct fd_slot *slot,
                               const struct fd_value **out)
{
    (void)slot;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

void fd_slot_reset(struct fd_slot *slot)
{
    (void)slot;
}

void fd_transform(unsigned char *restrict destination,
                  const unsigned char *restrict source, size_t count,
                  unsigned char mask)
{
    (void)destination;
    (void)source;
    (void)count;
    (void)mask;
}

enum fd_status fd_move(unsigned char *base, size_t length, size_t destination,
                       size_t source, size_t count)
{
    (void)base;
    (void)length;
    (void)destination;
    (void)source;
    (void)count;
    return FD_NOT_IMPLEMENTED;
}

void fd_pool_init(struct fd_pool *pool)
{
    (void)pool;
}

enum fd_status fd_pool_acquire(struct fd_pool *pool, struct fd_value value,
                               struct fd_handle *out)
{
    (void)pool;
    (void)value;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_pool_get(const struct fd_pool *pool, struct fd_handle handle,
                           struct fd_value *out)
{
    (void)pool;
    (void)handle;
    (void)out;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_pool_release(struct fd_pool *pool, struct fd_handle handle)
{
    (void)pool;
    (void)handle;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_apply(const unsigned char *wire, size_t length,
                        int authorized, int32_t *synthetic_counter)
{
    (void)wire;
    (void)length;
    (void)authorized;
    (void)synthetic_counter;
    return FD_NOT_IMPLEMENTED;
}

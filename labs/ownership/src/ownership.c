#include "ownership.h"

/* Compiling learner scaffold; no mechanism solution is included. */
enum ow_status ow_construct(struct ow_owner *out, struct v4_allocator a,
                            const size_t lengths[3])
{
    (void)out;
    (void)a;
    (void)lengths;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_destroy(struct ow_owner *o)
{
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_defer(struct ow_scope *s, ow_cleanup_fn fn, void *ctx,
                        enum ow_when when)
{
    (void)s;
    (void)fn;
    (void)ctx;
    (void)when;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_finish(struct ow_scope *s, int success)
{
    (void)s;
    (void)success;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_publish(struct ow_owner *published, struct ow_owner *staged)
{
    (void)published;
    (void)staged;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_move(struct ow_owner *dst, struct ow_owner *src)
{
    (void)dst;
    (void)src;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_consume(struct ow_owner *o)
{
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_borrow(struct ow_borrows *b, size_t off, size_t len,
                         int writer, struct ow_token *out)
{
    (void)b;
    (void)off;
    (void)len;
    (void)writer;
    (void)out;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_release(struct ow_borrows *b, struct ow_token t)
{
    (void)b;
    (void)t;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_split(struct ow_borrows *b, struct ow_token parent,
                        size_t left, struct ow_token children[2])
{
    (void)b;
    (void)parent;
    (void)left;
    (void)children;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_cell_read(const struct ow_cell *c, struct ow_token t,
                            int32_t *out)
{
    (void)c;
    (void)t;
    (void)out;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_cell_write(struct ow_cell *c, struct ow_token t,
                             int32_t value)
{
    (void)c;
    (void)t;
    (void)value;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_cell_replace(struct ow_cell *c, int32_t value, int32_t *old)
{
    (void)c;
    (void)value;
    (void)old;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_shared_retain(struct ow_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_shared_downgrade(struct ow_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_shared_upgrade(struct ow_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_shared_weak_drop(struct ow_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_shared_drop(struct ow_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_atomic_retain(struct ow_atomic_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_atomic_upgrade(struct ow_atomic_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_atomic_downgrade(struct ow_atomic_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_atomic_weak_drop(struct ow_atomic_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_atomic_drop(struct ow_atomic_shared *s)
{
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_cow_clone(struct ow_cow *c)
{
    (void)c;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_cow_write(struct ow_cow **owner, struct ow_cow *fresh,
                            size_t index, int32_t value)
{
    (void)owner;
    (void)fresh;
    (void)index;
    (void)value;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_cow_drop(struct ow_cow *c)
{
    (void)c;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_pin_init(struct ow_pin *p, const void *address,
                           uint64_t thread)
{
    (void)p;
    (void)address;
    (void)thread;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_pin_check(const struct ow_pin *p, const void *address,
                            uint64_t thread)
{
    (void)p;
    (void)address;
    (void)thread;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_pin_submit(struct ow_pin *p, const void *address,
                             uint64_t thread)
{
    (void)p;
    (void)address;
    (void)thread;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_pin_cancel(struct ow_pin *p, uint64_t thread)
{
    (void)p;
    (void)thread;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_pin_complete(struct ow_pin *p, uint64_t thread)
{
    (void)p;
    (void)thread;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ow_pin_rebind(struct ow_pin *p, const void *address,
                             uint64_t thread)
{
    (void)p;
    (void)address;
    (void)thread;
    return OW_NOT_IMPLEMENTED;
}

static int32_t *unsolved_take(void *context, size_t count)
{
    (void)context;
    (void)count;
    return NULL;
}

static void unsolved_release(void *context, int32_t *pointer)
{
    (void)context;
    (void)pointer;
}

struct v4_allocator ow_allocator(struct ow_allocator *a)
{
    return (struct v4_allocator){a, unsolved_take, unsolved_release};
}

enum ow_status ow_flawed_write(struct ow_tenant_arena *a, unsigned tenant,
                               unsigned offset, unsigned count, int32_t value)
{
    if (!a || tenant > 1 || count > 8 || offset + count > 8)
    {
        return OW_INVALID;
    }
    /* Deliberate defined wrap and masked access: remains within actual array. */
    for (unsigned i = 0; i < count; i++)
    {
        a->data[(tenant * 8u + offset + i) & 15u] = value;
    }
    return OW_OK;
}

enum ow_status ow_checked_write(struct ow_tenant_arena *a, unsigned tenant,
                                unsigned offset, unsigned count, int32_t value)
{
    (void)a;
    (void)tenant;
    (void)offset;
    (void)count;
    (void)value;
    return OW_NOT_IMPLEMENTED;
}

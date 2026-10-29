#include "depth.h"

/* These compiling stubs deliberately fail every exercise semantic gate.
 * Implement only the selected exercise, preserving the shared interface.
 * Full contracts are in CONTRACTS.md; no supplied stub is a worked solution. */
enum fd_class fd_verdict(enum fd_case q)
{
    (void)q;
    return FD_UNKNOWN;
}

enum fd_status fd_repair_step(int *v, int *p)
{
    (void)v;
    (void)p;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_extent(size_t o, size_t c, size_t w, size_t l, size_t *r)
{
    (void)o;
    (void)c;
    (void)w;
    (void)l;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_from_signed(intmax_t v, size_t *r)
{
    (void)v;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_take(struct fd_cursor *c, size_t n, const unsigned char **p)
{
    (void)c;
    (void)n;
    (void)p;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_subview(const struct fd_cursor *c, size_t o, size_t n,
                          struct fd_cursor *r)
{
    (void)c;
    (void)o;
    (void)n;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_encode(struct fd_record r, unsigned char *p, size_t n)
{
    (void)r;
    (void)p;
    (void)n;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_decode(const unsigned char *p, size_t n, struct fd_record *r)
{
    (void)p;
    (void)n;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_slot_acquire(struct fd_slot *s, struct fd_allocator a)
{
    (void)s;
    (void)a;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_slot_initialize(struct fd_slot *s, fd_initialize_fn i,
                                  fd_destroy_fn d, void *c)
{
    (void)s;
    (void)i;
    (void)d;
    (void)c;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_slot_publish(struct fd_slot *s, const struct fd_value **r)
{
    (void)s;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

void fd_slot_reset(struct fd_slot *s)
{
    (void)s;
}

void fd_transform(unsigned char *restrict d, const unsigned char *restrict s,
                  size_t n, unsigned char m)
{
    (void)d;
    (void)s;
    (void)n;
    (void)m;
}

enum fd_status fd_move(unsigned char *b, size_t l, size_t d, size_t s, size_t n)
{
    (void)b;
    (void)l;
    (void)d;
    (void)s;
    (void)n;
    return FD_NOT_IMPLEMENTED;
}

void fd_pool_init(struct fd_pool *p)
{
    (void)p;
}

enum fd_status fd_pool_acquire(struct fd_pool *p, struct fd_value v,
                               struct fd_handle *r)
{
    (void)p;
    (void)v;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_pool_get(const struct fd_pool *p, struct fd_handle h,
                           struct fd_value *r)
{
    (void)p;
    (void)h;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_pool_release(struct fd_pool *p, struct fd_handle h)
{
    (void)p;
    (void)h;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_apply(const unsigned char *p, size_t n, int a, int32_t *r)
{
    (void)p;
    (void)n;
    (void)a;
    (void)r;
    return FD_NOT_IMPLEMENTED;
}

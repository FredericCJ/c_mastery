#include "transfer.h"

/* Intentionally incomplete: release only with the selected transfer brief. */
struct fd_step fd_step_value(int v)
{
    (void)v;
    return (struct fd_step){FD_NOT_IMPLEMENTED, 0, 0};
}

enum fd_status fd_layout(size_t h, intmax_t a, size_t aw, intmax_t b, size_t bw,
                         size_t l, struct fd_layout *o)
{
    (void)h;
    (void)a;
    (void)aw;
    (void)b;
    (void)bw;
    (void)l;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_copy_segments(struct fd_segmented *c, size_t n,
                                unsigned char *d, size_t cap)
{
    (void)c;
    (void)n;
    (void)d;
    (void)cap;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_owner_subview(const struct fd_owner_view *s, size_t o,
                                size_t n, struct fd_owner_view *v)
{
    (void)s;
    (void)o;
    (void)n;
    (void)v;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_read_tagged(struct fd_cursor *s, uint16_t *o)
{
    (void)s;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_least_encode(struct fd_least_record r, unsigned char *o,
                               size_t n)
{
    (void)r;
    (void)o;
    (void)n;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_least_decode(const unsigned char *w, size_t n,
                               struct fd_least_record *o)
{
    (void)w;
    (void)n;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_encode_v2(uintmax_t c, intmax_t v, unsigned char *o, size_t n)
{
    (void)c;
    (void)v;
    (void)o;
    (void)n;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_decode_v2(const unsigned char *w, size_t n, uintmax_t *c,
                            intmax_t *v)
{
    (void)w;
    (void)n;
    (void)c;
    (void)v;
    return FD_NOT_IMPLEMENTED;
}

void *fd_typed_allocate(void *c, size_t n, size_t a)
{
    (void)c;
    (void)n;
    (void)a;
    return 0;
}

void fd_typed_release(void *c, void *v)
{
    (void)c;
    (void)v;
}

enum fd_status fd_slot_transfer(struct fd_slot *d, struct fd_slot *s)
{
    (void)d;
    (void)s;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_stride_move(unsigned char *b, size_t p, size_t l,
                              size_t stride, size_t d, size_t s, size_t n)
{
    (void)b;
    (void)p;
    (void)l;
    (void)stride;
    (void)d;
    (void)s;
    (void)n;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_compact(struct fd_stream_view *s)
{
    (void)s;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_inplace_xor(unsigned char *b, size_t n, unsigned char m)
{
    (void)b;
    (void)n;
    (void)m;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_pool_borrow(const struct fd_pool *p, struct fd_handle h,
                              const struct fd_value **o)
{
    (void)p;
    (void)h;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

void fd_registry_init(struct fd_registry *r)
{
    (void)r;
}

enum fd_status fd_registry_bind(struct fd_registry *r, size_t s,
                                struct fd_pool *p)
{
    (void)r;
    (void)s;
    (void)p;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_registry_unbind(struct fd_registry *r, size_t s)
{
    (void)r;
    (void)s;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_registry_relocate(struct fd_registry *r, size_t s,
                                    struct fd_pool *p)
{
    (void)r;
    (void)s;
    (void)p;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_registry_acquire(struct fd_registry *r, size_t s,
                                   struct fd_value v,
                                   struct fd_registry_handle *o)
{
    (void)r;
    (void)s;
    (void)v;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_registry_get(const struct fd_registry *r,
                               struct fd_registry_handle h, struct fd_value *o)
{
    (void)r;
    (void)h;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

void fd_frame_cancel(struct fd_frame *f)
{
    (void)f;
}

enum fd_status fd_effect(enum fd_effect e, const unsigned char *w, size_t n,
                         int a, int32_t *c, int32_t *o)
{
    (void)e;
    (void)w;
    (void)n;
    (void)a;
    (void)c;
    (void)o;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status fd_frame_feed(struct fd_frame *f, const unsigned char *b,
                             size_t n, int a, int32_t *c, int *done)
{
    (void)f;
    (void)b;
    (void)n;
    (void)a;
    (void)c;
    (void)done;
    return FD_NOT_IMPLEMENTED;
}

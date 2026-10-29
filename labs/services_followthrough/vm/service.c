#include "service.h"

int sfv_open(struct sfv_service *s, unsigned c, uint64_t n)
{
    (void)s;
    (void)c;
    (void)n;
    return SFV_UNIMPLEMENTED;
}

int sfv_submit(struct sfv_service *s, unsigned c, const struct sfv_request *r)
{
    (void)s;
    (void)c;
    (void)r;
    return SFV_UNIMPLEMENTED;
}

int sfv_step(struct sfv_service *s, unsigned c)
{
    (void)s;
    (void)c;
    return SFV_UNIMPLEMENTED;
}

int sfv_cancel(struct sfv_service *s, unsigned c, uint64_t n, uint64_t id)
{
    (void)s;
    (void)c;
    (void)n;
    (void)id;
    return SFV_UNIMPLEMENTED;
}

int sfv_reply(struct sfv_service *s, unsigned c, uint64_t n, uint64_t id,
              void *p, size_t z, size_t *w, unsigned *r)
{
    (void)s;
    (void)c;
    (void)n;
    (void)id;
    (void)p;
    (void)z;
    (void)w;
    (void)r;
    return SFV_UNIMPLEMENTED;
}

int sfv_handle(struct sfv_service *s, unsigned c, struct sfv_handle *h)
{
    (void)s;
    (void)c;
    (void)h;
    return SFV_UNIMPLEMENTED;
}

int sfv_field(struct sfv_service *s, unsigned c, const struct sfv_handle *h,
              int b, struct cvm_value *v)
{
    (void)s;
    (void)c;
    (void)h;
    (void)b;
    (void)v;
    return SFV_UNIMPLEMENTED;
}

#include "variants.h"

int ev_order(const unsigned char a[8], uint32_t *b)
{
    (void)a;
    (void)b;
    return ED_UNIMPLEMENTED;
}

int ev_prepare(struct ev_chain *a, const unsigned char *b, size_t c, unsigned d,
               uint32_t e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return ED_UNIMPLEMENTED;
}

int ev_own(struct ev_chain *a, unsigned b)
{
    (void)a;
    (void)b;
    return ED_UNIMPLEMENTED;
}

int ev_complete(struct ev_chain *a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return ED_UNIMPLEMENTED;
}

int ev_abort(struct ev_chain *a, unsigned b)
{
    (void)a;
    (void)b;
    return ED_UNIMPLEMENTED;
}

int ev_span(const unsigned char *a, size_t b, unsigned c, size_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return ED_UNIMPLEMENTED;
}

int ev_store(struct ev_visibility *a, unsigned b, unsigned char c)
{
    (void)a;
    (void)b;
    (void)c;
    return ED_UNIMPLEMENTED;
}

int ev_clean(struct ev_visibility *a, unsigned b)
{
    (void)a;
    (void)b;
    return ED_UNIMPLEMENTED;
}

int ev_publish(struct ev_visibility *a, unsigned b)
{
    (void)a;
    (void)b;
    return ED_UNIMPLEMENTED;
}

int ev_device_store(struct ev_visibility *a, unsigned b, unsigned char c)
{
    (void)a;
    (void)b;
    (void)c;
    return ED_UNIMPLEMENTED;
}

int ev_return(struct ev_visibility *a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return ED_UNIMPLEMENTED;
}

int ev_invalidate(struct ev_visibility *a, unsigned b)
{
    (void)a;
    (void)b;
    return ED_UNIMPLEMENTED;
}

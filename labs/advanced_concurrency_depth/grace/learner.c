#include "grace.h"

void ag_init(struct ag_domain *d, unsigned n)
{
    (void)d;
    (void)n;
}

int ag_register(struct ag_domain *d, struct ag_lease *l)
{
    (void)d;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int ag_enter(struct ag_domain *d, struct ag_lease l)
{
    (void)d;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int ag_leave(struct ag_domain *d, struct ag_lease l)
{
    (void)d;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int ag_depart(struct ag_domain *d, struct ag_lease l)
{
    (void)d;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int ag_advance(struct ag_domain *d)
{
    (void)d;
    return AG_UNIMPLEMENTED;
}

int ag_reclaimable(const struct ag_domain *d, unsigned e)
{
    (void)d;
    (void)e;
    return 0;
}

void ag_store_init(struct ag_store *s, unsigned n, unsigned char v)
{
    (void)s;
    (void)n;
    (void)v;
}

int ag_writer_acquire(struct ag_store *s, unsigned w)
{
    (void)s;
    (void)w;
    return AG_UNIMPLEMENTED;
}

int ag_writer_release(struct ag_store *s, unsigned w)
{
    (void)s;
    (void)w;
    return AG_UNIMPLEMENTED;
}

int ag_publish(struct ag_store *s, unsigned w, unsigned char v)
{
    (void)s;
    (void)w;
    (void)v;
    return AG_UNIMPLEMENTED;
}

int ag_read(struct ag_store *s, struct ag_lease l, struct ag_borrow *b)
{
    (void)s;
    (void)l;
    (void)b;
    return AG_UNIMPLEMENTED;
}

int ag_copy(struct ag_store *s, struct ag_borrow b, unsigned char v[32])
{
    (void)s;
    (void)b;
    (void)v;
    return AG_UNIMPLEMENTED;
}

int ag_read_end(struct ag_store *s, struct ag_borrow b)
{
    (void)s;
    (void)b;
    return AG_UNIMPLEMENTED;
}

unsigned ag_collect(struct ag_store *s)
{
    (void)s;
    return 0;
}

enum api_status ag_capture(struct ag_store *s, struct ag_borrow b,
                           ag_callback_fn c, void *ctx, struct api_handle *h)
{
    (void)s;
    (void)b;
    (void)c;
    (void)ctx;
    (void)h;
    return API_UNIMPLEMENTED;
}

enum api_status ag_dispatch(struct ag_store *s)
{
    (void)s;
    return API_UNIMPLEMENTED;
}

enum api_status ag_remove(struct ag_store *s, struct api_handle h)
{
    (void)s;
    (void)h;
    return API_UNIMPLEMENTED;
}

enum api_status ag_destroy_callbacks(struct ag_store *s)
{
    (void)s;
    return API_UNIMPLEMENTED;
}

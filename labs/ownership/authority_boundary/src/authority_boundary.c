#include "authority_boundary.h"

/* Compiling exercise scaffold: implement the checked boundary and trusted core. */
enum oaf_status oaf_init(struct oaf_root *r, struct ow_tenant_arena *a,
                         unsigned t, unsigned p)
{
    (void)r;
    (void)a;
    (void)t;
    (void)p;
    return OAF_INVALID;
}

enum oaf_status oaf_view(struct oaf_root *r, struct oaf_view *v)
{
    (void)r;
    (void)v;
    return OAF_INVALID;
}

enum oaf_status oaf_subview(const struct oaf_view *v, uint32_t o, uint32_t n,
                            unsigned p, struct oaf_view *out)
{
    (void)v;
    (void)o;
    (void)n;
    (void)p;
    (void)out;
    return OAF_INVALID;
}

enum oaf_status oaf_revoke(struct oaf_root *r)
{
    (void)r;
    return OAF_INVALID;
}

enum oaf_status oaf_encode(struct oaf_request r, unsigned char b[OAF_WIRE_SIZE])
{
    (void)r;
    (void)b;
    return OAF_INVALID;
}

enum oaf_status oaf_decode(const unsigned char *b, size_t n,
                           struct oaf_request *r)
{
    (void)b;
    (void)n;
    (void)r;
    return OAF_INVALID;
}

enum oaf_status oaf_write(const struct oaf_view *v, struct oaf_request r,
                          int32_t x)
{
    (void)v;
    (void)r;
    (void)x;
    return OAF_INVALID;
}

enum oaf_status oaf_read(const struct oaf_view *v, struct oaf_request r,
                         int32_t out[8])
{
    (void)v;
    (void)r;
    (void)out;
    return OAF_INVALID;
}

enum oaf_status oaf_wire_write(const struct oaf_view *v, const unsigned char *b,
                               size_t n, int32_t x)
{
    (void)v;
    (void)b;
    (void)n;
    (void)x;
    return OAF_INVALID;
}

enum oaf_status oaf_wire_read(const struct oaf_view *v, const unsigned char *b,
                              size_t n, int32_t out[8])
{
    (void)v;
    (void)b;
    (void)n;
    (void)out;
    return OAF_INVALID;
}

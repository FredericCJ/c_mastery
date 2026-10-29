#include "authority_boundary.h"
#include <limits.h>
#ifndef OAF_MUTATION
#define OAF_MUTATION 0
#endif
_Static_assert(CHAR_BIT == 8, "wire octets");

/* This translation-unit-private value is constructed only by authorize(). */
struct ow_validated
{
    struct ow_tenant_arena *arena;
    unsigned start, count;
};

static int root_valid(const struct oaf_root *r)
{
    return r && r->arena && r->tenant < 2 && !(r->rights & ~3u) && r->epoch &&
           r->retired <= 1;
}

static enum oaf_status view_valid(const struct oaf_view *v)
{
    if (!v || !root_valid(v->root) || v->offset > 8 ||
        v->count > 8 - v->offset || (v->rights & ~v->root->rights))
    {
        return OAF_INVALID;
    }
    if (OAF_MUTATION != 8 && (v->root->retired || v->epoch != v->root->epoch))
    {
        return OAF_STALE;
    }
    return OAF_OK;
}

enum oaf_status oaf_init(struct oaf_root *r, struct ow_tenant_arena *a,
                         unsigned tenant, unsigned rights)
{
    if (!r || !a || tenant > 1 || (rights & ~3u))
    {
        return OAF_INVALID;
    }
    *r = (struct oaf_root){a, 1, tenant, rights, 0};
    return OAF_OK;
}

enum oaf_status oaf_view(struct oaf_root *r, struct oaf_view *out)
{
    if (!root_valid(r) || !out)
    {
        return OAF_INVALID;
    }
    if (r->retired)
    {
        return OAF_STALE;
    }
    *out = (struct oaf_view){r, r->epoch, 0, 8, r->rights};
    return OAF_OK;
}

enum oaf_status oaf_subview(const struct oaf_view *v, uint32_t offset,
                            uint32_t count, unsigned rights,
                            struct oaf_view *out)
{
    enum oaf_status s = view_valid(v);
    if (s != OAF_OK)
    {
        return s;
    }
    if (!out || offset > v->count || count > v->count - offset ||
        (rights & ~3u))
    {
        return OAF_INVALID;
    }
    if (OAF_MUTATION != 3 && (rights & ~v->rights))
    {
        return OAF_DENIED;
    }
    struct oaf_view next = {v->root, v->epoch, v->offset + offset, count,
                            rights};
    *out = next;
    return OAF_OK;
}

enum oaf_status oaf_revoke(struct oaf_root *r)
{
    if (!root_valid(r))
    {
        return OAF_INVALID;
    }
    if (r->retired)
    {
        return OAF_FULL;
    }
    if (r->epoch == UINT32_MAX)
    {
        r->retired = 1;
    }
    else
    {
        ++r->epoch;
    }
    return OAF_OK;
}

static void put32(unsigned char *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = (unsigned char)(x >> (24 - 8 * i));
    }
}

static uint32_t get32(const unsigned char *p)
{
    uint32_t x = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        x = (x << 8) | p[i];
    }
    return x;
}

enum oaf_status oaf_encode(struct oaf_request r,
                           unsigned char out[OAF_WIRE_SIZE])
{
    if (!out || r.tenant > 1 || (r.rights & ~3u))
    {
        return OAF_INVALID;
    }
    unsigned char b[16] = {1, (unsigned char)r.tenant, (unsigned char)r.rights,
                           0};
    put32(b + 4, r.epoch);
    put32(b + 8, r.offset);
    put32(b + 12, r.count);
    for (unsigned i = 0; i < 16; i++)
    {
        out[i] = b[i];
    }
    return OAF_OK;
}

enum oaf_status oaf_decode(const unsigned char *b, size_t n,
                           struct oaf_request *out)
{
    if (!b || n != 16 || !out)
    {
        return OAF_INVALID;
    }
    struct oaf_request r = {get32(b + 4), get32(b + 8), get32(b + 12), b[1],
                            b[2]};
    if (OAF_MUTATION == 6)
    {
        *out = r;
    }
    if (b[0] != 1 || b[1] > 1 || (b[2] & ~3u) || b[3])
    {
        return OAF_INVALID;
    }
    *out = r;
    return OAF_OK;
}

static enum oaf_status authorize(const struct oaf_view *v, struct oaf_request r,
                                 unsigned operation, struct ow_validated *out)
{
    enum oaf_status s = view_valid(v);
    if (s != OAF_OK)
    {
        return s;
    }
    if (r.tenant > 1 || (r.rights & ~3u) || r.offset > v->count ||
        r.count > v->count - r.offset)
    {
        return OAF_INVALID;
    }
    if (OAF_MUTATION != 4 && r.epoch != v->epoch)
    {
        return OAF_STALE;
    }
    if ((OAF_MUTATION != 5 && r.tenant != v->root->tenant) ||
        (r.rights & ~v->rights) ||
        (OAF_MUTATION != 7 && !(r.rights & operation)))
    {
        return OAF_DENIED;
    }
    *out = (struct ow_validated){
        v->root->arena, v->root->tenant * 8u + v->offset + r.offset, r.count};
    return OAF_OK;
}

static void trusted_write(struct ow_validated d, int32_t value)
{
    for (unsigned i = 0; i < d.count; i++)
    {
        d.arena->data[d.start + i] = value;
    }
}

enum oaf_status oaf_write(const struct oaf_view *v, struct oaf_request r,
                          int32_t value)
{
    struct ow_validated d;
    enum oaf_status s = authorize(v, r, OAF_WRITE, &d);
    if (s != OAF_OK)
    {
        return s;
    }
    trusted_write(d, value);
    return OAF_OK;
}

enum oaf_status oaf_read(const struct oaf_view *v, struct oaf_request r,
                         int32_t out[8])
{
    if (!out)
    {
        return OAF_INVALID;
    }
    struct ow_validated d;
    enum oaf_status s = authorize(v, r, OAF_READ, &d);
    if (s != OAF_OK)
    {
        return s;
    }
    int32_t stage[8];
    for (unsigned i = 0; i < d.count; i++)
    {
        stage[i] = d.arena->data[d.start + i];
    }
    for (unsigned i = 0; i < d.count; i++)
    {
        out[i] = stage[i];
    }
    return OAF_OK;
}

enum oaf_status oaf_wire_write(const struct oaf_view *v, const unsigned char *b,
                               size_t n, int32_t value)
{
    struct oaf_request r;
    enum oaf_status s = oaf_decode(b, n, &r);
    return s == OAF_OK ? oaf_write(v, r, value) : s;
}

enum oaf_status oaf_wire_read(const struct oaf_view *v, const unsigned char *b,
                              size_t n, int32_t out[8])
{
    struct oaf_request r;
    enum oaf_status s = oaf_decode(b, n, &r);
    return s == OAF_OK ? oaf_read(v, r, out) : s;
}

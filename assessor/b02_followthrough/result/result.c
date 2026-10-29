#include "result.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
int bf_number(int x, unsigned tag, struct b2_result *out)
{
    if (!out || tag == 1 || tag > 2)
    {
        return B2_INVALID;
    }
    if (x < -9999 || x > 9999)
    {
        return B2_RANGE;
    }
    struct b2_result r;
    r.tag = BF_MUTANT == 2 ? 1 : tag;
    if (tag == 0)
    {
        r.value.number = x;
    }
    else
    {
        r.value.error = x;
    }
    *out = r;
    return B2_OK;
}

int bf_text(const unsigned char *p, size_t n, struct b2_result *out)
{
    if (!out || (!p && n))
    {
        return B2_INVALID;
    }
    struct b2_result r;
    r.tag = 1;
    r.value.text.data = p;
    r.value.text.length = n;
    *out = r;
    return B2_OK;
}

void bf_value_init(struct bf_value *v)
{
    v->tag = BF_EMPTY;
}

int bf_clone(struct bf_value *v, const unsigned char *p, size_t n,
             bf_alloc_fn alloc, bf_free_fn release, void *ctx)
{
    if (!v || v->tag != BF_EMPTY || (!p && n) || !alloc || !release)
    {
        return B2_INVALID;
    }
    if (n > 256)
    {
        return B2_RANGE;
    }
    struct bf_bytes b = {0, 0, 0, 0};
    if (n)
    {
        b.data = alloc(ctx, n);
        if (!b.data)
        {
            return B2_FAILED;
        }
        b.length = n;
        b.release = release;
        b.context = ctx;
        for (size_t i = 0; i < n; ++i)
        {
            b.data[i] = p[i];
        }
    }
    v->value.owned = b;
    v->tag = BF_OWNED;
    return B2_OK;
}

int bf_move(struct bf_value *dest, struct bf_value *src)
{
    if (!dest || !src || dest == src || dest->tag != BF_EMPTY ||
        src->tag != BF_OWNED)
    {
        return B2_INVALID;
    }
    *dest = *src;
    if (BF_MUTANT != 3)
    {
        src->tag = BF_EMPTY;
    }
    return B2_OK;
}

int bf_take(struct bf_value *v, struct bf_bytes *out)
{
    if (!v || !out || out->data || out->length || out->release ||
        out->context || v->tag != BF_OWNED)
    {
        return B2_INVALID;
    }
    *out = v->value.owned;
    if (BF_MUTANT != 4)
    {
        v->tag = BF_EMPTY;
    }
    return B2_OK;
}

void bf_bytes_drop(struct bf_bytes *b)
{
    struct bf_bytes old = *b;
    *b = (struct bf_bytes){0, 0, 0, 0};
    if (old.data)
    {
        old.release(old.context, old.data);
    }
}

void bf_reset(struct bf_value *v)
{
    if (v->tag == BF_OWNED)
    {
        struct bf_bytes b = v->value.owned;
        v->tag = BF_EMPTY;
        bf_bytes_drop(&b);
    }
    else
    {
        v->tag = BF_EMPTY;
    }
}

int bf_value_render(const struct bf_value *v, unsigned char *out, size_t n,
                    size_t *w)
{
    if (!v)
    {
        return B2_INVALID;
    }
    struct b2_result r;
    switch (v->tag)
    {
        case BF_INTEGER:
            r.tag = 0;
            r.value.number = v->value.number;
            break;
        case BF_ERROR:
            r.tag = 2;
            r.value.error = v->value.error;
            break;
        case BF_BORROWED:
            r.tag = 1;
            r.value.text.data = v->value.borrowed.data;
            r.value.text.length = v->value.borrowed.length;
            break;
        case BF_OWNED:
            r.tag = 1;
            r.value.text.data = v->value.owned.data;
            r.value.text.length = v->value.owned.length;
            break;
        default:
            return B2_INVALID;
    }
    return b2_render(&r, out, n, w);
}

int bf_encode(const struct bf_wire *r, unsigned char *out, size_t c, size_t *w)
{
    if (!r || !w || (!out && c) || (r->version != 1 && r->version != 2) ||
        r->tag > 3 || (r->version == 1 && r->tag == 3))
    {
        return B2_INVALID;
    }
    size_t n = r->tag == 1 ? r->length : r->tag == 3 ? 1 : 4;
    if (n > 256 || (r->tag == 3 && r->boolean > 1))
    {
        return B2_INVALID;
    }
    if ((r->tag == 0 || r->tag == 2) && (r->number < -9999 || r->number > 9999))
    {
        return B2_RANGE;
    }
    if (c < n + 4)
    {
        return B2_FULL;
    }
    out[0] = (unsigned char)r->version;
    out[1] = (unsigned char)r->tag;
    out[2] = (unsigned char)(n / 256);
    out[3] = (unsigned char)(n % 256);
    if (r->tag == 1)
    {
        for (size_t i = 0; i < n; ++i)
        {
            out[4 + i] = r->bytes[i];
        }
    }
    else if (r->tag == 3)
    {
        out[4] = (unsigned char)r->boolean;
    }
    else
    {
        uint32_t raw = (uint32_t)(int32_t)r->number;
        for (size_t i = 0; i < 4; ++i)
        {
            out[4 + i] = (unsigned char)(raw >> ((3 - i) * 8));
        }
    }
    *w = n + 4;
    return B2_OK;
}

int bf_decode(const unsigned char *p, size_t n, struct bf_wire *out)
{
    if (!p || !out || n < 4 || (p[0] != 1 && p[0] != 2))
    {
        return B2_INVALID;
    }
    unsigned tag = p[1];
    if (tag > 3 || (tag == 3 && p[0] == 1))
    {
        return B2_INVALID;
    }
    size_t length = (size_t)p[2] * 256u + p[3];
    if (length > 256 || n != length + 4)
    {
        return B2_INVALID;
    }
    struct bf_wire r = {0};
    r.version = p[0];
    r.tag = tag;
    if (tag == 1)
    {
        r.length = length;
        for (size_t i = 0; i < length; ++i)
        {
            r.bytes[i] = p[4 + i];
        }
    }
    else if (tag == 3)
    {
        if (length != 1 || (p[4] > 1 && BF_MUTANT != 5))
        {
            return B2_INVALID;
        }
        r.boolean = p[4];
    }
    else
    {
        if (length != 4)
        {
            return B2_INVALID;
        }
        uint32_t raw = (uint32_t)p[4] * UINT32_C(16777216) +
                       (uint32_t)p[5] * 65536u + (uint32_t)p[6] * 256u + p[7];
        int32_t value =
            raw <= INT32_MAX ? (int32_t)raw : -1 - (int32_t)(UINT32_MAX - raw);
        if (value < -9999 || value > 9999)
        {
            return B2_RANGE;
        }
        r.number = BF_MUTANT == 6 ? -value : value;
    }
    *out = r;
    return B2_OK;
}

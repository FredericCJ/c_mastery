#include "move.h"
#ifndef OMM_MUTATION
#define OMM_MUTATION 0
#endif
static int valid(const struct omm_value *v)
{
    return v && v->live == 1 && v->self == v && v->length > 0 &&
           v->length <= 8 && v->allocator.take && v->allocator.release &&
           v->data && (v->length > 4 || v->data == v->small);
}

enum ow_status omm_init(struct omm_value *out, struct v4_allocator a,
                        const int32_t *p, size_t n)
{
    if (!out || out->live || !p || !n || n > 8 || !a.take || !a.release)
        return OW_INVALID;
    int32_t *data = n <= 4 ? out->small : a.take(a.context, n);
    if (!data)
        return OW_NOMEM;
    for (size_t i = 0; i < n; ++i)
        data[i] = p[i];
    out->data = data;
    out->length = n;
    out->allocator = a;
    out->self = out;
    out->live = 1;
    return OW_OK;
}

enum ow_status omm_move(struct omm_value *dst, struct omm_value *src)
{
    if (!dst || !src || dst == src || dst->live || !valid(src))
        return OW_INVALID;
    *dst = *src;
    dst->self = dst;
    if (src->length <= 4 && OMM_MUTATION != 1)
        dst->data = dst->small;
    if (OMM_MUTATION != 2)
        *src = (struct omm_value){0};
    return OW_OK;
}

enum ow_status omm_read(const struct omm_value *v, size_t i, int32_t *out)
{
    if (!out || !valid(v) || i >= v->length)
        return OW_INVALID;
    *out = v->data[i];
    return OW_OK;
}

enum ow_status omm_consume(struct omm_value *v)
{
    if (!v || !v->live)
        return OW_DEAD;
    if (!valid(v))
        return OW_INVALID;
    int32_t *data = v->data;
    size_t n = v->length;
    struct v4_allocator a = v->allocator;
    *v = (struct omm_value){0};
    if (n > 4)
        a.release(a.context, data);
    return OW_OK;
}

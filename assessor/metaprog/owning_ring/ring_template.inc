/* Intentionally no include guard: each inclusion creates one typed instance. */
#if !defined(MPO_PREFIX) || !defined(MPO_RING) || !defined(MPO_ELEMENT) || \
    !defined(MPO_OPS)
#error MPO_REQUIRED_PARAMETERS
#endif
#if defined(MPO_JOIN) || defined(MPO_JOIN_INNER)
#error MPO_PRIVATE_MACRO_COLLISION
#endif
#define MPO_JOIN_INNER(a, b) a##b
#define MPO_JOIN(a, b) MPO_JOIN_INNER(a, b)
_Static_assert(_Generic(((MPO_RING *)0)->data[0], MPO_ELEMENT: 1, default: 0),
               "MPO_ASSOCIATED_TYPE");
_Static_assert(sizeof(((MPO_RING *)0)->data) / sizeof(MPO_ELEMENT) == 4,
               "MPO_CAPACITY");

int MPO_JOIN(MPO_PREFIX, _init)(MPO_RING *r, MPO_OPS ops)
{
    if (!r || !ops.clone || !ops.drop)
        return MPO_INVALID;
    MPO_RING empty = {0};
    empty.ops = ops;
    *r = empty;
    return MPO_OK;
}

int MPO_JOIN(MPO_PREFIX, _push)(MPO_RING *r, const MPO_ELEMENT *input)
{
    if (!r || !input)
        return MPO_INVALID;
    if (r->count == 4)
        return MPO_FULL;
    MPO_ELEMENT staged = {0};
    if (r->ops.clone(r->ops.context, input, &staged))
    {
        if (MPO_MUTATION != 1)
            r->ops.drop(r->ops.context, &staged);
        if (MPO_MUTATION == 2)
            r->count++;
        return MPO_CLONE;
    }
    r->data[(r->head + r->count) % 4] = staged;
    r->count++;
    return MPO_OK;
}

int MPO_JOIN(MPO_PREFIX, _pop)(MPO_RING *r, MPO_ELEMENT *out)
{
    if (!r || !out)
        return MPO_INVALID;
    if (!r->count)
    {
        if (MPO_MUTATION == 5)
            *out = (MPO_ELEMENT){0};
        return MPO_EMPTY;
    }
    *out = r->data[r->head];
    r->data[r->head] = (MPO_ELEMENT){0};
    r->head = (r->head + 1) % 4;
    r->count--;
    return MPO_OK;
}

void MPO_JOIN(MPO_PREFIX, _clear)(MPO_RING *r)
{
    while (r->count)
    {
        size_t slot =
            MPO_MUTATION == 3 ? (r->head + r->count - 1) % 4 : r->head;
        r->ops.drop(r->ops.context, &r->data[slot]);
        if (MPO_MUTATION != 3)
            r->head = (r->head + 1) % 4;
        r->count--;
    }
}

#undef MPO_JOIN
#undef MPO_JOIN_INNER
#undef MPO_PREFIX
#undef MPO_RING
#undef MPO_ELEMENT
#undef MPO_OPS

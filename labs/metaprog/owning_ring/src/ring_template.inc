/* Supply the repeated, typed owning-ring implementation here. No include guard. */
#define MPO_JOIN_INNER(a, b) a##b
#define MPO_JOIN(a, b) MPO_JOIN_INNER(a, b)

int MPO_JOIN(MPO_PREFIX, _init)(MPO_RING *r, MPO_OPS ops)
{
    (void)r;
    (void)ops;
    return MPO_INVALID;
}

int MPO_JOIN(MPO_PREFIX, _push)(MPO_RING *r, const MPO_ELEMENT *in)
{
    (void)r;
    (void)in;
    return MPO_INVALID;
}

int MPO_JOIN(MPO_PREFIX, _pop)(MPO_RING *r, MPO_ELEMENT *out)
{
    (void)r;
    (void)out;
    return MPO_INVALID;
}

void MPO_JOIN(MPO_PREFIX, _clear)(MPO_RING *r)
{
    (void)r;
}

#undef MPO_JOIN
#undef MPO_JOIN_INNER
#undef MPO_PREFIX
#undef MPO_RING
#undef MPO_ELEMENT
#undef MPO_OPS

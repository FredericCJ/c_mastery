#include "owning_ring.h"

int mpo_manual_init(struct mpo_blob_ring *r, struct mpo_blob_ops ops)
{
    if (!r || !ops.clone || !ops.drop)
        return MPO_INVALID;
    struct mpo_blob_ring empty = {0};
    empty.ops = ops;
    *r = empty;
    return MPO_OK;
}

int mpo_manual_push(struct mpo_blob_ring *r, const struct mpo_blob *input)
{
    if (!r || !input)
        return MPO_INVALID;
    if (r->count == 4)
        return MPO_FULL;
    struct mpo_blob staged = {0};
    if (r->ops.clone(r->ops.context, input, &staged))
    {
        r->ops.drop(r->ops.context, &staged);
        return MPO_CLONE;
    }
    r->data[(r->head + r->count) % 4] = staged;
    r->count++;
    return MPO_OK;
}

int mpo_manual_pop(struct mpo_blob_ring *r, struct mpo_blob *out)
{
    if (!r || !out)
        return MPO_INVALID;
    if (!r->count)
        return MPO_EMPTY;
    *out = r->data[r->head];
    r->data[r->head] = (struct mpo_blob){0};
    r->head = (r->head + 1) % 4;
    r->count--;
    return MPO_OK;
}

void mpo_manual_clear(struct mpo_blob_ring *r)
{
    while (r->count)
    {
        r->ops.drop(r->ops.context, &r->data[r->head]);
        r->head = (r->head + 1) % 4;
        r->count--;
    }
}

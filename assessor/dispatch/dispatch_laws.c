#include "dispatch_laws.h"
#ifndef DS_LAWS_MUTANT
#define DS_LAWS_MUTANT 0
#endif
int ds_copy_checked(struct ds_copy_view view, unsigned char *out,
                    size_t capacity, size_t *length)
{
    if (!view.object || !view.ops || !view.ops->length || !view.ops->copy ||
        !length || (!out && capacity))
        return DS_INVALID;
    size_t promised = 0;
    int rc = view.ops->length(view.object, &promised);
    if (rc != DS_OK)
        return rc;
    if (promised > 16 || promised > capacity)
        return DS_FULL;
    unsigned char staged[16] = {0};
    size_t copied = 0;
    rc = view.ops->copy(view.object, staged, promised, &copied);
    if (DS_LAWS_MUTANT == 2 && promised)
        out[0] = staged[0];
    if (rc != DS_OK)
        return rc;
    if (DS_LAWS_MUTANT != 1 && copied != promised)
        return DS_INVALID;
    for (size_t i = 0; i < promised; ++i)
        out[i] = staged[i];
    *length = promised;
    return DS_OK;
}

int ds_multi_views(const struct ds_multi *owner, struct ds_view out[2])
{
    if (!owner || !out || !owner->scalar.ops || !owner->scalar.ops->measure ||
        (!owner->bytes.bytes && owner->bytes.length) ||
        owner->bytes.length > 16)
        return DS_INVALID;
    struct ds_view bytes = ds_byte_interface(&owner->bytes).view,
                   scalar = {&owner->scalar.value, owner->scalar.ops};
    if (DS_LAWS_MUTANT == 3)
        bytes = scalar;
    if (DS_LAWS_MUTANT == 4)
    {
        static const int zero;
        scalar.object = &zero;
    }
    out[0] = bytes;
    out[1] = scalar;
    return DS_OK;
}

int ds_capture_checked(struct ds_erased *out, struct ds_capture_spec spec,
                       const int32_t *values, size_t count,
                       void (*drop)(void *), void *context)
{
    if (!out || !spec.required_alignment || spec.returns_borrow > 1)
        return DS_INVALID;
    if ((DS_LAWS_MUTANT != 5 && spec.kind != DS_CAPTURE_I32) ||
        spec.required_alignment > _Alignof(int32_t) ||
        (DS_LAWS_MUTANT != 6 && spec.returns_borrow))
        return DS_UNSUPPORTED;
    return ds_capture_i32(out, values, count, drop, context);
}

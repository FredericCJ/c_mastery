#include "owning_option.h"
#ifndef EOP_MUTATION
#define EOP_MUTATION 0
#endif
static int tag(unsigned t)
{
    return t == EOP_NONE || t == EOP_SOME;
}

int eop_scalar_make(uint32_t value, unsigned present, struct eop_scalar *out)
{
    if (!out || present > 1)
        return EOP_INVALID;
    *out = (struct eop_scalar){EOP_MUTATION == 5 && value == 0 ? 0 : present,
                               present ? value : 0};
    return EOP_OK;
}

int eop_scalar_get(const struct eop_scalar *o, uint32_t *out)
{
    if (!o || !out || (EOP_MUTATION != 6 && !tag(o->tag)))
        return EOP_INVALID;
    if (o->tag == EOP_NONE)
        return EOP_ABSENT;
    *out = o->value;
    return EOP_OK;
}

int eop_result_get(er_result r, uint32_t *out, size_t *offset)
{
    if (!out || !offset || r.tag < ER_OK || r.tag > ER_NOT_IMPLEMENTED)
        return EOP_INVALID;
    if (r.tag != ER_OK)
    {
        *offset = r.as.offset;
        return EOP_ABSENT;
    }
    *out = r.as.value;
    return EOP_OK;
}

int eop_create(struct eop_owner *out, struct v4_allocator a, uint32_t id,
               int32_t value)
{
    if (!out || !tag(out->tag) || !a.take || !a.release)
        return EOP_INVALID;
    if (out->tag == EOP_SOME)
        return EOP_BUSY;
    if (EOP_MUTATION == 1 && id == 0)
    {
        *out = (struct eop_owner){0};
        return EOP_OK;
    }
    int32_t *cell = a.take(a.context, 1);
    if (!cell)
    {
        if (EOP_MUTATION == 7)
            out->tag = EOP_SOME;
        return EOP_NOMEM;
    }
    *cell = value;
    struct eop_owner staged = {0};
    staged.tag = EOP_SOME;
    staged.as.some.id = id;
    staged.as.some.cell = cell;
    staged.as.some.allocator = a;
    *out = staged;
    return EOP_OK;
}

int eop_clone(const struct eop_owner *source, struct eop_owner *out)
{
    if (!source || !out || source == out || !tag(source->tag) || !tag(out->tag))
        return EOP_INVALID;
    if (out->tag == EOP_SOME)
        return EOP_BUSY;
    if (source->tag == EOP_NONE)
    {
        *out = (struct eop_owner){0};
        if (EOP_MUTATION == 8)
            out->tag = EOP_SOME;
        return EOP_OK;
    }
    if (EOP_MUTATION == 2)
    {
        *out = *source;
        return EOP_OK;
    }
    return eop_create(out, source->as.some.allocator, source->as.some.id,
                      *source->as.some.cell);
}

int eop_move(struct eop_owner *out, struct eop_owner *source)
{
    if (!out || !source || out == source || !tag(out->tag) || !tag(source->tag))
        return EOP_INVALID;
    if (out->tag == EOP_SOME)
        return EOP_BUSY;
    *out = *source;
    if (EOP_MUTATION != 3)
        *source = (struct eop_owner){0};
    return EOP_OK;
}

int eop_read(const struct eop_owner *source, uint32_t *id, int32_t *value)
{
    if (!source || !id || !value || !tag(source->tag))
        return EOP_INVALID;
    if (source->tag == EOP_NONE)
        return EOP_ABSENT;
    *id = source->as.some.id;
    *value = *source->as.some.cell;
    return EOP_OK;
}

int eop_drop(struct eop_owner *owner)
{
    if (!owner || !tag(owner->tag))
        return EOP_INVALID;
    if (owner->tag == EOP_NONE)
        return EOP_OK;
    int32_t *cell = owner->as.some.cell;
    struct v4_allocator a = owner->as.some.allocator;
    *owner = (struct eop_owner){0};
    if (EOP_MUTATION != 4)
        a.release(a.context, cell);
    return EOP_OK;
}

int eop_unit(enum eop_unit unit)
{
    return unit == EOP_UNIT ? EOP_OK : EOP_INVALID;
}

#include "composition.h"
#ifndef ERC_MUTATION
#define ERC_MUTATION 0
#endif
static int cause_valid(struct erc_cause c)
{
    return c.domain != 0 && c.code != 0;
}

static int error_valid(const struct erc_error *e)
{
    if (!e || e->count < 1 || e->count > 4)
    {
        return 0;
    }
    for (unsigned i = 0; i < e->count; ++i)
    {
        if (!cause_valid(e->cause[i]))
        {
            return 0;
        }
    }
    return 1;
}

static int result_valid(const struct erc_result *r)
{
    return r && (r->tag == ERC_EMPTY ||
                 (r->tag == ERC_VALUE && r->as.owner.tag == EOP_SOME) ||
                 (r->tag == ERC_ERROR && error_valid(&r->as.error)));
}

int erc_cause(struct erc_cause root, struct erc_error *out)
{
    if (!out || !cause_valid(root))
    {
        return ERC_INVALID;
    }
    struct erc_error e = {0};
    e.count = 1;
    e.cause[0] = root;
    *out = e;
    return ERC_OK;
}

int erc_wrap(const struct erc_error *inner, struct erc_cause outer,
             struct erc_error *out)
{
    if (!out || !error_valid(inner) || !cause_valid(outer))
    {
        return ERC_INVALID;
    }
    if (inner->count == 4 && ERC_MUTATION != 5)
    {
        return ERC_FULL;
    }
    struct erc_error next = *inner;
    if (next.count == 4)
    {
        next.count = 3;
    }
    next.cause[next.count++] = outer;
    if (ERC_MUTATION == 4)
    {
        next.cause[0].offset = 0;
    }
    *out = next;
    return ERC_OK;
}

int erc_error(const struct erc_error *error, struct erc_result *out)
{
    if (!error_valid(error) || !result_valid(out))
    {
        return ERC_INVALID;
    }
    if (out->tag != ERC_EMPTY)
    {
        return ERC_BUSY;
    }
    struct erc_result next = {0};
    next.tag = ERC_ERROR;
    next.as.error = *error;
    *out = next;
    return ERC_OK;
}

int erc_adopt(struct eop_owner *source, struct erc_result *out)
{
    if (!source || source->tag != EOP_SOME || !result_valid(out))
    {
        return ERC_INVALID;
    }
    if (out->tag != ERC_EMPTY)
    {
        return ERC_BUSY;
    }
    struct erc_result next = {0};
    next.tag = ERC_VALUE;
    if (ERC_MUTATION == 7)
    {
        next.as.owner = *source;
    }
    else if (eop_move(&next.as.owner, source) != EOP_OK)
    {
        return ERC_INVALID;
    }
    *out = next;
    return ERC_OK;
}

int erc_map_clone(const struct erc_result *source, erc_map_fn fn, void *context,
                  struct erc_result *out)
{
    if (!result_valid(source) || !result_valid(out) || source == out ||
        source->tag == ERC_EMPTY)
    {
        return ERC_INVALID;
    }
    if (out->tag != ERC_EMPTY)
    {
        return ERC_BUSY;
    }
    if (source->tag == ERC_ERROR)
    {
        if (ERC_MUTATION == 2 && fn)
        {
            (void)fn(0, context);
        }
        *out = *source;
        return ERC_OK;
    }
    if (!fn)
    {
        return ERC_INVALID;
    }
    int32_t early = 0;
    if (ERC_MUTATION == 6)
    {
        early = fn(*source->as.owner.as.some.cell, context);
    }
    struct erc_result next = {0};
    next.tag = ERC_VALUE;
    if (ERC_MUTATION == 1)
    {
        next.as.owner = source->as.owner;
    }
    else if (eop_clone(&source->as.owner, &next.as.owner) != EOP_OK)
    {
        return ERC_NOMEM;
    }
    *next.as.owner.as.some.cell =
        ERC_MUTATION == 6 ? early : fn(*next.as.owner.as.some.cell, context);
    *out = next;
    return ERC_OK;
}

int erc_drop(struct erc_result *r)
{
    if (!result_valid(r))
    {
        return ERC_INVALID;
    }
    if (r->tag != ERC_VALUE)
    {
        *r = (struct erc_result){0};
        return ERC_OK;
    }
    struct eop_owner owner = r->as.owner;
    *r = (struct erc_result){0};
    if (ERC_MUTATION != 3)
    {
        (void)eop_drop(&owner);
    }
    return ERC_OK;
}

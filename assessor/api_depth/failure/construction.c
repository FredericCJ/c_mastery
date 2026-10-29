#include "construction.h"

void ae_construction_init(struct ae_construction *c, struct ac_ledger *l)
{
    if (c)
    {
        *c = (struct ae_construction){0};
        c->ledger = l;
    }
}

static unsigned dependents(unsigned resource)
{
    return resource < 2 ? 12u : 0u;
}

static unsigned held(const struct ac_ledger *l)
{
    unsigned mask = 0;
    for (unsigned i = 0; i < 4; ++i)
        if (l->held[i])
            mask |= 1u << i;
    return mask;
}

static enum ae_code close_at(struct ae_construction *c, unsigned position,
                             unsigned failed)
{
    unsigned id = c->order[position];
    if (id > 3)
        return AE_CONTRACT;
    if (!(c->mask & (1u << id)))
        return AE_OK;
#ifndef AE_MUTANT_DEPENDENCY
    if (held(c->ledger) & dependents(id))
    {
        if (c->error.result.code == AE_OK)
            (void)ae_first(&c->error, AE_DEVICE, c->generation, id, 0,
                           90 + (int)id);
        return AE_DEVICE;
    }
#endif
#ifdef AE_MUTANT_MISSED_RELEASE
    if (id == 0)
        return AE_OK;
#endif
    enum ac_status r =
        ac_resource_close(c->ledger, id, !!(failed & (1u << id)));
    if (r != AC_OK)
    {
        if (c->error.result.code == AE_OK)
            (void)ae_first(&c->error, AE_DEVICE, c->generation, id, 0,
                           70 + (int)id);
        else
            (void)ae_secondary(&c->error, id, 70 + (int)id);
    }
    if (!c->ledger->held[id])
        c->mask &= ~(1u << id);
    return r == AC_OK ? AE_OK : AE_DEVICE;
}

static void close_accumulate(struct ae_construction *c, unsigned position,
                             unsigned failed, enum ae_code *result)
{
    enum ae_code r = close_at(c, position, failed);
    if (r != AE_OK)
        *result = r;
}

static enum ae_code cleanup_nested(struct ae_construction *c, unsigned failed)
{
    enum ae_code r = AE_OK;
    if (c->count)
    {
        if (c->count > 1)
        {
            if (c->count > 2)
            {
                if (c->count > 3)
                    close_accumulate(c, 3, failed, &r);
                close_accumulate(c, 2, failed, &r);
            }
            close_accumulate(c, 1, failed, &r);
        }
        close_accumulate(c, 0, failed, &r);
    }
    return r;
}

static enum ae_code cleanup_labels(struct ae_construction *c, unsigned failed)
{
    enum ae_code r = AE_OK;
    if (c->count == 0)
        goto done;
    if (c->count == 1)
        goto close0;
    if (c->count == 2)
        goto close1;
    if (c->count == 3)
        goto close2;
    close_accumulate(c, 3, failed, &r);
close2:
    close_accumulate(c, 2, failed, &r);
close1:
    close_accumulate(c, 1, failed, &r);
close0:
    close_accumulate(c, 0, failed, &r);
done:
    return r;
}

enum ae_code ae_cleanup(struct ae_construction *c, unsigned failed)
{
    if (!c || !c->ledger || failed > 15 || c->count > 4 || c->mask > 15 ||
        c->cleanup_style > AE_LOOP)
        return AE_INVALID;
    if (c->committed)
        return AE_CONFLICT;
    if (c->ledger->count > AC_EVENTS - 4)
        return AE_FULL;
    enum ae_code r = AE_OK;
    if (c->cleanup_style == AE_NESTED)
        r = cleanup_nested(c, failed);
    else if (c->cleanup_style == AE_LABELS)
        r = cleanup_labels(c, failed);
    else
        for (unsigned j = c->count; j > 0; --j)
            close_accumulate(c, j - 1, failed, &r);
    if (!c->mask)
    {
        c->state = AE_CSTOPPED;
        c->published = 0;
    }
    else
        c->state = AE_CFAILED;
    return r;
}

static enum ae_code acquire(struct ae_construction *c, unsigned position,
                            unsigned fail)
{
    unsigned id = c->order[position];
    enum ac_status r = ac_resource_acquire(c->ledger, id, c->generation,
                                           fail == position + 1, 0);
    if (r != AC_OK)
    {
        (void)ae_first(&c->error, AE_EXHAUSTED, c->generation, id, 0, 11);
        return AE_EXHAUSTED;
    }
    c->mask |= 1u << id;
    return AE_OK;
}

static enum ae_code nested(struct ae_construction *c, unsigned fail)
{
    enum ae_code r = acquire(c, 0, fail);
    if (r == AE_OK)
    {
        r = acquire(c, 1, fail);
        if (r == AE_OK)
        {
            r = acquire(c, 2, fail);
            if (r == AE_OK && c->count == 4)
                r = acquire(c, 3, fail);
        }
    }
    return r;
}

static enum ae_code labels(struct ae_construction *c, unsigned fail)
{
    enum ae_code r = acquire(c, 0, fail);
    if (r != AE_OK)
        goto failed;
    r = acquire(c, 1, fail);
    if (r != AE_OK)
        goto failed;
    r = acquire(c, 2, fail);
    if (r != AE_OK)
        goto failed;
    if (c->count == 4)
    {
        r = acquire(c, 3, fail);
        if (r != AE_OK)
            goto failed;
    }
    return AE_OK;
failed:
    return r;
}

static enum ae_code loop(struct ae_construction *c, unsigned fail)
{
    for (unsigned i = 0; i < c->count; ++i)
    {
        enum ae_code r = acquire(c, i, fail);
        if (r != AE_OK)
            return r;
    }
    return AE_OK;
}

enum ae_code ae_construct(struct ae_construction *c, unsigned count,
                          unsigned swapped, unsigned fail,
                          unsigned close_failed,
                          enum ae_construction_style style,
                          struct ae_construction_handle *out)
{
    if (!c || !c->ledger || !out || (count != 3 && count != 4) || swapped > 1 ||
        fail > count + 1 || close_failed > 15 || (unsigned)style > AE_LOOP ||
        (unsigned)c->state > AE_CSTOPPED)
        return AE_INVALID;
    if (c->mask || c->state == AE_CACTIVE || c->committed)
        return AE_CONFLICT;
    if (c->generation >= 3 || c->ledger->count > AC_EVENTS - 8)
        return AE_FULL;
    for (unsigned i = 0; i < 4; ++i)
        if (c->ledger->held[i])
            return AE_CONFLICT;
    c->error = (struct ae_error){0};
    c->count = count;
    c->cleanup_style = (unsigned)style;
    c->transfer_count = 0;
    c->order[0] = swapped ? 1 : 0;
    c->order[1] = swapped ? 0 : 1;
    c->order[2] = 2;
    c->order[3] = 3;
    ++c->generation;
    c->published = 0;
    c->state = AE_CPARTIAL;
    enum ae_code r = style == AE_NESTED   ? nested(c, fail)
                     : style == AE_LABELS ? labels(c, fail)
                                          : loop(c, fail);
    if (r == AE_OK && fail == count + 1)
    {
        r = AE_DEVICE;
        (void)ae_first(&c->error, AE_DEVICE, c->generation, 3, 0, 99);
    }
    if (r != AE_OK)
    {
        c->state = AE_CFAILED;
        (void)ae_cleanup(c, close_failed);
        return r;
    }
    c->state = AE_CACTIVE;
    c->published = 1;
    *out = (struct ae_construction_handle){c, c->generation};
    return AE_OK;
}

enum ae_code ae_construction_read(struct ae_construction_handle h,
                                  unsigned *out)
{
    if (!h.owner || !out || !h.generation)
        return AE_INVALID;
    if (h.owner->generation != h.generation || h.owner->state != AE_CACTIVE ||
        !h.owner->published)
        return AE_CONFLICT;
    *out = h.owner->mask;
    return AE_OK;
}

enum ae_code ae_transfer(struct ae_construction *c, unsigned id)
{
    if (!c || id > 3 || c->state != AE_CACTIVE || !(c->mask & (1u << id)) ||
        (held(c->ledger) & dependents(id)))
        return AE_INVALID;
    if (c->transfer_count == 4)
        return AE_FULL;
    c->transfers[c->transfer_count++] = (struct ae_transfer){id, c->generation};
    c->mask &= ~(1u << id);
    return AE_OK;
}

enum ae_code ae_after_publish(struct ae_construction *c, unsigned fail)
{
    if (!c || c->state != AE_CACTIVE || fail > 1 || c->committed)
        return AE_INVALID;
    c->committed = 1;
    if (fail)
    {
        (void)ae_first(&c->error, AE_AMBIGUOUS, c->generation, 3, 0, 88);
        return AE_AMBIGUOUS;
    }
    return AE_OK;
}

enum ae_code ae_compensate(struct ae_construction *c, unsigned present)
{
    if (!c || present > 1 || !c->committed)
        return AE_INVALID;
    if (present)
        return AE_AGAIN;
    c->committed = 0;
    return AE_OK;
}

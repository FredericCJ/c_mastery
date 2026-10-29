#include "grace.h"
#include <limits.h>

static int lease_valid(const struct ag_domain *d, struct ag_lease l)
{
    return d && l.owner == d && l.participant < 4 &&
           d->registered[l.participant] &&
           d->generation[l.participant] == l.generation;
}

void ag_init(struct ag_domain *d, unsigned nested)
{
    cmx_epoch_init(&d->retained);
    d->nested = !!nested;
    for (unsigned p = 0; p < 4; p++)
    {
        d->registered[p] = d->depth[p] = d->section[p] = 0;
        d->generation[p] = 1;
    }
}

int ag_register(struct ag_domain *d, struct ag_lease *out)
{
    if (!d || !out)
        return AG_INVALID;
    for (unsigned p = 0; p < 4; p++)
        if (!d->registered[p] && d->generation[p] != UINT_MAX)
        {
            d->registered[p] = 1;
            out->owner = d;
            out->participant = p;
            out->generation = d->generation[p];
            return AG_OK;
        }
    return AG_FULL;
}

int ag_enter(struct ag_domain *d, struct ag_lease l)
{
    if (!lease_valid(d, l))
        return AG_INVALID;
    unsigned p = l.participant;
    if (d->depth[p])
    {
        if (!d->nested)
            return AG_INVALID;
        if (d->depth[p] == UINT_MAX)
            return AG_EXHAUSTED;
        d->depth[p]++;
        return AG_OK;
    }
    if (d->section[p] == UINT_MAX)
        return AG_EXHAUSTED;
    if (cmx_epoch_enter(&d->retained, p) != CMX_OK)
        return AG_INVALID;
    d->depth[p] = 1;
    d->section[p]++;
    return AG_OK;
}

int ag_leave(struct ag_domain *d, struct ag_lease l)
{
    if (!lease_valid(d, l) || !d->depth[l.participant])
        return AG_INVALID;
    unsigned p = l.participant;
    d->depth[p]--;
#ifdef AG_MUTANT_NESTED
    cmx_epoch_leave(&d->retained, p);
#else
    if (!d->depth[p])
        cmx_epoch_leave(&d->retained, p);
#endif
    return AG_OK;
}

int ag_depart(struct ag_domain *d, struct ag_lease l)
{
    if (!lease_valid(d, l))
        return AG_INVALID;
    if (d->depth[l.participant])
        return AG_BUSY;
    d->registered[l.participant] = 0;
    d->generation[l.participant]++;
    return AG_OK;
}

int ag_advance(struct ag_domain *d)
{
    if (!d)
        return AG_INVALID;
    if (d->retained.epoch == UINT32_MAX)
        return AG_EXHAUSTED;
    return cmx_epoch_advance(&d->retained) ? AG_OK : AG_PENDING;
}

int ag_reclaimable(const struct ag_domain *d, unsigned retired)
{
    if (!d || retired > d->retained.epoch)
        return 0;
    for (unsigned p = 0; p < 4; p++)
        if (d->retained.active[p] && d->retained.seen[p]
#ifdef AG_MUTANT_EPOCH
                                         < retired
#else
                                         <= retired
#endif
        )
            return 0;
    return 1;
}

void ag_store_init(struct ag_store *s, unsigned nested, unsigned char initial)
{
    ag_init(&s->domain, nested);
    s->current = 0;
    s->writer = AG_NONE;
    s->drops = s->invocations = 0;
    api_registry_init(&s->callbacks);
    for (unsigned i = 0; i < AG_VERSIONS; i++)
    {
        struct ag_object *o = &s->objects[i];
        o->active = i == 0;
        o->generation = i == 0;
        o->retired = o->retired_epoch = o->refs = 0;
        for (unsigned j = 0; j < 32; j++)
            o->bytes[j] = initial;
    }
    for (unsigned i = 0; i < 4; i++)
        s->capture[i].occupied = 0;
}

int ag_writer_acquire(struct ag_store *s, unsigned writer)
{
    if (!s || writer == AG_NONE)
        return AG_INVALID;
    if (s->writer != AG_NONE
#ifdef AG_MUTANT_WRITER
        && 0
#endif
    )
        return AG_BUSY;
    s->writer = writer;
    return AG_OK;
}

int ag_writer_release(struct ag_store *s, unsigned writer)
{
    if (!s || writer == AG_NONE || s->writer != writer)
        return AG_INVALID;
    s->writer = AG_NONE;
    return AG_OK;
}

int ag_publish(struct ag_store *s, unsigned writer, unsigned char value)
{
    if (!s || writer == AG_NONE || s->writer != writer)
        return AG_INVALID;
    if (s->domain.retained.epoch == UINT32_MAX)
        return AG_EXHAUSTED;
    unsigned at = AG_NONE;
    for (unsigned i = 0; i < AG_VERSIONS; i++)
        if (!s->objects[i].active && s->objects[i].generation != UINT_MAX)
        {
            at = i;
            break;
        }
    if (at == AG_NONE)
        return AG_FULL;
    struct ag_object *fresh = &s->objects[at], *old = &s->objects[s->current];
    fresh->generation++;
    fresh->active = 1;
    fresh->retired = fresh->retired_epoch = fresh->refs = 0;
    for (unsigned j = 0; j < 32; j++)
        fresh->bytes[j] = value;
    old->retired = 1;
    old->retired_epoch = s->domain.retained.epoch;
    s->current = at;
    (void)ag_advance(&s->domain);
    return AG_OK;
}

static int version_valid(struct ag_store *s, struct ag_version v)
{
    return v.index < AG_VERSIONS && s->objects[v.index].active &&
           s->objects[v.index].generation == v.generation;
}

static int borrow_valid(struct ag_store *s, struct ag_borrow b)
{
    return s && lease_valid(&s->domain, b.lease) &&
           s->domain.depth[b.lease.participant] &&
           s->domain.section[b.lease.participant] == b.section &&
           version_valid(s, b.version);
}

int ag_read(struct ag_store *s, struct ag_lease l, struct ag_borrow *out)
{
    if (!s || !out)
        return AG_INVALID;
    int r = ag_enter(&s->domain, l);
    if (r != AG_OK)
        return r;
    out->lease = l;
    out->section = s->domain.section[l.participant];
    out->version =
        (struct ag_version){s->current, s->objects[s->current].generation};
    return AG_OK;
}

int ag_copy(struct ag_store *s, struct ag_borrow b, unsigned char out[32])
{
    if (!out || !borrow_valid(s, b))
        return AG_INVALID;
    for (unsigned i = 0; i < 32; i++)
        out[i] = s->objects[b.version.index].bytes[i];
    return AG_OK;
}

int ag_read_end(struct ag_store *s, struct ag_borrow b)
{
    if (!borrow_valid(s, b))
        return AG_INVALID;
    return ag_leave(&s->domain, b.lease);
}

unsigned ag_collect(struct ag_store *s)
{
    unsigned count = 0;
    for (unsigned i = 0; i < AG_VERSIONS; i++)
    {
        struct ag_object *o = &s->objects[i];
        if (o->active && o->retired &&
            ag_reclaimable(&s->domain, o->retired_epoch)
#ifndef AG_MUTANT_REFS
            && !o->refs
#endif
        )
        {
            o->active = 0;
            count++;
        }
    }
    return count;
}

static void captured(void *ctx, struct api_registry *r, struct api_handle h)
{
    (void)r;
    struct ag_capture *c = ctx;
    struct ag_store *s = c->owner;
    if (version_valid(s, c->version) && s->objects[c->version.index].refs)
    {
        s->invocations++;
        c->callback(c->context, s, h, s->objects[c->version.index].bytes);
    }
}

static void released(void *ctx)
{
    struct ag_capture *c = ctx;
    struct ag_store *s = c->owner;
    struct ag_object *o = &s->objects[c->version.index];
    o->refs--;
    c->occupied = 0;
    s->drops++;
}

enum api_status ag_capture(struct ag_store *s, struct ag_borrow b,
                           ag_callback_fn cb, void *ctx, struct api_handle *out)
{
    if (!cb || !out || !borrow_valid(s, b))
        return API_INVALID;
    struct ag_object *o = &s->objects[b.version.index];
    if (o->refs == UINT_MAX)
        return API_FULL;
    for (unsigned i = 0; i < 4; i++)
        if (!s->capture[i].occupied)
        {
            struct ag_capture *c = &s->capture[i];
            c->owner = s;
            c->version = b.version;
            c->callback = cb;
            c->context = ctx;
            c->occupied = 1;
#ifndef AG_MUTANT_CAPTURE
            o->refs++;
#endif
            enum api_status result =
                api_add(&s->callbacks, captured, c, released, out);
            if (result != API_OK)
            {
                o->refs--;
                c->occupied = 0;
            }
            return result;
        }
    return API_FULL;
}

enum api_status ag_dispatch(struct ag_store *s)
{
    return s ? api_dispatch(&s->callbacks) : API_INVALID;
}

enum api_status ag_remove(struct ag_store *s, struct api_handle h)
{
    return s ? api_remove(&s->callbacks, h) : API_INVALID;
}

enum api_status ag_destroy_callbacks(struct ag_store *s)
{
    return s ? api_destroy(&s->callbacks) : API_INVALID;
}

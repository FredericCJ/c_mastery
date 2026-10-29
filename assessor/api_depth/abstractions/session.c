#include "session.h"
#include <string.h>

struct as_opaque
{
    struct as_session session;
    struct ac_allocate allocator;
};

void as_session_init(struct as_session *s, struct ac_ledger *ledger)
{
    if (s)
    {
        *s = (struct as_session){AS_NEW, 0, 0, ledger};
    }
}

enum as_status as_construct(struct as_session *s, unsigned fail,
                            as_construction_observer_fn observer, void *ctx)
{
    if (!s || s->state != AS_NEW || s->ready || !s->ledger || fail > 1 ||
        s->ledger->held[0] || s->ledger->held[1])
    {
        return AS_INVALID;
    }
    if (s->ledger->count > AC_EVENTS - 4)
    {
        return AS_FULL;
    }
    if (ac_resource_acquire(s->ledger, 0, 1, 0, 0) != AC_OK)
    {
        return AS_FAILED;
    }
    /* The callback sees a valid non-active object, never uninitialized fields. */
    s->count = 0;
    s->ready = 1;
#ifdef AS_MUTANT_EARLY_ACTIVE
    s->state = AS_ACTIVE;
#endif
    if (observer)
    {
        observer(ctx, s);
    }
    if (ac_resource_acquire(s->ledger, 1, 1, (int)fail, 0) != AC_OK)
    {
        (void)ac_resource_close(s->ledger, 0, 0);
        s->ready = 0;
        s->state = AS_STOPPED;
        return AS_FAILED;
    }
    s->state = AS_ACTIVE;
    return AS_OK;
}

enum as_status as_event(struct as_session *s, enum as_event event)
{
    if (!s || s->state < AS_NEW || s->state > AS_STOPPED || s->count > 15 ||
        event < AS_START || event > AS_CANCEL)
    {
        return AS_INVALID;
    }
    if (!s->ledger || s->ready > 1)
    {
        return AS_INVALID;
    }
    if ((s->state == AS_ACTIVE || s->state == AS_SUSPENDED) &&
        (!s->ready || !s->ledger->held[0] || !s->ledger->held[1]))
    {
        return AS_INVALID;
    }
    if (s->state == AS_STOPPED &&
        (s->ready || s->ledger->held[0] || s->ledger->held[1]))
    {
        return AS_INVALID;
    }
    if (event == AS_START)
    {
        return as_construct(s, 0, 0, 0);
    }
    if (s->state == AS_NEW)
    {
        return AS_INVALID;
    }
    if (event == AS_STOP || event == AS_CANCEL)
    {
        if (s->state == AS_STOPPED)
        {
            return AS_OK;
        }
        if (!s->ledger || !s->ready || s->ledger->count > AC_EVENTS - 2)
        {
            return AS_INVALID;
        }
        if (ac_resource_close(s->ledger, 1, 0) != AC_OK ||
            ac_resource_close(s->ledger, 0, 0) != AC_OK)
        {
            return AS_FAILED;
        }
        s->state = AS_STOPPED;
        s->ready = 0;
        return AS_OK;
    }
    if (!s->ready)
    {
        return AS_INVALID;
    }
    if (event == AS_INCREMENT && s->state == AS_ACTIVE)
    {
#ifndef AS_MUTANT_OVERFLOW
        if (s->count == 15)
        {
            return AS_FULL;
        }
#endif
        ++s->count;
        return AS_OK;
    }
    if (event == AS_SUSPEND && s->state == AS_ACTIVE)
    {
        s->state = AS_SUSPENDED;
        return AS_OK;
    }
    if (event == AS_RESUME && s->state == AS_SUSPENDED)
    {
        s->state = AS_ACTIVE;
        return AS_OK;
    }
    return AS_INVALID;
}

enum as_status as_opaque_create(struct ac_allocate a, struct ac_ledger *ledger,
                                struct as_opaque **out)
{
    if (!out || !a.allocate || !a.release || !ledger)
    {
        return AS_INVALID;
    }
    struct as_opaque *o =
        a.allocate(a.context, sizeof *o, _Alignof(struct as_opaque));
    if (!o)
    {
        return AS_FAILED;
    }
    o->allocator = a;
    as_session_init(&o->session, ledger);
    *out = o;
    return AS_OK;
}

enum as_status as_opaque_event(struct as_opaque *o, enum as_event event)
{
    return o ? as_event(&o->session, event) : AS_INVALID;
}

enum as_status as_opaque_read(const struct as_opaque *o, unsigned *out)
{
    if (!o || !out)
    {
        return AS_INVALID;
    }
    *out = o->session.count;
    return AS_OK;
}

enum as_status as_opaque_destroy(struct as_opaque *o)
{
    if (!o)
    {
        return AS_OK;
    }
    if (o->session.state == AS_ACTIVE || o->session.state == AS_SUSPENDED)
    {
        if (as_event(&o->session, AS_STOP) != AS_OK)
        {
            return AS_FAILED;
        }
    }
    struct ac_allocate a = o->allocator;
    a.release(a.context, o);
    return AS_OK;
}

static enum as_status direct(void *ctx, enum as_event event)
{
    return as_event(ctx, event);
}

static enum as_status adapted(void *ctx, enum as_event event)
{
    struct as_extra *extra = ctx;
    if (!extra || extra->calls == UINT32_MAX)
    {
        return AS_INVALID;
    }
    ++extra->calls;
    return as_event(extra->session, event);
}

static enum as_status flush(void *ctx)
{
    return ctx ? AS_OK : AS_INVALID;
}
const struct as_ops_v1 as_direct_ops = {{1, 0, sizeof(struct as_ops_v1)},
                                        direct};
const struct as_ops_v2 as_adapted_ops = {
    {{2, 0, sizeof(struct as_ops_v2)}, adapted},
    flush};

enum as_status as_backend(const void *data, size_t available, void *ctx,
                          struct as_backend *out)
{
    if (!data || !ctx || !out || available < sizeof(struct as_header))
    {
        return AS_INVALID;
    }
    struct as_header header;
    memcpy(&header, data, sizeof header);
    if (header.bytes > available || header.bytes < sizeof(struct as_ops_v1) ||
        header.flags || (header.version != 1 && header.version != 2))
    {
        return AS_UNSUPPORTED;
    }
    struct as_ops_v1 base;
    memcpy(&base, data, sizeof base);
    if (!base.run)
    {
        return AS_INVALID;
    }
    as_flush_fn optional = 0;
    if (header.version == 2)
    {
        if (header.bytes < sizeof(struct as_ops_v2))
        {
            return AS_INVALID;
        }
        struct as_ops_v2 extended;
        memcpy(&extended, data, sizeof extended);
        optional = extended.flush;
    }
    *out = (struct as_backend){base.run, optional, ctx};
    return AS_OK;
}

enum as_status as_backend_run(struct as_backend b, enum as_event event)
{
    return b.run ? b.run(b.context, event) : AS_INVALID;
}

enum as_status as_backend_flush(struct as_backend b)
{
    return b.flush ? b.flush(b.context) : AS_UNSUPPORTED;
}

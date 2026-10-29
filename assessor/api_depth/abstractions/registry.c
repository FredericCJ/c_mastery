#include "registry.h"

static void drop(void *ctx)
{
    struct as_registration *slot = ctx;
    api_drop_fn release = slot->drop;
    void *context = slot->context;
    slot->occupied = slot->enabled = 0;
    slot->callback = 0;
    slot->context = 0;
    slot->drop = 0;
    if (release)
    {
        release(context);
    }
}

static void dispatch(void *ctx, struct api_registry *core,
                     struct api_handle handle)
{
    (void)core;
    struct as_registration *slot = ctx;
    if (slot->enabled)
    {
        slot->callback(slot->context, slot->owner, handle);
    }
}

void as_registry_init(struct as_registry *r, unsigned suppression,
                      as_terminal_fn terminal, void *ctx)
{
    if (!r)
    {
        return;
    }
    api_registry_init(&r->core);
    r->suppress = !!suppression;
    r->terminal = terminal;
    r->terminal_context = ctx;
    for (unsigned i = 0; i < 4; ++i)
    {
        r->registrations[i] = (struct as_registration){0};
    }
}

enum api_status as_register(struct as_registry *r, as_callback_fn cb, void *ctx,
                            api_drop_fn release, struct api_handle *out)
{
    if (!r || !cb || !out)
    {
        return API_INVALID;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        if (!r->registrations[i].occupied)
        {
            struct as_registration *slot = r->registrations + i;
            *slot = (struct as_registration){r, cb, ctx, release, {0, 0}, 1, 1};
            enum api_status status =
                api_add(&r->core, dispatch, slot, drop, &slot->handle);
            if (status != API_OK)
            {
                slot->occupied = 0;
                return status;
            }
            *out = slot->handle;
            return API_OK;
        }
    }
    return API_FULL;
}

enum api_status as_remove(struct as_registry *r, struct api_handle h)
{
    if (!r)
    {
        return API_INVALID;
    }
    enum api_status result = api_remove(&r->core, h);
    if (result == API_PENDING && r->suppress)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            if (r->registrations[i].occupied &&
                r->registrations[i].handle.index == h.index &&
                r->registrations[i].handle.generation == h.generation)
            {
                r->registrations[i].enabled = 0;
            }
        }
    }
    return result;
}

static void terminal_if_closed(struct as_registry *r)
{
    if (r->core.closed && r->terminal)
    {
        as_terminal_fn terminal = r->terminal;
        void *ctx = r->terminal_context;
        r->terminal = 0;
        r->terminal_context = 0;
        terminal(ctx,
                 r); /* Terminal receiver can free r. No access afterward. */
    }
}

enum api_status as_dispatch(struct as_registry *r)
{
    if (!r)
    {
        return API_INVALID;
    }
    enum api_status status = api_dispatch(&r->core);
    terminal_if_closed(r);
    return status;
}

enum api_status as_destroy(struct as_registry *r)
{
    if (!r)
    {
        return API_INVALID;
    }
    enum api_status status = api_destroy(&r->core);
    terminal_if_closed(r);
    return status;
}

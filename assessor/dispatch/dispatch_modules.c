#include "dispatch_modules.h"
#ifndef DS_MODULE_MUTANT
#define DS_MODULE_MUTANT 0
#endif
static void drop_share(void *p)
{
    (void)ow_shared_drop(p);
}

static void close_module(void *p)
{
    ds_module_close(p);
}

void ds_registry_init(struct ds_module_registry *r)
{
    for (unsigned i = 0; i < 2; ++i)
        r->entries[i] = (struct ds_module_entry){.generation = 1};
}

static struct ds_module_entry *entry(struct ds_module_lease l)
{
    if (!l.owner || l.slot >= 2)
        return NULL;
    struct ds_module_entry *e = &l.owner->entries[l.slot];
    if (!e->live || (DS_MODULE_MUTANT != 4 && e->generation != l.generation))
        return NULL;
    return e;
}

int ds_lease_open(struct ds_module_registry *r, struct ds_module_services s,
                  struct ow_shared *read_owner, struct ow_shared *io_owner,
                  unsigned cut, struct ds_module_lease *out)
{
    if (!r || !out || !s.input.read || !s.clock.now || !s.sink.write ||
        !read_owner || !io_owner || cut > 4)
        return DS_INVALID;
    unsigned slot = 0;
    while (slot < 2 &&
           (r->entries[slot].live || r->entries[slot].generation > 3))
        ++slot;
    if (slot == 2)
        return DS_FULL;
    if (cut == 1)
        return DS_FAILED;
    struct ow_scope scope = {0};
    if (ow_shared_retain(read_owner) != OW_OK)
        return DS_FAILED;
    if (ow_defer(&scope, drop_share, read_owner, OW_ON_ERROR) != OW_OK)
    {
        drop_share(read_owner);
        return DS_FAILED;
    }
    if (cut == 2)
        goto failed;
    if (ow_shared_retain(io_owner) != OW_OK)
        goto failed;
    if (ow_defer(&scope, drop_share, io_owner, OW_ON_ERROR) != OW_OK)
    {
        drop_share(io_owner);
        goto failed;
    }
    if (cut == 3)
        goto failed;
    struct ds_module *m = NULL;
    int rc = ds_module_open(s.input, &m);
    if (rc != DS_OK)
    {
        (void)ow_finish(&scope, 0);
        return rc;
    }
    if (ow_defer(&scope, close_module, m, OW_ON_ERROR) != OW_OK)
    {
        ds_module_close(m);
        goto failed;
    }
    if (cut == 4)
        goto failed;
    unsigned generation = r->entries[slot].generation;
    r->entries[slot] =
        (struct ds_module_entry){m, s, read_owner, io_owner, generation, 1};
    (void)ow_finish(&scope, 1);
    *out = (struct ds_module_lease){r, slot, generation};
    return DS_OK;
failed:
    if (DS_MODULE_MUTANT == 3)
    {
        (void)ow_finish(&scope, 1);
        return DS_FAILED;
    }
    (void)ow_finish(&scope, 0);
    return DS_FAILED;
}

int ds_lease_twice(struct ds_module_lease l, int *out)
{
    if (!out)
        return DS_INVALID;
    struct ds_module_entry *e = entry(l);
    if (!e)
        return DS_STALE;
    return ds_module_twice(e->module, out);
}

static int clock_only(struct ds_clock clock, int *out)
{
    int temporary = 0;
    int rc = clock.now(clock.context, &temporary);
    if (rc == DS_OK)
        *out = temporary;
    return rc;
}

static int sink_only(struct ds_sink sink, int value)
{
    return sink.write(sink.context, value);
}

int ds_lease_stamp(struct ds_module_lease l, int *out)
{
    if (!out)
        return DS_INVALID;
    struct ds_module_entry *e = entry(l);
    if (!e)
        return DS_STALE;
    return clock_only(e->services.clock, out);
}

int ds_lease_emit(struct ds_module_lease l, int value)
{
    struct ds_module_entry *e = entry(l);
    if (!e)
        return DS_STALE;
    return sink_only(e->services.sink, value);
}

int ds_lease_close(struct ds_module_lease l)
{
    struct ds_module_entry *e = entry(l);
    if (!e)
        return DS_STALE;
    struct ds_module *m = e->module;
    struct ow_shared *read = e->read_owner, *io = e->io_owner;
    unsigned generation = e->generation + 1;
    *e = (struct ds_module_entry){.generation = generation};
    ds_module_close(m);
    drop_share(io);
    drop_share(read);
    return DS_OK;
}

int ds_user_put(struct ds_user_map *m, struct ds_user_key k,
                struct ds_user_value v)
{
    if (!m || k.id >= 4)
        return DS_INVALID;
    m->values[k.id] = v;
    m->live[k.id] = 1;
    return DS_OK;
}

int ds_user_get(const struct ds_user_map *m, struct ds_user_key k,
                struct ds_user_value *out)
{
    if (!m || !out || k.id >= 4)
        return DS_INVALID;
    if (!m->live[k.id])
        return DS_EMPTY;
    *out = m->values[k.id];
    return DS_OK;
}

int ds_metric_put(struct ds_metric_map *m, struct ds_metric_key k,
                  struct ds_metric_value v)
{
    if (!m || k.slot >= 4)
        return DS_INVALID;
    m->values[k.slot] = v;
    m->live[k.slot] = 1;
    return DS_OK;
}

int ds_metric_get(const struct ds_metric_map *m, struct ds_metric_key k,
                  struct ds_metric_value *out)
{
    if (!m || !out || k.slot >= 4)
        return DS_INVALID;
    if (!m->live[k.slot])
        return DS_EMPTY;
    *out = m->values[k.slot];
    return DS_OK;
}

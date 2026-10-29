#include "dispatch.h"
#include <dlfcn.h>
#include <limits.h>

static int sum(int a, int b, int *out)
{
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return DS_RANGE;
    *out = a + b;
    return DS_OK;
}

int ds_measure(struct ds_view v, int *out)
{
    int value, status;
    if (!out || !v.object || !v.ops || !v.ops->measure)
        return DS_INVALID;
    status = v.ops->measure(v.object, &value);
#ifdef DS_MUTATE_TRAITS1
    if (status != DS_OK)
    {
        *out = value;
        return DS_OK;
    }
#else
    if (status != DS_OK)
        return status;
#endif

#ifdef DS_MUTATE_TRAITS2
    *out = value + 1;
#else
    *out = value;
#endif
    return DS_OK;
}

int ds_embedded(const struct ds_box *b, int *out)
{
    if (!b)
        return DS_INVALID;
#ifdef DS_MUTATE_FAT1
    if (!out)
        return DS_INVALID;
    *out = b->value;
    return DS_OK;
#else

#ifdef DS_MUTATE_FAT2
    int result = ds_measure((struct ds_view){&b->value, b->ops}, out);
    if (result == DS_OK)
        *out = 0;
    return result;
#else
    return ds_measure((struct ds_view){&b->value, b->ops}, out);
#endif

#endif
}

int ds_fold(const int *p, size_t n, int initial, ds_combine_fn operation,
            int *out)
{
    int value = initial, next, status;
    size_t i;
    if (!out || !operation || (!p && n) || n > 16)
        return DS_INVALID;
    for (i = 0; i < n; i++)
    {
#ifdef DS_MUTATE_DICTIONARY1
        status = operation(p[i], value, &next);
#else
        status = operation(value, p[i], &next);
#endif

#ifdef DS_MUTATE_DICTIONARY2
        if (status != DS_OK)
        {
            *out = value;
            return status;
        }
#else
        if (status != DS_OK)
            return status;
#endif
        value = next;
    }
    *out = value;
    return DS_OK;
}

int ds_sum(const int *p, size_t n, int initial, int *out)
{
    int value = initial, next, status;
    size_t i;
    if (!out || (!p && n) || n > 16)
        return DS_INVALID;
    for (i = 0; i < n; i++)
    {
        status = sum(value, p[i], &next);
        if (status != DS_OK)
            return status;
        value = next;
    }
    *out = value;
    return DS_OK;
}

int ds_closure_init(struct ds_closure *c, int capture, enum ds_mode mode)
{
    if (!c || mode < DS_SHARED || mode > DS_ONCE)
        return DS_INVALID;
    *c = (struct ds_closure){capture, mode, 1};
    return DS_OK;
}

int ds_closure_call(struct ds_closure *c, int arg, int *out)
{
    int value, status;
    if (!c || !out)
        return DS_INVALID;
    if (!c->active)
        return DS_EMPTY;
#ifdef DS_MUTATE_CLOSURES2
/* Incorrectly consumes only a successful once call. */
#else
    if (c->mode == DS_ONCE)
        c->active = 0;
#endif
    status = sum(c->capture, arg, &value);
    if (status != DS_OK)
        return status;
#ifdef DS_MUTATE_CLOSURES1
    c->capture = value;
#else
    if (c->mode == DS_MUTABLE)
        c->capture = value;
#endif

#ifdef DS_MUTATE_CLOSURES2
    if (c->mode == DS_ONCE)
        c->active = 0;
    *out = value;
#else
    *out = value;
#endif
    return DS_OK;
}

void ds_erased_init(struct ds_erased *e, struct d_allocator a)
{
    d_small_init(&e->capture, a);
    e->call = 0;
    e->destroy = 0;
    e->resource = 0;
    e->active = 0;
}

int ds_erased_assign(struct ds_erased *e, const unsigned char *p, size_t n,
                     ds_thunk_fn call, void (*destroy)(void *), void *resource)
{
    int status;
    if (!e || !call || e->active || (!p && n))
        return DS_INVALID;
    status = d_small_append(&e->capture, p, n);
    if (status != D_OK)
        return status == D_FULL ? DS_FULL : DS_FAILED;
    e->call = call;
    e->destroy = destroy;
    e->resource = resource;
    e->active = 1;
    return DS_OK;
}

int ds_erased_call(const struct ds_erased *e, int arg, int *out)
{
    int value, status;
    if (!e || !out)
        return DS_INVALID;
    if (!e->active)
        return DS_EMPTY;
    status = e->call(d_small_data(&e->capture), e->capture.length, arg, &value);
    if (status != DS_OK)
        return status;
    *out = value;
    return DS_OK;
}

int ds_erased_move(struct ds_erased *d, struct ds_erased *s)
{
    if (!d || !s || d == s || d->active || !s->active)
        return DS_INVALID;
    if (d_small_move(&d->capture, &s->capture) != D_OK)
        return DS_INVALID;
    d->call = s->call;
    d->destroy = s->destroy;
    d->resource = s->resource;
    d->active = 1;
#ifdef DS_MUTATE_ERASED2
    s->active = 1;
#else
    s->active = 0;
#endif
    s->call = 0;
    s->destroy = 0;
    s->resource = 0;
    return DS_OK;
}

void ds_erased_drop(struct ds_erased *e)
{
#ifdef DS_MUTATE_ERASED1
/* leaked logical resource: destroy omitted */
#else
    if (e->active && e->destroy)
        e->destroy(e->resource);
#endif
    d_small_reset(&e->capture);
    e->active = 0;
    e->call = 0;
    e->destroy = 0;
    e->resource = 0;
}

void ds_methods_init(struct ds_methods *m)
{
    unsigned i, j;
    m->epoch = 1;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            m->entries[i][j] = 0;
}

int ds_methods_set(struct ds_methods *m, unsigned left, unsigned right,
                   ds_combine_fn method)
{
    if (!m || left >= 3 || right >= 3)
        return DS_INVALID;
    if (m->epoch == 3)
        return DS_FULL;
    m->entries[left][right] = method;
    m->epoch++;
    return DS_OK;
}

int ds_dispatch(const struct ds_methods *m, struct ds_cache *c, unsigned left,
                unsigned right, int a, int b, int *out)
{
    ds_combine_fn f;
    int value, status;
    if (!m || !c || !out || left >= 3 || right >= 3)
        return DS_INVALID;
#ifdef DS_MUTATE_MULTIPLE2
    if (left > right)
    {
        unsigned t = left;
        left = right;
        right = t;
    }
#endif

#ifdef DS_MUTATE_MULTIPLE1
    if (c->epoch && c->left == left && c->right == right)
#else
    if (c->epoch == m->epoch && c->left == left && c->right == right)
#endif
        f = c->method;
    else
    {
        f = m->entries[left][right];
        *c = (struct ds_cache){m->epoch, left, right, f};
    }
    if (!f)
        return DS_UNSUPPORTED;
    status = f(a, b, &value);
    if (status != DS_OK)
        return status;
    *out = value;
    return DS_OK;
}

void ds_list_init(struct ds_list *l)
{
    l->head = (struct ds_link){&l->head, &l->head, l};
    l->count = 0;
}

void ds_item_init(struct ds_item *i, int value)
{
    i->link = (struct ds_link){0, 0, 0};
    i->value = value;
}

int ds_list_push(struct ds_list *l, struct ds_item *i)
{
    struct ds_link *last;
#ifdef DS_MUTATE_INTRUSIVE2
    if (!l || !i)
        return DS_INVALID;
    if (i->link.owner)
        return DS_OK;
#else
    if (!l || !i || i->link.owner)
        return DS_INVALID;
#endif
    if (l->count == 4)
        return DS_FULL;
    last = l->head.previous;
    i->link = (struct ds_link){last, &l->head, l};
    last->next = &i->link;
    l->head.previous = &i->link;
    l->count++;
    return DS_OK;
}

int ds_list_remove(struct ds_list *l, struct ds_item *i)
{
    if (!l || !i || i->link.owner != l)
        return DS_INVALID;
    i->link.previous->next = i->link.next;
#ifdef DS_MUTATE_INTRUSIVE1
/* missing inverse link repair */
#else
    i->link.next->previous = i->link.previous;
#endif
    i->link = (struct ds_link){0, 0, 0};
    l->count--;
    return DS_OK;
}

int ds_list_sum(const struct ds_list *l, int *out)
{
    const struct ds_link *p;
    int value = 0, next, status;
    if (!l || !out)
        return DS_INVALID;
    for (p = l->head.next; p != &l->head; p = p->next)
    {
        const struct ds_item *item = (const struct ds_item *)p;
        status = sum(value, item->value, &next);
        if (status != DS_OK)
            return status;
        value = next;
    }
    *out = value;
    return DS_OK;
}

struct ds_module
{
    unsigned live;
    struct ds_service service;
};
static struct ds_module modules[2];

int ds_module_open(struct ds_service s, struct ds_module **out)
{
    unsigned i;
    if (!out || !s.read)
        return DS_INVALID;
    for (i = 0; i < 2; i++)
        if (!modules[i].live)
        {
            modules[i] = (struct ds_module){1, s};
            *out = &modules[i];
            return DS_OK;
        }
    return DS_FULL;
}

int ds_module_twice(struct ds_module *m, int *out)
{
    int a, b, value, status;
    if (!m || !out || !m->live)
        return DS_INVALID;
    status = m->service.read(m->service.context, &a);
    if (status != DS_OK)
        return status;
#ifdef DS_MUTATE_MODULES1
    b = a;
    status = DS_OK;
#else
    status = m->service.read(m->service.context, &b);
#endif

#ifdef DS_MUTATE_MODULES2
    if (status != DS_OK)
    {
        *out = a;
        return status;
    }
#else
    if (status != DS_OK)
        return status;
#endif
    status = sum(a, b, &value);
    if (status != DS_OK)
        return status;
    *out = value;
    return DS_OK;
}

void ds_module_close(struct ds_module *m)
{
    m->live = 0;
    m->service = (struct ds_service){0, 0};
}

void ds_authority_init(struct ds_authority *a, unsigned rights)
{
    a->rights = rights & 3;
    a->epoch = 1;
    a->counter = 0;
}

int ds_cap_issue(const struct ds_authority *a, unsigned rights,
                 struct ds_capability *out)
{
    if (!a || !out || rights > 3)
        return DS_INVALID;
    if (a->epoch > 3 || (rights & ~a->rights))
        return DS_DENIED;
    *out = (struct ds_capability){a, rights, a->epoch};
    return DS_OK;
}

int ds_cap_attenuate(struct ds_capability c, unsigned rights,
                     struct ds_capability *out)
{
    if (!out || !c.owner || rights > 3)
        return DS_INVALID;
#ifdef DS_MUTATE_CAPABILITIES1
/* incorrectly permits rights amplification */
#else
    if (rights & ~c.rights)
        return DS_DENIED;
#endif
    *out = (struct ds_capability){c.owner, rights, c.epoch};
    return DS_OK;
}

int ds_cap_add(struct ds_authority *a, struct ds_capability c, int amount)
{
    int value, status;
    if (!a)
        return DS_INVALID;
#ifdef DS_MUTATE_CAPABILITIES2
    if (c.owner != a || a->epoch > 3)
        return DS_STALE;
#else
    if (c.owner != a || c.epoch != a->epoch || a->epoch > 3)
        return DS_STALE;
#endif
    if (!(c.rights & 2) || !(a->rights & 2))
        return DS_DENIED;
    status = sum(a->counter, amount, &value);
    if (status != DS_OK)
        return status;
    a->counter = value;
    return DS_OK;
}

int ds_cap_revoke(struct ds_authority *a)
{
    if (!a)
        return DS_INVALID;
    if (a->epoch > 3)
        return DS_FULL;
    a->epoch++;
    return DS_OK;
}

void ds_loaded_init(struct ds_loaded *m)
{
    *m = (struct ds_loaded){0, 0, 0};
}

int ds_loaded_open(struct ds_loaded *m, const char *path)
{
    void *h;
    const struct ds_plugin *api;
    const char *error;
    if (!m || !path || m->handle)
        return DS_INVALID;
    h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h)
        return DS_FAILED;
    dlerror();
    api = dlsym(h, "dispatch_plugin");
    error = dlerror();
    if (error || !api ||
#ifdef DS_MUTATE_PLUGIN1
        0
#else
        api->abi != 1
#endif
        || api->size != sizeof(*api) || !api->apply || !api->migrate)
    {
        dlclose(h);
        return DS_INVALID;
    }
    *m = (struct ds_loaded){h, api, 0};
    return DS_OK;
}

int ds_loaded_acquire(struct ds_loaded *m, const struct ds_plugin **out)
{
    if (!m || !out)
        return DS_INVALID;
    if (!m->handle)
        return DS_EMPTY;
    if (m->borrows == 4)
        return DS_FULL;
    m->borrows++;
    *out = m->api;
    return DS_OK;
}

int ds_loaded_release(struct ds_loaded *m)
{
    if (!m || !m->handle || !m->borrows)
        return DS_INVALID;
    m->borrows--;
    return DS_OK;
}

int ds_loaded_close(struct ds_loaded *m)
{
    if (!m)
        return DS_INVALID;
#ifdef DS_MUTATE_PLUGIN2
/* unloads while a client still borrows callbacks */
#else
    if (m->borrows)
        return DS_BUSY;
#endif
    if (!m->handle)
        return DS_OK;
    if (dlclose(m->handle))
        return DS_FAILED;
    ds_loaded_init(m);
    return DS_OK;
}

void ds_host_init(struct ds_host *h)
{
    ds_loaded_init(&h->current);
    ds_loaded_init(&h->retired);
    h->state = 0;
    h->generation = 0;
}

int ds_host_reload(struct ds_host *h, struct ds_loaded *candidate)
{
    int state, status;
    unsigned old_schema;
    if (!h || !candidate || !candidate->handle || candidate == &h->current ||
        candidate == &h->retired)
        return DS_INVALID;
    if (h->current.borrows || candidate->borrows || h->retired.handle)
        return DS_BUSY;
    if (h->generation == 3)
        return DS_FULL;
    old_schema = h->current.handle ? h->current.api->schema : 0;
    status = candidate->api->migrate(old_schema, h->state, &state);
#ifdef DS_MUTATE_RELOAD2
    if (status != DS_OK)
    {
        h->state = 0;
        return status;
    }
#else
    if (status != DS_OK)
        return status;
#endif
    h->retired = h->current;
    h->current = *candidate;
#ifdef DS_MUTATE_RELOAD1
/* duplicate ownership after transfer */
#else
    ds_loaded_init(candidate);
#endif
    h->state = state;
    h->generation++;
    return DS_OK;
}

int ds_host_collect(struct ds_host *h)
{
    if (!h)
        return DS_INVALID;
    return ds_loaded_close(&h->retired);
}

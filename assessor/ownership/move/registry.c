#include "move.h"
#ifndef OMM_MUTATION
#define OMM_MUTATION 0
#endif
struct ow_slot
{
    struct omm_value value;
    uint32_t generation;
    unsigned active, retired;
};

struct omm_registry
{
    struct ow_slot slot[2];
    struct omm_memory memory;
    struct v4_allocator cells;
};

static int empty(struct omm_token t)
{
    return !t.owner && !t.slot && !t.generation;
}

static int valid(struct omm_registry *r, struct omm_token t)
{
    return r && t.owner == r && t.slot < 2 && r->slot[t.slot].active &&
           (OMM_MUTATION == 4 || r->slot[t.slot].generation == t.generation);
}

enum ow_status omm_registry_create(struct omm_memory m, struct v4_allocator a,
                                   uint32_t seed, struct omm_registry **out)
{
    if (!out || *out || !m.take || !m.release || !a.take || !a.release)
        return OW_INVALID;
    struct omm_registry *r = m.take(m.context, sizeof(*r));
    if (!r)
        return OW_NOMEM;
    *r = (struct omm_registry){0};
    r->memory = m;
    r->cells = a;
    for (unsigned i = 0; i < 2; ++i)
    {
        r->slot[i].generation = seed;
        r->slot[i].retired = seed == UINT32_MAX;
    }
    *out = r;
    return OW_OK;
}

enum ow_status omm_registry_destroy(struct omm_registry **owner)
{
    if (!owner)
        return OW_INVALID;
    struct omm_registry *r = *owner;
    if (!r)
        return OW_OK;
    for (unsigned i = 0; i < 2; ++i)
        if (r->slot[i].active)
            return OW_BUSY;
    struct omm_memory m = r->memory;
    *owner = NULL;
    m.release(m.context, r);
    return OW_OK;
}

enum ow_status omm_token_open(struct omm_registry *r, const int32_t *p,
                              size_t n, struct omm_token *out)
{
    if (!r || !out || !empty(*out) || !p || !n || n > 8)
        return OW_INVALID;
    unsigned at = 2;
    for (unsigned i = 0; i < 2; ++i)
        if (!r->slot[i].active && !r->slot[i].retired)
        {
            at = i;
            break;
        }
    if (at == 2)
        return OW_FULL;
    struct ow_slot *s = &r->slot[at];
    enum ow_status code = omm_init(&s->value, r->cells, p, n);
    if (code != OW_OK)
        return code;
    ++s->generation;
    s->active = 1;
    *out = (struct omm_token){r, at, s->generation};
    return OW_OK;
}

enum ow_status omm_token_move(struct omm_registry *r, struct omm_token *dst,
                              struct omm_token *src)
{
    if (!dst || !src || dst == src || !empty(*dst))
        return OW_INVALID;
    if (!valid(r, *src))
        return OW_STALE;
    struct ow_slot *s = &r->slot[src->slot];
    if (s->generation == UINT32_MAX)
        return OW_RETIRED;
    if (OMM_MUTATION != 3)
        ++s->generation;
    *dst = (struct omm_token){r, src->slot, s->generation};
    *src = (struct omm_token){0};
    return OW_OK;
}

enum ow_status omm_token_read(struct omm_registry *r, struct omm_token t,
                              size_t i, int32_t *out)
{
    if (!valid(r, t))
        return OW_STALE;
    return omm_read(&r->slot[t.slot].value, i, out);
}

enum ow_status omm_token_consume(struct omm_registry *r, struct omm_token *t)
{
    if (!t)
        return OW_INVALID;
    if (!valid(r, *t))
        return OW_STALE;
    struct ow_slot *s = &r->slot[t->slot];
    enum ow_status status = omm_consume(&s->value);
    if (status != OW_OK)
        return status;
    s->active = 0;
    if (s->generation == UINT32_MAX)
        s->retired = 1;
    *t = (struct omm_token){0};
    return OW_OK;
}

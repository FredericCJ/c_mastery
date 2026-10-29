#include "frontier.h"
#ifndef OCF_MUTATION
#define OCF_MUTATION 0
#endif
static const unsigned prerequisites[4] = {0, 1, 1, 2};

static int valid(const struct ocf_plan *p)
{
    unsigned seen = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        unsigned id = p->order[i];
        if (id >= 4 || (seen & (1u << id)))
            return 0;
        unsigned required =
            OCF_MUTATION == 4 && id == 3 ? 0 : prerequisites[id];
        if ((seen & required) != required)
            return 0;
        seen |= 1u << id;
        if (!p->length[id] || p->length[id] > SIZE_MAX / sizeof(int32_t))
            return 0;
    }
    return 1;
}

static struct ocf_view view(const struct ocf_owner *o)
{
    struct ocf_view v = {
        {NULL, NULL, NULL, NULL}, {0, 0, 0, 0}, o->initialized};
    for (unsigned i = 0; i < 4; ++i)
        if (o->initialized & (1u << i))
        {
            v.block[i] = o->block[i];
            v.length[i] = o->length[i];
        }
    return v;
}

static void cleanup(struct ocf_owner *o)
{
    while (o->count)
    {
        unsigned id = o->order[--o->count];
        if (OCF_MUTATION == 2)
        {
            id = o->order[0];
            for (unsigned i = 0; i < o->count; ++i)
                o->order[i] = o->order[i + 1];
        }
        unsigned initialized = o->initialized & (1u << id);
        if (OCF_MUTATION != 9)
            o->initialized &= ~(1u << id);
        if (initialized)
        {
            struct ocf_view v = view(o);
            o->callbacks.destroy(o->callbacks.context, id, o->block[id],
                                 o->length[id], &v);
        }
        o->initialized &= ~(1u << id);
        o->acquired &= ~(1u << id);
        if (OCF_MUTATION != 5 || initialized)
            o->allocator.release(o->allocator.context, o->block[id]);
        o->block[id] = NULL;
        o->length[id] = 0;
    }
}

enum ocf_status ocf_construct(struct ocf_owner *out, struct v4_allocator a,
                              struct ocf_callbacks c, const struct ocf_plan *p)
{
    if (!out || !p || out->count || out->acquired || out->initialized ||
        !a.take || !a.release || !c.initialize || !c.destroy || !valid(p))
        return OCF_INVALID;
    struct ocf_owner staged = {0};
    staged.allocator = a;
    staged.callbacks = c;
    for (unsigned i = 0; i < 4; ++i)
    {
        unsigned id = p->order[i];
        int32_t *block = a.take(a.context, p->length[id]);
        if (!block)
        {
            if (OCF_MUTATION == 3)
                *out = staged;
            cleanup(&staged);
            return OCF_NOMEM;
        }
        staged.block[id] = block;
        staged.length[id] = p->length[id];
        staged.order[staged.count++] = id;
        staged.acquired |= 1u << id;
        struct ocf_view v = view(&staged);
        int result = c.initialize(c.context, id, block, p->length[id], &v);
        if (OCF_MUTATION == 1)
            staged.initialized |= 1u << id;
        if (result)
        {
            if (OCF_MUTATION == 3)
                *out = staged;
            cleanup(&staged);
            return OCF_INIT_FAILED;
        }
        if (OCF_MUTATION != 6 || id != 2)
            staged.initialized |= 1u << id;
    }
    *out = staged;
    return OCF_OK;
}

enum ocf_status ocf_destroy(struct ocf_owner *o)
{
    if (!o)
        return OCF_INVALID;
    cleanup(o);
    return OCF_OK;
}

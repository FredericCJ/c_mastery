#include "expression_follow.h"
#include <limits.h>
#ifndef CE_MUTANT
#define CE_MUTANT 0
#endif
static void cleanup(struct ce_frame *f)
{
    for (unsigned i = 0; i < f->depth; i++)
        if (f->stack[i].left)
        {
            if (CE_MUTANT != 1)
                f->allocator.release(f->allocator.context, f->stack[i].left);
            f->stack[i].left = NULL;
        }
    f->depth = 0;
}

int ce_init(struct ce_frame *f, const struct ce_node *n, size_t count,
            struct co_allocator a, ce_deliver_fn deliver, void *ctx)
{
    if (!f || !n || !a.allocate || !a.release)
        return CO_INVALID;
    if (!count || count > 16)
        return CO_RANGE;
    for (size_t i = 0; i < count; i++)
        if (n[i].kind > CE_MULTIPLY ||
            (n[i].kind != CE_LITERAL &&
             (n[i].left <= i || n[i].right <= i || n[i].left >= count ||
              n[i].right >= count)))
            return CO_INVALID;
    *f = (struct ce_frame){.nodes = n,
                           .count = count,
                           .allocator = a,
                           .depth = 1,
                           .max_depth = 1,
                           .deliver = deliver,
                           .context = ctx};
    return CO_OK;
}

static int fail(struct ce_frame *f, int error)
{
    cleanup(f);
    f->state = 3;
    return error;
}

int ce_step(struct ce_frame *f, unsigned budget, int32_t *out)
{
    if (!f || !out)
        return CO_INVALID;
    if (f->state == 2)
        return CO_DONE;
    if (f->state == 3)
        return CO_STATE;
    while (budget--)
    {
        f->steps++;
        if (!f->depth)
        {
            int r = f->deliver ? f->deliver(f->context, f->result) : CO_OK;
            if (r == CO_PENDING)
            {
                f->state = 1;
                return CO_PENDING;
            }
            if (r)
                return fail(f, CO_IO);
            f->state = 2;
            *out = f->result;
            return CO_DONE;
        }
        struct ce_continuation *c = &f->stack[f->depth - 1];
        const struct ce_node *n = &f->nodes[c->node];
        if (n->kind == CE_LITERAL)
        {
            f->result = n->value;
            f->have = 1;
            f->depth--;
            continue;
        }
        if (c->phase == 0)
        {
            c->phase = 1;
            f->stack[f->depth++] = (struct ce_continuation){n->left, 0, NULL};
            f->have = 0;
        }
        else if (c->phase == 1)
        {
            if (!f->have)
                return fail(f, CO_STATE);
            int32_t *p = f->allocator.allocate(f->allocator.context, sizeof *p);
            if (!p)
                return fail(f, CO_RANGE);
            *p = f->result;
            c->left = p;
            c->phase = 2;
            f->stack[f->depth++] = (struct ce_continuation){n->right, 0, NULL};
            f->have = 0;
        }
        else
        {
            if (!f->have || !c->left)
                return fail(f, CO_STATE);
            int64_t result = n->kind == CE_ADD ? (int64_t)*c->left + f->result
                                               : (int64_t)*c->left * f->result;
            if (CE_MUTANT != 2 && (result < INT32_MIN || result > INT32_MAX))
                return fail(f, CO_RANGE);
            f->allocator.release(f->allocator.context, c->left);
            c->left = NULL;
            f->result = (int32_t)result;
            f->depth--;
            f->have = 1;
        }
        if (f->depth > f->max_depth)
            f->max_depth = f->depth;
    }
    return CO_PENDING;
}

int ce_cancel(struct ce_frame *f)
{
    if (!f)
        return CO_INVALID;
    if (f->state == 2 || f->state == 3)
        return CO_OK;
    cleanup(f);
    f->state = 3;
    return CO_OK;
}

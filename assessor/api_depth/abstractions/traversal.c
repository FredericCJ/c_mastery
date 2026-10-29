#include "traversal.h"
#include <limits.h>

enum at_result at_begin(const struct ac_epoch_owner *owner, unsigned limit,
                        struct at_cursor *out)
{
    if (!owner || !owner->live || !out || limit > 255 ||
        !owner->bytes.generation)
        return AT_INVALID;
    struct f_view view;
    if (f_view(&owner->bytes, 0, owner->bytes.length, &view) != F_OK)
        return AT_INVALID;
    *out = (struct at_cursor){view, owner, 0, limit, AT_ITEM};
    return AT_ITEM;
}

enum at_result at_next(struct at_cursor *cursor, unsigned char *out)
{
    if (!cursor || !out)
        return AT_INVALID;
    if (cursor->terminal != AT_ITEM)
        return cursor->terminal;
    /* Owner control remains live while the cursor is active. */
    int dead = !cursor->owner->live;
#ifdef AT_MUTANT_LIFETIME
    dead = 0;
#endif
    if (dead || cursor->view.owner->generation != cursor->view.generation)
    {
        cursor->terminal = AT_STALE;
        return AT_STALE;
    }
    if (cursor->position == cursor->view.length)
    {
        cursor->terminal = AT_END;
        return AT_END;
    }
    uint8_t value;
    if (f_view_read(cursor->view, cursor->position, &value) != F_OK)
    {
        cursor->terminal = AT_STALE;
        return AT_STALE;
    }
#ifdef AT_MUTANT_STOP
    if (value >= cursor->limit)
    {
        cursor->terminal = AT_STOP;
        return AT_STOP;
    }
#else
    if (value > cursor->limit)
    {
        cursor->terminal = AT_STOP;
        return AT_STOP;
    }
#endif
    ++cursor->position;
    *out = value;
    return AT_ITEM;
}

enum at_result at_push(struct at_push *host, const struct ac_epoch_owner *owner,
                       unsigned limit, at_visitor_fn visitor, void *context)
{
    if (!host || !visitor)
        return AT_INVALID;
    if (host->depth >= 3)
        return AT_FULL;
    if (!host->depth)
    {
        host->visits = 0;
        host->max_depth = 0;
    }
    struct at_cursor cursor;
    enum at_result result = at_begin(owner, limit, &cursor);
    if (result != AT_ITEM)
        return result;
    ++host->depth;
    if (host->depth > host->max_depth)
        host->max_depth = host->depth;
    unsigned char value;
    while ((result = at_next(&cursor, &value)) == AT_ITEM)
    {
        if (host->visits == UINT_MAX)
        {
            result = AT_FULL;
            break;
        }
        ++host->visits;
        enum at_action action = visitor(context, value);
        if (action != AT_CONTINUE)
        {
            result = action == AT_STOP_NOW ? AT_STOP
                     : action == AT_REMOVE ? AT_DENIED
                                           : AT_INVALID;
            break;
        }
    }
    --host->depth;
    return result;
}

enum at_result at_defer(struct at_deferred *d,
                        const struct ac_epoch_owner *owner, unsigned limit,
                        at_visitor_fn visitor, void *ctx)
{
    if (!d || !visitor || d->active || d->running)
        return AT_INVALID;
    struct at_cursor cursor;
    enum at_result result = at_begin(owner, limit, &cursor);
    if (result != AT_ITEM)
        return result;
    *d = (struct at_deferred){cursor, visitor, ctx, 1, 0, AT_ITEM};
    return AT_ITEM;
}

static enum at_result finish(struct at_deferred *d, enum at_result result)
{
    d->active = 0;
    d->visitor = 0;
    d->context = 0;
    d->terminal = result;
    return result;
}

enum at_result at_pump(struct at_deferred *d, unsigned budget)
{
    if (!d || d->running)
        return AT_INVALID;
    if (!d->active)
        return d->terminal == AT_ITEM ? AT_INVALID : d->terminal;
    d->running = 1;
    enum at_result result = AT_ITEM;
    while (budget--)
    {
        unsigned char value;
        result = at_next(&d->cursor, &value);
        if (result != AT_ITEM)
            break;
        enum at_action action = d->visitor(d->context, value);
        if (action != AT_CONTINUE)
        {
            result = action == AT_STOP_NOW ? AT_STOP
                     : action == AT_REMOVE ? AT_DENIED
                                           : AT_INVALID;
            break;
        }
    }
    d->running = 0;
    return result == AT_ITEM ? AT_ITEM : finish(d, result);
}

enum at_result at_cancel(struct at_deferred *d)
{
    if (!d || d->running)
        return AT_INVALID;
    if (!d->active)
        return d->terminal == AT_ITEM ? AT_INVALID : d->terminal;
#ifdef AT_MUTANT_CANCEL
    return AT_CANCELLED;
#else
    return finish(d, AT_CANCELLED);
#endif
}

static enum at_result walk(const struct at_node *nodes, size_t count,
                           unsigned node, unsigned depth, unsigned limit,
                           unsigned char *out, size_t capacity, size_t *n)
{
    if (node == 0)
        return AT_END;
    if (node > count)
        return AT_INVALID;
    if (depth == limit || *n == capacity)
        return AT_FULL;
    const struct at_node *p = nodes + node - 1;
    out[(*n)++] = p->value;
    enum at_result result =
        walk(nodes, count, p->left, depth + 1, limit, out, capacity, n);
    if (result != AT_END)
        return result;
    return walk(nodes, count, p->right, depth + 1, limit, out, capacity, n);
}

enum at_result at_recursive(const struct at_node *nodes, size_t count,
                            unsigned root, unsigned limit, unsigned char *out,
                            size_t capacity, size_t *written)
{
    if ((count && !nodes) || count > 7 || limit > 3 || (capacity && !out) ||
        !written)
        return AT_INVALID;
    *written = 0;
    return walk(nodes, count, root, 0, limit, out, capacity, written);
}

enum at_result at_continuations(const struct at_node *nodes, size_t count,
                                unsigned root, unsigned limit,
                                unsigned char *out, size_t capacity,
                                size_t *written)
{
    struct ac_frame
    {
        unsigned node, stage;
    };
    struct ac_frame stack[3];
    unsigned top = 0;
    if ((count && !nodes) || count > 7 || limit > 3 || (capacity && !out) ||
        !written)
        return AT_INVALID;
    *written = 0;
    if (!root)
        return AT_END;
    if (root > count)
        return AT_INVALID;
    if (!limit)
        return AT_FULL;
    stack[top++] = (struct ac_frame){root, 0};
    while (top)
    {
        struct ac_frame *frame = stack + top - 1;
        const struct at_node *node = nodes + frame->node - 1;
        if (!frame->stage)
        {
            if (*written == capacity)
                return AT_FULL;
            out[(*written)++] = node->value;
            frame->stage = 1;
        }
        else if (frame->stage == 3)
            --top;
        else
        {
            unsigned child = frame->stage++ == 1 ? node->left : node->right;
            if (!child)
                continue;
            if (child > count)
                return AT_INVALID;
            if (top == limit)
                return AT_FULL;
            stack[top++] = (struct ac_frame){child, 0};
        }
    }
    return AT_END;
}

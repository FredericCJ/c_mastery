#include "events.h"
#include <limits.h>
#ifndef EV_MUTANT
#define EV_MUTANT 0
#endif
enum
{
    DATA,
    LT,
    OPEN,
    CLOSE_FIRST,
    CLOSE,
    EMPTY,
    ENTITY
};

static int alpha(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int namechar(unsigned char c)
{
    return alpha(c) || (c >= '0' && c <= '9') || c == '_' || c == '-';
}

static int space(unsigned char c)
{
    return c == ' ' || c == '\n' || c == '\t';
}

static int same(const unsigned char *a, size_t n, const unsigned char *b,
                size_t m)
{
    if (n != m)
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }
    return 1;
}

static void copy(unsigned char *a, const unsigned char *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        a[i] = b[i];
    }
}

static enum ev_status fail(struct ev_parser *p, enum ev_status s)
{
    p->terminal = s;
    return s;
}

enum ev_status ev_init(struct ev_parser *p, size_t limit)
{
    if (!p || !limit || limit > EV_DEPTH)
    {
        return EV_INVALID;
    }
    p->depth = 0;
    p->limit = limit;
    p->name_used = 0;
    p->text_used = 0;
    p->entity_used = 0;
    p->state = DATA;
    p->root_seen = 0;
    p->root_closed = 0;
    p->pending = 0;
    p->empty_end = 0;
    p->busy = 0;
    p->sequence = 0;
    p->terminal = EV_OK;
    return EV_OK;
}

static enum ev_status queue(struct ev_parser *p, enum ev_kind kind,
                            const unsigned char *s, size_t n, size_t depth)
{
    if (p->sequence == UINT32_MAX)
    {
        return fail(p, EV_LIMIT);
    }
    p->event.kind = kind;
    p->event.sequence = ++p->sequence;
    p->event.length = n;
    p->event.depth = depth;
    copy(p->event.data, s, n);
    p->pending = 1;
    return EV_OK;
}

static enum ev_status drain(struct ev_parser *p, ev_handler_fn h, void *ctx)
{
    for (;;)
    {
        if (p->pending)
        {
            enum ev_action a = h(ctx, &p->event);
            if (a == EV_WAIT)
            {
                if (EV_MUTANT == 2)
                {
                    p->pending = 0;
                }
                return EV_PAUSED;
            }
            if (a != EV_ACCEPT)
            {
                return fail(p, EV_CALLBACK);
            }
            p->pending = 0;
            if (p->event.kind == EV_DOCUMENT)
            {
                return fail(p, EV_DONE);
            }
        }
        if (!p->empty_end)
        {
            return EV_OK;
        }
        p->empty_end = 0;
        size_t d = p->depth;
        enum ev_status s =
            queue(p, EV_END, p->names[d - 1], p->lengths[d - 1], d);
        if (s != EV_OK)
        {
            return s;
        }
        p->depth--;
        if (!p->depth)
        {
            p->root_closed = 1;
        }
    }
}

static enum ev_status opened(struct ev_parser *p, int empty)
{
    if (p->root_closed || p->depth == p->limit)
    {
        return fail(p, p->root_closed ? EV_SYNTAX : EV_LIMIT);
    }
    size_t d = p->depth;
    copy(p->names[d], p->name, p->name_used);
    p->lengths[d] = p->name_used;
    p->depth++;
    p->root_seen = 1;
    p->state = DATA;
    p->empty_end = (unsigned)empty;
    return queue(p, EV_START, p->name, p->name_used, p->depth);
}

static enum ev_status closed(struct ev_parser *p)
{
    if (!p->depth)
    {
        return fail(p, EV_SYNTAX);
    }
    size_t d = p->depth;
    if (EV_MUTANT != 1 &&
        !same(p->name, p->name_used, p->names[d - 1], p->lengths[d - 1]))
    {
        return fail(p, EV_SYNTAX);
    }
    enum ev_status s = queue(p, EV_END, p->name, p->name_used, d);
    if (s != EV_OK)
    {
        return s;
    }
    p->depth--;
    if (!p->depth)
    {
        p->root_closed = 1;
    }
    p->state = DATA;
    return EV_OK;
}

static enum ev_status character(struct ev_parser *p, unsigned char c)
{
    if (!p->depth)
    {
        return space(c) ? EV_OK : fail(p, EV_SYNTAX);
    }
    p->text[p->text_used++] = c;
    if (p->text_used == EV_TEXT)
    {
        enum ev_status s =
            queue(p, EV_CHARACTERS, p->text, p->text_used, p->depth);
        p->text_used = 0;
        return s;
    }
    return EV_OK;
}

static enum ev_status byte(struct ev_parser *p, unsigned char c)
{
    switch (p->state)
    {
        case DATA:
            if (c == '<')
            {
                p->state = LT;
                return EV_OK;
            }
            if (c == '&')
            {
                if (!p->depth)
                {
                    return fail(p, EV_SYNTAX);
                }
                p->state = ENTITY;
                p->entity_used = 0;
                return EV_OK;
            }
            if (!space(c) && (c < 32 || c > 126))
            {
                return fail(p, EV_SYNTAX);
            }
            return character(p, c);
        case LT:
            p->name_used = 0;
            if (c == '/')
            {
                p->state = CLOSE_FIRST;
                return EV_OK;
            }
            if (!alpha(c))
            {
                return fail(p, EV_SYNTAX);
            }
            p->name[p->name_used++] = c;
            p->state = OPEN;
            return EV_OK;
        case CLOSE_FIRST:
            if (!alpha(c))
            {
                return fail(p, EV_SYNTAX);
            }
            p->name[p->name_used++] = c;
            p->state = CLOSE;
            return EV_OK;
        case OPEN:
        case CLOSE:
            if (c == '>')
            {
                return p->state == OPEN ? opened(p, 0) : closed(p);
            }
            if (c == '/' && p->state == OPEN)
            {
                p->state = EMPTY;
                return EV_OK;
            }
            if (!namechar(c))
            {
                return fail(p, EV_SYNTAX);
            }
            if (p->name_used == EV_NAME)
            {
                return fail(p, EV_LIMIT);
            }
            p->name[p->name_used++] = c;
            return EV_OK;
        case EMPTY:
            return c == '>' ? opened(p, 1) : fail(p, EV_SYNTAX);
        case ENTITY:
            if (c == ';')
            {
                static const unsigned char a[] = "amp", l[] = "lt", g[] = "gt",
                                           q[] = "quot", apos[] = "apos";
                unsigned char decoded;
                if (same(p->entity, p->entity_used, a, 3))
                {
                    decoded = '&';
                }
                else if (same(p->entity, p->entity_used, l, 2))
                {
                    decoded = '<';
                }
                else if (same(p->entity, p->entity_used, g, 2))
                {
                    decoded = '>';
                }
                else if (same(p->entity, p->entity_used, q, 4))
                {
                    decoded = '"';
                }
                else if (same(p->entity, p->entity_used, apos, 4))
                {
                    decoded = '\'';
                }
                else
                {
                    return fail(p, EV_SYNTAX);
                }
                p->state = DATA;
                return character(p, decoded);
            }
            if (!alpha(c) || p->entity_used == 5)
            {
                return fail(p, EV_SYNTAX);
            }
            p->entity[p->entity_used++] = c;
            return EV_OK;
        default:
            return fail(p, EV_SYNTAX);
    }
}

enum ev_status ev_feed(struct ev_parser *p, const unsigned char *data, size_t n,
                       ev_handler_fn h, void *ctx, size_t *used)
{
    if (!p || !h || !used || (!data && n))
    {
        return EV_INVALID;
    }
    if (p->busy)
    {
        return EV_BUSY;
    }
    *used = 0;
    if (p->terminal != EV_OK)
    {
        return p->terminal;
    }
    p->busy = 1;
    enum ev_status s = EV_OK;
    for (;;)
    {
        s = drain(p, h, ctx);
        if (s != EV_OK || *used == n)
        {
            break;
        }
        if (p->state == DATA && data[*used] == '<' && p->text_used)
        {
            s = queue(p, EV_CHARACTERS, p->text, p->text_used, p->depth);
            p->text_used = 0;
            if (s != EV_OK)
            {
                break;
            }
            continue;
        }
        unsigned char c = data[*used];
        (*used)++;
        s = byte(p, c);
        if (s != EV_OK)
        {
            break;
        }
    }
    p->busy = 0;
    return s;
}

enum ev_status ev_finish(struct ev_parser *p, ev_handler_fn h, void *ctx)
{
    if (!p || !h)
    {
        return EV_INVALID;
    }
    if (p->busy)
    {
        return EV_BUSY;
    }
    if (p->terminal != EV_OK)
    {
        return p->terminal;
    }
    p->busy = 1;
    enum ev_status s = drain(p, h, ctx);
    if (s == EV_OK)
    {
        if (p->state != DATA || p->depth || !p->root_seen || !p->root_closed)
        {
            s = fail(p, EV_SYNTAX);
        }
        else
        {
            s = queue(p, EV_DOCUMENT, 0, 0, 0);
            if (s == EV_OK)
            {
                s = drain(p, h, ctx);
            }
        }
    }
    p->busy = 0;
    return s;
}

enum ev_status ev_cancel(struct ev_parser *p)
{
    if (!p)
    {
        return EV_INVALID;
    }
    if (p->busy)
    {
        return EV_BUSY;
    }
    if (p->terminal != EV_OK)
    {
        return p->terminal;
    }
    p->pending = 0;
    p->empty_end = 0;
    return fail(p, EV_CANCELLED);
}

static void eventcopy(struct ev_event *a, const struct ev_event *b)
{
    a->kind = b->kind;
    a->sequence = b->sequence;
    a->depth = b->depth;
    a->length = b->length;
    copy(a->data, b->data, b->length);
}

void evq_init(struct ev_queue *q)
{
    q->head = 0;
    q->count = 0;
}

enum ev_action evq_offer(void *ctx, const struct ev_event *e)
{
    struct ev_queue *q = ctx;
    if (!q || !e || e->length > EV_DATA)
    {
        return EV_REJECT;
    }
    if (q->count == 2)
    {
        return EV_WAIT;
    }
    eventcopy(&q->events[(q->head + q->count) % 2], e);
    q->count++;
    return EV_ACCEPT;
}

enum ev_status evq_pop(struct ev_queue *q, struct ev_event *e)
{
    if (!q || !e)
    {
        return EV_INVALID;
    }
    if (!q->count)
    {
        return EV_PAUSED;
    }
    eventcopy(e, &q->events[q->head]);
    q->head = (q->head + 1) % 2;
    q->count--;
    return EV_OK;
}

void evs_init(struct ev_sum *s)
{
    s->depth = 0;
    s->sequence = 0;
    s->value = 0;
    s->sum = 0;
    s->digits = 0;
    s->seen = 0;
    s->closed = 0;
    s->done = 0;
}

enum ev_status evs_event(struct ev_sum *s, const struct ev_event *e)
{
    if (!s || !e || e->length > EV_DATA)
    {
        return EV_INVALID;
    }
    if (s->done || s->sequence == UINT32_MAX || e->sequence != s->sequence + 1)
    {
        return EV_SYNTAX;
    }
    unsigned role = 0;
    if (e->kind == EV_START)
    {
        if (same(e->data, e->length, (const unsigned char *)"sum", 3))
        {
            role = 1;
        }
        else if (same(e->data, e->length, (const unsigned char *)"g", 1))
        {
            role = 2;
        }
        else if (same(e->data, e->length, (const unsigned char *)"n", 1))
        {
            role = 3;
        }
        if (!role || s->depth == EV_DEPTH || e->depth != s->depth + 1 ||
            s->closed)
        {
            return EV_SYNTAX;
        }
        if (!s->depth)
        {
            if (s->seen || role != 1)
            {
                return EV_SYNTAX;
            }
        }
        else if (role == 1 || s->roles[s->depth - 1] == 3)
        {
            return EV_SYNTAX;
        }
        s->roles[s->depth++] = role;
        s->seen = 1;
        if (role == 3)
        {
            s->value = 0;
            s->digits = 0;
        }
    }
    else if (e->kind == EV_CHARACTERS)
    {
        if (!s->depth || e->depth != s->depth || !e->length)
        {
            return EV_SYNTAX;
        }
        uint32_t v = s->value;
        unsigned digits = s->digits;
        for (size_t i = 0; i < e->length; i++)
        {
            unsigned char c = e->data[i];
            if (s->roles[s->depth - 1] != 3)
            {
                if (!space(c))
                {
                    return EV_SYNTAX;
                }
                continue;
            }
            if (c < '0' || c > '9')
            {
                return EV_SYNTAX;
            }
            unsigned d = c - '0';
            if (v > (UINT32_MAX - d) / 10)
            {
                return EV_LIMIT;
            }
            v = v * 10 + d;
            digits = 1;
        }
        if (EV_MUTANT == 3 && s->roles[s->depth - 1] == 3)
        {
            v = (uint32_t)(e->data[e->length - 1] - '0');
        }
        s->value = v;
        s->digits = digits;
    }
    else if (e->kind == EV_END)
    {
        if (!s->depth || e->depth != s->depth)
        {
            return EV_SYNTAX;
        }
        role = s->roles[s->depth - 1];
        const unsigned char *name = (const unsigned char *)(role == 1   ? "sum"
                                                            : role == 2 ? "g"
                                                                        : "n");
        if (!same(e->data, e->length, name, role == 1 ? 3 : 1))
        {
            return EV_SYNTAX;
        }
        if (role == 3)
        {
            if (!s->digits)
            {
                return EV_SYNTAX;
            }
            if (s->sum > UINT64_MAX - s->value)
            {
                return EV_LIMIT;
            }
            s->sum += s->value;
        }
        s->depth--;
        if (!s->depth)
        {
            s->closed = 1;
        }
    }
    else if (e->kind == EV_DOCUMENT)
    {
        if (e->length || e->depth || !s->closed || s->depth)
        {
            return EV_SYNTAX;
        }
        s->done = 1;
    }
    else
    {
        return EV_INVALID;
    }
    if (EV_MUTANT != 4)
    {
        s->sequence = e->sequence;
    }
    return EV_OK;
}

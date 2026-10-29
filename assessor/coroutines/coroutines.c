#include "coroutines.h"

int co_next(struct co_range *r, uint32_t *out)
{
    if (!r || !out || !r->step || r->current > r->end || r->end > 65536)
        return CO_INVALID;
    if (r->done || r->current == r->end)
    {
        r->done = 1;
        return CO_DONE;
    }
    uint32_t value = r->current;
    if (r->step >= r->end - r->current)
    {
        r->current = r->end;
        r->done = 1;
    }
    else
        r->current += r->step;
    *out = value;
    return CO_OK;
}

int co_flow_next(struct co_flow *f, uint32_t *out)
{
    if (!f || !out)
        return CO_INVALID;
#ifndef CR_MUTATE_ITERATOR
    if (!f->remaining)
        return CO_DONE;
#endif
    for (;;)
    {
        uint32_t x;
        int r = co_next(&f->source, &x);
        f->pulls++;
        if (r)
            return r;
        if (x % 2 == 0)
        {
#ifdef CR_MUTATE_ITERATOR2
            *out = x;
#else
            *out = x * x;
#endif
            f->remaining--;
            return CO_OK;
        }
    }
}

int co_fold(struct co_flow *f, co_fold_fn fold, void *ctx, size_t *accepted)
{
    if (!f || !fold || !accepted)
        return CO_INVALID;
    size_t n = 0;
    for (;;)
    {
        uint32_t x;
        int r = co_flow_next(f, &x);
        if (r == CO_DONE)
        {
            *accepted = n;
            return CO_DONE;
        }
        if (r)
            return r;
        int choice = fold(ctx, x);
        if (choice < 0 || choice > 1)
            return CO_IO;
        n++;
        if (choice)
        {
            *accepted = n;
            return CO_OK;
        }
    }
}

int co_query_open(struct co_query *q, const struct co_row *rows, size_t n)
{
    if (!q || (!rows && n) || n > 64)
        return CO_INVALID;
    struct co_query fresh = {rows, n, 0, 0};
    *q = fresh;
    return CO_OK;
}

int co_query_next(struct co_query *q, unsigned *key, uint32_t *sum)
{
    if (!q || !key || !sum || q->index > q->count)
        return CO_INVALID;
    if (q->closed)
        return CO_STATE;
    size_t at = q->index;
    while (at < q->count)
    {
        unsigned group = q->rows[at].key;
        uint32_t total = 0;
        unsigned selected = 0;
        do
        {
            int32_t v = q->rows[at].value;
#ifdef CR_MUTATE_DATAFLOW
            if (1)
            {
#else
            if (v >= 0)
            {
#endif
                if ((uint32_t)v > UINT32_MAX - total)
                    return CO_RANGE;
                total += (uint32_t)v;
                selected = 1;
            }
            at++;
        } while (at < q->count && q->rows[at].key == group);
        if (selected)
        {
#ifdef CR_MUTATE_DATAFLOW2
            q->index = q->count;
#else
            q->index = at;
#endif
            *key = group;
            *sum = total;
            return CO_OK;
        }
    }
    q->index = at;
    return CO_DONE;
}

int co_query_close(struct co_query *q)
{
    if (!q)
        return CO_INVALID;
    q->closed = 1;
    q->rows = 0;
    return CO_OK;
}

int co_generator_create(struct co_allocator a, unsigned count,
                        struct co_generator **out)
{
    if (!a.allocate || !a.release || !out || count > 47)
        return CO_INVALID;
    struct co_generator *p = a.allocate(a.context, sizeof(*p));
    if (!p)
        return CO_RANGE;
    p->allocator = a;
    p->a = 0;
    p->b = 1;
    p->index = 0;
    p->count = count;
    *out = p;
    return CO_OK;
}

int co_generator_next(struct co_generator *p, uint32_t *out)
{
    if (!p || !out)
        return CO_INVALID;
#ifdef CR_MUTATE_GENERATOR
    if (p->index > p->count)
        return CO_DONE;
#else
    if (p->index >= p->count)
        return CO_DONE;
#endif
    uint32_t value = p->a;
    p->index++;
    if (p->index < p->count)
    {
        uint32_t next = p->a + p->b;
        p->a = p->b;
        p->b = next;
    }
    *out = value;
    return CO_OK;
}

int co_generator_destroy(struct co_generator **p)
{
    if (!p)
        return CO_INVALID;
    if (!*p)
        return CO_OK;
    struct co_generator *old = *p;
    struct co_allocator a = old->allocator;
    *p = 0;
#ifndef CR_MUTATE_GENERATOR2
    a.release(a.context, old);
#else
    (void)a;
    (void)old;
#endif
    return CO_OK;
}

int co_trampoline_init(struct co_trampoline *p, unsigned n)
{
    if (!p || n > 12)
        return CO_INVALID;
    struct co_trampoline v = {0};
    v.n = n;
    v.value = 1;
    *p = v;
    return CO_OK;
}

int co_trampoline_step(struct co_trampoline *p, unsigned budget, uint32_t *out)
{
    if (!p || !out || p->n > 12 || p->depth > 12 || p->phase > 3)
        return CO_INVALID;
    if (p->phase == 3)
        return CO_STATE;
    if (p->phase == 2)
        return CO_DONE;
    while (budget--)
    {
        if (!p->phase)
        {
            if (p->n)
            {
                p->stack[p->depth++] = p->n--;
                continue;
            }
            p->phase = 1;
#ifdef CR_MUTATE_CPS
            p->value = 0;
#endif
        }
        else if (p->depth)
        {
            unsigned multiplier = p->stack[--p->depth];
#ifndef CR_MUTATE_CPS2
            p->value *= multiplier;
#else
            (void)multiplier;
#endif
        }
        else
        {
            p->phase = 2;
            *out = p->value;
            return CO_DONE;
        }
    }
    return CO_PENDING;
}

int co_trampoline_cancel(struct co_trampoline *p)
{
    if (!p)
        return CO_INVALID;
    p->depth = 0;
    p->phase = 3;
    return CO_OK;
}

static int parse_node(const struct co_parser *p, unsigned node,
                      const unsigned char *s, size_t n, size_t *at,
                      unsigned *budget, unsigned *steps)
{
    if (!*budget)
        return CO_RANGE;
    (*budget)--;
    (*steps)++;
    size_t start = *at;
    int r;
    switch (p[node].kind)
    {
        case CO_EMPTY:
            return CO_OK;
        case CO_LITERAL:
            if (*at < n && s[*at] == p[node].literal)
            {
                (*at)++;
                return CO_OK;
            }
            return CO_NO_MATCH;
        case CO_SEQUENCE:
            r = parse_node(p, p[node].left, s, n, at, budget, steps);
            if (!r)
                r = parse_node(p, p[node].right, s, n, at, budget, steps);
            if (r)
                *at = start;
            return r;
        case CO_CHOICE:
            r = parse_node(p, p[node].left, s, n, at, budget, steps);
            if (r != CO_NO_MATCH)
                return r;
#ifdef CR_MUTATE_PARSER
            return r;
#else
            *at = start;
            return parse_node(p, p[node].right, s, n, at, budget, steps);
#endif
        case CO_MANY:
            for (;;)
            {
                size_t previous = *at;
                r = parse_node(p, p[node].left, s, n, at, budget, steps);
                if (r == CO_NO_MATCH)
                    return CO_OK;
                if (r)
                {
                    *at = start;
                    return r;
                }
                if (*at == previous)
                {
#ifdef CR_MUTATE_PARSER2
                    return CO_OK;
#else
                    *at = start;
                    return CO_STATE;
#endif
                }
            }
        default:
            return CO_INVALID;
    }
}

int co_parse(const struct co_parser *p, size_t pn, const unsigned char *s,
             size_t n, size_t start, unsigned budget, size_t *end,
             unsigned *steps)
{
    if (!p || !pn || pn > 16 || (!s && n) || !end || !steps || start > n)
        return CO_INVALID;
    for (size_t i = 0; i < pn; i++)
    {
        if (p[i].kind > CO_EMPTY)
            return CO_INVALID;
        if ((p[i].kind == CO_SEQUENCE || p[i].kind == CO_CHOICE ||
             p[i].kind == CO_MANY) &&
            (p[i].left <= i || p[i].left >= pn))
            return CO_INVALID;
        if ((p[i].kind == CO_SEQUENCE || p[i].kind == CO_CHOICE) &&
            (p[i].right <= i || p[i].right >= pn))
            return CO_INVALID;
    }
    size_t at = start;
    unsigned count = 0;
    int r = parse_node(p, 0, s, n, &at, &budget, &count);
    *steps = count;
    if (!r)
        *end = at;
    return r;
}

static int memo_s(const unsigned char *s, size_t n, size_t at,
                  struct co_memo *memo, unsigned *calls, size_t *end)
{
#ifndef CR_MUTATE_PACKRAT
    if (memo[at].state == 2)
    {
        *end = memo[at].end;
        return CO_OK;
    }
    if (memo[at].state == 3)
        return CO_NO_MATCH;
#endif
    (*calls)++;
    memo[at].state = 1;
    size_t inner = at;
    int r = CO_NO_MATCH;
    if (at < n && s[at] == 'a')
    {
        r = memo_s(s, n, at + 1, memo, calls, &inner);
        if (r == CO_OK && inner < n && s[inner] == 'b')
        {
            *end = inner + 1;
            r = CO_OK;
            goto done;
        }
        r = memo_s(s, n, at + 1, memo, calls, &inner);
        if (r == CO_OK && inner < n && s[inner] == 'c')
        {
            *end = inner + 1;
            r = CO_OK;
            goto done;
        }
    }
    if (at < n && s[at] == 'x')
    {
        *end = at + 1;
        r = CO_OK;
    }
    else
        r = CO_NO_MATCH;
done:
    memo[at].state = r == CO_OK ? 2 : 3;
    if (!r)
    {
#ifdef CR_MUTATE_PACKRAT2
        memo[at].end = at;
#else
        memo[at].end = *end;
#endif
    }
    return r;
}

int co_packrat(const unsigned char *s, size_t n, struct co_memo *memo,
               size_t cap, size_t *end, unsigned *calls)
{
    if ((!s && n) || !memo || !end || !calls || n > 64 || cap < n + 1)
        return CO_INVALID;
    for (size_t i = 0; i <= n; i++)
        memo[i].state = 0;
    unsigned count = 0;
    size_t result = 0;
    int r = memo_s(s, n, 0, memo, &count, &result);
    *calls = count;
    if (r)
        return r;
    if (result != n)
        return CO_NO_MATCH;
    *end = result;
    return CO_OK;
}

int co_async_init(struct co_async *p, size_t wanted, uint32_t generation)
{
    if (!p || !wanted || wanted > 8 || !generation)
        return CO_INVALID;
    struct co_async f = {0};
    f.wanted = wanted;
    f.generation = generation;
    *p = f;
    return CO_OK;
}

static void disarm(struct co_async *p, struct co_async_port port)
{
    if (p->registered)
    {
        p->registered = 0;
        port.disarm(port.context, p->generation);
    }
}

int co_async_poll(struct co_async *p, struct co_async_port port,
                  unsigned char *out, size_t *n)
{
    if (!p || !port.read || !port.arm || !port.disarm || !out || !n ||
        p->length > p->wanted || p->wanted > 8 || !p->wanted)
        return CO_INVALID;
    if (p->state == 2 || p->state == 3)
        return CO_STATE;
    if (p->state == 1)
        return CO_DONE;
    for (unsigned attempt = 0; attempt < 2; attempt++)
    {
        struct co_read_result r =
            port.read(port.context, p->data + p->length, p->wanted - p->length);
        if (r.status > CO_READ_ERROR || r.count > p->wanted - p->length ||
            (r.status == CO_READ_AGAIN && r.count))
        {
            disarm(p, port);
            p->state = 3;
            return CO_INVALID;
        }
        p->length += r.count;
        if (r.status == CO_READ_ERROR)
        {
            disarm(p, port);
            p->state = 3;
            return CO_IO;
        }
        if (p->length == p->wanted)
        {
            disarm(p, port);
            for (size_t i = 0; i < p->length; i++)
                out[i] = p->data[i];
            *n = p->length;
            p->state = 1;
            return CO_DONE;
        }
        if (r.status == CO_READ_EOF)
        {
            disarm(p, port);
            p->state = 3;
            return CO_NO_MATCH;
        }
        if (!p->registered)
        {
            if (port.arm(port.context, p->generation))
            {
                p->state = 3;
                return CO_IO;
            }
            p->registered = 1;
#ifdef CR_MUTATE_ASYNC
            return CO_PENDING;
#endif
        }
        else
            return CO_PENDING;
    }
    return CO_PENDING;
}

int co_async_notify(struct co_async *p, uint32_t gen)
{
    if (!p)
        return CO_INVALID;
    if (p->state || !p->registered || gen != p->generation)
        return CO_STATE;
    p->ready = 1;
    return CO_OK;
}

int co_async_cancel(struct co_async *p, struct co_async_port port)
{
    if (!p || !port.disarm)
        return CO_INVALID;
    if (p->state == 1)
        return CO_STATE;
#ifndef CR_MUTATE_ASYNC2
    disarm(p, port);
#endif
    p->state = 2;
    return CO_OK;
}

int co_effect_start(struct co_effect *p, struct co_continuation *out)
{
    if (!p || !out)
        return CO_INVALID;
    p->pc = 1;
    p->generation = 1;
    p->first = 0;
    struct co_continuation k = {p, 1, 1};
    *out = k;
    return CO_REQUEST;
}

int co_effect_resume(struct co_continuation *k, uint32_t value,
                     struct co_continuation *next, uint32_t *out)
{
    if (!k || !k->owner || !next || !out)
        return CO_INVALID;
    struct co_effect *p = k->owner;
#ifndef CR_MUTATE_EFFECT
    if (k->generation != p->generation || k->question != p->pc || p->pc < 1 ||
        p->pc > 2)
        return CO_STATE;
#endif
    if (p->pc == 2 && value > UINT32_MAX - p->first)
        return CO_RANGE;
    if (p->generation == UINT32_MAX)
        return CO_RANGE;
    p->generation++;
    k->owner = 0;
    if (p->pc == 1)
    {
        p->first = value;
        p->pc = 2;
        struct co_continuation q = {p, p->generation, 2};
        *next = q;
        return CO_REQUEST;
    }
#ifdef CR_MUTATE_EFFECT2
    *out = value;
#else
    *out = p->first + value;
#endif
    p->pc = 3;
    return CO_DONE;
}

int co_effect_abort(struct co_continuation *k)
{
    if (!k || !k->owner)
        return CO_INVALID;
    struct co_effect *p = k->owner;
    if (k->generation != p->generation || p->pc < 1 || p->pc > 2)
        return CO_STATE;
    p->pc = 4;
    k->owner = 0;
    return CO_OK;
}

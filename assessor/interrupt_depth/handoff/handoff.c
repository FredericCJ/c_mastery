#include "handoff.h"
#ifndef XI_MUTANT
#define XI_MUTANT 0
#endif
_Static_assert(sizeof(unsigned) == 4 && UINT_MAX == UINT32_MAX,
               "declared32-bit index");

static void increment(uint32_t *v)
{
    if (*v != UINT32_MAX)
    {
        ++*v;
    }
}

#include "queue.c"

int xi_service_init(struct xi_service *s, struct xi_queue *q)
{
    if (!s || !q)
    {
        return XI_INVALID;
    }
    *s = (struct xi_service){.queue = q};
    return XI_OK;
}

int xi_capture_begin(struct xi_service *s, uint32_t seq, uint32_t stamp,
                     const unsigned char *bytes, uint32_t n)
{
    if (!s || (n && !bytes))
    {
        return XI_INVALID;
    }
    if (s->active && XI_MUTANT != 4)
    {
        increment(&s->reentrant);
        return XI_BUSY;
    }
    increment(&s->acknowledged);
    int rc = xi_reserve(s->queue);
    if (rc == XI_FULL)
    {
        if (XI_MUTANT == 3)
        {
            ++s->dropped;
        }
        else
        {
            increment(&s->dropped);
        }
        return rc;
    }
    if (rc)
    {
        return rc;
    }
    struct xi_record r = {.sequence = seq,
                          .timestamp = stamp,
                          .original_length = n,
                          .copied = n > XI_RECORD_BYTES ? XI_RECORD_BYTES : n,
                          .flags = n > XI_RECORD_BYTES ? XI_TRUNCATED : 0};
    for (unsigned i = 0; i < r.copied; ++i)
    {
        r.bytes[i] = bytes[i];
    }
    rc = xi_fill(s->queue, &r);
    if (rc)
    {
        (void)xi_cancel(s->queue);
        return rc;
    }
    s->active = 1;
    return XI_OK;
}

int xi_capture_end(struct xi_service *s)
{
    if (!s)
    {
        return XI_INVALID;
    }
    if (!s->active)
    {
        return XI_STATE;
    }
    int rc = xi_commit(s->queue);
    if (!rc)
    {
        s->active = 0;
    }
    return rc;
}

void xi_latest_init(struct xi_latest *q)
{
    if (q)
    {
        *q = (struct xi_latest){0};
    }
}

int xi_latest_push(struct xi_latest *q, const struct xi_record *r)
{
    if (!q || !r || r->copied > XI_RECORD_BYTES)
    {
        return XI_INVALID;
    }
    if (q->count == 3)
    {
        q->first = (q->first + 1) % 3;
        increment(&q->dropped);
        --q->count;
    }
    q->records[(q->first + q->count) % 3] = *r;
    ++q->count;
    return XI_OK;
}

int xi_latest_pop(struct xi_latest *q, struct xi_record *r)
{
    if (!q || !r)
    {
        return XI_INVALID;
    }
    if (!q->count)
    {
        return XI_EMPTY;
    }
    *r = q->records[q->first];
    q->first = (q->first + 1) % 3;
    --q->count;
    return XI_OK;
}

static int admitted(struct xi_mask *s, unsigned value)
{
    if (!s->threshold)
    {
        return value <= 1;
    }
    struct xf_priority p;
    return xf_priority(s->config, value, &p) == XF_OK && p.encoded == value;
}

int xi_mask_init(struct xi_mask *s, struct xf_priority_config c,
                 unsigned threshold, unsigned prior)
{
    struct xf_priority p;
    if (!s || threshold > 1 || xf_priority(c, 0, &p))
    {
        return XI_INVALID;
    }
    struct xi_mask r = {
        .config = c, .threshold = threshold, .value = prior, .next = 1};
    if (!admitted(&r, prior))
    {
        return XI_INVALID;
    }
    *s = r;
    return XI_OK;
}

int xi_mask_enter(struct xi_mask *s, unsigned requested, uint32_t *token)
{
    if (!s || !token || !admitted(s, requested) ||
        (!s->threshold && requested != 1))
    {
        return XI_INVALID;
    }
    if (s->depth == 8)
    {
        return XI_FULL;
    }
    if (s->next == UINT32_MAX)
    {
        return XI_EXHAUSTED;
    }
    unsigned i = s->depth++;
    s->saved[i] = s->value;
    s->tokens[i] = s->next++;
    *token = s->tokens[i];
    if (!s->threshold)
    {
        s->value = 1;
    }
    else if (XI_MUTANT == 6 ||
             (requested && (!s->value || requested < s->value)))
    {
        s->value = requested;
    }
    return XI_OK;
}

int xi_mask_leave(struct xi_mask *s, uint32_t token)
{
    if (!s)
    {
        return XI_INVALID;
    }
    if (!s->depth || s->tokens[s->depth - 1] != token)
    {
        return XI_STATE;
    }
    --s->depth;
    s->value = XI_MUTANT == 5 ? 0 : s->saved[s->depth];
    return XI_OK;
}

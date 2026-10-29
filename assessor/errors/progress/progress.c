#include "progress.h"
#ifndef EPT_MUTATION
#define EPT_MUTATION 0
#endif
void ept_init(struct ept_stream *s)
{
    if (s)
    {
        struct ept_stream empty = {0};
        *s = empty;
        sf_init(&s->frame);
    }
}

static int valid(struct ept_handle h)
{
    return h.owner && h.owner->active && h.generation &&
           h.generation == h.owner->frame.generation;
}

static void observation(struct ept_stream *s, struct ept_observation *o)
{
    *o = (struct ept_observation){s->frame.offset, s->durable, s->epoch,
                                  s->state, s->uncertain};
}

unsigned ept_observe(struct ept_handle h, struct ept_observation *o)
{
    if (!o)
    {
        return EPT_INVALID;
    }
    if (!valid(h))
    {
        return EPT_STALE;
    }
    observation(h.owner, o);
    return EPT_OK;
}

unsigned ept_start(struct ept_stream *s, const unsigned char *bytes, size_t n,
                   struct ept_handle *out)
{
    if (!s || !out || (!bytes && n))
    {
        return EPT_INVALID;
    }
    if (s->active)
    {
        return EPT_BUSY;
    }
    if (n > 64 || s->frame.generation == UINT32_MAX)
    {
        return EPT_RANGE;
    }
    int result = sf_start(&s->frame, bytes, n, &s->base);
    if (result != SF_OK)
    {
        return EPT_CONTRACT;
    }
    s->active = 1;
    s->state = EPT_AGAIN;
    s->uncertain = 0;
    s->durable = 0;
    s->epoch = 0;
    *out = (struct ept_handle){s, s->frame.generation};
    return EPT_OK;
}

struct er_bridge
{
    er_sink sink;
    void *context;
    er_progress result;
    unsigned called;
};

static ptrdiff_t bridge(void *context, const unsigned char *bytes, size_t n)
{
    struct er_bridge *b = context;
    b->called = 1;
    b->result = er_transfer(bytes, n, 0, b->sink, b->context);
    if (b->result.code == ER_CONTRACT || b->result.code == ER_INVALID)
    {
        return -1;
    }
    if (b->result.next)
    {
        return (ptrdiff_t)b->result.next;
    }
    return b->result.code == ER_IO ? -1 : 0;
}

unsigned ept_step(struct ept_handle h, er_sink sink, void *context,
                  struct ept_observation *out)
{
    if (!sink || !out)
    {
        return EPT_INVALID;
    }
    if (!valid(h))
    {
        return EPT_STALE;
    }
    struct ept_stream *s = h.owner;
    if (s->uncertain)
    {
        observation(s, out);
        return EPT_UNCERTAIN;
    }
    if (s->state != EPT_AGAIN)
    {
        observation(s, out);
        return s->state;
    }
    struct er_bridge b = {sink, context, {0, ER_OK}, 0};
    size_t accepted = 0;
    int result = sf_pump(s->base, bridge, &b, 1, &accepted);
    if (b.called &&
        (b.result.code == ER_CONTRACT || b.result.code == ER_INVALID))
    {
        s->uncertain = EPT_MUTATION == 7 ? 0 : 2;
        s->state = EPT_CONTRACT;
    }
    else if (b.called && b.result.code == ER_IO && EPT_MUTATION != 1)
    {
        s->state = EPT_IO;
    }
    else if (result == SF_DONE && !(b.called && b.result.code == ER_AGAIN))
    {
        s->state = EPT_DONE;
    }
    else if (result != SF_PENDING && result != SF_DONE)
    {
        s->uncertain = 2;
        s->state = EPT_CONTRACT;
    }
    if (EPT_MUTATION == 2)
    {
        s->durable = s->frame.offset;
    }
    observation(s, out);
    return s->state;
}

unsigned ept_cancel(struct ept_handle h, struct ept_observation *out)
{
    if (!out)
    {
        return EPT_INVALID;
    }
    if (!valid(h))
    {
        return EPT_STALE;
    }
    struct ept_stream *s = h.owner;
    if (s->state != EPT_AGAIN)
    {
        observation(s, out);
        return s->state;
    }
    if (s->frame.state == 1 && sf_cancel(s->base) != SF_OK)
    {
        return EPT_CONTRACT;
    }
    if (EPT_MUTATION != 3)
    {
        s->state = EPT_CANCELLED;
    }
    observation(s, out);
    return s->state;
}

unsigned ept_checkpoint(struct ept_handle h, ept_flush_fn flush, void *context,
                        struct ept_observation *out)
{
    if (!flush || !out)
    {
        return EPT_INVALID;
    }
    if (!valid(h))
    {
        return EPT_STALE;
    }
    struct ept_stream *s = h.owner;
    if (s->uncertain)
    {
        observation(s, out);
        return EPT_UNCERTAIN;
    }
    if (s->durable == s->frame.offset)
    {
        observation(s, out);
        return EPT_OK;
    }
    if (s->epoch == UINT32_MAX)
    {
        observation(s, out);
        return EPT_RANGE;
    }
    struct ept_flush_result r =
        flush(context, s->frame.offset, s->durable, s->epoch);
    if (r.code == EPT_FLUSH_UNKNOWN)
    {
        s->uncertain = 1;
        observation(s, out);
        return EPT_UNCERTAIN;
    }
    if (r.code > EPT_FLUSH_IO || r.prefix < s->durable ||
        r.prefix > s->frame.offset ||
        (r.prefix == s->durable ? r.epoch != s->epoch
                                : r.epoch != s->epoch + 1) ||
        (r.code == EPT_FLUSH_OK && r.prefix == s->durable))
    {
        s->uncertain = EPT_MUTATION == 8 ? 0 : 1;
        observation(s, out);
        return EPT_CONTRACT;
    }
    if (EPT_MUTATION != 4 || r.code == EPT_FLUSH_OK)
    {
        s->durable = r.prefix;
        s->epoch = r.epoch;
    }
    observation(s, out);
    return r.code == EPT_FLUSH_IO ? EPT_IO : EPT_OK;
}

unsigned ept_reconcile(struct ept_handle h, size_t prefix, uint32_t epoch,
                       unsigned verified, struct ept_observation *out)
{
    if (!out || verified > 1)
    {
        return EPT_INVALID;
    }
    if (!valid(h))
    {
        return EPT_STALE;
    }
    struct ept_stream *s = h.owner;
    if (s->uncertain != 1 || (!verified && EPT_MUTATION != 6))
    {
        return EPT_BUSY;
    }
    if (prefix < s->durable || prefix > s->frame.offset || epoch < s->epoch ||
        (prefix == s->durable ? epoch != s->epoch : epoch != s->epoch + 1) ||
        (prefix > s->durable && s->epoch == UINT32_MAX))
    {
        return EPT_RANGE;
    }
    s->durable = prefix;
    s->epoch = epoch;
    s->uncertain = 0;
    observation(s, out);
    return EPT_OK;
}

unsigned ept_release(struct ept_handle h, unsigned quiescent, unsigned abandon)
{
    if (quiescent > 1 || abandon > 1)
    {
        return EPT_INVALID;
    }
    if (!valid(h))
    {
        return EPT_STALE;
    }
    struct ept_stream *s = h.owner;
    if (!quiescent || s->state == EPT_AGAIN)
    {
        return EPT_BUSY;
    }
    if (EPT_MUTATION != 5 && (s->uncertain || s->durable < s->frame.offset) &&
        !abandon)
    {
        return EPT_BUSY;
    }
    if (s->frame.state == 1 && sf_cancel(s->base) != SF_OK)
    {
        return EPT_CONTRACT;
    }
    s->active = 0;
    return EPT_OK;
}

#include "service.h"
#include <limits.h>
#ifndef CS_MUTANT
#define CS_MUTANT 0
#endif
static void bump(unsigned *value)
{
    if (*value != UINT_MAX)
    {
        ++*value;
    }
}

static void settle(struct cs_service *s)
{
    if (s->state != CS_OPEN && !s->count && !s->has_active)
    {
        s->state = CS_STOPPED;
    }
}

static void release_active(struct cs_service *s, int delivered)
{
    if (delivered)
    {
        ++s->delivered;
    }
    else
    {
        ++s->discarded;
    }
    ++s->releases;
    s->has_active = 0;
    s->borrowed = 0;
    s->failed = 0;
    settle(s);
}

static void copy_wire(struct cs_wire *to, const struct cs_wire *from)
{
    to->length = from->length;
    to->sequence = from->sequence;
    to->replaceable = from->replaceable;
    for (size_t i = 0; i < from->length; ++i)
    {
        to->bytes[i] = from->bytes[i];
    }
}

enum cs_status cs_init(struct cs_service *s, size_t capacity, unsigned limit)
{
    if (!s || !capacity || capacity > CS_QUEUE || !limit)
    {
        return CS_INVALID;
    }
    *s = (struct cs_service){0};
    s->capacity = capacity;
    s->limit = limit;
    return CS_OK;
}

enum cs_status cs_offer(struct cs_service *s, const struct cp_message *m,
                        int authorized, int replaceable, int overwrite)
{
    if (!s || !m || m->length > CP_PAYLOAD ||
        (replaceable != 0 && replaceable != 1) ||
        (overwrite != 0 && overwrite != 1))
    {
        return CS_INVALID;
    }
    if (s->state != CS_OPEN)
    {
        bump(&s->rejected);
        return CS_CLOSED;
    }
    if (!authorized && CS_MUTANT != 1)
    {
        bump(&s->denied);
        return CS_DENIED;
    }
    if (s->accepted == CS_MAX_ADMISSIONS)
    {
        bump(&s->rejected);
        return CS_FULL;
    }
    if (s->count == s->capacity)
    {
        if (!overwrite || !replaceable || !s->queue[s->head].replaceable)
        {
            bump(&s->rejected);
            return CS_FULL;
        }
        ++s->discarded;
        ++s->releases;
        s->head = (s->head + 1) % CS_QUEUE;
        --s->count;
    }
    struct cs_wire *w = &s->queue[(s->head + s->count) % CS_QUEUE];
    if (cp_encode(m, w->bytes, sizeof w->bytes, &w->length) != CP_OK)
    {
        return CS_INVALID;
    }
    w->sequence = m->sequence;
    w->replaceable = replaceable;
    ++s->count;
    ++s->accepted;
    return CS_FRAME;
}

enum cs_status cs_feed(struct cs_service *s, unsigned session,
                       const unsigned char *bytes, size_t n, size_t *used,
                       int authorized, int replaceable)
{
    if (!s || session >= CS_SESSIONS || (!bytes && n) || n > CS_WIRE || !used)
    {
        return CS_INVALID;
    }
    *used = 0;
    if (s->state != CS_OPEN)
    {
        return CS_CLOSED;
    }
    struct cs_input *in = &s->input[session];
    struct cp_parser p;
    cp_parser_init(&p);
    /* Restore only a state produced by the unchanged parser. No pending message
       survives a turn: its owner immediately admits or explicitly rejects it. */
    p.used = in->used;
    for (size_t i = 0; i < p.used; ++i)
    {
        p.candidate[i] = in->bytes[i];
    }
    enum cp_status status = cp_feed(&p, bytes, n, used);
    in->used = p.used;
    for (size_t i = 0; i < p.used; ++i)
    {
        in->bytes[i] = p.candidate[i];
    }
    if (status == CP_FRAME)
    {
        struct cp_message m;
        if (cp_peek(&p, &m) != CP_FRAME)
        {
            return CS_INVALID;
        }
        return cs_offer(s, &m, authorized, replaceable, 0);
    }
    return status == CP_REJECT      ? CS_REJECT
           : status == CP_NEED_MORE ? CS_MORE
                                    : CS_INVALID;
}

enum cs_status cs_begin(struct cs_service *s)
{
    if (!s)
    {
        return CS_INVALID;
    }
    if (s->has_active)
    {
        return CS_PENDING;
    }
    if (!s->count)
    {
        settle(s);
        return s->state == CS_STOPPED ? CS_CLOSED : CS_MORE;
    }
    if (s->generation == s->limit)
    {
        return CS_FAILED;
    }
    copy_wire(&s->active, &s->queue[s->head]);
    s->head = (s->head + 1) % CS_QUEUE;
    --s->count;
    s->has_active = 1;
    s->borrowed = 0;
    s->failed = 0;
    s->first_cause = 0;
    s->offset = 0;
    ++s->generation;
    return CS_OK;
}

enum cs_status cs_step(struct cs_service *s, cs_sink_fn sink, void *context)
{
    if (!s || !sink || !s->has_active || s->borrowed)
    {
        return CS_INVALID;
    }
    if (s->failed)
    {
        return CS_FAILED;
    }
    size_t remaining = s->active.length - s->offset;
    struct cs_io result = sink(context, s->active.bytes + s->offset, remaining);
    if (result.accepted > remaining || (unsigned)result.kind > CS_IO_ERROR ||
        ((result.kind == CS_IO_AGAIN || result.kind == CS_IO_INTERRUPTED) &&
         result.accepted))
    {
        s->failed = 1;
        if (!s->first_cause)
        {
            s->first_cause = 4096;
        }
        return CS_FAILED;
    }
    s->offset += result.accepted;
    if (result.kind == CS_IO_ERROR)
    {
        s->failed = 1;
        if (!s->first_cause)
        {
            s->first_cause = result.cause ? result.cause : 4096;
        }
        return CS_FAILED;
    }
    if (s->offset == s->active.length)
    {
        release_active(s, 1);
        return CS_DONE;
    }
    return CS_PENDING;
}

enum cs_status cs_recover(struct cs_service *s)
{
    if (!s || !s->has_active || s->borrowed || s->state == CS_CANCEL)
    {
        return CS_INVALID;
    }
    s->failed = 0;
    return CS_OK;
}

enum cs_status cs_lease(struct cs_service *s, const unsigned char **bytes,
                        size_t *n, unsigned *generation)
{
    if (!s || !bytes || !n || !generation || !s->has_active || s->borrowed ||
        s->failed)
    {
        return CS_INVALID;
    }
    s->borrowed = 1;
    *bytes = s->active.bytes + s->offset;
    *n = s->active.length - s->offset;
    *generation = s->generation;
    return CS_OK;
}

enum cs_status cs_complete(struct cs_service *s, unsigned generation,
                           int quiescent, size_t accepted, cs_done_fn done,
                           void *context)
{
    if (!s)
    {
        return CS_INVALID;
    }
    if (!s->has_active || !s->borrowed || generation != s->generation)
    {
        return CS_STALE;
    }
    if (!quiescent)
    {
        return CS_PENDING;
    }
    if (accepted > s->active.length - s->offset)
    {
        return CS_INVALID;
    }
    s->offset += accepted;
    s->borrowed = 0;
    if (s->state != CS_CANCEL && s->offset != s->active.length)
    {
        return CS_PENDING;
    }
    uint16_t id = s->active.sequence;
    enum cs_status status = s->state == CS_CANCEL ? CS_CLOSED : CS_DONE;
    release_active(s, status == CS_DONE);
    /* Terminal notification is the last object access. The callback may free s. */
    if (done)
    {
        done(context, id, status);
    }
    return status;
}

enum cs_status cs_close(struct cs_service *s, int cancel)
{
    if (!s || (cancel != 0 && cancel != 1))
    {
        return CS_INVALID;
    }
    if (s->state == CS_STOPPED)
    {
        return CS_CLOSED;
    }
    if (cancel)
    {
        s->state = CS_CANCEL;
        while (s->count)
        {
            s->head = (s->head + 1) % CS_QUEUE;
            --s->count;
            ++s->discarded;
            ++s->releases;
        }
        if (s->has_active && (!s->borrowed || CS_MUTANT == 2))
        {
            release_active(s, 0);
        }
    }
    else if (s->state == CS_OPEN)
    {
        s->state = CS_DRAIN;
    }
    settle(s);
    return s->state == CS_STOPPED ? CS_CLOSED : CS_PENDING;
}

int cs_conserved(const struct cs_service *s)
{
    return s && s->count <= s->capacity && s->capacity <= CS_QUEUE &&
           s->accepted == s->delivered + s->discarded + (unsigned)s->count +
                              (unsigned)s->has_active &&
           s->releases == s->delivered + s->discarded;
}

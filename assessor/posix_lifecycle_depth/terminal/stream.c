#include "stream.h"
#ifndef PS_MUTANT
#define PS_MUTANT 0
#endif
/* A session is one connection epoch of a byte stream feeding the borrowed
   checkpoint parser. Parser and queue state never survive a reconnect:
   pt_session_open() and pt_session_close() both reset them, so bytes of a
   half-received frame cannot be completed by the next connection. */
void pt_session_init(struct pt_session *s)
{
    if (s)
    {
        *s = (struct pt_session){0};
        cp_parser_init(&s->parser);
        cp_queue_init(&s->queue);
    }
}

/* Starts a new epoch; fails with CP_INVALID if already connected or the
   epoch counter would wrap. */
enum cp_status pt_session_open(struct pt_session *s)
{
    if (!s || s->connected || s->epoch == UINT64_MAX)
    {
        return CP_INVALID;
    }
    ++s->epoch;
    s->connected = true;
    s->rejects = 0;
#if PS_MUTANT != 1
    cp_parser_init(&s->parser);
    cp_queue_init(&s->queue);
#endif
    return CP_OK;
}

void pt_session_close(struct pt_session *s)
{
    if (s)
    {
        s->connected = false;
#if PS_MUTANT != 1
        cp_parser_init(&s->parser);
        cp_queue_init(&s->queue);
#endif
    }
}

/* Feeds `n` bytes, resynchronising after rejected frames. *used is the
   number of bytes consumed from `bytes` and is valid on every return except
   CP_INVALID. Each CP_REJECT counts toward s->rejects (saturating) and the
   loop carries on from the next unconsumed byte; a delivered frame is
   queued; any other parser status (for example CP_NEED_MORE) is returned
   to the caller as is. */
enum cp_status pt_session_feed(struct pt_session *s, const unsigned char *bytes,
                               size_t n, size_t *used)
{
    if (!s || !s->connected || !used || (!bytes && n))
    {
        return CP_INVALID;
    }
    *used = 0;
    for (;;)
    {
        size_t consumed = 0;
        /* NULL is passed only when n == 0 (the caller's buffer may be
           NULL). When n > 0 and every byte is consumed, this passes
           bytes + n, one past the end, with length 0. */
        enum cp_status result =
            cp_feed(&s->parser, n ? bytes + *used : NULL, n - *used, &consumed);
        *used += consumed;
        if (result == CP_FRAME)
        {
            result = cp_deliver(&s->parser, &s->queue);
            if (result != CP_OK)
            {
                return result;
            }
        }
        else if (result == CP_REJECT)
        {
            if (s->rejects != SIZE_MAX)
            {
                ++s->rejects;
            }
#if PS_MUTANT == 2
            *used = n;
            return CP_NEED_MORE;
#endif
        }
        else
        {
            return result;
        }
    }
}

enum cp_status pt_session_take(struct pt_session *s, struct cp_message *m)
{
    if (!s || !s->connected || !m)
    {
        return CP_INVALID;
    }
    return cp_take(&s->queue, m);
}

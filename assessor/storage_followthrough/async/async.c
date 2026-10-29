#include "async.h"
#ifndef SA_MUTANT
#define SA_MUTANT 0
#endif
int sa_start(struct sa_read *s, struct sa_port p, uint32_t page, void *buffer)
{
    if (!s || !buffer || !p.submit_read || !p.wait)
    {
        return SA_INVALID;
    }
    if (s->state != SA_EMPTY)
    {
        return SA_BUSY;
    }
    int64_t t = p.submit_read(p.context, page, buffer);
    if (t <= 0)
    {
        return SA_IO;
    }
    *s = (struct sa_read){p, buffer, (uint64_t)t, SA_OWNED, 0};
    return SA_OK;
}

int sa_abandon(struct sa_read *s)
{
    if (!s || s->state == SA_EMPTY)
    {
        return SA_INVALID;
    }
    s->abandoned = 1;
    if (SA_MUTANT == 1)
    {
        s->state = SA_COMPLETED;
    }
    return s->state == SA_COMPLETED     ? SA_DRAINED
           : s->state == SA_QUARANTINED ? SA_UNKNOWN
                                        : SA_BUSY;
}

int sa_finish(struct sa_read *s)
{
    if (!s || s->state == SA_EMPTY)
    {
        return SA_INVALID;
    }
    if (s->state == SA_QUARANTINED)
    {
        return SA_UNKNOWN;
    }
    if (s->state == SA_COMPLETED)
    {
        return s->abandoned ? SA_DRAINED : SA_READY;
    }
    int64_t r = s->port.wait(s->port.context, s->ticket);
    if (r)
    {
        s->state = SA_MUTANT == 2 ? SA_COMPLETED : SA_QUARANTINED;
        return SA_UNKNOWN;
    }
    s->state = SA_COMPLETED;
    return s->abandoned && SA_MUTANT != 3 ? SA_DRAINED : SA_READY;
}

int sa_release(struct sa_read *s, void **out)
{
    if (!s || !out)
    {
        return SA_INVALID;
    }
    if (s->state != SA_COMPLETED)
    {
        return s->state == SA_QUARANTINED ? SA_UNKNOWN : SA_BUSY;
    }
    *out = s->buffer;
    *s = (struct sa_read){0};
    return SA_OK;
}

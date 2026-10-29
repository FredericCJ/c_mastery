#include "superloop.h"
/* Deliberately compilable unsolved learner scaffold. */
#define IGNORE(x) (void)(x)

int sc_bound(const uint64_t *a, size_t b, uint64_t c, int d, struct sc_bound *e)
{
    IGNORE(a);
    IGNORE(b);
    IGNORE(c);
    IGNORE(d);
    IGNORE(e);
    return SC_INVALID;
}

int sc_task_init(struct sc_task *a, const unsigned char *b, size_t c)
{
    IGNORE(a);
    IGNORE(b);
    IGNORE(c);
    return SC_INVALID;
}

int sc_task_step(struct sc_task *a, size_t b, size_t *c)
{
    IGNORE(a);
    IGNORE(b);
    IGNORE(c);
    return SC_INVALID;
}

int sc_task_cancel(struct sc_task *a)
{
    IGNORE(a);
    return SC_INVALID;
}

int sc_task_result(const struct sc_task *a, unsigned char *b, size_t c,
                   size_t *d)
{
    IGNORE(a);
    IGNORE(b);
    IGNORE(c);
    IGNORE(d);
    return SC_INVALID;
}

int sc_events_init(struct sc_events *a)
{
    IGNORE(a);
    return SC_INVALID;
}

int sc_events_push(struct sc_events *a, struct sc_event b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_events_pop(struct sc_events *a, struct sc_event *b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_latest_init(struct sc_latest *a)
{
    IGNORE(a);
    return SC_INVALID;
}

int sc_latest_put(struct sc_latest *a, uint64_t b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_latest_take(struct sc_latest *a, uint64_t *b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_releases_init(struct sc_releases *a, uint64_t b, uint64_t c, unsigned d)
{
    IGNORE(a);
    IGNORE(b);
    IGNORE(c);
    IGNORE(d);
    return SC_INVALID;
}

int sc_releases_poll(struct sc_releases *a, uint64_t b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_releases_pop(struct sc_releases *a, uint64_t *b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_schedule_init(struct sc_schedule *a, unsigned b)
{
    IGNORE(a);
    IGNORE(b);
    return SC_INVALID;
}

int sc_schedule_next(struct sc_schedule *a, uint64_t b, unsigned c, unsigned *d)
{
    IGNORE(a);
    IGNORE(b);
    IGNORE(c);
    IGNORE(d);
    return SC_INVALID;
}

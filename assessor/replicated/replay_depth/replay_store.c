#include "replay_depth.h"
#include <string.h>
#ifndef RPR_MUTATION
#define RPR_MUTATION 0
#endif
static int status(store *p, int r)
{
    if (!r)
        return RPR_OK;
    if (p->poisoned)
        return RPR_UNCERTAIN;
    return r == STORE_FULL ? RPR_FULL : r == STORE_STATE ? RPR_STATE : RPR_IO;
}

static int save(store *p, const struct rpr_session *s)
{
    unsigned char wire[512];
    unsigned i;
    int r;
    if (!p || p->active || p->poisoned)
        return RPR_STATE;
    r = rpr_encode(s, wire);
    if (r)
        return r;
    r = store_begin(p);
    for (i = 0; !r && i < 8; ++i)
    {
        unsigned char key[2] = {'R', (unsigned char)i};
        r = store_put(p, key, 2, wire + 64 * i, 64);
    }
    if (!r)
        r = store_commit(p);
    if (r)
        store_abort(p);
    return status(p, r);
}

static int load(store *p, uint64_t timeline, struct rpr_session *out)
{
    unsigned char wire[512];
    unsigned i;
    if (!p || p->active || p->poisoned)
        return RPR_STATE;
    for (i = 0; i < 8; ++i)
    {
        unsigned char key[2] = {'R', (unsigned char)i};
        size_t n = 0;
        int r = store_get(p, key, 2, wire + 64 * i, 64, &n);
        if (r || n != 64)
            return r == STORE_IO ? RPR_IO : RPR_CORRUPT;
    }
    return rpr_decode(timeline, wire, out);
}

static int current(struct rpr_durable *d)
{
    struct rpr_session stored;
    unsigned char a[512], b[512];
    int r;
    if (!d || !d->pages || !rpr_invariant(&d->session))
        return RPR_INVALID;
    if (RPR_MUTATION == 7)
        return RPR_OK;
    r = load(d->pages, d->session.timeline, &stored);
    if (r)
        return r;
    if (rpr_encode(&stored, a) || rpr_encode(&d->session, b))
        return RPR_CORRUPT;
    return memcmp(a, b, 512) ? RPR_STATE : RPR_OK;
}

static int publish(struct rpr_durable *d, const struct rpr_session *next)
{
    unsigned char a[512], b[512];
    int r;
    if (rpr_encode(&d->session, a) || rpr_encode(next, b))
        return RPR_INVALID;
    if (!memcmp(a, b, 512))
        return RPR_OK;
    if (RPR_MUTATION == 4)
        d->session = *next;
    r = save(d->pages, next);
    if (!r)
        d->session = *next;
    return r;
}

int rpr_create(store *p, uint64_t timeline, unsigned horizon,
               const struct rp_state initial[2], struct rpr_durable *out)
{
    struct rpr_session s;
    unsigned i;
    int r;
    if (!p || !out)
        return RPR_INVALID;
    if (p->active || p->poisoned)
        return RPR_STATE;
    r = rpr_init(timeline, horizon, initial, &s);
    if (r)
        return r;
    for (i = 0; i < 8; ++i)
    {
        unsigned char key[2] = {'R', (unsigned char)i}, buf[64];
        size_t n;
        int q = store_get(p, key, 2, buf, sizeof buf, &n);
        if (q != STORE_NOT_FOUND)
            return q == STORE_OK ? RPR_STATE : status(p, q);
    }
    r = save(p, &s);
    if (!r)
        *out = (struct rpr_durable){p, s};
    return r;
}

int rpr_open(store *p, uint64_t timeline, struct rpr_durable *out)
{
    struct rpr_session s;
    int r;
    if (!out || !timeline)
        return RPR_INVALID;
    r = load(p, timeline, &s);
    if (!r)
        *out = (struct rpr_durable){p, s};
    return r;
}

int rpr_advance_durable(struct rpr_durable *d, int movement,
                        struct rpr_notice *out)
{
    struct rpr_session next;
    struct rpr_notice n;
    int r;
    if (!out)
        return RPR_INVALID;
    r = current(d);
    if (r)
        return r;
    next = d->session;
    r = rpr_advance(&next, movement, &n);
    if (!r)
        r = publish(d, &next);
    if (!r)
        *out = n;
    return r;
}

int rpr_correct_durable(struct rpr_durable *d, uint64_t timeline, unsigned tick,
                        int movement, struct rpr_notice *out)
{
    struct rpr_session next;
    struct rpr_notice n;
    int r;
    if (!out)
        return RPR_INVALID;
    r = current(d);
    if (r)
        return r;
    next = d->session;
    r = rpr_correct(&next, timeline, tick, movement, &n);
    if (!r)
        r = publish(d, &next);
    if (!r)
        *out = n;
    return r;
}

int rpr_prepare_durable(struct rpr_durable *d, struct rpr_effect *out)
{
    struct rpr_session next;
    struct rpr_effect e;
    int r;
    if (!out)
        return RPR_INVALID;
    r = current(d);
    if (r)
        return r;
    next = d->session;
    r = rpr_prepare(&next, &e);
    if (!r)
        r = publish(d, &next);
    if (!r)
        *out = e;
    return r;
}

int rpr_feedback_durable(struct rpr_durable *d, struct rpr_effect effect,
                         unsigned result)
{
    struct rpr_session next;
    int r = current(d);
    if (r)
        return r;
    next = d->session;
    r = rpr_feedback(&next, effect, result);
    if (!r)
        r = publish(d, &next);
    return r;
}

#include "object_runtime_rc.h"
#ifndef ORR_MUTANT
#define ORR_MUTANT 0
#endif
static uint16_t bit(unsigned i)
{
    return (uint16_t)(1u << i);
}

static int valid(const struct orr_heap *h, struct orr_handle a)
{
    return a.slot >= 1 && a.slot <= 16 && a.generation >= 1 &&
           a.generation <= 3 && h->generation[a.slot - 1] == a.generation &&
           (h->counts.live & bit(a.slot - 1));
}

static int target(const struct orr_heap *h, struct orr_handle a)
{
    return (!a.slot && !a.generation) || valid(h, a);
}

static enum or_status admission(const struct orr_heap *h)
{
    if (h->closed)
        return OR_STATE;
    return h->revision == 65535 ? OR_FULL : OR_OK;
}

static void candidate(struct orr_heap *h, unsigned i)
{
    if (ORR_MUTANT != 4)
        h->candidates |= bit(i);
}

static void retire(struct orr_heap *h, unsigned i)
{
    h->counts.live &= (uint16_t)~bit(i);
    h->candidates &= (uint16_t)~bit(i);
    h->counts.refs[i] = 0;
    h->roots[i] = 0;
    h->generation[i]++;
    h->counts.edge[i][0] = 0;
    h->counts.edge[i][1] = 0;
}

static uint16_t decrement(struct orr_heap *h, unsigned i)
{
    uint16_t dead = 0;
    unsigned j, k;
    h->counts.refs[i]--;
    if (h->counts.refs[i])
        candidate(h, i);
    for (;;)
    {
        for (j = 0; j < 16; j++)
            if ((h->counts.live & bit(j)) && !h->counts.refs[j])
                break;
        if (j == 16)
            break;
        for (k = 0; k < 2; k++)
        {
            unsigned t = h->counts.edge[j][k];
            if (t && ORR_MUTANT != 1)
            {
                h->counts.refs[t - 1]--;
                if (h->counts.refs[t - 1])
                    candidate(h, t - 1);
            }
        }
        dead |= bit(j);
        retire(h, j);
    }
    return dead;
}

static int queue_full(const struct orr_heap *h)
{
    return h->length == 8 && ORR_MUTANT != 3;
}

static void enqueue(struct orr_heap *h, unsigned slot)
{
    /* Mutant3 silently drops the oldest obligation rather than writing out of bounds. */
    if (h->length == 8)
    {
        h->head = (uint8_t)((h->head + 1) % 8);
        h->length--;
    }
    h->pending[(h->head + h->length) % 8] = (uint8_t)slot;
    h->length++;
}

void orr_init(struct orr_heap *h)
{
    unsigned i;
    *h = (struct orr_heap){0};
    h->revision = 1;
    for (i = 0; i < 16; i++)
        h->generation[i] = 1;
}

enum or_status orr_new(struct orr_heap *h, struct orr_handle *out)
{
    unsigned i;
    enum or_status s;
    if (!h || !out)
        return OR_INVALID;
    s = admission(h);
    if (s != OR_OK)
        return s;
    for (i = 0; i < 16; i++)
        if (!(h->counts.live & bit(i)) && h->generation[i] <= 3)
        {
            h->counts.live |= bit(i);
            h->counts.refs[i] = 1;
            h->roots[i] = 1;
            h->revision++;
            *out = (struct orr_handle){(uint8_t)(i + 1), h->generation[i]};
            return OR_OK;
        }
    return OR_FULL;
}

enum or_status orr_retain(struct orr_heap *h, struct orr_handle a)
{
    enum or_status s;
    if (!h)
        return OR_INVALID;
    s = admission(h);
    if (s != OR_OK)
        return s;
    if (!valid(h, a))
        return OR_INVALID;
    if (h->roots[a.slot - 1] == 255)
        return OR_FULL;
    h->roots[a.slot - 1]++;
    h->counts.refs[a.slot - 1]++;
    h->revision++;
    return OR_OK;
}

enum or_status orr_release(struct orr_heap *h, struct orr_handle a, int defer,
                           uint16_t *out)
{
    enum or_status s;
    uint16_t dead = 0;
    if (!h || !out || (defer != 0 && defer != 1))
        return OR_INVALID;
    s = admission(h);
    if (s != OR_OK)
        return s;
    if (!valid(h, a))
        return OR_INVALID;
    if (!h->roots[a.slot - 1] && ORR_MUTANT != 2)
        return OR_STATE;
    if (defer && queue_full(h))
        return OR_FULL;
    h->roots[a.slot - 1]--;
    if (defer)
        enqueue(h, a.slot);
    else
        dead = decrement(h, a.slot - 1);
    h->revision++;
    *out = dead;
    return OR_OK;
}

enum or_status orr_store(struct orr_heap *h, struct orr_handle a,
                         unsigned field, struct orr_handle b, int defer,
                         uint16_t *out)
{
    enum or_status s;
    unsigned old;
    uint16_t dead = 0;
    if (!h || !out || field > 1 || (defer != 0 && defer != 1))
        return OR_INVALID;
    s = admission(h);
    if (s != OR_OK)
        return s;
    if (!valid(h, a) || !target(h, b))
        return OR_INVALID;
    old = h->counts.edge[a.slot - 1][field];
    if (old != b.slot)
    {
        if (old && defer && queue_full(h))
            return OR_FULL;
        if (b.slot)
            h->counts.refs[b.slot - 1]++;
        h->counts.edge[a.slot - 1][field] = b.slot;
        if (old)
        {
            if (defer)
                enqueue(h, old);
            else
                dead = decrement(h, old - 1);
        }
    }
    h->revision++;
    *out = dead;
    return OR_OK;
}

enum or_status orr_flush(struct orr_heap *h, unsigned budget,
                         unsigned *processed, uint16_t *out)
{
    enum or_status s;
    unsigned n = 0;
    uint16_t dead = 0;
    if (!h || !processed || !out)
        return OR_INVALID;
    s = admission(h);
    if (s != OR_OK)
        return s;
    while (n < budget && h->length)
    {
        unsigned slot = h->pending[h->head];
        h->pending[h->head] = 0;
        h->head = (uint8_t)((h->head + 1) % 8);
        h->length--;
        n++;
        dead |= decrement(h, slot - 1);
    }
    if (n)
        h->revision++;
    *processed = n;
    *out = dead;
    return h->length ? OR_OK : OR_DONE;
}

enum or_status orr_prepare(struct orr_heap *h, struct orr_trial *out)
{
    struct orr_trial t;
    uint16_t closure, old;
    unsigned i, j;
    enum or_status s;
    if (!h || !out)
        return OR_INVALID;
    if (h->closed || (h->length && ORR_MUTANT != 6))
        return OR_STATE;
    closure = h->candidates;
    do
    {
        old = closure;
        for (i = 0; i < 16; i++)
            if (closure & bit(i))
                for (j = 0; j < 2; j++)
                {
                    unsigned child = h->counts.edge[i][j];
                    if (child)
                        closure |= bit(child - 1);
                }
    } while (closure != old);
    t = (struct orr_trial){0};
    t.owner = h;
    t.result = h->counts;
    t.revision = h->revision;
    s = or_trial_delete(&t.result, closure, &t.dead);
    if (s != OR_OK)
        return s;
    *out = t;
    return OR_OK;
}

enum or_status orr_commit(struct orr_heap *h, const struct orr_trial *t,
                          uint16_t *out)
{
    enum or_status s;
    unsigned i;
    if (!h || !t || !out)
        return OR_INVALID;
    s = admission(h);
    if (s != OR_OK)
        return s;
    if (t->owner != h || h->length ||
        (t->revision != h->revision && ORR_MUTANT != 5))
        return OR_STATE;
    h->counts = t->result;
    for (i = 0; i < 16; i++)
        if (t->dead & bit(i))
            retire(h, i);
    h->candidates = 0;
    h->revision++;
    *out = t->dead;
    return OR_OK;
}

enum or_status orr_close(struct orr_heap *h)
{
    if (!h)
        return OR_INVALID;
    if (h->closed)
        return OR_STATE;
    h->counts = (struct or_counts){0};
    h->length = 0;
    h->candidates = 0;
    h->closed = 1;
    return OR_OK;
}

int orr_invariant(const struct orr_heap *h)
{
    unsigned sums[16] = {0}, i, j;
    if (!h || h->closed || !h->revision || h->length > 8 || h->head > 7 ||
        (h->candidates & ~h->counts.live))
        return 0;
    for (i = 0; i < 16; i++)
    {
        if (h->generation[i] < 1 || h->generation[i] > 4)
            return 0;
        if (h->counts.live & bit(i))
        {
            if (h->generation[i] > 3 || !h->counts.refs[i])
                return 0;
            sums[i] += h->roots[i];
            for (j = 0; j < 2; j++)
            {
                unsigned t = h->counts.edge[i][j];
                if (t > 16 || (t && !(h->counts.live & bit(t - 1))))
                    return 0;
                if (t)
                    sums[t - 1]++;
            }
        }
        else if (h->roots[i] || h->counts.refs[i] || h->counts.edge[i][0] ||
                 h->counts.edge[i][1])
            return 0;
    }
    for (i = 0; i < h->length; i++)
    {
        unsigned t = h->pending[(h->head + i) % 8];
        if (!t || t > 16 || !(h->counts.live & bit(t - 1)))
            return 0;
        sums[t - 1]++;
    }
    for (i = 0; i < 16; i++)
        if (sums[i] != h->counts.refs[i])
            return 0;
    return 1;
}

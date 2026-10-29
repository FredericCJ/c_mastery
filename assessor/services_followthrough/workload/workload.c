#include "workload.h"

void sfw_init(struct sf_work *s)
{
    *s = (struct sf_work){0};
    af_ring_init(&s->ring, UINT32_MAX - 7);
    atomic_init(&s->phase, 0);
}

int sfw_offer(struct sf_work *s, unsigned burst)
{
    if (!s || (burst != 1 && burst != 4))
    {
        return AF_INVALID;
    }
    for (unsigned n = 0; n < burst && s->offered < 128; n++)
    {
        uint32_t id = ++s->offered;
        struct af_message m = {1, id, 4, {0}};
        for (unsigned i = 0; i < 4; i++)
        {
            m.bytes[i] = (uint8_t)(id >> (8 * i));
        }
        int r = af_batch(&s->ring, 1, &m, 1);
        if (r == AF_FULL)
        {
            s->rejected++;
            s->state[id - 1] = 3;
        }
        else if (r)
        {
            return r;
        }
        else
        {
            s->admitted++;
            s->state[id - 1] = 1;
        }
    }
    return AF_OK;
}

int sfw_work(struct sf_work *s)
{
    unsigned char bytes[4];
    size_t length;
    uint32_t id;
    int r = af_receive(&s->ring, 1, bytes, sizeof bytes, &id, &length);
    if (r)
    {
        return r;
    }
    uint32_t n = (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 |
                 (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
    if (length != 4 || n != id || !id || id > 128 || s->state[id - 1] != 1)
    {
        return AF_INVALID;
    }
    s->state[id - 1] = 2;
    s->completed++;
    s->checksum += n + 1;
    return AF_OK;
}

int sfw_quiescent(struct sf_work *s, unsigned peer, unsigned device)
{
    if (!s || !peer || !device)
    {
        return AF_BUSY;
    }
    while (af_queued(&s->ring))
    {
        unsigned char bytes[4];
        size_t length;
        uint32_t id;
        if (af_receive(&s->ring, 1, bytes, 4, &id, &length) || !id ||
            id > 128 || s->state[id - 1] != 1)
        {
            return AF_INVALID;
        }
        s->state[id - 1] = 4;
        s->uncertain++;
    }
    return AF_OK;
}

int sfw_check(const struct sf_work *s)
{
    if (!s || s->offered != 128 || s->admitted + s->rejected != 128 ||
        s->completed + s->uncertain != s->admitted || af_queued(&s->ring))
    {
        return AF_INVALID;
    }
    unsigned counts[5] = {0};
    uint32_t sum = 0;
    for (unsigned i = 0; i < 128; i++)
    {
        if (s->state[i] > 4)
        {
            return AF_INVALID;
        }
        counts[s->state[i]]++;
        if (s->state[i] == 2)
        {
            sum += i + 2;
        }
    }
    return counts[0] || counts[1] || counts[2] != s->completed ||
                   counts[3] != s->rejected || counts[4] != s->uncertain ||
                   sum != s->checksum
               ? AF_INVALID
               : AF_OK;
}

int sfw_run(struct sf_work *s, unsigned burst)
{
    sfw_init(s);
    while (s->offered < 128)
    {
        int r = sfw_offer(s, burst);
        if (r)
        {
            return r;
        }
        r = sfw_work(s);
        if (r)
        {
            return r;
        }
    }
    while (af_queued(&s->ring) > 2)
    {
        if (sfw_work(s))
        {
            return AF_INVALID;
        }
    }
    if (sfw_quiescent(s, 0, 1) != AF_BUSY || sfw_quiescent(s, 1, 0) != AF_BUSY)
    {
        return AF_INVALID;
    }
    int r = sfw_quiescent(s, 1, 1);
    return r ? r : sfw_check(s);
}

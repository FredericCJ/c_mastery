#include "observability.h"

static void increment(unsigned *p)
{
    if (*p < 65535)
        ++*p;
}

enum ae_code ae_record_encode(const struct ae_record *r, unsigned char *out,
                              size_t n)
{
    if (!r || !out || n < 12 || r->severity > 3 || r->unit < 1 || r->unit > 2 ||
        (r->width != 8 && r->width != 16) || r->generation < 1 ||
        r->generation > 3 || r->length > 4 ||
        r->tick > (r->width == 8 ? 255u : 65535u))
        return AE_INVALID;
    unsigned char b[12] = {1,
                           (unsigned char)r->severity,
                           (unsigned char)r->unit,
                           (unsigned char)r->width,
                           (unsigned char)r->generation,
                           (unsigned char)r->length,
                           (unsigned char)(r->tick & 255),
                           (unsigned char)(r->tick >> 8),
                           0,
                           0,
                           0,
                           0};
    for (unsigned i = 0; i < r->length; ++i)
        b[8 + i] = r->payload[i];
    for (unsigned i = 0; i < 12; ++i)
        out[i] = b[i];
    return AE_OK;
}

enum ae_code ae_record_decode(const unsigned char *p, size_t n,
                              struct ae_record *out)
{
    if (!p || !out || n != 12 || p[0] != 1)
        return AE_INVALID;
    struct ae_record r = {
        p[1], p[2], p[3], p[4], p[5], (unsigned)p[6] + 256u * p[7], {0}};
    if (r.length > 4)
        return AE_INVALID;
    for (unsigned i = 0; i < 4; ++i)
    {
        if (i >= r.length && p[8 + i])
            return AE_INVALID;
        r.payload[i] = p[8 + i];
    }
    unsigned char b[12];
    if (ae_record_encode(&r, b, 12) != AE_OK)
        return AE_INVALID;
    *out = r;
    return AE_OK;
}

enum ae_code ae_tick_order(unsigned a, unsigned b, unsigned width, int *out)
{
    if (!out || (width != 8 && width != 16))
        return AE_INVALID;
    unsigned modulus = width == 8 ? 256u : 65536u;
    if (a >= modulus || b >= modulus)
        return AE_INVALID;
    unsigned delta = (a + modulus - b) % modulus;
    if (delta == modulus / 2)
        return AE_AMBIGUOUS;
    *out = delta == 0 ? 0 : delta < modulus / 2 ? 1 : -1;
    return AE_OK;
}

enum ae_code ae_rate_refill(struct ae_rate *r, unsigned elapsed)
{
    if (!r || r->tokens > 3 || r->remainder > 1 || elapsed > 127)
        return AE_INVALID;
    unsigned total = elapsed + r->remainder, add = total / 2;
    r->tokens = r->tokens + add > 3 ? 3 : r->tokens + add;
    r->remainder = total % 2;
    return AE_OK;
}

enum ae_code ae_log(struct ae_log *l, const struct ae_record *r,
                    enum ae_log_policy policy)
{
    unsigned char b[12];
    if (!l || l->count > 3 || l->rate.tokens > 3 ||
        (unsigned)policy > AE_PROTECT_HIGH ||
        ae_record_encode(r, b, 12) != AE_OK)
        return AE_INVALID;
    if (!l->rate.tokens)
    {
        increment(&l->limited);
        return AE_AGAIN;
    }
    if (l->count == 3 && policy == AE_PROTECT_HIGH)
    {
        unsigned high = r->severity >= 2;
        for (unsigned i = 0; i < 3; ++i)
            high |= l->records[i].severity >= 2;
        if (high)
        {
            increment(&l->rejected);
            return AE_FULL;
        }
    }
    if (l->count == 3)
    {
        for (unsigned i = 1; i < 3; ++i)
            l->records[i - 1] = l->records[i];
        --l->count;
        increment(&l->dropped);
    }
    --l->rate.tokens;
    l->records[l->count++] = *r;
    return AE_OK;
}

enum ae_code ae_log_drain(struct ae_log *l, ae_sink_fn sink, void *context)
{
    if (!l || !sink || l->count > 3)
        return AE_INVALID;
    if (!l->count)
        return AE_OK;
    unsigned char b[12];
    if (ae_record_encode(&l->records[0], b, 12) != AE_OK)
        return AE_CONTRACT;
    int result = sink(context, b, 12);
    if (result == 0)
    {
        increment(&l->stalls);
        return AE_AGAIN;
    }
    if (result != 1)
        return AE_DEVICE;
    for (unsigned i = 1; i < l->count; ++i)
        l->records[i - 1] = l->records[i];
    --l->count;
    return AE_OK;
}

static void fallback(struct ae_reporter *r, const struct ae_record *record)
{
    if (r->fallback_available && r->stored < 2)
        r->fallback[r->stored++] = *record;
    else
        increment(&r->lost);
}

enum ae_code ae_report(struct ae_reporter *r, const struct ae_record *record,
                       ae_sink_fn sink, void *context)
{
    unsigned char b[12];
    if (!r || !sink || r->stored > 2 ||
        ae_record_encode(record, b, 12) != AE_OK)
        return AE_INVALID;
    if (r->busy)
    {
        fallback(r, record);
        return AE_AGAIN;
    }
    r->busy = 1;
    int result = sink(context, b, 12);
    r->busy = 0;
    if (result != 1)
    {
#ifndef AE_MUTANT_NO_FALLBACK
        fallback(r, record);
#endif
        return result == 0 ? AE_AGAIN : AE_DEVICE;
    }
    return AE_OK;
}

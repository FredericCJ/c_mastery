#include "peripheral.h"
#ifndef DP_MUTANT
#define DP_MUTANT 0
#endif
int dp_predict(unsigned c, unsigned *out)
{
    unsigned r = 0;
    if (!out || c > 31)
    {
        return DP_INVALID;
    }
    if (c & DP_GATE)
    {
        r |= DP_READBACK;
    }
    if ((c & (DP_GATE | DP_SEQUENCE)) == (DP_GATE | DP_SEQUENCE))
    {
        r |= DP_ACCEPTED;
    }
    if ((r & DP_ACCEPTED) && (c & DP_MODE) &&
        ((c & DP_ROUTE) || DP_MUTANT == 1))
    {
        r |= DP_EDGE;
    }
    if ((r & DP_EDGE) && (c & DP_TRIGGER))
    {
        r |= DP_CAPTURED;
    }
    *out = r;
    return DP_OK;
}

int dp_correlate(const struct dp_calibration *c, uint64_t tick, uint64_t begin,
                 uint64_t end, struct dp_correlation *out)
{
    uint64_t d, center;
    struct dp_correlation r;
    if (!c || !out || !c->ns_per_tick || c->provenance > DP_MEASURED ||
        begin > end || tick < c->anchor_tick)
    {
        return DP_INVALID;
    }
    d = tick - c->anchor_tick;
    if (d > UINT64_MAX / c->ns_per_tick)
    {
        return DP_RANGE;
    }
    d *= c->ns_per_tick;
    if (d > UINT64_MAX - c->reference_ns)
    {
        return DP_RANGE;
    }
    center = c->reference_ns + d;
    if (center < c->uncertainty_ns || c->uncertainty_ns > UINT64_MAX - center)
    {
        return DP_RANGE;
    }
    r.earliest_ns = center - c->uncertainty_ns;
    r.latest_ns = center + c->uncertainty_ns;
    r.compatible = r.earliest_ns <= end && begin <= r.latest_ns;
    r.provenance = DP_MUTANT == 2 ? DP_MEASURED : c->provenance;
    *out = r;
    return DP_OK;
}

int dp_spi_end(unsigned mode, unsigned txe, unsigned busy, unsigned received,
               unsigned seen, uint32_t elapsed, uint32_t bound)
{
    if (mode > DP_SLAVE_TRANSMIT || txe > 1 || busy > 1 || received > 1 ||
        seen > 1)
    {
        return DP_INVALID;
    }
    if (mode == DP_MASTER_DUPLEX)
    {
        return txe && received && (!busy || DP_MUTANT == 3) ? DP_OK : DP_WAIT;
    }
    if (mode == DP_SLAVE_RECEIVE)
    {
        return received ? DP_OK : DP_WAIT;
    }
    if (!bound || bound >= UINT32_C(0x80000000) ||
        elapsed >= UINT32_C(0x80000000))
    {
        return DP_INVALID;
    }
    if (!seen || !txe)
    {
        return DP_WAIT;
    }
    return !busy || elapsed >= bound ? DP_OK : DP_WAIT;
}

int dp_progress(unsigned n, uint64_t halted, unsigned frozen, unsigned budget,
                unsigned bytes, struct dp_progress *out)
{
    struct dp_progress r = {0};
    unsigned remaining = bytes, wd = budget;
    if (!out || !n || n > 64 || frozen > 1 || !budget || budget > 64 ||
        !bytes || bytes > 32)
    {
        return DP_INVALID;
    }
    for (unsigned i = 0; i < n; i++)
    {
        unsigned stop = (unsigned)((halted >> i) & 1u);
        if (remaining && !(DP_MUTANT == 4 && stop))
        {
            remaining--;
            if (!remaining)
            {
                r.dma_complete_at = i + 1;
            }
        }
        if (!(stop && frozen))
        {
            wd--;
            if (!wd)
            {
                r.reset_at = i + 1;
                break;
            }
        }
        if (!stop)
        {
            if (!remaining && !r.observed_at)
            {
                r.observed_at = i + 1;
            }
            wd = budget;
            r.fed++;
        }
    }
    r.remaining = remaining;
    *out = r;
    return DP_OK;
}

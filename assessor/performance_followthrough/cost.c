#include "cost.h"

static void add(uint64_t *v, uint64_t n, struct pf_work *w)
{
    if (n > UINT64_MAX - *v)
    {
        w->available = 0;
        *v = UINT64_MAX;
    }
    else
        *v += n;
}

static int compare(struct pf_bytes a, struct pf_bytes b, struct pf_work *w)
{
    add(&w->calls, 1, w);
    size_t n = a.length < b.length ? a.length : b.length;
    for (size_t i = 0; i < n; ++i)
    {
        add(&w->bytes, 1, w);
        if (a.data[i] != b.data[i])
            return a.data[i] < b.data[i] ? -1 : 1;
    }
    return (a.length > b.length) - (a.length < b.length);
}

int pf_sort(struct pf_bytes *a, size_t n, unsigned binary, struct pf_work *w)
{
    if (!w || (!a && n) || n > 256 || binary > 1)
        return 1;
    for (size_t i = 0; i < n; ++i)
        if (a[i].length > 1024 || (!a[i].data && a[i].length))
            return 1;
    for (size_t i = 1; i < n; ++i)
    {
        struct pf_bytes x = a[i];
        size_t j = i;
        if (binary)
        {
            size_t lo = 0, hi = i;
            while (lo < hi)
            {
                size_t mid = lo + (hi - lo) / 2;
                if (compare(a[mid], x, w) <= 0)
                    lo = mid + 1;
                else
                    hi = mid;
            }
            j = lo;
            for (size_t k = i; k > j; --k)
            {
                a[k] = a[k - 1];
                add(&w->shifts, 1, w);
            }
        }
        else
            while (j && compare(a[j - 1], x, w) > 0)
            {
                a[j] = a[j - 1];
                --j;
                add(&w->shifts, 1, w);
            }
        a[j] = x;
        add(&w->placements, 1, w);
    }
    return 0;
}

int pf_cache(struct pf_cache *c, unsigned item, unsigned writing)
{
    if (!c || item >= 6 || writing > 1 || c->fifo > 1 ||
        c->tick == UINT32_MAX || c->useful > UINT64_MAX - 4)
        return 1;
    for (unsigned i = 0; i < 2; ++i)
        if (c->slots[i].valid > 1 || c->slots[i].dirty > 1 ||
            c->slots[i].block >= 3 || c->slots[i].age > c->tick)
            return 1;
    struct pf_cache v = *c;
    unsigned block = item / 2, slot = 2;
    for (unsigned i = 0; i < 2; ++i)
        if (v.slots[i].valid && v.slots[i].block == block)
            slot = i;
    if (slot == 2)
    {
        for (unsigned i = 0; i < 2; ++i)
            if (!v.slots[i].valid)
            {
                slot = i;
                break;
            }
        if (slot == 2)
            slot = v.slots[0].age < v.slots[1].age ? 0 : 1;
        if (v.reads > UINT64_MAX - 8 ||
            (v.slots[slot].dirty && v.writes > UINT64_MAX - 8))
            return 1;
        if (v.slots[slot].dirty)
            v.writes += 8;
        v.reads += 8;
        v.slots[slot] = (struct pf_slot){1, 0, block, ++v.tick};
    }
    else
    {
        ++v.tick;
        if (!v.fifo)
            v.slots[slot].age = v.tick;
    }
    v.slots[slot].dirty |= writing;
    v.useful += 4;
    *c = v;
    return 0;
}

int pf_flush(struct pf_cache *c)
{
    if (!c)
        return 1;
    struct pf_cache v = *c;
    for (unsigned i = 0; i < 2; ++i)
        if (v.slots[i].dirty)
        {
            if (v.writes > UINT64_MAX - 8)
                return 1;
            v.writes += 8;
            v.slots[i].dirty = 0;
        }
    *c = v;
    return 0;
}

int pf_interval(uint64_t s, unsigned n, uint64_t e, unsigned m, uint64_t *out)
{
    if (!out || n >= 1000000000u || m >= 1000000000u || e < s ||
        (e == s && m < n))
        return 1;
    uint64_t sec = e - s;
    unsigned ns;
    if (m < n)
    {
        --sec;
        ns = 1000000000u - n + m;
    }
    else
        ns = m - n;
    if (sec > (UINT64_MAX - ns) / UINT64_C(1000000000))
        return 1;
    *out = sec * UINT64_C(1000000000) + ns;
    return 0;
}

static uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b)
    {
        uint64_t x = a % b;
        a = b;
        b = x;
    }
    return a;
}

static int ratio(uint64_t a, uint64_t b, uint64_t d, struct pf_ratio *out)
{
    if (!d)
        return 1;
    uint64_t g = gcd(a, d);
    a /= g;
    d /= g;
    g = gcd(b, d);
    b /= g;
    d /= g;
    if (b && a > UINT64_MAX / b)
        return 1;
    *out = (struct pf_ratio){a * b, d};
    return 0;
}

int pf_scale(struct pf_event a, struct pf_ratio *out)
{
    if (!out || !a.running || a.running > a.enabled)
        return 1;
    return ratio(a.raw, a.enabled, a.running, out);
}

int pf_cpi(struct pf_event a, struct pf_event b, struct pf_ratio *out)
{
    struct pf_ratio x, y;
    if (!out || !a.raw || a.group != b.group || a.id == b.id ||
        a.enabled != b.enabled || a.running != b.running || pf_scale(a, &x) ||
        pf_scale(b, &y))
        return 1;
    return ratio(b.raw, 1, a.raw, out);
}

int pf_profile(const struct pf_hook *e, size_t n, unsigned dropped,
               struct pf_profile *out)
{
    if (!out || (!e && n) || n > 128)
        return 1;
    struct pf_profile r = {{0}, {0}, {0}, 0};

    struct ad_frame
    {
        unsigned id;
        uint64_t start, child;
    } stack[4][16];

    unsigned top[4] = {0};
    uint64_t last[4] = {0};
    unsigned seen[4] = {0};
    int good = !dropped;
    for (size_t i = 0; i < n; ++i)
    {
        struct pf_hook h = e[i];
        if (h.task >= 4 || h.function >= 8 || h.enter > 1)
        {
            good = 0;
            break;
        }
        unsigned t = h.task;
        if (seen[t] && h.time < last[t])
        {
            good = 0;
            break;
        }
        last[t] = h.time;
        seen[t] = 1;
        if (h.enter)
        {
            if (top[t] == 16)
            {
                good = 0;
                break;
            }
            stack[t][top[t]++] = (struct ad_frame){h.function, h.time, 0};
        }
        else
        {
            if (!top[t] || stack[t][top[t] - 1].id != h.function)
            {
                good = 0;
                break;
            }
            struct ad_frame f = stack[t][--top[t]];
            uint64_t d = h.time - f.start;
            if (f.child > d || r.inclusive[f.id] > UINT64_MAX - d ||
                r.exclusive[f.id] > UINT64_MAX - (d - f.child) ||
                r.calls[f.id] == UINT64_MAX)
            {
                good = 0;
                break;
            }
            r.inclusive[f.id] += d;
            r.exclusive[f.id] += d - f.child;
            ++r.calls[f.id];
            if (top[t])
            {
                struct ad_frame *p = &stack[t][top[t] - 1];
                if (p->child > UINT64_MAX - d)
                {
                    good = 0;
                    break;
                }
                p->child += d;
            }
        }
    }
    for (unsigned t = 0; t < 4; ++t)
        if (top[t])
            good = 0;
    if (!good)
        r = (struct pf_profile){{0}, {0}, {0}, 0};
    else
        r.complete = 1;
    *out = r;
    return 0;
}

int pf_cycle(struct pf_cycle c, struct pf_cycle_result *out)
{
    if (!out || !c.hz)
        return PF_CYCLE_OVERFLOW;
    if (!c.trcena)
        return PF_CYCLE_UNKNOWN;
    if (c.nocyc)
        return PF_CYCLE_ABSENT;
    if (!c.owned)
        return PF_CYCLE_UNOWNED;
    if (!c.enabled)
        return PF_CYCLE_DISABLED;
    if (c.bound >= UINT64_C(4294967296))
        return PF_CYCLE_AMBIGUOUS;
    struct pf_cycle_result r = {0};
    r.delta = c.end - c.start;
    if (r.delta > c.bound)
        return PF_CYCLE_BOUND;
    if (c.halted)
    {
        *out = r;
        return PF_CYCLE_HALTED;
    }
    if (ratio(r.delta, 1000000000, c.hz, &r.ns))
        return PF_CYCLE_OVERFLOW;
    if (c.overhead <= r.delta)
    {
        if (ratio(r.delta - c.overhead, 1000000000, c.hz, &r.sensitivity_ns))
            return PF_CYCLE_OVERFLOW;
        r.sensitivity_valid = 1;
    }
    *out = r;
    return PF_CYCLE_OK;
}

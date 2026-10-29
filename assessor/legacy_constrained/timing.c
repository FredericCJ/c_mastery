#include "timing.h"
#ifndef CT_MUTANT
#define CT_MUTANT 0
#endif
static uint32_t modulus(unsigned bits)
{
    return bits == 8 ? 256u : bits == 16 ? 65536u : 0;
}

int ct_timeout(uint64_t nanoseconds, unsigned bits, uint64_t quantum,
               uint32_t *out)
{
    uint32_t mod = modulus(bits);
    if (!mod || !quantum || !out)
        return CT_INVALID;
    uint64_t ticks = nanoseconds / quantum;
    if (nanoseconds % quantum && CT_MUTANT != 1)
        ticks++;
    if (ticks >= mod / 2)
        return CT_RANGE;
    *out = (uint32_t)ticks;
    return CT_OK;
}

int ct_order(unsigned bits, uint32_t now, uint32_t deadline, unsigned *out)
{
    uint32_t mod = modulus(bits);
    if (!mod || now >= mod || deadline >= mod || !out)
        return CT_INVALID;
    uint32_t distance = (now + mod - deadline) % mod;
    if (distance == mod / 2 && CT_MUTANT != 2)
        return CT_AMBIGUOUS;
    *out = distance == 0 ? CT_EQUAL : distance < mod / 2 ? CT_DUE : CT_FUTURE;
    return CT_OK;
}

int ct_elapsed(unsigned bits, uint32_t then, uint32_t now, int bounded,
               uint32_t *out)
{
    uint32_t mod = modulus(bits);
    if (!mod || then >= mod || now >= mod || !out ||
        (bounded != 0 && bounded != 1))
        return CT_INVALID;
    if (!bounded)
        return CT_AMBIGUOUS;
    *out = (now + mod - then) % mod;
    return CT_OK;
}

static int counter(struct ct_stamp s, uint64_t *out)
{
    uint32_t mod = modulus(s.bits);
    if (!s.boot || !s.quantum_ns || !mod || s.low >= mod)
        return CT_INVALID;
    if (s.epoch > (UINT64_MAX - s.low) / mod)
        return CT_RANGE;
    *out = s.epoch * mod + s.low;
    return CT_OK;
}

int ct_clock_init(struct ct_clock *c, struct ct_stamp s)
{
    uint64_t count;
    if (!c)
        return CT_INVALID;
    int status = counter(s, &count);
    if (status)
        return status;
    atomic_init(&c->sequence, 0);
    atomic_init(&c->boot, s.boot);
    atomic_init(&c->epoch, s.epoch);
    atomic_init(&c->quantum_ns, s.quantum_ns);
    atomic_init(&c->low, s.low);
    atomic_init(&c->bits, s.bits);
    return CT_OK;
}

static int snapshot(struct ct_clock *c, unsigned attempts, ct_hook_fn hook,
                    void *context, struct ct_stamp *out, uint64_t *revision)
{
    if (!c || !out || !attempts || attempts > 1024)
        return CT_INVALID;
    for (unsigned i = 0; i < attempts; i++)
    {
        uint64_t first = atomic_load(&c->sequence);
        if (first & 1)
            continue;
        struct ct_stamp s;
        s.boot = atomic_load(&c->boot);
        s.epoch = atomic_load(&c->epoch);
        if (hook)
            hook(context, 0);
        s.low = atomic_load(&c->low);
        s.bits = atomic_load(&c->bits);
        s.quantum_ns = atomic_load(&c->quantum_ns);
        if (hook)
            hook(context, 1);
        uint64_t last = atomic_load(&c->sequence);
        if (first == last || CT_MUTANT == 3)
        {
            *out = s;
            if (revision)
                *revision = last;
            return CT_OK;
        }
    }
    return CT_BUSY;
}

int ct_snapshot(struct ct_clock *c, unsigned attempts, ct_hook_fn hook,
                void *context, struct ct_stamp *out)
{
    return snapshot(c, attempts, hook, context, out, NULL);
}

int ct_publish(struct ct_clock *c, struct ct_stamp next)
{
    if (!c)
        return CT_INVALID;
    uint64_t next_count, previous_count, seq;
    int status = counter(next, &next_count);
    if (status)
        return status;
    struct ct_stamp previous;
    status = snapshot(c, 1, NULL, NULL, &previous, &seq);
    if (status)
        return status;
    status = counter(previous, &previous_count);
    if (status)
        return status;
    if (next.boot < previous.boot)
        return CT_DIFFERENT;
    if (next.boot == previous.boot &&
        (next.bits != previous.bits || next.quantum_ns != previous.quantum_ns ||
         next_count < previous_count))
        return CT_DIFFERENT;
    if (seq > UINT64_MAX - 3)
        return CT_RANGE;
    if (!atomic_compare_exchange_strong(&c->sequence, &seq, seq + 1))
        return CT_BUSY;
    atomic_store(&c->boot, next.boot);
    atomic_store(&c->epoch, next.epoch);
    atomic_store(&c->low, next.low);
    atomic_store(&c->bits, next.bits);
    atomic_store(&c->quantum_ns, next.quantum_ns);
    atomic_store(&c->sequence, seq + 2);
    return CT_OK;
}

int ct_duration(struct ct_stamp a, struct ct_stamp b, uint64_t *out)
{
    uint64_t first, last;
    if (!out)
        return CT_INVALID;
    int status = counter(a, &first);
    if (status)
        return status;
    status = counter(b, &last);
    if (status)
        return status;
    if ((a.boot != b.boot && CT_MUTANT != 4) || a.bits != b.bits ||
        a.quantum_ns != b.quantum_ns || last < first)
        return CT_DIFFERENT;
    uint64_t delta = last - first;
    if (delta > UINT64_MAX / a.quantum_ns)
        return CT_RANGE;
    *out = delta * a.quantum_ns;
    return CT_OK;
}

static void put(unsigned char *p, uint64_t n, unsigned bytes)
{
    for (unsigned i = 0; i < bytes; i++)
    {
        p[i] = (unsigned char)(n & 255u);
        n >>= 8;
    }
}

static uint64_t get(const unsigned char *p, unsigned bytes)
{
    uint64_t n = 0;
    for (unsigned i = 0; i < bytes; i++)
        n |= (uint64_t)p[i] << (i * 8);
    return n;
}

int ct_encode(struct ct_stamp s, unsigned char out[40])
{
    uint64_t n;
    if (!out)
        return CT_INVALID;
    int status = counter(s, &n);
    if (status)
        return status;
    const unsigned char magic[4] = {'C', 'T', 'S', '1'};
    for (unsigned i = 0; i < 4; i++)
        out[i] = magic[i];
    put(out + 4, 1, 4);
    put(out + 8, s.boot, 8);
    put(out + 16, s.epoch, 8);
    put(out + 24, s.low, 4);
    put(out + 28, s.bits, 4);
    put(out + 32, s.quantum_ns, 8);
    return CT_OK;
}

int ct_decode(const unsigned char *input, size_t length, struct ct_stamp *out)
{
    if (!input || !out || length != 40)
        return CT_INVALID;
    if (input[0] != 'C' || input[1] != 'T' || input[2] != 'S' ||
        input[3] != '1' || get(input + 4, 4) != 1)
        return CT_INVALID;
    uint64_t bits = get(input + 28, 4);
    if (bits != 8 && bits != 16)
        return CT_INVALID;
    struct ct_stamp s = {get(input + 8, 8), get(input + 16, 8),
                         get(input + 32, 8), (uint32_t)get(input + 24, 4),
                         (unsigned)bits};
    uint64_t n;
    int status = counter(s, &n);
    if (status)
        return status;
    *out = s;
    return CT_OK;
}

int ct_metrics(uint64_t release, uint64_t start, uint64_t finish,
               const struct ct_slice *slices, size_t count,
               struct ct_metrics *out)
{
    if (!out || (!slices && count) || count > 64 || release > start ||
        start > finish)
        return CT_INVALID;
    struct ct_metrics r = {.latency = start - release,
                           .elapsed = finish - start,
                           .response = finish - release};
    uint64_t cursor = start, parts[5] = {0};
    for (size_t i = 0; i < count; i++)
    {
        struct ct_slice s = slices[i];
        if (s.kind > 4 || s.begin != cursor || s.end <= s.begin ||
            s.end > finish)
            return CT_INVALID;
        parts[s.kind] += s.end - s.begin;
        cursor = s.end;
    }
    if (cursor != finish)
        return CT_INVALID;
    r.cpu = parts[0];
    r.isr = parts[1];
    r.preempted = parts[2];
    r.dma = parts[3];
    r.blocked = parts[4];
    if (CT_MUTANT == 5)
        r.response += r.isr;
    if (CT_MUTANT == 6)
    {
        r.cpu += r.dma;
        r.dma = 0;
    }
    *out = r;
    return CT_OK;
}

int ct_jitter(uint64_t ideal, uint64_t actual, struct ct_offset *out)
{
    if (!out)
        return CT_INVALID;
    *out = (struct ct_offset){actual < ideal,
                              actual < ideal ? ideal - actual : actual - ideal};
    return CT_OK;
}

int ct_stats(const uint64_t *input, size_t count, uint64_t threshold,
             struct ct_stats *out)
{
    if (!input || !out || !count || count > 128)
        return CT_INVALID;
    struct ct_stats r = {
        .minimum = input[0], .maximum = input[0], .count = count};
    size_t run = 0;
    for (size_t i = 0; i < count; i++)
    {
        uint64_t v = input[i];
        if (UINT64_MAX - r.sum < v)
            return CT_RANGE;
        r.sum += v;
        if (v < r.minimum)
            r.minimum = v;
        if (v > r.maximum)
            r.maximum = v;
        if (v > threshold)
        {
            if (!run)
                r.exceedance_runs++;
            run++;
            if (run > r.longest_exceedance_run)
                r.longest_exceedance_run = run;
        }
        else
            run = 0;
    }
    r.mean_integer = r.sum / count;
    r.mean_remainder = r.sum % count;
    if (CT_MUTANT == 7)
        r.longest_exceedance_run = r.maximum > threshold;
    *out = r;
    return CT_OK;
}

#include "cache.h"
#define CHECK(x)                       \
    do                                 \
    {                                  \
        if (!(x))                      \
        {                              \
            ach_failure(#x, __LINE__); \
            return 41;                 \
        }                              \
    } while (0)

static int name(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

static int equal(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
    {
        if (x[i] != y[i])
        {
            return 0;
        }
    }
    return 1;
}

static void trace(unsigned key, const int64_t *a, size_t n, struct ach_config c,
                  const struct ach_trace *r)
{
    uint32_t head[] = {key,         (uint32_t)n,   c.lines, c.block_bytes,
                       c.ways,      c.shadow_fifo, r->hits, r->compulsory,
                       r->conflict, r->capacity};
    ach_emit("head", head, 10);
    for (unsigned i = 0; i < n; ++i)
    {
        const struct ach_step *s = &r->steps[i];
        uint32_t row[33] = {key,      i,      (uint32_t)a[i], s->offset,
                            s->index, s->tag, s->category,    s->shadow_count};
        for (unsigned j = 0; j < 8; ++j)
        {
            row[8 + j] = s->blocks[j];
            row[16 + j] = s->valid[j];
            row[24 + j] = s->shadow[j];
        }
        ach_emit("step", row, 32);
    }
}

static int ordinary(void)
{
    static struct ach_trace r;
    int64_t a[] = {0, 16, 0, 64, 0, 16, 80, 16, 0};
    struct ach_config c = {4, 16, 1, 0};
    CHECK(ach_trace(a, 9, c, &r) == ACH_OK &&
          r.steps[0].category == ACH_COMPULSORY);
    CHECK(r.hits == 3 && r.compulsory == 4 && r.conflict == 2 && !r.capacity);
    uint32_t raw[9];
    for (unsigned i = 0; i < 9; ++i)
    {
        raw[i] = (uint32_t)a[i];
    }
    algo_cache_result old;
    CHECK(algo_cache(raw, 9, &old) == ALGO_OK && old.hits == r.hits &&
          old.compulsory == r.compulsory && old.conflict == r.conflict &&
          old.capacity == r.capacity);
    trace(0, a, 9, c, &r);
    int64_t b[] = {0, 16, 32, 48, 64, 0};
    CHECK(ach_trace(b, 6, c, &r) == ACH_OK &&
          r.steps[5].category == ACH_CAPACITY);
    trace(1, b, 6, c, &r);
    c.ways = 2;
    CHECK(ach_trace(a, 9, c, &r) == ACH_OK && r.hits == 5 && !r.conflict);
    trace(2, a, 9, c, &r);
    int64_t witness[] = {0, 16, 32, 48, 0, 64, 0};
    c.ways = 1;
    CHECK(ach_trace(witness, 7, c, &r) == ACH_OK &&
          r.steps[6].category == ACH_CONFLICT);
    trace(3, witness, 7, c, &r);
    c.shadow_fifo = 1;
    CHECK(ach_trace(witness, 7, c, &r) == ACH_OK &&
          r.steps[6].category == ACH_CAPACITY);
    trace(4, witness, 7, c, &r);
    return 0;
}

static int layouts(void)
{
    static struct ach_trace r;
    int64_t a[64];
    struct ach_config c = {4, 16, 1, 0};
    for (unsigned i = 0; i < 16; ++i)
    {
        a[i] = (int64_t)(i % 8) * 32;
    }
    CHECK(ach_trace(a, 16, c, &r) == ACH_OK && !r.hits);
    trace(10, a, 16, c, &r);
    for (unsigned i = 0; i < 16; ++i)
    {
        a[i] = (int64_t)(i % 8) * 4;
    }
    CHECK(ach_trace(a, 16, c, &r) == ACH_OK && r.hits == 14);
    trace(11, a, 16, c, &r);
    for (unsigned i = 0; i < 64; ++i)
    {
        a[i] = 4 * i;
    }
    CHECK(ach_trace(a, 64, c, &r) == ACH_OK && r.hits == 48);
    trace(12, a, 64, c, &r);
    for (unsigned i = 0; i < 64; ++i)
    {
        a[i] = (i % 8) * 32 + (i / 8) * 4;
    }
    CHECK(ach_trace(a, 64, c, &r) == ACH_OK && !r.hits);
    trace(13, a, 64, c, &r);
    return 0;
}

static int geometry(void)
{
    static struct ach_trace r, old;
    int64_t a[] = {0, 1, 15, 16, 31, 32, 63, 64, 255, 256, 0, UINT32_MAX};
    unsigned key = 100;
    for (unsigned lines = 1; lines <= 8; lines *= 2)
    {
        for (unsigned ways = 1; ways <= lines; ways *= 2)
        {
            for (unsigned block = 1; block <= 256; block *= 16)
            {
                for (unsigned fifo = 0; fifo < 2; ++fifo)
                {
                    struct ach_config c = {lines, block, ways, fifo};
                    CHECK(ach_trace(a, 12, c, &r) == ACH_OK);
                    trace(key++, a, 12, c, &r);
                }
            }
        }
    }
    struct ach_config c = {4, 16, 1, 0};
    CHECK(ach_trace(0, 0, c, &r) == ACH_OK && !r.count);
    r.count = 99;
    old = r;
    int64_t bad[] = {0, -1};
    CHECK(ach_trace(bad, 2, c, &r) == ACH_INVALID &&
          equal(&r, &old, sizeof(r)));
    bad[1] = (int64_t)UINT32_MAX + 1;
    CHECK(ach_trace(bad, 2, c, &r) == ACH_INVALID &&
          equal(&r, &old, sizeof(r)));
    c.ways = 3;
    CHECK(ach_trace(a, 12, c, &r) == ACH_INVALID && equal(&r, &old, sizeof(r)));
    c = (struct ach_config){0, 16, 1, 0};
    CHECK(ach_trace(a, 12, c, &r) == ACH_INVALID && equal(&r, &old, sizeof(r)));
    return 0;
}

static void emit_cost(unsigned key, const struct ach_cost *r)
{
    const struct ach_fraction a[] = {r->amat_ns, r->base_cycles,
                                     r->stall_cycles, r->total_cycles,
                                     r->time_ns};
    uint32_t row[11] = {key};
    for (unsigned i = 0; i < 5; ++i)
    {
        row[1 + 2 * i] = a[i].numerator;
        row[2 + 2 * i] = a[i].denominator;
    }
    ach_emit("cost", row, 11);
}

static int costs(void)
{
    struct ach_cost_input x = {{1, 1}, {30, 1}, {6, 5}, {1, 4},     6, 9,
                               100,    24,      20,     2000000000, 0};
    struct ach_cost r, old;
    CHECK(ach_cost(x, &r) == ACH_OK && r.amat_ns.numerator == 21 &&
          r.amat_ns.denominator == 1 && r.total_cycles.numerator == 240 &&
          r.time_ns.numerator == 120 && r.time_ns.denominator == 1);
    emit_cost(0, &r);
    x.miss_ns.numerator = 31;
    x.total_miss_latency = 1;
    CHECK(ach_cost(x, &r) == ACH_OK && r.amat_ns.numerator == 21 &&
          r.amat_ns.denominator == 1);
    emit_cost(1, &r);
    x = (struct ach_cost_input){{3, 2}, {13, 2}, {7, 3}, {2, 7},     2, 7,
                                9,      7,       5,      1000000000, 0};
    CHECK(ach_cost(x, &r) == ACH_OK && r.amat_ns.numerator == 47 &&
          r.amat_ns.denominator == 14 && r.total_cycles.numerator == 31);
    emit_cost(2, &r);
    old = r;
    x.miss_rate.numerator = 8;
    CHECK(ach_cost(x, &r) == ACH_INVALID && equal(&r, &old, sizeof(r)));
    x.miss_rate.numerator = 2;
    x.hit_ns.denominator = 0;
    CHECK(ach_cost(x, &r) == ACH_INVALID && equal(&r, &old, sizeof(r)));
    x.hit_ns = (struct ach_fraction){UINT32_MAX, 1};
    CHECK(ach_cost(x, &r) == ACH_RANGE && equal(&r, &old, sizeof(r)));
    x.hit_ns = (struct ach_fraction){3, 2};
    x.miss_ns = (struct ach_fraction){1, 1};
    x.total_miss_latency = 1;
    CHECK(ach_cost(x, &r) == ACH_RANGE && equal(&r, &old, sizeof(r)));
    x.total_miss_latency = 0;
    x.references = 0;
    CHECK(ach_cost(x, &r) == ACH_INVALID && equal(&r, &old, sizeof(r)));
    x.references = 7;
    x.frequency_hz = 0;
    CHECK(ach_cost(x, &r) == ACH_INVALID && equal(&r, &old, sizeof(r)));
    return 0;
}

static int overlap(void)
{
    struct ach_miss a[4] = {{0, 20, -1}, {0, 20, -1}, {0, 20, -1}, {0, 20, -1}};
    struct ach_overlap r, old;
    for (unsigned mode = 0; mode < 3; ++mode)
    {
        for (unsigned slots = 1; slots <= 4; ++slots)
        {
            for (unsigned i = 0; i < 4; ++i)
            {
                a[i].predecessor = mode == 1 ? (int)i - 1 : -1;
                a[i].release = mode == 2 ? i * 7 : 0;
                a[i].latency = mode == 2 ? 5 + i * 3 : 20;
            }
            CHECK(ach_overlap(a, 4, slots, &r) == ACH_OK);
            if (mode == 0 && slots == 2)
            {
                CHECK(r.makespan == 41);
            }
            if (mode == 0 && slots == 4)
            {
                CHECK(r.makespan == 23);
            }
            if (mode == 1)
            {
                CHECK(r.makespan == 80);
            }
            for (unsigned i = 0; i < 4; ++i)
            {
                uint32_t row[] = {mode,
                                  slots,
                                  i,
                                  a[i].release,
                                  a[i].latency,
                                  (uint32_t)(a[i].predecessor + 1),
                                  r.start[i],
                                  r.finish[i],
                                  r.makespan};
                ach_emit("overlap", row, 9);
            }
        }
    }
    old = r;
    a[3].predecessor = 3;
    CHECK(ach_overlap(a, 4, 2, &r) == ACH_INVALID &&
          equal(&r, &old, sizeof(r)));
    CHECK(ach_overlap(0, 0, 2, &r) == ACH_OK && !r.count && !r.makespan);
    struct ach_miss maximum[16];
    for (unsigned i = 0; i < 16; ++i)
    {
        maximum[i] = (struct ach_miss){256, 256, (int)i - 1};
    }
    CHECK(ach_overlap(maximum, 16, 4, &r) == ACH_OK && r.makespan == 4352);
    return 0;
}

int ach_test(const char *s)
{
    if (name(s, "ordinary"))
    {
        return ordinary();
    }
    if (name(s, "layouts"))
    {
        return layouts();
    }
    if (name(s, "geometry"))
    {
        return geometry();
    }
    if (name(s, "costs"))
    {
        return costs();
    }
    if (name(s, "overlap"))
    {
        return overlap();
    }
    return 42;
}

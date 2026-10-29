#include "sort.h"
#define CHECK(x)                             \
    do                                       \
    {                                        \
        if (!(x))                            \
            return as_failure(#x, __LINE__); \
    } while (0)
static const algo_pair fixture[5] = {{3, 0}, {1, 1}, {3, 2}, {2, 3}, {1, 4}};

static void copy(algo_pair *out, const algo_pair *in, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        out[i] = in[i];
    }
}

static int same(const algo_pair *a, const algo_pair *b, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        if (a[i].key != b[i].key || a[i].tag != b[i].tag)
        {
            return 0;
        }
    }
    return 1;
}

static int sorted(const algo_pair *a, unsigned n, unsigned descending)
{
    for (unsigned i = 1; i < n; ++i)
    {
        if (descending ? a[i - 1].key < a[i].key : a[i - 1].key > a[i].key)
        {
            return 0;
        }
    }
    return 1;
}

static int permutation(const algo_pair *a, const algo_pair *b, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned left = 0, right = 0;
        for (unsigned j = 0; j < n; ++j)
        {
            left += a[j].key == a[i].key && a[j].tag == a[i].tag;
            right += b[j].key == a[i].key && b[j].tag == a[i].tag;
        }
        if (left != right)
        {
            return 0;
        }
    }
    return 1;
}

static int stable(const algo_pair *original, const algo_pair *out, unsigned n,
                  unsigned descending)
{
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned rank = 0;
        for (unsigned j = 0; j < n; ++j)
        {
            rank += (descending ? original[j].key > original[i].key
                                : original[j].key < original[i].key) ||
                    (original[j].key == original[i].key && j < i);
        }
        if (out[rank].key != original[i].key ||
            out[rank].tag != original[i].tag)
        {
            return 0;
        }
    }
    return 1;
}

static void input(algo_pair *a, unsigned n, unsigned distribution)
{
    for (unsigned i = 0; i < n; ++i)
    {
        uint32_t key = distribution == 0   ? 7
                       : distribution == 1 ? i
                       : distribution == 2 ? n - i
                                           : (i % 5 == 0   ? UINT32_MAX
                                              : i % 5 == 1 ? 0x80000000u
                                                           : i % 3);
        a[i] = (algo_pair){key, i};
    }
}

static int insertion(void)
{
    algo_pair a[32], before[32];
    uint32_t ignored = 0;
    for (unsigned n = 0; n <= 32; ++n)
    {
        for (unsigned distribution = 0; distribution < 4; ++distribution)
        {
            input(a, n, distribution);
            copy(before, a, n);
            CHECK(algo_checked_insert(a, n, n) == ALGO_OK &&
                  stable(before, a, n, 0));
            copy(a, before, n);
            CHECK(algo_abi_insert_probe(a, n, 0, &ignored) == 1 &&
                  stable(before, a, n, 0));
        }
    }
    copy(a, fixture, 5);
    as_stats = (struct as_stats){0};
    as_prefixes = (struct as_prefixes){0};
    as_instrumented_insert(a, 5);
    CHECK(as_stats.comparisons == 9 && as_stats.shifts == 6 &&
          as_stats.placements == 4 && as_stats.prefixes == 4);
    CHECK(stable(fixture, a, 5, 0));
    for (unsigned i = 0; i < 4; ++i)
    {
        unsigned prefix = i + 2;
        CHECK(as_prefixes.length[i] == prefix &&
              stable(fixture, as_prefixes.state[i], prefix, 0) &&
              permutation(fixture, as_prefixes.state[i], 5));
        CHECK(
            same(fixture + prefix, as_prefixes.state[i] + prefix, 5 - prefix));
        as_observe(prefix, as_prefixes.state[i], 5);
    }
    copy(a, fixture, 5);
    CHECK(algo_checked_insert(a, 5, 4) == ALGO_BAD && same(a, fixture, 5));
    CHECK(algo_checked_insert(NULL, 0, 0) == ALGO_OK);
    CHECK(algo_checked_insert(a, 33, 33) == ALGO_BAD && same(a, fixture, 5));
    return 0;
}

static int counts(void)
{
    algo_pair a[32], before[32];
    for (unsigned n = 0; n <= 32; ++n)
    {
        for (unsigned distribution = 0; distribution < 4; ++distribution)
        {
            input(a, n, distribution);
            copy(before, a, n);
            unsigned shifts = 0, comparisons = 0;
            for (unsigned i = 1; i < n; ++i)
            {
                unsigned greater = 0;
                for (unsigned j = 0; j < i; ++j)
                {
                    greater += a[j].key > a[i].key;
                }
                shifts += greater;
                comparisons += greater + (greater < i);
            }
            as_stats = (struct as_stats){0};
            as_prefixes = (struct as_prefixes){0};
            as_instrumented_insert(a, n);
            CHECK(as_stats.comparisons == comparisons &&
                  as_stats.shifts == shifts &&
                  as_stats.placements == (n ? n - 1 : 0) &&
                  as_stats.prefixes == (n ? n - 1 : 0));
            CHECK(stable(before, a, n, 0));
            as_metric(n, distribution, &as_stats);
        }
    }
    return 0;
}

static int descending(void)
{
    algo_pair a[32], before[32];
    for (unsigned n = 0; n <= 32; ++n)
    {
        for (unsigned distribution = 0; distribution < 4; ++distribution)
        {
            input(a, n, distribution);
            copy(before, a, n);
            as_descending_insert(a, n);
            CHECK(sorted(a, n, 1) && stable(before, a, n, 1));
        }
    }
    return 0;
}

static int wide(void)
{
    struct as_wide a[32], before[32];
    uint32_t ignored = 0;
    for (unsigned n = 0; n <= 32; ++n)
    {
        for (unsigned i = 0; i < n; ++i)
        {
            a[i] = (struct as_wide){i % 5 == 0 ? UINT32_MAX : i % 3,
                                    UINT64_C(0x8123456700000000) + i};
        }
        for (unsigned i = 0; i < n; ++i)
        {
            before[i] = a[i];
        }
        CHECK(as_wide_abi_probe(a, n, 0, &ignored) == 1);
        for (unsigned i = 0; i < n; ++i)
        {
            unsigned rank = 0;
            for (unsigned j = 0; j < n; ++j)
            {
                rank += before[j].key < before[i].key ||
                        (before[j].key == before[i].key && j < i);
            }
            CHECK(a[rank].key == before[i].key && a[rank].tag == before[i].tag);
        }
    }
    return 0;
}

struct as_compare
{
    unsigned calls, fail, malformed;
};

static int compare(void *v, uint32_t a, uint32_t b, int *out)
{
    struct as_compare *c = v;
    ++c->calls;
    if (c->calls == c->fail)
    {
        if (c->malformed)
        {
            *out = 2;
            return 0;
        }
        return 1;
    }
    *out = a < b ? -1 : a > b;
    return 0;
}

static int failure(void)
{
    algo_pair a[5];
    for (unsigned malformed = 0; malformed < 2; ++malformed)
    {
        for (unsigned fail = 1; fail <= 9; ++fail)
        {
            copy(a, fixture, 5);
            struct as_compare c = {0, fail, malformed};
            struct as_stats work = {0};
            CHECK(as_fallible(a, 5, 5, compare, &c, &work) == 2 &&
                  same(a, fixture, 5) && c.calls == fail &&
                  work.comparisons == fail);
        }
    }
    copy(a, fixture, 5);
    struct as_compare c = {0, 0, 0};
    struct as_stats work = {0};
    CHECK(as_fallible(a, 5, 5, compare, &c, &work) == 0 &&
          stable(fixture, a, 5, 0) && work.comparisons == 9 &&
          work.shifts == 6 && work.placements == 4 && c.calls == 9);
    c = (struct as_compare){0};
    work = (struct as_stats){99, 99, 99, 99};
    copy(a, fixture, 5);
    CHECK(as_fallible(a, 5, 4, compare, &c, &work) == 1 &&
          same(a, fixture, 5) && c.calls == 0 && work.comparisons == 99);
    CHECK(as_fallible(a, 33, 33, compare, &c, &work) == 1 &&
          same(a, fixture, 5));
    CHECK(as_fallible(NULL, 0, 0, compare, &c, &work) == 0 && !c.calls &&
          !work.comparisons);
    return 0;
}
#ifndef AS_NO_PRIOR
static int prior(void)
{
    algo_pair original[16], out[16], assembly[16];
    unsigned unstable = 0;
    for (unsigned n = 0; n <= 16; ++n)
    {
        for (unsigned d = 0; d < 4; ++d)
        {
            input(original, n, d);
            copy(assembly, original, n);
            algo_asm_insert(assembly, n);
            for (unsigned method = 0; method < 3; ++method)
            {
                struct as_prior_cost cost;
                CHECK(as_prior(original, n, method, out, &cost) == 0 &&
                      sorted(out, n, 0) && permutation(original, out, n));
                if (method < 2)
                {
                    CHECK(same(out, assembly, n));
                }
                else if (!same(out, assembly, n))
                {
                    ++unstable;
                }
            }
        }
    }
    CHECK(unstable > 0);
    const algo_pair runs[5] = {{1, 1}, {3, 0}, {1, 4}, {2, 3}, {3, 2}},
                    expected[5] = {{1, 1}, {1, 4}, {2, 3}, {3, 0}, {3, 2}};
    struct as_prior_cost cost;
    CHECK(as_prior_merge(runs, 5, 2, out, &cost) == 0 &&
          same(out, expected, 5));
    return 0;
}
#endif
int as_test(unsigned which)
{
    switch (which)
    {
        case 0:
            return insertion();
        case 1:
            return counts();
        case 2:
            return descending();
        case 3:
            return wide();
        case 4:
            return failure();
#ifndef AS_NO_PRIOR
        case 5:
            return prior();
#endif
        default:
            return 2;
    }
}

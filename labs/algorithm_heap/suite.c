#include "heap.h"
#include <limits.h>
#define CHECK(x)                             \
    do                                       \
    {                                        \
        if (!(x))                            \
            return ah_failure(#x, __LINE__); \
    } while (0)
static struct ah_trace trace;
static uint32_t a[1024], before[1024];

static int same(const uint32_t *x, const uint32_t *y, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
        if (x[i] != y[i])
            return 0;
    return 1;
}

static void copy(uint32_t *x, const uint32_t *y, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
        x[i] = y[i];
}

static int better(uint32_t x, uint32_t y, unsigned minimum)
{
    return minimum ? x < y : x > y;
}

static int edges(const uint32_t *x, unsigned n, unsigned p, unsigned arity,
                 unsigned minimum)
{
    for (unsigned i = 1; i < n; ++i)
        if ((i - 1) / arity != p && better(x[i], x[(i - 1) / arity], minimum))
            return 0;
    return 1;
}

static int multiset(const uint32_t *x, const uint32_t *y, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned nx = 0, ny = 0;
        for (unsigned j = 0; j < n; ++j)
        {
            nx += x[i] == x[j];
            ny += x[i] == y[j];
        }
        if (nx != ny)
            return 0;
    }
    return 1;
}

static unsigned height(unsigned n, unsigned p, unsigned arity)
{
    unsigned h = 0;
    while (n >= 2 && p <= (n - 2) / arity)
    {
        p = arity * p + 1;
        ++h;
    }
    return h;
}

static int certificate(const uint32_t *original, const uint32_t *result,
                       unsigned n, unsigned p, unsigned arity, unsigned minimum,
                       const struct ah_stats *s, const struct ah_trace *t)
{
    if (t->used != s->swaps || t->used > height(n, p, arity))
        return 0;
    const uint32_t *last = original;
    unsigned comparisons = 0;
    for (unsigned k = 0; k < t->used; ++k)
    {
        unsigned next = t->position[k];
        if (!n || next >= n || !next || (next - 1) / arity != p ||
            height(n, next, arity) >= height(n, p, arity))
            return 0;
        unsigned first = arity * p + 1,
                 children = n - first < arity ? n - first : arity;
        comparisons += children;
        if (!better(last[next], last[p], minimum))
            return 0;
        for (unsigned j = first; j < first + children; ++j)
            if (better(last[j], last[next], minimum) ||
                (last[j] == last[next] && j < next))
                return 0;
        for (unsigned j = 0; j < n; ++j)
        {
            uint32_t expected = j == p      ? last[next]
                                : j == next ? last[p]
                                            : last[j];
            if (t->state[k][j] != expected)
                return 0;
        }
        if (!multiset(original, t->state[k], n) ||
            !edges(t->state[k], n, next, arity, minimum))
            return 0;
        last = t->state[k];
        p = next;
    }
    if (n >= 2 && p <= (n - 2) / arity)
    {
        unsigned first = arity * p + 1;
        comparisons += n - first < arity ? n - first : arity;
    }
    return comparisons == s->comparisons && same(last, result, n) &&
           multiset(original, result, n) &&
           edges(result, n, UINT32_MAX, arity, minimum);
}

static int premise(void)
{
    const uint32_t bad[5] = {10, 1, 9, 20, 0};
    copy(a, bad, 5);
    struct ah_stats s = {77, 88};
    trace.used = 99;
    CHECK(algo_checked_sift(a, 5, 5, 1) == ALGO_BAD && same(a, bad, 5));
    CHECK(ah_trace(a, 5, 5, 1, 2, 0, &s, &trace) == 1 && same(a, bad, 5) &&
          s.comparisons == 77 && trace.used == 99);
    algo_asm_sift(a, 5, 1);
    CHECK(
        a[1] == 20 && a[0] == 10 &&
        !edges(
            a, 5, UINT32_MAX, 2,
            0)); /* Memory-safe input; deliberately excluded ordering premise. */
    copy(a, bad, 5);
    CHECK(ah_trace(a, 5, 4, 0, 2, 0, &s, &trace) == 1 && same(a, bad, 5));
    CHECK(ah_trace(a, 5, 5, UINT32_MAX, 2, 0, &s, &trace) == 1);
    CHECK(ah_trace(a, 1025, 1025, 0, 2, 0, &s, &trace) == 1);
    CHECK(ah_trace(NULL, 0, 0, UINT32_MAX, 3, 1, &s, &trace) == 0 &&
          s.comparisons == 0 && trace.used == 0);
    return 0;
}

static const uint32_t fixtures[3][7] = {{5, 7, 8, 3, 6, 2},
                                        {1, 8, 7, 6, 5, 4, 3},
                                        {5, 5, 4}};
static const unsigned lengths[3] = {6, 7, 3}, counts[3] = {3, 4, 2},
                      swaps[3] = {1, 2, 0};

static int scalar(void)
{
    uint32_t ignored = 0;
    for (unsigned f = 0; f < 3; ++f)
    {
        unsigned n = lengths[f];
        copy(a, fixtures[f], n);
        CHECK(algo_abi_sift_probe(a, n, 0, &ignored) == 1);
        CHECK(edges(a, n, UINT32_MAX, 2, 0) && multiset(a, fixtures[f], n));
    }
    const uint32_t one[2] = {0, UINT32_MAX};
    copy(a, one, 2);
    CHECK(algo_checked_sift(a, 2, 2, 0) == ALGO_OK && a[0] == UINT32_MAX &&
          a[1] == 0);
    const uint32_t equal[3] = {0, 9, 9};
    copy(a, equal, 3);
    CHECK(algo_checked_sift(a, 3, 3, 0) == ALGO_OK && a[1] == 0 && a[2] == 9);
    for (unsigned i = 0; i < 1024; ++i)
    {
        a[i] = 2048 - i;
    }
    a[0] = 0;
    CHECK(algo_checked_sift(a, 1024, 1024, 0) == ALGO_OK &&
          edges(a, 1024, UINT32_MAX, 2, 0));
    CHECK(algo_checked_sift(a, 1024, 1024, 1023) == ALGO_OK);
    return 0;
}

static int observed(void)
{
    for (unsigned f = 0; f < 3; ++f)
    {
        unsigned n = lengths[f];
        copy(a, fixtures[f], n);
        ah_stats = (struct ah_stats){0};
        ah_observed.used = 0;
        ah_instrumented_sift(a, n, 0);
        CHECK(ah_stats.comparisons == counts[f] && ah_stats.swaps == swaps[f]);
        CHECK(certificate(fixtures[f], a, n, 0, 2, 0, &ah_stats, &ah_observed));
        ah_emit(f, a, n, &ah_stats, &ah_observed);
    }
    copy(a, fixtures[0], 6);
    ah_stats = (struct ah_stats){0};
    ah_observed.used = 0;
    ah_instrumented_sift(a, 6, 5);
    CHECK(ah_stats.comparisons == 0 && ah_stats.swaps == 0 &&
          same(a, fixtures[0], 6));
    return 0;
}

static int exhaustive(unsigned arity, unsigned minimum)
{
    unsigned cells = 0;
    for (unsigned n = 0; n <= 6; ++n)
    {
        unsigned limit = 1;
        for (unsigned i = 0; i < n; ++i)
            limit *= 3;
        for (unsigned bits = 0; bits < limit; ++bits)
        {
            unsigned x = bits;
            for (unsigned i = 0; i < n; ++i)
            {
                before[i] = x % 3;
                x /= 3;
            }
            for (unsigned p = 0; p < (n ? n : 1); ++p)
            {
                copy(a, before, n);
                struct ah_stats s = {0};
                int status = ah_trace(a, n, n, p, arity, minimum, &s, &trace);
                if (status)
                {
                    CHECK(same(a, before, n));
                    continue;
                }
                CHECK(certificate(before, a, n, p, arity, minimum, &s, &trace));
                ++cells;
            }
        }
    }
    CHECK(cells > 100);
    return 0;
}

static int equivalent(const struct cd_heap *x, const struct cd_heap *y)
{
    if (x->count != y->count || x->max_heap != y->max_heap ||
        x->sequence != y->sequence || x->comparisons != y->comparisons)
        return 0;
    for (unsigned i = 0; i < 16; ++i)
        if (x->index[i] != y->index[i] ||
            x->generation[i] != y->generation[i] || x->state[i] != y->state[i])
            return 0;
    for (unsigned i = 0; i < x->count; ++i)
    {
        struct cd_heap_item a = x->data[i], b = y->data[i];
        if (a.priority != b.priority || a.identity != b.identity ||
            a.slot != b.slot || a.sequence != b.sequence)
            return 0;
    }
    return 1;
}

static int valid(const struct cd_heap *h)
{
    for (unsigned i = 0; i < h->count; ++i)
    {
        struct cd_heap_item a = h->data[i];
        if (a.slot >= 16 || h->index[a.slot] != i || h->state[a.slot] != 1)
            return 0;
        if (i)
        {
            struct cd_heap_item b = h->data[(i - 1) / 2];
            if (a.priority == b.priority ? a.sequence < b.sequence
                : h->max_heap            ? a.priority > b.priority
                                         : a.priority < b.priority)
                return 0;
        }
    }
    return 1;
}

static int prior(unsigned maximum)
{
    struct cd_heap x, y;
    int input[16];
    uint32_t ignored = 0;
    for (unsigned n = 0; n <= 16; ++n)
        for (unsigned distribution = 0; distribution < 4; ++distribution)
        {
            for (unsigned i = 0; i < n; ++i)
                input[i] = distribution == 0   ? 7
                           : distribution == 1 ? (int)i - 8
                           : distribution == 2 ? 8 - (int)i
                                               : (i % 3 == 0   ? INT_MIN
                                                  : i % 3 == 1 ? INT_MAX
                                                               : 0);
            CHECK(cd_heap_build(&x, input, n, maximum) == CD_OK &&
                  ah_heap_build(&y, input, n, maximum) == CD_OK);
            CHECK(equivalent(&x, &y) && valid(&y));
            if (n)
            {
                struct cd_heap_handle hx = {&x, 0, 1}, hy = {&y, 0, 1};
                for (unsigned j = 0; j < 3; ++j)
                {
                    int key = j == 0 ? INT_MIN : j == 1 ? INT_MAX : 0;
                    CHECK(cd_heap_change(&x, hx, key) == CD_OK &&
                          ah_heap_change(&y, hy, key) == CD_OK);
                    CHECK(equivalent(&x, &y) && valid(&y));
                }
                CHECK(ah_cd_probe(&y, n - 1, 0, &ignored) == 1 &&
                      equivalent(&x, &y));
            }
            while (x.count)
            {
                int p = 0, q = 0;
                unsigned i = 0, j = 0;
                CHECK(cd_heap_take(&x, &p, &i) == CD_OK &&
                      ah_heap_take(&y, &q, &j) == CD_OK && p == q && i == j);
                CHECK(equivalent(&x, &y) && valid(&y));
            }
        }
    cd_heap_init(&x, maximum);
    ah_heap_init(&y, maximum);
    for (unsigned round = 0; round < 3; ++round)
    {
        struct cd_heap_handle hx[16], hy[16];
        for (unsigned i = 0; i < 16; ++i)
        {
            CHECK(cd_heap_insert(&x, (int)(i % 4), i, &hx[i]) == CD_OK &&
                  ah_heap_insert(&y, (int)(i % 4), i, &hy[i]) == CD_OK);
            CHECK(equivalent(&x, &y) && valid(&y));
        }
        for (unsigned i = 0; i < 16; ++i)
        {
            int p, q;
            unsigned a_id, b_id;
            CHECK(cd_heap_take(&x, &p, &a_id) == CD_OK &&
                  ah_heap_take(&y, &q, &b_id) == CD_OK && p == q &&
                  a_id == b_id);
            CHECK(ah_heap_get(&y, hy[b_id], &q, &b_id) == CD_INVALID);
            CHECK(equivalent(&x, &y) && valid(&y));
        }
    }
    struct cd_heap_handle h;
    CHECK(ah_heap_insert(&y, 1, 1, &h) == CD_FULL &&
          cd_heap_insert(&x, 1, 1, &h) == CD_FULL);
    int p;
    unsigned id;
    CHECK(ah_heap_take(&y, &p, &id) == CD_EMPTY);
    int init[3] = {3, 2, 1};
    CHECK(cd_heap_build(&x, init, 3, maximum) == CD_OK &&
          ah_heap_build(&y, init, 3, maximum) == CD_OK);
    x.comparisons = y.comparisons = UINT32_MAX;
    CHECK(cd_heap_take(&x, &p, &id) == CD_OK &&
          ah_heap_take(&y, &p, &id) == CD_OK && equivalent(&x, &y));
    CHECK(cd_heap_build(&x, init, 3, maximum) == CD_OK &&
          ah_heap_build(&y, init, 3, maximum) == CD_OK);
    struct cd_heap_handle root = {&x, x.data[0].slot, 1};
    int key = maximum ? INT_MIN : INT_MAX;
    y.data[0].priority = key;
    CHECK(cd_heap_change(&x, root, key) == CD_OK &&
          ah_cd_probe(&y, 0, 0, &ignored) == 1 && equivalent(&x, &y) &&
          valid(&y));
    return 0;
}

int ah_test(unsigned which)
{
    switch (which)
    {
        case 0:
            return premise();
        case 1:
            return scalar();
        case 2:
            return observed();
        case 3:
            return prior(1);
        case 4:
        {
            int r = prior(0);
            return r ? r : exhaustive(2, 1);
        }
        case 5:
            return exhaustive(3, 0);
        case 6:
            return exhaustive(2, 0);
        default:
            return 2;
    }
}

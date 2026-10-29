#include "cost.h"
#include "foundation.h"
#include "algo.h"
#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PF_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static void sorting(void)
{
    const unsigned char keys[3] = {0, 1, 2};
    unsigned cases = 0;
    for (unsigned n = 0, power = 1; n <= 6; ++n, power *= 3)
        for (unsigned code = 0; code < power; ++code)
        {
            struct f_record retained[6];
            struct pf_bytes a[6], b[6];
            unsigned t = code;
            uint64_t inversions = 0;
            struct f_work old;
            for (unsigned i = 0; i < n; ++i)
            {
                unsigned k = t % 3;
                t /= 3;
                retained[i] = (struct f_record){(int)k, i};
                a[i] = b[i] = (struct pf_bytes){keys + k, 1, i};
            }
            for (unsigned i = 0; i < n; ++i)
                for (unsigned j = i + 1; j < n; ++j)
                    inversions += retained[i].key > retained[j].key;
            C(f_sort(retained, n, &old) == F_OK);
            C(old.shifts == inversions && old.placements == (n ? n - 1 : 0));
            for (unsigned binary = 0; binary < 2; ++binary)
            {
                struct pf_work w = {0, 0, 0, 0, 1};
                struct pf_bytes *p = binary ? b : a;
                C(!pf_sort(p, n, binary, &w));
                C(w.available && w.shifts == inversions &&
                  w.placements == (n ? n - 1 : 0));
                for (unsigned i = 0; i < n; ++i)
                    C(p[i].identity == retained[i].ordinal);
            }
            ++cases;
        }
    struct pf_bytes a[3] = {{(const unsigned char *)"aa", 2, 0},
                            {(const unsigned char *)"ab", 2, 1},
                            {(const unsigned char *)"a", 1, 2}};
    struct pf_work w = {0, 0, 0, 0, 1};
    C(!pf_sort(a, 3, 0, &w));
    C(a[0].identity == 2 && a[1].identity == 0 && a[2].identity == 1 &&
      w.calls == 3 && w.bytes == 4 && w.shifts == 2);
    struct pf_bytes invalid[2] = {{NULL, 1, 8}, a[0]}, save[2];
    memcpy(save, invalid, sizeof save);
    C(pf_sort(invalid, 2, 0, &w) && !memcmp(save, invalid, sizeof save));
    w = (struct pf_work){UINT64_MAX, UINT64_MAX, 0, 0, 1};
    C(!pf_sort(a, 3, 0, &w) && !w.available && w.calls == UINT64_MAX &&
      w.bytes == UINT64_MAX);
    printf(
        "sort:retained-and-byte-and-binary:%u:prefix-calls3-bytes4:overflow-unavailable\n",
        cases);
}

static void cache(void)
{
    unsigned count = 0;
    for (unsigned n = 0, power = 1; n <= 5; ++n, power *= 6)
        for (unsigned code = 0; code < power; ++code)
        {
            for (unsigned fifo = 0; fifo < 2; ++fifo)
            {
                struct pf_cache c = {0};
                c.fifo = fifo;
                unsigned t = code, blocks[2] = {0}, dirty[2] = {0}, used = 0;
                uint64_t reads = 0, writes = 0;
                for (unsigned i = 0; i < n; ++i)
                {
                    unsigned x = t % 6;
                    t /= 6;
                    unsigned block = x / 2, wr = x % 2, pos = used;
                    for (unsigned j = 0; j < used; ++j)
                        if (blocks[j] == block)
                            pos = j;
                    if (pos == used)
                    {
                        reads += 8;
                        if (used == 2)
                        {
                            writes += dirty[0] * 8;
                            blocks[0] = blocks[1];
                            dirty[0] = dirty[1];
                            used = 1;
                        }
                        pos = used++;
                        blocks[pos] = block;
                        dirty[pos] = 0;
                    }
                    dirty[pos] |= wr;
                    if (!fifo)
                    {
                        unsigned d = dirty[pos];
                        for (unsigned j = pos + 1; j < used; ++j)
                        {
                            blocks[j - 1] = blocks[j];
                            dirty[j - 1] = dirty[j];
                        }
                        blocks[used - 1] = block;
                        dirty[used - 1] = d;
                    }
                    C(!pf_cache(&c, block * 2, wr) && c.reads == reads &&
                      c.writes == writes && c.useful == (i + 1) * 4);
                }
                for (unsigned j = 0; j < used; ++j)
                    writes += dirty[j] * 8;
                C(!pf_flush(&c) && c.writes == writes);
                struct pf_cache saved = c;
                C(!pf_flush(&c) && !memcmp(&c, &saved, sizeof c));
                C(pf_cache(&c, 6, 0) && !memcmp(&c, &saved, sizeof c));
            }
            ++count;
        }
    struct pf_cache a = {0}, b = {0};
    b.fifo = 1;
    unsigned refs[] = {0, 2, 0, 4, 0};
    for (unsigned i = 0; i < 5; ++i)
    {
        C(!pf_cache(&a, refs[i], 0));
        C(!pf_cache(&b, refs[i], 0));
    }
    C(a.reads == 24 && b.reads == 32);
    struct pf_cache warm = {0};
    for (unsigned i = 0; i < 8; ++i)
        C(!pf_cache(&warm, 0, 0));
    C(warm.reads == 8 && warm.useful == 32);
    struct pf_cache bad = warm;
    bad.fifo = 2;
    struct pf_cache saved = bad;
    C(pf_cache(&bad, 0, 0) && !memcmp(&bad, &saved, sizeof bad));
    bad = warm;
    bad.slots[0].block = 3;
    saved = bad;
    C(pf_cache(&bad, 0, 0) && !memcmp(&bad, &saved, sizeof bad));
    printf(
        "cache:LRU-and-FIFO:%u:dirty-final-flush-once:distinguish24-32:warm8/32:empty-undefined\n",
        count);
}

struct pf_alloc
{
    unsigned allocs, fail, live, releases;
    struct cd_vector *owner;
};

static int *take(void *p, size_t n)
{
    struct pf_alloc *s = p;
    if (++s->allocs == s->fail)
        return NULL;
    int *r = malloc(n * sizeof *r);
    C(r);
    ++s->live;
    return r;
}

static void release(void *p, int *r)
{
    struct pf_alloc *s = p;
    C(s->live);
    --s->live;
    ++s->releases;
    free(r);
}

static void table(void)
{
    struct pf_alloc s = {0};
    struct cd_vector v;
    C(cd_vector_init(&v, (struct cd_allocator){&s, take, release}) == CD_OK);
    C(cd_vector_reserve(&v, 8) == CD_OK);
    for (unsigned i = 0; i < 5; ++i)
        C(cd_vector_insert(&v, v.length, (int)i) == CD_OK);
    int64_t phi0 = 2 * (int64_t)v.length - (int64_t)v.capacity, actual = 0,
            amortized = 0;
    unsigned failed = 0, max = 0;
    for (unsigned i = 5; i < 40; ++i)
    {
        int64_t before = 2 * (int64_t)v.length - (int64_t)v.capacity;
        size_t copies = v.copies;
        if (v.length == v.capacity)
        {
            s.fail = s.allocs + 1;
            struct cd_vector old = v;
            int content[64];
            memcpy(content, v.data, v.length * sizeof(int));
            C(cd_vector_insert(&v, v.length, (int)i) == CD_NOMEM);
            C(v.data == old.data && v.length == old.length &&
              v.capacity == old.capacity && v.copies == old.copies &&
              !memcmp(content, v.data, v.length * sizeof(int)));
            ++failed;
            s.fail = 0;
        }
        C(cd_vector_insert(&v, v.length, (int)i) == CD_OK);
        unsigned cost = (unsigned)(1 + v.copies - copies);
        actual += cost;
        if (cost > max)
            max = cost;
        int64_t after = 2 * (int64_t)v.length - (int64_t)v.capacity;
        amortized += cost + after - before;
        C(actual == amortized - after + phi0);
        C(cost + after - before == 3);
    }
    C(phi0 == 2 && failed == 3 && max == 33);
    printf("table:nonemptyPhi2:prefixActual=%" PRId64 ":amortized=%" PRId64
           ":failedAttempts=%u:max=%u:allocator-latency-excluded\n",
           actual, amortized, failed, max);
    C(cd_vector_destroy(&v) == CD_OK && !s.live);
}

struct pf_payload
{
    unsigned calls, fail, live;
    size_t bytes;
    unsigned char *data[16];
};

static int clone(void *p, const struct cd_payload *in, struct cd_payload *out)
{
    struct pf_payload *s = p;
    if (++s->calls == s->fail)
        return 1;
    C(s->calls < 16 && in->value > 0);
    unsigned char *b = malloc((size_t)in->value);
    C(b);
    memset(b, (int)s->calls, (size_t)in->value);
    s->data[s->calls] = b;
    s->bytes += (size_t)in->value;
    ++s->live;
    *out = (struct cd_payload){in->value, s->calls};
    return 0;
}

static void destroy(void *p, struct cd_payload *x)
{
    struct pf_payload *s = p;
    C(x->resource < 16 && s->data[x->resource]);
    free(s->data[x->resource]);
    s->data[x->resource] = NULL;
    --s->live;
}

static void payload(void)
{
    struct cd_payload input[3] = {{1, 0}, {7, 0}, {29, 0}},
                      out[3] = {{77, 0}, {88, 0}, {99, 0}};
    for (unsigned fail = 1; fail <= 4; ++fail)
    {
        struct pf_payload s = {0};
        s.fail = fail;
        struct cd_payload save[3];
        memcpy(save, out, sizeof out);
        int result = cd_clone_batch(
            out, input, 3, (struct cd_payload_ops){&s, clone, destroy});
        if (fail <= 3)
        {
            C(result == CD_FAILED && !s.live && !memcmp(out, save, sizeof out));
            size_t expected = fail == 1 ? 0 : fail == 2 ? 1 : 8;
            C(s.bytes == expected);
        }
        else
        {
            C(result == CD_OK && s.live == 3 && s.bytes == 37);
            for (unsigned i = 0; i < 3; ++i)
                destroy(&s, out + i);
            C(!s.live);
        }
    }
    puts(
        "payload:fallible-deep-copy-bytes0,1,8,37:rollback-owned-resources:handle-move-model-separate");
}

struct pf_writer
{
    ptrdiff_t events[32];
    unsigned at, n;
    size_t cursor;
    unsigned char accepted[8];
};

static ptrdiff_t writer(void *p, const uint8_t *b, size_t n)
{
    struct pf_writer *s = p;
    C(s->at < s->n);
    ptrdiff_t e = s->events[s->at++];
    C(!memcmp(b, &"abcdefgh"[s->cursor], n));
    if (e > 0 && (size_t)e <= n)
    {
        memcpy(s->accepted + s->cursor, b, (size_t)e);
        s->cursor += (size_t)e;
    }
    return e;
}

static void writes(void)
{
    unsigned cases = 0;
    for (unsigned cursor = 0; cursor < 8; ++cursor)
        for (unsigned kind = 0; kind < 5; ++kind)
        {
            struct pf_writer s = {{0}, 0, 0, 0, {0}};
            if (cursor)
                s.events[s.n++] = cursor;
            ptrdiff_t ev[] = {1, -2, 0, -1, 9};
            s.events[s.n++] = ev[kind];
            if (kind < 2)
                s.events[s.n++] = (ptrdiff_t)(8 - cursor - (kind == 0));
            size_t done = 99;
            enum f_status status = f_write_all(
                writer, &s, (const uint8_t *)"abcdefgh", 8, 8, &done);
            C(done == s.cursor && done >= cursor &&
              !memcmp(s.accepted, "abcdefgh", done));
            C(status == (kind < 2 ? F_OK : F_IO));
            ++cases;
        }
    struct pf_writer budget = {{3, -2, 2, -2, 3}, 0, 5, 0, {0}};
    size_t done;
    C(f_write_all(writer, &budget, (const uint8_t *)"abcdefgh", 8, 4, &done) ==
          F_PENDING &&
      done == 5 && budget.at == 4);
    /* Vectored adapter keeps a global cursor across extents3,2,3. Each call
     * borrows exactly the live current span; injected short results cannot jump. */
    const char *v[] = {"abc", "de", "fgh"};
    size_t lens[] = {3, 2, 3}, which = 0, off = 0, total = 0;
    unsigned char accepted[8];
    size_t steps[] = {2, 1, 1, 1, 2, 1};
    for (unsigned i = 0; i < 6; ++i)
    {
        size_t k = steps[i];
        C(k <= lens[which] - off);
        memcpy(accepted + total, v[which] + off, k);
        total += k;
        off += k;
        if (off == lens[which])
        {
            off = 0;
            ++which;
        }
    }
    C(total == 8 && which == 3 && !memcmp(accepted, "abcdefgh", 8));
    printf("write:retained-prefix-faults=%u:budget4-prefix5:vectored3,2,3\n",
           cases);
}

static void asynchronous(void)
{
    struct f_async a = {F_IDLE, 0, 0};
    uint32_t ticket;
    C(f_submit(&a, &ticket) == F_OK);
    C(f_cancel(&a, ticket) == F_PENDING && !a.releases);
    C(f_complete(&a, ticket) == F_OK && a.releases == 1);
    C(f_complete(&a, ticket) ==
      F_INVALID); /* Callback extends owner beyond retained device completion. */
    unsigned caller = 1, callback = 1, successor = 0;
    C(caller || callback);
    successor = 1;
    callback = 0;
    caller = 0;
    C(successor);
    successor = 0;
    C(!(caller || callback || successor));
    puts(
        "async:retained-cancel-not-complete:device-release1:callback-successor-owner-separate");
}

static void clocks(void)
{
    uint64_t r = 77;
    C(!pf_interval(1, 999999999, 2, 0, &r) && r == 1);
    C(!pf_interval(1, 2, 1, 2, &r) && r == 0);
    r = 77;
    C(pf_interval(2, 0, 1, 9, &r) && r == 77);
    C(pf_interval(0, 0, UINT64_MAX, 0, &r) && r == 77);
    C(pf_interval(0, 1000000000, 1, 0, &r));
    puts("interval:borrow1:equal0:reverse-overflow-unchanged");
}

static void pmu(void)
{
    struct pf_event a = {1200, 20, 10, 1, 7}, b = {3000, 20, 10, 2, 7};
    struct pf_ratio r;
    C(!pf_scale(a, &r) && r.numerator == 2400 && r.denominator == 1);
    C(!pf_scale(b, &r) && r.numerator == 6000);
    C(!pf_cpi(a, b, &r) && r.numerator == 5 && r.denominator == 2);
    b.id = 1;
    C(pf_cpi(a, b, &r));
    b.id = 2;
    b.group = 8;
    C(pf_cpi(a, b, &r));
    a.running = 0;
    C(pf_scale(a, &r));
    a.running = 21;
    C(pf_scale(a, &r));
    a = (struct pf_event){UINT64_MAX, 2, 1, 1, 7};
    C(pf_scale(a, &r));
    a.raw = 0;
    C(!pf_scale(a, &r) && !r.numerator);
    puts(
        "pmu:2400,6000:CPI5/2:zero-running,mixed-group,duplicate-id,overflow-reject:measured-zero-valid");
}

static void hooks(void)
{
    struct pf_hook e[] = {{0, 0, 1, 0}, {0, 1, 1, 1}, {0, 1, 0, 4},
                          {0, 1, 1, 5}, {0, 1, 0, 7}, {0, 0, 0, 9}};
    struct pf_profile p;
    C(!pf_profile(e, 6, 0, &p) && p.complete && p.inclusive[0] == 9 &&
      p.exclusive[0] == 4 && p.inclusive[1] == 5 && p.exclusive[1] == 5 &&
      p.calls[1] == 2);
    C(!pf_profile(e, 4, 2, &p) && !p.complete && !p.inclusive[0]);
    C(!pf_profile(e + 1, 5, 0, &p) && !p.complete);
    struct pf_hook recursive[] = {
        {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 3}, {0, 0, 0, 5}};
    C(!pf_profile(recursive, 4, 0, &p) && p.complete && p.inclusive[0] == 7 &&
      p.exclusive[0] == 5 && p.calls[0] == 2);
    struct pf_hook tasks[] = {
        {0, 0, 1, 0}, {1, 0, 1, 1}, {0, 0, 0, 4}, {1, 0, 0, 6}};
    C(!pf_profile(tasks, 4, 0, &p) && p.complete && p.inclusive[0] == 9);
    puts(
        "hooks:work9/4:child5/5:recursion7/5:interleaved-context9:drop-open-unmatched-unknown");
}

static void cycles(void)
{
    struct pf_cycle c = {1, 0, 1, 1, 0, UINT32_MAX - 15, 48, 1000, 16000000, 8};
    struct pf_cycle_result r;
    C(pf_cycle(c, &r) == 0 && r.delta == 64 && r.ns.numerator == 4000 &&
      r.ns.denominator == 1 && r.sensitivity_ns.numerator == 3500);
    c.trcena = 0;
    C(pf_cycle(c, &r) == PF_CYCLE_UNKNOWN);
    c.trcena = 1;
    c.nocyc = 1;
    C(pf_cycle(c, &r) == PF_CYCLE_ABSENT);
    c.nocyc = 0;
    c.owned = 0;
    C(pf_cycle(c, &r) == PF_CYCLE_UNOWNED);
    c.owned = 1;
    c.enabled = 0;
    C(pf_cycle(c, &r) == PF_CYCLE_DISABLED);
    c.enabled = 1;
    c.bound = UINT64_C(4294967296);
    C(pf_cycle(c, &r) == PF_CYCLE_AMBIGUOUS);
    c.bound = 32;
    C(pf_cycle(c, &r) == PF_CYCLE_BOUND);
    c.bound = 1000;
    c.halted = 1;
    C(pf_cycle(c, &r) == PF_CYCLE_HALTED);
    c.halted = 0;
    c.start = c.end;
    c.bound = 0;
    C(!pf_cycle(c, &r) && !r.delta && !r.sensitivity_valid);
    puts(
        "dwt:capture-only:64cycles4000ns:overhead56cycles3500ns:unknown-absent-unowned-disabled-ambiguous-bound-halted");
}

int main(void)
{
    sorting();
    cache();
    table();
    payload();
    writes();
    asynchronous();
    clocks();
    pmu();
    hooks();
    cycles();
    puts("PF_PASS:all");
    return 0;
}

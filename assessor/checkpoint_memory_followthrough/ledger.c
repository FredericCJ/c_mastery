#define _POSIX_C_SOURCE 200809L
#include "memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "P1_LEDGER_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                       \
        }                                                                  \
    } while (0)

enum
{
    LEDGER_BLOCKS = 8, /* live allocations the ledger can track */
    SUBSCRIPTION_COOKIE = 71,
    WRONG_COOKIE = 70,
    UNSET = 917, /* sentinel for outputs that must be rewritten */
    ARENA_BACKING_BYTES = 64,
    ARENA_ALIGN = 8
};

/* One live allocation known to the ledger. */
struct p1_block
{
    void *pointer;
    size_t size;
};

/*
 * Instrumented backend. It owns every pointer it hands out until `release`,
 * counts attempts so a test can fail the Nth acquisition (`fail`), and
 * `async` makes unsubscribe complete only on a later acknowledgement.
 */
struct p1_ledger
{
    struct p1_block blocks[LEDGER_BLOCKS];
    unsigned attempts;
    unsigned fail;
    unsigned live;
    unsigned releases;
    unsigned async;
    unsigned unregisters;
    size_t bytes; /* sum of live block sizes */
    size_t peak; /* highest `bytes` ever reached */
};

/* Live blocks are aligned records, do not overlap and match the counters. */
static void verify(struct p1_ledger *l)
{
    unsigned live = 0;
    size_t bytes = 0;
    for (unsigned i = 0; i < LEDGER_BLOCKS; i++)
    {
        if (!l->blocks[i].pointer)
        {
            continue;
        }
        uintptr_t a = (uintptr_t)l->blocks[i].pointer;
        size_t n = l->blocks[i].size;
        CHECK(a % _Alignof(struct cpx_record) == 0 && n <= UINTPTR_MAX - a);
        ++live;
        bytes += n;
        for (unsigned j = i + 1; j < LEDGER_BLOCKS; j++)
        {
            if (!l->blocks[j].pointer)
            {
                continue;
            }
            uintptr_t b = (uintptr_t)l->blocks[j].pointer;
            size_t m = l->blocks[j].size;
            CHECK(m <= UINTPTR_MAX - b && (a + n <= b || b + m <= a));
        }
    }
    CHECK(live == l->live && bytes == l->bytes && bytes <= l->peak);
}

/* Returns NULL for the injected failure; any other failure aborts the test. */
static void *allocate(void *context, size_t n)
{
    struct p1_ledger *l = context;
    if (++l->attempts == l->fail)
    {
        return NULL;
    }
    void *p = malloc(n);
    CHECK(p);
    for (unsigned i = 0; i < LEDGER_BLOCKS; i++)
    {
        if (l->blocks[i].pointer)
        {
            continue;
        }
        l->blocks[i] = (struct p1_block){p, n};
        ++l->live;
        l->bytes += n;
        if (l->bytes > l->peak)
        {
            l->peak = l->bytes;
        }
        verify(l);
        return p;
    }
    CHECK(0); /* the code under test holds more blocks than the ledger tracks */
    return NULL;
}

/* A release of a pointer the ledger does not own is a test failure. */
static void release(void *context, void *p)
{
    struct p1_ledger *l = context;
    for (unsigned i = 0; i < LEDGER_BLOCKS; i++)
    {
        if (l->blocks[i].pointer != p)
        {
            continue;
        }
        l->bytes -= l->blocks[i].size;
        l->blocks[i] = (struct p1_block){0};
        --l->live;
        ++l->releases;
        free(p);
        verify(l);
        return;
    }
    CHECK(0);
}

static int subscribe(void *context, uint64_t *cookie)
{
    struct p1_ledger *l = context;
    if (++l->attempts == l->fail)
    {
        return 0;
    }
    *cookie = SUBSCRIPTION_COOKIE;
    return 1;
}

/* Returns nonzero when the unsubscription completed synchronously. */
static int unsubscribe(void *context, uint64_t cookie)
{
    struct p1_ledger *l = context;
    CHECK(cookie == SUBSCRIPTION_COOKIE);
    ++l->unregisters;
    return !l->async;
}

static struct cpx_backend backend(struct p1_ledger *l)
{
    return (struct cpx_backend){l, allocate, release, subscribe, unsubscribe};
}

/* Record `index` with `length` bytes whose content is index + position. */
static struct cpx_record record(unsigned index, size_t length)
{
    struct cpx_record r = {0};
    r.value = (int)index;
    r.length = length;
    for (size_t i = 0; i < length; i++)
    {
        r.bytes[i] = (unsigned char)(index + i);
    }
    return r;
}

/* cpx_init cut at each of its three acquisitions leaves nothing behind. */
static void check_init_cuts(void)
{
    for (unsigned failure = 1; failure <= 3; failure++)
    {
        struct p1_ledger l = {0};
        l.fail = failure;
        struct cpx_store s = {0};
        unsigned char before[sizeof s];
        memcpy(before, &s, sizeof s);
        CHECK(cpx_init(&s, backend(&l), 4, 4) != CPX_OK &&
              memcmp(before, &s, sizeof s) == 0);
        CHECK(l.live == 0 && l.bytes == 0 &&
              l.releases == (failure == 1 ? 0 : failure - 1));
        verify(&l);
    }
}

/* A cut during growth leaves the old store byte-identical and usable. */
static void check_growth_cuts(void)
{
    for (unsigned failure = 1; failure <= 2; failure++)
    {
        struct p1_ledger l = {0};
        struct cpx_store s = {0};
        CHECK(cpx_init(&s, backend(&l), 2, 2) == CPX_OK);
        struct cpx_record r = record(77, 16);
        struct cpx_handle h;
        CHECK(cpx_allocate(&s, &r, &h) == CPX_OK);
        unsigned char owner_before[sizeof s];
        unsigned char payload_before[2 * sizeof(struct cpx_record)];
        memcpy(owner_before, &s, sizeof s);
        memcpy(payload_before, s.records, sizeof payload_before);
        size_t original_bytes = l.bytes;
        l.fail = l.attempts + failure;
        CHECK(cpx_grow(&s, 4) == CPX_FULL &&
              memcmp(owner_before, &s, sizeof s) == 0 &&
              memcmp(payload_before, s.records, sizeof payload_before) == 0);
        CHECK(l.live == 2 && l.bytes == original_bytes);
        verify(&l);
        l.fail = 0;
        CHECK(cpx_grow(&s, 4) == CPX_OK && l.live == 2 &&
              l.bytes ==
                  4 * (sizeof(struct cpx_record) + sizeof(struct cpx_node)));
        const struct cpx_record *found;
        CHECK(cpx_lookup(&s, h, &found) == CPX_OK && found->value == 77 &&
              found->bytes[15] == 92);
        CHECK(cpx_release(&s, h) == CPX_OK && cpx_close(&s) == CPX_OK &&
              l.bytes == 0);
        verify(&l);
    }
}

/* A node-allocation cut in a batch keeps exactly the accepted prefix. */
static void check_node_cuts(void)
{
    for (unsigned failure = 1; failure <= 4; failure++)
    {
        struct p1_ledger l = {0};
        struct cpx_store s = {0};
        CHECK(cpx_init(&s, backend(&l), 4, 4) == CPX_OK);
        s.node_fail_at = failure;
        struct cpx_record rows[4];
        for (unsigned i = 0; i < 4; i++)
        {
            rows[i] = record(i + 1, i + 1);
        }
        size_t accepted = UNSET;
        CHECK(cpx_batch(&s, rows, 4, &accepted) == CPX_FULL &&
              accepted == failure - 1);
        unsigned logical = 0;
        unsigned pins = 0;
        for (unsigned i = 0; i < 4; i++)
        {
            logical += s.state[i] == 1;
            pins += s.pins[i];
        }
        CHECK(logical == accepted && pins == accepted && s.count == accepted);
        verify(&l);
        for (unsigned i = 0; i < accepted; i++)
        {
            struct cpx_handle h;
            const struct cpx_record *r;
            CHECK(cpx_pop(&s, &h) == CPX_OK &&
                  cpx_lookup(&s, h, &r) == CPX_OK && r->value == (int)i + 1 &&
                  r->length == i + 1);
            CHECK(cpx_release(&s, h) == CPX_OK);
        }
        CHECK(cpx_close(&s) == CPX_OK && l.live == 0 && l.bytes == 0);
        verify(&l);
    }
}

/* Close stays pending, and keeps memory, until the cookie is acknowledged. */
static void check_asynchronous_close(void)
{
    struct p1_ledger l = {0};
    l.async = 1;
    struct cpx_store s = {0};
    CHECK(cpx_init(&s, backend(&l), 4, 4) == CPX_OK);
    size_t retained = l.bytes;
    CHECK(cpx_close(&s) == CPX_PENDING && cpx_close(&s) == CPX_PENDING &&
          l.unregisters == 1 && l.bytes == retained);
    CHECK(cpx_acknowledge(&s, WRONG_COOKIE, 1) == CPX_INVALID &&
          cpx_acknowledge(&s, SUBSCRIPTION_COOKIE, 0) == CPX_PENDING &&
          l.bytes == retained);
    CHECK(cpx_acknowledge(&s, SUBSCRIPTION_COOKIE, 1) == CPX_OK &&
          l.bytes == 0 && l.releases == 2);
    CHECK(cpx_close(&s) == CPX_OK &&
          cpx_acknowledge(&s, SUBSCRIPTION_COOKIE, 1) == CPX_INVALID &&
          l.releases == 2);
}

static void failures(void)
{
    check_init_cuts();
    check_growth_cuts();
    check_node_cuts();
    check_asynchronous_close();
    puts(
        "P1_LEDGER_PASS init_cuts=3 growth_cuts=2 node_cuts=4 live_extents alignment contents asynchronous_cleanup");
}

enum
{
    NS_PER_SECOND = 1000000000,
    ROWS = 4, /* records offered per iteration */
    ITERATIONS = 128, /* iterations per sample */
    WARMUP_SAMPLES = 3, /* discarded */
    SAMPLES = 31, /* reported */
    DISTRIBUTIONS = 3,
    HEADER_BYTES_PER_SPAN = 3,
    POOL_CAPACITY = 4 /* records and nodes of the measured pool */
};

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC_RAW, &t) == 0);
    return (uint64_t)t.tv_sec * (uint64_t)NS_PER_SECOND + (uint64_t)t.tv_nsec;
}

/* Keeps the checksum observable so the read loops cannot be optimised away. */
static volatile size_t consumed;

/*
 * Workload under measurement. `distribution` picks the record lengths
 * (0: all 1 byte, 1: all 16 bytes, 2: 1, 4, 8, 16); `pool` selects the
 * typed-record pool instead of the 64-byte arena; `instrument` adds
 * bookkeeping inside the timed interval.
 */
struct measure_case
{
    unsigned distribution;
    unsigned instrument;
    unsigned pool;
};

/* Totals over the iterations of one sample. */
struct totals
{
    uint64_t ns;
    size_t payload;
    size_t headers;
    size_t padding;
    size_t accepted;
    size_t retained;
    size_t backing;
    size_t peak;
    size_t checksum;
};

static void lengths_for(unsigned distribution, size_t lengths[ROWS])
{
    static const size_t mixed[ROWS] = {1, 4, 8, 16};
    for (unsigned i = 0; i < ROWS; i++)
    {
        if (distribution == 0)
        {
            lengths[i] = 1;
        }
        else if (distribution == 1)
        {
            lengths[i] = 16;
        }
        else
        {
            lengths[i] = mixed[i];
        }
    }
}

/* Sum of the lengths of the pool records currently live. */
static size_t pool_live_bytes(const struct cpx_store *store)
{
    size_t live = 0;
    for (unsigned j = 0; j < POOL_CAPACITY; j++)
    {
        if (store->state[j] == 1)
        {
            live += store->records[j].length;
        }
    }
    return live;
}

struct offer_result
{
    size_t accepted;
    size_t payload;
    size_t observed_peak; /* instrumented runs only */
};

/*
 * Row i is stored at index i whether or not it was accepted. read_back treats
 * the first `accepted` entries as valid, which is a property of the four fixed
 * workloads only: their lengths never decrease, so once a row is refused every
 * later row is refused too. The instrumentation cross-checks the payload
 * against the pool's own state after every offer; it runs inside the timed
 * interval by design.
 */
static struct offer_result
offer_rows(const struct measure_case *c, struct cpx_store *store,
           struct cpm_arena *arena, unsigned char *bytes,
           const struct cpx_record rows[ROWS], const size_t lengths[ROWS],
           struct cpx_handle *handles, struct cpm_span *spans)
{
    struct offer_result result = {0, 0, 0};
    size_t payload = 0;
    size_t accepted = 0;
    size_t observed_peak = 0;
    for (unsigned i = 0; i < ROWS; i++)
    {
        int good = c->pool
                       ? cpx_allocate(store, &rows[i], &handles[i]) == CPX_OK
                       : cpm_reserve(arena, lengths[i], &spans[i]) == CPM_OK;
        if (good)
        {
            if (!c->pool)
            {
                memcpy(bytes + spans[i].offset, rows[i].bytes, lengths[i]);
            }
            payload += lengths[i];
            ++accepted;
        }
        if (c->instrument)
        {
            size_t live = c->pool ? pool_live_bytes(store) : payload;
            CHECK(live == payload);
            if (live > observed_peak)
            {
                observed_peak = live;
            }
        }
    }
    result.accepted = accepted;
    result.payload = payload;
    result.observed_peak = observed_peak;
    return result;
}

/* Reads every accepted row back, releases pool rows and returns a checksum. */
static size_t read_back(const struct measure_case *c, struct cpx_store *store,
                        const unsigned char *bytes, size_t accepted,
                        const size_t lengths[ROWS],
                        const struct cpx_handle *handles,
                        const struct cpm_span *spans)
{
    size_t checksum = 0;
    for (size_t i = 0; i < accepted; i++)
    {
        if (c->pool)
        {
            const struct cpx_record *r;
            CHECK(cpx_lookup(store, handles[i], &r) == CPX_OK &&
                  r->length == lengths[i]);
            for (size_t j = 0; j < r->length; j++)
            {
                checksum += r->bytes[j];
            }
            CHECK(cpx_release(store, handles[i]) == CPX_OK);
        }
        else
        {
            for (size_t j = 0; j < spans[i].length; j++)
            {
                checksum += bytes[spans[i].offset + j];
            }
        }
    }
    return checksum;
}

/*
 * One iteration. Backing allocation and teardown are outside the timed
 * interval, which covers offering, reading back and logical release.
 */
static void run_iteration(const struct measure_case *c, struct totals *t)
{
    struct p1_ledger l = {0};
    struct cpx_store store = {0};
    _Alignas(8) unsigned char bytes[ARENA_BACKING_BYTES];
    struct cpm_arena arena;
    CHECK(cpm_arena_init(&arena, bytes, ARENA_BACKING_BYTES, ARENA_ALIGN, 1) ==
          CPM_OK);
    if (c->pool)
    {
        CHECK(cpx_init(&store, backend(&l), POOL_CAPACITY, POOL_CAPACITY) ==
              CPX_OK);
    }
    size_t lengths[ROWS];
    lengths_for(c->distribution, lengths);
    struct cpx_record rows[ROWS];
    for (unsigned i = 0; i < ROWS; i++)
    {
        rows[i] = record(i + 1, lengths[i]);
    }
    struct cpx_handle handles[ROWS];
    struct cpm_span spans[ROWS];
    uint64_t begin = now();
    struct offer_result offered =
        offer_rows(c, &store, &arena, bytes, rows, lengths, handles, spans);
    size_t accepted = offered.accepted;
    size_t payload = offered.payload;
    t->checksum +=
        read_back(c, &store, bytes, accepted, lengths, handles, spans);
    uint64_t end = now();
    CHECK(end >= begin);
    t->ns += end - begin;
    /* Arena distribution 1 (4 x 16 bytes + headers) must refuse the 4th. */
    CHECK(accepted == (!c->pool && c->distribution == 1 ? 3u : ROWS));
    if (c->instrument)
    {
        CHECK(offered.observed_peak == payload);
    }
    t->payload += payload;
    t->accepted += accepted;
    if (!c->pool)
    {
        size_t headers = HEADER_BYTES_PER_SPAN * accepted;
        t->headers += headers;
        t->padding += arena.used - payload - headers;
        t->retained += arena.used;
    }
    t->backing = c->pool ? l.bytes : sizeof bytes;
    t->peak = c->pool ? l.peak : sizeof bytes;
    if (c->pool)
    {
        CHECK(cpx_close(&store) == CPX_OK && l.bytes == 0);
    }
    else
    {
        CHECK(cpm_reset(&arena) == CPM_OK);
    }
}

/* Totals over 128 iterations, from an independent derivation of the rows. */
static void check_sample_totals(const struct measure_case *c,
                                const struct totals *t)
{
    static const struct
    {
        size_t checksum;
        size_t payload;
    } expected[DISTRIBUTIONS][2] = {
        {{10, 4}, {10, 4}}, {{456, 48}, {640, 64}}, {{251, 29}, {251, 29}}};

    CHECK(t->checksum ==
          ITERATIONS * expected[c->distribution][c->pool].checksum);
    CHECK(t->payload ==
          ITERATIONS * expected[c->distribution][c->pool].payload);
    CHECK(t->headers == (c->pool ? 0 : HEADER_BYTES_PER_SPAN * t->accepted) &&
          t->padding == 0);
}

static void print_sample(const struct measure_case *c, int sample,
                         const struct totals *t)
{
    printf(
        "{\"distribution\":%u,\"instrumented\":%u,\"pool\":%u,\"sample\":%d,\"iterations\":128,\"ns\":%llu,\"offered\":512,\"accepted\":%zu,\"payload\":%zu,\"headers\":%zu,\"padding\":%zu,\"retained_after_logical_release\":%zu,\"backing\":%zu,\"peak_backing\":%zu,\"control_bytes\":%zu,\"pool_slot_payload_capacity\":64,\"checksum\":%zu}\n",
        c->distribution, c->instrument, c->pool, sample,
        (unsigned long long)t->ns, t->accepted, t->payload, t->headers,
        t->padding, t->retained, t->backing, t->peak,
        c->pool ? sizeof(struct cpx_store) : sizeof(struct cpm_arena),
        t->checksum);
}

static void measurements(void)
{
    for (unsigned distribution = 0; distribution < DISTRIBUTIONS;
         distribution++)
    {
        for (unsigned instrument = 0; instrument < 2; instrument++)
        {
            for (unsigned pool = 0; pool < 2; pool++)
            {
                struct measure_case c = {distribution, instrument, pool};
                for (int sample = -WARMUP_SAMPLES; sample < SAMPLES; sample++)
                {
                    struct totals t = {0};
                    for (unsigned i = 0; i < ITERATIONS; i++)
                    {
                        run_iteration(&c, &t);
                    }
                    consumed = t.checksum;
                    check_sample_totals(&c, &t);
                    if (sample >= 0)
                    {
                        print_sample(&c, sample, &t);
                    }
                }
            }
        }
    }
    puts(
        "P1_MEASURE_PASS samples=372 paired_workloads allocation_outside_interval explicit_different_acceptance");
}

int main(int argc, char **argv)
{
    failures();
    if (argc == 2 && strcmp(argv[1], "--measure") == 0)
    {
        measurements();
    }
    else if (argc != 1)
    {
        return 2;
    }
    return 0;
}

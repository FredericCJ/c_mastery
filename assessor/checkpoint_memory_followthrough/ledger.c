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

struct p1_block
{
    void *pointer;
    size_t size;
};

struct p1_ledger
{
    struct p1_block blocks[8];
    unsigned attempts, fail, live, releases, async, unregisters;
    size_t bytes, peak;
};

static void verify(struct p1_ledger *l)
{
    unsigned live = 0;
    size_t bytes = 0;
    for (unsigned i = 0; i < 8; i++)
        if (l->blocks[i].pointer)
        {
            uintptr_t a = (uintptr_t)l->blocks[i].pointer;
            size_t n = l->blocks[i].size;
            CHECK(a % _Alignof(struct cpx_record) == 0 && n <= UINTPTR_MAX - a);
            ++live;
            bytes += n;
            for (unsigned j = i + 1; j < 8; j++)
                if (l->blocks[j].pointer)
                {
                    uintptr_t b = (uintptr_t)l->blocks[j].pointer;
                    size_t m = l->blocks[j].size;
                    CHECK(m <= UINTPTR_MAX - b && (a + n <= b || b + m <= a));
                }
        }
    CHECK(live == l->live && bytes == l->bytes && bytes <= l->peak);
}

static void *allocate(void *context, size_t n)
{
    struct p1_ledger *l = context;
    if (++l->attempts == l->fail)
        return NULL;
    void *p = malloc(n);
    CHECK(p);
    for (unsigned i = 0; i < 8; i++)
        if (!l->blocks[i].pointer)
        {
            l->blocks[i] = (struct p1_block){p, n};
            ++l->live;
            l->bytes += n;
            if (l->bytes > l->peak)
                l->peak = l->bytes;
            verify(l);
            return p;
        }
    CHECK(0);
    return NULL;
}

static void release(void *context, void *p)
{
    struct p1_ledger *l = context;
    for (unsigned i = 0; i < 8; i++)
        if (l->blocks[i].pointer == p)
        {
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
        return 0;
    *cookie = 71;
    return 1;
}

static int unsubscribe(void *context, uint64_t cookie)
{
    struct p1_ledger *l = context;
    CHECK(cookie == 71);
    ++l->unregisters;
    return !l->async;
}

static struct cpx_backend backend(struct p1_ledger *l)
{
    return (struct cpx_backend){l, allocate, release, subscribe, unsubscribe};
}

static struct cpx_record record(unsigned index, size_t length)
{
    struct cpx_record r = {0};
    r.value = (int)index;
    r.length = length;
    for (size_t i = 0; i < length; i++)
        r.bytes[i] = (unsigned char)(index + i);
    return r;
}

static void failures(void)
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
    for (unsigned failure = 1; failure <= 2; failure++)
    {
        struct p1_ledger l = {0};
        struct cpx_store s = {0};
        CHECK(cpx_init(&s, backend(&l), 2, 2) == CPX_OK);
        struct cpx_record r = record(77, 16);
        struct cpx_handle h;
        CHECK(cpx_allocate(&s, &r, &h) == CPX_OK);
        unsigned char owner_before[sizeof s],
            payload_before[2 * sizeof(struct cpx_record)];
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
    for (unsigned failure = 1; failure <= 4; failure++)
    {
        struct p1_ledger l = {0};
        struct cpx_store s = {0};
        CHECK(cpx_init(&s, backend(&l), 4, 4) == CPX_OK);
        s.node_fail_at = failure;
        struct cpx_record rows[4];
        for (unsigned i = 0; i < 4; i++)
            rows[i] = record(i + 1, i + 1);
        size_t accepted = 917;
        CHECK(cpx_batch(&s, rows, 4, &accepted) == CPX_FULL &&
              accepted == failure - 1);
        unsigned logical = 0, pins = 0;
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
    struct p1_ledger l = {0};
    l.async = 1;
    struct cpx_store s = {0};
    CHECK(cpx_init(&s, backend(&l), 4, 4) == CPX_OK);
    size_t retained = l.bytes;
    CHECK(cpx_close(&s) == CPX_PENDING && cpx_close(&s) == CPX_PENDING &&
          l.unregisters == 1 && l.bytes == retained);
    CHECK(cpx_acknowledge(&s, 70, 1) == CPX_INVALID &&
          cpx_acknowledge(&s, 71, 0) == CPX_PENDING && l.bytes == retained);
    CHECK(cpx_acknowledge(&s, 71, 1) == CPX_OK && l.bytes == 0 &&
          l.releases == 2);
    CHECK(cpx_close(&s) == CPX_OK &&
          cpx_acknowledge(&s, 71, 1) == CPX_INVALID && l.releases == 2);
    puts(
        "P1_LEDGER_PASS init_cuts=3 growth_cuts=2 node_cuts=4 live_extents alignment contents asynchronous_cleanup");
}

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC_RAW, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static volatile size_t consumed;

static void measurements(void)
{
    for (unsigned distribution = 0; distribution < 3; distribution++)
        for (unsigned instrument = 0; instrument < 2; instrument++)
            for (unsigned pool = 0; pool < 2; pool++)
                for (int sample = -3; sample < 31; sample++)
                {
                    uint64_t ns = 0;
                    size_t all_payload = 0, all_headers = 0, all_padding = 0,
                           all_accepted = 0, all_retained = 0, backing = 0,
                           peak = 0, checksum = 0;
                    for (unsigned iteration = 0; iteration < 128; iteration++)
                    {
                        struct p1_ledger l = {0};
                        struct cpx_store store = {0};
                        _Alignas(8) unsigned char bytes[64];
                        struct cpm_arena arena;
                        CHECK(cpm_arena_init(&arena, bytes, 64, 8, 1) ==
                              CPM_OK);
                        if (pool)
                            CHECK(cpx_init(&store, backend(&l), 4, 4) ==
                                  CPX_OK);
                        size_t lengths[4] = {1, 4, 8, 16};
                        if (distribution < 2)
                            for (unsigned i = 0; i < 4; i++)
                                lengths[i] = distribution ? 16 : 1;
                        struct cpx_record rows[4];
                        for (unsigned i = 0; i < 4; i++)
                            rows[i] = record(i + 1, lengths[i]);
                        size_t payload = 0, accepted = 0, observed_peak = 0;
                        struct cpx_handle handles[4];
                        struct cpm_span spans[4];
                        uint64_t begin = now();
                        for (unsigned i = 0; i < 4; i++)
                        {
                            int good = pool
                                           ? cpx_allocate(&store, &rows[i],
                                                          &handles[i]) == CPX_OK
                                           : cpm_reserve(&arena, lengths[i],
                                                         &spans[i]) == CPM_OK;
                            if (good)
                            {
                                if (!pool)
                                    memcpy(bytes + spans[i].offset,
                                           rows[i].bytes, lengths[i]);
                                payload += lengths[i];
                                ++accepted;
                            }
                            if (instrument)
                            {
                                size_t live = 0;
                                if (pool)
                                {
                                    for (unsigned j = 0; j < 4; j++)
                                        if (store.state[j] == 1)
                                            live += store.records[j].length;
                                }
                                else
                                    live = payload;
                                CHECK(live == payload);
                                if (live > observed_peak)
                                    observed_peak = live;
                            }
                        }
                        for (unsigned i = 0; i < accepted; i++)
                        {
                            if (pool)
                            {
                                const struct cpx_record *r;
                                CHECK(cpx_lookup(&store, handles[i], &r) ==
                                          CPX_OK &&
                                      r->length == lengths[i]);
                                for (size_t j = 0; j < r->length; j++)
                                    checksum += r->bytes[j];
                                CHECK(cpx_release(&store, handles[i]) ==
                                      CPX_OK);
                            }
                            else
                                for (size_t j = 0; j < spans[i].length; j++)
                                    checksum += bytes[spans[i].offset + j];
                        }
                        uint64_t end = now();
                        CHECK(end >= begin);
                        ns += end - begin;
                        CHECK(accepted ==
                              (!pool && distribution == 1 ? 3u : 4u));
                        if (instrument)
                            CHECK(observed_peak == payload);
                        all_payload += payload;
                        all_accepted += accepted;
                        all_headers += pool ? 0 : 3 * accepted;
                        all_padding +=
                            pool ? 0 : arena.used - payload - 3 * accepted;
                        all_retained += pool ? 0 : arena.used;
                        backing = pool ? l.bytes : sizeof bytes;
                        peak = pool ? l.peak : sizeof bytes;
                        if (pool)
                            CHECK(cpx_close(&store) == CPX_OK && l.bytes == 0);
                        else
                            CHECK(cpm_reset(&arena) == CPM_OK);
                    }
                    consumed = checksum;
                    CHECK(checksum == 128 * (distribution == 0 ? 10u
                                             : distribution == 1
                                                 ? (pool ? 640u : 456u)
                                                 : 251u));
                    CHECK(all_payload == 128 * (distribution == 0 ? 4u
                                                : distribution == 1
                                                    ? (pool ? 64u : 48u)
                                                    : 29u));
                    CHECK(all_headers == (pool ? 0 : 3 * all_accepted) &&
                          all_padding == 0);
                    if (sample >= 0)
                        printf(
                            "{\"distribution\":%u,\"instrumented\":%u,\"pool\":%u,\"sample\":%d,\"iterations\":128,\"ns\":%llu,\"offered\":512,\"accepted\":%zu,\"payload\":%zu,\"headers\":%zu,\"padding\":%zu,\"retained_after_logical_release\":%zu,\"backing\":%zu,\"peak_backing\":%zu,\"control_bytes\":%zu,\"pool_slot_payload_capacity\":64,\"checksum\":%zu}\n",
                            distribution, instrument, pool, sample,
                            (unsigned long long)ns, all_accepted, all_payload,
                            all_headers, all_padding, all_retained, backing,
                            peak,
                            pool ? sizeof(struct cpx_store)
                                 : sizeof(struct cpm_arena),
                            checksum);
                }
    puts(
        "P1_MEASURE_PASS samples=372 paired_workloads allocation_outside_interval explicit_different_acceptance");
}

int main(int argc, char **argv)
{
    failures();
    if (argc == 2 && strcmp(argv[1], "--measure") == 0)
        measurements();
    else if (argc != 1)
        return 2;
    return 0;
}

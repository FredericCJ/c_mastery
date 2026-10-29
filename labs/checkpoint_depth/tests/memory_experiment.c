#define _POSIX_C_SOURCE 200809L
#include "checkpoint_memory.h"
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define REQUIRE(x)                                              \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "CP_MEMORY_CHECK: %d\n", __LINE__); \
            exit(1);                                            \
        }                                                       \
    } while (0)
static volatile uint64_t sink;

static uint64_t now(void)
{
    struct timespec t;
    REQUIRE(!clock_gettime(CLOCK_MONOTONIC_RAW, &t));
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static unsigned count_live(const struct cpm_pool *p)
{
    unsigned count = 0;
    for (unsigned i = 0; i < CPM_SLOTS; ++i)
        count += p->state[i] == 1;
    return count;
}

int main(void)
{
    _Alignas(8) unsigned char bytes[32];
    struct cpm_arena arena;
    struct cpm_span first, second;
    REQUIRE(cpm_arena_init(&arena, bytes, sizeof bytes, 8, 8) == CPM_OK &&
            cpm_reserve(&arena, 5, &first) == CPM_OK &&
            cpm_reserve(&arena, 9, &second) == CPM_OK);
    REQUIRE(first.offset == 8 && second.offset == 16 && arena.used == 25);
    size_t payload = first.length + second.length;
    printf(
        "{\"kind\":\"layout\",\"pool_bytes\":%zu,\"ring_bytes\":%zu,\"payload_capacity_bytes\":%zu,\"arena_capacity\":%zu,\"arena_used\":%zu,\"arena_payload\":%zu,\"arena_headers\":6,\"arena_padding\":%zu,\"arena_tail\":%zu}\n",
        sizeof(struct cpm_pool), sizeof(struct cpm_ring),
        sizeof(struct cpm_record) * CPM_SLOTS, arena.capacity, arena.used,
        payload, arena.used - payload - 6, arena.capacity - arena.used);
    for (unsigned heap = 0; heap < 2; ++heap)
        for (unsigned fail = 0; fail < 2; ++fail)
            for (unsigned instrument = 0; instrument < 2; ++instrument)
                for (int sample = -5; sample < 31; ++sample)
                {
                    struct cpm_pool local,
                        *pool = heap ? malloc(sizeof *pool) : &local;
                    REQUIRE(pool);
                    struct cpm_ring ring;
                    uint64_t total = 0;
                    unsigned peak = 0;
                    uint64_t begin = now();
                    for (unsigned round = 0; round < 2000; ++round)
                    {
                        cpm_pool_init(pool);
                        cpm_ring_init(&ring);
                        if (fail)
                            cpm_fail_at(pool, 3);
                        int values[] = {11, 22, 33, 44};
                        size_t accepted;
                        enum cpm_status status =
                            cpm_prefix(pool, &ring, values, 4, &accepted);
                        REQUIRE(status == (fail ? CPM_FULL : CPM_OK) &&
                                accepted == (fail ? 2u : 4u));
                        if (instrument)
                        {
                            unsigned live = count_live(pool);
                            if (live > peak)
                                peak = live;
                            REQUIRE(live == accepted);
                        }
                        for (size_t i = 0; i < accepted; ++i)
                        {
                            struct cpm_handle h;
                            const struct cpm_record *r;
                            REQUIRE(cpm_pop(&ring, &h) == CPM_OK &&
                                    cpm_lookup(pool, h, &r) == CPM_OK);
                            total += (unsigned)r->value;
                            REQUIRE(cpm_release(pool, h) == CPM_OK);
                        }
                        if (instrument)
                            REQUIRE(count_live(pool) == 0);
                    }
                    uint64_t end = now();
                    REQUIRE(total == (fail ? 66000u : 220000u));
                    sink = total;
                    if (sample >= 0)
                        printf(
                            "{\"kind\":\"sample\",\"heap_backing\":%u,\"failure_ordinal\":%u,\"instrumented\":%u,\"sample\":%d,\"rounds\":2000,\"ns\":%llu,\"peak_live\":%u,\"sink\":%llu}\n",
                            heap, fail ? 3 : 0, instrument, sample,
                            (unsigned long long)(end - begin), peak,
                            (unsigned long long)sink);
                    if (heap)
                        free(pool);
                }
    return 0;
}

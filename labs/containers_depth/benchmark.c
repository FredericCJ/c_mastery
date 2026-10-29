#define _POSIX_C_SOURCE 200809L
#include "containers.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "CONTAINERS_BENCH_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static int *take(void *c, size_t n)
{
    (void)c;
    return malloc(n * sizeof(int));
}

static void release(void *c, int *p)
{
    (void)c;
    free(p);
}

static struct cd_bucket *htake(void *c, unsigned n)
{
    (void)c;
    return malloc(n * sizeof(struct cd_bucket));
}

static void hrelease(void *c, struct cd_bucket *p)
{
    (void)c;
    free(p);
}

int main(void)
{
    for (unsigned n = 4; n <= 16; n *= 2)
        for (unsigned cluster = 0; cluster < 2; ++cluster)
        {
            struct cd_vector vector;
            CHECK(cd_vector_init(&vector,
                                 (struct cd_allocator){0, take, release}) ==
                      CD_OK &&
                  cd_vector_reserve(&vector, n) == CD_OK);
            struct cd_list list;
            cd_list_init(&list);
            struct cd_link nodes[16] = {{0}};
            struct cd_hash hash;
            CHECK(cd_hash_init(&hash, n,
                               (struct cd_hash_allocator){0, htake, hrelease},
                               cluster) == CD_OK);
            unsigned probes;
            for (unsigned i = 0; i < n; ++i)
            {
                int value = (int)((i * 5) % n);
                CHECK(cd_vector_insert(&vector, vector.length, value) == CD_OK);
                nodes[i].value = value;
                CHECK(cd_list_push(&list, &nodes[i], 0) == CD_OK &&
                      cd_hash_put(&hash, (uint32_t)value, value, &probes) ==
                          CD_OK);
            }
            for (unsigned sample = 0; sample < 5; ++sample)
                for (unsigned kind = 0; kind < 3; ++kind)
                {
                    uint64_t comparisons = 0, sum = 0, start = now();
                    for (unsigned round = 0; round < 2000; ++round)
                    {
                        for (unsigned q = 0; q < n; ++q)
                        {
                            unsigned key = (q * 5 + round) % n;
                            int found = -1;
                            if (kind == 0)
                            {
                                for (unsigned i = 0; i < n; ++i)
                                {
                                    ++comparisons;
                                    if (vector.data[i] == (int)key)
                                    {
                                        found = vector.data[i];
                                        break;
                                    }
                                }
                            }
                            else if (kind == 1)
                            {
                                for (struct cd_link *p = list.head; p;
                                     p = p->next)
                                {
                                    ++comparisons;
                                    if (p->value == (int)key)
                                    {
                                        found = p->value;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                CHECK(cd_hash_get(&hash, key, &found,
                                                  &probes) == CD_OK);
                                comparisons += probes;
                            }
                            CHECK(found == (int)key);
                            sum += (unsigned)found;
                        }
                    }
                    uint64_t elapsed = now() - start;
                    CHECK(sum == UINT64_C(2000) * n * (n - 1) / 2);
                    printf(
                        "{\"workload\":\"lookup-permuted\",\"size\":%u,\"clustered\":%u,\"representation\":%u,\"sample\":%u,\"operations\":%u,\"nanoseconds\":%" PRIu64
                        ",\"comparisons_or_probes\":%" PRIu64
                        ",\"checksum\":%" PRIu64 "}\n",
                        n, cluster, kind, sample, 2000 * n, elapsed,
                        comparisons, sum);
                }
            for (unsigned sample = 0; sample < 5; ++sample)
            {
                uint64_t totalprobes = 0, sum = 0, start = now();
                for (unsigned round = 0; round < 2000; ++round)
                {
                    unsigned key = round % n;
                    CHECK(cd_hash_delete(&hash, key) == CD_OK &&
                          cd_hash_put(&hash, key, (int)key, &probes) == CD_OK);
                    totalprobes += probes;
                    int value;
                    CHECK(cd_hash_get(&hash, key, &value, &probes) == CD_OK);
                    totalprobes += probes;
                    sum += (unsigned)value;
                }
                uint64_t elapsed = now() - start;
                printf(
                    "{\"workload\":\"delete-insert-lookup-churn\",\"size\":%u,\"clustered\":%u,\"sample\":%u,\"operations\":6000,\"nanoseconds\":%" PRIu64
                    ",\"insert_lookup_probes\":%" PRIu64
                    ",\"checksum\":%" PRIu64 "}\n",
                    n, cluster, sample, elapsed, totalprobes, sum);
            }
            /* Indexed-read control uses logical positions; the hash is not an ordered-sequence substitute. */
            uint64_t steps = 0, sum = 0, start = now();
            for (unsigned round = 0; round < 2000; ++round)
                for (unsigned i = 0; i < n; ++i)
                {
                    struct cd_link *p = list.head;
                    for (unsigned j = 0; j < i; ++j)
                    {
                        p = p->next;
                        ++steps;
                    }
                    CHECK(p->value == vector.data[i]);
                    sum += (unsigned)p->value;
                }
            printf(
                "{\"workload\":\"list-indexed-vs-array-check\",\"size\":%u,\"nanoseconds\":%" PRIu64
                ",\"pointer_steps\":%" PRIu64 ",\"checksum\":%" PRIu64 "}\n",
                n, now() - start, steps, sum);
            CHECK(cd_vector_destroy(&vector) == CD_OK &&
                  cd_hash_destroy(&hash) == CD_OK);
        }
    return 0;
}

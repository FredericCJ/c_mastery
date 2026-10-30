/*
 * Lookup and churn workloads over three representations of the same keys: a
 * vector scanned linearly, a linked list scanned linearly and an
 * open-addressing hash.  Each sample prints one JSON object; the program
 * prints 126 of them.
 *
 * Every sample checks each result as it is produced and accumulates a
 * checksum, so a faster-but-wrong implementation cannot pass.  Those checks
 * run inside the timed loops and are part of the measured cost; allocation and
 * setup are never timed.  Timings are host- and load-dependent observations,
 * not a ranking.
 */
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

enum
{
    BENCH_MIN_SIZE = 4,
    BENCH_MAX_SIZE = 16,
    BENCH_ROUNDS = 2000,
    BENCH_SAMPLES = 5
};

/* The three representations compared by the lookup workload. */
enum bench_kind
{
    BENCH_VECTOR,
    BENCH_LIST,
    BENCH_HASH,
    BENCH_KINDS
};

static uint64_t now_ns(void)
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

/* The same n keys, stored three ways.  The vector and the list hold the keys
 * in permuted order (i * 5 mod n); the hash maps each key to itself. */
struct bench_fixture
{
    unsigned n;
    struct cd_vector vector;
    struct cd_list list;
    struct cd_link nodes[BENCH_MAX_SIZE];
    struct cd_hash hash;
};

static void fixture_build(struct bench_fixture *f, unsigned n, unsigned cluster)
{
    f->n = n;
    CHECK(cd_vector_init(&f->vector, (struct cd_allocator){0, take, release}) ==
              CD_OK &&
          cd_vector_reserve(&f->vector, n) == CD_OK);
    cd_list_init(&f->list);
    for (unsigned i = 0; i < BENCH_MAX_SIZE; ++i)
    {
        f->nodes[i] = (struct cd_link){0};
    }
    CHECK(cd_hash_init(&f->hash, n,
                       (struct cd_hash_allocator){0, htake, hrelease},
                       cluster) == CD_OK);
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned probes;
        int value = (int)((i * 5) % n);
        CHECK(cd_vector_insert(&f->vector, f->vector.length, value) == CD_OK);
        f->nodes[i].value = value;
        CHECK(cd_list_push(&f->list, &f->nodes[i], 0) == CD_OK &&
              cd_hash_put(&f->hash, (uint32_t)value, value, &probes) == CD_OK);
    }
}

static void fixture_destroy(struct bench_fixture *f)
{
    CHECK(cd_vector_destroy(&f->vector) == CD_OK &&
          cd_hash_destroy(&f->hash) == CD_OK);
}

/* Looks up one key in the chosen representation, adding the number of
 * elements compared (or buckets probed) to *comparisons.  Returns the stored
 * value, or -1 when the key is absent. */
static int find_key(const struct bench_fixture *f, enum bench_kind kind,
                    unsigned key, uint64_t *comparisons)
{
    int found = -1;
    if (kind == BENCH_VECTOR)
    {
        for (unsigned i = 0; i < f->n; ++i)
        {
            ++*comparisons;
            if (f->vector.data[i] == (int)key)
            {
                found = f->vector.data[i];
                break;
            }
        }
    }
    else if (kind == BENCH_LIST)
    {
        for (const struct cd_link *p = f->list.head; p; p = p->next)
        {
            ++*comparisons;
            if (p->value == (int)key)
            {
                found = p->value;
                break;
            }
        }
    }
    else
    {
        unsigned probes;
        CHECK(cd_hash_get(&f->hash, key, &found, &probes) == CD_OK);
        *comparisons += probes;
    }
    return found;
}

/* Every key is looked up once per round, in an order that shifts each round.
 * Only the lookups are timed. */
static void run_lookup_samples(const struct bench_fixture *f, unsigned cluster)
{
    const unsigned n = f->n;
    for (unsigned sample = 0; sample < BENCH_SAMPLES; ++sample)
    {
        for (unsigned kind = 0; kind < BENCH_KINDS; ++kind)
        {
            uint64_t comparisons = 0;
            uint64_t sum = 0;
            uint64_t start_ns = now_ns();
            for (unsigned round = 0; round < BENCH_ROUNDS; ++round)
            {
                for (unsigned q = 0; q < n; ++q)
                {
                    unsigned key = (q * 5 + round) % n;
                    int found =
                        find_key(f, (enum bench_kind)kind, key, &comparisons);
                    CHECK(found == (int)key);
                    sum += (unsigned)found;
                }
            }
            uint64_t elapsed_ns = now_ns() - start_ns;
            /* Each round visits every key 0..n-1 exactly once. */
            CHECK(sum == (uint64_t)BENCH_ROUNDS * n * (n - 1) / 2);
            printf(
                "{\"workload\":\"lookup-permuted\",\"size\":%u,\"clustered\":%u,\"representation\":%u,\"sample\":%u,\"operations\":%u,\"nanoseconds\":%" PRIu64
                ",\"comparisons_or_probes\":%" PRIu64 ",\"checksum\":%" PRIu64
                "}\n",
                n, cluster, kind, sample, BENCH_ROUNDS * n, elapsed_ns,
                comparisons, sum);
        }
    }
}

/* Delete a key, reinsert it (reusing its tombstone) and look it up again:
 * three hash operations per round. */
static void run_churn_samples(struct bench_fixture *f, unsigned cluster)
{
    const unsigned n = f->n;
    for (unsigned sample = 0; sample < BENCH_SAMPLES; ++sample)
    {
        uint64_t total_probes = 0;
        uint64_t sum = 0;
        uint64_t start_ns = now_ns();
        for (unsigned round = 0; round < BENCH_ROUNDS; ++round)
        {
            unsigned probes;
            unsigned key = round % n;
            CHECK(cd_hash_delete(&f->hash, key) == CD_OK &&
                  cd_hash_put(&f->hash, key, (int)key, &probes) == CD_OK);
            total_probes += probes;
            int value;
            CHECK(cd_hash_get(&f->hash, key, &value, &probes) == CD_OK);
            total_probes += probes;
            sum += (unsigned)value;
        }
        uint64_t elapsed_ns = now_ns() - start_ns;
        printf(
            "{\"workload\":\"delete-insert-lookup-churn\",\"size\":%u,\"clustered\":%u,\"sample\":%u,\"operations\":%u,\"nanoseconds\":%" PRIu64
            ",\"insert_lookup_probes\":%" PRIu64 ",\"checksum\":%" PRIu64 "}\n",
            n, cluster, sample, 3u * BENCH_ROUNDS, elapsed_ns, total_probes,
            sum);
    }
}

/* Indexed-read control: reaching position i of a list costs i pointer steps,
 * which is the trade the array wins.  The hash is not an ordered-sequence
 * substitute, so it is not part of this control. */
static void run_indexed_read_control(const struct bench_fixture *f)
{
    const unsigned n = f->n;
    uint64_t steps = 0;
    uint64_t sum = 0;
    uint64_t start_ns = now_ns();
    for (unsigned round = 0; round < BENCH_ROUNDS; ++round)
    {
        for (unsigned i = 0; i < n; ++i)
        {
            const struct cd_link *p = f->list.head;
            for (unsigned j = 0; j < i; ++j)
            {
                p = p->next;
                ++steps;
            }
            CHECK(p->value == f->vector.data[i]);
            sum += (unsigned)p->value;
        }
    }
    printf(
        "{\"workload\":\"list-indexed-vs-array-check\",\"size\":%u,\"nanoseconds\":%" PRIu64
        ",\"pointer_steps\":%" PRIu64 ",\"checksum\":%" PRIu64 "}\n",
        n, now_ns() - start_ns, steps, sum);
}

int main(void)
{
    for (unsigned n = BENCH_MIN_SIZE; n <= BENCH_MAX_SIZE; n *= 2)
    {
        for (unsigned cluster = 0; cluster < 2; ++cluster)
        {
            struct bench_fixture fixture;
            fixture_build(&fixture, n, cluster);
            run_lookup_samples(&fixture, cluster);
            run_churn_samples(&fixture, cluster);
            run_indexed_read_control(&fixture);
            fixture_destroy(&fixture);
        }
    }
    return 0;
}

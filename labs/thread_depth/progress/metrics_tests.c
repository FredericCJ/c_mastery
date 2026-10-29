#define _GNU_SOURCE
#include "metrics.h"
#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "TP_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(41);                                                        \
        }                                                                    \
    } while (0)

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

struct tw_counter
{
    _Atomic(unsigned) *counter;
    unsigned iterations;
    pthread_barrier_t *start;
};

static void *count(void *arg)
{
    struct tw_counter *c = arg;
    int e = pthread_barrier_wait(c->start);
    CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
    for (unsigned i = 0; i < c->iterations; i++)
    {
        atomic_fetch_add_explicit(c->counter, 1, memory_order_relaxed);
    }
    return NULL;
}

static void layout(void)
{
    CHECK(tp_separated(0, 4, 2, 4, 64) == 0);
    CHECK(tp_separated(0, 64, 2, 4, 64) == 1);
    CHECK(tp_separated(0, 64, 2, 4, 128) == 0);
    CHECK(tp_separated(4, 64, 2, 64, 64) == 0);
    CHECK(tp_separated(0, 64, 2, 64, 64) == 1);
    CHECK(tp_separated(UINTPTR_MAX - 1, 64, 2, 4, 64) == -1);
    long detected = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    CHECK(detected >= 8 && detected <= 4096 && !(detected & (detected - 1)));
    size_t line = (size_t)detected;
    cpu_set_t allowed;
    CHECK(sched_getaffinity(0, sizeof allowed, &allowed) == 0);
    printf("LAYOUT reported_line=%zu allowed_cpus=%d affinity_unchanged=1\n",
           line, CPU_COUNT(&allowed));
    for (unsigned repeat = 0; repeat < 7; repeat++)
    {
        for (unsigned separated = 0; separated < 2; separated++)
        {
            size_t stride = separated ? line : sizeof(_Atomic(unsigned));
            void *allocation = NULL;
            CHECK(posix_memalign(&allocation, line, 2 * stride) == 0);
            struct tw_counter args[2];
            pthread_t threads[2];
            pthread_barrier_t start;
            CHECK(pthread_barrier_init(&start, NULL, 3) == 0);
            for (unsigned i = 0; i < 2; i++)
            {
                args[i] = (struct tw_counter){
                    .counter =
                        (_Atomic(unsigned) *)((unsigned char *)allocation +
                                              i * stride),
                    .iterations = repeat ? 100000 : 1000,
                    .start = &start};
                atomic_init(args[i].counter, 0);
                CHECK(pthread_create(&threads[i], NULL, count, &args[i]) == 0);
            }
            int separation = tp_separated((uintptr_t)allocation, stride, 2,
                                          sizeof(_Atomic(unsigned)), line);
            CHECK(separation == (int)separated);
            uint64_t begin = now();
            int e = pthread_barrier_wait(&start);
            CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
            for (unsigned i = 0; i < 2; i++)
            {
                CHECK(pthread_join(threads[i], NULL) == 0);
                CHECK(atomic_load(args[i].counter) == args[i].iterations);
            }
            uint64_t elapsed = now() - begin;
            printf(
                "SAMPLE layout warmup=%u repeat=%u separated=%u base=%" PRIuPTR
                " second=%" PRIuPTR
                " stride=%zu iterations=%u nanoseconds=%" PRIu64 "\n",
                !repeat, repeat, separated, (uintptr_t)args[0].counter,
                (uintptr_t)args[1].counter, stride, args[0].iterations,
                elapsed);
            CHECK(pthread_barrier_destroy(&start) == 0);
            free(allocation);
        }
    }
    puts(
        "OBS layout actual adjacent/separate single-owner atomics; changed line "
        "and multi-hot-byte alignment model; no speedup requirement");
}

struct tw_acc
{
    const unsigned *data;
    size_t n;
    unsigned id, mode, intensity;
    _Atomic(uint64_t) *shared;
    struct tp_work work;
};

static void *acc(void *p)
{
    struct tw_acc *a = p;
    CHECK(tp_accumulate(a->data, a->n, a->id, 2, a->mode, 32, a->intensity,
                        a->shared, &a->work) == TP_OK);
    return NULL;
}

static uint32_t reference_mix(uint32_t x, unsigned intensity)
{
    for (unsigned i = 0; i < intensity; i++)
    {
        uint32_t shift = x / 128u;
        x = x * 1664525u + 1013904223u;
        x ^= shift;
    }
    return x;
}

static void aggregation(void)
{
    unsigned data[20000];
    uint64_t expected = 0;
    for (size_t i = 0; i < 20000; i++)
    {
        data[i] = (unsigned)(i % 97) + 1;
        expected += data[i];
    }
    for (unsigned intensity = 0; intensity <= 32; intensity += 32)
    {
        uint64_t checksum = 0;
        for (size_t i = 0; i < 20000; i++)
        {
            checksum += reference_mix(data[i], intensity);
        }
        for (unsigned repeat = 0; repeat < 7; repeat++)
        {
            for (unsigned mode = 0; mode < 3; mode++)
            {
                _Atomic(uint64_t) shared;
                atomic_init(&shared, 0);
                struct tw_acc a[2];
                pthread_t t[2];
                uint64_t start = now();
                for (unsigned i = 0; i < 2; i++)
                {
                    a[i] = (struct tw_acc){.data = data,
                                           .n = 20000,
                                           .id = i,
                                           .mode = mode,
                                           .intensity = intensity,
                                           .shared = &shared};
                    CHECK(pthread_create(&t[i], NULL, acc, &a[i]) == 0);
                }
                uint64_t sum = 0, check = 0, updates = 0;
                for (unsigned i = 0; i < 2; i++)
                {
                    CHECK(pthread_join(t[i], NULL) == 0);
                    sum += a[i].work.sum;
                    check += a[i].work.checksum;
                    updates += a[i].work.shared_updates;
                    CHECK(a[i].work.items == 10000);
                }
                uint64_t elapsed = now() - start;
                CHECK(sum == expected && check == checksum);
                CHECK(mode == 0 ? atomic_load(&shared) == 0
                                : atomic_load(&shared) == expected);
                CHECK(updates == (mode == 0 ? 0 : mode == 1 ? 626 : 20000));
                printf(
                    "SAMPLE aggregation warmup=%u repeat=%u mode=%u intensity=%u "
                    "sum=%" PRIu64 " checksum=%" PRIu64
                    " shared_updates=%" PRIu64 " nanoseconds=%" PRIu64 "\n",
                    !repeat, repeat, mode, intensity, sum, check, updates,
                    elapsed);
            }
        }
    }
    puts(
        "OBS aggregation private/batch32/per-item exact sum; explicit merge and "
        "visibility differences; changed intensity0/32");
}

struct tw_fresh
{
    struct tp_total *total;
    atomic_int *stop;
    unsigned reads;
};

static void *fresh_writer(void *arg)
{
    struct tp_total *t = arg;
    for (unsigned i = 0; i < 10000; i++)
    {
        CHECK(tp_total_add(t, 1) == TP_OK);
    }
    return NULL;
}

static void *fresh_reader(void *arg)
{
    struct tw_fresh *f = arg;
    do
    {
        uint64_t value, items;
        CHECK(tp_total_read(f->total, &value, &items) == 0);
        CHECK(value == items);
        f->reads++;
    } while (!atomic_load(f->stop));
    return NULL;
}

static void freshness(void)
{
    struct tp_total total;
    CHECK(tp_total_init(&total) == 0);
    atomic_int stop;
    atomic_init(&stop, 0);
    struct tw_fresh f = {.total = &total, .stop = &stop};
    pthread_t r, w[2];
    CHECK(pthread_create(&r, NULL, fresh_reader, &f) == 0);
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_create(&w[i], NULL, fresh_writer, &total) == 0);
    }
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_join(w[i], NULL) == 0);
    }
    atomic_store(&stop, 1);
    CHECK(pthread_join(r, NULL) == 0);
    uint64_t value, items;
    CHECK(tp_total_read(&total, &value, &items) == 0 && value == 20000 &&
          items == 20000 && f.reads > 0);
    total.value = UINT64_MAX;
    total.items = 3;
    CHECK(tp_total_add(&total, 1) == TP_OVERFLOW);
    CHECK(tp_total_read(&total, &value, &items) == 0 && value == UINT64_MAX &&
          items == 3);
    CHECK(tp_total_destroy(&total) == 0);
    puts("OBS freshness live mutex snapshot linearizes current committed "
         "totals;20000 updates; bounded publication differs from private/batch "
         "freshness");
}

static void cost(void)
{
    struct timespec resolution;
    CHECK(clock_getres(CLOCK_MONOTONIC, &resolution) == 0);
    printf("TIMER resolution_seconds=%ld resolution_nanoseconds=%ld\n",
           (long)resolution.tv_sec, resolution.tv_nsec);
    pthread_mutex_t mutex;
    CHECK(pthread_mutex_init(&mutex, NULL) == 0);
    uint64_t overhead[32];
    for (unsigned i = 0; i < 32; i++)
    {
        uint64_t a = now();
        overhead[i] = now() - a;
        printf("TIMER overhead%u=%" PRIu64 "\n", i, overhead[i]);
    }
    for (unsigned intensity = 1; intensity <= 32; intensity += 31)
    {
        for (unsigned repeat = 0; repeat < 7; repeat++)
        {
            for (unsigned mode = 0; mode < 3; mode++)
            {
                uint64_t expected = 0;
                for (unsigned i = 0; i < 20000; i++)
                {
                    expected += reference_mix(i % 97 + 1, intensity);
                }
                uint64_t sum = 0, locks = 0, start = now();
                for (unsigned i = 0; i < 20000; i++)
                {
                    if (mode != 1)
                    {
                        CHECK(pthread_mutex_lock(&mutex) == 0);
                        locks++;
                    }
                    if (mode != 0)
                    {
                        sum += tp_mix(i % 97 + 1, intensity);
                    }
                    if (mode != 1)
                    {
                        CHECK(pthread_mutex_unlock(&mutex) == 0);
                    }
                }
                uint64_t elapsed = now() - start;
                CHECK(locks == (mode == 1 ? 0 : 20000));
                CHECK(sum == (mode == 0 ? 0 : expected));
                printf("SAMPLE cost warmup=%u repeat=%u mode=%u intensity=%u "
                       "locks=%" PRIu64 " checksum=%" PRIu64
                       " nanoseconds=%" PRIu64 "\n",
                       !repeat, repeat, mode, intensity, locks, sum, elapsed);
            }
        }
    }
    CHECK(pthread_mutex_destroy(&mutex) == 0);
    puts("OBS cost matched20000 empty-lock/useful/composed "
         "iterations;timer32pairs;intensity1/32; no subtraction identity or "
         "universal bottleneck claim");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "layout"))
    {
        layout();
    }
    else if (!strcmp(argv[1], "aggregation"))
    {
        aggregation();
    }
    else if (!strcmp(argv[1], "freshness"))
    {
        freshness();
    }
    else if (!strcmp(argv[1], "cost"))
    {
        cost();
    }
    else
    {
        return 2;
    }
    return 0;
}

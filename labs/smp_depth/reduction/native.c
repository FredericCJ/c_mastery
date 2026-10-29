#define _GNU_SOURCE
#include "reduction.h"
#include "reduction_float.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 65539u
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "SR_NATIVE_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                                 \
        }                                                             \
    } while (0)
static uint8_t bytes[N];
static atomic_uint visits[N];
static _Alignas(128) atomic_uint_fast64_t bins[64];

struct sa_native
{
    struct sr_world world;
    unsigned workers, stride, cpus[4], observed[4];
    atomic_uint ready, go, abort, done, bad;
    uint64_t compute_ns[4];
};

struct sa_argument
{
    struct sa_native *n;
    unsigned id;
};

static uint64_t now(clockid_t clock)
{
    struct timespec t;
    if (clock_gettime(clock, &t))
    {
        perror("clock_gettime");
        exit(2);
    }
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static void *work(void *opaque)
{
    struct sa_argument *arg = opaque;
    struct sa_native *n = arg->n;
    unsigned id = arg->id;
    cpu_set_t allowed;
    CPU_ZERO(&allowed);
    CPU_SET(n->cpus[id], &allowed);
    if (pthread_setaffinity_np(pthread_self(), sizeof allowed, &allowed))
    {
        atomic_store(&n->bad, 1);
    }
    n->observed[id] = (unsigned)sched_getcpu();
    atomic_fetch_add(&n->ready, 1);
    while (!atomic_load_explicit(&n->go, memory_order_acquire))
    {
        sched_yield();
    }
    if (!atomic_load(&n->abort))
    {
        uint64_t begin = now(CLOCK_THREAD_CPUTIME_ID);
        if (sr_work(&n->world, id, SIZE_MAX) != SR_OK)
        {
            atomic_store(&n->bad, 1);
        }
        struct sr_range range;
        if (sr_partition(N, n->workers, id, &range) != SR_OK)
        {
            atomic_store(&n->bad, 1);
        }
        else
        {
            for (size_t i = range.begin; i < range.end; i++)
            {
                atomic_fetch_add_explicit(bins + id * n->stride +
                                              (bytes[i] & 3u),
                                          1, memory_order_relaxed);
            }
        }
        n->compute_ns[id] = now(CLOCK_THREAD_CPUTIME_ID) - begin;
    }
    atomic_fetch_add_explicit(&n->done, 1, memory_order_release);
    return NULL;
}

static int trial(unsigned workers, unsigned layout, unsigned index,
                 const unsigned *cpus, unsigned fail_after)
{
    struct sa_native n = {0};
    n.workers = workers;
    n.stride = layout ? 16 : 4;
    for (unsigned i = 0; i < workers; i++)
    {
        n.cpus[i] = cpus[i];
    }
    for (unsigned i = 0; i < 64; i++)
    {
        atomic_init(bins + i, 0);
    }
    CHECK(atomic_is_lock_free(bins));
    CHECK(sr_init(&n.world, bytes, visits, N, workers, SR_BYTES) == SR_OK);
    uint64_t wall0 = now(CLOCK_MONOTONIC), cpu0 = now(CLOCK_PROCESS_CPUTIME_ID);
    pthread_t threads[4];
    struct sa_argument args[4];
    unsigned made = 0;
    for (unsigned i = 0; i < workers; i++)
    {
        args[i] = (struct sa_argument){&n, i};
        if (i == fail_after)
        {
            break;
        }
        int result = pthread_create(threads + i, NULL, work, args + i);
        if (result)
        {
            atomic_store(&n.bad, 1);
            break;
        }
        made++;
    }
    while (atomic_load(&n.ready) != made)
    {
        sched_yield();
    }
    uint64_t ready = now(CLOCK_MONOTONIC);
    if (made != workers || atomic_load(&n.bad))
    {
        atomic_store(&n.abort, 1);
    }
    atomic_store_explicit(&n.go, 1, memory_order_release);
    while (atomic_load_explicit(&n.done, memory_order_acquire) != made)
    {
        sched_yield();
    }
    uint64_t terminal = now(CLOCK_MONOTONIC);
    for (unsigned i = 0; i < made; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            exit(2);
        }
    }
    uint64_t joined = now(CLOCK_MONOTONIC);
    CHECK(!atomic_load(&n.bad));
    struct sr_result result = {0};
    result.hash = 91;
    int finished = sr_finish(&n.world, &result);
    uint64_t complete = now(CLOCK_MONOTONIC),
             cpu1 = now(CLOCK_PROCESS_CPUTIME_ID);
    if (made != workers)
    {
        CHECK(finished == SR_INCOMPLETE && result.hash == 91);
        for (unsigned i = 0; i < N; i++)
        {
            CHECK(atomic_load(visits + i) == 0);
        }
        printf("SR_LAUNCH_FAIL created=%u joined=%u visits=0 unchanged=1\n",
               made, made);
        return 0;
    }
    CHECK(finished == SR_OK && result.count == N);
    uint64_t total[4] = {0}, compute = 0;
    for (unsigned i = 0; i < workers; i++)
    {
        CHECK(n.observed[i] == cpus[i]);
        compute += n.compute_ns[i];
        for (unsigned b = 0; b < 4; b++)
        {
            total[b] += atomic_load(bins + i * n.stride + b);
        }
    }
    for (unsigned b = 0; b < 4; b++)
    {
        uint64_t expected = 0;
        for (unsigned j = b; j < 16; j += 4)
        {
            expected += result.bins[j];
        }
        CHECK(total[b] == expected);
    }
    for (unsigned i = 0; i < N; i++)
    {
        CHECK(atomic_load(visits + i) == 1);
    }
    printf(
        "SR_NATIVE trial=%u workers=%u layout=%u count=%u wall_ns=%llu cpu_ns=%llu launch_ns=%llu join_tail_ns=%llu combine_ns=%llu worker_cpu_ns=%llu hash=%llu\n",
        index, workers, layout, N, (unsigned long long)(complete - wall0),
        (unsigned long long)(cpu1 - cpu0), (unsigned long long)(ready - wall0),
        (unsigned long long)(joined - terminal),
        (unsigned long long)(complete - joined), (unsigned long long)compute,
        (unsigned long long)result.hash);
    for (unsigned i = 0; i < workers; i++)
    {
        printf("SR_PLACE trial=%u workers=%u layout=%u worker=%u cpu=%u\n",
               index, workers, layout, i, n.observed[i]);
    }
    return 0;
}

struct float_arg
{
    unsigned id, workers;
    int compensation;
    struct sr_float value;
    int result;
};

static void *floating(void *opaque)
{
    struct float_arg *a = opaque;
    const double values[] = {1e16, 1, -1e16, 1};
    a->result = sr_float_chunk(values, 4, a->workers, a->id, a->compensation,
                               &a->value);
    return NULL;
}

static int floats(void)
{
    for (unsigned workers = 1; workers <= 4; workers *= 2)
    {
        for (unsigned repeat = 0; repeat < 3; repeat++)
        {
            for (int comp = 0; comp <= 1; comp++)
            {
                pthread_t threads[4];
                struct float_arg args[4];
                struct sr_float parts[4];
                unsigned made = 0;
                for (unsigned i = 0; i < workers; i++)
                {
                    args[i] = (struct float_arg){
                        i, workers, comp, {0, 0}, SR_INVALID};
                    if (pthread_create(threads + i, NULL, floating, args + i))
                    {
                        break;
                    }
                    made++;
                }
                if (made != workers)
                {
                    for (unsigned i = 0; i < made; i++)
                    {
                        if (pthread_join(threads[i], NULL))
                        {
                            exit(2);
                        }
                    }
                    CHECK(made == workers);
                }
                for (unsigned i = workers; i; i--)
                {
                    CHECK(!pthread_join(threads[i - 1], NULL));
                    CHECK(args[i - 1].result == SR_OK);
                    parts[i - 1] = args[i - 1].value;
                }
                double result;
                CHECK(sr_float_join(parts, workers, comp, &result) == SR_OK);
                CHECK(result == (comp ? 2 : workers == 2 ? 0 : 1));
                printf(
                    "SR_FLOAT workers=%u repeat=%u compensated=%d value=%a exact=2\n",
                    workers, repeat, comp, result);
            }
        }
    }
    return 0;
}

int main(void)
{
    cpu_set_t allowed;
    CHECK(!sched_getaffinity(0, sizeof allowed, &allowed));
    unsigned cpus[4], count = 0;
    for (unsigned i = 0; i < CPU_SETSIZE && count < 4; i++)
    {
        if (CPU_ISSET(i, &allowed))
        {
            cpus[count++] = i;
        }
    }
    CHECK(count == 4);
    for (unsigned i = 0; i < N; i++)
    {
        bytes[i] = (uint8_t)((i * 7 + 3) % 16);
    }
    for (unsigned fail = 0; fail < 4; fail++)
    {
        CHECK(!trial(4, 0, 0, cpus, fail));
    }
    for (unsigned index = 0; index < 3; index++)
    {
        for (unsigned workers = 1; workers <= 4; workers *= 2)
        {
            for (unsigned layout = 0; layout < 2; layout++)
            {
                CHECK(!trial(workers, layout, index, cpus, 4));
            }
        }
    }
    CHECK(!floats());
    puts("SR_NATIVE_PASS:ownership-clock-layout-floating");
    return 0;
}

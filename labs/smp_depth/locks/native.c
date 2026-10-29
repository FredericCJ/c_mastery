#define _GNU_SOURCE
#include "locks.h"
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

enum
{
    PARTICIPANTS = 4,
    ITERATIONS = 2000
};

struct sa_worker
{
    struct sl_node node;
    struct sl_stats stats;
    unsigned id, cpu, acquired, max_gap, failed;
    int node_id;
    uint64_t sum_ns, max_ns;
};
static struct sl_lock lock;
static struct sa_worker workers[PARTICIPANTS];
static atomic_uint ready, go;
static unsigned sequence, order[PARTICIPANTS * ITERATIONS];
static uint64_t payload;

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
        abort();
    return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}

static int node_of(unsigned cpu)
{
    char path[160];
    for (unsigned n = 0; n < 64; n++)
    {
        snprintf(path, sizeof path, "/sys/devices/system/node/node%u/cpu%u", n,
                 cpu);
        if (access(path, F_OK) == 0)
            return (int)n;
    }
    return -1;
}

static void *run(void *context)
{
    struct sa_worker *w = context;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(w->cpu, &set);
    if (pthread_setaffinity_np(pthread_self(), sizeof set, &set))
    {
        w->failed = 1;
        return NULL;
    }
    atomic_fetch_add_explicit(&ready, 1, memory_order_release);
    while (!atomic_load_explicit(&go, memory_order_acquire))
        sched_yield();
    for (unsigned i = 0; i < ITERATIONS; i++)
    {
        struct sl_guard guard = {0};
        uint64_t begin = now();
        int result = sl_enter(&lock, &w->node, &guard, &w->stats);
        uint64_t waited = now() - begin;
        if (result)
        {
            w->failed = 1;
            return NULL;
        }
        if (sequence >= PARTICIPANTS * ITERATIONS)
        {
            w->failed = 1;
            return NULL;
        }
        order[sequence++] = w->id;
        for (unsigned j = 0; j < 16; j++)
            payload += j;
        if (sl_leave(&guard, &w->stats))
        {
            w->failed = 1;
            return NULL;
        }
        w->acquired++;
        w->sum_ns += waited;
        if (waited > w->max_ns)
            w->max_ns = waited;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int require_numa = argc == 2 && argv[1][0] == 'n';
    cpu_set_t allowed;
    unsigned selected[PARTICIPANTS], count = 0;
    if (sched_getaffinity(0, sizeof allowed, &allowed))
        return 2;
    for (unsigned cpu = 0; cpu < CPU_SETSIZE && count < PARTICIPANTS; cpu++)
        if (CPU_ISSET(cpu, &allowed))
            selected[count++] = cpu;
    if (count < PARTICIPANTS)
        return 3;
    if (require_numa &&
        node_of(selected[0]) == node_of(selected[PARTICIPANTS - 1]))
        return 4;
    for (unsigned kind = 0; kind < 3; kind++)
    {
        if (sl_init(&lock, kind, 1, kind == SL_TICKET ? 14 : 0))
            return 5;
        sequence = 0;
        payload = 0;
        atomic_store(&ready, 0);
        atomic_store(&go, 0);
        pthread_t threads[PARTICIPANTS];
        for (unsigned i = 0; i < PARTICIPANTS; i++)
        {
            workers[i] = (struct sa_worker){
                .id = i, .cpu = selected[i], .node_id = node_of(selected[i])};
            sl_node_init(&workers[i].node);
            if (pthread_create(&threads[i], NULL, run, &workers[i]))
                return 6;
        }
        uint64_t start = now();
        while (atomic_load_explicit(&ready, memory_order_acquire) !=
               PARTICIPANTS)
        {
            if (now() - start > UINT64_C(10000000000))
                return 7;
            sched_yield();
        }
        atomic_store_explicit(&go, 1, memory_order_release);
        for (unsigned i = 0; i < PARTICIPANTS; i++)
            if (pthread_join(threads[i], NULL))
                return 8;
        uint64_t elapsed = now() - start;
        if (sequence != PARTICIPANTS * ITERATIONS ||
            payload != 120u * PARTICIPANTS * ITERATIONS)
            return 9;
        for (unsigned i = 0; i < PARTICIPANTS; i++)
        {
            unsigned seen = 0, previous = 0;
            for (unsigned j = 0; j < sequence; j++)
                if (order[j] == i)
                {
                    unsigned gap = seen ? j - previous - 1 : j;
                    if (gap > workers[i].max_gap)
                        workers[i].max_gap = gap;
                    previous = j;
                    seen++;
                }
            struct sa_worker *w = &workers[i];
            if (w->failed || w->acquired != ITERATIONS || seen != ITERATIONS ||
                atomic_load(&w->node.active))
                return 10;
            printf(
                "SL_COST kind=%u participant=%u cpu=%u node=%d acquisitions=%u exchanges=%" PRIu64
                " failed=%" PRIu64 " waiting=%" PRIu64 " wait_sum_ns=%" PRIu64
                " wait_max_ns=%" PRIu64 " gap=%u elapsed_ns=%" PRIu64
                " count=%u sum=%" PRIu64 "\n",
                kind, i, w->cpu, w->node_id, w->acquired, w->stats.exchanges,
                w->stats.failed_exchanges, w->stats.waiting_loads, w->sum_ns,
                w->max_ns, w->max_gap, elapsed, sequence, payload);
        }
    }
    puts(require_numa ? "SL_NATIVE_PASS:equal-work-cross-virtual-NUMA"
                      : "SL_NATIVE_PASS:equal-work-native-logical-CPUs");
    return 0;
}

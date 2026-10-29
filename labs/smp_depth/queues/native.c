#define _POSIX_C_SOURCE 200809L
#include "queues.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

enum
{
    N = 4,
    ROUNDS = 2000
};

struct sa_worker
{
    unsigned cpu, failed;
    struct sl_stats stats;
    uint64_t work;
};
static struct sq_world world;
static atomic_uint ready, go;
static struct sa_worker workers[N];

static uint64_t clock_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}

static void *worker(void *context)
{
    struct sa_worker *w = context;
    struct sq_handle task;
    struct sq_event event;
    atomic_fetch_add_explicit(&ready, 1, memory_order_release);
    while (!atomic_load_explicit(&go, memory_order_acquire))
    {
        sched_yield();
    }
    for (unsigned i = 0; i < ROUNDS; i++)
    {
        if (sq_dispatch(&world, w->cpu, &task, &event, &w->stats) != SQ_OK)
        {
            w->failed = 1;
            return NULL;
        }
        for (unsigned j = 0; j < 16; j++)
        {
            w->work += j;
        }
        unsigned next =
            i + 1 == ROUNDS ? SQ_EXITED : (i % 7 == 0 ? SQ_BLOCKED : SQ_READY);
        if (sq_finish(&world, w->cpu, task, next, &event, &w->stats) != SQ_OK)
        {
            w->failed = 2;
            return NULL;
        }
        if (next == SQ_BLOCKED &&
            sq_wake(&world, w->cpu, w->cpu, task, &event, &w->stats) != SQ_OK)
        {
            w->failed = 3;
            return NULL;
        }
    }
    return NULL;
}

static void *mover(void *context)
{
    struct sa_worker *w = context;
    unsigned home = w->cpu;
    struct sq_handle id = {w->cpu, 1};
    struct sq_transfer ticket;
    struct sq_event event;
    atomic_fetch_add_explicit(&ready, 1, memory_order_release);
    while (!atomic_load_explicit(&go, memory_order_acquire))
    {
        sched_yield();
    }
    for (unsigned i = 0; i < ROUNDS; i++)
    {
        if (sq_transfer_begin(&world, w->cpu, home, 1 - home, id, &ticket,
                              &event, &w->stats) != SQ_OK ||
            sq_transfer_end(&world, w->cpu, ticket, 1, &event, &w->stats) !=
                SQ_OK)
        {
            w->failed = 1;
            return NULL;
        }
        home = 1 - home;
        w->work++;
    }
    return NULL;
}

int main(void)
{
    for (unsigned coarse = 0; coarse < 2; coarse++)
    {
        if (sq_init(&world, N, 2, coarse) != SQ_OK)
        {
            return 1;
        }
        atomic_store(&ready, 0);
        atomic_store(&go, 0);
        pthread_t threads[N];
        for (unsigned i = 0; i < N; i++)
        {
            struct sq_handle id;
            struct sq_event e;
            if (sq_create(&world, 0, i, i, &id, &e) != SQ_OK)
            {
                return 2;
            }
            workers[i] = (struct sa_worker){.cpu = i};
            if (pthread_create(&threads[i], NULL, worker, &workers[i]))
            {
                return 3;
            }
        }
        while (atomic_load_explicit(&ready, memory_order_acquire) != N)
        {
            sched_yield();
        }
        uint64_t begin = clock_ns();
        atomic_store_explicit(&go, 1, memory_order_release);
        for (unsigned i = 0; i < N; i++)
        {
            if (pthread_join(threads[i], NULL))
            {
                return 4;
            }
        }
        uint64_t elapsed = clock_ns() - begin;
        if (!sq_invariant(&world))
        {
            return 5;
        }
        for (unsigned i = 0; i < N; i++)
        {
            struct sa_worker *w = &workers[i];
            if (w->failed || w->work != 240000 ||
                atomic_load(&world.task[i].state) != SQ_EXITED)
            {
                return 6;
            }
            printf("SQ_COST coarse=%u actor=%u rounds=%u work=%" PRIu64
                   " exchanges=%" PRIu64 " failed=%" PRIu64 " waiting=%" PRIu64
                   " elapsed_ns=%" PRIu64 "\n",
                   coarse, i, ROUNDS, w->work, w->stats.exchanges,
                   w->stats.failed_exchanges, w->stats.waiting_loads, elapsed);
        }
        if (sq_init(&world, 2, 4, coarse) != SQ_OK)
        {
            return 7;
        }
        atomic_store(&ready, 0);
        atomic_store(&go, 0);
        for (unsigned i = 0; i < 2; i++)
        {
            struct sq_handle id;
            struct sq_event e;
            if (sq_create(&world, 0, i, i, &id, &e) != SQ_OK)
            {
                return 8;
            }
            workers[i] = (struct sa_worker){.cpu = i};
            if (pthread_create(&threads[i], NULL, mover, &workers[i]))
            {
                return 9;
            }
        }
        while (atomic_load_explicit(&ready, memory_order_acquire) != 2)
        {
            sched_yield();
        }
        atomic_store_explicit(&go, 1, memory_order_release);
        for (unsigned i = 0; i < 2; i++)
        {
            if (pthread_join(threads[i], NULL))
            {
                return 10;
            }
        }
        if (!sq_invariant(&world))
        {
            return 11;
        }
        for (unsigned i = 0; i < 2; i++)
        {
            if (workers[i].failed || workers[i].work != ROUNDS ||
                atomic_load(&world.task[i].home) != i ||
                world.task[i].revision != 1 + 2 * ROUNDS)
            {
                return 12;
            }
        }
        printf(
            "SQ_MOVES coarse=%u actors=2 transfers=4000 revisions=4001 invariant=1\n",
            coarse);
    }
    puts("SQ_NATIVE_PASS:equal-contract-four-actors");
    return 0;
}

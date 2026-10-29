#define _GNU_SOURCE
#include "balance.h"
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum
{
    WORDS = 32768,
    QUANTA = 10
};
static struct sb_world world;
static struct sq_handle ids[4];
static uint64_t data[4][WORDS];
static unsigned progress[4], seen[4], logical[2], physical[2];
static uint64_t first_ns[4], repeat_ns[4], checksum[4];
static atomic_uint done, failed, ready, go, executing[4];
static struct sl_stats stats[2];

static int ring(void *ctx, unsigned cpu)
{
    (void)ctx;
    (void)cpu;
    return 1;
}

static void ack(void *ctx, unsigned cpu)
{
    (void)ctx;
    (void)cpu;
}

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    return (uint64_t)t.tv_sec * 1000000000 + (uint64_t)t.tv_nsec;
}

static void fail(void)
{
    atomic_store(&failed, 1);
}

static void *worker(void *arg)
{
    unsigned cpu = *(unsigned *)arg;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(physical[cpu], &set);
    if (pthread_setaffinity_np(pthread_self(), sizeof set, &set))
    {
        fail();
        return NULL;
    }
    atomic_fetch_add(&ready, 1);
    while (!atomic_load(&go) && !atomic_load(&failed))
    {
        sched_yield();
    }
    while (atomic_load(&done) < 4 && !atomic_load(&failed))
    {
        struct sq_handle id;
        struct sq_event event;
        int r = sb_dispatch(&world, cpu, &id, &event, &stats[cpu]);
        if (r == SQ_IDLE)
        {
            sched_yield();
            continue;
        }
        if (r != SQ_OK)
        {
            fail();
            break;
        }
        unsigned slot = id.slot;
        if (slot >= 4 || sched_getcpu() != (int)physical[cpu] ||
            atomic_exchange(&executing[slot], 1))
        {
            fail();
            break;
        }
        if (slot == 0 && cpu != 0)
        {
            fail();
            break;
        }
        seen[slot] |= 1u << cpu;
        for (unsigned pass = 0; pass < 2; pass++)
        {
            uint64_t begin = now(), sum = 0;
            for (unsigned i = 0; i < WORDS; i++)
            {
                data[slot][i]++;
                sum += data[slot][i];
            }
            uint64_t elapsed = now() - begin;
            checksum[slot] += sum;
            if (pass)
            {
                repeat_ns[slot] += elapsed;
            }
            else
            {
                first_ns[slot] += elapsed;
            }
        }
        progress[slot]++;
        atomic_store(&executing[slot], 0);
        if (sb_finish(&world, cpu, id,
                      progress[slot] == QUANTA ? SQ_EXITED : SQ_READY, &event,
                      &stats[cpu]) != SQ_OK)
        {
            fail();
            break;
        }
        if (progress[slot] == QUANTA)
        {
            atomic_fetch_add(&done, 1);
        }
    }
    return NULL;
}

int main(void)
{
    cpu_set_t allowed;
    CPU_ZERO(&allowed);
    if (sched_getaffinity(0, sizeof allowed, &allowed))
    {
        perror("sched_getaffinity");
        return 2;
    }
    unsigned n = 0;
    for (unsigned i = 0; i < CPU_SETSIZE && n < 2; i++)
    {
        if (CPU_ISSET(i, &allowed))
        {
            physical[n++] = i;
        }
    }
    if (n != 2)
    {
        fputs("SB_ENV:two allowed native CPUs required\n", stderr);
        return 2;
    }
    for (unsigned trial = 0; trial < 3; trial++)
    {
        for (unsigned order = 0; order < 2; order++)
        {
            unsigned policy = (trial + order) % 2;
            atomic_init(&done, 0);
            atomic_init(&failed, 0);
            atomic_init(&ready, 0);
            atomic_init(&go, 0);
            if (sb_init(&world, 2, 4, (struct sw_port){ring, ack, NULL}) !=
                SQ_OK)
            {
                return 1;
            }
            struct sq_event e, last;
            for (unsigned slot = 0; slot < 4; slot++)
            {
                atomic_init(&executing[slot], 0);
                progress[slot] = seen[slot] = 0;
                first_ns[slot] = repeat_ns[slot] = checksum[slot] = 0;
                for (unsigned i = 0; i < WORDS; i++)
                {
                    data[slot][i] = 0;
                }
                if (sb_create(&world, 0, 0, slot, policy && slot ? 3 : 1,
                              &ids[slot], &e) != SQ_OK)
                {
                    return 1;
                }
            }
            uint64_t balance_start = now();
            if (policy)
            {
                if (sb_balance(&world, 0, &e, &last, NULL) != SQ_OK ||
                    sb_balance(&world, 0, &e, &last, NULL) != SQ_OK)
                {
                    return 1;
                }
            }
            uint64_t balance_ns = now() - balance_start;
            unsigned imbalance = world.wake.queues.queue[0].count -
                                 world.wake.queues.queue[1].count;
            pthread_t threads[2];
            for (unsigned c = 0; c < 2; c++)
            {
                logical[c] = c;
                stats[c] = (struct sl_stats){0};
                if (pthread_create(&threads[c], NULL, worker, &logical[c]))
                {
                    return 2;
                }
            }
            while (atomic_load(&ready) < 2 && !atomic_load(&failed))
            {
                sched_yield();
            }
            uint64_t begin = now();
            atomic_store(&go, 1);
            for (unsigned c = 0; c < 2; c++)
            {
                if (pthread_join(threads[c], NULL))
                {
                    return 2;
                }
            }
            uint64_t elapsed = now() - begin;
            if (atomic_load(&failed) || atomic_load(&done) != 4 ||
                !sb_invariant(&world))
            {
                fputs("SB_NATIVE_CHECK:work-or-ownership\n", stderr);
                return 1;
            }
            uint64_t sum = 0, migrations = 0, first = 0, repeat = 0;
            for (unsigned slot = 0; slot < 4; slot++)
            {
                if (progress[slot] != QUANTA ||
                    seen[slot] !=
                        (policy && (slot == 1 || slot == 2) ? 2u : 1u))
                {
                    return 1;
                }
                sum += checksum[slot];
                migrations += world.migrations[slot];
                first += first_ns[slot];
                repeat += repeat_ns[slot];
            }
            if (sum != UINT64_C(27525120) || migrations != (policy ? 2u : 0u))
            {
                return 1;
            }
            printf(
                "SB_NATIVE trial=%u policy=%u cpu0=%u cpu1=%u completed=4 quanta=40 checksum=%llu migrations=%llu initial_imbalance=%u balance_ns=%llu elapsed_ns=%llu first_sweep_ns=%llu repeat_sweep_ns=%llu\n",
                trial, policy, physical[0], physical[1],
                (unsigned long long)sum, (unsigned long long)migrations,
                imbalance, (unsigned long long)balance_ns,
                (unsigned long long)elapsed, (unsigned long long)first,
                (unsigned long long)repeat);
        }
    }
    puts("SB_NATIVE_PASS:equal-work-pinned-and-movable");
    return 0;
}

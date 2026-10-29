#define _GNU_SOURCE
#include "partition.h"
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef RP_NATIVE_MUTANT
#define RP_NATIVE_MUTANT 0
#endif
static void failure(unsigned line)
{
    fprintf(stderr, "RP_NATIVE_CHECK:%u\n", line);
    exit(1);
}

#define CHECK(x)               \
    do                         \
    {                          \
        if (!(x))              \
            failure(__LINE__); \
    } while (0)

struct tq_shared
{
    pthread_barrier_t barrier;
    pthread_mutex_t mutex;
    struct rp_lease lease;
    struct rp_plan plan;
    struct rp_task tasks[4];
    int cpus[2];
    uint64_t payload;
};

struct tq_worker
{
    struct tq_shared *shared;
    unsigned id;
    uint64_t count, hash, wall_ns, cpu_ns;
    unsigned samples;
    int observed;
};

static uint64_t ns(clockid_t clock)
{
    struct timespec t;
    CHECK(clock_gettime(clock, &t) == 0);
    return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}

static void synchronize(struct tq_shared *s)
{
    int r = pthread_barrier_wait(&s->barrier);
    CHECK(r == 0 || r == PTHREAD_BARRIER_SERIAL_THREAD);
}

static uint64_t work(uint64_t v, unsigned n)
{
    for (unsigned i = 0; i < n; i++)
        v = v * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return v;
}

static void *worker(void *argument)
{
    struct tq_worker *w = argument;
    struct tq_shared *s = w->shared;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(s->cpus[w->id], &set);
    CHECK(sched_setaffinity(0, sizeof(set), &set) == 0);
    CPU_ZERO(&set);
    CHECK(sched_getaffinity(0, sizeof(set), &set) == 0 &&
          CPU_COUNT(&set) == 1 && CPU_ISSET(s->cpus[w->id], &set));
    synchronize(s);
    uint64_t wall = ns(CLOCK_MONOTONIC), cpu = ns(CLOCK_THREAD_CPUTIME_ID);
    w->hash = 1;
    for (unsigned repetition = 0; repetition < 64; repetition++)
    {
        synchronize(s);
        for (unsigned i = 0; i < 4; i++)
            if (s->plan.cpu[i] == w->id)
            {
                int observed = sched_getcpu();
                CHECK(observed == (RP_NATIVE_MUTANT == 1 ? s->cpus[1 - w->id]
                                                         : s->cpus[w->id]));
                w->observed = observed;
                w->samples++;
                unsigned units = s->tasks[i].cost[w->id];
                w->hash = work(w->hash, units * 1000u);
                w->count += units;
            }
        synchronize(s);
    }
    w->wall_ns = ns(CLOCK_MONOTONIC) - wall;
    w->cpu_ns = ns(CLOCK_THREAD_CPUTIME_ID) - cpu;
    /* One task payload migrates only after the old work interval is drained. */
    if (w->id == 0)
    {
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rp_begin(&s->lease, 0, 1) == RP_OK);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
        s->payload = work(1, 200);
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rp_finish(&s->lease, 0, 1, 200) == RP_OK);
        CHECK(rp_prepare(&s->lease, 0, 1, 1) == RP_OK);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
    }
    synchronize(s);
    if (w->id == 1)
    {
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rp_accept(&s->lease, 1, 1) == RP_OK && s->lease.epoch == 2);
        CHECK(rp_begin(&s->lease, 1, 2) == RP_OK);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
        CHECK(s->payload == work(1, 200));
        s->payload = work(s->payload, 200);
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rp_finish(&s->lease, 1, 2, 200) == RP_OK);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
    }
    synchronize(s);
    if (w->id == 0)
    {
        CHECK(pthread_mutex_lock(&s->mutex) == 0);
        CHECK(rp_begin(&s->lease, 0, 1) == RP_STALE);
        CHECK(pthread_mutex_unlock(&s->mutex) == 0);
    }
    return NULL;
}

int main(void)
{
    struct tq_shared s = {0};
    cpu_set_t allowed;
    CPU_ZERO(&allowed);
    if (sched_getaffinity(0, sizeof(allowed), &allowed) != 0)
    {
        puts("RP_NATIVE_ENV:affinity-mask-unavailable");
        return 77;
    }
    unsigned found = 0;
    for (unsigned c = 0; c < CPU_SETSIZE && found < 2; c++)
        if (CPU_ISSET(c, &allowed))
            s.cpus[found++] = (int)c;
    if (found < 2)
    {
        puts("RP_NATIVE_ENV:two-allowed-CPUs-required");
        return 77;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        unsigned cost = i < 2 ? 4 : 6;
        s.tasks[i] = (struct rp_task){
            .job = {.period = 10, .deadline = 10, .cost = cost},
            .cost = {cost, cost, 0, 0},
            .affinity = 3};
    }
    CHECK(rp_fit(s.tasks, 4, 2, 10, 0, &s.plan) == RP_OK &&
          rp_lease_init(&s.lease, 0) == RP_OK);
    CHECK(pthread_mutex_init(&s.mutex, NULL) == 0 &&
          pthread_barrier_init(&s.barrier, NULL, 2) == 0);
    struct tq_worker w[2] = {{.shared = &s, .id = 0}, {.shared = &s, .id = 1}};
    pthread_t threads[2];
    CHECK(pthread_create(&threads[0], NULL, worker, &w[0]) == 0 &&
          pthread_create(&threads[1], NULL, worker, &w[1]) == 0);
    CHECK(pthread_join(threads[0], NULL) == 0 &&
          pthread_join(threads[1], NULL) == 0);
    CHECK(w[0].count == 640 && w[1].count == 640 &&
          w[0].hash == work(1, 640000) && w[1].hash == w[0].hash &&
          w[0].samples == 128 && w[1].samples == 128);
    CHECK(s.payload == work(1, 400) && s.lease.work == 400 &&
          s.lease.owner == 1 && s.lease.epoch == 2 && !s.lease.busy &&
          !s.lease.prepared);
    for (unsigned i = 0; i < 2; i++)
        printf("RP_NATIVE_TRACE %u %d %d %llu %u %llu %llu %llu\n", i,
               s.cpus[i], w[i].observed, (unsigned long long)w[i].count,
               w[i].samples, (unsigned long long)w[i].hash,
               (unsigned long long)w[i].wall_ns,
               (unsigned long long)w[i].cpu_ns);
    printf("RP_NATIVE_TRANSFER %u %llu %llu %llu\n", s.lease.owner,
           (unsigned long long)s.lease.epoch, (unsigned long long)s.lease.work,
           (unsigned long long)s.payload);
    CHECK(pthread_barrier_destroy(&s.barrier) == 0 &&
          pthread_mutex_destroy(&s.mutex) == 0);
    puts("RP_NATIVE_PASS:two-pinned-workers-and-one-drained-transfer");
    return 0;
}

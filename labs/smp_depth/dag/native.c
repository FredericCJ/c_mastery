#define _GNU_SOURCE
#include "dag.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define CHECK(e)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(e))                                                      \
        {                                                              \
            printf("DG_NATIVE_CHECK:%u:%s\n", (unsigned)__LINE__, #e); \
            return 1;                                                  \
        }                                                              \
    } while (0)
struct sa_context;

struct sa_argument
{
    struct sa_context *context;
    unsigned index;
};

struct sa_context
{
    struct sa_dg graph;
    struct dg_spec spec;
    pthread_t threads[4];
    struct sa_argument args[4];
    unsigned workers, variant;
    int cpus[4], actual[16];
    atomic_uint gate, bad, diamond, visits[16];
    uint64_t start[16], before[16], after[16], value[16], init_before,
        init_after;
    unsigned owner[16];
};

static uint64_t now(clockid_t clock)
{
    struct timespec t;
    if (clock_gettime(clock, &t))
        abort();
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static uint64_t mix(uint64_t offset, uint64_t count)
{
    uint64_t total = 0;
    for (uint64_t k = 0; k < count; k++)
    {
        uint64_t x = offset + k + 1;
        x ^= x >> 17;
        x *= UINT64_C(6364136223846793005);
        x ^= x >> 31;
        total += x & 1023u;
    }
    return total;
}

static struct dg_spec specification(unsigned variant)
{
    struct dg_spec s = {0};
    s.count = variant == 2 ? 5 : 4;
    s.costs[0] = 2;
    s.costs[1] = 3;
    s.costs[2] = variant == 2 ? 2 : 5;
    s.costs[3] = variant == 2 ? 3 : 1;
    if (variant == 2)
        s.costs[4] = 1;
    s.edges[s.edge_count++] = (struct dg_edge){0, 1};
    s.edges[s.edge_count++] = (struct dg_edge){0, 2};
    if (variant == 2)
    {
        s.edges[s.edge_count++] = (struct dg_edge){0, 3};
        for (unsigned i = 1; i <= 3; i++)
            s.edges[s.edge_count++] = (struct dg_edge){i, 4};
    }
    else
    {
        s.edges[s.edge_count++] = (struct dg_edge){1, 3};
        s.edges[s.edge_count++] = (struct dg_edge){2, 3};
        if (variant == 1)
            s.edges[s.edge_count++] = (struct dg_edge){1, 2};
    }
    return s;
}

static void *worker(void *pointer)
{
    struct sa_argument *arg = pointer;
    struct sa_context *c = arg->context;
    unsigned index = arg->index;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(c->cpus[index], &set);
    if (pthread_setaffinity_np(pthread_self(), sizeof set, &set) ||
        sched_getcpu() != c->cpus[index])
        atomic_store(&c->bad, 1);
    while (!atomic_load_explicit(&c->gate, memory_order_acquire))
        sched_yield();
    while (dg_drained(&c->graph) != DG_OK)
    {
        struct dg_claim claim;
        int status = dg_claim(&c->graph, index, &claim);
        if (status == DG_EMPTY)
        {
            sched_yield();
            continue;
        }
        if (status != DG_OK)
        {
            atomic_store(&c->bad, 1);
            return NULL;
        }
        unsigned n = claim.node;
        if (atomic_fetch_add(&c->visits[n], 1))
        {
            atomic_store(&c->bad, 1);
            return NULL;
        }
        c->start[n] = now(CLOCK_MONOTONIC);
        c->actual[n] = sched_getcpu();
        c->owner[n] = index;
        uint64_t result;
        if (n == 0)
        {
            volatile uint64_t observed = mix(100000, claim.cost * 10000);
            (void)observed;
            result = 0;
        }
        else if (n == 1)
            result = mix(50000, 30000);
        else if (n == 2)
            result = mix(0, claim.cost * 10000);
        else if (c->variant == 2 && n == 3)
            result = mix(20000, 30000);
        else
        {
            volatile uint64_t observed = mix(200000, claim.cost * 10000);
            (void)observed;
            result = claim.input;
        }
        c->value[n] = result;
        if (c->variant == 0 && c->workers > 1 && (n == 1 || n == 2))
        {
            atomic_fetch_add_explicit(&c->diamond, 1, memory_order_release);
            while (atomic_load_explicit(&c->diamond, memory_order_acquire) != 2)
                sched_yield();
        }
        c->before[n] = now(CLOCK_MONOTONIC);
        if (dg_finish(&c->graph, claim, DG_DONE, result) != DG_OK)
            atomic_store(&c->bad, 1);
        c->after[n] = now(CLOCK_MONOTONIC);
    }
    return NULL;
}

static int experiment(unsigned workers, unsigned variant, unsigned trial)
{
    struct sa_context c;
    memset(&c, 0, sizeof c);
    c.workers = workers;
    c.variant = variant;
    c.spec = specification(variant);
    atomic_init(&c.gate, 0);
    atomic_init(&c.bad, 0);
    atomic_init(&c.diamond, 0);
    for (unsigned i = 0; i < 16; i++)
        atomic_init(&c.visits[i], 0);
    cpu_set_t allowed;
    CHECK(!sched_getaffinity(0, sizeof allowed, &allowed));
    unsigned found = 0;
    for (int cpu = 0; cpu < CPU_SETSIZE && found < 4; cpu++)
        if (CPU_ISSET(cpu, &allowed))
            c.cpus[found++] = cpu;
    CHECK(found >= workers);
    uint64_t begin = now(CLOCK_MONOTONIC), cpu = now(CLOCK_PROCESS_CPUTIME_ID);
    c.init_before = now(CLOCK_MONOTONIC);
    CHECK(dg_init(&c.graph, 1, &c.spec) == DG_OK);
    c.init_after = now(CLOCK_MONOTONIC);
    CHECK(dg_seal(&c.graph) == DG_OK);
    for (unsigned i = 0; i < workers; i++)
    {
        c.args[i] = (struct sa_argument){&c, i};
        CHECK(!pthread_create(&c.threads[i], NULL, worker, &c.args[i]));
    }
    atomic_store_explicit(&c.gate, 1, memory_order_release);
    for (unsigned i = 0; i < workers; i++)
        CHECK(!pthread_join(c.threads[i], NULL));
    cpu = now(CLOCK_PROCESS_CPUTIME_ID) - cpu;
    uint64_t wall = now(CLOCK_MONOTONIC) - begin;
    struct dg_snapshot snapshot;
    CHECK(!atomic_load(&c.bad) && dg_snapshot(&c.graph, &snapshot) == DG_OK &&
          snapshot.terminal == c.spec.count &&
          snapshot.results[c.spec.count - 1] == mix(0, 80000));
    printf(
        "DG_NATIVE_SAMPLE:{\"workers\":%u,\"variant\":%u,\"trial\":%u,\"nodes\":%u,\"work\":%llu,\"span\":%llu,\"completed\":%u,\"result\":%llu,\"wall_ns\":%llu,\"process_cpu_ns\":%llu,\"diamond_waiters\":%u,\"ledger\":[",
        workers, variant, trial, c.spec.count,
        (unsigned long long)snapshot.work, (unsigned long long)snapshot.span,
        snapshot.terminal,
        (unsigned long long)snapshot.results[c.spec.count - 1],
        (unsigned long long)wall, (unsigned long long)cpu,
        atomic_load(&c.diamond));
    for (unsigned i = 0; i < c.spec.count; i++)
    {
        uint64_t lower = 0, upper = 0;
        unsigned predecessors = 0;
        for (unsigned j = 0; j < c.spec.edge_count; j++)
            if (c.spec.edges[j].to == i)
            {
                unsigned p = c.spec.edges[j].from;
                predecessors++;
                if (c.before[p] > lower)
                    lower = c.before[p];
                if (c.after[p] > upper)
                    upper = c.after[p];
            }
        if (!predecessors)
        {
            lower = c.init_before;
            upper = c.init_after;
        }
        CHECK(atomic_load(&c.visits[i]) == 1 && snapshot.ready_events[i] == 1 &&
              c.start[i] >= lower && c.before[i] >= c.start[i] &&
              c.after[i] >= c.before[i] && c.actual[i] == c.cpus[c.owner[i]]);
        printf(
            "%s{\"node\":%u,\"visits\":%u,\"ready_events\":%u,\"ready_lower_ns\":%llu,\"ready_upper_ns\":%llu,\"start_ns\":%llu,\"finish_before_ns\":%llu,\"finish_after_ns\":%llu,\"worker\":%u,\"cpu\":%d,\"cost\":%llu,\"result\":%llu}",
            i ? "," : "", i, atomic_load(&c.visits[i]),
            snapshot.ready_events[i], (unsigned long long)(lower - begin),
            (unsigned long long)(upper - begin),
            (unsigned long long)(c.start[i] - begin),
            (unsigned long long)(c.before[i] - begin),
            (unsigned long long)(c.after[i] - begin), c.owner[i], c.actual[i],
            (unsigned long long)c.spec.costs[i],
            (unsigned long long)c.value[i]);
    }
    puts("]}");
    return 0;
}

struct sa_runtime
{
    struct sa_dg graph;
    atomic_uint claimed, release, bad;
    unsigned fail;
};

static void *runtime_worker(void *pointer)
{
    struct sa_runtime *r = pointer;
    struct dg_claim claim;
    if (dg_claim(&r->graph, 0, &claim) != DG_OK)
    {
        atomic_store(&r->bad, 1);
        return NULL;
    }
    atomic_store_explicit(&r->claimed, 1, memory_order_release);
    while (!atomic_load_explicit(&r->release, memory_order_acquire))
        sched_yield();
    if (dg_finish(&r->graph, claim, r->fail ? DG_FAILED : DG_DONE, 7) != DG_OK)
        atomic_store(&r->bad, 1);
    return NULL;
}

static int runtime(void)
{
    for (unsigned mode = 0; mode < 4; mode++)
    {
        struct sa_runtime r;
        memset(&r, 0, sizeof r);
        atomic_init(&r.claimed, 0);
        atomic_init(&r.release, 0);
        atomic_init(&r.bad, 0);
        r.fail = mode == 2;
        struct dg_spec s = {0};
        s.count = 1;
        s.costs[0] = 1;
        s.payloads[0] = 1;
        CHECK(dg_init(&r.graph, mode + 1, &s) == DG_OK);
        pthread_t thread;
        CHECK(!pthread_create(&thread, NULL, runtime_worker, &r));
        while (!atomic_load_explicit(&r.claimed, memory_order_acquire))
            sched_yield();
        unsigned parent = 0, node;
        struct dg_snapshot snapshot;
        if (mode == 1)
        {
            atomic_store_explicit(&r.release, 1, memory_order_release);
            CHECK(!pthread_join(thread, NULL));
        }
        CHECK(dg_append(&r.graph, 2, 1, &parent, 1, &node) == DG_OK &&
              node == 1 && dg_seal(&r.graph) == DG_OK);
        if (mode == 3)
        {
            CHECK(dg_cancel(&r.graph, 0) == DG_REQUESTED &&
                  dg_drained(&r.graph) == DG_BUSY &&
                  dg_snapshot(&r.graph, &snapshot) == DG_OK &&
                  snapshot.states[0] == DG_RUNNING);
        }
        if (mode != 1)
        {
            atomic_store_explicit(&r.release, 1, memory_order_release);
            CHECK(!pthread_join(thread, NULL));
        }
        CHECK(!atomic_load(&r.bad));
        struct dg_claim claim;
        if (mode < 2)
            CHECK(dg_claim(&r.graph, 1, &claim) == DG_OK && claim.node == 1 &&
                  claim.input == 9 &&
                  dg_finish(&r.graph, claim, DG_DONE, claim.input) == DG_OK);
        else
            CHECK(dg_claim(&r.graph, 1, &claim) == DG_EMPTY);
        CHECK(dg_snapshot(&r.graph, &snapshot) == DG_OK &&
              snapshot.terminal == 2 && dg_drained(&r.graph) == DG_OK);
        if (mode >= 2)
            CHECK(snapshot.states[1] == DG_SKIPPED && snapshot.results[1] == 0);
    }
    puts("DG_NATIVE_RUNTIME:before=1 after=1 skipped=1 retained-cancel=1");
    return 0;
}

int main(void)
{
    if (runtime())
        return 1;
    for (unsigned trial = 0; trial < 3; trial++)
        for (unsigned workers = 1; workers <= 4; workers *= 2)
            for (unsigned variant = 0; variant < 3; variant++)
                if (experiment(workers, variant, trial))
                    return 1;
    puts(
        "DG_NATIVE_PASS:actual-last-predecessors-runtime-admission-and-equivalent-split-work");
    return 0;
}

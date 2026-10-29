#include "cmk2_abi.h"
#include "cmk2_parallel.h"
#define JOBS 257u
#define NODES 16u
static unsigned run_mode, rounds, cursor, phase, done, errors, mask,
    node_state[NODES], node_value[NODES];
static uint64_t partial[4];
static unsigned counts[4];

static uint64_t stamp(void)
{
    unsigned a, d;
    __asm__ volatile("lfence;rdtsc" : "=a"(a), "=d"(d)::"memory");
    return (uint64_t)d << 32 | a;
}

static void yield(void)
{
    cmk_call(CMK_YIELD, 0, 0, 0);
}

static void run_jobs(unsigned id)
{
    for (;;)
    {
        unsigned j = __atomic_fetch_add(&cursor, 1, __ATOMIC_RELAXED);
        if (j >= JOBS)
        {
            break;
        }
        uint64_t n = j + 1;
        partial[id] += n * n;
        counts[id]++;
    }
}

static void run_dag(void)
{
    for (;;)
    {
        unsigned complete = 0, progress = 0;
        for (unsigned i = 0; i < NODES; i++)
        {
            unsigned state = __atomic_load_n(&node_state[i], __ATOMIC_ACQUIRE);
            if (state == 2)
            {
                complete++;
                continue;
            }
            if (state)
            {
                continue;
            }
            if (i >= 4 &&
                (__atomic_load_n(&node_state[i - 4], __ATOMIC_ACQUIRE) != 2 ||
                 __atomic_load_n(&node_state[i - 3], __ATOMIC_ACQUIRE) != 2))
            {
                continue;
            }
            unsigned expected = 0;
            if (!__atomic_compare_exchange_n(&node_state[i], &expected, 1, 0,
                                             __ATOMIC_ACQUIRE,
                                             __ATOMIC_RELAXED))
            {
                continue;
            }
            node_value[i] =
                i + 1 + (i >= 4 ? node_value[i - 4] + node_value[i - 3] : 0);
            __atomic_store_n(&node_state[i], 2, __ATOMIC_RELEASE);
            progress = 1;
        }
        if (complete == NODES)
        {
            break;
        }
        if (!progress)
        {
            yield();
        }
    }
}

static void pin(unsigned id)
{
    if (cmk_call(CMK2_SET_AFFINITY, 1u << id, 0, 0))
    {
        __atomic_fetch_add(&errors, 1, __ATOMIC_RELAXED);
    }
    if ((unsigned)cmk_call(CMK2_CPU_INFO, 0, 0, 0) != id)
    {
        __atomic_fetch_add(&errors, 1, __ATOMIC_RELAXED);
    }
    __atomic_fetch_or(&mask, 1u << id, __ATOMIC_RELAXED);
}

static void worker(uintptr_t argument)
{
    unsigned id = (unsigned)argument;
    pin(id);
    for (unsigned round = 1; round <= rounds; round++)
    {
        while (__atomic_load_n(&phase, __ATOMIC_ACQUIRE) != round)
        {
            yield();
        }
        run_jobs(id);
        __atomic_fetch_add(&done, 1, __ATOMIC_RELEASE);
    }
    if (run_mode == 3)
    {
        while (__atomic_load_n(&phase, __ATOMIC_ACQUIRE) != 3)
        {
            yield();
        }
        run_dag();
    }
    cmk2_thread_exit(0);
}

int cmk2_parallel_run(struct cmk2_parallel_result *r, unsigned mode)
{
    if (mode < 1 || mode > 3)
    {
        return -1;
    }
    run_mode = mode;
    rounds = mode == 1 ? 1 : 2;
    unsigned workers =
        (unsigned)((uint64_t)cmk_call(CMK2_CPU_INFO, 0, 0, 0) >> 32);
    if (!r || !workers || workers > 4)
    {
        return -1;
    }
    uint32_t handles[4];
    phase = done = cursor = errors = mask = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        partial[i] = 0;
        counts[i] = 0;
    }
    for (unsigned i = 0; i < NODES; i++)
    {
        node_state[i] = 0;
        node_value[i] = 0;
    }
    uint64_t begin = stamp();
    for (unsigned i = 0; i + 1 < workers; i++)
    {
        void *s = cmk_alloc(4);
        if (!s)
        {
            return -1;
        }
        int64_t h = cmk2_thread_create(worker, (unsigned char *)s + 16384, i);
        if (h < 0)
        {
            return -1;
        }
        handles[i] = (uint32_t)h;
    }
    pin(workers - 1);
    for (unsigned round = 1; round <= rounds; round++)
    {
        cursor = 0;
        __atomic_store_n(&phase, round, __ATOMIC_RELEASE);
        run_jobs(workers - 1);
        while (__atomic_load_n(&done, __ATOMIC_ACQUIRE) < round * (workers - 1))
        {
            yield();
        }
    }
    __atomic_store_n(&phase, 3, __ATOMIC_RELEASE);
    if (mode == 3)
    {
        run_dag();
    }
    for (unsigned i = 0; i + 1 < workers; i++)
    {
        if (cmk2_thread_join(handles[i]))
        {
            return -1;
        }
    }
    if (cmk_call(CMK2_SET_AFFINITY, (1u << workers) - 1, 0, 0))
    {
        return -1;
    }
    r->sum = 0;
    r->jobs = 0;
    r->cycles = stamp() - begin;
    r->mask = mask;
    r->errors = errors;
    for (unsigned i = 0; i < workers; i++)
    {
        r->sum += partial[i];
        r->jobs += counts[i];
    }
    unsigned oracle[NODES];
    for (unsigned i = 0; i < NODES; i++)
    {
        oracle[i] = i + 1 + (i >= 4 ? oracle[i - 4] + oracle[i - 3] : 0);
        if (mode == 3 && node_value[i] != oracle[i])
        {
            r->errors++;
        }
    }
    r->dag_value = node_value[NODES - 1];
    return 0;
}

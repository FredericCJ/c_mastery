#include "cmk2_abi.h"
#include "pool.h"
#ifndef SPG_MUTANT
#define SPG_MUTANT 0
#endif
static struct sp_launch launch;
static struct sp_pool pool;
static unsigned char *arena[4];
static unsigned leased, created, joined, released, allocate_fail, create_fail,
    join_fail, cpus, mode;
static atomic_uint bad, mask, visits[12];
static uint64_t results[12];

static void message(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    cmk_write(s, n);
}

static void number(uint64_t v)
{
    char text[24];
    unsigned n = 0;
    do
    {
        text[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (n)
        cmk_write(&text[--n], 1);
}

static void stop(unsigned status)
{
    cmk_call(CMK_EXIT, status, 0, 0);
    for (;;)
    {
    }
}

#define G_CHECK(e)                      \
    do                                  \
    {                                   \
        if (!(e))                       \
        {                               \
            message("SP_GUEST_CHECK:"); \
            number(__LINE__);           \
            message(":" #e "\n");       \
            stop(91);                   \
        }                               \
    } while (0)

static void pause_worker(void)
{
    cmk_call(CMK_YIELD, 0, 0, 0);
}

static uint64_t work(uint64_t id, uint64_t cost)
{
    uint64_t v = id + 1;
    for (uint64_t i = 0; i < cost; i++)
        v = v * UINT64_C(6364136223846793005) + 1;
    return v;
}

static void entry(uintptr_t argument)
{
    unsigned index = (unsigned)argument;
    unsigned cpu = SPG_MUTANT == 1 ? 0 : index % cpus;
    if (cmk_call(CMK2_SET_AFFINITY, 1u << cpu, 0, 0) ||
        (unsigned)cmk_call(CMK2_CPU_INFO, 0, 0, 0) != index % cpus)
        atomic_store(&bad, 1);
    atomic_fetch_or(&mask, 1u << (unsigned)cmk_call(CMK2_CPU_INFO, 0, 0, 0));
    unsigned gate;
    while (!(gate = atomic_load_explicit(&launch.gate, memory_order_acquire)))
        pause_worker();
    while (gate != 2)
    {
        if (mode)
        {
            struct sp_claim claim;
            int r = sp_claim(&pool, index, &claim);
            if (r == SP_OK)
            {
                if (claim.payload >= 12 ||
                    atomic_fetch_add(&visits[claim.payload], 1))
                    atomic_store(&bad, 1);
                else
                {
                    uint64_t result = work(claim.payload, claim.cost);
                    results[claim.payload] = result;
                    if (sp_complete(&pool, claim, result) != SP_OK)
                        atomic_store(&bad, 1);
                }
            }
            else if (r != SP_EMPTY)
                atomic_store(&bad, 1);
        }
        pause_worker();
        gate = atomic_load_explicit(&launch.gate, memory_order_acquire);
    }
    cmk2_thread_exit(0);
}

static void *allocate(void *unused, unsigned index)
{
    (void)unused;
    if (index == allocate_fail)
        return NULL;
    G_CHECK(!(leased & (1u << index)));
    leased |= 1u << index;
    return arena[index];
}

static int create(void *unused, unsigned index, void *stack, uintptr_t *handle)
{
    (void)unused;
    if (index == create_fail)
        return 1;
    int64_t h =
        cmk2_thread_create(entry, (unsigned char *)stack + 16384, index);
    if (h < 0)
        return 1;
    created |= 1u << index;
    *handle = (uintptr_t)h;
    return 0;
}

static int join(void *unused, uintptr_t handle)
{
    (void)unused;
    unsigned index = 4;
    for (unsigned i = 0; i < launch.count; i++)
        if ((launch.started & (1u << i)) && launch.handles[i] == handle)
            index = i;
    G_CHECK(index < 4);
    if (index == join_fail)
        return 1;
    int64_t r = cmk2_thread_join((uint32_t)handle);
    if (!r)
        joined |= 1u << index;
    return r != 0;
}

static void release(void *unused, unsigned index, void *stack)
{
    (void)unused;
    G_CHECK(stack == arena[index] && (leased & (1u << index)) &&
            (!(created & (1u << index)) || (joined & (1u << index))));
    leased &= ~(1u << index);
    released++;
}

static void reset(unsigned selected)
{
    G_CHECK(!leased);
    created = joined = released = 0;
    allocate_fail = create_fail = join_fail = 4;
    mode = selected;
    atomic_init(&bad, 0);
    atomic_init(&mask, 0);
    for (unsigned i = 0; i < 12; i++)
    {
        atomic_init(&visits[i], 0);
        results[i] = 0;
    }
    G_CHECK(sp_launch_init(&launch,
                           (struct sp_launch_port){allocate, create, join,
                                                   release, NULL}) == SP_OK);
}

void _start(void)
{
    cpus = (unsigned)((uint64_t)cmk_call(CMK2_CPU_INFO, 0, 0, 0) >> 32);
    G_CHECK(cpus == 1 || cpus == 2 || cpus == 4);
    for (unsigned i = 0; i < 4; i++)
    {
        arena[i] = cmk_alloc(4);
        G_CHECK(arena[i]);
    }
    unsigned cuts = 0;
    for (unsigned n = 0; n <= 4; n++)
    {
        reset(0);
        int r = sp_launch(&launch, n);
        G_CHECK(r == (n == 4 ? SP_FAILED : SP_OK));
        if (n < 4)
            G_CHECK(sp_shutdown(&launch) == SP_OK);
        if (atomic_load(&bad))
        {
            message("SP_GUEST_CHECK:worker-placement\n");
            stop(91);
        }
        G_CHECK(!leased && released == n);
        cuts++;
        for (unsigned cut = 0; cut < n; cut++)
            for (unsigned kind = 0; kind < 2; kind++)
            {
                reset(0);
                if (kind)
                    create_fail = cut;
                else
                    allocate_fail = cut;
                G_CHECK(sp_launch(&launch, n) == SP_FAILED && !leased &&
                        !atomic_load(&bad) && released == cut + kind);
                cuts++;
            }
    }
    reset(0);
    G_CHECK(sp_launch(&launch, 3) == SP_OK);
    join_fail = 1;
    G_CHECK(sp_shutdown(&launch) == SP_DEGRADED && leased == 2 &&
            launch.started == 2 && released == 2);
    join_fail = 4;
    G_CHECK(sp_shutdown(&launch) == SP_OK && !leased && released == 3 &&
            !atomic_load(&bad));
    unsigned workers = cpus < 4 ? cpus : 3;
    reset(1);
    G_CHECK(sp_init(&pool, 31, workers, 0) == SP_OK &&
            sp_launch(&launch, workers) == SP_OK);
    struct sp_id ids[12];
    for (unsigned i = 0; i < 12; i++)
    {
        int r;
        do
        {
            r = sp_submit(&pool, 0, i, i == 0 ? 50000 : 1000, &ids[i]);
            if (r == SP_FULL)
                pause_worker();
        } while (r == SP_FULL);
        G_CHECK(r == SP_OK);
    }
    G_CHECK(sp_close(&pool, 0) == SP_OK);
    struct sp_snapshot snapshot;
    do
    {
        G_CHECK(sp_snapshot(&pool, &snapshot) == SP_OK);
        pause_worker();
    } while (snapshot.completed != 12);
    G_CHECK(sp_shutdown(&launch) == SP_OK && !leased && !atomic_load(&bad) &&
            atomic_load(&mask) == (1u << workers) - 1);
    for (unsigned i = 0; i < 12; i++)
    {
        unsigned state;
        uint64_t result;
        G_CHECK(atomic_load(&visits[i]) == 1 &&
                sp_result(&pool, ids[i], &state, &result) == SP_OK &&
                state == SP_COMPLETED && result == results[i] &&
                result == work(i, i == 0 ? 50000 : 1000));
        G_CHECK(sp_collect(&pool, ids[i]) == SP_OK);
    }
    message("SP_GUEST_RESULT:cpus=");
    number(cpus);
    message(" workers=");
    number(workers);
    message(" startup_cuts=");
    number(cuts);
    message(" jobs=12 mask=");
    number(atomic_load(&mask));
    message(" arena_slots=4 degraded_retained=1 retry_joined=1\n");
    message(
        "SP_GUEST_PASS:actual-thread-create-join-owned-stack-arena-and-capacity3-jobs\n");
    stop(0);
}

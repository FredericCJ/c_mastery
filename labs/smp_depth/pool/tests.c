#include "pool.h"
#include <stdio.h>
#include <string.h>
#define CHECK(e)                                                \
    do                                                          \
    {                                                           \
        if (!(e))                                               \
        {                                                       \
            printf("SP_CHECK:%u:%s\n", (unsigned)__LINE__, #e); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int trace(void)
{
    struct sp_pool p;
    struct sp_id a, b, c, d, unchanged = {15, 91, 82};
    struct sp_claim x;
    struct sp_snapshot s;
    CHECK(sp_init(&p, 1, 2, 0) == SP_OK);
    CHECK(sp_submit(&p, 0, 10, 1, &a) == SP_OK &&
          sp_submit(&p, 1, 20, 1, &b) == SP_OK);
    CHECK(sp_claim(&p, 0, &x) == SP_OK && x.payload == 10);
    CHECK(sp_submit(&p, 0, 30, 1, &c) == SP_OK &&
          sp_submit(&p, 0, 40, 1, &d) == SP_OK);
    CHECK(sp_submit(&p, 0, 50, 1, &unchanged) == SP_FULL &&
          unchanged.world == 91);
    CHECK(sp_snapshot(&p, &s) == SP_OK && s.accepted == 4 && s.queued == 3 &&
          s.running == 1 && s.completed == 0);
    unsigned state = 99;
    uint64_t value = 98;
    CHECK(sp_result(&p, a, &state, &value) == SP_BUSY && state == 99 &&
          value == 98);
    CHECK(sp_complete(&p, x, 100) == SP_OK &&
          sp_complete(&p, x, 101) == SP_STALE);
    CHECK(sp_claim(&p, 1, &x) == SP_OK && x.payload == 20);
    CHECK(sp_complete(&p, x, 200) == SP_OK && sp_close(&p, 0) == SP_OK);
    CHECK(sp_submit(&p, 0, 60, 1, &unchanged) == SP_CLOSED &&
          unchanged.world == 91);
    CHECK(sp_claim(&p, 0, &x) == SP_OK && x.payload == 30 &&
          sp_complete(&p, x, 300) == SP_OK);
    CHECK(sp_claim(&p, 0, &x) == SP_OK && x.payload == 40 &&
          sp_complete(&p, x, 400) == SP_OK);
    CHECK(sp_claim(&p, 0, &x) == SP_EMPTY && sp_snapshot(&p, &s) == SP_OK);
    CHECK(s.completed == 4 && s.cancelled == 0 && s.queued == 0 &&
          s.running == 0 && s.terminal == 4);
    CHECK(sp_result(&p, a, &state, &value) == SP_OK && state == SP_COMPLETED &&
          value == 100);
    CHECK(sp_collect(&p, a) == SP_OK && sp_collect(&p, b) == SP_OK &&
          sp_collect(&p, c) == SP_OK && sp_collect(&p, d) == SP_OK);
    CHECK(sp_result(&p, a, &state, &value) == SP_STALE);
    return 0;
}

static int cancellation(void)
{
    struct sp_pool p;
    struct sp_id a, b, c;
    struct sp_claim x;
    struct sp_snapshot s;
    unsigned state = 99;
    uint64_t value = 98;
    CHECK(sp_init(&p, 2, 1, 0) == SP_OK &&
          sp_submit(&p, 0, 1, 1, &a) == SP_OK &&
          sp_submit(&p, 0, 2, 1, &b) == SP_OK);
    CHECK(sp_claim(&p, 0, &x) == SP_OK && sp_cancel(&p, a) == SP_REQUESTED);
    CHECK(sp_collect(&p, a) == SP_BUSY &&
          sp_result(&p, a, &state, &value) == SP_BUSY && state == 99 &&
          value == 98);
    CHECK(sp_cancel(&p, b) == SP_OK && sp_snapshot(&p, &s) == SP_OK &&
          s.cancelled == 1 && s.running == 1);
    CHECK(sp_complete(&p, x, 123) == SP_OK &&
          sp_result(&p, a, &state, &value) == SP_OK && state == SP_CANCELLED &&
          value == 0);
    CHECK(sp_collect(&p, a) == SP_OK && sp_submit(&p, 0, 3, 1, &c) == SP_OK &&
          c.slot == a.slot && c.generation != a.generation);
    CHECK(sp_cancel(&p, a) == SP_STALE && sp_close(&p, 1) == SP_OK);
    CHECK(sp_snapshot(&p, &s) == SP_OK && s.accepted == 3 && s.cancelled == 3 &&
          s.running == 0 && s.queued == 0);
    CHECK(sp_collect(&p, b) == SP_OK && sp_collect(&p, c) == SP_OK);
    return 0;
}

static int finite(void)
{
    struct sp_pool p;
    struct sp_id id = {0}, old;
    struct sp_claim claim;
    struct sp_snapshot s;
    CHECK(sp_init(&p, 3, 0, 0) == SP_OK &&
          sp_submit(&p, 0, 1, 1, &id) == SP_NO_WORKERS);
    CHECK(sp_init(&p, 4, 1, 0) == SP_OK);
    /* White-box finite-limit setup under exclusive ownership, never an API operation. */
    for (unsigned i = 0; i < SP_JOBS; i++)
    {
        p.jobs[i].generation = UINT64_MAX;
    }
    p.jobs[0].generation = UINT64_MAX - 1;
    p.accepted = UINT64_MAX - 1;
    p.completed = UINT64_MAX - 1;
    CHECK(sp_submit(&p, 0, 1, 1, &id) == SP_OK && id.generation == UINT64_MAX);
    CHECK(sp_claim(&p, 0, &claim) == SP_OK &&
          sp_complete(&p, claim, 1) == SP_OK && sp_collect(&p, id) == SP_OK);
    CHECK(sp_submit(&p, 0, 1, 1, &old) == SP_FULL && sp_close(&p, 0) == SP_OK);
    CHECK(sp_snapshot(&p, &s) == SP_OK && s.accepted == UINT64_MAX &&
          s.completed == UINT64_MAX);
    CHECK(sp_init(NULL, 1, 1, 0) == SP_INVALID &&
          sp_init(&p, 0, 1, 0) == SP_INVALID &&
          sp_init(&p, 1, 5, 0) == SP_INVALID);
    return 0;
}

static int nesting(void)
{
    struct sp_pool p;
    struct sp_id parent, children[3], out = {0, 99, 99};
    struct sp_claim root, child;
    struct sp_snapshot s;
    CHECK(sp_init(&p, 5, 1, 0) == SP_OK &&
          sp_submit(&p, 0, 7, 1, &parent) == SP_OK &&
          sp_claim(&p, 0, &root) == SP_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(sp_submit(&p, 0, 10 + i, 1, &children[i]) == SP_OK);
    }
    CHECK(sp_submit(&p, 0, 13, 1, &out) == SP_FULL && out.world == 99);
    /* A bounded helper executes outside the lock while its parent remains RUNNING. */
    CHECK(sp_claim(&p, 0, &child) == SP_OK && child.payload == 10 &&
          sp_complete(&p, child, 100) == SP_OK);
    CHECK(sp_submit(&p, 0, 13, 1, &out) == SP_OK &&
          sp_complete(&p, root, 7) == SP_OK);
    CHECK(sp_close(&p, 0) == SP_OK);
    while (sp_claim(&p, 0, &child) == SP_OK)
    {
        CHECK(sp_complete(&p, child, child.payload) == SP_OK);
    }
    CHECK(sp_snapshot(&p, &s) == SP_OK && s.accepted == 5 && s.completed == 5 &&
          s.running == 0 && s.queued == 0);
    CHECK(sp_init(&p, 6, 3, 1) == SP_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(sp_submit(&p, 0, 10 + i, 1, &children[i]) == SP_OK);
    }
    CHECK(sp_claim(&p, 1, &child) == SP_OK && child.payload == 10 &&
          sp_complete(&p, child, 10) == SP_OK);
    CHECK(sp_claim(&p, 0, &child) == SP_OK && child.payload == 12 &&
          sp_complete(&p, child, 12) == SP_OK);
    CHECK(sp_claim(&p, 2, &child) == SP_OK && child.payload == 11 &&
          sp_complete(&p, child, 11) == SP_OK);
    CHECK(sp_snapshot(&p, &s) == SP_OK && s.completed == 3);
    return 0;
}

struct sa_mock
{
    struct sp_launch launch;
    unsigned alloc_fail, create_fail, join_fail, allocated, alive, released,
        bad;
    unsigned char stacks[4];
};

static void *allocate(void *context, unsigned index)
{
    struct sa_mock *m = context;
    if (index == m->alloc_fail)
    {
        return NULL;
    }
    m->allocated |= 1u << index;
    return &m->stacks[index];
}

static int create(void *context, unsigned index, void *stack, uintptr_t *handle)
{
    struct sa_mock *m = context;
    if (stack != &m->stacks[index])
    {
        m->bad = 1;
    }
    if (index == m->create_fail)
    {
        return 1;
    }
    m->alive |= 1u << index;
    *handle = index + 1;
    return 0;
}

static int join(void *context, uintptr_t handle)
{
    struct sa_mock *m = context;
    unsigned i = (unsigned)handle - 1;
    if (i >= 4)
    {
        m->bad = 1;
        return 1;
    }
    if (atomic_load_explicit(&m->launch.gate, memory_order_acquire) != 2)
    {
        m->bad = 1;
    }
    if (i == m->join_fail)
    {
        return 1;
    }
    m->alive &= ~(1u << i);
    return 0;
}

static void release(void *context, unsigned index, void *stack)
{
    struct sa_mock *m = context;
    if (stack != &m->stacks[index] || (m->alive & (1u << index)) ||
        !(m->allocated & (1u << index)))
    {
        m->bad = 1;
    }
    m->allocated &= ~(1u << index);
    m->released++;
}

static int reset(struct sa_mock *m)
{
    memset(m, 0, sizeof *m);
    m->alloc_fail = m->create_fail = m->join_fail = 4;
    return sp_launch_init(&m->launch, (struct sp_launch_port){
                                          allocate, create, join, release, m});
}

static int launching(void)
{
    struct sa_mock m;
    for (unsigned n = 0; n <= 4; n++)
    {
        CHECK(reset(&m) == SP_OK && sp_launch(&m.launch, n) == SP_OK &&
              atomic_load(&m.launch.gate) == 1);
        CHECK(m.alive == (1u << n) - 1 && sp_shutdown(&m.launch) == SP_OK &&
              m.alive == 0 && m.allocated == 0 && !m.bad && m.released == n);
        CHECK(sp_shutdown(&m.launch) == SP_OK && m.released == n);
        for (unsigned cut = 0; cut < n; cut++)
        {
            for (unsigned kind = 0; kind < 2; kind++)
            {
                CHECK(reset(&m) == SP_OK);
                if (kind)
                {
                    m.create_fail = cut;
                }
                else
                {
                    m.alloc_fail = cut;
                }
                CHECK(sp_launch(&m.launch, n) == SP_FAILED &&
                      m.launch.phase == 3 && m.alive == 0 && m.allocated == 0 &&
                      !m.bad);
                CHECK(m.released == cut + kind);
            }
        }
    }
    CHECK(reset(&m) == SP_OK && sp_launch(&m.launch, 4) == SP_OK);
    m.join_fail = 1;
    CHECK(sp_shutdown(&m.launch) == SP_DEGRADED && m.launch.phase == 2 &&
          !m.bad && m.alive == 2 && m.allocated == 2 && m.released == 3);
    CHECK(sp_launch(&m.launch, 2) == SP_BUSY);
    m.join_fail = 4;
    CHECK(sp_shutdown(&m.launch) == SP_OK && !m.bad && m.allocated == 0 &&
          m.alive == 0 && m.released == 4);
    CHECK(reset(&m) == SP_OK);
    m.create_fail = 3;
    m.join_fail = 1;
    CHECK(sp_launch(&m.launch, 4) == SP_DEGRADED && !m.bad &&
          m.allocated == 2 && m.alive == 2);
    m.join_fail = 4;
    CHECK(sp_shutdown(&m.launch) == SP_OK && !m.bad && m.allocated == 0 &&
          m.alive == 0);
    return 0;
}

int main(void)
{
    if (trace() || cancellation() || finite() || nesting() || launching())
    {
        return 1;
    }
    puts(
        "SP_PASS:states-generation-nesting-deques-partial-launch-and-degraded-retention");
    return 0;
}

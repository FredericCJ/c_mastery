#include "list.h"
#include "cmx_port.h"
static struct sx_list domain;
static struct sx_participant participants[2];
static atomic_uint phase, ready, released, done;
static _Alignas(8) uint32_t stacks[4][512];
#define CHECK(x)                 \
    do                           \
    {                            \
        if (!(x))                \
            cmx_port_finish(80); \
    } while (0)
#define CALL(expr)                \
    do                            \
    {                             \
        int rc;                   \
        do                        \
        {                         \
            rc = (expr);          \
            if (rc == MB_BUSY)    \
                cmx_port_yield(); \
        } while (rc == MB_BUSY);  \
        CHECK(rc == MB_OK);       \
    } while (0)

static void idle(void)
{
    for (;;)
        __asm__ volatile("wfi");
}

static void reader(unsigned id)
{
    for (unsigned round = 1; round <= 20; round++)
    {
        struct sx_observed observed;
        struct sx_protected reference;
        unsigned value;
        while (atomic_load_explicit(&phase, memory_order_acquire) < round)
            cmx_port_yield();
        CALL(sx_enter(&domain, participants[id]));
        CALL(sx_observe(&domain, participants[id], 0, &observed));
        CALL(sx_protect(&domain, participants[id], 0, observed, &reference));
        atomic_fetch_or_explicit(&ready, 1u << id, memory_order_release);
        while (atomic_load_explicit(&released, memory_order_acquire) < round)
            cmx_port_yield();
        CALL(sx_read(&domain, participants[id], reference, &value));
        CHECK(value == 11);
        CALL(sx_leave(&domain, participants[id]));
        atomic_fetch_or_explicit(&done, 1u << id, memory_order_release);
    }
    for (;;)
        cmx_port_yield();
}

static void reader0(void)
{
    reader(0);
}

static void reader1(void)
{
    reader(1);
}

static void writer(void)
{
    for (unsigned round = 1; round <= 20; round++)
    {
        unsigned mode = (round - 1) % 2, value, count;
        CHECK(sx_list_init(&domain, mode) == MB_OK);
        for (unsigned id = 0; id < 2; id++)
            CHECK(sx_register(&domain, &participants[id]) == MB_OK);
        for (unsigned i = 1; i <= 3; i++)
            CHECK(sx_append(&domain, 11 * i) == MB_OK);
        atomic_store(&ready, 0);
        atomic_store(&done, 0);
        atomic_store_explicit(&phase, round, memory_order_release);
        while (atomic_load_explicit(&ready, memory_order_acquire) != 3)
            cmx_port_yield();
        for (unsigned i = 1; i <= 3; i++)
        {
            CALL(sx_remove(&domain, &value));
            CHECK(value == 11 * i);
        }
        if (mode == SX_EPOCH)
        {
            CHECK(sx_advance(&domain) == MB_OK &&
                  sx_advance(&domain) == MB_BUSY);
        }
        CALL(sx_collect(&domain, &count));
        CHECK(count == (mode == SX_HAZARD ? 2u : 0u));
        CHECK(sx_depart(&domain, participants[0]) == MB_BUSY);
        atomic_store_explicit(&released, round, memory_order_release);
        while (atomic_load_explicit(&done, memory_order_acquire) != 3)
            cmx_port_yield();
        if (mode == SX_EPOCH)
            CHECK(sx_advance(&domain) == MB_OK);
        CALL(sx_collect(&domain, &count));
        CHECK(count == (mode == SX_HAZARD ? 1u : 3u));
        for (unsigned id = 0; id < 2; id++)
            CHECK(sx_depart(&domain, participants[id]) == MB_OK);
    }
    while (cmx_port_ticks < 2)
        cmx_port_yield();
    CHECK(cmx_port_switches >= 100);
    cmx_port_finish(0);
}

void cmx_workload_main(void)
{
    atomic_init(&phase, 0);
    atomic_init(&ready, 0);
    atomic_init(&released, 0);
    atomic_init(&done, 0);
    cmx_port_init();
    CHECK(cmx_port_task(reader0, stacks[0], 512, 4) >= 0 &&
          cmx_port_task(reader1, stacks[1], 512, 4) >= 0 &&
          cmx_port_task(writer, stacks[2], 512, 4) >= 0 &&
          cmx_port_task(idle, stacks[3], 512, 255) >= 0);
    cmx_port_start();
}

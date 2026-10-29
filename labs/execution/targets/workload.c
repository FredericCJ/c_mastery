#include "cmx_port.h"
static _Alignas(8) uint32_t stacks[3][512];
static cmx_sem permits;
static cmx_queue messages;
static volatile uint32_t heartbeat[2], finished, received, checksum;
static volatile uint32_t initialized_word = 0x13579bdfu;
static volatile uint32_t initial_zero[8];
static volatile uint32_t handler_checks;

void cmx_port_tick_hook(void)
{
    uint32_t v;
    if (cmx_port_take(&permits, cmx_forever()) != CMX_INVALID ||
        cmx_port_send(&messages, 0, cmx_forever()) != CMX_INVALID ||
        cmx_port_recv(&messages, &v, cmx_forever()) != CMX_INVALID)
    {
        cmx_port_finish(55);
    }
    handler_checks++;
}

static void idle(void)
{
    for (;;)
    {
        cmx_amp_service();
#if CMX_STAGE == 1
        cmx_port_yield();
#else
        __asm__ volatile("wfi");
#endif
    }
}

static void exercise(unsigned id, uint32_t fp)
{
    uint32_t outer = cmx_port_lock(), inner = cmx_port_lock(), observed;
    if (outer != 0 || inner != 1)
    {
        cmx_port_finish(52);
    }
    uint32_t ignored;
    if (cmx_port_take(&permits, cmx_forever()) != CMX_INVALID ||
        cmx_port_send(&messages, 0, cmx_forever()) != CMX_INVALID ||
        cmx_port_recv(&messages, &ignored, cmx_forever()) != CMX_INVALID)
    {
        cmx_port_finish(56);
    }
    cmx_port_unlock(inner);
    __asm__ volatile("mrs %0,primask" : "=r"(observed));
    if (observed != 1)
    {
        cmx_port_finish(53);
    }
    cmx_port_unlock(outer);
    __asm__ volatile("mrs %0,primask" : "=r"(observed));
    if (observed != 0)
    {
        cmx_port_finish(54);
    }
#if CMX_STAGE >= 3
    cmx_fp_set(fp);
#else
    (void)fp;
#endif
#if CMX_STAGE == 1
    for (unsigned n = 0; n < 20; n++)
    {
        heartbeat[id]++;
        cmx_port_yield();
    }
#else
    while (cmx_port_ticks < 8)
    {
        heartbeat[id]++;
#if CMX_STAGE >= 3
        if (cmx_fp_get() != fp)
        {
            cmx_port_finish(31 + id);
        }
#endif
    }
#endif
    uint32_t saved = cmx_port_lock();
    finished |= 1u << id;
    cmx_port_unlock(saved);
}

static void producer(void)
{
    exercise(0, 0x42290000u);
#if CMX_STAGE >= 4
    if (cmx_port_give(&permits) != CMX_OK)
    {
        cmx_port_finish(33);
    }
    for (uint32_t n = 0; n < 256; n++)
    {
        if (cmx_port_send(&messages, n, cmx_at(cmx_port_ticks + 500)) != CMX_OK)
        {
            cmx_port_finish(37);
        }
    }
    cmx_port_close(&messages);
#endif
    while (finished != 3)
    {
        cmx_port_yield();
    }
}

static void consumer(void)
{
    exercise(1, 0xc1580000u);
#if CMX_STAGE >= 4
    if (cmx_port_take(&permits, cmx_at(cmx_port_ticks + 500)) != CMX_OK)
    {
        cmx_port_finish(35);
    }
    for (;;)
    {
        uint32_t value;
        enum cmx_result r =
            cmx_port_recv(&messages, &value, cmx_at(cmx_port_ticks + 500));
        if (r == CMX_CLOSED)
        {
            break;
        }
        if (r != CMX_OK || value != received)
        {
            cmx_port_finish(38);
        }
        checksum += value;
        received++;
    }
    if (received != 256 || checksum != 32640)
    {
        cmx_port_finish(39);
    }
#endif
    while (finished != 3)
    {
        cmx_port_yield();
    }
    if (cmx_port_switches < 5 || !heartbeat[0] || !heartbeat[1])
    {
        cmx_port_finish(36);
    }
#if CMX_STAGE >= 2
    if (!handler_checks)
    {
        cmx_port_finish(57);
    }
#endif
#ifdef CMX_H745
    for (;;)
    {
        cmx_amp_service();
        cmx_port_yield();
    }
#else
    cmx_port_finish(0);
#endif
}

void cmx_workload_main(void)
{
#if CMX_STAGE == 0
    if (initialized_word != 0x13579bdfu)
    {
        cmx_port_finish(50);
    }
    for (unsigned i = 0; i < 8; i++)
    {
        if (initial_zero[i])
        {
            cmx_port_finish(51);
        }
    }
    cmx_port_finish(0);
#else
    cmx_port_init();
    cmx_sem_init(&permits, 0, 1);
    cmx_queue_init(&messages, 8);
    if (cmx_port_task(producer, stacks[0], 512, 4) < 0 ||
        cmx_port_task(consumer, stacks[1], 512, 4) < 0 ||
        cmx_port_task(idle, stacks[2], 512, 255) < 0)
    {
        cmx_port_finish(30);
    }
    cmx_port_start();
#endif
}

#include "cmx_port.h"
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#ifdef CMX_H745
/* ST H745: RCC 0x58024400; HSEM 0x58026400. Original direct-register port.
 * NUCLEO-H745ZI-Q contract: global reset; CM4 BOOT_ADD0=0x08100000 and
 * BCM4 disabled so CM7 initializes shared memory before releasing CM4. */
#define SHARED_ADDRESS UINT32_C(0x30040000)
#define MAGIC UINT32_C(0x434d5833)

typedef struct
{
    uint32_t epoch, sequence, value;
} packet;

typedef struct
{
    volatile uint32_t head, tail;
    packet item[16];
} ring;

typedef struct
{
    volatile uint32_t magic, epoch, ack, progress[2], fault;
    ring to4, to7;
} shared;

static shared *const bus = (shared *)(uintptr_t)SHARED_ADDRESS;
volatile uint32_t cmx_amp_irq_count, cmx_amp_received, cmx_amp_sent,
    cmx_amp_epoch;
#ifdef CMX_CORE_M4
#define CORE_ID 1u
#define OWN 1u
#define OUT_SEM 1u
#define IN_SEM 0u
#define IRQ_NUMBER 126u
#define HSEM_COMMON UINT32_C(0x58026510)
#else
#define CORE_ID 3u
#define OWN 0u
#define OUT_SEM 0u
#define IN_SEM 1u
#define IRQ_NUMBER 125u
#define HSEM_COMMON UINT32_C(0x58026500)
#endif
static void barrier(void)
{
    __asm__ volatile("dmb sy" ::: "memory");
}

static void doorbell(void)
{ /* Nonblocking one-step take, process ID zero. */
    uint32_t got = REG(0x58026480u + 4u * OUT_SEM);
    if (got == (0x80000000u | (CORE_ID << 8)))
    {
        barrier();
        REG(0x58026400u + 4u * OUT_SEM) = CORE_ID << 8;
    } /* ring remains authoritative if coalesced/busy */
}

static int put(ring *r, packet p)
{
    uint32_t h = r->head, t = r->tail;
    barrier();
    if ((uint32_t)(h - t) >= 16)
    {
        return 0;
    }
    r->item[h & 15u] = p;
    barrier();
    r->head = h + 1;
    doorbell();
    return 1;
}

static int get(ring *r, packet *p)
{
    uint32_t t = r->tail, h = r->head;
    barrier();
    if (t == h)
    {
        return 0;
    }
    *p = r->item[t & 15u];
    barrier();
    r->tail = t + 1;
    return 1;
}

void HSEM_Handler(void)
{
    uint32_t pending = REG(HSEM_COMMON + 12u);
    REG(HSEM_COMMON + 4u) = pending;
    barrier();
    cmx_amp_irq_count++; /* No blocking, parsing, or cross-core spin. */
}

void cmx_board_init(void)
{
    REG(0x580244e0) |= 1u << 25;
    REG(0x580244dc) |= 0xe0000000u;
    (void)REG(0x580244dc);
    barrier();
#ifndef CMX_CORE_M4
    /* Region 7: 32 KiB normal, shareable, noncacheable, XN, full data access.
     * Cold-reset caches are disabled; no dirty cache line may predate this map.
     */
    REG(0xe000ed94) = 0;
    REG(0xe000ed98) = 7;
    REG(0xe000ed9c) = SHARED_ADDRESS;
    REG(0xe000eda0) =
        (1u << 28) | (3u << 24) | (1u << 19) | (1u << 18) | (14u << 1) | 1u;
    REG(0xe000ed94) = 5;
    __asm__ volatile("dsb\nisb" ::: "memory");
    bus->magic = 0;
    bus->epoch = 1;
    bus->ack = 0;
    bus->fault = 0;
    bus->progress[0] = 0;
    bus->progress[1] = 0;
    bus->to4.head = 0;
    bus->to4.tail = 0;
    bus->to7.head = 0;
    bus->to7.tail = 0;
    barrier();
    bus->magic = MAGIC;
    barrier();
    REG(0x580244a0) |= 8u;
#else
    unsigned spins;
    for (spins = 0; spins < 10000000u; spins++)
    {
        if (bus->magic == MAGIC && bus->epoch == 1)
        {
            break;
        }
    }
    if (spins == 10000000u)
    {
        cmx_port_finish(41);
    }
    barrier();
    bus->ack = bus->epoch;
    barrier();
#endif
    cmx_amp_epoch = bus->epoch;
    REG(HSEM_COMMON + 4u) = 1u << IN_SEM;
    REG(HSEM_COMMON) = 1u << IN_SEM;
    *(volatile unsigned char *)(uintptr_t)(0xe000e400u + IRQ_NUMBER) = 0xa0u;
    REG(0xe000e100u + 4u * (IRQ_NUMBER / 32u)) = 1u << (IRQ_NUMBER % 32u);
}

void cmx_amp_service(void)
{
    uint32_t saved = cmx_port_lock();
    if (bus->magic != MAGIC || bus->epoch != cmx_amp_epoch)
    {
        bus->fault = 42;
        cmx_port_finish(42);
    }
    if (bus->ack != cmx_amp_epoch)
    {
        cmx_port_unlock(saved);
        return;
    }
    packet p;
#ifdef CMX_CORE_M4
    if (get(&bus->to4, &p))
    {
        if (p.epoch != cmx_amp_epoch || p.sequence != cmx_amp_received)
        {
            bus->fault = 43;
            cmx_port_finish(43);
        }
        cmx_amp_received++;
        p.value += 1;
        if (!put(&bus->to7, p))
        {
            bus->fault = 44;
            cmx_port_finish(44);
        }
        cmx_amp_sent++;
    }
#else
    if (cmx_amp_sent < 1000u && cmx_amp_sent - cmx_amp_received < 8u)
    {
        p = (packet){cmx_amp_epoch, cmx_amp_sent, cmx_amp_sent * 3u};
        if (put(&bus->to4, p))
        {
            cmx_amp_sent++;
        }
    }
    if (get(&bus->to7, &p))
    {
        if (p.epoch != cmx_amp_epoch || p.sequence != cmx_amp_received ||
            p.value != cmx_amp_received * 3u + 1u)
        {
            bus->fault = 45;
            cmx_port_finish(45);
        }
        cmx_amp_received++;
    }
#endif
    bus->progress[OWN] = cmx_amp_received;
    if (cmx_amp_received == 1000u)
    {
        cmx_port_status = 0x600d0000u;
    }
    cmx_port_unlock(saved);
}
#else
void cmx_board_init(void)
{
}

void cmx_amp_service(void)
{
}

void HSEM_Handler(void)
{
    cmx_port_finish(40);
}
#endif

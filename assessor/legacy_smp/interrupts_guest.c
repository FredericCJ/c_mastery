/* Explicitly appended to the completed startup's private copied translation unit. */
#include "interrupts.h"
#include "interrupts.c"
#ifndef SI_MODE
#define SI_MODE 0
#endif
#ifndef SI_GUEST_MUTANT
#define SI_GUEST_MUTANT 0
#endif
extern void si_timer_irq(void), si_ipi_irq(void), si_spurious_irq(void);
static struct si_box si_boxes[CMK2_MAX_CPUS];

static struct
{
    unsigned ready, command, seen, held, stopped, finished, phase, blocked,
        features, arat;
    unsigned one, periodic, ipis, eois, work, done;
    uint64_t sum, handler_ticks, work_ticks;
} si_cpu[CMK2_MAX_CPUS];

static unsigned si_go, si_finish, si_release_held;

static void si_fail(const char *reason)
{
    text("SI_GUEST_CHECK:");
    text(reason);
    text("\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
        __asm__ volatile("cli;hlt");
}

static uint64_t si_msr(unsigned address)
{
    unsigned a, d;
    __asm__ volatile("rdmsr" : "=a"(a), "=d"(d) : "c"(address));
    return (uint64_t)d << 32 | a;
}

static void si_write_msr(unsigned address, uint64_t value)
{
    __asm__ volatile("wrmsr" ::"c"(address), "a"((unsigned)value),
                     "d"((unsigned)(value >> 32))
                     : "memory");
}

static uint32_t si_read(unsigned offset)
{
    return SI_MODE ? (uint32_t)si_msr(0x800 + offset / 16) : lapic[offset / 4];
}

static void si_write(unsigned offset, uint32_t value)
{
    if (SI_MODE)
        si_write_msr(0x800 + offset / 16, value);
    else
        lapic[offset / 4] = value;
}

static unsigned si_index(void)
{
    unsigned id = SI_MODE ? si_read(0x20) : si_read(0x20) >> 24;
    for (unsigned i = 0; i < topology.count; i++)
        if (topology.ids[i] == id)
            return i;
    si_fail("identity");
    return 0;
}

static uint64_t si_ticks(void)
{
    unsigned a, d;
    __asm__ volatile("lfence;rdtsc" : "=a"(a), "=d"(d)::"memory");
    return (uint64_t)d << 32 | a;
}

static void si_send(unsigned index)
{
    /* ISO C publication and device notification are separate ordering obligations. */
    __asm__ volatile("mfence" ::: "memory");
    if (SI_MODE)
    {
        si_write_msr(0x830, (uint64_t)topology.ids[index] << 32 | 0x4000 | 241);
        return;
    }
    for (unsigned spin = 0; si_read(0x300) & 0x1000; spin++)
        if (spin == 10000000)
            si_fail("ICR-busy");
    si_write(0x310, (unsigned)topology.ids[index] << 24);
    si_write(0x300, 0x4000 | 241);
    for (unsigned spin = 0; si_read(0x300) & 0x1000; spin++)
        if (spin == 10000000)
            si_fail("ICR-send-pending");
}

static void si_consume(void *context, uint32_t item)
{
    unsigned index = (unsigned)(uintptr_t)context;
    struct si_box *box = &si_boxes[index];
    if (atomic_load_explicit(&box->acknowledged, memory_order_acquire) ==
        atomic_load_explicit(&box->published, memory_order_relaxed))
        si_fail("ack-before-work");
    si_cpu[index].sum += item;
    si_cpu[index].work++;
}

void si_interrupt(uint64_t *frame)
{
    unsigned index = si_index(), vector = (unsigned)frame[15];
    uint64_t begin = si_ticks(), work_begin = 0, work_end = 0;
    if (vector == 240)
    {
        unsigned phase =
            __atomic_load_n(&si_cpu[index].phase, __ATOMIC_RELAXED);
        if (phase == 1)
            __atomic_fetch_add(&si_cpu[index].one, 1, __ATOMIC_RELAXED);
        else if (phase == 2)
        {
            unsigned count = __atomic_add_fetch(&si_cpu[index].periodic, 1,
                                                __ATOMIC_RELAXED);
            if (count >= 3)
            {
                si_write(0x380, 0);
                si_write(0x320, 0x10000 | 240);
            }
        }
        else
            si_fail("timer-phase");
    }
    else if (vector == 241)
    {
        __atomic_fetch_add(&si_cpu[index].ipis, 1, __ATOMIC_RELAXED);
        uint64_t generation = atomic_load_explicit(&si_boxes[index].published,
                                                   memory_order_acquire);
        work_begin = si_ticks();
        int result = si_drain(&si_boxes[index], generation, si_consume,
                              (void *)(uintptr_t)index);
        work_end = si_ticks();
        if (result != SI_OK && result != SI_DUPLICATE)
            si_fail("mailbox-result");
    }
    else if (vector == 255)
        return;
    else
        si_fail("unexpected-vector");
    if (!(SI_GUEST_MUTANT == 2 && vector == 241 && index == topology.count - 1))
    {
        si_write(0xb0, 0);
        __atomic_fetch_add(&si_cpu[index].eois, 1, __ATOMIC_RELAXED);
    }
    si_cpu[index].handler_ticks += si_ticks() - begin;
    if (work_begin)
        si_cpu[index].work_ticks += work_end - work_begin;
    __atomic_add_fetch(&si_cpu[index].done, 1, __ATOMIC_RELEASE);
}

static void si_local(unsigned index)
{
    unsigned a, b, c, d;
    __asm__ volatile("cpuid"
                     : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                     : "a"(1), "c"(0));
    si_cpu[index].features = c;
    if (!(d & (1u << 9)) || !(d & (1u << 26)))
        si_fail("required-APIC-SSE2-feature");
    if (SI_MODE)
    {
        if (!(c & (1u << 21)))
            si_fail("x2APIC-feature-unavailable");
        si_write_msr(0x1b, si_msr(0x1b) | 0xc00);
        if ((si_msr(0x1b) & 0xc00) != 0xc00)
            si_fail("x2APIC-mode-transition");
    }
    if (si_index() != index)
        si_fail("mode-identity");
    __asm__ volatile("cpuid"
                     : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                     : "a"(0), "c"(0));
    si_cpu[index].arat = 2;
    if (a >= 6)
    {
        __asm__ volatile("cpuid"
                         : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                         : "a"(6), "c"(0));
        si_cpu[index].arat = (a >> 2) & 1;
    }
    void (*handlers[])(void) = {si_timer_irq, si_ipi_irq, si_spurious_irq};
    unsigned vectors[] = {240, 241, 255};
    for (unsigned j = 0; j < 3; j++)
    {
        uint64_t address = (uintptr_t)handlers[j];
        cpus[index].idt[vectors[j]] = (struct gate){
            address & 65535,           8, 1, 0x8e, (address >> 16) & 65535,
            (uint32_t)(address >> 32), 0};
    }
    si_write(0x80, 0);
    si_write(0xf0, 0x100 | 255);
    si_write(0x320, 0x10000 | 240);
    si_write(0x380, 0);
    si_write(0x3e0, 3);
    __atomic_store_n(&si_cpu[index].ready, 1, __ATOMIC_RELEASE);
    __asm__ volatile("sti" ::: "memory");
}

static void si_step(unsigned index)
{
    if (SI_GUEST_MUTANT == 2 && index == topology.count - 1 &&
        atomic_load_explicit(&si_boxes[index].published,
                             memory_order_acquire) == 2 &&
        atomic_load_explicit(&si_boxes[index].acknowledged,
                             memory_order_acquire) == 1 &&
        (si_read(0x170) & (1u << 17)) && (si_read(0x270) & (1u << 17)))
        __atomic_store_n(&si_cpu[index].blocked, 1, __ATOMIC_RELEASE);
    unsigned command =
        __atomic_load_n(&si_cpu[index].command, __ATOMIC_ACQUIRE);
    if (command == si_cpu[index].seen)
        return;
    si_cpu[index].seen = command;
    if (command == 1 || command == 2)
    {
        __asm__ volatile("cli" ::: "memory");
        si_write(0x380, 0);
        __atomic_store_n(&si_cpu[index].phase, command, __ATOMIC_RELAXED);
        si_write(0x320, 240 | (command == 2 ? 0x20000 : 0));
        si_write(0x380, 1000000);
        __asm__ volatile("sti" ::: "memory");
    }
    else if (command == 3)
    {
        __asm__ volatile("cli" ::: "memory");
        si_write(0x380, 0);
        si_write(0x320, 0x10000 | 240);
        if (si_read(0x390) != 0)
            si_fail("timer-zero-stop");
        __atomic_store_n(&si_cpu[index].stopped, 1, __ATOMIC_RELEASE);
        __asm__ volatile("sti" ::: "memory");
    }
    else if (command == 4)
    {
        __asm__ volatile("cli" ::: "memory");
        __atomic_store_n(&si_cpu[index].held, 1, __ATOMIC_RELEASE);
        while (!__atomic_load_n(&si_release_held, __ATOMIC_ACQUIRE))
            pause_cpu();
        __asm__ volatile("sti" ::: "memory");
    }
}

static void si_worker(unsigned index)
{
    while (!__atomic_load_n(&si_go, __ATOMIC_ACQUIRE))
        pause_cpu();
    si_local(index);
    while (!__atomic_load_n(&si_finish, __ATOMIC_ACQUIRE))
    {
        si_step(index);
        pause_cpu();
    }
    __asm__ volatile("cli" ::: "memory");
    __atomic_store_n(&si_cpu[index].finished, 1, __ATOMIC_RELEASE);
    for (;;)
        __asm__ volatile("hlt");
}

static void si_wait_field(unsigned *field, unsigned minimum, const char *reason)
{
    for (unsigned spin = 0; __atomic_load_n(field, __ATOMIC_ACQUIRE) < minimum;
         spin++)
    {
        if (spin == 100000000)
            si_fail(reason);
        si_step(0);
        pause_cpu();
    }
}

static void si_wait_ack(unsigned index, uint64_t generation)
{
    for (unsigned spin = 0; si_status(&si_boxes[index], generation) != SI_OK;
         spin++)
    {
        if (spin == (SI_GUEST_MUTANT == 2 ? 1000000u : 100000000u))
        {
            if (SI_GUEST_MUTANT == 2 && index == topology.count - 1)
            {
                if (!__atomic_load_n(&si_cpu[index].blocked, __ATOMIC_ACQUIRE))
                    si_fail("missing-EOI-state-mismatch");
                if (si_timeout(&si_boxes[index], generation) != SI_TIMED_OUT)
                    si_fail("missing-EOI-no-outstanding");
                si_fail("missing-EOI-outstanding");
            }
            si_fail("application-ack-timeout");
        }
        si_step(0);
        pause_cpu();
    }
}

void si_bsp(void)
{
    for (unsigned i = 0; i < topology.count; i++)
        si_init(&si_boxes[i]);
    __atomic_store_n(&si_go, 1, __ATOMIC_RELEASE);
    si_local(0);
    for (unsigned i = 0; i < topology.count; i++)
        si_wait_field(&si_cpu[i].ready, 1, "CPU-ready");
    for (unsigned i = 0; i < topology.count; i++)
        __atomic_store_n(&si_cpu[i].command, 1, __ATOMIC_RELEASE);
    for (unsigned i = 0; i < topology.count; i++)
        si_wait_field(&si_cpu[i].one, 1, "one-shot");
    for (unsigned spin = 0; spin < 100000; spin++)
        pause_cpu();
    for (unsigned i = 0; i < topology.count; i++)
        if (__atomic_load_n(&si_cpu[i].one, __ATOMIC_ACQUIRE) != 1)
            si_fail("one-shot-reloaded");
    for (unsigned i = 0; i < topology.count; i++)
        __atomic_store_n(&si_cpu[i].command, 2, __ATOMIC_RELEASE);
    for (unsigned i = 0; i < topology.count; i++)
        si_wait_field(&si_cpu[i].periodic, 3, "periodic");
    for (unsigned i = 0; i < topology.count; i++)
        __atomic_store_n(&si_cpu[i].command, 3, __ATOMIC_RELEASE);
    for (unsigned i = 0; i < topology.count; i++)
        si_wait_field(&si_cpu[i].stopped, 1, "stop-observation");
    for (unsigned generation = 1; generation <= 7; generation++)
    {
        uint32_t items[4] = {generation, generation + 1, generation + 2,
                             generation + 3};
        for (unsigned i = 0; i < topology.count; i++)
        {
            uint64_t ticket = 0;
            if (si_publish(&si_boxes[i], items, generation == 7 ? 4 : 1,
                           &ticket) != SI_OK ||
                ticket != generation)
                si_fail("publish");
            si_send(i);
            if (generation == 7)
                si_send(i);
        }
        for (unsigned i = 0; i < topology.count; i++)
            si_wait_ack(i, generation);
    }
    unsigned delayed = topology.count > 1 ? topology.count - 1 : 0;
    if (delayed)
    {
        __atomic_store_n(&si_cpu[delayed].command, 4, __ATOMIC_RELEASE);
        si_wait_field(&si_cpu[delayed].held, 1, "hold-entry");
    }
    for (unsigned i = 0; i < topology.count; i++)
    {
        uint32_t item = 100 + i;
        uint64_t ticket;
        if (si_publish(&si_boxes[i], &item, 1, &ticket) != SI_OK || ticket != 8)
            si_fail("late-publish");
        si_send(i);
    }
    if (delayed)
    {
        for (unsigned i = 0; i < delayed; i++)
            si_wait_ack(i, 8);
        for (unsigned spin = 0; spin < 100000; spin++)
            pause_cpu();
        if (si_timeout(&si_boxes[delayed], 8) != SI_TIMED_OUT)
            si_fail("delay-not-retained");
        uint32_t item = 999;
        uint64_t untouched = 999;
        if (si_publish(&si_boxes[delayed], &item, 1, &untouched) != SI_BUSY ||
            untouched != 999 || si_boxes[delayed].items[0] != 100 + delayed)
            si_fail("timeout-reuse");
        text("SI_TIMEOUT_RETAIN cpu=");
        number(delayed);
        text(" generation=8 peers=");
        number(delayed);
        text("\n");
        __atomic_store_n(&si_release_held, 1, __ATOMIC_RELEASE);
    }
    for (unsigned i = 0; i < topology.count; i++)
        si_wait_ack(i, 8);
    __atomic_store_n(&si_finish, 1, __ATOMIC_RELEASE);
    for (unsigned i = 1; i < topology.count; i++)
        si_wait_field(&si_cpu[i].finished, 1, "finish");
    __asm__ volatile("cli" ::: "memory");
    for (unsigned i = 0; i < topology.count; i++)
    {
        if (si_cpu[i].work != 11 || si_cpu[i].sum != 155 + i ||
            si_cpu[i].one != 1 || si_cpu[i].periodic < 3 ||
            si_cpu[i].periodic > 4 ||
            si_cpu[i].eois !=
                si_cpu[i].one + si_cpu[i].periodic + si_cpu[i].ipis ||
            si_cpu[i].handler_ticks < si_cpu[i].work_ticks)
            si_fail("per-CPU-accounting");
        text("SI_CPU index=");
        number(i);
        text(" apic=");
        number(topology.ids[i]);
        text(" mode=");
        number(SI_MODE);
        text(" cpuid1c=");
        number(si_cpu[i].features);
        text(" arat=");
        number(si_cpu[i].arat);
        text(" one=");
        number(si_cpu[i].one);
        text(" periodic=");
        number(si_cpu[i].periodic);
        text(" ipis=");
        number(si_cpu[i].ipis);
        text(" eois=");
        number(si_cpu[i].eois);
        text(" work=");
        number(si_cpu[i].work);
        text(" sum=");
        number((unsigned)si_cpu[i].sum);
        text(" ack=8 timeout=");
        number(si_boxes[i].timed_out);
        text(" handler_ticks=");
        ss_u64(si_cpu[i].handler_ticks);
        text(" work_ticks=");
        ss_u64(si_cpu[i].work_ticks);
        text("\n");
    }
    text("SI_GUEST_PASS:timer-publication-work-ack-EOI\n");
    unsigned value = 16;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
        __asm__ volatile("cli;hlt");
}

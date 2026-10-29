/* Same task bodies in the dual H745 images and the single-core AN386
 * scheduler experiment. AN386 establishes actual exception/task mechanics,
 * never H745 parallelism, DMA, clock or noncoherent cache observations. */
#ifndef AF_BURST
#define AF_BURST 1
#endif
#ifndef AF_LOCAL
#define AF_LOCAL 0
#endif
#ifndef AF_INTERFERENCE
#define AF_INTERFERENCE 0
#endif
#ifndef AF_PIPE_MUTANT
#define AF_PIPE_MUTANT 0
#endif
_Static_assert(AF_BURST >= 1 && AF_BURST <= 4, "bounded ISR capture");
static _Alignas(8) uint32_t task_stacks[4][512];
static struct af_ring acquisition;
#if !AF_M7 || AF_QEMU
static _Atomic uint32_t pipe_epoch;
static _Atomic uint32_t pipe_limit = 256;
#endif
#if AF_M7
static _Atomic unsigned device_pause, device_parked;
#endif
static void high(unsigned i, uint32_t count)
{
    uint32_t old = atomic_load(&SH->high[i]);
    if (count > old)
    {
        atomic_store(&SH->high[i], count);
    }
}

void cmx_port_tick_hook(void)
{
    af_systick();
#if !AF_M7 || AF_QEMU
    uint32_t epoch = atomic_load_explicit(&pipe_epoch, memory_order_acquire);
    if (epoch &&
        atomic_load_explicit(&SH->boot.ready, memory_order_acquire) == epoch)
    {
        for (unsigned n = 0; n < AF_BURST; n++)
        {
            uint32_t seq = atomic_load(&SH->samples) + 1;
            if (seq > pipe_limit)
            {
                break;
            }
            atomic_store(&SH->samples, seq);
            struct af_message m = {
                .epoch = epoch, .sequence = seq, .length = 4};
            for (unsigned k = 0; k < 4; k++)
            {
                m.bytes[k] = (unsigned char)(seq >> (8 * k));
            }
            if (af_queued(&acquisition) >= 2 ||
                af_batch(&acquisition, epoch, &m, 1) != AF_OK)
            {
                SH->rejected[(seq - 1) / 32] |= 1u << ((seq - 1) % 32);
                atomic_fetch_add(&SH->dropped, 1);
            }
            else
            {
                high(0, af_queued(&acquisition));
            }
        }
    }
    if (AF_INTERFERENCE && cmx_port_ticks % 17 == 0)
    {
        for (volatile unsigned n = 0; n < 5000; n++)
        {
            __asm__ volatile("nop");
        }
    }
#endif
}

void cmx_board_init(void)
{
    machine();
}

static void idle_task(void)
{
    for (;;)
    {
        cmx_port_yield();
    }
}
#if !AF_M7 || AF_QEMU
static void producer_task(void)
{
    uint32_t epoch = 0, start = now();
    struct af_message pending = {0};
    unsigned have = 0;
    for (;;)
    {
        uint32_t ready =
            atomic_load_explicit(&SH->boot.ready, memory_order_acquire);
        if (!ready)
        {
            atomic_store(&pipe_epoch, 0);
            if (epoch)
            {
                atomic_store_explicit(&SH->boot.parked, epoch,
                                      memory_order_release);
            }
            unsigned expired;
            if (af_elapsed(start, now(), 100000000, &expired) != AF_OK ||
                expired)
            {
                fail(0xe0);
            }
            cmx_port_yield();
            continue;
        }
        if (ready != epoch)
        {
            if (have || af_queued(&acquisition))
            {
                fail(0xe1);
            }
            if (af_boot_ack(&SH->boot, 2, AF_LAYOUT, ready) != AF_OK)
            {
                fail(0xe2);
            }
            epoch = ready;
            pipe_limit = epoch == 1 ? 256 : 512;
            atomic_store(&pipe_epoch, epoch);
        }
        start = now();
        if (!have)
        {
            size_t len;
            uint32_t seq;
            int r =
                af_receive(&acquisition, epoch, pending.bytes, 32, &seq, &len);
            if (r == AF_OK)
            {
                pending.epoch = epoch;
                pending.sequence = seq;
                pending.length = (uint32_t)len;
                have = 1;
            }
            else if (r != AF_EMPTY)
            {
                fail(0xe3);
            }
        }
        if (have)
        {
            struct af_ring *out = AF_LOCAL ? &SH->response : &SH->request;
            unsigned cap = AF_LOCAL ? 8 : 4;
            if (af_queued(out) < cap)
            {
                if (AF_LOCAL)
                {
                    uint32_t value = pending.sequence + 1;
                    for (unsigned k = 0; k < 4; k++)
                    {
                        pending.bytes[k] = (unsigned char)(value >> (8 * k));
                    }
                }
                if (af_batch(out, epoch, &pending, 1) != AF_OK)
                {
                    fail(0xe4);
                }
                high(AF_LOCAL ? 2 : 1, af_queued(out));
                if (!AF_LOCAL)
                {
                    atomic_fetch_add(&SH->traffic, 1);
                }
                have = 0;
                ring();
            }
        }
        atomic_fetch_add(&SH->progress[1], 1);
        cmx_port_yield();
    }
}

static void sink_task(void)
{
    uint32_t last = 0;
    for (;;)
    {
        uint32_t epoch = atomic_load(&pipe_epoch);
        if (epoch && atomic_load(&SH->boot.ready) == epoch)
        {
            unsigned char bytes[32];
            size_t len;
            uint32_t seq;
            int r = af_receive(&SH->response, epoch, bytes, 32, &seq, &len);
            if (r == AF_OK)
            {
                uint32_t value = 0;
                for (unsigned k = 0; k < 4; k++)
                {
                    value |= (uint32_t)bytes[k] << (8 * k);
                }
                if (len != 4 || seq <= last || seq > 512 || value != seq + 1)
                {
                    fail(0xe5);
                }
                last = seq;
                if (AF_PIPE_MUTANT != 2)
                {
                    SH->completed[(seq - 1) / 32] |= 1u << ((seq - 1) % 32);
                }
                uint32_t done = atomic_fetch_add(&SH->done, 1);
                if (done < 128)
                {
                    SH->trace[done] = seq;
                }
            }
            else if (r != AF_EMPTY)
            {
                fail(0xe6);
            }
        }
        cmx_port_yield();
    }
}
#endif
#if AF_M7
#if AF_QEMU
static void print_word(uint32_t value)
{
    char text[10];
    for (unsigned i = 0; i < 8; i++)
    {
        unsigned d = (value >> (28 - 4 * i)) & 15;
        text[i] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
    }
    text[8] = '\n';
    text[9] = 0;
    register unsigned r0 __asm__("r0") = 4;
    register const void *r1 __asm__("r1") = text;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}
#endif
static void worker_task(void)
{
    uint32_t epoch = 1;
    unsigned booted = 0;
    uint32_t watchdog = now(), progress = 0;
    for (;;)
    {
        if (!booted)
        {
            int r = af_boot_status(&supervisor, &SH->boot, now());
            if (r == AF_OK)
            {
                booted = 1;
            }
            else if (r != AF_BUSY)
            {
                fail(0xe7);
            }
            else
            {
                cmx_port_yield();
                continue;
            }
        }
        if (!AF_LOCAL && af_queued(&SH->response) < 8)
        {
            unsigned char bytes[32];
            size_t len;
            uint32_t seq;
            int r = af_receive(&SH->request, epoch, bytes, 32, &seq, &len);
            if (r == AF_OK)
            {
                if (len != 4 || !seq || seq > 512)
                {
                    fail(0xe8);
                }
                struct af_message m = {
                    .epoch = epoch, .sequence = seq, .length = 4};
                uint32_t value = seq + (AF_PIPE_MUTANT == 1 ? 2 : 1);
                for (unsigned k = 0; k < 4; k++)
                {
                    m.bytes[k] = (unsigned char)(value >> (8 * k));
                }
                int32_t applied;
                if (af_effect(&receipt, 1, receipt.last_id + 1, 1, &applied) !=
                    AF_OK)
                {
                    fail(0xe9);
                }
                if (af_batch(&SH->response, epoch, &m, 1) != AF_OK)
                {
                    fail(0xea);
                }
                high(2, af_queued(&SH->response));
                ring();
            }
            else if (r != AF_EMPTY)
            {
                fail(0xeb);
            }
        }
        atomic_fetch_add(&SH->progress[0], 1);
        uint32_t seen = atomic_load(&SH->progress[1]);
        if (seen != progress)
        {
            progress = seen;
            watchdog = now();
        }
        else
        {
            unsigned expired;
            if (af_elapsed(watchdog, now(), 100000000, &expired) != AF_OK ||
                expired)
            {
                fail(0xec);
            }
        }
        uint32_t samples = atomic_load(&SH->samples),
                 done = atomic_load(&SH->done),
                 dropped = atomic_load(&SH->dropped);
        if (samples == (epoch == 1 ? 256u : 512u) && samples == done + dropped)
        {
            if (epoch == 2)
            {
                for (unsigned i = 0; i < 16; i++)
                {
                    if ((SH->rejected[i] & SH->completed[i]) ||
                        (SH->rejected[i] | SH->completed[i]) != UINT32_MAX)
                    {
                        fail(0xed);
                    }
                }
                if (atomic_load(&SH->high[0]) > 2 ||
                    atomic_load(&SH->high[1]) > 4 ||
                    atomic_load(&SH->high[2]) > 8 || cmx_port_switches < 20)
                {
                    fail(0xee);
                }
#if AF_QEMU
                print_word(samples);
                print_word(done);
                print_word(dropped);
                print_word(epoch);
                print_word(atomic_load(&SH->traffic));
                for (unsigned i = 0; i < 3; i++)
                {
                    print_word(atomic_load(&SH->high[i]));
                }
                print_word(cmx_port_switches);
                for (unsigned i = 0; i < 16; i++)
                {
                    print_word(SH->rejected[i]);
                    print_word(SH->completed[i]);
                }
                cmx_port_finish(0);
#else
                for (;;)
                {
                    cmx_port_yield();
                }
#endif
            }
            atomic_store_explicit(&SH->boot.ready, 0, memory_order_release);
            uint32_t start = now();
            while (atomic_load_explicit(&SH->boot.parked,
                                        memory_order_acquire) != epoch)
            {
                unsigned expired;
                if (af_elapsed(start, now(), 100000000, &expired) != AF_OK ||
                    expired)
                {
                    fail(0xef);
                }
                cmx_port_yield();
            }
#if !AF_QEMU
            atomic_store(&device_pause, 1);
            while (!atomic_load(&device_parked))
            {
                unsigned expired;
                if (af_elapsed(start, now(), 100000000, &expired) != AF_OK ||
                    expired)
                {
                    fail(0xf7);
                }
                cmx_port_yield();
            }
#endif
            if (af_boot_close(&supervisor, &SH->boot, 1, 1) != AF_OK)
            {
                fail(0xf1);
            }
            publish(epoch);
            epoch = supervisor.next_epoch;
            booted = 0;
            atomic_store(&device_parked, 0);
            atomic_store(&device_pause, 0);
            ring();
        }
        cmx_port_yield();
    }
}

static void device_task(void)
{
    for (;;)
    {
#if !AF_QEMU
        if (!atomic_load(&device_pause) || buffers.transfer.phase)
        {
            dma_step();
        }
        if (atomic_load(&device_pause) && !buffers.transfer.phase)
        {
            atomic_store(&device_parked, 1);
        }
#else
        (void)dma_step;
        (void)port;
        (void)buffers;
#endif
        cmx_port_yield();
    }
}

static void primary(void)
{
    memory_policy();
    *SH = (struct af_shared){0};
    publish(0);
    af_ring_init(&acquisition, 0);
    struct xd_fence cold = {0, 1, 1, 1};
    if (af_buffers_init(&buffers, &cold) != AF_OK)
    {
        fail(0xf2);
    }
    cmx_port_init();
#if AF_QEMU
    if (cmx_port_task(producer_task, task_stacks[0], 512, 4) < 0 ||
        cmx_port_task(sink_task, task_stacks[1], 512, 4) < 0 ||
        cmx_port_task(worker_task, task_stacks[2], 512, 4) < 0 ||
        cmx_port_task(idle_task, task_stacks[3], 512, 255) < 0)
    {
        fail(0xf3);
    }
    (void)device_task;
#else
    if (cmx_port_task(worker_task, task_stacks[0], 512, 4) < 0 ||
        cmx_port_task(device_task, task_stacks[1], 512, 4) < 0 ||
        cmx_port_task(idle_task, task_stacks[2], 512, 255) < 0)
    {
        fail(0xf4);
    }
    REG(0x580244a0u) |= 8;
    fence(0);
#endif
    cmx_port_start();
}
#else
static void secondary(void)
{
    af_ring_init(&acquisition, 0);
    cmx_port_init();
    uint32_t previous = atomic_load(&SH->boot.ready);
    if (previous && atomic_load(&SH->boot.ack) == previous)
    {
        fail(0xf5);
    }
    if (cmx_port_task(producer_task, task_stacks[0], 512, 4) < 0 ||
        cmx_port_task(sink_task, task_stacks[1], 512, 4) < 0 ||
        cmx_port_task(idle_task, task_stacks[2], 512, 255) < 0)
    {
        fail(0xf6);
    }
    cmx_port_start();
}
#endif

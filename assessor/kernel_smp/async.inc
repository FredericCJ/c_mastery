struct async_request
{
    unsigned state, owner, waiter, ticket, type, irq_before;
    uint64_t buffer, deadline;
    int result;
};
static struct async_request io_request;
static unsigned async_completions, async_wait_blocks;

static void async_finish(int result)
{
    tasks[io_request.owner].io_pending = 0;
    async_pending = 0;
    io_request.result = result;
    io_request.state = 2;
    async_completions++;
    if (io_request.waiter < THREADS &&
        uthreads[io_request.waiter].state == UT_IO)
    {
        uthreads[io_request.waiter].frame->rax = (uint64_t)(int64_t)result;
        uthreads[io_request.waiter].state = UT_READY;
        io_request.state = 0;
    }
    else if (!tasks[io_request.owner].alive)
        io_request.state = 0;
}

static void async_poll(void)
{
    if (!async_pending)
        return;
    barrier();
    if (__atomic_load_n(&disk_irq_sequence, __ATOMIC_ACQUIRE) ==
            io_request.irq_before ||
        vu.idx == used_idx)
    {
        if (ticks > io_request.deadline)
        {
            disk_ready = 0;
            common_cfg[20] |=
                128; /* DMA storage remains static and is not reused after poison. */
            async_finish(-1);
        }
        return;
    }
    unsigned slot = used_idx % 8;
    if (vu.ring[slot].id != 0 || vu.idx != (uint16_t)(used_idx + 1) ||
        request_status)
    {
        disk_ready = 0;
        async_finish(-1);
        return;
    }
    used_idx++;
    struct task *p = &tasks[io_request.owner];
    if (io_request.type == 0 && p->alive)
    {
        if (!user_ok(p, io_request.buffer, 4096, 1))
        {
            async_finish(-1);
            return;
        }
        for (unsigned j = 0; j < 4096; j++)
            *(unsigned char *)physical(p, io_request.buffer + j) = bounce[j];
    }
    async_finish(0);
}

static int64_t async_submit(uint64_t handle, uint64_t pointer)
{
    unsigned pid = uthreads[current].process;
    struct task *p = &tasks[pid];
    if (handle > UINT32_MAX ||
        !cmk_handle_valid((uint32_t)handle, 0, p->handle_generation,
                          p->handle_open))
        return -1;
    if (!disk_ready ||
        !user_ok(p, pointer, sizeof(struct cmk2_block_request), 0))
        return -1;
    if (io_request.state)
        return CMK2_EAGAIN;
    struct cmk2_block_request r;
    memcpy(&r, (const void *)(uintptr_t)pointer, sizeof r);
    if (r.type != 0 && r.type != 1 && r.type != 4)
        return -1;
    if (r.type != 4 &&
        (r.page >= disk_pages || !user_ok(p, r.buffer, 4096, r.type == 0)))
        return -1;
    if (io_request.ticket == UINT32_MAX)
        return -1;
    p->io_pending = 1;
    io_request.owner = pid;
    io_request.waiter = THREADS;
    io_request.ticket++;
    io_request.type = r.type;
    io_request.buffer = r.buffer;
    io_request.deadline = ticks + 100;
    io_request.state = 1;
    io_request.irq_before =
        __atomic_load_n(&disk_irq_sequence, __ATOMIC_ACQUIRE);
    if (r.type == 1)
        memcpy(bounce, (const void *)(uintptr_t)r.buffer, 4096);
    request.type = r.type;
    request.reserved = 0;
    request.sector = (uint64_t)r.page * 8;
    request_status = 255;
    vd[0] = (struct vdesc){(uintptr_t)&request, sizeof request, 1,
                           (uint16_t)(r.type == 4 ? 2 : 1)};
    vd[1] = (struct vdesc){(uintptr_t)bounce, 4096,
                           (uint16_t)(1 | (r.type == 0 ? 2 : 0)), 2};
    vd[2] = (struct vdesc){(uintptr_t)&request_status, 1, 2, 0};
    va.ring[va.idx % 8] = 0;
    barrier();
    va.idx++;
    barrier();
    async_pending = 1;
    *notify = 0;
    return io_request.ticket;
}

static int64_t async_wait(uint64_t ticket)
{
    if (!io_request.state || ticket != io_request.ticket ||
        io_request.owner != uthreads[current].process ||
        io_request.waiter != THREADS)
        return CMK2_EINVAL;
    if (io_request.state == 2)
    {
        int result = io_request.result;
        io_request.state = 0;
        return result;
    }
    io_request.waiter = current;
    uthreads[current].state = UT_IO;
    async_wait_blocks++;
    return 0;
}

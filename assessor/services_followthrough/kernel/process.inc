/* Included after retained SDK services. Protected metadata uses the kernel lock. */
/* Separate ABI-v2 formatter: the historical cmk_handle accepts only 16 slots.
 * Preserve it. SF tables explicitly admit their own bounded slot domains. */
static uint32_t sf_raw_handle(unsigned slot, unsigned generation,
                              unsigned limit)
{
#ifdef SF_MUTANT_THREAD16
    if (limit == THREADS && slot >= 16)
        return 0;
#endif
    return slot < limit && limit <= 65535 && generation && generation < 65536
               ? ((uint32_t)generation << 16) | (slot + 1u)
               : 0;
}

static uint64_t sf_process_token(unsigned pid)
{
    return SF_PROCESS_TAG | cmk_handle(pid, tasks[pid].generation);
}

static unsigned sf_process(uint64_t token)
{
    if ((token & ~UINT64_C(0xffffffff)) != SF_PROCESS_TAG)
        return TASKS;
    unsigned slot = (unsigned)(token & 65535);
    if (!slot || --slot >= task_count ||
        !cmk_handle_valid((uint32_t)token, slot, tasks[slot].generation, 1))
        return TASKS;
    return slot;
}

static unsigned sf_thread(uint64_t token, unsigned pid)
{
    if ((token & ~UINT64_C(0xffffffff)) != SF_THREAD_TAG)
        return THREADS;
    unsigned slot = (unsigned)(token & 65535);
    if (!slot || --slot >= thread_count || uthreads[slot].process != pid ||
        uthreads[slot].state == UT_UNUSED ||
        (uint32_t)token !=
            sf_raw_handle(slot, uthreads[slot].generation, THREADS))
        return THREADS;
    return slot;
}

static int sf_child(unsigned child, unsigned parent)
{
    return child < TASKS && tasks[child].parent == parent &&
           tasks[child].parent_generation == tasks[parent].generation;
}

static uint64_t sf_process_state(unsigned pid)
{
    unsigned remote = 0;
    for (unsigned c = 0; c < cmk2_cpu_count(); c++)
        if (currents[c] < thread_count && uthreads[currents[c]].process == pid)
            remote = 1;
    return (unsigned)!!tasks[pid].alive |
           ((uint64_t)tasks[pid].quiescent << 1) |
           ((uint64_t)tasks[pid].reaped << 2) |
           ((uint64_t)tasks[pid].waited << 3) |
           ((uint64_t)!!tasks[pid].io_pending << 4) | ((uint64_t)remote << 5);
}

static int64_t sf_process_call(struct frame *f)
{
    unsigned pid = uthreads[current].process;
    struct task *p = &tasks[pid];
    uint64_t a = f->rdi, b = f->rsi, c = f->rdx;
    switch (f->rax)
    {
        case SF_INFO:
            if (a == 0)
                return (int64_t)sf_process_token(pid);
            if (a == 1)
                return (int64_t)(SF_THREAD_TAG |
                                 sf_raw_handle(current,
                                               uthreads[current].generation,
                                               THREADS));
            if (a == 2)
                return (int64_t)uthreads[current].cpu_mask;
            if (a == 3)
            {
                unsigned parent = tasks[pid].parent;
                if (parent == TASKS ||
                    tasks[parent].generation != tasks[pid].parent_generation)
                    return SF_STALE;
                for (unsigned i = 0; i < thread_count; i++)
                    if (uthreads[i].process == parent &&
                        uthreads[i].state == UT_WAIT && uthreads[i].wait == pid)
                        return 1;
                return 0;
            }
            if (a == 4)
            {
                unsigned live = 0;
                for (unsigned i = 0; i < thread_count; i++)
                    if (uthreads[i].process == pid &&
                        uthreads[i].state != UT_UNUSED &&
                        uthreads[i].state != UT_DEAD)
                        live++;
                return (int64_t)(((uint64_t)live << 32) | thread_count);
            }
            return SF_INVALID;
        case SF_TYPED_THREAD:
        {
            if (!user_ok(p, a, 1, 0) ||
                (p->pt[(a - CMK_USER_BASE) / 4096] & PTE_NX) || b < 16 ||
                (b & 15) || !user_ok(p, b - 16, 16, 1))
                return CMK2_EINVAL;
            unsigned id = new_thread(pid, 0);
            if (id == THREADS)
                return CMK2_EAGAIN;
            struct frame *r = uthreads[id].frame;
            r->rip = a;
            r->rsp = b - 8;
            r->rdi = c;
            r->cs = 27;
            r->ss = 35;
            r->flags = 0x202;
            return (
                int64_t)(SF_THREAD_TAG |
                         sf_raw_handle(id, uthreads[id].generation, THREADS));
        }
        case SF_TYPED_JOIN:
        {
            unsigned tid = sf_thread(a, pid);
            if (tid == THREADS || tid == current)
                return SF_STALE;
            struct uthread *u = &uthreads[tid];
            if (u->joiner != THREADS)
                return SF_STALE;
            if (u->state == UT_DEAD)
            {
                int status = u->status;
                u->state = UT_UNUSED;
                return status;
            }
            u->joiner = current;
            uthreads[current].state = UT_JOIN;
            uthreads[current].wait = tid;
            return 0;
        }
        case SF_TYPED_SPAWN:
        {
            struct frame next = *f;
            next.rax = CMK2_SPAWN;
            int64_t r = syscall_v2(&next);
            return r < 0 ? r : (int64_t)(SF_PROCESS_TAG | (uint64_t)r);
        }
        case SF_TYPED_WAIT:
        {
            unsigned child = sf_process(a);
            if (!sf_child(child, pid))
                return SF_STALE;
            struct frame next = *f;
            next.rax = CMK2_WAIT_PROCESS;
            next.rdi = (uint32_t)a;
            return syscall_v2(&next);
        }
        case SF_PROCESS_KILL:
        {
            unsigned child = sf_process(a);
            if (!sf_child(child, pid) || !tasks[child].alive)
                return SF_STALE;
            if (c > 1)
                return SF_INVALID;
            if (c == 1 && !(sf_process_state(child) & 32))
                return SF_RETRY;
            process_exit(child, (int)b);
            uint64_t state = sf_process_state(child);
            reap();
            if ((state & 32) && tasks[child].reaped)
                panic("sf remote address space reused");
            return (int64_t)state;
        }
        case SF_PROCESS_STATE:
        {
            unsigned child = sf_process(a);
            if (!sf_child(child, pid))
                return SF_STALE;
            return (int64_t)sf_process_state(child);
        }
        case SF_FAULT_CONTROL:
            if (a != UINT64_MAX && a > 512)
                return SF_INVALID;
            if (b > 1)
                return SF_INVALID;
            sf_fault_remaining = a == UINT64_MAX ? -1 : (int)a;
            sf_seed_frames = (unsigned)b;
            return 0;
        case SF_MAPPING:
            if (a < CMK_USER_BASE || a >= CMK_USER_LIMIT || b > 1)
                return SF_INVALID;
            if (b)
            {
                uint64_t e = p->pt[(a - CMK_USER_BASE) / 4096];
                return e & 1 ? page_refs[(e & UINT64_C(0x000ffffffffff000)) /
                                         4096]
                             : 0;
            }
            return (int64_t)p->pt[(a - CMK_USER_BASE) / 4096];
        case SF_FAULT_CAUSES:
        {
            /*64 cause words times8 representative current mappings; classify independently. */
            const uint64_t entries[] = {0,
                                        PTE_ZERO | 6 | PTE_NX,
                                        7 | PTE_NX,
                                        5 | PTE_NX,
                                        7,
                                        5,
                                        5 | PTE_COW | PTE_NX,
                                        7 | PTE_SHARED | PTE_NX};
            unsigned checked = 0;
            for (unsigned error = 0; error < 32; error++)
                for (unsigned i = 0; i < 8; i++)
                {
                    unsigned got = sf_fault_class(error, entries[i]),
                             expected = 0;
                    if ((error & 4) && !(error & 8))
                    {
                        if (!(error & 1) && i == 1 && !(error & 16))
                            expected = 1;
                        else if ((error & 7) == 7 && i == 6 && !(error & 16))
                            expected = 2;
                        else if (i >= 2 &&
                                 (!(error & 2) || i == 2 || i == 4 || i == 7) &&
                                 (!(error & 16) || i == 4 || i == 5))
                            expected = 3;
                    }
                    if (got != expected)
                        return SF_INVALID;
                    checked++;
                }
            return checked;
        }
        case SF_CLONE_SATURATION:
        {
            if (!user_ok(p, a, 1, 0))
                return SF_COPY;
            unsigned index = (unsigned)((a - CMK_USER_BASE) / 4096);
            unsigned frame =
                (unsigned)((p->pt[index] & UINT64_C(0x000ffffffffff000)) /
                           4096);
            uint16_t saved = page_refs[frame];
            static uint64_t before[512];
            memcpy(before, p->pt, sizeof before);
            unsigned nt = thread_count, np = task_count;
            page_refs[frame] = 65535;
            struct frame next = *f;
            next.rax = SDK_CLONE_PRIVATE;
            int64_t r = sd_syscall(&next);
            unsigned unchanged = page_refs[frame] == 65535 &&
                                 nt == thread_count && np == task_count &&
                                 !memcmp(before, p->pt, sizeof before);
            page_refs[frame] = saved;
            return r == SDK_FULL && unchanged ? 0 : SF_INVALID;
        }
        default:
            (void)c;
            return SF_INVALID;
    }
}

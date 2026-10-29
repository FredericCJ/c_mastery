/* Explicit replacement for the retained load_task; linked only in this adapter.
 * Static page-table controls are reserved slots, not separately allocated frames.
 * All calls hold the shared kernel lock or run before user CPUs start. */
static int sd_fail_after = -1;
static unsigned sd_allocations;

static uint32_t sd_take(void)
{
    if (sd_fail_after >= 0 && sd_allocations++ == (unsigned)sd_fail_after)
        return 0;
    return cmk_frame_take(&frames);
}

static int load_task(const char *name, int disk)
{
    unsigned id = task_slot(), tid;
    if (id == TASKS)
        return -2;
    for (tid = 0; tid < THREADS; tid++)
        if (uthreads[tid].state == UT_UNUSED &&
            uthreads[tid].generation < 65535)
            break;
    if (tid == THREADS)
        return -2;
    const unsigned char *data;
    size_t size;
    struct cmk_elf elf;
    if (cmk_archive_find(archive, archive_size, name, &data, &size) ||
        cmk_elf_parse(data, size, &elf))
        return -1;
    struct task *t = &tasks[id];
    /* Preserve the whole unowned descriptor on failure; never publish its tables. */
    static struct task saved;
    memcpy(&saved, t, sizeof saved);
    unsigned acquired[512], count = 0;
    memset(t, 0, sizeof *t);
    t->generation = saved.generation + 1;
    t->handle_generation = 1;
    t->name = name;
    t->handle_open = disk;
    t->parent = TASKS;
    t->pml4[0] = (uintptr_t)t->pdpt | 7;
    for (unsigned i = 0; i < 4; i++)
        t->pdpt[i] = boot_pdpt[i];
    t->pdpt[1] = (uintptr_t)t->pd | 7;
    t->pd[0] = (uintptr_t)t->pt | 7;
    for (unsigned segment = 0; segment <= elf.count; segment++)
    {
        uint64_t first, end;
        unsigned flags;
        if (segment == elf.count)
        {
            first = CMK_USER_BASE + 480 * 4096;
            end = CMK_USER_LIMIT;
            flags = 6;
        }
        else
        {
            struct cmk_segment *s = &elf.segments[segment];
            first = s->address & ~UINT64_C(4095);
            end = s->address + s->memsz;
            flags = s->flags;
        }
        for (uint64_t va = first; va < end; va += 4096)
        {
            unsigned index = (unsigned)((va - CMK_USER_BASE) / 4096);
            uint32_t frame = sd_take();
            if (!frame)
                goto rollback;
            acquired[count++] = frame;
            memset((void *)(uintptr_t)frame, 0, 4096);
            page_refs[frame / 4096] = 1;
            t->pt[index] = (uint64_t)frame | 5 | ((flags & 2) ? 2 : 0) |
                           ((flags & 1) ? 0 : PTE_NX);
        }
        if (segment < elf.count)
        {
            struct cmk_segment *s = &elf.segments[segment];
            for (uint64_t j = 0; j < s->filesz; j++)
                *(unsigned char *)physical(t, s->address + j) =
                    data[s->offset + j];
        }
    }
    t->frame = (struct frame *)(void *)(t->stack + sizeof t->stack -
                                        sizeof(struct frame));
    memset(t->frame, 0, sizeof *t->frame);
    t->frame->rip = elf.entry;
    t->frame->cs = 27;
    t->frame->ss = 35;
    t->frame->flags = 0x202;
    t->frame->rsp = CMK_USER_LIMIT - 8;
    if (new_thread(id, t->frame) == THREADS)
        goto rollback;
    t->alive = 1;
    if (id == task_count)
        task_count++;
    return 0;
rollback:
    while (count)
    {
        uint32_t frame = acquired[--count];
#ifndef SDK_MUTANT_LOAD_LEAK
        page_refs[frame / 4096] = 0;
        memset((void *)(uintptr_t)frame, 0, 4096);
        if (cmk_frame_put(&frames, frame))
            panic("sd rollback ownership");
#else
        (void)frame;
#endif
    }
    memcpy(t, &saved, sizeof *t);
    return -2;
}

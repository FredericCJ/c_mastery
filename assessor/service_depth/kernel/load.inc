/* Explicit replacement for the retained load_task; linked only in this adapter.
 * Static page-table controls are reserved slots, not separately allocated frames.
 * All calls hold the shared kernel lock or run before user CPUs start.
 *
 * Contract: returns 0 only after the task is completely published; -1 for an
 * image the checked ELF parser rejects or an archive with no entry of that
 * name; -2 when a slot, thread or frame is
 * unavailable. Every failure restores the task descriptor, the frame
 * allocator and the page reference counts exactly (the author probe
 * SDK_LOAD_PROBE injects a failure at every allocation point to prove it).
 * SDK_MUTANT_LOAD_LEAK removes the frame release in the rollback. */
static int sd_fail_after = -1;
static unsigned sd_allocations;

enum
{
    SD_PAGE_BYTES = 4096,
    /* A task maps at most one page table of pages. */
    SD_MAX_TASK_FRAMES = 512,
    SD_PTE_PRESENT = 1,
    SD_PTE_WRITABLE = 2,
    SD_PTE_USER = 4,
    SD_TABLE_FLAGS = SD_PTE_PRESENT | SD_PTE_WRITABLE | SD_PTE_USER,
    /* ELF segment flag bits. */
    SD_ELF_EXECUTE = 1,
    SD_ELF_WRITE = 2,
    SD_ELF_READ = 4,
    SD_STACK_FIRST_PAGE = 480,
    SD_STACK_ELF_FLAGS = SD_ELF_WRITE | SD_ELF_READ,
    SD_USER_CODE_SELECTOR = 27,
    SD_USER_STACK_SELECTOR = 35,
    SD_INITIAL_RFLAGS = 0x202
};

/* Allocates one frame; returns 0 (no frame) at the injected failure point. */
static uint32_t sd_take(void)
{
    if (sd_fail_after >= 0 && sd_allocations++ == (unsigned)sd_fail_after)
    {
        return 0;
    }
    return cmk_frame_take(&frames);
}

/* Lowest unused thread slot whose generation can still advance, or THREADS. */
static unsigned sd_free_thread_slot(void)
{
    for (unsigned tid = 0; tid < THREADS; tid++)
    {
        if (uthreads[tid].state == UT_UNUSED &&
            uthreads[tid].generation < 65535)
        {
            return tid;
        }
    }
    return THREADS;
}

static uint64_t sd_leaf_entry(uint32_t frame, unsigned elf_flags)
{
    uint64_t entry = (uint64_t)frame | SD_PTE_PRESENT | SD_PTE_USER;
    if (elf_flags & SD_ELF_WRITE)
    {
        entry |= SD_PTE_WRITABLE;
    }
    if (!(elf_flags & SD_ELF_EXECUTE))
    {
        entry |= PTE_NX;
    }
    return entry;
}

/* Maps zeroed frames over [first, end) and records each in acquired[] so a
 * later failure can return them. Returns 0, or -1 when no frame is left. */
static int sd_map_range(struct task *t, uint64_t first, uint64_t end,
                        unsigned elf_flags, unsigned *acquired, unsigned *count)
{
    for (uint64_t va = first; va < end; va += SD_PAGE_BYTES)
    {
        unsigned index = (unsigned)((va - CMK_USER_BASE) / SD_PAGE_BYTES);
        uint32_t frame = sd_take();
        if (!frame)
        {
            return -1;
        }
        acquired[(*count)++] = frame;
        memset((void *)(uintptr_t)frame, 0, SD_PAGE_BYTES);
        page_refs[frame / SD_PAGE_BYTES] = 1;
        t->pt[index] = sd_leaf_entry(frame, elf_flags);
    }
    return 0;
}

/* Returns every frame taken so far, newest first. */
static void sd_release_frames(unsigned *acquired, unsigned count)
{
    while (count)
    {
        uint32_t frame = acquired[--count];
#ifndef SDK_MUTANT_LOAD_LEAK
        page_refs[frame / SD_PAGE_BYTES] = 0;
        memset((void *)(uintptr_t)frame, 0, SD_PAGE_BYTES);
        if (cmk_frame_put(&frames, frame))
        {
            panic("sd rollback ownership");
        }
#else
        (void)frame;
#endif
    }
}

/* Builds the first user-mode frame at the top of the kernel stack. */
static void sd_start_frame(struct task *t, uint64_t entry)
{
    t->frame = (struct frame *)(void *)(t->stack + sizeof t->stack -
                                        sizeof(struct frame));
    memset(t->frame, 0, sizeof *t->frame);
    t->frame->rip = entry;
    t->frame->cs = SD_USER_CODE_SELECTOR;
    t->frame->ss = SD_USER_STACK_SELECTOR;
    t->frame->flags = SD_INITIAL_RFLAGS;
    t->frame->rsp = CMK_USER_LIMIT - 8;
}

static int load_task(const char *name, int disk)
{
    unsigned id = task_slot();
    if (id == TASKS)
    {
        return -2;
    }
    if (sd_free_thread_slot() == THREADS)
    {
        return -2;
    }
    const unsigned char *data;
    size_t size;
    struct cmk_elf elf;
    if (cmk_archive_find(archive, archive_size, name, &data, &size) ||
        cmk_elf_parse(data, size, &elf))
    {
        return -1;
    }
    struct task *t = &tasks[id];
    /* Preserve the whole unowned descriptor on failure; never publish its tables. */
    static struct task saved;
    memcpy(&saved, t, sizeof saved);
    unsigned acquired[SD_MAX_TASK_FRAMES];
    unsigned count = 0;
    memset(t, 0, sizeof *t);
    t->generation = saved.generation + 1;
    t->handle_generation = 1;
    t->name = name;
    t->handle_open = disk;
    t->parent = TASKS;
    t->pml4[0] = (uintptr_t)t->pdpt | SD_TABLE_FLAGS;
    for (unsigned i = 0; i < 4; i++)
    {
        t->pdpt[i] = boot_pdpt[i];
    }
    t->pdpt[1] = (uintptr_t)t->pd | SD_TABLE_FLAGS;
    t->pd[0] = (uintptr_t)t->pt | SD_TABLE_FLAGS;
    for (unsigned segment = 0; segment < elf.count; segment++)
    {
        struct cmk_segment *s = &elf.segments[segment];
        uint64_t first = s->address & ~((uint64_t)SD_PAGE_BYTES - 1);
        uint64_t end = s->address + s->memsz;
        if (sd_map_range(t, first, end, s->flags, acquired, &count))
        {
            goto rollback;
        }
        for (uint64_t j = 0; j < s->filesz; j++)
        {
            *(unsigned char *)physical(t, s->address + j) = data[s->offset + j];
        }
    }
    /* The user stack is one more mapping after the image segments. */
    if (sd_map_range(t, CMK_USER_BASE + SD_STACK_FIRST_PAGE * SD_PAGE_BYTES,
                     CMK_USER_LIMIT, SD_STACK_ELF_FLAGS, acquired, &count))
    {
        goto rollback;
    }
    sd_start_frame(t, elf.entry);
    if (new_thread(id, t->frame) == THREADS)
    {
        goto rollback;
    }
    t->alive = 1;
    if (id == task_count)
    {
        task_count++;
    }
    return 0;
rollback:
    sd_release_frames(acquired, count);
    memcpy(t, &saved, sizeof *t);
    return -2;
}

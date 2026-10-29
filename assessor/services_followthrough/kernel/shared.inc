/* Creation owns one frame reference. Every admitted mapping owns another. */
struct sf_shared
{
    unsigned generation, open, owner, owner_generation, frame, rights[TASKS],
        process_generation[TASKS];
};
static struct sf_shared sf_shared[4];
static unsigned char sf_owned_mapping[TASKS][512];

static unsigned sf_shared_lookup(uint64_t token, unsigned pid)
{
    if ((token & ~UINT64_C(0xffffffff)) != SF_SHARED_TAG)
    {
        return 4;
    }
    unsigned i = (unsigned)(token & 65535);
    if (!i || --i >= 4)
    {
        return 4;
    }
    struct sf_shared *o = &sf_shared[i];
    if (!o->open || !cmk_handle_valid((uint32_t)token, i, o->generation, 1) ||
        !o->rights[pid] || o->process_generation[pid] != tasks[pid].generation)
    {
        return 4;
    }
    return i;
}

static void sf_shared_close(unsigned i)
{
    struct sf_shared *o = &sf_shared[i];
    o->open = 0;
    if (!page_refs[o->frame / 4096])
    {
        panic("sf object reference");
    }
    if (!--page_refs[o->frame / 4096] && cmk_frame_put(&frames, o->frame))
    {
        panic("sf object free");
    }
}

static void sf_shared_cleanup(unsigned pid)
{
    for (unsigned i = 0; i < 4; i++)
    {
        if (sf_shared[i].open)
        {
            if (sf_shared[i].owner == pid &&
                sf_shared[i].owner_generation == tasks[pid].generation)
            {
                sf_shared_close(i);
            }
            sf_shared[i].rights[pid] = 0;
        }
    }
    /* Retained process reaping drops actual PTE references only after all CPUs leave. */
    memset(sf_owned_mapping[pid], 0, sizeof sf_owned_mapping[pid]);
}

static int64_t sf_shared_call(struct frame *f)
{
    unsigned pid = uthreads[current].process;
    struct task *p = &tasks[pid];
    uint64_t a = f->rdi, b = f->rsi;
    switch (f->rax)
    {
        case SF_SHM_CREATE:
            for (unsigned i = 0; i < 4; i++)
            {
                if (!sf_shared[i].open && sf_shared[i].generation < 65535)
                {
                    uint32_t frame = cmk_frame_take(&frames);
                    if (!frame)
                    {
                        return SF_FULL;
                    }
                    memset((void *)(uintptr_t)frame, 0, 4096);
                    page_refs[frame / 4096] = 1;
                    struct sf_shared *o = &sf_shared[i];
                    unsigned generation = o->generation + 1;
                    memset(o, 0, sizeof *o);
                    o->generation = generation;
                    o->open = 1;
                    o->owner = pid;
                    o->owner_generation = p->generation;
                    o->frame = frame;
                    o->rights[pid] = 3;
                    o->process_generation[pid] = p->generation;
                    return (int64_t)(SF_SHARED_TAG | cmk_handle(i, generation));
                }
            }
            return SF_FULL;
        case SF_SHM_GRANT:
        {
            unsigned i = sf_shared_lookup(a, pid);
            if (i == 4)
            {
                return SF_DENIED;
            }
            struct sf_grant g;
            if (!user_ok(p, b, sizeof g, 0))
            {
                return SF_COPY;
            }
            memcpy(&g, (void *)(uintptr_t)b, sizeof g);
            unsigned who = sf_process(g.recipient);
            struct sf_shared *o = &sf_shared[i];
            if (who == TASKS || !tasks[who].alive || g.reserved || !g.rights ||
                (g.rights & ~3u) || (g.rights & o->rights[pid]) != g.rights ||
                o->rights[who])
            {
                return SF_DENIED;
            }
            o->rights[who] = g.rights;
            o->process_generation[who] = tasks[who].generation;
            return (int64_t)a;
        }
        case SF_SHM_MAP:
        {
            unsigned i = sf_shared_lookup(a, pid);
            if (i == 4)
            {
                return SF_DENIED;
            }
            struct sf_map m;
            if (!user_ok(p, b, sizeof m, 0))
            {
                return SF_COPY;
            }
            memcpy(&m, (void *)(uintptr_t)b, sizeof m);
            struct sf_shared *o = &sf_shared[i];
            if (m.reserved || m.length != 4096 || m.offset || !m.rights ||
                (m.rights & ~3u) || (m.rights & o->rights[pid]) != m.rights)
            {
                return SF_DENIED;
            }
            unsigned index;
            if (m.address)
            {
                if (m.address < CMK_USER_BASE + 256 * 4096 ||
                    m.address >= CMK_USER_BASE + 479 * 4096 || m.address % 4096)
                {
                    return SF_INVALID;
                }
                index = (unsigned)((m.address - CMK_USER_BASE) / 4096);
                if (p->pt[index])
                {
                    return SF_FULL;
                }
            }
            else
            {
                for (index = 256; index < 479; index++)
                {
                    if (!p->pt[index])
                    {
                        break;
                    }
                }
                if (index == 479)
                {
                    return SF_FULL;
                }
            }
            if (!page_refs[o->frame / 4096] ||
                page_refs[o->frame / 4096] == 65535)
            {
                return SF_FULL;
            }
            page_refs[o->frame / 4096]++;
            p->pt[index] = (uint64_t)o->frame | 5 | PTE_SHARED | PTE_NX |
                           ((m.rights & 2) ? 2 : 0);
            sf_owned_mapping[pid][index] = 1;
            cmk2_flush_remote();
            return (int64_t)(CMK_USER_BASE + (uint64_t)index * 4096);
        }
        case SF_SHM_UNMAP:
        {
            if (a < CMK_USER_BASE + 256 * 4096 ||
                a >= CMK_USER_BASE + 479 * 4096 || a % 4096 || b != 4096)
            {
                return SF_INVALID;
            }
            unsigned i = (unsigned)((a - CMK_USER_BASE) / 4096);
            if (!sf_owned_mapping[pid][i] || !(p->pt[i] & 1))
            {
                return SF_STALE;
            }
            unsigned frame = (unsigned)(p->pt[i] & UINT64_C(0xfffff000));
            p->pt[i] = 0;
            sf_owned_mapping[pid][i] = 0;
            cmk2_flush_remote();
            __asm__ volatile("invlpg (%0)" ::"r"(a) : "memory");
            if (!page_refs[frame / 4096])
            {
                panic("sf map reference");
            }
            if (!--page_refs[frame / 4096] && cmk_frame_put(&frames, frame))
            {
                panic("sf map free");
            }
            return 0;
        }
        case SF_SHM_CLOSE:
        {
            unsigned i = sf_shared_lookup(a, pid);
            if (i == 4 || sf_shared[i].owner != pid ||
                sf_shared[i].owner_generation != p->generation)
            {
                return SF_DENIED;
            }
            sf_shared_close(i);
            return 0;
        }
        default:
            return SF_INVALID;
    }
}

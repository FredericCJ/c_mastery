#include "process.inc"
#include "channels.inc"
#include "shared.inc"
static struct sf_bootstrap sf_bootstraps[TASKS];
static unsigned sf_bootstrap_ready[TASKS];

static int sf_flat_name(const unsigned char *name, size_t n)
{
    if (!n || n >= 24 || (n == 1 && name[0] == '.') ||
        (n == 2 && name[0] == '.' && name[1] == '.'))
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        unsigned c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'))
        {
            return 0;
        }
    }
    return 1;
}

static int sf_archive_find(const void *v, size_t size, const char *name,
                           const unsigned char **data, size_t *length)
{
    const unsigned char *p = v;
    size_t wanted = 0;
    while (wanted < 24 && name[wanted])
    {
        wanted++;
    }
    if (!sf_flat_name((const unsigned char *)name, wanted))
    {
        return -1;
    }
    const unsigned char *candidate;
    size_t bytes;
    if (cmk_archive_find(v, size, name, &candidate, &bytes))
    {
        return -1;
    }
    unsigned count = (unsigned)p[8] | ((unsigned)p[9] << 8) |
                     ((unsigned)p[10] << 16) | ((unsigned)p[11] << 24);
    for (unsigned i = 0; i < count; i++)
    {
        const unsigned char *entry = p + 16 + 40 * i;
        size_t n = 0;
        while (n < 24 && entry[n])
        {
            n++;
        }
        if (!sf_flat_name(entry, n))
        {
            return -1;
        }
    }
    *data = candidate;
    *length = bytes;
    return 0;
}

static void sf_fork_maps(unsigned parent, unsigned child)
{
    memcpy(sf_owned_mapping[child], sf_owned_mapping[parent],
           sizeof sf_owned_mapping[child]);
}

static void sf_process_cleanup(unsigned pid)
{
    sf_bootstrap_ready[pid] = 0;
    sf_channel_cleanup(pid);
    sf_shared_cleanup(pid);
}

static void sf_thread_cleanup(unsigned tid)
{
    sf_waiters[tid].active = 0;
}

static int64_t sf_syscall(struct frame *f)
{
    unsigned pid = uthreads[current].process;
    struct task *p = &tasks[pid];
    uint64_t a = f->rdi, b = f->rsi;
    if (f->rax == SF_LOAD_FULL)
    {
        static struct task old_tasks[TASKS];
        static struct uthread old_threads[THREADS];
        memcpy(old_tasks, tasks, sizeof tasks);
        memcpy(old_threads, uthreads, sizeof uthreads);
        struct frame next = *f;
        next.rax = SDK_LOAD_PROBE;
        int64_t result = sd_syscall(&next);
        return memcmp(old_tasks, tasks, sizeof tasks) ||
                       memcmp(old_threads, uthreads, sizeof uthreads)
                   ? SF_INVALID
                   : result;
    }
    if (f->rax == SF_FAULT_BARRIER)
    {
        if (!a)
        {
            atomic_store_explicit(&sf_fault_gate, 0, memory_order_release);
            return atomic_load_explicit(&sf_fault_arrivals,
                                        memory_order_acquire);
        }
        if (a % 4096 || a < CMK_USER_BASE || a >= CMK_USER_LIMIT ||
            !(p->pt[(a - CMK_USER_BASE) / 4096] & PTE_ZERO))
        {
            return SF_INVALID;
        }
        sf_fault_address = a;
        atomic_store_explicit(&sf_fault_arrivals, 0, memory_order_relaxed);
        atomic_store_explicit(&sf_fault_gate, 1, memory_order_release);
        return 0;
    }
    if (f->rax == SF_IO_CONTROL)
    {
        if (pid || a > 1 || b)
        {
            return SF_DENIED;
        }
        sf_io_hold = (unsigned)a;
        return 0;
    }
    if (f->rax == SF_IO_SNAPSHOT)
    {
        if (pid || !user_ok(p, a, sizeof(struct sf_io_snapshot), 1))
        {
            return SF_DENIED;
        }
        unsigned owner = io_request.owner;
        struct task *o = &tasks[owner];
        struct sf_io_snapshot s = {
            sf_process_token(owner),
            io_request.state,
            io_request.ticket,
            o->io_pending,
            (unsigned)!!o->alive,
            o->quiescent,
            o->reaped,
            o->waited,
            (unsigned)(__atomic_load_n(&disk_irq_sequence, __ATOMIC_ACQUIRE) !=
                           io_request.irq_before &&
                       vu.idx != used_idx),
            task_slot()};
        memcpy((void *)(uintptr_t)a, &s, sizeof s);
        return 0;
    }
    if (f->rax == SF_OPEN)
    {
        if (!b || b > 23 || !user_ok(p, a, b, 0))
        {
            return SDK_COPY;
        }
        char name[24];
        memcpy(name, (void *)(uintptr_t)a, (size_t)b);
        name[b] = 0;
        const unsigned char *data;
        size_t length;
        if (sf_archive_find(archive, archive_size, name, &data, &length))
        {
            return SDK_INVALID;
        }
        struct frame next = *f;
        next.rax = SDK_OPEN;
        return sd_syscall(&next);
    }
    if (f->rax == SF_ARCHIVE_PROBE)
    {
        unsigned char bytes[98] = {0};
        memcpy(bytes, "CMKA", 4);
        bytes[4] = 1;
        bytes[8] = 2;
        bytes[12] = 98;
        bytes[16] = 'a';
        bytes[40] = 96;
        bytes[44] = 1;
        bytes[56] = 'b';
        bytes[80] = 97;
        bytes[84] = 1;
        bytes[96] = 11;
        bytes[97] = 12;
        const unsigned char *out = (const unsigned char *)(uintptr_t)1;
        size_t size = 9;
        unsigned count = 0;
        if (sf_archive_find(bytes, sizeof bytes, "a", &out, &size) ||
            size != 1 || *out != 11)
        {
            return SF_INVALID;
        }
        count++;
        bytes[56] = 'a';
        if (!sf_archive_find(bytes, sizeof bytes, "a", &out, &size))
        {
            return SF_INVALID;
        }
        count++;
        bytes[56] = 'b';
        bytes[56] = '.';
        bytes[57] = '.';
        bytes[58] = '/';
        bytes[59] = 'b';
        if (!sf_archive_find(bytes, sizeof bytes, "a", &out, &size))
        {
            return SF_INVALID;
        }
        count++;
        memset(bytes + 56, 0, 24);
        bytes[56] = 'b';
        bytes[84] = 255;
        if (!sf_archive_find(bytes, sizeof bytes, "a", &out, &size))
        {
            return SF_INVALID;
        }
        count++;
        bytes[84] = 1;
        if (!sf_archive_find(bytes, sizeof bytes, "../a", &out, &size) ||
            !sf_archive_find(bytes, sizeof bytes, ".", &out, &size))
        {
            return SF_INVALID;
        }
        return count + 2;
    }
    if (f->rax == SF_BIND)
    {
        unsigned child = sf_process(a);
        if (!sf_child(child, pid) || !tasks[child].alive ||
            sf_bootstrap_ready[child])
        {
            return SF_DENIED;
        }
        struct sf_bootstrap value;
        if (!user_ok(p, b, sizeof value, 0))
        {
            return SF_COPY;
        }
        memcpy(&value, (void *)(uintptr_t)b, sizeof value);
        if (value.reserved || !value.session || value.role > 8 ||
            (value.request && sf_cap(value.request, child, 0) == 64) ||
            (value.reply && sf_cap(value.reply, child, 0) == 64))
        {
            return SF_DENIED;
        }
        sf_bootstraps[child] = value;
        sf_bootstrap_ready[child] = 1;
        return 0;
    }
    if (f->rax == SF_BOOTSTRAP)
    {
        if (!sf_bootstrap_ready[pid])
        {
            return SF_FULL;
        }
        if (!user_ok(p, a, sizeof(struct sf_bootstrap), 1))
        {
            return SF_COPY;
        }
        memcpy((void *)(uintptr_t)a, &sf_bootstraps[pid],
               sizeof(struct sf_bootstrap));
        return 0;
    }
    if (f->rax == SF_TABLE_LEDGER)
    {
        if (!user_ok(p, a, sizeof(struct sf_ledger), 1))
        {
            return SF_COPY;
        }
        struct sf_ledger v = {0};
        for (unsigned i = 0; i < task_count; i++)
        {
            v.processes += (unsigned)!!tasks[i].alive;
            v.pending_io += (unsigned)!!tasks[i].io_pending;
        }
        for (unsigned i = 0; i < thread_count; i++)
        {
            if (uthreads[i].state != UT_UNUSED)
            {
                v.threads++;
                if (uthreads[i].state == UT_READY ||
                    uthreads[i].state == UT_RUNNING)
                {
                    v.ready++;
                }
                else if (uthreads[i].state == UT_DEAD)
                {
                    v.dead++;
                }
                else
                {
                    v.blocked++;
                }
            }
        }
        for (unsigned i = 0; i < CMK_MAX_FRAMES; i++)
        {
            v.frames += (unsigned)(frames.state[i] == 2);
            v.references += page_refs[i];
        }
        for (unsigned i = 0; i < 8; i++)
        {
            if (sf_queues[i].state)
            {
                v.channel_messages += sf_queues[i].count;
            }
        }
        for (unsigned i = 0; i < THREADS; i++)
        {
            v.channel_waiters += sf_waiters[i].active;
        }
        for (unsigned i = 0; i < 4; i++)
        {
            v.shared_open += sf_shared[i].open;
        }
        for (unsigned process = 0; process < TASKS; process++)
        {
            for (unsigned i = 0; i < 512; i++)
            {
                v.shared_mapping_refs += sf_owned_mapping[process][i];
            }
        }
        memcpy((void *)(uintptr_t)a, &v, sizeof v);
        return 0;
    }
    if (f->rax == SF_SEEK_EXTENDED)
    {
        unsigned h = sd_file(a, pid);
        if (h == 16)
        {
            return SDK_STALE;
        }
        struct sdk_seek s;
        if (!user_ok(p, b, sizeof s, 0))
        {
            return SDK_COPY;
        }
        memcpy(&s, (void *)(uintptr_t)b, sizeof s);
        if (s.origin > 2 || s.reserved)
        {
            return SDK_INVALID;
        }
        struct sd_open_description *d =
            &sd_descriptions[sd_handles[h].description];
        size_t length =
            d->live ? sd_versions[sd_current_version].length : d->length;
        int64_t base =
            s.origin == 0 ? 0 : (int64_t)(s.origin == 1 ? d->offset : length);
        if ((s.offset > 0 && base > INT64_MAX - s.offset) ||
            (s.offset < 0 && base < INT64_MIN - s.offset))
        {
            return SDK_INVALID;
        }
        int64_t position = base + s.offset;
        if (position < 0 || position > 1048576)
        {
            return SDK_INVALID;
        }
        d->offset = (size_t)position;
        return position;
    }
    if (f->rax >= SF_CHANNEL && f->rax <= SF_BLOCKED)
    {
        return sf_channel_call(f);
    }
    if (f->rax >= SF_SHM_CREATE && f->rax <= SF_SHM_CLOSE)
    {
        return sf_shared_call(f);
    }
    return sf_process_call(f);
}

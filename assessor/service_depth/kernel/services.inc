#include "kernel_abi.h"

/* Kernel-private tables; user numeric tokens never contain authoritative rights. */
struct sd_open_description
{
    const unsigned char *data;
    size_t length, offset;
    unsigned refs, version, live;
};

struct sd_file_handle
{
    unsigned generation, owners, description, open;
};
static struct sd_open_description sd_descriptions[8];
static struct sd_file_handle sd_handles[16];

struct sd_version
{
    unsigned refs, length;
    unsigned char bytes[32];
};
static struct sd_version sd_versions[2];
static unsigned sd_current_version, sd_mutable_ready, sd_mutable_owner,
    sd_mutable_owner_generation;

static unsigned sd_file(uint64_t token, unsigned pid)
{
    for (unsigned i = 0; i < 16; i++)
    {
        if (sd_handles[i].open && (sd_handles[i].owners & (1u << pid)) &&
            token <= UINT32_MAX &&
            cmk_handle_valid((uint32_t)token, i, sd_handles[i].generation, 1))
        {
            return i;
        }
    }
    return 16;
}

static void sd_file_close(unsigned h, unsigned pid)
{
    struct sd_open_description *d = &sd_descriptions[sd_handles[h].description];
    if (!d->refs)
    {
        panic("sd file references");
    }
    d->refs--;
    sd_handles[h].owners &= ~(1u << pid);
    if (!sd_handles[h].owners)
    {
        sd_handles[h].open = 0;
    }
    if (!d->refs && d->version < 2 && !d->live)
    {
        sd_versions[d->version].refs--;
    }
}

static void sd_fork_files(unsigned parent, unsigned child)
{
#ifdef SDK_MUTANT_FILE_FORK
    (void)parent;
    (void)child;
    return;
#endif
    for (unsigned h = 0; h < 16; h++)
    {
        if (sd_handles[h].open && (sd_handles[h].owners & (1u << parent)))
        {
            sd_handles[h].owners |= 1u << child;
            sd_descriptions[sd_handles[h].description].refs++;
        }
    }
}

struct sd_message
{
    unsigned n, consumed, reader;
    unsigned char bytes[32];
};

struct sd_queue
{
    unsigned open, owner, count, head, generation;
    struct sd_message message[2];
};

struct sd_capability
{
    unsigned open, owner, generation, queue, incarnation, rights;
};
static struct sd_queue sd_queues[8];
static struct sd_capability sd_caps[32];

/* One pending syscall per thread, with a kernel-owned payload on send.
 * Wake returns RETRY. A receiver never leaves a user pointer in the wait table. */
struct sd_pending
{
    unsigned active, queue, incarnation, sending;
    struct sd_message message;
};
static struct sd_pending sd_waiters[THREADS];

static void sd_thread_cleanup(unsigned tid)
{
    sd_waiters[tid].active = 0;
    for (unsigned q = 0; q < 8; q++)
    {
        if (sd_queues[q].open && sd_queues[q].count)
        {
            struct sd_message *m = &sd_queues[q].message[sd_queues[q].head];
            if (m->reader == tid)
            {
                m->reader = THREADS;
            }
        }
    }
}

static unsigned sd_cap(uint64_t token, unsigned pid, unsigned rights)
{
    if (token > UINT32_MAX)
    {
        return 32;
    }
    for (unsigned i = 0; i < 32; i++)
    {
        struct sd_capability *c = &sd_caps[i];
        if (c->open && c->owner == pid &&
            cmk_handle_valid((uint32_t)token, i, c->generation, 1) &&
            c->queue < 8 && sd_queues[c->queue].open &&
            sd_queues[c->queue].generation == c->incarnation)
        {
#ifndef SDK_MUTANT_RIGHTS
            if ((c->rights & rights) != rights)
            {
                return 32;
            }
#else
            (void)rights;
#endif
            return i;
        }
    }
    return 32;
}

static int64_t sd_new_cap(unsigned pid, unsigned q, unsigned rights)
{
    for (unsigned i = 0; i < 32; i++)
    {
        if (!sd_caps[i].open && sd_caps[i].generation < 65535)
        {
            struct sd_capability *c = &sd_caps[i];
            c->generation++;
            c->open = 1;
            c->owner = pid;
            c->queue = q;
            c->incarnation = sd_queues[q].generation;
            c->rights = rights;
            return cmk_handle(i, c->generation);
        }
    }
    return SDK_FULL;
}

static void sd_wake(unsigned q)
{
    for (unsigned i = 0; i < thread_count; i++)
    {
        if (sd_waiters[i].active && sd_waiters[i].queue == q)
        {
            if (sd_waiters[i].incarnation != sd_queues[q].generation)
            {
                sd_waiters[i].active = 0;
                uthreads[i].frame->rax = (uint64_t)(int64_t)SDK_STALE;
                uthreads[i].state = UT_READY;
                continue;
            }
            if (sd_waiters[i].sending)
            {
                if (sd_queues[q].open == 1 && sd_queues[q].count == 2)
                {
                    continue;
                }
                if (sd_queues[q].open == 1)
                {
                    sd_queues[q]
                        .message[(sd_queues[q].head + sd_queues[q].count) % 2] =
                        sd_waiters[i].message;
                    sd_queues[q].count++;
                    uthreads[i].frame->rax = 0;
                }
                else
                {
                    uthreads[i].frame->rax = (uint64_t)(int64_t)SDK_CLOSED;
                }
            }
            else
            {
                uthreads[i].frame->rax = SDK_RETRY;
            }
            sd_waiters[i].active = 0;
            uthreads[i].state = UT_READY;
        }
    }
}

static void sd_close_queue(unsigned q)
{
    sd_queues[q].open = 2;
    sd_wake(q);
}

static void sd_process_cleanup(unsigned pid)
{
    for (unsigned h = 0; h < 16; h++)
    {
        if (sd_handles[h].open && (sd_handles[h].owners & (1u << pid)))
        {
            sd_file_close(h, pid);
        }
    }
    for (unsigned i = 0; i < thread_count; i++)
    {
        if (uthreads[i].process == pid)
        {
            sd_waiters[i].active = 0;
        }
    }
    for (unsigned q = 0; q < 8; q++)
    {
        if (sd_queues[q].open && sd_queues[q].owner == pid)
        {
            sd_close_queue(q);
            /* This profile grants only within the owning process; no surviving grantee. */
            sd_queues[q].open = 0;
        }
    }
    for (unsigned i = 0; i < 32; i++)
    {
        if (sd_caps[i].owner == pid)
        {
            sd_caps[i].open = 0;
        }
    }
}

static int sd_span(struct task *p, uint64_t address, struct sdk_span *span)
{
    if (!user_ok(p, address, sizeof *span, 0))
    {
        return 0;
    }
    memcpy(span, (void *)(uintptr_t)address, sizeof *span);
    return 1;
}

static int64_t sd_syscall(struct frame *f)
{
    uint64_t a = f->rdi, b = f->rsi, c = f->rdx;
    unsigned pid = uthreads[current].process;
    struct task *p = &tasks[pid];
    switch (f->rax)
    {
        case SDK_LOAD_PROBE:
        {
            /* Author instrument: all failed admission points of the actual checked ELF. */
            const unsigned char *data;
            size_t size;
            struct cmk_elf elf;
            if (cmk_archive_find(archive, archive_size, "spawn", &data,
                                 &size) ||
                cmk_elf_parse(data, size, &elf))
            {
                return SDK_INVALID;
            }
            unsigned expected = 32;
            for (unsigned s = 0; s < elf.count; s++)
            {
                expected +=
                    (unsigned)(((elf.segments[s].address +
                                 elf.segments[s].memsz + 4095) &
                                ~UINT64_C(4095)) -
                               (elf.segments[s].address & ~UINT64_C(4095))) /
                    4096;
            }
            static struct cmk_frames old_frames;
            static uint16_t old_refs[CMK_MAX_FRAMES];
            for (unsigned cut = 0; cut < expected; cut++)
            {
                memcpy(&old_frames, &frames, sizeof frames);
                memcpy(old_refs, page_refs, sizeof old_refs);
                unsigned old_tasks = task_count, old_threads = thread_count;
                sd_fail_after = (int)cut;
                sd_allocations = 0;
                int r = load_task("spawn", 0);
                sd_fail_after = -1;
                if (r != -2 || task_count != old_tasks ||
                    thread_count != old_threads ||
                    memcmp(&old_frames, &frames, sizeof frames) ||
                    memcmp(old_refs, page_refs, sizeof old_refs))
                {
                    return SDK_INVALID;
                }
            }
            return expected;
        }
        case SDK_CLONE_PRIVATE:
        case SDK_CLONE_IMMUTABLE:
            /* New restricted operation; retained CMK2_FORK_SINGLE shared behavior is intact. */
            for (unsigned i = 0; i < 512; i++)
            {
                if ((p->pt[i] & PTE_SHARED) &&
                    (f->rax == SDK_CLONE_PRIVATE || (p->pt[i] & 2)))
                {
                    return SDK_DENIED;
                }
            }
            for (unsigned i = 0; i < 512; i++)
            {
                if (p->pt[i] & 1)
                {
                    unsigned n =
                        (unsigned)((p->pt[i] & UINT64_C(0x000ffffffffff000)) /
                                   4096);
                    if (n >= CMK_MAX_FRAMES || !page_refs[n] ||
                        page_refs[n] == 65535)
                    {
                        return SDK_FULL;
                    }
                }
            }
            return fork_single(f);
        case SDK_SHARE_IMMUTABLE:
        {
            if (a % 4096 || !user_ok(p, a, 4096, 0))
            {
                return SDK_INVALID;
            }
            unsigned i = (unsigned)((a - CMK_USER_BASE) / 4096);
            uint64_t e = p->pt[i];
            if (!(e & PTE_NX) || (e & PTE_COW))
            {
                return SDK_DENIED;
            }
            p->pt[i] = (e & ~UINT64_C(2)) | PTE_SHARED;
#ifdef SDK_MUTANT_IMMUTABLE
            p->pt[i] =
                (e & ~UINT64_C(2)) |
                PTE_COW; /* Incorrectly substitutes private COW for immutability. */
#endif
            cmk2_flush_remote();
            return 0;
        }
        case SDK_MUTATE:
        {
            if (sd_mutable_ready &&
                (sd_mutable_owner != pid ||
                 sd_mutable_owner_generation != p->generation))
            {
                return SDK_DENIED;
            }
            struct sdk_span span;
            if (!sd_span(p, a, &span) || span.length > 32 ||
                (span.length && !user_ok(p, span.pointer, span.length, 0)))
            {
                return SDK_COPY;
            }
            unsigned v;
            for (v = 0; v < 2; v++)
            {
                if (!sd_versions[v].refs)
                {
                    break;
                }
            }
            if (v == 2)
            {
                return SDK_FULL;
            }
            if (span.length)
            {
                memcpy(sd_versions[v].bytes, (void *)(uintptr_t)span.pointer,
                       (size_t)span.length);
            }
            sd_versions[v].length = (unsigned)span.length;
            sd_versions[v].refs = 1;
            if (sd_mutable_ready)
            {
                sd_versions[sd_current_version].refs--;
            }
            sd_current_version = v;
            sd_mutable_ready = 1;
            sd_mutable_owner = pid;
            sd_mutable_owner_generation = p->generation;
            return 0;
        }
        case SDK_OPEN_SNAPSHOT:
        case SDK_OPEN_LIVE:
        {
            if (!sd_mutable_ready)
            {
                return SDK_INVALID;
            }
            unsigned h, d;
            for (h = 0; h < 16; h++)
            {
                if (!sd_handles[h].open && sd_handles[h].generation < 65535)
                {
                    break;
                }
            }
            for (d = 0; d < 8; d++)
            {
                if (!sd_descriptions[d].refs)
                {
                    break;
                }
            }
            if (h == 16 || d == 8)
            {
                return SDK_FULL;
            }
            unsigned live = f->rax == SDK_OPEN_LIVE, v = sd_current_version;
#ifdef SDK_MUTANT_SNAPSHOT
            live = 1;
#endif
            sd_descriptions[d] = (struct sd_open_description){
                sd_versions[v].bytes, sd_versions[v].length, 0, 1, v, live};
            if (!live)
            {
                sd_versions[v].refs++;
            }
            sd_handles[h].generation++;
            sd_handles[h].open = 1;
            sd_handles[h].owners = 1u << pid;
            sd_handles[h].description = d;
            return cmk_handle(h, sd_handles[h].generation);
        }
        case SDK_OPEN:
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
            if (cmk_archive_find(archive, archive_size, name, &data, &length))
            {
                return SDK_INVALID;
            }
            unsigned h, d;
            for (h = 0; h < 16; h++)
            {
                if (!sd_handles[h].open && sd_handles[h].generation < 65535)
                {
                    break;
                }
            }
            for (d = 0; d < 8; d++)
            {
                if (!sd_descriptions[d].refs)
                {
                    break;
                }
            }
            if (h == 16 || d == 8)
            {
                return SDK_FULL;
            }
            sd_descriptions[d] =
                (struct sd_open_description){data, length, 0, 1, 2, 0};
            sd_handles[h].generation++;
            sd_handles[h].open = 1;
            sd_handles[h].owners = 1u << pid;
            sd_handles[h].description = d;
            return cmk_handle(h, sd_handles[h].generation);
        }
        case SDK_READ:
        {
            unsigned h = sd_file(a, pid);
            if (h == 16)
            {
                return SDK_STALE;
            }
            struct sdk_span span;
            if (!sd_span(p, b, &span) || span.length > 256)
            {
                return SDK_COPY;
            }
            struct sd_open_description *d =
                &sd_descriptions[sd_handles[h].description];
            const unsigned char *data =
                d->live ? sd_versions[sd_current_version].bytes : d->data;
            size_t length =
                d->live ? sd_versions[sd_current_version].length : d->length;
            size_t n = d->offset < length ? length - d->offset : 0;
            if (n > span.length)
            {
                n = (size_t)span.length;
            }
#ifdef SDK_MUTANT_OFFSET
            d->offset += n;
#endif
            if (n && !user_ok(p, span.pointer, n, 1))
            {
                return SDK_COPY;
            }
            if (n)
            {
                memcpy((void *)(uintptr_t)span.pointer, data + d->offset, n);
            }
#ifndef SDK_MUTANT_OFFSET
            d->offset += n;
#endif
            return (int64_t)n;
        }
        case SDK_SEEK:
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
            int64_t base = s.origin == 0
                               ? 0
                               : (int64_t)(s.origin == 1 ? d->offset : length);
            if ((s.offset > 0 && base > INT64_MAX - s.offset) ||
                (s.offset < 0 && base < INT64_MIN - s.offset))
            {
                return SDK_INVALID;
            }
            int64_t result = base + s.offset;
            if (result < 0 || (uint64_t)result > length)
            {
                return SDK_INVALID;
            }
            d->offset = (size_t)result;
            return result;
        }
        case SDK_DUP:
        {
            unsigned h = sd_file(a, pid);
            if (h == 16)
            {
                return SDK_STALE;
            }
            for (unsigned j = 0; j < 16; j++)
            {
                if (!sd_handles[j].open && sd_handles[j].generation < 65535)
                {
                    sd_handles[j].generation++;
                    sd_handles[j].open = 1;
                    sd_handles[j].owners = 1u << pid;
                    sd_handles[j].description = sd_handles[h].description;
                    sd_descriptions[sd_handles[j].description].refs++;
                    return cmk_handle(j, sd_handles[j].generation);
                }
            }
            return SDK_FULL;
        }
        case SDK_CLOSE:
        {
            unsigned h = sd_file(a, pid);
            if (h == 16)
            {
                return SDK_STALE;
            }
            sd_file_close(h, pid);
            return 0;
        }
        case SDK_CHANNEL:
            for (unsigned q = 0; q < 8; q++)
            {
                if (!sd_queues[q].open && sd_queues[q].generation < 65535)
                {
                    sd_queues[q].generation++;
                    sd_queues[q].open = 1;
                    sd_queues[q].owner = pid;
                    sd_queues[q].count = sd_queues[q].head = 0;
                    int64_t cap = sd_new_cap(pid, q, 7);
                    if (cap < 0)
                    {
                        sd_queues[q].open = 0;
                    }
                    return cap;
                }
            }
            return SDK_FULL;
        case SDK_ATTENUATE:
        {
            unsigned cap = sd_cap(a, pid, 0);
            if (cap == 32 || !b || b > 7 || (b & sd_caps[cap].rights) != b)
            {
                return SDK_DENIED;
            }
            return sd_new_cap(pid, sd_caps[cap].queue, (unsigned)b);
        }
        case SDK_SEND:
        case SDK_RECEIVE:
        case SDK_RECEIVE_PARTIAL:
        {
            unsigned sending = f->rax == SDK_SEND,
                     cap = sd_cap(a, pid, sending ? 1 : 2);
            if (cap == 32)
            {
                return SDK_DENIED;
            }
            if (c > 1)
            {
                return SDK_INVALID;
            }
            struct sdk_span span;
            if (!sd_span(p, b, &span) || span.length > 32)
            {
                return SDK_COPY;
            }
            unsigned q = sd_caps[cap].queue;
            struct sd_queue *queue = &sd_queues[q];
            if (sending)
            {
                if (queue->open != 1)
                {
                    return SDK_CLOSED;
                }
                struct sd_message m = {0};
                m.n = (unsigned)span.length;
                m.reader = THREADS;
                if (m.n && !user_ok(p, span.pointer, m.n, 0))
                {
                    return SDK_COPY;
                }
                if (m.n)
                {
                    memcpy(m.bytes, (void *)(uintptr_t)span.pointer, m.n);
                }
                if (queue->count == 2)
                {
                    if (c)
                    {
                        return SDK_FULL;
                    }
                    sd_waiters[current] =
                        (struct sd_pending){1, q, queue->generation, 1, m};
                    uthreads[current].state = UT_SD_WAIT;
                    return 0;
                }
                queue->message[(queue->head + queue->count) % 2] = m;
                queue->count++;
                sd_wake(q);
                return 0;
            }
            if (!queue->count)
            {
                if (queue->open != 1)
                {
                    return SDK_CLOSED;
                }
                if (c)
                {
                    return SDK_FULL;
                }
                sd_waiters[current].active = 1;
                sd_waiters[current].queue = q;
                sd_waiters[current].incarnation = queue->generation;
                sd_waiters[current].sending = 0;
                uthreads[current].state = UT_SD_WAIT;
                return 0;
            }
            struct sd_message *m = &queue->message[queue->head];
            unsigned n = m->n - m->consumed;
            if (m->reader != THREADS && m->reader != current)
            {
                return SDK_DENIED;
            }
            if (f->rax == SDK_RECEIVE_PARTIAL)
            {
                if (n && !span.length)
                {
                    return SDK_FULL;
                }
                if (n > span.length)
                {
                    n = (unsigned)span.length;
                }
            }
            else if (m->consumed || span.length < n)
            {
                return SDK_FULL;
            }
            if (n && !user_ok(p, span.pointer, n, 1))
            {
                return SDK_COPY;
            }
            if (n)
            {
                memcpy((void *)(uintptr_t)span.pointer, m->bytes + m->consumed,
                       n);
            }
#ifdef SDK_MUTANT_PARTIAL
            if (f->rax != SDK_RECEIVE_PARTIAL)
            {
                m->consumed += n;
            }
#else
            m->consumed += n;
#endif
            m->reader = current;
            unsigned ended = m->consumed == m->n;
            if (ended)
            {
                queue->head = (queue->head + 1) % 2;
                queue->count--;
                sd_wake(q);
            }
            return n + ((f->rax == SDK_RECEIVE_PARTIAL && ended)
                            ? SDK_PARTIAL_END
                            : 0);
        }
        case SDK_CHANNEL_CLOSE:
        {
            unsigned cap = sd_cap(a, pid, 4);
            if (cap == 32)
            {
                return SDK_DENIED;
            }
            sd_close_queue(sd_caps[cap].queue);
            return 0;
        }
        case SDK_CHANNEL_DESTROY:
        {
            unsigned cap = sd_cap(a, pid, 4);
            if (cap == 32)
            {
                return SDK_DENIED;
            }
            unsigned q = sd_caps[cap].queue;
            if (sd_queues[q].open != 2 || sd_queues[q].count)
            {
                return SDK_INVALID;
            }
            for (unsigned i = 0; i < THREADS; i++)
            {
                if (sd_waiters[i].active && sd_waiters[i].queue == q)
                {
                    return SDK_INVALID;
                }
            }
            sd_queues[q].open = 0;
            for (unsigned i = 0; i < 32; i++)
            {
                if (sd_caps[i].queue == q)
                {
                    sd_caps[i].open = 0;
                }
            }
            return 0;
        }
        case SDK_INCARNATION:
        {
            unsigned cap = sd_cap(a, pid, 4);
            return cap == 32
                       ? SDK_DENIED
                       : (int64_t)(((uint64_t)sd_queues[sd_caps[cap].queue]
                                        .generation
                                    << 32) |
                                   sd_caps[cap].queue);
        }
        case SDK_DEFERRED_WAKE:
        {
            unsigned q = (unsigned)a;
            if (q >= 8 || sd_queues[q].generation != (unsigned)(a >> 32) ||
                !sd_queues[q].open)
            {
                return SDK_STALE;
            }
            sd_wake(q);
            return 0;
        }
        case SDK_CANCEL:
        case SDK_BLOCKED:
        {
            unsigned tid = (unsigned)(a & 65535);
            if (a > UINT32_MAX || !tid || --tid >= thread_count ||
                uthreads[tid].process != pid ||
                !cmk_handle_valid((uint32_t)a, tid, uthreads[tid].generation,
                                  1))
            {
                return SDK_STALE;
            }
            if (f->rax == SDK_BLOCKED)
            {
                return sd_waiters[tid].active;
            }
            if (!sd_waiters[tid].active)
            {
                return SDK_INVALID;
            }
#ifndef SDK_MUTANT_CANCEL
            sd_waiters[tid].active = 0;
#endif
            uthreads[tid].frame->rax = (uint64_t)(int64_t)SDK_CANCELLED;
            uthreads[tid].state = UT_READY;
            return 0;
        }
        case SDK_CUT:
            if (!p->handle_open || block(0, 0))
            {
                return SDK_DENIED;
            }
            return (uint32_t)bounce[0] | (uint32_t)bounce[1] << 8 |
                   (uint32_t)bounce[2] << 16 | (uint32_t)bounce[3] << 24;
        case SDK_CRASH:
            stop(1);
        default:
            return SDK_INVALID;
    }
}

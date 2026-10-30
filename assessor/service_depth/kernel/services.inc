#include "kernel_abi.h"

/* Reference service syscalls (SDK_*) added to the retained teaching kernel.
 * This file is spliced into kernel.c, so kernel types and helpers (struct
 * task, uthreads, user_ok, cmk_handle, ...) are in scope. Every entry point
 * is entered with the kernel metadata lock held, and every user pointer is
 * validated with user_ok() and copied before use.
 *
 * Two services live here: reference-counted file handles over a table of
 * shared open descriptions, and capability-checked message queues. Handles
 * and capabilities returned to user code are opaque tokens; the rights and
 * the owner live only in the kernel-private tables below, never in token
 * bits. SDK_MUTANT_* macros each switch on one deliberate defect for the
 * mutation campaign. */

enum
{
    SD_FILE_HANDLES = 16,
    SD_DESCRIPTIONS = 8,
    SD_VERSION_SLOTS = 2,
    /* A description backed by an archive file, not by a mutable version. */
    SD_ARCHIVE_BACKED = SD_VERSION_SLOTS,
    SD_QUEUES = 8,
    SD_CAPABILITIES = 32,
    SD_QUEUE_SLOTS = 2,
    SD_MESSAGE_BYTES = 32,
    SD_VERSION_BYTES = 32,
    SD_READ_BYTES_MAX = 256,
    SD_NAME_BYTES_MAX = 23,
    /* Slot generations stop at the limit, so a token is never reissued. */
    SD_GENERATION_LIMIT = 65535,
    /* "None found" answers are one past the last valid index. */
    SD_NO_FILE = SD_FILE_HANDLES,
    SD_NO_CAPABILITY = SD_CAPABILITIES,
    /* struct sd_queue.open states */
    SD_QUEUE_FREE = 0,
    SD_QUEUE_OPEN = 1,
    SD_QUEUE_CLOSED = 2,
    SD_CAP_SEND = SDK_SEND_RIGHT,
    SD_CAP_RECEIVE = SDK_RECEIVE_RIGHT,
    SD_CAP_CLOSE = SDK_CLOSE_RIGHT,
    SD_CAP_ALL = SD_CAP_SEND | SD_CAP_RECEIVE | SD_CAP_CLOSE,
    SD_PAGE_SIZE = 4096
};

/* ---- File handles over shared open descriptions ------------------------- */

/* One open file description: what is open and the offset every duplicate of
 * the handle shares. `version` selects mutable snapshot storage (0..1) or
 * SD_ARCHIVE_BACKED; `live` reads the current version instead of a snapshot. */
struct sd_open_description
{
    const unsigned char *data;
    size_t length;
    size_t offset;
    unsigned refs;
    unsigned version;
    unsigned live;
};

/* `owners` is a bit set of process ids sharing this handle after fork. */
struct sd_file_handle
{
    unsigned generation;
    unsigned owners;
    unsigned description;
    unsigned open;
};
static struct sd_open_description sd_descriptions[SD_DESCRIPTIONS];
static struct sd_file_handle sd_handles[SD_FILE_HANDLES];

struct sd_version
{
    unsigned refs;
    unsigned length;
    unsigned char bytes[SD_VERSION_BYTES];
};
static struct sd_version sd_versions[SD_VERSION_SLOTS];
static unsigned sd_current_version;
static unsigned sd_mutable_ready;
static unsigned sd_mutable_owner;
static unsigned sd_mutable_owner_generation;

/* Index of the handle `token` names for process `pid`, or SD_NO_FILE. */
static unsigned sd_file(uint64_t token, unsigned pid)
{
    for (unsigned i = 0; i < SD_FILE_HANDLES; i++)
    {
        if (sd_handles[i].open && (sd_handles[i].owners & (1u << pid)) &&
            token <= UINT32_MAX &&
            cmk_handle_valid((uint32_t)token, i, sd_handles[i].generation, 1))
        {
            return i;
        }
    }
    return SD_NO_FILE;
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
    if (!d->refs && d->version < SD_VERSION_SLOTS && !d->live)
    {
        sd_versions[d->version].refs--;
    }
}

/* A forked child shares every parent handle, and with it the file offset. */
static void sd_fork_files(unsigned parent, unsigned child)
{
#ifdef SDK_MUTANT_FILE_FORK
    (void)parent;
    (void)child;
    return;
#endif
    for (unsigned h = 0; h < SD_FILE_HANDLES; h++)
    {
        if (sd_handles[h].open && (sd_handles[h].owners & (1u << parent)))
        {
            sd_handles[h].owners |= 1u << child;
            sd_descriptions[sd_handles[h].description].refs++;
        }
    }
}

/* ---- Capability-checked message queues ---------------------------------- */

/* `consumed` counts bytes already delivered by partial receives; `reader` is
 * the one thread allowed to continue a partially read message. */
struct sd_message
{
    unsigned n;
    unsigned consumed;
    unsigned reader;
    unsigned char bytes[SD_MESSAGE_BYTES];
};

struct sd_queue
{
    unsigned open;
    unsigned owner;
    unsigned count;
    unsigned head;
    unsigned generation;
    struct sd_message message[SD_QUEUE_SLOTS];
};

/* Kernel-side rights table entry: `queue` plus `incarnation` identify the
 * queue it was granted on; a recycled queue invalidates old capabilities. */
struct sd_capability
{
    unsigned open;
    unsigned owner;
    unsigned generation;
    unsigned queue;
    unsigned incarnation;
    unsigned rights;
};
static struct sd_queue sd_queues[SD_QUEUES];
static struct sd_capability sd_caps[SD_CAPABILITIES];

/* One pending syscall per thread, with a kernel-owned payload on send.
 * Wake returns RETRY. A receiver never leaves a user pointer in the wait table. */
struct sd_pending
{
    unsigned active;
    unsigned queue;
    unsigned incarnation;
    unsigned sending;
    struct sd_message message;
};
static struct sd_pending sd_waiters[THREADS];

static void sd_thread_cleanup(unsigned tid)
{
    sd_waiters[tid].active = 0;
    for (unsigned q = 0; q < SD_QUEUES; q++)
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

/* Index of the capability `token` names for `pid`, provided its queue is
 * still the one it was granted on and it holds every bit of `rights`;
 * otherwise SD_NO_CAPABILITY. */
static unsigned sd_cap(uint64_t token, unsigned pid, unsigned rights)
{
    if (token > UINT32_MAX)
    {
        return SD_NO_CAPABILITY;
    }
    for (unsigned i = 0; i < SD_CAPABILITIES; i++)
    {
        struct sd_capability *c = &sd_caps[i];
        if (c->open && c->owner == pid &&
            cmk_handle_valid((uint32_t)token, i, c->generation, 1) &&
            c->queue < SD_QUEUES && sd_queues[c->queue].open &&
            sd_queues[c->queue].generation == c->incarnation)
        {
#ifndef SDK_MUTANT_RIGHTS
            if ((c->rights & rights) != rights)
            {
                return SD_NO_CAPABILITY;
            }
#else
            (void)rights;
#endif
            return i;
        }
    }
    return SD_NO_CAPABILITY;
}

static int64_t sd_new_cap(unsigned pid, unsigned q, unsigned rights)
{
    for (unsigned i = 0; i < SD_CAPABILITIES; i++)
    {
        if (!sd_caps[i].open && sd_caps[i].generation < SD_GENERATION_LIMIT)
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

/* Completes one blocked thread of queue `q` if its condition now holds. */
static void sd_wake_one(unsigned i, unsigned q)
{
    if (sd_waiters[i].incarnation != sd_queues[q].generation)
    {
        sd_waiters[i].active = 0;
        uthreads[i].frame->rax = (uint64_t)(int64_t)SDK_STALE;
        uthreads[i].state = UT_READY;
        return;
    }
    if (sd_waiters[i].sending)
    {
        if (sd_queues[q].open == SD_QUEUE_OPEN &&
            sd_queues[q].count == SD_QUEUE_SLOTS)
        {
            /* Still full: stay blocked. */
            return;
        }
        if (sd_queues[q].open == SD_QUEUE_OPEN)
        {
            unsigned tail =
                (sd_queues[q].head + sd_queues[q].count) % SD_QUEUE_SLOTS;
            sd_queues[q].message[tail] = sd_waiters[i].message;
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
        /* A blocked receiver is told to retry; it copies out itself. */
        uthreads[i].frame->rax = SDK_RETRY;
    }
    sd_waiters[i].active = 0;
    uthreads[i].state = UT_READY;
}

static void sd_wake(unsigned q)
{
    for (unsigned i = 0; i < thread_count; i++)
    {
        if (sd_waiters[i].active && sd_waiters[i].queue == q)
        {
            sd_wake_one(i, q);
        }
    }
}

static void sd_close_queue(unsigned q)
{
    sd_queues[q].open = SD_QUEUE_CLOSED;
    sd_wake(q);
}

static void sd_process_cleanup(unsigned pid)
{
    for (unsigned h = 0; h < SD_FILE_HANDLES; h++)
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
    for (unsigned q = 0; q < SD_QUEUES; q++)
    {
        if (sd_queues[q].open && sd_queues[q].owner == pid)
        {
            sd_close_queue(q);
            /* This profile grants only within the owning process; no surviving grantee. */
            sd_queues[q].open = SD_QUEUE_FREE;
        }
    }
    for (unsigned i = 0; i < SD_CAPABILITIES; i++)
    {
        if (sd_caps[i].owner == pid)
        {
            sd_caps[i].open = 0;
        }
    }
}

/* ---- Syscall handlers ---------------------------------------------------- */

/* Arguments of one SDK_* call. a, b, c are the raw user registers. */
struct sd_call
{
    struct frame *frame;
    struct task *task;
    unsigned pid;
    uint64_t a;
    uint64_t b;
    uint64_t c;
};

/* Copies a user (pointer, length) pair; returns 0 if it is not readable. */
static int sd_span(struct task *p, uint64_t address, struct sdk_span *span)
{
    if (!user_ok(p, address, sizeof *span, 0))
    {
        return 0;
    }
    memcpy(span, (void *)(uintptr_t)address, sizeof *span);
    return 1;
}

/* Author instrument: injects a failure at every allocation point of the
 * actual checked ELF load and requires the allocator, page references and
 * task counts to come back unchanged each time. Returns the number of
 * injection points, or SDK_INVALID if any rollback left a trace. */
static int64_t sd_op_load_probe(void)
{
    const unsigned char *data;
    size_t size;
    struct cmk_elf elf;
    if (cmk_archive_find(archive, archive_size, "spawn", &data, &size) ||
        cmk_elf_parse(data, size, &elf))
    {
        return SDK_INVALID;
    }
    /* 32 stack pages plus every page the image segments cover. */
    unsigned expected = 32;
    for (unsigned s = 0; s < elf.count; s++)
    {
        uint64_t first = elf.segments[s].address & ~UINT64_C(4095);
        uint64_t end =
            (elf.segments[s].address + elf.segments[s].memsz + UINT64_C(4095)) &
            ~UINT64_C(4095);
        expected += (unsigned)(end - first) / SD_PAGE_SIZE;
    }
    static struct cmk_frames old_frames;
    static uint16_t old_refs[CMK_MAX_FRAMES];
    for (unsigned cut = 0; cut < expected; cut++)
    {
        memcpy(&old_frames, &frames, sizeof frames);
        memcpy(old_refs, page_refs, sizeof old_refs);
        unsigned old_tasks = task_count;
        unsigned old_threads = thread_count;
        sd_fail_after = (int)cut;
        sd_allocations = 0;
        int r = load_task("spawn", 0);
        sd_fail_after = -1;
        if (r != -2 || task_count != old_tasks || thread_count != old_threads ||
            memcmp(&old_frames, &frames, sizeof frames) ||
            memcmp(old_refs, page_refs, sizeof old_refs))
        {
            return SDK_INVALID;
        }
    }
    return expected;
}

/* New restricted fork: private clone needs no shared page at all; the
 * immutable variant tolerates shared pages only if they are read-only.
 * Retained CMK2_FORK_SINGLE shared behavior is intact. */
static int64_t sd_op_clone(const struct sd_call *call)
{
    struct task *p = call->task;
    for (unsigned i = 0; i < 512; i++)
    {
        if ((p->pt[i] & PTE_SHARED) &&
            (call->frame->rax == SDK_CLONE_PRIVATE || (p->pt[i] & 2)))
        {
            return SDK_DENIED;
        }
    }
    for (unsigned i = 0; i < 512; i++)
    {
        if (p->pt[i] & 1)
        {
            unsigned n = (unsigned)((p->pt[i] & UINT64_C(0x000ffffffffff000)) /
                                    SD_PAGE_SIZE);
            if (n >= CMK_MAX_FRAMES || !page_refs[n] || page_refs[n] == 65535)
            {
                return SDK_FULL;
            }
        }
    }
    return fork_single(call->frame);
}

/* Marks an immutable data page as shared read-only. Only a page the caller
 * maps non-executable and not yet copy-on-write qualifies. */
static int64_t sd_op_share_immutable(const struct sd_call *call)
{
    struct task *p = call->task;
    uint64_t address = call->a;
    if (address % SD_PAGE_SIZE || !user_ok(p, address, SD_PAGE_SIZE, 0))
    {
        return SDK_INVALID;
    }
    unsigned i = (unsigned)((address - CMK_USER_BASE) / SD_PAGE_SIZE);
    uint64_t entry = p->pt[i];
    if (!(entry & PTE_NX) || (entry & PTE_COW))
    {
        return SDK_DENIED;
    }
    p->pt[i] = (entry & ~UINT64_C(2)) | PTE_SHARED;
#ifdef SDK_MUTANT_IMMUTABLE
    p->pt[i] =
        (entry & ~UINT64_C(2)) |
        PTE_COW; /* Incorrectly substitutes private COW for immutability. */
#endif
    cmk2_flush_remote();
    return 0;
}

/* Lowest mutable-version slot nobody references, or SD_VERSION_SLOTS. */
static unsigned sd_free_version(void)
{
    unsigned v = 0;
    while (v < SD_VERSION_SLOTS && sd_versions[v].refs)
    {
        v++;
    }
    return v;
}

/* Publishes a new version of the process-owned mutable byte string. Readers
 * that opened a snapshot keep the old version alive through its refs. */
static int64_t sd_op_mutate(const struct sd_call *call)
{
    struct task *p = call->task;
    if (sd_mutable_ready && (sd_mutable_owner != call->pid ||
                             sd_mutable_owner_generation != p->generation))
    {
        return SDK_DENIED;
    }
    struct sdk_span span;
    if (!sd_span(p, call->a, &span) || span.length > SD_VERSION_BYTES ||
        (span.length && !user_ok(p, span.pointer, span.length, 0)))
    {
        return SDK_COPY;
    }
    unsigned v = sd_free_version();
    if (v == SD_VERSION_SLOTS)
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
    sd_mutable_owner = call->pid;
    sd_mutable_owner_generation = p->generation;
    return 0;
}

/* Lowest handle slot that can still issue a fresh generation. */
static unsigned sd_free_handle_slot(void)
{
    for (unsigned h = 0; h < SD_FILE_HANDLES; h++)
    {
        if (!sd_handles[h].open &&
            sd_handles[h].generation < SD_GENERATION_LIMIT)
        {
            return h;
        }
    }
    return SD_NO_FILE;
}

/* Reserves one handle slot and one description slot for a new open file.
 * Returns 0, or SDK_FULL (nothing reserved). */
static int64_t sd_reserve_file_slots(unsigned *handle, unsigned *description)
{
    unsigned h = sd_free_handle_slot();
    unsigned d = 0;
    while (d < SD_DESCRIPTIONS && sd_descriptions[d].refs)
    {
        d++;
    }
    if (h == SD_NO_FILE || d == SD_DESCRIPTIONS)
    {
        return SDK_FULL;
    }
    *handle = h;
    *description = d;
    return 0;
}

/* Makes slot `h` a new handle owned by `pid` on description `d`. */
static int64_t sd_install_handle(unsigned h, unsigned d, unsigned pid)
{
    sd_handles[h].generation++;
    sd_handles[h].open = 1;
    sd_handles[h].owners = 1u << pid;
    sd_handles[h].description = d;
    return cmk_handle(h, sd_handles[h].generation);
}

/* Opens the mutable string: a snapshot pins the version current now, a live
 * handle always reads whichever version is current at read time. */
static int64_t sd_op_open_version(const struct sd_call *call)
{
    if (!sd_mutable_ready)
    {
        return SDK_INVALID;
    }
    unsigned h;
    unsigned d;
    int64_t reserved = sd_reserve_file_slots(&h, &d);
    if (reserved)
    {
        return reserved;
    }
    unsigned live = call->frame->rax == SDK_OPEN_LIVE;
    unsigned v = sd_current_version;
#ifdef SDK_MUTANT_SNAPSHOT
    live = 1;
#endif
    sd_descriptions[d] = (struct sd_open_description){
        sd_versions[v].bytes, sd_versions[v].length, 0, 1, v, live};
    if (!live)
    {
        sd_versions[v].refs++;
    }
    return sd_install_handle(h, d, call->pid);
}

/* Opens an archive file by name; the name is copied from user space first. */
static int64_t sd_op_open_archive(const struct sd_call *call)
{
    uint64_t name_address = call->a;
    uint64_t name_bytes = call->b;
    if (!name_bytes || name_bytes > SD_NAME_BYTES_MAX ||
        !user_ok(call->task, name_address, name_bytes, 0))
    {
        return SDK_COPY;
    }
    char name[SD_NAME_BYTES_MAX + 1];
    memcpy(name, (void *)(uintptr_t)name_address, (size_t)name_bytes);
    name[name_bytes] = 0;
    const unsigned char *data;
    size_t length;
    if (cmk_archive_find(archive, archive_size, name, &data, &length))
    {
        return SDK_INVALID;
    }
    unsigned h;
    unsigned d;
    int64_t reserved = sd_reserve_file_slots(&h, &d);
    if (reserved)
    {
        return reserved;
    }
    sd_descriptions[d] =
        (struct sd_open_description){data, length, 0, 1, SD_ARCHIVE_BACKED, 0};
    return sd_install_handle(h, d, call->pid);
}

/* Bytes and length a description currently reads from. */
static const unsigned char *
sd_description_bytes(const struct sd_open_description *d)
{
    return d->live ? sd_versions[sd_current_version].bytes : d->data;
}

static size_t sd_description_length(const struct sd_open_description *d)
{
    return d->live ? sd_versions[sd_current_version].length : d->length;
}

/* Copies up to span.length bytes at the shared offset to user space. A
 * rejected copy must leave the offset alone (SDK_MUTANT_OFFSET advances it
 * first). Returns the byte count or a negative SDK_* status. */
static int64_t sd_op_read(const struct sd_call *call)
{
    struct task *p = call->task;
    unsigned h = sd_file(call->a, call->pid);
    if (h == SD_NO_FILE)
    {
        return SDK_STALE;
    }
    struct sdk_span span;
    if (!sd_span(p, call->b, &span) || span.length > SD_READ_BYTES_MAX)
    {
        return SDK_COPY;
    }
    struct sd_open_description *d = &sd_descriptions[sd_handles[h].description];
    const unsigned char *data = sd_description_bytes(d);
    size_t length = sd_description_length(d);
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

static int64_t sd_op_seek(const struct sd_call *call)
{
    unsigned h = sd_file(call->a, call->pid);
    if (h == SD_NO_FILE)
    {
        return SDK_STALE;
    }
    struct sdk_seek request;
    if (!user_ok(call->task, call->b, sizeof request, 0))
    {
        return SDK_COPY;
    }
    memcpy(&request, (void *)(uintptr_t)call->b, sizeof request);
    if (request.origin > 2 || request.reserved)
    {
        return SDK_INVALID;
    }
    struct sd_open_description *d = &sd_descriptions[sd_handles[h].description];
    size_t length = sd_description_length(d);
    int64_t base = 0;
    if (request.origin == 1)
    {
        base = (int64_t)d->offset;
    }
    else if (request.origin == 2)
    {
        base = (int64_t)length;
    }
    /* Reject a sum that would overflow before computing it. */
    if ((request.offset > 0 && base > INT64_MAX - request.offset) ||
        (request.offset < 0 && base < INT64_MIN - request.offset))
    {
        return SDK_INVALID;
    }
    int64_t result = base + request.offset;
    if (result < 0 || (uint64_t)result > length)
    {
        return SDK_INVALID;
    }
    d->offset = (size_t)result;
    return result;
}

static int64_t sd_op_dup(const struct sd_call *call)
{
    unsigned h = sd_file(call->a, call->pid);
    if (h == SD_NO_FILE)
    {
        return SDK_STALE;
    }
    unsigned slot = sd_free_handle_slot();
    if (slot == SD_NO_FILE)
    {
        return SDK_FULL;
    }
    unsigned d = sd_handles[h].description;
    sd_descriptions[d].refs++;
    return sd_install_handle(slot, d, call->pid);
}

static int64_t sd_op_close(const struct sd_call *call)
{
    unsigned h = sd_file(call->a, call->pid);
    if (h == SD_NO_FILE)
    {
        return SDK_STALE;
    }
    sd_file_close(h, call->pid);
    return 0;
}

/* Creates a queue and returns an all-rights capability for it. */
static int64_t sd_op_channel(const struct sd_call *call)
{
    for (unsigned q = 0; q < SD_QUEUES; q++)
    {
        if (!sd_queues[q].open && sd_queues[q].generation < SD_GENERATION_LIMIT)
        {
            sd_queues[q].generation++;
            sd_queues[q].open = SD_QUEUE_OPEN;
            sd_queues[q].owner = call->pid;
            sd_queues[q].count = 0;
            sd_queues[q].head = 0;
            int64_t cap = sd_new_cap(call->pid, q, SD_CAP_ALL);
            if (cap < 0)
            {
                sd_queues[q].open = SD_QUEUE_FREE;
            }
            return cap;
        }
    }
    return SDK_FULL;
}

/* A new capability may only hold a nonempty subset of the source's rights. */
static int64_t sd_op_attenuate(const struct sd_call *call)
{
    unsigned cap = sd_cap(call->a, call->pid, 0);
    uint64_t wanted = call->b;
    if (cap == SD_NO_CAPABILITY || !wanted || wanted > SD_CAP_ALL ||
        (wanted & sd_caps[cap].rights) != wanted)
    {
        return SDK_DENIED;
    }
    return sd_new_cap(call->pid, sd_caps[cap].queue, (unsigned)wanted);
}

/* Send: copies the payload into a kernel message first. A full queue blocks
 * the thread (c == 0) with that kernel-owned copy, or returns SDK_FULL
 * (c == 1, non-blocking). */
static int64_t sd_queue_send(const struct sd_call *call, unsigned q,
                             const struct sdk_span *span)
{
    struct sd_queue *queue = &sd_queues[q];
    if (queue->open != SD_QUEUE_OPEN)
    {
        return SDK_CLOSED;
    }
    struct sd_message m = {0};
    m.n = (unsigned)span->length;
    m.reader = THREADS;
    if (m.n && !user_ok(call->task, span->pointer, m.n, 0))
    {
        return SDK_COPY;
    }
    if (m.n)
    {
        memcpy(m.bytes, (void *)(uintptr_t)span->pointer, m.n);
    }
    if (queue->count == SD_QUEUE_SLOTS)
    {
        if (call->c)
        {
            return SDK_FULL;
        }
        sd_waiters[current] =
            (struct sd_pending){1, q, queue->generation, 1, m};
        uthreads[current].state = UT_SD_WAIT;
        return 0;
    }
    queue->message[(queue->head + queue->count) % SD_QUEUE_SLOTS] = m;
    queue->count++;
    sd_wake(q);
    return 0;
}

/* Receive and partial receive. Whole receive needs the entire message to fit;
 * partial receive delivers what fits and reports the end with
 * SDK_PARTIAL_END, remembering the reader so no other thread can take the
 * rest. An empty open queue blocks (c == 0) or returns SDK_FULL (c == 1). */
static int64_t sd_queue_receive(const struct sd_call *call, unsigned q,
                                const struct sdk_span *span)
{
    struct sd_queue *queue = &sd_queues[q];
    int partial = call->frame->rax == SDK_RECEIVE_PARTIAL;
    if (!queue->count)
    {
        if (queue->open != SD_QUEUE_OPEN)
        {
            return SDK_CLOSED;
        }
        if (call->c)
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
    if (partial)
    {
        if (n && !span->length)
        {
            return SDK_FULL;
        }
        if (n > span->length)
        {
            n = (unsigned)span->length;
        }
    }
    else if (m->consumed || span->length < n)
    {
        return SDK_FULL;
    }
    if (n && !user_ok(call->task, span->pointer, n, 1))
    {
        return SDK_COPY;
    }
    if (n)
    {
        memcpy((void *)(uintptr_t)span->pointer, m->bytes + m->consumed, n);
    }
#ifdef SDK_MUTANT_PARTIAL
    if (!partial)
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
        queue->head = (queue->head + 1) % SD_QUEUE_SLOTS;
        queue->count--;
        sd_wake(q);
    }
    /* The syscall number is deliberately re-read from the frame AFTER
     * sd_wake(): under SDK_MUTANT_CANCEL a cancelled thread stays active and
     * sd_wake() may overwrite this frame's rax, which the reference
     * semantics observe. Do not replace this with the earlier `partial`. */
    int partial_after_wake = call->frame->rax == SDK_RECEIVE_PARTIAL;
    return n + ((partial_after_wake && ended) ? SDK_PARTIAL_END : 0);
}

/* Shared front end of SDK_SEND, SDK_RECEIVE and SDK_RECEIVE_PARTIAL. */
static int64_t sd_op_transfer(const struct sd_call *call)
{
    int sending = call->frame->rax == SDK_SEND;
    unsigned cap =
        sd_cap(call->a, call->pid, sending ? SD_CAP_SEND : SD_CAP_RECEIVE);
    if (cap == SD_NO_CAPABILITY)
    {
        return SDK_DENIED;
    }
    if (call->c > 1)
    {
        return SDK_INVALID;
    }
    struct sdk_span span;
    if (!sd_span(call->task, call->b, &span) || span.length > SD_MESSAGE_BYTES)
    {
        return SDK_COPY;
    }
    unsigned q = sd_caps[cap].queue;
    return sending ? sd_queue_send(call, q, &span)
                   : sd_queue_receive(call, q, &span);
}

static int64_t sd_op_channel_close(const struct sd_call *call)
{
    unsigned cap = sd_cap(call->a, call->pid, SD_CAP_CLOSE);
    if (cap == SD_NO_CAPABILITY)
    {
        return SDK_DENIED;
    }
    sd_close_queue(sd_caps[cap].queue);
    return 0;
}

/* Frees a closed, drained queue with no blocked thread, and every capability
 * naming it. */
static int64_t sd_op_channel_destroy(const struct sd_call *call)
{
    unsigned cap = sd_cap(call->a, call->pid, SD_CAP_CLOSE);
    if (cap == SD_NO_CAPABILITY)
    {
        return SDK_DENIED;
    }
    unsigned q = sd_caps[cap].queue;
    if (sd_queues[q].open != SD_QUEUE_CLOSED || sd_queues[q].count)
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
    sd_queues[q].open = SD_QUEUE_FREE;
    for (unsigned i = 0; i < SD_CAPABILITIES; i++)
    {
        if (sd_caps[i].queue == q)
        {
            sd_caps[i].open = 0;
        }
    }
    return 0;
}

/* Author instrument: (generation << 32) | queue index of a capability's queue. */
static int64_t sd_op_incarnation(const struct sd_call *call)
{
    unsigned cap = sd_cap(call->a, call->pid, SD_CAP_CLOSE);
    if (cap == SD_NO_CAPABILITY)
    {
        return SDK_DENIED;
    }
    unsigned q = sd_caps[cap].queue;
    return (int64_t)(((uint64_t)sd_queues[q].generation << 32) | q);
}

/* Author instrument: wakes a queue's waiters on request. The argument is the
 * incarnation word from sd_op_incarnation(). */
static int64_t sd_op_deferred_wake(const struct sd_call *call)
{
    unsigned q = (unsigned)call->a;
    if (q >= SD_QUEUES ||
        sd_queues[q].generation != (unsigned)(call->a >> 32) ||
        !sd_queues[q].open)
    {
        return SDK_STALE;
    }
    sd_wake(q);
    return 0;
}

/* SDK_BLOCKED reports whether a sibling thread waits; SDK_CANCEL ends that
 * wait with SDK_CANCELLED. The token packs (thread index + 1) in the low 16
 * bits with the thread generation. */
static int64_t sd_op_cancel_or_blocked(const struct sd_call *call)
{
    uint64_t token = call->a;
    unsigned tid = (unsigned)(token & 65535);
    if (token > UINT32_MAX || !tid || --tid >= thread_count ||
        uthreads[tid].process != call->pid ||
        !cmk_handle_valid((uint32_t)token, tid, uthreads[tid].generation, 1))
    {
        return SDK_STALE;
    }
    if (call->frame->rax == SDK_BLOCKED)
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

/* Author instrument: reads the first four little-endian bytes of disk block
 * 0, for processes that were granted the disk. */
static int64_t sd_op_cut(const struct sd_call *call)
{
    if (!call->task->handle_open || block(0, 0))
    {
        return SDK_DENIED;
    }
    return (uint32_t)bounce[0] | (uint32_t)bounce[1] << 8 |
           (uint32_t)bounce[2] << 16 | (uint32_t)bounce[3] << 24;
}

/* Entry point for every SDK_* number (>= SDK_LOAD_PROBE). */
static int64_t sd_syscall(struct frame *f)
{
    unsigned pid = uthreads[current].process;
    struct sd_call call = {f, &tasks[pid], pid, f->rdi, f->rsi, f->rdx};
    switch (f->rax)
    {
        case SDK_LOAD_PROBE:
            return sd_op_load_probe();
        case SDK_CLONE_PRIVATE:
        case SDK_CLONE_IMMUTABLE:
            return sd_op_clone(&call);
        case SDK_SHARE_IMMUTABLE:
            return sd_op_share_immutable(&call);
        case SDK_MUTATE:
            return sd_op_mutate(&call);
        case SDK_OPEN_SNAPSHOT:
        case SDK_OPEN_LIVE:
            return sd_op_open_version(&call);
        case SDK_OPEN:
            return sd_op_open_archive(&call);
        case SDK_READ:
            return sd_op_read(&call);
        case SDK_SEEK:
            return sd_op_seek(&call);
        case SDK_DUP:
            return sd_op_dup(&call);
        case SDK_CLOSE:
            return sd_op_close(&call);
        case SDK_CHANNEL:
            return sd_op_channel(&call);
        case SDK_ATTENUATE:
            return sd_op_attenuate(&call);
        case SDK_SEND:
        case SDK_RECEIVE:
        case SDK_RECEIVE_PARTIAL:
            return sd_op_transfer(&call);
        case SDK_CHANNEL_CLOSE:
            return sd_op_channel_close(&call);
        case SDK_CHANNEL_DESTROY:
            return sd_op_channel_destroy(&call);
        case SDK_INCARNATION:
            return sd_op_incarnation(&call);
        case SDK_DEFERRED_WAKE:
            return sd_op_deferred_wake(&call);
        case SDK_CANCEL:
        case SDK_BLOCKED:
            return sd_op_cancel_or_blocked(&call);
        case SDK_CUT:
            return sd_op_cut(&call);
        case SDK_CRASH:
            stop(1);
        default:
            return SDK_INVALID;
    }
}

#include "service_depth.h"

/* Reference implementation of the four service cores in service_depth.h:
 * frame ownership, a capability channel, read-only file handles and a
 * two-slot receipt journal. Contracts: labs/service_depth/CONTRACTS.md.
 *
 * The SD_MUTATE_* macros select one deliberate defect each for the mutation
 * campaign (two per core); leaving them undefined builds the correct code. */

/* Limits fixed by the public structures. */
enum
{
    FRAME_COUNT = 16,
    FRAME_BYTES = 64,
    IMAGE_MAX_FRAMES = 4,
    IMAGE_MAX_BYTES = IMAGE_MAX_FRAMES * FRAME_BYTES,
    REFERENCE_LIMIT = 65535,
    /* "No frame" and "no handle" answers are one past the last index. */
    NO_FRAME = FRAME_COUNT,

    QUEUE_SLOTS = 2,
    MESSAGE_MAX_BYTES = 32,
    RIGHT_SEND = 1,
    RIGHT_RECEIVE = 2,
    RIGHT_CLOSE = 4,
    RIGHTS_ALL = RIGHT_SEND | RIGHT_RECEIVE | RIGHT_CLOSE,

    ENTRY_MAX = 4,
    DESCRIPTION_MAX = 4,
    HANDLE_MAX = 8,
    NO_HANDLE = HANDLE_MAX,
    NAME_MAX_BYTES = 12,
    FILE_MAX_BYTES = 256,
    SEEK_FROM_START = 0,
    SEEK_FROM_CURRENT = 1,
    SEEK_FROM_END = 2,

    JOURNAL_SLOTS = 2,
    RECORD_BYTES = 32,
    LAST_FAULT_STAGE = 4
};

_Static_assert(sizeof(((struct sd_memory *)0)->refs) / sizeof(unsigned) ==
                   FRAME_COUNT,
               "frame count");
_Static_assert(sizeof(((struct sd_memory *)0)->bytes[0]) == FRAME_BYTES,
               "frame size");
_Static_assert(sizeof(((struct sd_image *)0)->frames) / sizeof(unsigned) ==
                   IMAGE_MAX_FRAMES,
               "image frames");
_Static_assert(sizeof(((struct sd_channel *)0)->slots) /
                       sizeof(struct sd_message) ==
                   QUEUE_SLOTS,
               "queue slots");
_Static_assert(sizeof(((struct sd_message *)0)->bytes) == MESSAGE_MAX_BYTES,
               "message size");
_Static_assert(sizeof(((struct sd_files *)0)->entries) /
                       sizeof(struct sd_entry) ==
                   ENTRY_MAX,
               "entries");
_Static_assert(sizeof(((struct sd_files *)0)->descriptions) /
                       sizeof(struct sd_open) ==
                   DESCRIPTION_MAX,
               "descriptions");
_Static_assert(sizeof(((struct sd_files *)0)->handles) /
                       sizeof(struct sd_file_handle) ==
                   HANDLE_MAX,
               "handles");
_Static_assert(sizeof(((struct sd_journal *)0)->durable[0]) == RECORD_BYTES,
               "journal record");

/* The core is freestanding-friendly: these loops stand in for string.h. */
static void copy_bytes(void *to, const void *from, size_t n)
{
    unsigned char *d = to;
    const unsigned char *s = from;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
}

static void zero_bytes(void *to, size_t n)
{
    unsigned char *d = to;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = 0;
    }
}

/* ---- Memory: frame ownership with copy-on-write ------------------------ */

/* Claims the lowest free frame (reference count 1) or returns NO_FRAME. */
static unsigned take_frame(struct sd_memory *m)
{
    for (unsigned i = 0; i < FRAME_COUNT; i++)
    {
        if (!m->refs[i])
        {
            m->refs[i] = 1;
            return i;
        }
    }
    return NO_FRAME;
}

/* True when every frame the image lists exists and is referenced. */
static int image_frames_owned(const struct sd_memory *m,
                              const struct sd_image *p)
{
    for (unsigned i = 0; i < p->count; i++)
    {
        if (p->frames[i] >= FRAME_COUNT || !m->refs[p->frames[i]])
        {
            return 0;
        }
    }
    return 1;
}

int sd_release(struct sd_memory *m, struct sd_image *p)
{
    if (!m || !p || !p->active || p->count > IMAGE_MAX_FRAMES)
    {
        return SD_INVALID;
    }
    /* Validate the whole list before touching any count, so a rejected
     * release changes nothing. */
    if (!image_frames_owned(m, p))
    {
        return SD_INVALID;
    }
    for (unsigned i = 0; i < p->count; i++)
    {
        unsigned frame = p->frames[i];
        if (!--m->refs[frame])
        {
            zero_bytes(m->bytes[frame], FRAME_BYTES);
        }
    }
    p->active = 0;
    return SD_OK;
}

/* Fills one fresh frame with page `page` of the input; the tail is zeroed. */
static void fill_page(struct sd_memory *m, unsigned frame,
                      const unsigned char *data, size_t length, unsigned page)
{
    size_t start = (size_t)page * FRAME_BYTES;
    size_t chunk = length - start;
    if (chunk > FRAME_BYTES)
    {
        chunk = FRAME_BYTES;
    }
    zero_bytes(m->bytes[frame], FRAME_BYTES);
    copy_bytes(m->bytes[frame], data + start, chunk);
}

int sd_load(struct sd_memory *m, const unsigned char *data, size_t n,
            int fail_after, struct sd_image *out)
{
    if (!m || !data || !out || !n || n > IMAGE_MAX_BYTES || fail_after < -1)
    {
        return SD_INVALID;
    }
    /* Build in a local descriptor and publish it only when every frame was
     * obtained, so a failure leaves *out as it was. */
    struct sd_image image = {0};
    image.active = 1;
    image.length = (unsigned)n;
    unsigned pages = (unsigned)((n + FRAME_BYTES - 1) / FRAME_BYTES);
    for (unsigned page = 0; page < pages; page++)
    {
        int injected = fail_after >= 0 && page == (unsigned)fail_after;
        unsigned frame = injected ? NO_FRAME : take_frame(m);
        if (frame == NO_FRAME)
        {
#ifndef SD_MUTATE_MEMORY
            /* Give back only the frames this call took. */
            (void)sd_release(m, &image);
#endif
            return SD_FULL;
        }
        image.frames[image.count++] = frame;
        fill_page(m, frame, data, n, page);
    }
    *out = image;
    return SD_OK;
}

/* Clone preflight: inspects the whole parent before anything is modified. */
static int clone_preflight(const struct sd_memory *m,
                           const struct sd_image *parent)
{
    for (unsigned i = 0; i < parent->count; i++)
    {
        unsigned frame = parent->frames[i];
        if (frame >= FRAME_COUNT || !m->refs[frame] ||
            m->refs[frame] >= REFERENCE_LIMIT)
        {
            return SD_FULL;
        }
#ifndef SD_MUTATE_MEMORY2
        /* Shared or device pages cannot be made copy-on-write. */
        if (parent->kind[i])
        {
            return SD_DENIED;
        }
#endif
    }
    return SD_OK;
}

int sd_clone(struct sd_memory *m, struct sd_image *p, unsigned threads,
             struct sd_image *out)
{
    if (!m || !p || !out || !p->active || !p->count ||
        p->count > IMAGE_MAX_FRAMES || threads != 1)
    {
        return SD_INVALID;
    }
    int status = clone_preflight(m, p);
    if (status != SD_OK)
    {
        return status;
    }
    struct sd_image child = *p;
    for (unsigned i = 0; i < p->count; i++)
    {
        m->refs[p->frames[i]]++;
        p->cow[i] = 1;
        child.cow[i] = 1;
    }
    *out = child;
    return SD_OK;
}

/* Replaces image page `page` by a private copy of its shared frame.
 * Failure leaves the image and all counts unchanged. */
static int unshare_page(struct sd_memory *m, struct sd_image *p, unsigned page)
{
    unsigned shared = p->frames[page];
    unsigned fresh = take_frame(m);
    if (fresh == NO_FRAME)
    {
        return SD_FULL;
    }
    copy_bytes(m->bytes[fresh], m->bytes[shared], FRAME_BYTES);
    m->refs[shared]--;
    p->frames[page] = fresh;
    return SD_OK;
}

int sd_write(struct sd_memory *m, struct sd_image *p, unsigned off,
             unsigned char value, int allowed)
{
    if (!m || !p || !p->active || off >= p->length || allowed < 0 ||
        allowed > 1)
    {
        return SD_INVALID;
    }
    unsigned page = off / FRAME_BYTES;
    unsigned frame = p->frames[page];
    if (frame >= FRAME_COUNT || !m->refs[frame])
    {
        return SD_INVALID;
    }
    if (m->refs[frame] > 1)
    {
        if (!allowed)
        {
            return SD_FULL;
        }
        int status = unshare_page(m, p, page);
        if (status != SD_OK)
        {
            return status;
        }
        frame = p->frames[page];
    }
    p->cow[page] = 0;
    m->bytes[frame][off % FRAME_BYTES] = value;
    return SD_OK;
}

/* ---- Channel: capability-checked two-slot byte queue -------------------- */

/* Order of checks: bad channel, then retired/epoch (stale), then rights. */
static int cap_check(const struct sd_channel *q, struct sd_cap c,
                     unsigned right)
{
    if (!q)
    {
        return SD_INVALID;
    }
    if (q->retired || c.epoch != q->epoch)
    {
        return SD_STALE;
    }
    if (c.rights > RIGHTS_ALL || (c.rights & right) != right)
    {
        return SD_DENIED;
    }
    return SD_OK;
}

int sd_channel_init(struct sd_channel *q)
{
    if (!q)
    {
        return SD_INVALID;
    }
    *q = (struct sd_channel){0};
    q->epoch = 1;
    return SD_OK;
}

int sd_grant(const struct sd_channel *q, struct sd_cap c, unsigned rights,
             struct sd_cap *out)
{
    if (!out || !rights || rights > RIGHTS_ALL)
    {
        return SD_INVALID;
    }
    int status = cap_check(q, c, rights);
    if (status != SD_OK)
    {
        return status;
    }
    *out = (struct sd_cap){q->epoch, rights};
    return SD_OK;
}

int sd_send(struct sd_channel *q, struct sd_cap c, const unsigned char *data,
            size_t n)
{
    int status = cap_check(q, c, RIGHT_SEND);
    if (status != SD_OK)
    {
        return status;
    }
    if ((!data && n) || n > MESSAGE_MAX_BYTES)
    {
        return SD_INVALID;
    }
    if (q->closed)
    {
        return SD_CLOSED;
    }
    if (q->count == QUEUE_SLOTS)
    {
        return SD_FULL;
    }
    struct sd_message *m = &q->slots[(q->head + q->count) % QUEUE_SLOTS];
    m->length = (unsigned)n;
    copy_bytes(m->bytes, data, n);
    q->count++;
    return SD_OK;
}

/* Removes the oldest queued message. */
static void drop_oldest(struct sd_channel *q)
{
    q->head = (q->head + 1) % QUEUE_SLOTS;
    q->count--;
}

int sd_receive(struct sd_channel *q, struct sd_cap c, void *out, size_t cap,
               size_t *n, sd_validate_fn check, void *ctx)
{
#ifdef SD_MUTATE_CHANNEL2
    int status = cap_check(q, c, 0);
#else
    int status = cap_check(q, c, RIGHT_RECEIVE);
#endif
    if (status != SD_OK)
    {
        return status;
    }
    if (!n || !check)
    {
        return SD_INVALID;
    }
    if (!q->count)
    {
        return q->closed ? SD_CLOSED : SD_EMPTY;
    }
    struct sd_message *m = &q->slots[q->head];
    if (cap < m->length || (!out && m->length))
    {
        return SD_FULL;
    }
#ifdef SD_MUTATE_CHANNEL
    /* Defect: the message is consumed before the copy-out check. */
    drop_oldest(q);
#endif
    if (check(ctx, out, m->length))
    {
        return SD_COPY;
    }
    copy_bytes(out, m->bytes, m->length);
    *n = m->length;
#ifndef SD_MUTATE_CHANNEL
    /* Dequeue only after the copy-out is known to have succeeded. */
    drop_oldest(q);
#endif
    return SD_OK;
}

int sd_channel_close(struct sd_channel *q, struct sd_cap c)
{
    int status = cap_check(q, c, RIGHT_CLOSE);
    if (status != SD_OK)
    {
        return status;
    }
    q->closed = 1;
    return SD_OK;
}

int sd_channel_reopen(struct sd_channel *q)
{
    if (!q || !q->closed || q->count)
    {
        return SD_INVALID;
    }
    if (q->retired)
    {
        return SD_STALE;
    }
    /* The epoch never wraps: at its maximum the channel retires for good, so
     * a very old capability can never match a recycled epoch. */
    if (q->epoch == UINT32_MAX)
    {
        q->retired = 1;
        return SD_FULL;
    }
    q->epoch++;
    q->closed = 0;
    q->head = 0;
    return SD_OK;
}

/* ---- Files: read-only entries with shared open descriptions ------------- */

static int name_char_valid(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' ||
           c == '-' || c == '.';
}

/* 1..11 characters from [a-z0-9_.-], excluding "." and "..". The scan stops
 * at the 12th byte, so an unterminated name is never read further. */
static int name_valid(const char *s)
{
    if (!s)
    {
        return 0;
    }
    unsigned n = 0;
    for (; n < NAME_MAX_BYTES && s[n]; n++)
    {
        if (!name_char_valid((unsigned char)s[n]))
        {
            return 0;
        }
    }
    int dot = n == 1 && s[0] == '.';
    int dotdot = n == 2 && s[0] == '.' && s[1] == '.';
    return n && n < NAME_MAX_BYTES && !dot && !dotdot;
}

static int names_equal(const char *a, const char *b)
{
    unsigned i = 0;
    while (a[i] && a[i] == b[i])
    {
        i++;
    }
    return a[i] == b[i];
}

int sd_files_init(struct sd_files *fs, const struct sd_entry *entries,
                  unsigned n)
{
    if (!fs || !entries || !n || n > ENTRY_MAX)
    {
        return SD_INVALID;
    }
    /* Validate everything, duplicates included, in a local table first. */
    struct sd_files next = {0};
    for (unsigned i = 0; i < n; i++)
    {
        if (!name_valid(entries[i].name) ||
            entries[i].length > FILE_MAX_BYTES ||
            (!entries[i].data && entries[i].length))
        {
            return SD_INVALID;
        }
        for (unsigned j = 0; j < i; j++)
        {
            if (names_equal(entries[i].name, entries[j].name))
            {
                return SD_INVALID;
            }
        }
        next.entries[i] = entries[i];
    }
    next.count = n;
    *fs = next;
    return SD_OK;
}

/* Index of the live handle `h` names, or NO_HANDLE if it is closed, never
 * issued or from an older incarnation of the slot (generation mismatch). */
static unsigned live_handle(const struct sd_files *fs, struct sd_handle h)
{
    if (fs && h.index < HANDLE_MAX && fs->handles[h.index].active &&
        fs->handles[h.index].generation == h.generation)
    {
        return h.index;
    }
    return NO_HANDLE;
}

/* A slot whose generation reached the maximum is retired: reusing it would
 * wrap and revive an old handle value. */
static unsigned free_handle(const struct sd_files *fs)
{
    for (unsigned i = 0; i < HANDLE_MAX; i++)
    {
        if (!fs->handles[i].active && fs->handles[i].generation < UINT32_MAX)
        {
            return i;
        }
    }
    return NO_HANDLE;
}

/* Lowest unused open description, or DESCRIPTION_MAX. */
static unsigned free_description(const struct sd_files *fs)
{
    unsigned d = 0;
    while (d < DESCRIPTION_MAX && fs->descriptions[d].active)
    {
        d++;
    }
    return d;
}

/* Binds a free handle slot to description `d` and returns the new handle. */
static struct sd_handle issue_handle(struct sd_files *fs, unsigned slot,
                                     unsigned d)
{
    struct sd_file_handle *handle = &fs->handles[slot];
    handle->active = 1;
    handle->open = d;
    handle->generation++;
    return (struct sd_handle){slot, handle->generation};
}

int sd_open(struct sd_files *fs, const char *name, struct sd_handle *out)
{
    if (!fs || !out || !name_valid(name))
    {
        return SD_INVALID;
    }
    unsigned entry = 0;
    while (entry < fs->count && !names_equal(name, fs->entries[entry].name))
    {
        entry++;
    }
    if (entry == fs->count)
    {
        return SD_INVALID;
    }
    unsigned slot = free_handle(fs);
    unsigned d = free_description(fs);
    if (slot == NO_HANDLE || d == DESCRIPTION_MAX)
    {
        return SD_FULL;
    }
    fs->descriptions[d] = (struct sd_open){1, entry, 0, 1};
    *out = issue_handle(fs, slot, d);
    return SD_OK;
}

int sd_dup(struct sd_files *fs, struct sd_handle h, struct sd_handle *out)
{
    if (!fs || !out)
    {
        return SD_INVALID;
    }
    unsigned old = live_handle(fs, h);
    if (old == NO_HANDLE)
    {
        return SD_STALE;
    }
    unsigned slot = free_handle(fs);
    if (slot == NO_HANDLE)
    {
        return SD_FULL;
    }
    /* A duplicate shares the open description, hence the offset. */
    struct sd_handle issued = issue_handle(fs, slot, fs->handles[old].open);
    unsigned shared = fs->handles[slot].open;
#ifdef SD_MUTATE_FILES
    /* Defect: the duplicate gets its own description and a separate offset. */
    unsigned d = free_description(fs);
    if (d < DESCRIPTION_MAX)
    {
        fs->descriptions[d] = fs->descriptions[shared];
        fs->descriptions[d].refs = 0;
        fs->handles[slot].open = d;
        shared = d;
    }
#endif
    fs->descriptions[shared].refs++;
    *out = issued;
    return SD_OK;
}

/* Base position for a seek origin; the caller has checked origin <= 2. */
static int64_t seek_base(const struct sd_files *fs, const struct sd_open *d,
                         unsigned origin)
{
    if (origin == SEEK_FROM_START)
    {
        return 0;
    }
    if (origin == SEEK_FROM_CURRENT)
    {
        return d->offset;
    }
    return fs->entries[d->entry].length;
}

int sd_seek(struct sd_files *fs, struct sd_handle h, unsigned origin,
            int32_t delta)
{
    if (!fs || origin > SEEK_FROM_END)
    {
        return SD_INVALID;
    }
    unsigned i = live_handle(fs, h);
    if (i == NO_HANDLE)
    {
        return SD_STALE;
    }
    struct sd_open *d = &fs->descriptions[fs->handles[i].open];
    /* 64-bit arithmetic: base + delta cannot overflow, so the range check is
     * the only test needed. */
    int64_t offset = seek_base(fs, d, origin) + delta;
    if (offset < 0 || offset > fs->entries[d->entry].length)
    {
        return SD_INVALID;
    }
    d->offset = (unsigned)offset;
    return SD_OK;
}

int sd_read(struct sd_files *fs, struct sd_handle h, void *out, size_t cap,
            size_t requested, size_t *n, sd_validate_fn check, void *ctx)
{
    if (!fs || !n || !check || requested > cap)
    {
        return SD_INVALID;
    }
    unsigned i = live_handle(fs, h);
    if (i == NO_HANDLE)
    {
        return SD_STALE;
    }
    struct sd_open *d = &fs->descriptions[fs->handles[i].open];
    struct sd_entry *e = &fs->entries[d->entry];
    size_t amount = e->length - d->offset;
    if (amount > requested)
    {
        amount = requested;
    }
    if (!out && amount)
    {
        return SD_INVALID;
    }
#ifdef SD_MUTATE_FILES2
    /* Defect: the shared offset advances before the copy-out check. */
    d->offset += (unsigned)amount;
#endif
    if (check(ctx, out, amount))
    {
        return SD_COPY;
    }
    if (amount)
    {
#ifdef SD_MUTATE_FILES2
        copy_bytes(out, e->data + d->offset - amount, amount);
#else
        copy_bytes(out, e->data + d->offset, amount);
#endif
    }
#ifndef SD_MUTATE_FILES2
    d->offset += (unsigned)amount;
#endif
    *n = amount;
    return SD_OK;
}

int sd_close(struct sd_files *fs, struct sd_handle h)
{
    if (!fs)
    {
        return SD_INVALID;
    }
    unsigned i = live_handle(fs, h);
    if (i == NO_HANDLE)
    {
        return SD_STALE;
    }
    struct sd_file_handle *handle = &fs->handles[i];
    struct sd_open *d = &fs->descriptions[handle->open];
    handle->active = 0;
    if (!--d->refs)
    {
        d->active = 0;
    }
    return SD_OK;
}

/* ---- Journal: exactly-once receipt over two alternating slots ----------- */

/* Record layout (little-endian u32 fields). Bytes 24..31 are zero padding. */
enum
{
    RECORD_MAGIC_AT = 0,
    RECORD_ID_AT = 4,
    RECORD_DELTA_AT = 8,
    RECORD_VALUE_AT = 12,
    RECORD_CHECKSUM_AT = 16,
    RECORD_COMMIT_AT = 20,
    CHECKSUMMED_BYTES = 16
};

#define RECORD_MAGIC UINT32_C(0x53445231)
#define RECORD_COMMIT_MARKER UINT32_C(0xc017c017)
#define FNV_OFFSET_BASIS UINT32_C(2166136261)
#define FNV_PRIME UINT32_C(16777619)

static uint32_t read32(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = (unsigned char)(v >> (i * 8));
    }
}

/* Two's-complement reinterpretation without implementation-defined casts. */
static int32_t signed32(uint32_t n)
{
    return n <= INT32_MAX ? (int32_t)n : -(int32_t)(UINT32_MAX - n) - 1;
}

/* FNV-1a over the first 16 bytes: detects accidents, not forgery. */
static uint32_t checksum(const unsigned char *p)
{
    uint32_t n = FNV_OFFSET_BASIS;
    for (unsigned i = 0; i < CHECKSUMMED_BYTES; i++)
    {
        n = (n ^ p[i]) * FNV_PRIME;
    }
    return n;
}

int sd_recover(const struct sd_journal *j, struct sd_receipt *out)
{
    if (!j || !out)
    {
        return SD_INVALID;
    }
    struct sd_receipt latest = {0};
    for (unsigned i = 0; i < JOURNAL_SLOTS; i++)
    {
        const unsigned char *p = j->durable[i];
        /* Without the commit marker the slot is an interrupted write, not
         * a receipt: skip it. */
        if (read32(p + RECORD_COMMIT_AT) != RECORD_COMMIT_MARKER)
        {
            continue;
        }
        uint32_t id = read32(p + RECORD_ID_AT);
        if (read32(p + RECORD_MAGIC_AT) != RECORD_MAGIC || !id ||
            read32(p + RECORD_CHECKSUM_AT) != checksum(p))
        {
            return SD_CORRUPT;
        }
        if (id == latest.last)
        {
            return SD_CORRUPT;
        }
        if (id > latest.last)
        {
            latest =
                (struct sd_receipt){id, signed32(read32(p + RECORD_DELTA_AT)),
                                    signed32(read32(p + RECORD_VALUE_AT))};
        }
    }
    *out = latest;
    return SD_OK;
}

/* Builds the uncommitted record for (id, delta, value) in `p`. */
static void prepare_record(unsigned char *p, uint32_t id, int32_t delta,
                           int32_t value)
{
    zero_bytes(p, RECORD_BYTES);
    put32(p + RECORD_MAGIC_AT, RECORD_MAGIC);
    put32(p + RECORD_ID_AT, id);
    put32(p + RECORD_DELTA_AT, (uint32_t)delta);
    put32(p + RECORD_VALUE_AT, (uint32_t)value);
    put32(p + RECORD_CHECKSUM_AT, checksum(p));
}

/* Decides what a request means relative to the latest receipt. Returns
 * SD_OK for "new request, proceed"; any other status (including the
 * idempotent SD_OK replay, signalled by *replayed) ends the call. */
static int classify_request(const struct sd_receipt *latest, uint32_t id,
                            int32_t delta, int *replayed)
{
    *replayed = 0;
    if (id == latest->last)
    {
#ifndef SD_MUTATE_JOURNAL2
        /* Same id, different payload: a conflicting reuse, not a retry. */
        if (delta != latest->delta)
        {
            return SD_CONFLICT;
        }
#else
        (void)delta;
#endif
        *replayed = 1;
        return SD_OK;
    }
    if (id < latest->last)
    {
        return SD_STALE;
    }
    if (latest->last == UINT32_MAX || id != latest->last + 1)
    {
        return SD_INVALID;
    }
    return SD_OK;
}

int sd_apply(struct sd_journal *j, uint32_t id, int32_t delta, unsigned fault,
             int32_t *out)
{
    if (!j || !out || fault > LAST_FAULT_STAGE || !id)
    {
        return SD_INVALID;
    }
    struct sd_receipt latest;
    int status = sd_recover(j, &latest);
    if (status != SD_OK)
    {
        return status;
    }
    int replayed;
    status = classify_request(&latest, id, delta, &replayed);
    if (replayed)
    {
        *out = latest.value;
        return SD_OK;
    }
    if (status != SD_OK)
    {
        return status;
    }
    int64_t value = (int64_t)latest.value + delta;
    if (value < INT32_MIN || value > INT32_MAX)
    {
        return SD_FULL;
    }
    /* The two slots alternate by id parity, so the previous receipt stays
     * intact while the next one is written. Commit protocol: prepare, flush
     * the uncommitted record, add the marker, flush again. Each flush is an
     * atomic whole-record copy; `fault` stops the sequence after a step. */
    unsigned slot = id & 1u;
    unsigned char *pending = j->pending[slot];
    prepare_record(pending, id, delta, (int32_t)value);
    if (fault == 1)
    {
        return SD_UNCERTAIN;
    }
    copy_bytes(j->durable[slot], pending, RECORD_BYTES);
    if (fault == 2)
    {
        return SD_UNCERTAIN;
    }
    put32(pending + RECORD_COMMIT_AT, RECORD_COMMIT_MARKER);
#ifdef SD_MUTATE_JOURNAL
    /* Defect: success is reported before the committed record is flushed. */
    if (fault == 3)
    {
        *out = (int32_t)value;
        return SD_OK;
    }
#else
    if (fault == 3)
    {
        return SD_UNCERTAIN;
    }
#endif
    copy_bytes(j->durable[slot], pending, RECORD_BYTES);
    if (fault == 4)
    {
        /* Committed and durable, but the caller gets SD_UNCERTAIN, not the value. */
        return SD_UNCERTAIN;
    }
    *out = (int32_t)value;
    return SD_OK;
}

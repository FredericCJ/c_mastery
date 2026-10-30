#include "allocation.h"

/*
 * Physical block layout (all offsets in bytes from the block start, all
 * fields little-endian 16-bit unless noted):
 *
 *   header  +0 total block size   +2 payload length (0 = free block)
 *           +4 live flag (0/1)    +5 zero
 *           +6 0xa5               +7 0x5a      payload begins at +8
 *   footer  total-4: total size   total-2: live flag   total-1: 0xc3
 *
 * Invariants checked by al_heap_check before every mutation:
 *   - blocks tile [0, capacity) exactly; each is >= 16 bytes, a multiple of
 *     8, and has matching header and footer;
 *   - a live block has 1..total-12 payload bytes and exactly one entry in
 *     `live`; a free block has none and appears in `free_offsets` in address
 *     order;
 *   - with immediate coalescing (deferred == 0) two free blocks are never
 *     adjacent;
 *   - the cached payload/charged totals equal the sums over the chain.
 */
enum heap_layout
{
    HEADER_BYTES = 8,
    FOOTER_BYTES = 4,
    OVERHEAD_BYTES = HEADER_BYTES + FOOTER_BYTES,
    GRANULE_BYTES = 8,
    MIN_BLOCK_BYTES = 16,
    MAX_PAYLOAD_BYTES = AL_HEAP_BYTES - OVERHEAD_BYTES,
    MAX_ALIGNMENT = 8,

    HEADER_TOTAL = 0,
    HEADER_LENGTH = 2,
    HEADER_LIVE = 4,
    HEADER_ZERO = 5,
    HEADER_MAGIC_LOW = 6,
    HEADER_MAGIC_HIGH = 7,
    FOOTER_LIVE = 2,
    FOOTER_MAGIC = 3,

    MAGIC_LOW = 0xa5,
    MAGIC_HIGH = 0x5a,
    MAGIC_FOOTER = 0xc3,
    NO_SLOT = AL_HEAP_BLOCKS
};

static unsigned get16(const unsigned char *p)
{
    return p[0] | ((unsigned)p[1] << 8);
}

static void put16(unsigned char *p, unsigned n)
{
    p[0] = (unsigned char)n;
    p[1] = (unsigned char)(n >> 8);
}

static unsigned block_total(const struct al_heap *h, unsigned at)
{
    return get16(h->bytes + at + HEADER_TOTAL);
}

static unsigned block_length(const struct al_heap *h, unsigned at)
{
    return get16(h->bytes + at + HEADER_LENGTH);
}

/* Block size that holds `payload` bytes: overhead added, rounded to 8. */
static unsigned block_bytes_for(unsigned payload)
{
    return (payload + OVERHEAD_BYTES + GRANULE_BYTES - 1) &
           ~(unsigned)(GRANULE_BYTES - 1);
}

/*
 * Bytes to charge when `need` bytes are carved out of an `available` block:
 * a remainder smaller than the minimum block would be unusable, so it is
 * absorbed into the allocation.
 */
static unsigned charge_for(unsigned available, unsigned need)
{
    return available - need < MIN_BLOCK_BYTES ? available : need;
}

/* Writes header and footer of the block at `at`; `n` == 0 marks it free. */
static void tag(struct al_heap *h, unsigned at, unsigned total, unsigned n)
{
    unsigned char *header = h->bytes + at;
    unsigned char live = (unsigned char)(n != 0);
    put16(header + HEADER_TOTAL, total);
    put16(header + HEADER_LENGTH, n);
    header[HEADER_LIVE] = live;
    header[HEADER_ZERO] = 0;
    header[HEADER_MAGIC_LOW] = MAGIC_LOW;
    header[HEADER_MAGIC_HIGH] = MAGIC_HIGH;

    unsigned char *footer = header + total - FOOTER_BYTES;
    put16(footer + HEADER_TOTAL, total);
    footer[FOOTER_LIVE] = live;
    footer[FOOTER_MAGIC] = MAGIC_FOOTER;
}

static int geometry_ok(const struct al_heap *h)
{
    return h->capacity >= MIN_BLOCK_BYTES && h->capacity <= AL_HEAP_BYTES &&
           h->capacity % GRANULE_BYTES == 0 &&
           h->free_count <= AL_HEAP_BLOCKS && h->policy <= 1 &&
           h->deferred <= 1;
}

/* Header sanity of the block at `at`, before the footer is trusted. */
static int header_ok(const struct al_heap *h, unsigned at)
{
    const unsigned char *p = h->bytes + at;
    unsigned total = get16(p + HEADER_TOTAL);
    unsigned n = get16(p + HEADER_LENGTH);
    unsigned live = p[HEADER_LIVE];
    if (total < MIN_BLOCK_BYTES || total % GRANULE_BYTES ||
        total > h->capacity - at || live > 1 || p[HEADER_ZERO] ||
        p[HEADER_MAGIC_LOW] != MAGIC_LOW || p[HEADER_MAGIC_HIGH] != MAGIC_HIGH)
    {
        return 0;
    }
    if (!live)
    {
        return n == 0;
    }
    return n != 0 && n <= total - OVERHEAD_BYTES;
}

/* The footer mirrors the header; `total` is already bounded by header_ok. */
static int footer_ok(const struct al_heap *h, unsigned at)
{
#if AL_MUTANT != 3
    const unsigned char *p = h->bytes + at;
    unsigned total = get16(p + HEADER_TOTAL);
    const unsigned char *footer = p + total - FOOTER_BYTES;
    if (get16(footer + HEADER_TOTAL) != total ||
        footer[FOOTER_LIVE] != p[HEADER_LIVE] ||
        footer[FOOTER_MAGIC] != MAGIC_FOOTER)
    {
        return 0;
    }
#else
    (void)h;
    (void)at;
#endif
    return 1;
}

/* Number of `live` table entries that claim the block at `at`. */
static unsigned live_claims(const struct al_heap *h, unsigned at)
{
    unsigned claims = 0;
    for (unsigned j = 0; j < AL_HEAP_BLOCKS; ++j)
    {
        if (h->live[j].id && h->live[j].offset == at)
        {
            ++claims;
        }
    }
    return claims;
}

struct chain_totals
{
    unsigned free_seen;
    unsigned allocated;
    size_t payload;
    size_t charged;
};

/* Walks the physical chain; returns 0 on the first violated invariant. */
static int walk_chain(const struct al_heap *h, struct chain_totals *totals)
{
    unsigned at = 0;
    unsigned previous_free = 0;
    *totals = (struct chain_totals){0};
    while (at < h->capacity)
    {
        if (h->capacity - at < MIN_BLOCK_BYTES || !header_ok(h, at) ||
            !footer_ok(h, at))
        {
            return 0;
        }
        unsigned total = block_total(h, at);
        unsigned live = h->bytes[at + HEADER_LIVE];
        unsigned claims = live_claims(h, at);
        if (live)
        {
            if (claims != 1)
            {
                return 0;
            }
            ++totals->allocated;
            totals->payload += block_length(h, at);
            totals->charged += total;
        }
        else
        {
            if (claims || totals->free_seen >= h->free_count ||
                h->free_offsets[totals->free_seen++] != at ||
                (!h->deferred && previous_free))
            {
                return 0;
            }
        }
        previous_free = !live;
        at += total;
    }
    return 1;
}

/* Live ids must be distinct and not newer than the serial counter. */
static int live_table_ok(const struct al_heap *h, unsigned *entries)
{
    *entries = 0;
    for (unsigned i = 0; i < AL_HEAP_BLOCKS; ++i)
    {
        if (!h->live[i].id)
        {
            continue;
        }
        ++*entries;
        if (h->live[i].id > h->serial)
        {
            return 0;
        }
        for (unsigned j = 0; j < i; ++j)
        {
            if (h->live[j].id == h->live[i].id)
            {
                return 0;
            }
        }
    }
    return 1;
}

int al_heap_check(const struct al_heap *h)
{
    if (!h || !geometry_ok(h))
    {
        return AL_CORRUPT;
    }
    struct chain_totals totals;
    unsigned entries;
    if (!walk_chain(h, &totals) || !live_table_ok(h, &entries))
    {
        return AL_CORRUPT;
    }
    int cached_ok =
        totals.payload == h->payload && totals.charged == h->charged;
    int index_ok =
        totals.free_seen == h->free_count && totals.allocated == entries;
    return index_ok && cached_ok ? AL_OK : AL_CORRUPT;
}

/*
 * Recomputes the free index and the cached totals from the chain, optionally
 * merging each run of adjacent free blocks first, and raises the peaks.
 */
static void rebuild(struct al_heap *h, int merge)
{
    unsigned at = 0;
    h->free_count = 0;
    h->payload = 0;
    h->charged = 0;
    while (at < h->capacity)
    {
        unsigned total = block_total(h, at);
        unsigned n = block_length(h, at);
        if (!n)
        {
            while (merge && at + total < h->capacity &&
                   !h->bytes[at + total + HEADER_LIVE])
            {
                total += block_total(h, at + total);
                tag(h, at, total, 0);
            }
            h->free_offsets[h->free_count++] = at;
        }
        else
        {
            h->payload += n;
            h->charged += total;
        }
        at += total;
    }
    if (h->payload > h->peak_payload)
    {
        h->peak_payload = h->payload;
    }
    if (h->charged > h->peak_charged)
    {
        h->peak_charged = h->charged;
    }
}

int al_heap_init(struct al_heap *h, unsigned n, unsigned best,
                 unsigned deferred, unsigned instrument)
{
    if (!h || n < MIN_BLOCK_BYTES || n > AL_HEAP_BYTES || n % GRANULE_BYTES ||
        best > 1 || deferred > 1 || instrument > 1)
    {
        return AL_BAD;
    }
    *h = (struct al_heap){0};
    h->capacity = n;
    h->policy = best;
    h->deferred = deferred;
    h->instrument = instrument;
    tag(h, 0, n, 0);
    rebuild(h, 0);
    return AL_OK;
}

/* Index into `live` of the object named by `handle`, or -1 if stale. */
static int slot(const struct al_heap *h, struct al_handle handle)
{
    if (!h || handle.owner != h || !handle.id)
    {
        return -1;
    }
    for (unsigned i = 0; i < AL_HEAP_BLOCKS; ++i)
    {
        if (h->live[i].id == handle.id)
        {
            return (int)i;
        }
    }
    return -1;
}

static unsigned first_empty_slot(const struct al_heap *h)
{
    for (unsigned i = 0; i < AL_HEAP_BLOCKS; ++i)
    {
        if (!h->live[i].id)
        {
            return i;
        }
    }
    return NO_SLOT;
}

static size_t add_saturating(size_t a, size_t b)
{
    return b > SIZE_MAX - a ? SIZE_MAX : a + b;
}

/*
 * Picks the free block for `need` bytes: the first sufficient block (policy 0)
 * or the smallest sufficient block, earliest on ties (policy 1). Returns the
 * block offset or AL_HEAP_BYTES for none; `*size` receives its total and
 * `*steps` the number of free blocks examined.
 */
static unsigned find_free_block(const struct al_heap *h, unsigned need,
                                unsigned *size, unsigned *steps)
{
    unsigned choice = AL_HEAP_BYTES;
    unsigned best = AL_HEAP_BYTES + 1;
    *steps = 0;
    for (unsigned i = 0; i < h->free_count; ++i)
    {
        unsigned at = h->free_offsets[i];
        unsigned total = block_total(h, at);
        ++*steps;
        if (total >= need && total < best)
        {
            choice = at;
            best = total;
            if (!h->policy)
            {
                break;
            }
        }
    }
    *size = best;
    return choice;
}

/*
 * Allocates `n` payload bytes in `h` (already checked; callers pass a scratch
 * copy so that failure never leaves a partial change). On AL_OK `*idx` is the
 * new entry of `live`.
 */
static int reserve(struct al_heap *h, unsigned n, unsigned *idx)
{
    unsigned need = block_bytes_for(n);
    unsigned empty = first_empty_slot(h);
    if (empty == NO_SLOT)
    {
        return AL_FULL;
    }
    unsigned best;
    unsigned steps;
    unsigned choice = find_free_block(h, need, &best, &steps);
    if (choice == AL_HEAP_BYTES)
    {
        return AL_FULL;
    }
    unsigned total = charge_for(best, need);
#if AL_MUTANT == 4
    total = need;
#endif
    tag(h, choice, total, n);
    if (best > total)
    {
        tag(h, choice + total, best - total, 0);
    }
    h->live[empty].offset = choice;
    h->live[empty].id = ++h->serial;
    if (h->instrument)
    {
        h->search_steps = add_saturating(h->search_steps, steps);
    }
    rebuild(h, 0);
    *idx = empty;
    return AL_OK;
}

int al_heap_take(struct al_heap *h, size_t n, size_t alignment,
                 struct al_handle *out)
{
    if (!h || !out || !n || n > MAX_PAYLOAD_BYTES || !alignment ||
        alignment > MAX_ALIGNMENT || (alignment & (alignment - 1)))
    {
        return AL_BAD;
    }
    if (al_heap_check(h))
    {
        return AL_CORRUPT;
    }
    if (h->serial == UINT32_MAX)
    {
        return AL_RETIRED;
    }
    struct al_heap scratch = *h;
    unsigned i;
    int status = reserve(&scratch, (unsigned)n, &i);
    if (status)
    {
        return status;
    }
    *h = scratch;
    *out = (struct al_handle){h, h->live[i].id};
    return AL_OK;
}

int al_heap_view(struct al_heap *h, struct al_handle handle,
                 unsigned char **out, size_t *n)
{
    if (!h || !out || !n)
    {
        return AL_BAD;
    }
    if (al_heap_check(h))
    {
        return AL_CORRUPT;
    }
    int i = slot(h, handle);
    if (i < 0)
    {
        return AL_STALE;
    }
    unsigned at = h->live[i].offset;
    *out = h->bytes + at + HEADER_BYTES;
    *n = block_length(h, at);
    return AL_OK;
}

int al_heap_release(struct al_heap *h, struct al_handle handle)
{
    if (!h)
    {
        return AL_BAD;
    }
    if (al_heap_check(h))
    {
        return AL_CORRUPT;
    }
    int i = slot(h, handle);
    if (i < 0)
    {
        return AL_STALE;
    }
    unsigned at = h->live[i].offset;
    tag(h, at, block_total(h, at), 0);
    h->live[i].id = 0;
    rebuild(h, !h->deferred);
    return AL_OK;
}

int al_heap_collect(struct al_heap *h)
{
    if (al_heap_check(h))
    {
        return AL_CORRUPT;
    }
    rebuild(h, 1);
    return AL_OK;
}

int al_heap_stats(const struct al_heap *h, size_t *total, size_t *largest)
{
    if (!total || !largest)
    {
        return AL_BAD;
    }
    if (al_heap_check(h))
    {
        return AL_CORRUPT;
    }
    size_t sum = 0;
    size_t big = 0;
    for (unsigned i = 0; i < h->free_count; ++i)
    {
        size_t n = block_total(h, h->free_offsets[i]);
        sum += n;
        if (n > big)
        {
            big = n;
        }
    }
    *total = sum;
    *largest = big;
    return AL_OK;
}

/*
 * In-place resize: the block at `at` is retagged to hold `n` bytes inside
 * `available` bytes (itself plus an adjacent free block, caller-verified).
 * The object keeps its slot but receives a new identity, so the old handle
 * goes stale.
 */
static void resize_in_place(struct al_heap *t, unsigned slot_index, unsigned at,
                            unsigned available, unsigned n)
{
    unsigned need = block_bytes_for(n);
    unsigned total = charge_for(available, need);
    tag(t, at, total, n);
    if (available > total)
    {
        tag(t, at + total, available - total, 0);
    }
    t->live[slot_index].id = ++t->serial;
    rebuild(t, !t->deferred);
}

/*
 * Moving resize: allocate a new block, copy min(old, new) payload bytes with
 * the caller's copier, then free the old block. A failing copier returns
 * AL_CLEANUP and the caller discards the scratch heap.
 * On AL_OK `*new_slot` is the entry of the moved object.
 */
static int resize_by_move(struct al_heap *t, unsigned slot_index, unsigned n,
                          al_copy_fn copier, void *context, unsigned *new_slot)
{
    unsigned at = t->live[slot_index].offset;
    unsigned old_length = block_length(t, at);
    unsigned old_size = block_total(t, at);
    unsigned j;
    int status = reserve(t, n, &j);
    if (status)
    {
        return status;
    }
    unsigned copy = old_length < n ? old_length : n;
    if (copier(context, t->bytes + t->live[j].offset + HEADER_BYTES,
               t->bytes + at + HEADER_BYTES, copy))
    {
        return AL_CLEANUP;
    }
    tag(t, at, old_size, 0);
    t->live[slot_index].id = 0;
    rebuild(t, !t->deferred);
    *new_slot = j;
    return AL_OK;
}

int al_heap_resize_copy(struct al_heap *h, struct al_handle handle, size_t n,
                        int fail_acquire, al_copy_fn copier, void *context,
                        struct al_handle *out)
{
    if (!h || !out || !n || n > MAX_PAYLOAD_BYTES || !copier)
    {
        return AL_BAD;
    }
    if (al_heap_check(h))
    {
        return AL_CORRUPT;
    }
    int i = slot(h, handle);
    if (i < 0)
    {
        return AL_STALE;
    }
    if (h->serial == UINT32_MAX)
    {
        return AL_RETIRED;
    }
    struct al_heap scratch = *h;
    unsigned at = scratch.live[i].offset;
    unsigned size = block_total(&scratch, at);
    unsigned need = block_bytes_for((unsigned)n);
    unsigned available = size;
    int next_is_free =
        at + size < scratch.capacity && !scratch.bytes[at + size + HEADER_LIVE];
    if (need > size && next_is_free)
    {
        available += block_total(&scratch, at + size);
    }
    if (need <= available)
    {
        resize_in_place(&scratch, (unsigned)i, at, available, (unsigned)n);
    }
    else
    {
        if (fail_acquire)
        {
            return AL_FULL;
        }
        unsigned moved;
        int status = resize_by_move(&scratch, (unsigned)i, (unsigned)n, copier,
                                    context, &moved);
        if (status)
        {
#if AL_MUTANT == 5
            if (status == AL_CLEANUP)
            {
                *h = scratch;
            }
#endif
            return status;
        }
        i = (int)moved;
    }
    *h = scratch;
    *out = (struct al_handle){h, h->live[i].id};
    return AL_OK;
}

/*
 * Copier used by al_heap_resize: `context` points at a size_t byte limit and
 * the copy fails once more than that many bytes must be moved. SIZE_MAX
 * means the copy never fails.
 */
static int byte_copy(void *context, unsigned char *dest,
                     const unsigned char *src, size_t n)
{
    size_t limit = *(size_t *)context;
    for (size_t i = 0; i < n; ++i)
    {
        if (i == limit)
        {
            return 1;
        }
        dest[i] = src[i];
    }
    return 0;
}

int al_heap_resize(struct al_heap *h, struct al_handle handle, size_t n,
                   int fail_acquire, size_t fail_after, struct al_handle *out)
{
    return al_heap_resize_copy(h, handle, n, fail_acquire, byte_copy,
                               &fail_after, out);
}

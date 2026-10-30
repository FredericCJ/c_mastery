#include "depth.h"

/* Assessor reference implementation of the foundation_depth contracts in
 * labs/foundation_depth/include/depth.h. FD_MUTATE_* macros select compiled
 * defects for the mutation controls; none is defined in a normal build. */

/* Octet positions inside a wire record. The payload follows the header. */
enum wire_offset
{
    WIRE_VERSION = 0,
    WIRE_FLAGS = 1,
    WIRE_COUNT_HIGH = 2,
    WIRE_COUNT_LOW = 3,
    WIRE_PAYLOAD = FD_WIRE_HEADER_BYTES
};

/* Answers are indexed by enum fd_case. */
enum fd_class fd_verdict(enum fd_case question)
{
    static const enum fd_class answers[FD_CASE_COUNT] = {
        FD_DEFINED,
        FD_DEFINED,
        FD_UNDEFINED,
        FD_UNSPECIFIED,
        FD_IMPLEMENTATION_DEFINED,
        FD_UNDEFINED,
        FD_UNDEFINED,
        FD_CONSTRAINT,
        FD_UNDEFINED,
        FD_DEFINED};
#ifdef FD_MUTATE_SEMANTIC
    if (question == FD_UNSEQUENCED)
    {
        return FD_UNSPECIFIED;
    }
#endif
    /* The unsigned cast also rejects negative identifiers. */
    return (unsigned)question < FD_CASE_COUNT ? answers[question] : FD_UNKNOWN;
}

enum fd_status fd_repair_step(int *value, int *previous)
{
    if (!value || !previous || value == previous)
    {
        return FD_INVALID;
    }
    if (*value == INT_MAX)
    {
        return FD_RANGE;
    }
    *previous = *value;
    *value += 1;
    return FD_OK;
}

enum fd_status fd_extent(size_t offset, size_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (!out)
    {
        return FD_INVALID;
    }
#ifdef FD_MUTATE_ZERO_OFFSET
    if (count == 0)
    {
        *out = offset;
        return FD_OK;
    }
#endif
#ifdef FD_MUTATE_EARLY_EXTENT
    *out = offset;
#endif
#ifdef FD_MUTATE_EXTENT
    size_t wrapped = offset + count * width;
    if (wrapped > limit)
    {
        return FD_RANGE;
    }
    *out = wrapped;
#else
    /* offset <= limit makes (limit - offset) exact; dividing by width instead
     * of multiplying count * width cannot wrap. A zero width consumes nothing
     * but still needs offset <= limit. */
    if (offset > limit || (width != 0 && count > (limit - offset) / width))
    {
        return FD_RANGE;
    }
    *out = offset + count * width;
#endif
    return FD_OK;
}

enum fd_status fd_from_signed(intmax_t value, size_t *out)
{
    if (!out)
    {
        return FD_INVALID;
    }
    /* Negative values are rejected first, so the cast to the unsigned wide
     * type preserves the value being compared. */
    if (value < 0 || (uintmax_t)value > SIZE_MAX)
    {
        return FD_RANGE;
    }
    *out = (size_t)value;
    return FD_OK;
}

/* A cursor is usable when position <= length and a positive length has a
 * base. The all-zero cursor is valid. */
static int cursor_valid(const struct fd_cursor *cursor)
{
    return cursor && cursor->position <= cursor->length &&
           (cursor->base || cursor->length == 0);
}

enum fd_status fd_take(struct fd_cursor *cursor, size_t count,
                       const unsigned char **out)
{
    if (!out || !cursor_valid(cursor))
    {
        return FD_INVALID;
    }
    if (count > cursor->length - cursor->position)
    {
#ifdef FD_MUTATE_CURSOR
        cursor->position = cursor->length;
#endif
        return FD_RANGE;
    }
    /* A null base is only valid for length zero; do not form base + 0. */
    const unsigned char *taken =
        cursor->base ? cursor->base + cursor->position : 0;
    cursor->position += count;
    *out = taken;
    return FD_OK;
}

enum fd_status fd_subview(const struct fd_cursor *parent, size_t offset,
                          size_t count, struct fd_cursor *out)
{
    if (!out || !cursor_valid(parent))
    {
        return FD_INVALID;
    }
    const size_t remaining = parent->length - parent->position;
    if (offset > remaining || count > remaining - offset)
    {
        return FD_RANGE;
    }
    /* Read everything from the parent before writing: out may alias it. */
    const unsigned char *window_base =
        parent->base ? parent->base + parent->position + offset : 0;
    out->base = window_base;
    out->length = count;
    out->position = 0;
    return FD_OK;
}

enum fd_status fd_encode(struct fd_record record, unsigned char *out,
                         size_t capacity)
{
    if (!out || (record.flags & ~FD_FLAGS_MASK))
    {
        return FD_INVALID;
    }
    if (capacity != FD_WIRE_BYTES)
    {
        return FD_RANGE;
    }
    const uint32_t bits = (uint32_t)record.value;
    out[WIRE_VERSION] = FD_WIRE_VERSION;
    out[WIRE_FLAGS] = record.flags;
    out[WIRE_COUNT_HIGH] = (unsigned char)(record.count >> CHAR_BIT);
    out[WIRE_COUNT_LOW] = (unsigned char)record.count;
    /* Big-endian: the most significant octet is stored first. */
    for (unsigned i = 0; i < FD_WIRE_PAYLOAD_BYTES; i++)
    {
        const unsigned shift = CHAR_BIT * (FD_WIRE_PAYLOAD_BYTES - 1 - i);
        out[WIRE_PAYLOAD + i] = (unsigned char)(bits >> shift);
    }
    return FD_OK;
}

/* Maps a 32-bit two's-complement pattern to its signed value arithmetically,
 * without an implementation-defined unsigned-to-signed conversion. Patterns
 * above INT32_MAX denote bits - 2^32 = -1 - (UINT32_MAX - bits). */
static int32_t value_from_bits(uint32_t bits)
{
    if (bits <= INT32_MAX)
    {
        return (int32_t)bits;
    }
    return -1 - (int32_t)(UINT32_MAX - bits);
}

enum fd_status fd_decode(const unsigned char *wire, size_t length,
                         struct fd_record *out)
{
    if (!out || !wire)
    {
        return FD_INVALID;
    }
    /* Exactly one header plus one payload; compose the checked extent rather
     * than comparing lengths ad hoc. */
    size_t extent;
    if (fd_extent(FD_WIRE_HEADER_BYTES, 1, FD_WIRE_PAYLOAD_BYTES, length,
                  &extent) != FD_OK ||
        extent != length)
    {
        return FD_RANGE;
    }
    struct fd_cursor cursor = {wire, length, 0};
    const unsigned char *header;
    const unsigned char *payload;
    if (fd_take(&cursor, FD_WIRE_HEADER_BYTES, &header) != FD_OK ||
        fd_take(&cursor, FD_WIRE_PAYLOAD_BYTES, &payload) != FD_OK)
    {
        return FD_RANGE;
    }
    if (header[WIRE_VERSION] != FD_WIRE_VERSION ||
        (header[WIRE_FLAGS] & ~FD_FLAGS_MASK))
    {
        return FD_INVALID;
    }
    uint32_t bits = 0;
    for (unsigned i = 0; i < FD_WIRE_PAYLOAD_BYTES; i++)
    {
        bits = (bits << CHAR_BIT) | payload[i];
    }
#ifdef FD_MUTATE_CODEC
    bits &= UINT32_C(0x7fffffff);
#endif
    /* Commit only after every check passed. */
    out->flags = header[WIRE_FLAGS];
    out->count = (uint16_t)(((uint16_t)header[WIRE_COUNT_HIGH] << CHAR_BIT) |
                            header[WIRE_COUNT_LOW]);
    out->value = value_from_bits(bits);
    return FD_OK;
}

enum fd_status fd_slot_acquire(struct fd_slot *slot,
                               struct fd_allocator allocator)
{
    if (!slot || !allocator.allocate || !allocator.deallocate)
    {
        return FD_INVALID;
    }
    if (slot->state != FD_EMPTY)
    {
        return FD_STATE;
    }
    void *storage = allocator.allocate(
        allocator.context, sizeof(struct fd_value), _Alignof(struct fd_value));
    if (!storage)
    {
        return FD_NOMEM;
    }
    slot->storage = storage;
    slot->allocator = allocator;
    slot->state = FD_ACQUIRED;
    slot->destroy = 0;
    slot->callback_context = 0;
    return FD_OK;
}

/* Ownership: a slot in any state but FD_EMPTY owns slot->storage. The
 * destructor runs only for values that finished initialization, and only once
 * because the state returns to FD_EMPTY below. */
void fd_slot_reset(struct fd_slot *slot)
{
    if (!slot || slot->state == FD_EMPTY)
    {
        return;
    }
    const int value_constructed =
        slot->state == FD_INITIALIZED || slot->state == FD_PUBLISHED;
    if (value_constructed && slot->destroy)
    {
        slot->destroy(slot->callback_context, slot->storage);
    }
    slot->allocator.deallocate(slot->allocator.context, slot->storage);
    slot->storage = 0;
    slot->state = FD_EMPTY;
    slot->allocator.context = 0;
    slot->allocator.allocate = 0;
    slot->allocator.deallocate = 0;
    slot->destroy = 0;
    slot->callback_context = 0;
}

enum fd_status fd_slot_initialize(struct fd_slot *slot,
                                  fd_initialize_fn initialize,
                                  fd_destroy_fn destroy, void *context)
{
    if (!slot || !initialize)
    {
        return FD_INVALID;
    }
    if (slot->state != FD_ACQUIRED)
    {
        return FD_STATE;
    }
    if (initialize(context, slot->storage))
    {
        /* The failed value was never constructed, so no destructor runs:
         * the state is still FD_ACQUIRED and the reset only frees storage. */
#ifndef FD_MUTATE_LIFECYCLE
        fd_slot_reset(slot);
#endif
        return FD_INIT_FAILED;
    }
    slot->destroy = destroy;
    slot->callback_context = context;
    slot->state = FD_INITIALIZED;
    return FD_OK;
}

enum fd_status fd_slot_publish(struct fd_slot *slot,
                               const struct fd_value **out)
{
    if (!slot || !out)
    {
        return FD_INVALID;
    }
    if (slot->state != FD_INITIALIZED && slot->state != FD_PUBLISHED)
    {
        return FD_STATE;
    }
    *out = slot->storage;
    slot->state = FD_PUBLISHED;
    return FD_OK;
}

void fd_transform(unsigned char *restrict destination,
                  const unsigned char *restrict source, size_t count,
                  unsigned char mask)
{
    for (size_t i = 0; i < count; i++)
    {
        destination[i] = (unsigned char)(source[i] ^ mask);
    }
}

enum fd_status fd_move(unsigned char *base, size_t length, size_t destination,
                       size_t source, size_t count)
{
    if (!base && length)
    {
        return FD_INVALID;
    }
    /* Validate offsets before forming any pointer; the subtractions cannot
     * wrap because both offsets are already <= length. */
    if (destination > length || source > length ||
        count > length - destination || count > length - source)
    {
        return FD_RANGE;
    }
    /* Snapshot semantics without a buffer: when the destination window starts
     * after the source, a forward copy would overwrite unread source bytes,
     * so copy from the end. */
#ifndef FD_MUTATE_OVERLAP
    if (destination > source)
    {
        for (size_t i = count; i > 0; i--)
        {
            base[destination + i - 1] = base[source + i - 1];
        }
    }
    else
#endif
    {
        for (size_t i = 0; i < count; i++)
        {
            base[destination + i] = base[source + i];
        }
    }
    return FD_OK;
}

void fd_pool_init(struct fd_pool *pool)
{
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        pool->entries[i].generation = 1;
        pool->entries[i].live = 0;
        pool->entries[i].retired = 0;
    }
}

enum fd_status fd_pool_acquire(struct fd_pool *pool, struct fd_value value,
                               struct fd_handle *out)
{
    if (!pool || !out)
    {
        return FD_INVALID;
    }
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        struct fd_entry *entry = &pool->entries[i];
        if (!entry->live && !entry->retired)
        {
            entry->value = value;
            entry->live = 1;
            out->owner = pool;
            out->index = i;
            out->generation = entry->generation;
            return FD_OK;
        }
    }
    return FD_FULL;
}

/* A handle names a live entry of this very pool at its current generation. The
 * index is checked before it is used to subscript. */
static int handle_valid(const struct fd_pool *pool, struct fd_handle handle)
{
    return pool && handle.owner == pool && handle.index < FD_POOL_CAPACITY &&
           pool->entries[handle.index].live &&
           pool->entries[handle.index].generation == handle.generation;
}

enum fd_status fd_pool_get(const struct fd_pool *pool, struct fd_handle handle,
                           struct fd_value *out)
{
    if (!pool || !out)
    {
        return FD_INVALID;
    }
    if (!handle_valid(pool, handle))
    {
        return FD_STALE;
    }
    *out = pool->entries[handle.index].value;
    return FD_OK;
}

enum fd_status fd_pool_release(struct fd_pool *pool, struct fd_handle handle)
{
    if (!pool)
    {
        return FD_INVALID;
    }
    if (!handle_valid(pool, handle))
    {
        return FD_STALE;
    }
    struct fd_entry *entry = &pool->entries[handle.index];
    entry->live = 0;
    /* Advancing the generation invalidates every outstanding handle to this
     * slot. At FD_GENERATION_MAX the slot is retired for good: wrapping would
     * let an ancient handle become valid again. */
#ifdef FD_MUTATE_HANDLES
    entry->generation =
        entry->generation == FD_GENERATION_MAX ? 1 : entry->generation + 1;
#else
    if (entry->generation == FD_GENERATION_MAX)
    {
        entry->retired = 1;
    }
    else
    {
        entry->generation++;
    }
#endif
    return FD_OK;
}

enum fd_status fd_apply(const unsigned char *wire, size_t length,
                        int authorized, int32_t *synthetic_counter)
{
    if (!synthetic_counter)
    {
        return FD_INVALID;
    }
    struct fd_record record;
    const enum fd_status status = fd_decode(wire, length, &record);
    if (status != FD_OK)
    {
        return status;
    }
    /* count <= UINT16_MAX and |value| <= 2^31, so the product fits int64_t,
     * and so does its sum with any int32_t counter. */
    const int64_t result =
        (int64_t)*synthetic_counter + (int64_t)record.count * record.value;
    if (result < INT32_MIN || result > INT32_MAX)
    {
        return FD_RANGE;
    }
#ifdef FD_MUTATE_DEFENSE
    *synthetic_counter = (int32_t)result;
#endif
    /* Authority is the caller's argument, never a wire flag. */
#ifdef FD_MUTATE_FLAG_AUTHORITY
    if (!authorized && !(record.flags & 1u))
    {
        return FD_DENIED;
    }
#else
    if (!authorized)
    {
        return FD_DENIED;
    }
#endif
    *synthetic_counter = (int32_t)result;
    return FD_OK;
}

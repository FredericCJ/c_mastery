/* Assessor reference implementations of the withheld changed-condition
 * interfaces declared in transfer.h. FD_TRANSFER_* macros select compiled
 * defects for the transfer mutation controls; none is defined in a normal
 * build. */
#include "transfer.h"

/* Version-2 wire record: little-endian, unlike the version-1 record in
 * depth.c. Octet 0 is the version, octet 1 is reserved zero, octets 2-3 the
 * 16-bit count and octets 4-7 the 32-bit signed value. */
enum wire_v2_offset
{
    V2_VERSION = 0,
    V2_RESERVED = 1,
    V2_COUNT_LOW = 2,
    V2_COUNT_HIGH = 3,
    V2_VALUE = 4
};

#define V2_VERSION_TAG 2u

/* Tagged read: marker octet, then tag version octet. */
#define TAG_MARKER 0xa5u
#define TAG_VERSION 2u
#define TAG_BYTES 2u
#define TAG_VALUE_BYTES 2u

enum fd_status fd_owner_subview(const struct fd_owner_view *source,
                                size_t offset, size_t count,
                                struct fd_owner_view *out)
{
    if (!source || !out || source->begin > source->end ||
        source->end > source->owner_length ||
        source->position > source->end - source->begin ||
        (!source->owner && source->owner_length))
    {
        return FD_INVALID;
    }
    const size_t remaining = source->end - source->begin - source->position;
    if (offset > remaining || count > remaining - offset)
    {
        return FD_RANGE;
    }
    /* Build the result aside: out may alias source. */
    struct fd_owner_view next = *source;
    next.begin += next.position + offset;
    next.end = next.begin + count;
    next.position = 0;
    *out = next;
    return FD_OK;
}

enum fd_status fd_read_tagged(struct fd_cursor *source, uint16_t *out)
{
    if (!source || !out)
    {
        return FD_INVALID;
    }
    /* Work on a copy so that any failure leaves the caller's cursor alone. */
    struct fd_cursor trial = *source;
    const unsigned char *part;
    enum fd_status status = fd_take(&trial, TAG_BYTES, &part);
    if (status != FD_OK)
    {
        return status;
    }
    if (part[0] != TAG_MARKER || part[1] != TAG_VERSION)
    {
        return FD_INVALID;
    }
#ifdef FD_TRANSFER_TAG_COMMIT
    *source = trial;
#endif
    status = fd_take(&trial, TAG_VALUE_BYTES, &part);
    if (status != FD_OK)
    {
        return status;
    }
    const uint16_t value =
        (uint16_t)((uint16_t)part[0] | (uint16_t)((uint16_t)part[1] << 8));
    *source = trial;
    *out = value;
    return FD_OK;
}

enum fd_status fd_least_encode(struct fd_least_record record,
                               unsigned char *out, size_t capacity)
{
    return fd_encode_v2(record.count, record.value, out, capacity);
}

enum fd_status fd_least_decode(const unsigned char *wire, size_t length,
                               struct fd_least_record *out)
{
    if (!out)
    {
        return FD_INVALID;
    }
    uintmax_t count;
    intmax_t value;
    const enum fd_status status = fd_decode_v2(wire, length, &count, &value);
    if (status != FD_OK)
    {
        return status;
    }
    /* Decoded ranges are a subset of each least-width destination range. */
    *out =
        (struct fd_least_record){(uint_least16_t)count, (int_least32_t)value};
    return FD_OK;
}

enum fd_status fd_inplace_xor(unsigned char *base, size_t length,
                              unsigned char mask)
{
    if (!base && length)
    {
        return FD_INVALID;
    }
    for (size_t i = 0; i < length; ++i)
    {
        base[i] = (unsigned char)(base[i] ^ mask);
    }
    return FD_OK;
}

enum fd_status fd_pool_borrow(const struct fd_pool *pool,
                              struct fd_handle handle,
                              const struct fd_value **out)
{
    if (!out)
    {
        return FD_INVALID;
    }
    /* fd_pool_get validates owner, index and generation; its copy is only
     * used to reuse that check without repeating it here. */
    struct fd_value validated_copy;
    const enum fd_status status = fd_pool_get(pool, handle, &validated_copy);
    if (status != FD_OK)
    {
        return status;
    }
    *out = &pool->entries[handle.index].value;
    return FD_OK;
}

enum fd_status fd_registry_relocate(struct fd_registry *registry, size_t slot,
                                    struct fd_pool *destination)
{
    if (!registry || slot >= FD_REGISTRY_SLOTS || !destination)
    {
        return FD_INVALID;
    }
    struct fd_registry_entry *entry = &registry->entries[slot];
    if (!entry->owner || entry->retired)
    {
        return FD_STATE;
    }
    for (size_t i = 0; i < FD_REGISTRY_SLOTS; ++i)
    {
        if (registry->entries[i].owner == destination)
        {
            return FD_STATE;
        }
    }
    struct fd_pool *old_owner = entry->owner;
    *destination = *old_owner;
    /* Single ownership: the copied entries stay live only in the destination,
     * so the old pool is emptied and retired. */
#ifndef FD_TRANSFER_OWNER_DUPLICATE
    for (size_t i = 0; i < FD_POOL_CAPACITY; ++i)
    {
        old_owner->entries[i].live = 0;
        old_owner->entries[i].retired = 1;
    }
#endif
    entry->owner = destination;
    return FD_OK;
}

enum fd_status fd_effect(enum fd_effect effect, const unsigned char *wire,
                         size_t length, int authority, int32_t *counter,
                         int32_t *observed)
{
    if (!counter || !observed || (effect != FD_QUERY && effect != FD_UPDATE))
    {
        return FD_INVALID;
    }
    struct fd_record record;
    enum fd_status status = fd_decode(wire, length, &record);
    if (status != FD_OK)
    {
        return status;
    }
    /* A query only reads, so by default it needs no authority. */
#ifdef FD_TRANSFER_QUERY_AUTH
    if (!authority)
    {
        return FD_DENIED;
    }
#endif
    if (effect == FD_UPDATE)
    {
        status = fd_apply(wire, length, authority, counter);
        if (status != FD_OK)
        {
            return status;
        }
    }
    *observed = *counter;
    return FD_OK;
}

struct fd_step fd_step_value(int value)
{
    struct fd_step result = {FD_RANGE, value, value};
    if (value != INT_MAX)
    {
        result.status = FD_OK;
        result.new_value = value + 1;
    }
    return result;
}

enum fd_status fd_layout(size_t header, intmax_t first_count,
                         size_t first_width, intmax_t second_count,
                         size_t second_width, size_t limit,
                         struct fd_layout *out)
{
    if (!out)
    {
        return FD_INVALID;
    }
    size_t first_n;
    size_t second_n;
    if (fd_from_signed(first_count, &first_n) != FD_OK ||
        fd_from_signed(second_count, &second_n) != FD_OK)
    {
        return FD_RANGE;
    }
    size_t second_start;
    if (fd_extent(header, first_n, first_width, limit, &second_start) != FD_OK)
    {
        return FD_RANGE;
    }
    /* The second array begins where the first ends, not at the header. */
    size_t end;
#ifdef FD_TRANSFER_LAYOUT
    if (fd_extent(header, second_n, second_width, limit, &end) != FD_OK)
    {
        return FD_RANGE;
    }
#else
    if (fd_extent(second_start, second_n, second_width, limit, &end) != FD_OK)
    {
        return FD_RANGE;
    }
#endif
    *out = (struct fd_layout){header, second_start, end};
    return FD_OK;
}

enum fd_status fd_copy_segments(struct fd_segmented *cursor, size_t count,
                                unsigned char *destination, size_t capacity)
{
    if (!cursor || (!destination && count))
    {
        return FD_INVALID;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (!cursor->segments[i].base && cursor->segments[i].length)
        {
            return FD_INVALID;
        }
    }
    /* total = first length + 1 * second length, without wrapping. */
    size_t total;
    if (fd_extent(cursor->segments[0].length, 1, cursor->segments[1].length,
                  SIZE_MAX, &total) != FD_OK ||
        cursor->position > total)
    {
        return FD_INVALID;
    }
    if (count > total - cursor->position || count > capacity)
    {
        return FD_RANGE;
    }
    const size_t first_length = cursor->segments[0].length;
    for (size_t i = 0; i < count; ++i)
    {
        const size_t index = cursor->position + i;
        destination[i] = index < first_length
                             ? cursor->segments[0].base[index]
                             : cursor->segments[1].base[index - first_length];
    }
    cursor->position += count;
    return FD_OK;
}

enum fd_status fd_encode_v2(uintmax_t count, intmax_t value, unsigned char *out,
                            size_t capacity)
{
    if (!out)
    {
        return FD_INVALID;
    }
    if (capacity != FD_WIRE_BYTES || count > UINT16_MAX || value < INT32_MIN ||
        value > INT32_MAX)
    {
        return FD_RANGE;
    }
    const uint32_t bits = (uint32_t)value;
    out[V2_VERSION] = V2_VERSION_TAG;
    out[V2_RESERVED] = 0;
    out[V2_COUNT_LOW] = (unsigned char)count;
    out[V2_COUNT_HIGH] = (unsigned char)(count >> CHAR_BIT);
    for (unsigned i = 0; i < FD_WIRE_PAYLOAD_BYTES; ++i)
    {
        out[V2_VALUE + i] = (unsigned char)(bits >> (CHAR_BIT * i));
    }
    return FD_OK;
}

enum fd_status fd_decode_v2(const unsigned char *wire, size_t length,
                            uintmax_t *count, intmax_t *value)
{
    if (!wire || !count || !value)
    {
        return FD_INVALID;
    }
    if (length != FD_WIRE_BYTES)
    {
        return FD_RANGE;
    }
    if (wire[V2_VERSION] != V2_VERSION_TAG || wire[V2_RESERVED] != 0)
    {
        return FD_INVALID;
    }
    uint32_t bits = 0;
    for (unsigned i = 0; i < FD_WIRE_PAYLOAD_BYTES; ++i)
    {
        bits |= (uint32_t)wire[V2_VALUE + i] << (CHAR_BIT * i);
    }
    *count = (uintmax_t)wire[V2_COUNT_LOW] +
             ((uintmax_t)wire[V2_COUNT_HIGH] << CHAR_BIT);
    /* Two's complement by arithmetic, as in the version-1 decoder. */
    *value =
        bits <= INT32_MAX ? (intmax_t)bits : -1 - (intmax_t)(UINT32_MAX - bits);
    return FD_OK;
}

void *fd_typed_allocate(void *context, size_t size_bytes, size_t alignment)
{
    struct fd_typed_storage *storage = context;
    if (!storage || storage->reserved || size_bytes != sizeof storage->value ||
        alignment != _Alignof(struct fd_value))
    {
        return 0;
    }
    storage->reserved = 1;
    return &storage->value;
}

void fd_typed_release(void *context, void *value)
{
    struct fd_typed_storage *storage = context;
    /* Only the exact outstanding element is admitted by this adapter. */
    if (storage && value == &storage->value)
    {
        storage->reserved = 0;
    }
}

enum fd_status fd_slot_transfer(struct fd_slot *destination,
                                struct fd_slot *source)
{
    if (!destination || !source || destination == source)
    {
        return FD_INVALID;
    }
    if (destination->state != FD_EMPTY || source->state == FD_EMPTY)
    {
        return FD_STATE;
    }
    /* Ownership moves: afterwards only the destination may reset or free. */
    *destination = *source;
    *source = (struct fd_slot)FD_SLOT_INITIALIZER;
    return FD_OK;
}

enum fd_status fd_stride_move(unsigned char *base, size_t physical_length,
                              size_t logical_length, size_t stride,
                              size_t destination, size_t source, size_t count)
{
    if ((!base && physical_length) || !stride)
    {
        return FD_INVALID;
    }
    /* The last logical element starts at (logical_length - 1) * stride and
     * needs one byte, so the extent is 1 + (logical_length - 1) * stride. */
    size_t extent = 0;
    if (logical_length && fd_extent(1, logical_length - 1, stride,
                                    physical_length, &extent) != FD_OK)
    {
        return FD_RANGE;
    }
    if (destination > logical_length || source > logical_length ||
        count > logical_length - destination || count > logical_length - source)
    {
        return FD_RANGE;
    }
    if (destination > source)
    {
        for (size_t i = count; i > 0; --i)
        {
            base[(destination + i - 1) * stride] =
                base[(source + i - 1) * stride];
        }
    }
    else
    {
        for (size_t i = 0; i < count; ++i)
        {
            base[(destination + i) * stride] = base[(source + i) * stride];
        }
    }
    return FD_OK;
}

enum fd_status fd_compact(struct fd_stream_view *stream)
{
    if (!stream || stream->position > stream->length ||
        (!stream->base && stream->length))
    {
        return FD_INVALID;
    }
    /* Refuse before moving anything: the epoch could not advance. */
    if (stream->epoch == SIZE_MAX)
    {
        return FD_FULL;
    }
    const size_t unread = stream->length - stream->position;
    const enum fd_status status =
        fd_move(stream->base, stream->length, 0, stream->position, unread);
    if (status != FD_OK)
    {
        return status;
    }
    stream->length = unread;
    stream->position = 0;
    ++stream->epoch;
    return FD_OK;
}

void fd_registry_init(struct fd_registry *registry)
{
    for (unsigned i = 0; i < FD_REGISTRY_SLOTS; ++i)
    {
        registry->entries[i] = (struct fd_registry_entry){0, 1, 0};
    }
}

enum fd_status fd_registry_bind(struct fd_registry *registry, size_t slot,
                                struct fd_pool *pool)
{
    if (!registry || !pool || slot >= FD_REGISTRY_SLOTS)
    {
        return FD_INVALID;
    }
    struct fd_registry_entry *entry = &registry->entries[slot];
    if (entry->retired)
    {
        return FD_FULL;
    }
    if (entry->owner)
    {
        return FD_STATE;
    }
    /* One current registry binding per owner. */
    for (unsigned i = 0; i < FD_REGISTRY_SLOTS; ++i)
    {
        if (registry->entries[i].owner == pool)
        {
            return FD_STATE;
        }
    }
    entry->owner = pool;
    return FD_OK;
}

enum fd_status fd_registry_unbind(struct fd_registry *registry, size_t slot)
{
    if (!registry || slot >= FD_REGISTRY_SLOTS)
    {
        return FD_INVALID;
    }
    struct fd_registry_entry *entry = &registry->entries[slot];
    if (!entry->owner)
    {
        return FD_STATE;
    }
    entry->owner = 0;
    /* Each binding gets a new epoch so older handles go stale. After the
     * last epoch the slot retires: wrapping would revive old handles. */
#ifdef FD_TRANSFER_EPOCH
    entry->epoch = entry->epoch == FD_EPOCH_MAX ? 1 : entry->epoch + 1;
#else
    if (entry->epoch == FD_EPOCH_MAX)
    {
        entry->retired = 1;
    }
    else
    {
        ++entry->epoch;
    }
#endif
    return FD_OK;
}

enum fd_status fd_registry_acquire(struct fd_registry *registry, size_t slot,
                                   struct fd_value value,
                                   struct fd_registry_handle *out)
{
    if (!registry || !out || slot >= FD_REGISTRY_SLOTS)
    {
        return FD_INVALID;
    }
    const struct fd_registry_entry *entry = &registry->entries[slot];
    if (!entry->owner)
    {
        return FD_STATE;
    }
    struct fd_handle local;
    const enum fd_status status = fd_pool_acquire(entry->owner, value, &local);
    if (status != FD_OK)
    {
        return status;
    }
    *out = (struct fd_registry_handle){slot, local.index, entry->epoch,
                                       local.generation};
    return FD_OK;
}

enum fd_status fd_registry_get(const struct fd_registry *registry,
                               struct fd_registry_handle handle,
                               struct fd_value *out)
{
    if (!registry || !out)
    {
        return FD_INVALID;
    }
    if (handle.registry_slot >= FD_REGISTRY_SLOTS)
    {
        return FD_STALE;
    }
    const struct fd_registry_entry *entry =
        &registry->entries[handle.registry_slot];
    if (!entry->owner || entry->retired || entry->epoch != handle.epoch)
    {
        return FD_STALE;
    }
    const struct fd_handle local = {entry->owner, handle.local_slot,
                                    handle.generation};
    return fd_pool_get(entry->owner, local, out);
}

void fd_frame_cancel(struct fd_frame *frame)
{
    if (!frame)
    {
        return;
    }
    frame->used = 0;
#ifndef FD_TRANSFER_AUTHORITY
    frame->authority = 0;
#endif
    for (unsigned i = 0; i < FD_WIRE_BYTES; ++i)
    {
        frame->bytes[i] = 0;
    }
}

enum fd_status fd_frame_feed(struct fd_frame *frame, const unsigned char *bytes,
                             size_t count, int authority, int32_t *counter,
                             int *committed)
{
    if (!frame || !counter || !committed || (!bytes && count) ||
        frame->used > FD_WIRE_BYTES)
    {
        return FD_INVALID;
    }
    if (count > FD_WIRE_BYTES - frame->used)
    {
        return FD_RANGE;
    }
    *committed = 0;
    if (!count)
    {
        return FD_OK;
    }
    /* Authority is latched from the first chunk of a record and ignored in
     * later chunks, so a late caller-supplied value cannot upgrade it. */
#ifdef FD_TRANSFER_AUTHORITY
    if (!frame->used)
    {
        frame->authority = frame->authority || authority;
    }
#else
    if (!frame->used)
    {
        frame->authority = authority != 0;
    }
#endif
    for (size_t i = 0; i < count; ++i)
    {
        frame->bytes[frame->used + i] = bytes[i];
    }
    frame->used += count;
    if (frame->used < FD_WIRE_BYTES)
    {
        return FD_OK;
    }
    const enum fd_status status =
        fd_apply(frame->bytes, FD_WIRE_BYTES, frame->authority, counter);
    fd_frame_cancel(frame);
    if (status == FD_OK)
    {
        *committed = 1;
    }
    return status;
}

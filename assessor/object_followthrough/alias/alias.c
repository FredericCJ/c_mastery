/*
 * Reference implementation of the alias laboratory (private assessor copy).
 * Contracts are stated in labs/object_followthrough/alias/alias.h; this file
 * adds only the implementation notes.
 *
 * Test hooks: the OS_MUTANT_* macros each break exactly one contract so the
 * campaign can prove that the tests reject the breakage.
 */
#include "alias.h"

/* Wire format of a value: two big-endian unsigned 32-bit integers. */
enum
{
    OS_WIRE_INTEGER_BYTES = 4,
    OS_WIRE_RECORD_BYTES = 2 * OS_WIRE_INTEGER_BYTES,
    OS_OCTET_BITS = 8
};

/* The tagging experiment keeps a two-bit tag (values 0..3) in the low bits
 * of a four-byte-aligned address. */
#define OS_TAG_MASK ((uintptr_t)3u)
#define OS_TAG_ALIGNMENT_BYTES 4u

/*
 * An allocator is admitted for a request when both callbacks exist, its
 * promised capacity covers the size, and its promised alignment is a
 * multiple of the one requested (a stronger power-of-two alignment satisfies
 * a weaker one). alignment_bytes must be nonzero: it comes from _Alignof.
 */
static int admits(struct os_allocator a, size_t size_bytes,
                  size_t alignment_bytes)
{
    if (a.allocator.allocate == NULL || a.allocator.deallocate == NULL)
    {
        return 0;
    }
    if (a.capacity < size_bytes)
    {
        return 0;
    }
    return a.alignment >= alignment_bytes && a.alignment % alignment_bytes == 0;
}

int os_acquire(struct fd_slot *slot, struct os_allocator a)
{
    if (slot == NULL ||
        !admits(a, sizeof(struct fd_value), _Alignof(struct fd_value)))
    {
        return -1;
    }
    return fd_slot_acquire(slot, a.allocator) == FD_OK ? 0 : -1;
}

int os_wide_acquire(struct os_wide_slot *slot, struct os_allocator a, int value,
                    os_drop_fn drop, void *context)
{
    if (slot == NULL || slot->state != OS_EMPTY || drop == NULL ||
        !admits(a, sizeof(struct os_wide), _Alignof(struct os_wide)))
    {
        return -1;
    }
    struct os_wide *wide = a.allocator.allocate(
        a.allocator.context, sizeof(*wide), _Alignof(struct os_wide));
    if (wide == NULL)
    {
        return -1;
    }
    /* The allocation has no declared type yet: fields are typed writes. */
    wide->value = value;
    wide->resource = NULL;
    /* Publish into the slot only after every step that can fail is done. */
    slot->value = wide;
    slot->allocation = a;
    slot->drop = drop;
    slot->context = context;
    slot->state = OS_LIVE;
    return 0;
}

int os_wide_borrow(struct os_wide_slot *slot, const struct os_wide **out)
{
    if (slot == NULL || out == NULL || slot->state != OS_LIVE)
    {
        return -1;
    }
    *out = slot->value;
    slot->state = OS_BORROWED;
    return 0;
}

int os_wide_return(struct os_wide_slot *slot, const struct os_wide **borrowed)
{
    if (slot == NULL || borrowed == NULL || slot->state != OS_BORROWED ||
        *borrowed != slot->value)
    {
        return -1;
    }
    *borrowed = NULL;
    slot->state = OS_LIVE;
    return 0;
}

/*
 * Invariant: the allocation is released only after drop() succeeds, and the
 * slot keeps value, drop and context until then, so a failed destructor can
 * be retried with exactly the resources it needs.
 */
int os_wide_reset(struct os_wide_slot *slot)
{
    if (slot == NULL)
    {
        return -1;
    }
    if (slot->state == OS_EMPTY)
    {
        return 0;
    }
    if (slot->state == OS_BORROWED)
    {
        return -1;
    }
    slot->state = OS_CLEANUP;
#ifdef OS_MUTANT_DESTRUCTOR
    (void)slot->drop(slot->context, slot->value);
#else
    if (slot->drop(slot->context, slot->value) != 0)
    {
        return -1;
    }
#endif
    slot->allocation.allocator.deallocate(slot->allocation.allocator.context,
                                          slot->value);
    slot->value = NULL;
    slot->state = OS_EMPTY;
    slot->drop = NULL;
    slot->context = NULL;
    return 0;
}

int os_clone(void *allocated, const struct fd_value *source)
{
    if (allocated == NULL || source == NULL)
    {
        return -1;
    }
    /* Character-type access: the destination has no declared type yet. */
    unsigned char *to = allocated;
    const unsigned char *from = (const unsigned char *)source;
    for (size_t i = 0; i < sizeof(*source); ++i)
    {
        to[i] = from[i];
    }
    return 0;
}

uint32_t os_wire_integer(const unsigned char *bytes)
{
    uint32_t number = 0;
    for (size_t i = 0; i < OS_WIRE_INTEGER_BYTES; ++i)
    {
        number = (number << OS_OCTET_BITS) | bytes[i];
    }
    return number;
}

int os_wire(const unsigned char *bytes, size_t count, struct fd_value *out)
{
    if (bytes == NULL || out == NULL || count != OS_WIRE_RECORD_BYTES)
    {
        return -1;
    }
    uint32_t number = os_wire_integer(bytes);
    if (number > INT32_MAX)
    {
        return -1;
    }
    struct fd_value decoded;
    decoded.value = (int32_t)number;
    decoded.serial = os_wire_integer(bytes + OS_WIRE_INTEGER_BYTES);
#ifdef OS_MUTANT_WIRE
    decoded.value = bytes[0];
#endif
    *out = decoded;
    return 0;
}

uint32_t os_host_representation(uint32_t value)
{
    uint32_t copy;
    unsigned char *to = (unsigned char *)&copy;
    const unsigned char *from = (const unsigned char *)&value;
    for (size_t i = 0; i < sizeof value; ++i)
    {
        to[i] = from[i];
    }
    return copy;
}

/*
 * The four transforms below are deliberately the same loop with different
 * pointer qualification; the campaign compares their object code and timing.
 * Keep them textually parallel.
 */
void os_transform(unsigned char *restrict dst,
                  const unsigned char *restrict src, size_t count,
                  unsigned char mask)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = (unsigned char)(src[i] ^ mask);
    }
}

void os_ordinary(unsigned char *dst, const unsigned char *src, size_t count,
                 unsigned char mask)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = (unsigned char)(src[i] ^ mask);
    }
}

void os_nested(unsigned char *restrict dst, const unsigned char *restrict src,
               size_t count, unsigned char mask)
{
    /* Variant: the restrict-qualified pointers are declared in a nested
     * block instead of (only) on the parameters. */
    {
        unsigned char *restrict inner_dst = dst;
        const unsigned char *restrict inner_src = src;
        for (size_t i = 0; i < count; ++i)
        {
            inner_dst[i] = (unsigned char)(inner_src[i] ^ mask);
        }
    }
}

void os_shared(unsigned char *restrict dst, const unsigned char *restrict left,
               const unsigned char *restrict right, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = (unsigned char)(left[i] ^ right[i]);
    }
}

/* dst, src and count are plain integers; no pointer is formed from them
 * until both ranges are known to lie inside [0, length_bytes]. */
static int partition_in_bounds(size_t length_bytes, size_t dst, size_t src,
                               size_t count)
{
    return dst <= length_bytes && src <= length_bytes &&
           count <= length_bytes - dst && count <= length_bytes - src;
}

/* Two ranges of count > 0 bytes starting at different offsets share bytes
 * exactly when the distance between their starts is less than count. */
static int partition_overlaps(size_t dst, size_t src, size_t count)
{
    size_t distance = dst < src ? src - dst : dst - src;
    return count > distance;
}

int os_partition(unsigned char *base, size_t length_bytes, size_t dst,
                 size_t src, size_t count, unsigned char mask, int inplace)
{
    if ((base == NULL && length_bytes != 0) ||
        !partition_in_bounds(length_bytes, dst, src, count))
    {
        return -1;
    }
    if (count == 0)
    {
        return 0;
    }
    if (dst == src)
    {
        /* Identical ranges: allowed only as an explicit separate in-place
         * loop, never through the restrict-qualified transform. */
        if (!inplace)
        {
            return -1;
        }
        for (size_t i = 0; i < count; ++i)
        {
            base[dst + i] ^= mask;
        }
        return 0;
    }
    if (partition_overlaps(dst, src, count))
    {
#ifdef OS_MUTANT_PARTITION
        /* Wrong success without touching a conflicting restricted range. */
        return 0;
#else
        return -1;
#endif
    }
    os_transform(base + dst, base + src, count, mask);
    return 0;
}

enum fd_status os_move(unsigned char *base, size_t length_bytes, size_t dst,
                       size_t src, size_t count)
{
    return fd_move(base, length_bytes, dst, src, count);
}

int os_member(struct os_container *owner, int *member, int *out)
{
    if (owner == NULL || member == NULL || out == NULL)
    {
        return -1;
    }
    /* The member pointer is trusted only if it is the member's own address
     * inside this owner: compare it with the address computed from owner. */
    const unsigned char *whole = (const unsigned char *)owner;
    int *expected =
        (int *)(void *)(whole + offsetof(struct os_container, member));
    if (member != expected)
    {
        return -1;
    }
    *out = owner->member;
    return 0;
}

#ifdef UINTPTR_MAX
int os_tag_roundtrip(struct os_container *owner, unsigned tag,
                     uintptr_t *representation, int *out)
{
    if (owner == NULL || representation == NULL || out == NULL ||
        tag > OS_TAG_MASK ||
        _Alignof(struct os_container) < OS_TAG_ALIGNMENT_BYTES)
    {
        return -1;
    }
    uintptr_t raw = (uintptr_t)(void *)owner;
    if ((raw & OS_TAG_MASK) != 0)
    {
        return -1;
    }
    uintptr_t tagged = raw | tag;
    uintptr_t restored = tagged & ~OS_TAG_MASK;
    if (restored != raw)
    {
        return -1;
    }
    void *round_trip = (void *)restored;
    if (round_trip != owner)
    {
        return -1;
    }
    *representation = tagged;
    *out = owner->member;
    return 0;
}
#endif

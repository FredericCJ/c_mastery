/*
 * Reference implementation of the atlas laboratory (private assessor copy).
 * Contracts are stated in labs/object_followthrough/atlas/atlas.h and
 * CONTRACTS.md; this file adds implementation notes only.
 *
 * Test hooks: each OA_MUTANT_* macro breaks exactly one contract so the
 * campaign can prove that the tests reject the breakage.
 */
#include "atlas.h"
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

/* Wire format of oa_encode: marker, big-endian number, big-endian binary64. */
enum
{
    OA_MARKER_BYTES = 1,
    OA_NUMBER_BYTES = 4,
    OA_MEASURE_BYTES = 8,
    OA_ENCODED_BYTES = OA_MARKER_BYTES + OA_NUMBER_BYTES + OA_MEASURE_BYTES,
    OA_LABEL_BYTES = 4, /* "cat" and its terminator */
    OA_OCTET_BITS = 8
};

#define OA_QUIET_NAN_BITS UINT64_C(0x7ff8000000000000)
#define OA_BINARY64_ONE_BITS UINT64_C(0x3ff0000000000000)

static int is_power_of_two(size_t value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

/*
 * Copies the first min(from_bytes, to_bytes) bytes and zero-fills the rest of
 * the destination. The two blocks are distinct allocations.
 */
static void copy_prefix_zero_rest(unsigned char *to, size_t to_bytes,
                                  const unsigned char *from, size_t from_bytes)
{
    size_t copied = from_bytes < to_bytes ? from_bytes : to_bytes;
    size_t i = 0;
    for (; i < copied; ++i)
    {
        to[i] = from[i];
    }
    for (; i < to_bytes; ++i)
    {
        to[i] = 0;
    }
}

/*
 * Order matters for failure atomicity: the new block is obtained first and
 * the owner is modified only after every fallible step has succeeded.
 */
int oa_resize(struct oa_owner *owner, size_t bytes,
              const struct oa_allocator *allocator)
{
    if (owner == NULL || allocator == NULL || allocator->get == NULL ||
        allocator->put == NULL || bytes == 0 || owner->generation == UINT_MAX ||
        (owner->data == NULL && owner->length != 0))
    {
        return -1;
    }
    unsigned char *next = allocator->get(allocator->context, bytes);
    if (next == NULL)
    {
#ifdef OA_MUTANT_FAIL
        owner->length = 0;
#endif
        return -1;
    }
    copy_prefix_zero_rest(next, bytes, owner->data, owner->length);
    if (owner->data != NULL)
    {
        allocator->put(allocator->context, owner->data);
    }
    owner->data = next;
    owner->length = bytes;
    ++owner->generation;
    return 0;
}

void oa_release(struct oa_owner *owner, const struct oa_allocator *allocator)
{
    if (owner == NULL || allocator == NULL || allocator->put == NULL)
    {
        return;
    }
    if (owner->data != NULL)
    {
        allocator->put(allocator->context, owner->data);
    }
    owner->data = NULL;
    owner->length = 0;
}

int oa_deep_copy(struct oa_view source, char *dest, size_t capacity_bytes,
                 struct oa_view *out)
{
    if (out == NULL || dest == NULL || source.data == NULL ||
        source.length > capacity_bytes)
    {
        return -1;
    }
#ifdef OA_MUTANT_BORROW
    *out = source;
    return 0;
#else
    for (size_t i = 0; i < source.length; ++i)
    {
        dest[i] = source.data[i];
    }
    out->data = dest;
    out->length = source.length;
    return 0;
#endif
}

int oa_label(char *dest, size_t capacity_bytes, struct oa_view *out)
{
    /* Built in a local array and copied: the result must name dest, never
     * the callee's own storage. */
    const char text[OA_LABEL_BYTES] = {'c', 'a', 't', '\0'};
    if (out == NULL || dest == NULL || capacity_bytes < OA_LABEL_BYTES)
    {
        return -1;
    }
    for (size_t i = 0; i < OA_LABEL_BYTES; ++i)
    {
        dest[i] = text[i];
    }
    out->data = dest;
    out->length = OA_LABEL_BYTES - 1;
    return 0;
}

/*
 * Application equality: member-wise, never by object bytes (padding is
 * unspecified). The measure comparison treats NaN specially because NaN
 * compares unequal to itself under ==.
 */
int oa_equal(const struct oa_record *left, const struct oa_record *right)
{
    if (left == NULL || right == NULL)
    {
        return 0;
    }
#ifdef OA_MUTANT_BYTES
    return memcmp(left, right, sizeof *left) == 0;
#else
    if (left->marker != right->marker || left->number != right->number)
    {
        return 0;
    }
    return left->measure == right->measure ||
           (isnan(left->measure) && isnan(right->measure));
#endif
}

/* True when the encoding's target premises hold. Check 1 (compile-time
 * parameters): CHAR_BIT, sizeof(double), FLT_RADIX, DBL_MANT_DIG and
 * DBL_MAX_EXP describe 8-bit bytes and an 8-byte binary64 double. Check 2
 * (representation): the bytes of 1.0 are 0x3ff0000000000000. Neither check
 * alone proves the whole floating layout, so both are made. */
static int target_model_supported(void)
{
    if (CHAR_BIT != OA_OCTET_BITS || sizeof(double) != OA_MEASURE_BYTES ||
        FLT_RADIX != 2 || DBL_MANT_DIG != 53 || DBL_MAX_EXP != 1024)
    {
        return 0;
    }
    const double one = 1.0;
    uint64_t one_bits;
    memcpy(&one_bits, &one, sizeof one_bits);
    return one_bits == OA_BINARY64_ONE_BITS;
}

/* The normalized bit pattern of a measure; see CONTRACTS.md for the policy. */
static uint64_t canonical_measure_bits(double measure)
{
    uint64_t bits;
    memcpy(&bits, &measure, sizeof bits);
    if (isnan(measure))
    {
        return OA_QUIET_NAN_BITS;
    }
#ifndef OA_MUTANT_ZERO
    if (measure == 0.0)
    {
        return 0;
    }
#endif
    return bits;
}

/* Stores the low `bytes` octets of value, most significant first. */
static void store_big_endian(unsigned char *out, uint64_t value, size_t bytes)
{
    for (size_t i = 0; i < bytes; ++i)
    {
        size_t shift_bits = OA_OCTET_BITS * (bytes - 1 - i);
        out[i] = (unsigned char)(value >> shift_bits);
    }
}

int oa_encode(const struct oa_record *record, unsigned char *out,
              size_t capacity_bytes)
{
    if (record == NULL || out == NULL || capacity_bytes != OA_ENCODED_BYTES)
    {
        return -1;
    }
    if (!target_model_supported())
    {
        return -2;
    }
    /* Encode into a local buffer so that out is untouched on any failure. */
    unsigned char encoded[OA_ENCODED_BYTES];
    encoded[0] = (unsigned char)record->marker;
    store_big_endian(encoded + OA_MARKER_BYTES, (uint32_t)record->number,
                     OA_NUMBER_BYTES);
    store_big_endian(encoded + OA_MARKER_BYTES + OA_NUMBER_BYTES,
                     canonical_measure_bits(record->measure), OA_MEASURE_BYTES);
    for (size_t i = 0; i < OA_ENCODED_BYTES; ++i)
    {
        out[i] = encoded[i];
    }
    return 0;
}

/*
 * The slice [start, start + length) is checked against the region before any
 * arithmetic that could wrap; padding is then at most alignment - 1 and is
 * compared with length, so start + padding cannot overflow.
 */
int oa_place(size_t total_bytes, size_t start, size_t length,
             size_t width_bytes, size_t alignment_bytes, size_t *out)
{
    if (out == NULL || !is_power_of_two(alignment_bytes) ||
        start > total_bytes || length > total_bytes - start)
    {
        return -1;
    }
    size_t padding =
        (alignment_bytes - start % alignment_bytes) % alignment_bytes;
#ifdef OA_MUTANT_SLICE
    padding = 0;
#endif
    if (padding > length || width_bytes > length - padding)
    {
        return -1;
    }
    *out = start + padding;
    return 0;
}

int oa_slot(struct oa_storage *storage, size_t offset_bytes,
            size_t capacity_bytes, struct oa_record **out)
{
    const size_t record_bytes = sizeof(struct oa_record);
    if (storage == NULL || out == NULL || offset_bytes % record_bytes != 0 ||
        offset_bytes > sizeof(*storage) - record_bytes ||
        capacity_bytes < record_bytes ||
        capacity_bytes > sizeof(*storage) - offset_bytes)
    {
        return -1;
    }
    *out = &storage->records[offset_bytes / record_bytes];
    return 0;
}

int oa_wide_admit(size_t guarantee_bytes)
{
#ifdef OA_MUTANT_WIDE
    return guarantee_bytes >= _Alignof(max_align_t);
#else
    return is_power_of_two(guarantee_bytes) &&
           guarantee_bytes >= _Alignof(struct oa_wide);
#endif
}

int oa_decode_u32(const unsigned char *bytes, size_t length_bytes,
                  uint32_t *out)
{
    if (bytes == NULL || out == NULL || length_bytes < OA_NUMBER_BYTES ||
        CHAR_BIT != OA_OCTET_BITS)
    {
        return -1;
    }
    /* Byte-wise assembly: never forms a (possibly misaligned) uint32_t *. */
    uint32_t value = 0;
    for (size_t i = 0; i < OA_NUMBER_BYTES; ++i)
    {
        value = (value << OA_OCTET_BITS) | bytes[i];
    }
    *out = value;
    return 0;
}

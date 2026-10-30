/*
 * Observation of this compiler's struct, bit-field and packing layout, set
 * against the portable explicit codec in bits.h.
 *
 * Nothing printed here is a portable golden result: padding bytes, bit-field
 * placement and alignment are implementation-defined and depend on the
 * compiler, ABI and options (-DFB_PACK=1 wraps the structs in #pragma pack).
 * The only portable claims are that the codec round-trips the value and that
 * a host-endian reinterpretation of wire octets differs from it.
 */
#include "bits.h"
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if FB_PACK
#pragma pack(push, 1)
#endif
/* 3 + 5 + 9 bits: whether they share one unsigned int is implementation-
 * defined. */
struct layout_fields
{
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 9;
};

/* A bit-field of type unsigned long is a documented compiler extension. */
struct layout_wide_fields
{
    unsigned long a : 3;
    unsigned long b : 5;
    unsigned long c : 9;
};

struct layout_record
{
    unsigned char tag;
    uint32_t value;
    unsigned short count;
};

/* Same members in a different order: padding moves with them. */
struct layout_reordered
{
    unsigned short count;
    uint32_t value;
    unsigned char tag;
};
#if FB_PACK
#pragma pack(pop)
#endif

/* The same members with no padding, by a compiler attribute. */
struct __attribute__((packed)) layout_packed
{
    unsigned char tag;
    uint32_t value;
    unsigned short count;
};

#define WIRE_OCTETS 4u
#define SAMPLE_TAG 1u
#define SAMPLE_COUNT 2u
#define SAMPLE_VALUE UINT32_C(0x12345678)

/* Octets of the 32-bit sample value: 0x12 0x34 0x56 0x78. */
#define SAMPLE_FIRST_OCTET_BIG 18u
#define SAMPLE_FIRST_OCTET_LITTLE 120u

static void dump(const char *name, const void *object, size_t size_bytes,
                 size_t alignment_bytes)
{
    const unsigned char *bytes = object;
    printf("LAYOUT %s size=%zu align=%zu bytes=", name, size_bytes,
           alignment_bytes);
    for (size_t i = 0; i < size_bytes; ++i)
    {
        printf("%02x", bytes[i]);
    }
    putchar('\n');
}

/* The codec must give the documented octets in both orders and round-trip. */
static int check_codec(uint32_t value, fb_word *decoded)
{
    unsigned char big[WIRE_OCTETS];
    unsigned char little[WIRE_OCTETS];
    const struct fb_span big_span = {big, WIRE_OCTETS};

    if ((fb_encode(value, FB_BIG, big, WIRE_OCTETS) != FB_OK) ||
        (fb_encode(value, FB_LITTLE, little, WIRE_OCTETS) != FB_OK) ||
        (big[0] != SAMPLE_FIRST_OCTET_BIG) ||
        (little[0] != SAMPLE_FIRST_OCTET_LITTLE))
    {
        return 1;
    }
    if ((fb_decode(big_span, FB_BIG, decoded) != FB_OK) || (*decoded != value))
    {
        return 1;
    }
    return 0;
}

int main(void)
{
    _Static_assert(CHAR_BIT == 8 && sizeof(uint32_t) == 4 &&
                       sizeof(unsigned short) == 2,
                   "recorded native layout model");

    struct layout_fields fields;
    struct layout_wide_fields wide;
    struct layout_record record;
    struct layout_reordered reordered;
    struct layout_packed packed;
    /* Zero everything first so padding bytes print as 00 rather than as
     * indeterminate stack contents. */
    memset(&fields, 0, sizeof fields);
    memset(&wide, 0, sizeof wide);
    memset(&record, 0, sizeof record);
    memset(&reordered, 0, sizeof reordered);
    memset(&packed, 0, sizeof packed);

    fields.a = 5;
    fields.b = 17;
    fields.c = 257;
    wide.a = 5;
    wide.b = 17;
    wide.c = 257;
    record.tag = SAMPLE_TAG;
    reordered.tag = SAMPLE_TAG;
    packed.tag = SAMPLE_TAG;
    record.value = SAMPLE_VALUE;
    reordered.value = SAMPLE_VALUE;
    packed.value = SAMPLE_VALUE;
    record.count = SAMPLE_COUNT;
    reordered.count = SAMPLE_COUNT;
    packed.count = SAMPLE_COUNT;

    printf(
        "ABI char_bits=%d pointer=%zu uint=%zu ulong=%zu pack=%d edition=%ld\n",
        CHAR_BIT, sizeof(void *), sizeof(unsigned), sizeof(unsigned long),
        FB_PACK, (long)__STDC_VERSION__);
    dump("fields359", &fields, sizeof fields, _Alignof(struct layout_fields));
    dump("wide359", &wide, sizeof wide, _Alignof(struct layout_wide_fields));
    dump("record", &record, sizeof record, _Alignof(struct layout_record));
    dump("reordered", &reordered, sizeof reordered,
         _Alignof(struct layout_reordered));
    dump("packed", &packed, sizeof packed, _Alignof(struct layout_packed));
    printf(
        "OFFSETS record=%zu,%zu,%zu reordered=%zu,%zu,%zu packed=%zu,%zu,%zu\n",
        offsetof(struct layout_record, tag),
        offsetof(struct layout_record, value),
        offsetof(struct layout_record, count),
        offsetof(struct layout_reordered, tag),
        offsetof(struct layout_reordered, value),
        offsetof(struct layout_reordered, count),
        offsetof(struct layout_packed, tag),
        offsetof(struct layout_packed, value),
        offsetof(struct layout_packed, count));

    fb_word decoded = 0;
    if (check_codec(record.value, &decoded) != 0)
    {
        return 1;
    }

    /* A declared packed object avoids forging a typed lvalue into a char
     * array (no effective-type or alignment violation). Its host-endian
     * interpretation of the copied wire octets still differs from the
     * big-endian codec: that difference is the observation. */
    const unsigned char wire[] = {1, 18, 52, 86, 120, 0, 2};
    memcpy(&packed, wire, sizeof packed);
    printf("PACKED_WIRE_OBS value=%ju count=%u portable_decoder=%ju\n",
           (uintmax_t)packed.value, (unsigned)packed.count, (uintmax_t)decoded);
    printf(
        "FB_LAYOUT_PASS values=%u,%u,%u no_padding_goldens no_bitfield_address\n",
        fields.a, fields.b, fields.c);
    return 0;
}

#include "atlas.h"
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void *allocations[8];

static void cleanup(void)
{
    for (size_t i = 0; i < 8; ++i)
    {
        if (allocations[i])
        {
            free(allocations[i]);
            allocations[i] = NULL;
        }
    }
}

#define REQUIRE(x)                                 \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "OA_ASSERT:%s\n", #x); \
            cleanup();                             \
            return 41;                             \
        }                                          \
    } while (0)

struct ob_heap
{
    size_t gets, puts, live;
    int fail;
};

static void *get(void *context, size_t n)
{
    struct ob_heap *h = context;
    void *p;
    ++h->gets;
    if (h->fail)
    {
        return NULL;
    }
    p = malloc(n);
    if (p)
    {
        for (size_t i = 0; i < 8; ++i)
        {
            if (!allocations[i])
            {
                allocations[i] = p;
                ++h->live;
                return p;
            }
        }
        free(p);
        return NULL;
    }
    return p;
}

static void put(void *context, void *p)
{
    struct ob_heap *h = context;
    ++h->puts;
    --h->live;
    for (size_t i = 0; i < 8; ++i)
    {
        if (allocations[i] == p)
        {
            allocations[i] = NULL;
            break;
        }
    }
    free(p);
}

static int lifetime(void)
{
    static struct oa_record fixed = {'s', 17, 2.0};
    struct oa_record automatic = {'a', 23, 3.0};
    struct ob_heap h = {0};
    struct oa_allocator a = {&h, get, put};
    struct oa_owner o = {0};
    REQUIRE(fixed.number == 17 && automatic.number == 23);
    REQUIRE(oa_resize(&o, 4, &a) == 0 && o.length == 4 && o.generation == 1);
    o.data[0] = 3;
    o.data[1] = 7;
    o.data[2] = 9;
    o.data[3] = 11;
    {
        unsigned char *still_live = o.data;
        h.fail = 1;
        REQUIRE(oa_resize(&o, 8, &a) == -1 && o.data == still_live &&
                o.length == 4 && o.generation == 1 && o.data[3] == 11);
        still_live = NULL;
        (void)still_live;
    }
    h.fail = 0;
#ifdef UINTPTR_MAX
    {
        uintptr_t history = (uintptr_t)(void *)o.data;
        REQUIRE(oa_resize(&o, 8, &a) == 0);
        printf("OA_ADDRESS_HISTORY:%" PRIuPTR " current:%" PRIuPTR "\n",
               history, (uintptr_t)(void *)o.data);
    }
#else
    REQUIRE(oa_resize(&o, 8, &a) == 0);
#endif
    REQUIRE(o.length == 8 && o.generation == 2 && o.data[0] == 3 &&
            o.data[3] == 11 && o.data[4] == 0 && h.live == 1 && h.puts == 1);
    REQUIRE(oa_resize(&o, 2, &a) == 0 && o.data[1] == 7 && o.generation == 3);
    o.generation = UINT_MAX;
    REQUIRE(oa_resize(&o, 4, &a) == -1 && o.length == 2);
    oa_release(&o, &a);
    REQUIRE(!o.data && !o.length && !h.live && h.puts == 3);
    puts("OA_LIFETIME_PASS");
    return 0;
}

static int escape(void)
{
    char dest[8] = "zzzzzzz", copy[8] = "xxxxxxx";
    struct oa_view v = {dest, 7}, deep = {copy, 7};
    REQUIRE(oa_label(dest, 3, &v) == -1 && dest[0] == 'z' && v.length == 7);
    REQUIRE(oa_label(dest, sizeof dest, &v) == 0 && v.data == dest &&
            v.length == 3 && !strcmp(dest, "cat"));
    REQUIRE(oa_deep_copy(v, copy, 2, &deep) == -1 && copy[0] == 'x' &&
            deep.length == 7);
    REQUIRE(oa_deep_copy(v, copy, sizeof copy, &deep) == 0 &&
            deep.data == copy && deep.length == 3);
    dest[0] = 'b';
    REQUIRE(deep.data[0] == 'c' && v.data[0] == 'b');
    puts("OA_ESCAPE_PASS");
    return 0;
}

static int representation(void)
{
    struct oa_record a = {'x', -2, 1.5}, b = {'x', -2, 1.5};

    struct fb_reordered
    {
        double measure;
        int32_t number;
        char marker;
    } reordered = {1.5, -2, 'x'};

    unsigned char *ap = (unsigned char *)&a, *bp = (unsigned char *)&b;
    size_t padding = 0;
    for (size_t i = 0; i < sizeof a; ++i)
    {
        int field =
            (i < sizeof a.marker) ||
            (i >= offsetof(struct oa_record, number) &&
             i < offsetof(struct oa_record, number) + sizeof a.number) ||
            (i >= offsetof(struct oa_record, measure) &&
             i < offsetof(struct oa_record, measure) + sizeof a.measure);
        if (!field)
        {
            ap[i] = 17;
            bp[i] = 99;
            ++padding;
        }
    }
    REQUIRE(oa_equal(&a, &b));
    if (padding)
    {
        REQUIRE(memcmp(&a, &b, sizeof a) != 0);
    }
    printf(
        "OA_LAYOUT:%zu,%zu,%zu,%zu,%zu reordered:%zu,%zu,%zu,%zu padding:%zu model:%zu,%zu,%d,%d\n",
        sizeof a, _Alignof(struct oa_record),
        offsetof(struct oa_record, marker), offsetof(struct oa_record, number),
        offsetof(struct oa_record, measure), sizeof reordered,
        offsetof(struct fb_reordered, marker),
        offsetof(struct fb_reordered, number),
        offsetof(struct fb_reordered, measure), padding, sizeof(void *),
        sizeof(long), CHAR_BIT, DBL_MANT_DIG);
    printf("OA_BYTES:");
    for (size_t i = 0; i < sizeof a; ++i)
    {
        printf("%02x", ap[i]);
    }
    puts("");
    REQUIRE(reordered.marker == a.marker && reordered.number == a.number &&
            reordered.measure == a.measure);
    puts("OA_REPRESENTATION_PASS");
    return 0;
}

static int equality(void)
{
    struct oa_record a = {'z', -2, 0.0}, b = {'z', -2, -0.0};
    unsigned char x[13], y[13], prior[13];
    REQUIRE(a.measure == b.measure && signbit(b.measure));
    REQUIRE(oa_equal(&a, &b));
    REQUIRE(oa_encode(&a, x, 13) == 0 && oa_encode(&b, y, 13) == 0 &&
            !memcmp(x, y, 13));
    REQUIRE(x[0] == 'z' && x[1] == 255 && x[2] == 255 && x[3] == 255 &&
            x[4] == 254);
    for (size_t i = 5; i < 13; ++i)
    {
        REQUIRE(x[i] == 0);
    }
    a.measure = NAN;
    b.measure = -NAN;
    REQUIRE(a.measure != a.measure && oa_equal(&a, &b));
    REQUIRE(oa_encode(&a, x, 13) == 0 && oa_encode(&b, y, 13) == 0 &&
            !memcmp(x, y, 13));
    REQUIRE(x[5] == 127 && x[6] == 248);
    for (size_t i = 7; i < 13; ++i)
    {
        REQUIRE(x[i] == 0);
    }
    a.measure = 1.5;
    REQUIRE(oa_encode(&a, x, 13) == 0);
    printf("OA_CANONICAL:");
    for (size_t i = 0; i < 13; ++i)
    {
        printf("%02x", x[i]);
    }
    puts("");
    memcpy(prior, x, 13);
    REQUIRE(oa_encode(&a, x, 12) == -1 && !memcmp(x, prior, 13));
    puts("OA_EQUALITY_PASS");
    return 0;
}

static int alignment(void)
{
    const size_t offsets[4] = {0, 1, 7, 8}, answers[4] = {0, 8, 8, 8};
    size_t at = 99;
    struct oa_storage storage = {0};
    struct oa_record *slot = &storage.records[0];
    const unsigned char wire[8] = {9, 1, 2, 3, 4, 9, 9, 9};
    uint32_t decoded = 77;
    for (size_t i = 0; i < 4; ++i)
    {
        REQUIRE(oa_place(32, offsets[i], 16, 8, 8, &at) == 0 &&
                at == answers[i]);
    }
    REQUIRE(oa_place(32, 1, 7, 1, 8, &at) == -1 && at == 8);
    REQUIRE(oa_place(32, 1, 8, 1, 8, &at) == 0 && at == 8);
    REQUIRE(oa_place(SIZE_MAX, SIZE_MAX, 1, 1, 8, &at) == -1 && at == 8);
    REQUIRE(oa_place(32, 0, 32, 1, 3, &at) == -1);
    REQUIRE(oa_slot(&storage, 1, sizeof storage - 1, &slot) == -1 &&
            slot == &storage.records[0]);
    REQUIRE(oa_slot(&storage, sizeof(struct oa_record),
                    sizeof(struct oa_record) - 1, &slot) == -1);
    REQUIRE(oa_slot(&storage, sizeof(struct oa_record),
                    sizeof(struct oa_record), &slot) == 0 &&
            slot == &storage.records[1]);
    slot->number = 73;
    REQUIRE(storage.records[1].number == 73);
    for (size_t i = 0; i < 4; ++i)
    {
        REQUIRE((i * sizeof(struct oa_record)) % _Alignof(struct oa_record) ==
                0);
    }
    REQUIRE(oa_wide_admit(64) && !oa_wide_admit(3));
    REQUIRE(oa_decode_u32(wire + 1, 3, &decoded) == -1 && decoded == 77);
    REQUIRE(oa_decode_u32(wire + 1, 4, &decoded) == 0 &&
            decoded == UINT32_C(0x01020304));
    if (_Alignof(max_align_t) < _Alignof(struct oa_wide))
    {
        REQUIRE(!oa_wide_admit(_Alignof(max_align_t)));
    }
    printf("OA_ALIGNMENT:record=%zu max=%zu wide=%zu\n",
           _Alignof(struct oa_record), _Alignof(max_align_t),
           _Alignof(struct oa_wide));
    puts("OA_ALIGNMENT_PASS");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "--table"))
    {
        const size_t lengths[] = {0, 1, 7, 8, 16, 33}, widths[] = {0, 1, 8},
                     aligns[] = {0, 1, 3, 8, 64};
        for (size_t start = 0; start <= 40; ++start)
        {
            for (size_t l = 0; l < 6; ++l)
            {
                for (size_t w = 0; w < 3; ++w)
                {
                    for (size_t a = 0; a < 5; ++a)
                    {
                        size_t out = 99;
                        int rc = oa_place(32, start, lengths[l], widths[w],
                                          aligns[a], &out);
                        printf("%zu,%zu,%zu,%zu,%d,%zu\n", start, lengths[l],
                               widths[w], aligns[a], rc, out);
                    }
                }
            }
        }
        return 0;
    }
    if (argc == 3 && !strcmp(argv[1], "--place"))
    {
        size_t start = (size_t)strtoull(argv[2], NULL, 10), out = 99;
        int rc = oa_place(32, start, 8, 1, 8, &out);
        printf("OA_PLACE:%d,%zu\n", rc, out);
        return 0;
    }
    switch (argc == 2 ? atoi(argv[1]) : -1)
    {
        case 0:
            return lifetime();
        case 1:
            return escape();
        case 2:
            return representation();
        case 3:
            return equality();
        case 4:
            return alignment();
        default:
            return 2;
    }
}
